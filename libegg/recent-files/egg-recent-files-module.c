/* egg-recent-files-module.c: GnomeVFS module to browse recent files list
 *
 * Copyright (C) 2003 Independent kids, Inc., Kristian Rietveld
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include <libgnomevfs/gnome-vfs-context.h>
#include <libgnomevfs/gnome-vfs-method.h>
#include <libgnomevfs/gnome-vfs-module.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <glib.h>
#include <string.h>

#include "egg-recent-model.h"

/* WARNING: there is some really evil code in here. Yes, I need to fix
 * some of that code up someday
 */

static gboolean utf8_string_match	(const gchar *a,
					 const gchar *b);

/* our shared EggRecentModel */
static EggRecentModel *recent_model = NULL;
G_LOCK_DEFINE_STATIC (recent_model);

/* la la la ... evil monitoring code */
static GList *monitoring_handles = NULL;
static GList *current_files = NULL;
G_LOCK_DEFINE_STATIC (monitoring_lock);


static void
monitoring_add_handle (gpointer handle)
{
	G_LOCK (monitoring_lock);
	monitoring_handles = g_list_append (monitoring_handles, handle);
	G_UNLOCK (monitoring_lock);
}

static void
monitoring_remove_handle (gpointer handle)
{
	G_LOCK (monitoring_lock);
	monitoring_handles = g_list_remove (monitoring_handles, handle);
	G_UNLOCK (monitoring_lock);
}

/* called in monitoring_lock */
static void
monitoring_refresh_current_files (void)
{
	GList *i;
	GList *list;

	list = egg_recent_model_get_list (recent_model);

	g_list_foreach (current_files, (GFunc)g_free, NULL);
	g_list_free (current_files);
	current_files = NULL;

	for (i = list; i; i = i->next)
		current_files = g_list_prepend (current_files,
				egg_recent_item_get_uri_utf8 (i->data));

	current_files = g_list_reverse (current_files);

	EGG_RECENT_ITEM_LIST_UNREF (list);
}

static void
monitoring_handle_changed (EggRecentModel *model,
			   GList          *list)
{
	GList *i;

	G_LOCK (monitoring_lock);

	for (i = list; i; i = i->next) {
		gchar *name;
		GList *j;
		gboolean handled = FALSE;
		EggRecentItem *item = (EggRecentItem *)i->data;

		name = egg_recent_item_get_uri_utf8 (item);

		for (j = current_files; j; j = j->next) {
			if (utf8_string_match (name, j->data)) {
				g_free (j->data);
				current_files = g_list_remove_link (current_files, j);
				handled = TRUE;
				break;
			}
		}

		g_free (name);

		if (!handled) {
			/* file was not in above list, a new file! */
			GnomeVFSURI *uri;
			gchar *tmp;
			int l, k;

			tmp = egg_recent_item_get_uri_utf8 (item);

			l = strlen (tmp);

			for (k = l - 1; k >= 0; k--)
				if (tmp[k] == '/')
					break;

			name = g_strdup_printf ("recent-files:///%s",
						tmp + k + 1);
			g_free (tmp);

			uri = gnome_vfs_uri_new (name);
			g_free (name);


			for (j = monitoring_handles; j; j = j->next)
				gnome_vfs_monitor_callback (j->data,
							    uri,
							    GNOME_VFS_MONITOR_EVENT_CREATED);

			gnome_vfs_uri_unref (uri);
		}
	}

	/* current_files now contains files which weren't handled above,
	 * so these files can be deleted
	 */

	for (i = current_files; i; i = i->next) {
		int l, k;
		gchar *tmp = i->data;
		gchar *name;
		GList *j;
		GnomeVFSURI *uri;

		l = strlen (tmp);

		for (k = l - 1; k >= 0; k--)
			if (tmp[k] == '/')
				break;

		name = g_strdup_printf ("recent-files:///%s", tmp + k + 1);
		uri = gnome_vfs_uri_new (name);
		g_free (name);

		for (j = monitoring_handles; j; j = j->next)
			gnome_vfs_monitor_callback (j->data,
						    uri,
						    GNOME_VFS_MONITOR_EVENT_DELETED);

		gnome_vfs_uri_unref (uri);
	}

	/* update current_files */
	monitoring_refresh_current_files ();

	G_UNLOCK (monitoring_lock);
}

static void
monitoring_setup (void)
{
	/* we need to sort on MRU, else we won't get the 10 most recently
	 * used items here, according to mister snorp.
	 */
	G_LOCK (recent_model);
	recent_model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	G_UNLOCK (recent_model);

	monitoring_refresh_current_files ();

	G_LOCK (recent_model);
	g_signal_connect (recent_model, "changed",
			  G_CALLBACK (monitoring_handle_changed), NULL);
	G_UNLOCK (recent_model);
}

static void
monitoring_cleanup (void)
{
	G_LOCK (recent_model);
	if (recent_model)
		g_object_unref (G_OBJECT (recent_model));
	recent_model = NULL;
	G_UNLOCK (recent_model);
}


/* somewhat more sane code */

static gchar *
get_filename_from_uri (const GnomeVFSURI *uri)
{
	gchar *path;
	gchar *ret;

	path = gnome_vfs_unescape_string (uri->text, G_DIR_SEPARATOR_S);

	if (!path)
		return NULL;

	if (path[0] != G_DIR_SEPARATOR) {
		g_free (path);
		return NULL;
	}

	ret = g_strdup (path + 1);
	g_free (path);

	return ret;
}

static gboolean
utf8_string_match (const gchar *a,
		   const gchar *b)
{
	gchar *normalized_a;
	gchar *normalized_b;
	gboolean ret;
	gint tmp;

	normalized_a = g_utf8_normalize (a, -1, G_NORMALIZE_ALL);
	normalized_b = g_utf8_normalize (b, -1, G_NORMALIZE_ALL);

	/* evil */
	tmp = strlen (normalized_b) - strlen (normalized_a);

	ret = !strcmp (normalized_b + tmp, normalized_a);

	g_free (normalized_a);
	g_free (normalized_b);

	return ret;
}

static EggRecentItem *
get_recent_item_from_list (const gchar *filename)
{
	GList *list, *i;

	G_LOCK (recent_model);
	list = egg_recent_model_get_list (recent_model);
	G_UNLOCK (recent_model);

	if (!list)
		return NULL;

	for (i = list; i; i = i->next) {
		gchar *name;
		EggRecentItem *item = (EggRecentItem *)i->data;

		name = egg_recent_item_get_uri_for_display (item);

		if (!strncmp (name, "recent-files:", 13)) {
			/* continue here to avoid endless loops */
			g_free (name);
			continue;
		}

		if (utf8_string_match (filename, name)) {
			g_free (name);

			egg_recent_item_ref (item);
			EGG_RECENT_ITEM_LIST_UNREF (list);

			return item;
		}

		g_free (name);
	}

	EGG_RECENT_ITEM_LIST_UNREF (list);

	return NULL;
}

static gchar *
get_real_uri_from_virtual_uri (const GnomeVFSURI *uri)
{
	gchar *filename;
	gchar *utf8_uri;
	EggRecentItem *item;

	filename = get_filename_from_uri (uri);
	if (!filename)
		return NULL;

	item = get_recent_item_from_list (filename);
	if (!item) {
		g_free (filename);
		return NULL;
	}

	utf8_uri = egg_recent_item_get_uri_utf8 (item);

	g_free (filename);
	egg_recent_item_unref (item);

	return utf8_uri;
}



static GnomeVFSResult
do_open (GnomeVFSMethod        *method,
	 GnomeVFSMethodHandle **method_handle,
	 GnomeVFSURI           *uri,
	 GnomeVFSOpenMode       mode,
	 GnomeVFSContext       *context)
{
	gchar *text_uri;
	GnomeVFSResult res;
	GnomeVFSHandle *handle = NULL;

	_GNOME_VFS_METHOD_PARAM_CHECK (method_handle != NULL);
	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	text_uri = get_real_uri_from_virtual_uri (uri);
	if (!text_uri)
		return GNOME_VFS_ERROR_INVALID_URI;

	res = gnome_vfs_open (&handle, text_uri, mode);
	g_free (text_uri);

	*method_handle = (GnomeVFSMethodHandle *)handle;

	return res;
}

static GnomeVFSResult
do_close (GnomeVFSMethod       *method,
	  GnomeVFSMethodHandle *method_handle,
	  GnomeVFSContext      *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_close (handle);
}

static GnomeVFSResult
do_read (GnomeVFSMethod       *method,
	 GnomeVFSMethodHandle *method_handle,
	 gpointer              buffer,
	 GnomeVFSFileSize      num_bytes,
	 GnomeVFSFileSize     *bytes_read,
	 GnomeVFSContext      *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_read (handle, buffer, num_bytes, bytes_read);
}

static GnomeVFSResult
do_write (GnomeVFSMethod       *method,
	  GnomeVFSMethodHandle *method_handle,
	  gconstpointer         buffer,
	  GnomeVFSFileSize      num_bytes,
	  GnomeVFSFileSize     *bytes_written,
	  GnomeVFSContext      *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_write (handle, buffer, num_bytes, bytes_written);
}

static GnomeVFSResult
do_seek (GnomeVFSMethod       *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSSeekPosition  whence,
	 GnomeVFSFileOffset    offset,
	 GnomeVFSContext      *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_seek (handle, whence, offset);
}

static GnomeVFSResult
do_tell (GnomeVFSMethod       *method,
	 GnomeVFSMethodHandle *method_handle,
	 GnomeVFSFileOffset   *offset_return)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_tell (handle, offset_return);
}

static GnomeVFSResult
do_truncate_handle (GnomeVFSMethod       *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSFileSize      where,
		    GnomeVFSContext      *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_truncate_handle (handle, where);
}

typedef struct {
	GList *items;
	GList *current;

	GnomeVFSFileInfoOptions options;
} DirectoryHandle;

static DirectoryHandle *
directory_handle_new (GnomeVFSFileInfoOptions options)
{
	DirectoryHandle *handle;

	handle = g_new0 (DirectoryHandle, 1);

	G_LOCK (recent_model);
	handle->items = egg_recent_model_get_list (recent_model);
	G_UNLOCK (recent_model);
	handle->current = handle->items;
	handle->options = options;

	return handle;
}

static void
directory_handle_free (DirectoryHandle *handle)
{
	EGG_RECENT_ITEM_LIST_UNREF (handle->items);

	g_free (handle);
}

static GnomeVFSResult
do_open_directory (GnomeVFSMethod          *method,
		   GnomeVFSMethodHandle   **method_handle,
		   GnomeVFSURI             *uri,
		   GnomeVFSFileInfoOptions  options,
		   GnomeVFSContext         *context)
{
	const gchar *scheme;
	gchar *tmp;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	scheme = gnome_vfs_uri_get_scheme (uri);
	if (strncmp (scheme, "recent-files", 12))
		return GNOME_VFS_ERROR_INVALID_URI;

	tmp = uri->text; /* evil ? */
	if (!tmp || strlen (tmp) != 1 || tmp[0] != GNOME_VFS_URI_PATH_CHR)
		return GNOME_VFS_ERROR_INVALID_URI;

	*method_handle = (GnomeVFSMethodHandle *)directory_handle_new (options);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_close_directory (GnomeVFSMethod       *method,
		    GnomeVFSMethodHandle *method_handle,
		    GnomeVFSContext      *context)
{
	DirectoryHandle *handle = (DirectoryHandle *)method_handle;

	directory_handle_free (handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_read_directory (GnomeVFSMethod       *method,
		   GnomeVFSMethodHandle *method_handle,
		   GnomeVFSFileInfo     *file_info,
		   GnomeVFSContext      *context)
{
	DirectoryHandle *handle = (DirectoryHandle *)method_handle;
	GnomeVFSResult res;
	gchar *uri;

	if (!handle->current)
		return GNOME_VFS_ERROR_EOF;

	uri = egg_recent_item_get_uri ((EggRecentItem *)handle->current->data);
	res = gnome_vfs_get_file_info (uri, file_info, handle->options);
	g_free (uri);

	handle->current = handle->current->next;

	return res; /* FIXME: or the res from _get_file_info? */
}

static GnomeVFSResult
do_get_file_info (GnomeVFSMethod          *method,
		  GnomeVFSURI             *uri,
		  GnomeVFSFileInfo        *file_info,
		  GnomeVFSFileInfoOptions  options,
		  GnomeVFSContext         *context)
{
	gchar *text_uri;
	GnomeVFSResult res;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	if (strlen (uri->text) == 1 && uri->text[0] == GNOME_VFS_URI_PATH_CHR) {
		/* handle root directory */
		file_info->valid_fields = 0;

		g_free (file_info->name);
		file_info->name = g_strdup ("Recent Files");

		file_info->type = GNOME_VFS_FILE_TYPE_DIRECTORY;
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_TYPE;

		g_free (file_info->mime_type);
		file_info->mime_type = g_strdup ("x-directory/normal");
		file_info->valid_fields |= GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;

		return GNOME_VFS_OK;
	}

	text_uri = get_real_uri_from_virtual_uri (uri);
	if (!text_uri)
		return GNOME_VFS_ERROR_INVALID_URI;

	res = gnome_vfs_get_file_info (text_uri, file_info, options);
	g_free (text_uri);

	return res;
}

static GnomeVFSResult
do_get_file_info_from_handle (GnomeVFSMethod          *method,
			      GnomeVFSMethodHandle    *method_handle,
			      GnomeVFSFileInfo        *file_info,
			      GnomeVFSFileInfoOptions  options,
			      GnomeVFSContext         *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_get_file_info_from_handle (handle,
						    file_info, options);
}

static gboolean
do_is_local (GnomeVFSMethod    *method,
	     const GnomeVFSURI *uri)
{
	gchar *text_uri;
	GnomeVFSURI *tmp_uri;
	gboolean ret;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	text_uri = get_real_uri_from_virtual_uri (uri);
	if (!text_uri)
		return GNOME_VFS_ERROR_INVALID_URI;

	tmp_uri = gnome_vfs_uri_new (text_uri);
	g_free (text_uri);
	if (!tmp_uri)
		return GNOME_VFS_ERROR_INTERNAL;

	ret = gnome_vfs_uri_is_local (tmp_uri);
	gnome_vfs_uri_unref (tmp_uri);

	return ret;
}

static GnomeVFSResult
do_unlink (GnomeVFSMethod  *method,
	   GnomeVFSURI     *uri,
	   GnomeVFSContext *context)
{
	gchar *text_uri;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	text_uri = get_real_uri_from_virtual_uri (uri);
	if (!text_uri)
		return GNOME_VFS_ERROR_INVALID_URI;

	G_LOCK (recent_model);
	egg_recent_model_delete (recent_model, text_uri);
	G_UNLOCK (recent_model);

	g_free (text_uri);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_check_same_fs (GnomeVFSMethod  *method,
		  GnomeVFSURI     *source_uri,
		  GnomeVFSURI     *target_uri,
		  gboolean        *same_fs_return,
		  GnomeVFSContext *context)
{
	gchar *a, *b;
	GnomeVFSResult res;

	_GNOME_VFS_METHOD_PARAM_CHECK (source_uri != NULL);
	_GNOME_VFS_METHOD_PARAM_CHECK (target_uri != NULL);

	a = get_real_uri_from_virtual_uri (source_uri);
	if (!a)
		return GNOME_VFS_ERROR_INVALID_URI;

	b = get_real_uri_from_virtual_uri (target_uri);
	if (!b) {
		g_free (a);
		return GNOME_VFS_ERROR_INVALID_URI;
	}

	res = gnome_vfs_check_same_fs (a, b, same_fs_return);

	g_free (a);
	g_free (b);

	return res;
}

static GnomeVFSResult
do_set_file_info (GnomeVFSMethod          *method,
		  GnomeVFSURI             *uri,
		  const GnomeVFSFileInfo  *info,
		  GnomeVFSSetFileInfoMask  mask,
		  GnomeVFSContext         *context)
{
	gchar *text_uri;
	GnomeVFSResult res;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	text_uri = get_real_uri_from_virtual_uri (uri);
	if (!text_uri)
		return GNOME_VFS_ERROR_INVALID_URI;

	res = gnome_vfs_set_file_info (text_uri, info, mask);
	g_free (text_uri);

	return res;
}

static GnomeVFSResult
do_truncate (GnomeVFSMethod   *method,
	     GnomeVFSURI      *uri,
	     GnomeVFSFileSize  where,
	     GnomeVFSContext  *context)
{
	gchar *text_uri;
	GnomeVFSResult res;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	text_uri = get_real_uri_from_virtual_uri (uri);
	if (!text_uri)
		return GNOME_VFS_ERROR_INVALID_URI;

	res = gnome_vfs_truncate (text_uri, where);
	g_free (text_uri);

	return res;
}

typedef struct {
	/* will prolly take up 4 bytes anyway ... */
	short foo:1;
} DummyMonitoringHandle;

static GnomeVFSResult
do_monitor_add (GnomeVFSMethod        *method,
		GnomeVFSMethodHandle **method_handle_return,
		GnomeVFSURI           *uri,
		GnomeVFSMonitorType    monitor_type)
{
	gchar *tmp;
	const gchar *scheme;

	_GNOME_VFS_METHOD_PARAM_CHECK (uri != NULL);

	scheme = gnome_vfs_uri_get_scheme (uri);
	if (strncmp (scheme, "recent-files", 12))
		return GNOME_VFS_ERROR_INVALID_URI;

	tmp = uri->text; /* evil? */
	if (!tmp || strlen (tmp) != 1 || tmp[0] != GNOME_VFS_URI_PATH_CHR)
		return GNOME_VFS_ERROR_NOT_SUPPORTED;

	if (monitor_type != GNOME_VFS_MONITOR_DIRECTORY)
		return GNOME_VFS_ERROR_BAD_PARAMETERS;

	/* accepted*/
	*method_handle_return = (GnomeVFSMethodHandle *)g_new (DummyMonitoringHandle, 1);

	monitoring_add_handle (*method_handle_return);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_monitor_cancel (GnomeVFSMethod       *method,
		   GnomeVFSMethodHandle *method_handle)
{
	/* remove handle */
	monitoring_remove_handle (method_handle);

	g_free (method_handle);

	return GNOME_VFS_OK;
}

static GnomeVFSResult
do_file_control (GnomeVFSMethod       *method,
		 GnomeVFSMethodHandle *method_handle,
		 const char           *operation,
		 gpointer              operation_data,
		 GnomeVFSContext      *context)
{
	GnomeVFSHandle *handle = (GnomeVFSHandle *)method_handle;

	return gnome_vfs_file_control (handle, operation, operation_data);
}

static GnomeVFSMethod method = {
	sizeof (GnomeVFSMethod),
	do_open,
	NULL, /* do_create */
	do_close,
	do_read,
	do_write,
	do_seek,
	do_tell,
	do_truncate_handle,
	do_open_directory,
	do_close_directory,
	do_read_directory,
	do_get_file_info,
	do_get_file_info_from_handle,
	do_is_local,
	NULL, /* do_make_directory */
	NULL, /* do_remove_directory */
	NULL, /* do_move */
	do_unlink,
	do_check_same_fs,
	do_set_file_info,
	do_truncate,
	NULL, /* do_find_directory */
	NULL, /* do_create_symbolic_link */
	do_monitor_add,
	do_monitor_cancel,
	do_file_control

};


/* shouldn't need locking here IIRC */
GnomeVFSMethod *
vfs_module_init (const char *method_name, const char *args)
{
	monitoring_setup ();

	return &method;
}

void
vfs_module_shutdown (GnomeVFSMethod *method)
{
	monitoring_cleanup ();
}
