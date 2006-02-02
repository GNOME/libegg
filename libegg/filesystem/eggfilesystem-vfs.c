/* eggfilesystem-unix.c
 * Copyright (C) 2002  Anders Carlsson <andersca@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "eggfilesystem-vfs.h"
#include "eggfilesystemutil.h"
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomeui/gnome-icon-theme.h>
#include <gconf/gconf-client.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <string.h>

#ifndef EGG_COMPILATION
#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif
#else
#define _(x) x
#define N_(x) x
#endif

static void egg_file_system_vfs_class_init (EggFileSystemVfsClass *klass);
static void egg_file_system_vfs_init       (EggFileSystemVfs      *system);

static void egg_file_system_vfs_file_system_init (EggFileSystemIface *iface);

static EggFileSystemItem *   egg_file_system_vfs_get_folder           (EggFileSystem            *system,
									EggFileSystemFolderType   folder_type,
									GError                  **error);
static gchar *               egg_file_system_vfs_get_current_path    (EggFileSystem            *system,
								       EggFileSystemItem        *item,
								       GError                  **error);
static EggFileSystemItem *   egg_file_system_vfs_get_item_by_name     (EggFileSystem            *system,
									const gchar              *item_name,
									GError                  **error);
static EggFileSystemItem *   egg_file_system_vfs_get_item_parent      (EggFileSystem            *system,
									EggFileSystemItem        *item,
									GError                  **error);
static GList *               egg_file_system_vfs_get_item_children    (EggFileSystem            *system,
									EggFileSystemItem        *item,
									GError                  **error);
static gchar *               egg_file_system_vfs_get_item_name        (EggFileSystem            *system,
									EggFileSystemItem        *item);
static gchar *               egg_file_system_vfs_get_item_description (EggFileSystem            *system,
									EggFileSystemItem        *item);
static EggFileSystemItemType egg_file_system_vfs_get_item_type        (EggFileSystem            *system,
									EggFileSystemItem        *item);
static GdkPixbuf *           egg_file_system_vfs_get_item_icon        (EggFileSystem            *system,
									EggFileSystemItem        *item,
									EggFileSystemIconSize     icon_size);
static gboolean              egg_file_system_vfs_item_is_root         (EggFileSystem            *system,
									EggFileSystemItem        *item);
static void                  egg_file_system_vfs_free_item            (EggFileSystem            *system,
									EggFileSystemItem        *item);


     
struct _EggFileSystemItemPrivate
{
  EggFileSystemItemType type;
  gchar *path;
  gchar *mime_type;
};

struct _EggFileSystemVfsPrivate
{
  GConfClient *gc;
  GnomeIconTheme *theme;
  gboolean show_hidden;
};

GType
egg_file_system_vfs_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
	{	  
	  sizeof (EggFileSystemVfsClass),
	  NULL,		/* base_init */
	  NULL,		/* base_finalize */
	  (GClassInitFunc) egg_file_system_vfs_class_init,
	  NULL,		/* class_finalize */
	  NULL,		/* class_data */
	  sizeof (EggFileSystemVfs),
	  0,            /* n_preallocs */
	  (GInstanceInitFunc) egg_file_system_vfs_init
	};

      static const GInterfaceInfo file_system_info =
      {
	(GInterfaceInitFunc) egg_file_system_vfs_file_system_init,
	NULL,
	NULL
      };
      
      object_type = g_type_register_static (G_TYPE_OBJECT, "EggFileSystemVfs", &object_info, 0);

      g_type_add_interface_static (object_type,
				   EGG_TYPE_FILE_SYSTEM,
				   &file_system_info);
    }

  return object_type;
  
}

static void
egg_file_system_vfs_file_system_init (EggFileSystemIface *iface)
{
  iface->get_folder = egg_file_system_vfs_get_folder;
  iface->get_item_current_path = egg_file_system_vfs_get_current_path;
  iface->get_item_by_name = egg_file_system_vfs_get_item_by_name;
  iface->get_item_name = egg_file_system_vfs_get_item_name;
  iface->get_item_description = egg_file_system_vfs_get_item_description;
  iface->get_item_type = egg_file_system_vfs_get_item_type;
  iface->get_item_icon = egg_file_system_vfs_get_item_icon;
  iface->get_item_children = egg_file_system_vfs_get_item_children;
  iface->get_item_parent = egg_file_system_vfs_get_item_parent;
  iface->item_is_root = egg_file_system_vfs_item_is_root;
  iface->free_item = egg_file_system_vfs_free_item;
}

static void
egg_file_system_vfs_finalize (GObject *object)
{
  EggFileSystemVfs *system;

  system = EGG_FILE_SYSTEM_VFS (object);

  g_object_unref (system->priv->gc);
  g_object_unref (system->priv->theme);
  g_free (system->priv);
}

static void
egg_file_system_vfs_class_init (EggFileSystemVfsClass *klass)
{
  if (gnome_vfs_initialized () == FALSE)
    gnome_vfs_init ();
  G_OBJECT_CLASS (klass)->finalize = egg_file_system_vfs_finalize;
}

static void
show_hidden_changed (GConfClient *client, guint cnxn_id,
		GConfEntry *entry, gpointer user_data)
{
	EggFileSystemVfs *system = (EggFileSystemVfs *) user_data;

	g_return_if_fail (entry != NULL);
	g_return_if_fail (entry->value != NULL);
	system->priv->show_hidden = gconf_value_get_bool (entry->value);
}

static void
egg_file_system_vfs_init (EggFileSystemVfs *system)
{
  system->priv = g_new (EggFileSystemVfsPrivate, 1);

  system->priv->theme = gnome_icon_theme_new ();
  system->priv->gc = gconf_client_get_default ();
  system->priv->show_hidden = gconf_client_get_bool (system->priv->gc,
      			"/desktop/gnome/file_views/show_hidden_files", NULL);
  gconf_client_add_dir (system->priv->gc, "/desktop/gnome/file_views",
      			GCONF_CLIENT_PRELOAD_NONE, NULL);
  gconf_client_notify_add (system->priv->gc,
      			"/desktop/gnome/file_views/show_hidden_files",
			show_hidden_changed, (gpointer) system, NULL, NULL);
}

static EggFileSystemItem *
egg_file_system_vfs_get_folder (EggFileSystem            *system,
			        EggFileSystemFolderType   folder_type,
				GError                  **error)
{
  EggFileSystemItem *item;
  char *path;

  /* FIXME: Handle the case where the folder is a file (!) */
  switch (folder_type)
    {
    case EGG_FILE_SYSTEM_FOLDER_CURRENT:
      path = g_strconcat ("file://", g_get_current_dir (), NULL);
      item = egg_file_system_get_item_by_name (system, path, error);
      g_free (path);
      return item;
    case EGG_FILE_SYSTEM_FOLDER_ROOT:
      return egg_file_system_get_item_by_name (system, "file:///", error);
    case EGG_FILE_SYSTEM_FOLDER_HOME:
      path = g_strconcat ("file://", g_get_home_dir (), NULL);
      item = egg_file_system_get_item_by_name (system, path, error);
      g_free (path);
      return item;
    default:
      g_set_error (error,
		   EGG_FILE_SYSTEM_ERROR,
		   EGG_FILE_SYSTEM_ERROR_UNKNOWN_FOLDER,
		   _("Unknown folder type %d"),
		   folder_type);
      return NULL;
    }
}

static gchar *
egg_file_system_vfs_get_current_path (EggFileSystem            *system,
				       EggFileSystemItem        *item,
				       GError                  **error)
{ 
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM_VFS (system), NULL);

  return item->priv->path;
}


static EggFileSystemItem *
egg_file_system_vfs_get_item_by_name (EggFileSystem *system,
				       const gchar   *item_name,
				       GError      **error)
{
  EggFileSystemVfs *vfs_system;
  EggFileSystemItem *item;
  GnomeVFSURI *uri;
  GnomeVFSFileInfo *info;
  
  vfs_system = EGG_FILE_SYSTEM_VFS (system);
  uri = gnome_vfs_uri_new (item_name);

  if (gnome_vfs_uri_exists (uri) == FALSE)
    {
      gnome_vfs_uri_unref (uri);
      /* FIXME: May be more than this error */
      g_set_error (error,
		   EGG_FILE_SYSTEM_ERROR,
		   EGG_FILE_SYSTEM_ERROR_DOES_NOT_EXIST,
		   _("File %s does not exist"),
		   item_name);
      return NULL;
    }
  
  /* Create a new item */
  item = egg_file_system_item_new (system);
  item->priv = g_new0 (EggFileSystemItemPrivate, 1);
  item->priv->path = g_strdup (item_name);
  item->priv->mime_type = gnome_vfs_get_mime_type (item_name);

  info = gnome_vfs_file_info_new ();
  gnome_vfs_get_file_info_uri (uri, info, GNOME_VFS_FILE_INFO_DEFAULT);
  gnome_vfs_uri_unref (uri);

  if (info->type == GNOME_VFS_FILE_TYPE_DIRECTORY)
    item->priv->type = EGG_FILE_SYSTEM_ITEM_FOLDER;
  else
    item->priv->type = EGG_FILE_SYSTEM_ITEM_FILE;

  gnome_vfs_file_info_unref (info);

  return item;
}

static GList *
egg_file_system_vfs_get_item_children (EggFileSystem      *system,
					EggFileSystemItem *item,
					GError           **error)
{
  GnomeVFSDirectoryHandle *handle;
  EggFileSystemVfs *vfs_system;
  const char *name;
  char *full_name;
  GList *list = NULL;
  EggFileSystemItem *child;
  GnomeVFSFileInfo *info;
  int len;

  if (gnome_vfs_directory_open (&handle, item->priv->path,
        GNOME_VFS_FILE_INFO_DEFAULT) != GNOME_VFS_OK)
    return NULL;

  vfs_system = EGG_FILE_SYSTEM_VFS (system);

  len = strlen (item->priv->path);

  while (1)
    {
      info = gnome_vfs_file_info_new ();
      if (gnome_vfs_directory_read_next (handle, info) != GNOME_VFS_OK)
      {
        gnome_vfs_file_info_unref (info);
	break;
      }

      if ((vfs_system->priv->show_hidden == FALSE) && (info->name[0] == '.'))
      {
        gnome_vfs_file_info_unref (info);
	continue;
      }

      if (strcmp (info->name, ".") == 0 || strcmp (info->name, "..") == 0)
      {
        gnome_vfs_file_info_unref (info);
	continue;
      }

      name = g_strdup (info->name);

      if (item->priv->path[len] == G_DIR_SEPARATOR)
        full_name = g_strdup_printf ("%s%s", item->priv->path, name);
      else
        full_name = g_strdup_printf ("%s%s%s", item->priv->path,
            G_DIR_SEPARATOR_S, name);

      child = egg_file_system_vfs_get_item_by_name (system, full_name, NULL);
      g_free (full_name);

      if (child)
	list = g_list_prepend (list, child);

      gnome_vfs_file_info_unref (info);
    }

  gnome_vfs_directory_close (handle);

  return list;
}

static EggFileSystemItem *
egg_file_system_vfs_get_item_parent (EggFileSystem     *system,
				      EggFileSystemItem *item,
				      GError           **error)
{
  gchar *path;
  EggFileSystemItem *parent;
  GnomeVFSURI *path_uri, *path_parent;

  if (item == NULL)
    return NULL;

  path_uri = gnome_vfs_uri_new (item->priv->path);
  path_parent = gnome_vfs_uri_get_parent (path_uri);
  if (path_parent == NULL)
  {
    gnome_vfs_uri_unref (path_uri);

    /* We're already at the root, so the parent is the root again */
    parent = egg_file_system_item_new (system);
    parent->priv = g_new0 (EggFileSystemItemPrivate, 1);
    parent->priv->type = item->priv->type;
    parent->priv->path = g_strdup (item->priv->path);
    parent->priv->mime_type = g_strdup (item->priv->mime_type);

    return parent;
  }
  path = gnome_vfs_uri_to_string (path_parent, 0);

  gnome_vfs_uri_unref (path_uri);
  gnome_vfs_uri_unref (path_parent);

  parent = egg_file_system_vfs_get_item_by_name (system, path, error);

  g_free (path);

  return parent;
}

static gchar *
egg_file_system_vfs_get_item_name (EggFileSystem     *system,
				    EggFileSystemItem *item)
{
  return g_path_get_basename (item->priv->path);
}

static gchar *
egg_file_system_vfs_get_item_description (EggFileSystem     *system,
					   EggFileSystemItem *item)
{
  char *desc;

  desc = g_strdup (gnome_vfs_mime_get_description (item->priv->mime_type));

  return desc;
}

static EggFileSystemItemType
egg_file_system_vfs_get_item_type (EggFileSystem     *system,
				    EggFileSystemItem *item)
{
  return item->priv->type;
}

static GdkPixbuf *
egg_file_system_vfs_get_item_icon (EggFileSystem        *system,
				    EggFileSystemItem    *item,
				    EggFileSystemIconSize icon_size)
{
  EggFileSystemVfs *vfs;
  static GdkPixbuf *pixbuf;

  vfs = EGG_FILE_SYSTEM_VFS (system);
  pixbuf = egg_file_system_util_get_icon (vfs->priv->theme, item->priv->path,
      					  item->priv->mime_type);

  return pixbuf;
}

static gboolean
egg_file_system_vfs_item_is_root (EggFileSystem     *system,
				   EggFileSystemItem *item)
{
  GnomeVFSURI *uri;
  gboolean retval = FALSE;

  uri = gnome_vfs_uri_new (item->priv->path);
  if (strcmp (gnome_vfs_uri_get_path (uri), "/") == 0)
    retval = TRUE;
  gnome_vfs_uri_unref (uri);

  return retval;
}

static void
egg_file_system_vfs_free_item (EggFileSystem     *system,
				EggFileSystemItem *item)
{
  g_free (item->priv->mime_type);
  g_free (item->priv->path);
  g_free (item->priv);
  g_free (item);
}
