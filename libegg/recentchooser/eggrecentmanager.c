/* GTK - The GIMP Toolkit
 * eggrecentmanager.c: a manager for the recently used resources
 *
 * Copyright (C) 2005 Emmanuele Bassi
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
 */

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>

#include <glib/gi18n.h>

#include <gtk/gtkstock.h>
#include <gtk/gtkicontheme.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkenums.h>
#include <gtk/gtktypebuiltins.h>

#ifdef G_OS_UNIX
#define XDG_PREFIX _gtk_xdg
#include "xdgmime/xdgmime.h"
#endif

#include "eggbookmarkfile.h"

#include "eggrecenttypebuiltins.h"
#include "eggrecentmanager.h"

/* the file where we store the recently used items */
#define EGG_RECENTLY_USED_FILE	".recently-used.xbel"

/* a poll per second should be enough */
#define POLL_DELTA	1000

/* return all items by default */
#define DEFAULT_LIMIT	-1

/* keep in sync with xdgmime */
#define EGG_RECENT_DEFAULT_MIME	"application/octet-stream"

typedef struct
{
  gchar *name;
  gchar *exec;
  
  guint count;
  
  time_t stamp;
} RecentAppInfo;

struct _EggRecentInfo
{
  gchar *uri;
  
  gchar *display_name;
  gchar *description;
  
  time_t added;
  time_t modified;
  time_t visited;
  
  gchar *mime_type;
  
  GSList *applications;
  GHashTable *apps_lookup;
  
  GSList *groups;
  
  gboolean is_private;
  
  GdkPixbuf *icon;
  
  gint ref_count;
};

struct _EggRecentManagerPrivate
{
  gchar *filename;

  guint is_dirty : 1;
  guint write_in_progress : 1;
  guint read_in_progress : 1;
  
  gint limit;
  gint size;
  
  EggBookmarkFile *recent_items;
  
  time_t last_mtime;
  guint poll_timeout;
};

enum
{
  PROP_0,
  
  PROP_LIMIT,
  PROP_SIZE
};

static void           egg_recent_manager_finalize     (GObject               *object);

static void           egg_recent_manager_set_property (GObject               *object,
						       guint                  prop_id,
						       const GValue          *value,
						       GParamSpec            *pspec);
static void           egg_recent_manager_get_property (GObject               *object,
						       guint                  prop_id,
						       GValue                *value,
						       GParamSpec            *pspec);
static void           egg_recent_manager_changed      (EggRecentManager      *manager);

static void           egg_recent_manager_real_changed (EggRecentManager      *manager);
static gboolean       egg_recent_manager_poll_timeout (gpointer               data);

static void           build_recent_items_list         (EggRecentManager      *manager);
static void           purge_recent_items_list         (EggRecentManager      *manager,
						       GError               **error);

static RecentAppInfo *recent_app_info_new             (const gchar           *app_name);
static void           recent_app_info_free            (RecentAppInfo         *app_info);

static EggRecentInfo *egg_recent_info_new             (const gchar           *uri);
static void           egg_recent_info_free            (EggRecentInfo         *recent_info);

static guint signal_changed = 0;

G_DEFINE_TYPE (EggRecentManager, egg_recent_manager, G_TYPE_OBJECT);

GQuark
egg_recent_manager_error_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("egg-recent-manager-error-quark");
  return quark;
}


static void
egg_recent_manager_class_init (EggRecentManagerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  egg_recent_manager_parent_class = g_type_class_peek_parent (klass);
  
  gobject_class->set_property = egg_recent_manager_set_property;
  gobject_class->get_property = egg_recent_manager_get_property;
  gobject_class->finalize = egg_recent_manager_finalize;
  
  /**
   * EggRecentManager:limit
   *
   * The maximum number of items to be returned by the
   * egg_recent_manager_get_items() function.
   *
   * Since: 2.10
   */
  g_object_class_install_property (gobject_class,
  				   PROP_LIMIT,
  				   g_param_spec_int ("limit",
  				   		     _("Limit"),
  				   		     _("The maximum number of items to be returned by egg_recent_manager_get_items()"),
  				   		     -1,
  				   		     G_MAXINT,
  				   		     DEFAULT_LIMIT,
  				   		     G_PARAM_READWRITE));
  /**
   * EggRecentManager:size
   * 
   * The size of the recently used resources list.
   *
   * Since: 2.10
   */
  g_object_class_install_property (gobject_class,
		  		   PROP_SIZE,
				   g_param_spec_int ("size",
					   	     _("Size"),
						     _("The size of the recently used resources list"),
						     -1,
						     G_MAXINT,
						     0,
						     G_PARAM_READABLE));
  
  /**
   * EggRecentManager::changed
   * @recent_manager: the recent manager
   *
   * Emitted when the current recently used resources manager changes its
   * contents.
   *
   * Since: 2.10
   */
  signal_changed =
    g_signal_new ("changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggRecentManagerClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
  
  klass->changed = egg_recent_manager_real_changed;
  
  g_type_class_add_private (klass, sizeof (EggRecentManagerPrivate));
}

static void
egg_recent_manager_init (EggRecentManager *manager)
{
  EggRecentManagerPrivate *priv;
  
  priv = g_type_instance_get_private ((GTypeInstance *) manager,
  				      EGG_TYPE_RECENT_MANAGER);
  manager->priv = priv;
  
  /* where we store the recently used resources data */
  priv->filename = g_build_filename (g_get_home_dir (),
  				     EGG_RECENTLY_USED_FILE,
  				     NULL);
  
  priv->limit = DEFAULT_LIMIT;
  priv->size = 0;
  
  build_recent_items_list (manager);
  
  priv->poll_timeout = g_timeout_add (POLL_DELTA,
  				      egg_recent_manager_poll_timeout,
  				      manager);
  
  priv->is_dirty = FALSE;
  priv->write_in_progress = FALSE;
  priv->read_in_progress = FALSE;
}

static void
egg_recent_manager_set_property (GObject               *object,
				 guint                  prop_id,
				 const GValue          *value,
				 GParamSpec            *pspec)
{
  EggRecentManager *recent_manager = EGG_RECENT_MANAGER (object);
 
  switch (prop_id)
    {
    case PROP_LIMIT:
      egg_recent_manager_set_limit (recent_manager, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_recent_manager_get_property (GObject               *object,
				 guint                  prop_id,
				 GValue                *value,
				 GParamSpec            *pspec)
{
  EggRecentManager *recent_manager = EGG_RECENT_MANAGER (object);
  
  switch (prop_id)
    {
    case PROP_LIMIT:
      g_value_set_int (value, recent_manager->priv->limit);
      break;
    case PROP_SIZE:
      g_value_set_int (value, recent_manager->priv->size);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
} 

static void
egg_recent_manager_finalize (GObject *object)
{
  EggRecentManager *manager = EGG_RECENT_MANAGER (object);
  EggRecentManagerPrivate *priv = manager->priv;

  /* remove the poll timeout */
  if (priv->poll_timeout)
    g_source_remove (priv->poll_timeout);
  
  if (priv->filename)
    g_free (priv->filename);
  
  if (priv->recent_items)
    egg_bookmark_file_free (priv->recent_items);
  
  /* chain up parent's finalize method */  
  G_OBJECT_CLASS (egg_recent_manager_parent_class)->finalize (object);
}

static void
egg_recent_manager_real_changed (EggRecentManager *manager)
{
  EggRecentManagerPrivate *priv = manager->priv;

  g_object_freeze_notify (G_OBJECT (manager));

  if (priv->is_dirty)
    {
      GError *write_error;
      struct stat stat_buf;
      
      /* we are marked as dirty, so we dump the content of our
       * recently used items list
       */
      g_assert (priv->filename != NULL);

      priv->write_in_progress = TRUE;

      /* if no container object has been defined, we create a new
       * empty container, and dump it
       */
      if (!priv->recent_items)
        {
          priv->recent_items = egg_bookmark_file_new ();
	  priv->size = 0;
	}

      write_error = NULL;
      egg_bookmark_file_to_file (priv->recent_items,
		      		 priv->filename,
				 &write_error);
      if (write_error)
        {
          g_warning ("Attempting to store changes into `%s', "
		     "but failed: %s",
		     priv->filename,
		     write_error->message);
	  g_error_free (write_error);
	}

      /* we have sync'ed our list with the storage file, so we
       * update the file mtime in order to skip the timed check
       * and spare us from a re-read.
       */
      if (stat (priv->filename, &stat_buf) < 0)
	{
          g_warning ("Unable to stat() the recently used resources file "
		     "at `%s': %s.",
		     priv->filename,
		     g_strerror (errno));

	  priv->write_in_progress = FALSE;
	  
	  g_object_thaw_notify (G_OBJECT (manager));

	  return;
	}
      
      priv->last_mtime = stat_buf.st_mtime;
      
      /* mark us as clean */
      priv->is_dirty = FALSE;
    }
  else
    {
      /* we are not marked as dirty, so we have been called
       * because the recently used resources file has been
       * changed (and not from us).
       */
      build_recent_items_list (manager);
    }

  g_object_thaw_notify (G_OBJECT (manager));
}

/* timed poll()-ing of the recently used resources file.
 * an event-based system would be more efficient.
 */
static gboolean
egg_recent_manager_poll_timeout (gpointer data)
{
  EggRecentManager *manager = EGG_RECENT_MANAGER (data);
  EggRecentManagerPrivate *priv = manager->priv;
  struct stat stat_buf;
  int stat_res;

  /* wait for the next timeout if we have a read/write in progress */
  if (priv->write_in_progress || priv->read_in_progress)
    return TRUE;

  stat_res = g_stat (priv->filename, &stat_buf);
  if (stat_res < 0)
    {
      /* the file does not exist, yet, so we wait */
      if (errno == ENOENT)
        return TRUE;
      
      g_warning ("Unable to stat() the recently used resources file "
		 "at `%s': %s.",
		 priv->filename,
		 g_strerror (errno));
      
      return TRUE;
    }

  /* the file didn't change from the last poll(), so we bail out */
  if (stat_buf.st_mtime == priv->last_mtime)
    return TRUE;

  /* the file has been changed, hence we emit the "changed" signal */
  egg_recent_manager_changed (manager);

  return TRUE;
}

/* reads the recently used resources file and builds the items list.
 * we keep the items list inside the parser object, and build the
 * RecentInfo object only on user's demand to avoid useless replication.
 */
static void
build_recent_items_list (EggRecentManager *manager)
{
  EggRecentManagerPrivate *priv;
  struct stat stat_buf;
  int stat_res;
  gboolean res;
  GError *read_error;
  gint size;

  priv = manager->priv;
  g_assert (priv->filename != NULL);
  
  if (!priv->recent_items)
    {
      priv->recent_items = egg_bookmark_file_new ();
      priv->size = 0;
    }

  stat_res = g_stat (priv->filename, &stat_buf);
  if (stat_res < 0)
    {
      /* the file doesn't exists, so we bail out and wait for the first
       * write operation
       */

      if (errno == ENOENT)
        return;
      else
        {
          g_warning ("Attempting to read the recently used resources file "
                     "at `%s', but an error occurred: %s. Aborting.",
                     priv->filename,
                     g_strerror (errno));

          return;
        }
    }

  /* record the last mtime, for later use */
  priv->last_mtime = stat_buf.st_mtime;

#if 0  
  /* the file exists, but it's empty; make it a valid recently used
   * resources file, and return
   *
   * XXX - This branch has been removed like the similar code in
   * EggRecentModel, since it spawned spurious events inside applications
   * like Sabayon; a more "polite" and conservative approach has been
   * preferred, where the storage file is created only when we actually
   * have something to write into it.  See bug #310721 for further
   * references. 
   */
  if (stat_buf.st_size == 0)
    {
      GError *write_error = NULL;
       
      egg_bookmark_file_to_file (priv->recent_items,
				 priv->filename,
				 &write_error);
      if (write_error)
	{
          g_warning ("Unable to create the recently used resources file "
		     "at `%s': %s.",
		     priv->filename,
		     write_error->message);
		       
	  g_error_free (write_error);
	    
	  egg_bookmark_file_free (priv->recent_items);
	    
	  priv->recent_items = NULL;
        }

      return;
    }
#endif

  priv->read_in_progress = TRUE;

  /* the file exists, and it's valid (we hope); if not, destroy the container
   * object and hope for a better result when the next "changed" signal is
   * fired. */
  read_error = NULL;
  res = egg_bookmark_file_load_from_file (priv->recent_items,
		  			  priv->filename,
					  &read_error);
  if (read_error)
    {
      g_warning ("Attempting to read the recently used resources file "
		 "at `%s', but the parser failed: %s.",
		 priv->filename,
		 read_error->message);

      egg_bookmark_file_free (priv->recent_items);
      priv->recent_items = NULL;

      g_error_free (read_error);
    }

  size = egg_bookmark_file_get_size (priv->recent_items);
  if (priv->size != size)
    {
      priv->size = size;
      
      g_object_notify (G_OBJECT (manager), "size");
    }

  priv->read_in_progress = FALSE;
}


/********************
 * EggRecentManager *
 ********************/


/**
 * egg_recent_manager_new:
 * 
 * Creates a new recent manager object.  Recent manager objects are used to
 * handle the list of recently used resources.  A #EggRecentManager object
 * monitors the recently used resources list, and emits the "changed" signal
 * each time something inside the list changes.
 *
 * #EggRecentManager objects are expansive: be sure to create them only when
 * needed.
 *
 * Return value: the newly created #EggRecentManager object.
 *
 * Since: 2.10
 */
EggRecentManager *
egg_recent_manager_new (void)
{
  return g_object_new (EGG_TYPE_RECENT_MANAGER, NULL);
}

/**
 * egg_recent_manager_set_limit:
 * @recent_manager: a #EggRecentManager
 * @limit: the maximum number of items to return, or -1.
 *
 * Sets the maximum number of item that the egg_recent_manager_get_items()
 * function should return.  If @limit is set to -1, then return all the
 * items.
 *
 * Since: 2.10
 */
void
egg_recent_manager_set_limit (EggRecentManager *recent_manager,
			      gint              limit)
{
  EggRecentManagerPrivate *priv;
  
  g_return_if_fail (EGG_IS_RECENT_MANAGER (recent_manager));
  
  priv = recent_manager->priv;
  priv->limit = limit;
}

/**
 * egg_recent_manager_get_limit:
 * @recent_manager: a #EggRecentManager
 *
 * Gets the maximum number of items that the egg_recent_manager_get_items()
 * function should return.
 *
 * Return value: the number of items to return, or -1 for every item.
 *
 * Since: 2.10
 */
gint
egg_recent_manager_get_limit (EggRecentManager *recent_manager)
{
  EggRecentManagerPrivate *priv;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), DEFAULT_LIMIT);
  
  priv = recent_manager->priv;
  return priv->limit;
}

/**
 * egg_recent_manager_add_item:
 * @recent_manager: a #EggRecentManager
 * @uri: a valid URI
 * @error: return location for a #GError, or %NULL
 *
 * Adds a new resource, pointed by @uri, into the recently used resources list.
 *
 * This function automatically retrieving some of the needed metadata and
 * setting other metadata to common default values; it then feeds the data to
 * egg_recent_manager_add_full().
 *
 * See egg_recent_manager_add_full() if you want to explicitely define the
 * metadata for the resource pointed by @uri.
 *
 * Return value: %TRUE if the new item was successfully added to the
 * recently used resources list, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_manager_add_item (EggRecentManager  *recent_manager,
			     const gchar       *uri,
			     GError           **error)
{
  EggRecentData *recent_data;
  GError *add_error;
  gboolean retval;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  recent_data = g_slice_new (EggRecentData);
  
  recent_data->display_name = NULL;
  recent_data->description = NULL;
  
#ifdef G_OS_UNIX
  if (g_str_has_prefix (uri, "file://"))
    {
      gchar *filename;
      const gchar *mime_type;
      
      filename = g_filename_from_uri (uri, NULL, NULL);
      mime_type = xdg_mime_get_mime_type_for_file (filename);
      if (!mime_type)
        recent_data->mime_type = g_strdup (EGG_RECENT_DEFAULT_MIME);
      else
        recent_data->mime_type = g_strdup (mime_type);
      
      g_free (filename);
    }
  else
#endif
    recent_data->mime_type = g_strdup (EGG_RECENT_DEFAULT_MIME);
  
  recent_data->app_name = g_strdup (g_get_application_name ());
  recent_data->app_exec = g_strjoin (" ", g_get_prgname (), "%u", NULL);
  
  recent_data->groups = NULL;
  
  recent_data->is_private = FALSE;
  
  add_error = NULL;
  retval = egg_recent_manager_add_full (recent_manager, uri, recent_data, &add_error);
  
  g_free (recent_data->mime_type);
  g_free (recent_data->app_name);
  g_free (recent_data->app_exec);

  g_slice_free (EggRecentData, recent_data);
  
  if (!retval)
    {
      g_propagate_error (error, add_error);
      
      return FALSE;
    }
  
  return retval;
}

/**
 * egg_recent_manager_add_full:
 * @recent_manager: a #EggRecentManager
 * @uri: a valid URI
 * @recent_data: metadata of the resource
 * @error: return location for a #GError, or %NULL
 *
 * Adds a new resource, pointed by @uri, into the recently used
 * resources list, using the metadata specified inside the #EggRecentData
 * structure passed in @recent_data.
 *
 * The passed URI will be used to identify this resource inside the
 * list.
 *
 * In order to register the new recently used resource, metadata about
 * the resource must be passed as well as the URI; the metadata is
 * stored in a #EggRecentData structure, which must contain the MIME
 * type of the resource pointed by the URI; the name of the application
 * that is registering the item, and a command line to be used when
 * launching the item.
 *
 * Optionally, a #EggRecentData structure might contain a UTF-8 string
 * to be used when viewing the item instead of the last component of the
 * URI; a short description of the item; whether the item should be
 * considered private - that is, should be displayed only by the
 * applications that have registered it.
 *
 * Return value: %TRUE if the new item was successfully added to the
 * recently used resources list, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_manager_add_full (EggRecentManager     *recent_manager,
			     const gchar          *uri,
			     const EggRecentData  *data,
			     GError              **error)
{
  EggRecentManagerPrivate *priv;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);
  g_return_val_if_fail (data != NULL, FALSE);

  /* sanity checks */
  if ((data->display_name) &&
      (!g_utf8_validate (data->display_name, -1, NULL)))
    {
      g_set_error  (error, EGG_RECENT_MANAGER_ERROR,
          	    EGG_RECENT_MANAGER_ERROR_INVALID_ENCODING,
          	    _("The display name of the recently used resource "
          	      "must be a valid UTF-8 encoded string."));
      return FALSE;
    }
  
  if ((data->description) &&
      (!g_utf8_validate (data->description, -1, NULL)))
    {
      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
          	   EGG_RECENT_MANAGER_ERROR_INVALID_ENCODING,
          	   _("The description of the recently used resource "
          	     "must by a valid UTF-8 encoded string."));
      return FALSE;
    }

 
  if (!data->mime_type)
    {
      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
                   EGG_RECENT_MANAGER_ERROR_INVALID_MIME,
		   _("You must specify the MIME type of the "
		     "resource pointed by `%s'"),
		   uri);
      return FALSE;
    }
  
  if (!data->app_name)
    {
      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
      		   EGG_RECENT_MANAGER_ERROR_NOT_REGISTERED,
      		   _("You must specify the name of the application "
      		     "that is registering the recently used resource "
      		     "pointed by `%s'"),
      		   uri);
      return FALSE;
    }
  
  if (!data->app_exec)
    {
      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
                   EGG_RECENT_MANAGER_ERROR_BAD_EXEC_STRING,
		   _("You must specify a command line to "
		     "be used when launching the resource "
		     "pointed by `%s'"),
		   uri);
      return FALSE;
    }
  
  priv = recent_manager->priv;

  if (!priv->recent_items)
    {
      priv->recent_items = egg_bookmark_file_new ();
      priv->size = 0;
    }

  if (data->display_name)  
    egg_bookmark_file_set_title (priv->recent_items, uri,
    				 data->display_name);
  
  if (data->description)
    egg_bookmark_file_set_description (priv->recent_items, uri,
    				       data->description);

  egg_bookmark_file_set_mime_type (priv->recent_items, uri,
		  		   data->mime_type);
  
  if (data->groups && data->groups[0] != '\0')
    {
      gint j;
      
      for (j = 0; (data->groups)[j] != NULL; j++)
        egg_bookmark_file_add_group (priv->recent_items, uri,
				     (data->groups)[j]);
    }
  
  /* register the application; this will take care of updating the
   * registration count and time in case the application has
   * already registered the same document inside the list
   */
  egg_bookmark_file_add_application (priv->recent_items, uri,
		  		     data->app_name,
				     data->app_exec);
  
  egg_bookmark_file_set_is_private (priv->recent_items, uri,
		  		    data->is_private);
  
  /* mark us as dirty, so that when emitting the "changed" signal we
   * will dump our changes
   */
  priv->is_dirty = TRUE;
  
  egg_recent_manager_changed (recent_manager);
  
  return TRUE;
}

/**
 * egg_recent_manager_remove_item:
 * @recent_manager: a #EggRecentManager
 * @uri: the URI of the item you wish to remove
 * @error: return location for a #GError, or %NULL
 *
 * Removes a resource pointed by @uri from the recently used resources
 * list handled by a recent manager.
 *
 * Return value: %TRUE if the item pointed by @uri has been successfully
 *   removed by the recently used resources list, and %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_manager_remove_item (EggRecentManager  *recent_manager,
				const gchar       *uri,
				GError           **error)
{
  EggRecentManagerPrivate *priv;
  GError *remove_error = NULL;

  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  
  priv = recent_manager->priv;
  
  if (!priv->recent_items)
    {
      priv->recent_items = egg_bookmark_file_new ();
      priv->size = 0;

      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
		   EGG_RECENT_MANAGER_ERROR_NOT_FOUND,
		   _("Unable to find an item with URI '%s'"),
		   uri);

      return FALSE;
    }

  egg_bookmark_file_remove_item (priv->recent_items, uri, &remove_error);
  if (remove_error)
    {
      g_propagate_error (error, remove_error);
      
      return FALSE;
    }

  priv->is_dirty = TRUE;

  egg_recent_manager_changed (recent_manager);
  
  return TRUE;
}

/**
 * egg_recent_manager_has_item:
 * @recent_manager: a #EggRecentManager
 * @uri: a URI
 *
 * Checks whether there is a recently used resource registered
 * with @uri inside the recent manager.
 *
 * Return value: %TRUE if the resource was found, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_manager_has_item (EggRecentManager *recent_manager,
			     const gchar      *uri)
{
  EggRecentManagerPrivate *priv;

  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);

  priv = recent_manager->priv;
  g_return_val_if_fail (priv->recent_items != NULL, FALSE);

  return egg_bookmark_file_has_item (priv->recent_items, uri);
}

static gboolean
build_recent_info (EggBookmarkFile  *bookmarks,
		   EggRecentInfo    *info)
{
  gchar **apps, **groups;
  gsize apps_len, groups_len, i;

  g_assert (bookmarks != NULL);
  g_assert (info != NULL);
  
  info->display_name = egg_bookmark_file_get_title (bookmarks, info->uri, NULL);
  info->description = egg_bookmark_file_get_description (bookmarks, info->uri, NULL);
  info->mime_type = egg_bookmark_file_get_mime_type (bookmarks, info->uri, NULL);
    
  info->is_private = egg_bookmark_file_get_is_private (bookmarks, info->uri, NULL);
  
  info->added = egg_bookmark_file_get_added (bookmarks, info->uri, NULL);
  info->modified = egg_bookmark_file_get_modified (bookmarks, info->uri, NULL);
  info->visited = egg_bookmark_file_get_visited (bookmarks, info->uri, NULL);
  
  groups = egg_bookmark_file_get_groups (bookmarks, info->uri, &groups_len, NULL);
  for (i = 0; i < groups_len; i++)
    {
      gchar *group_name = g_strdup (groups[i]);
      
      info->groups = g_slist_append (info->groups, group_name);
    }

  g_strfreev (groups);
  
  apps = egg_bookmark_file_get_applications (bookmarks, info->uri, &apps_len, NULL);
  for (i = 0; i < apps_len; i++)
    {
      gchar *app_name, *app_exec;
      guint count;
      time_t stamp;
      RecentAppInfo *app_info;
      gboolean res;
      
      app_name = apps[i];
      
      res = egg_bookmark_file_get_app_info (bookmarks,
      					    info->uri,
      					    app_name,
      					    &app_exec,
      					    &count,
      					    &stamp,
      					    NULL);
      if (!res)
        continue;
      
      app_info = recent_app_info_new (app_name);
      app_info->exec = app_exec;
      app_info->count = count;
      app_info->stamp = stamp;
      
      info->applications = g_slist_append (info->applications,
      					   app_info);
      g_hash_table_replace (info->apps_lookup, app_info->name, app_info);
    }
  
  g_strfreev (apps);
  
  return TRUE; 
}

/**
 * egg_recent_manager_lookup_item:
 * @recent_manager: a #EggRecentManager
 * @uri: a URI
 * @error: a return location for a #GError, or %NULL
 *
 * Searches for a URI inside the recently used resources list, and
 * returns a structure containing informations about the resource
 * like its MIME type, or its display name.
 *
 * Return value: a #EggRecentInfo structure containing information
 *   about the resource pointed by @uri, or %NULL if the URI was
 *   not registered in the recently used resources list.  Free with
 *   egg_recent_info_unref().
 **/
EggRecentInfo *
egg_recent_manager_lookup_item (EggRecentManager  *recent_manager,
				const gchar       *uri,
				GError           **error)
{
  EggRecentManagerPrivate *priv;
  EggRecentInfo *info = NULL;
  gboolean res;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), NULL);
  g_return_val_if_fail (uri != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  
  priv = recent_manager->priv;
  if (!priv->recent_items)
    {
      priv->recent_items = egg_bookmark_file_new ();
      priv->size = 0;

      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
		   EGG_RECENT_MANAGER_ERROR_NOT_FOUND,
		   _("Unable to find an item with URI '%s'"),
		   uri);

      return NULL;
    }
  
  if (!egg_bookmark_file_has_item (priv->recent_items, uri))
    {
      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
      		   EGG_RECENT_MANAGER_ERROR_NOT_FOUND,
      		   _("Unable to find an item with URI '%s'"),
      		   uri);
      return NULL;
    }
  
  info = egg_recent_info_new (uri);
  g_return_val_if_fail (info != NULL, NULL);
  
  /* fill the RecentInfo structure with the data retrieved by our
   * parser object from the storage file 
   */
  res = build_recent_info (priv->recent_items, info);
  if (!res)
    {
      egg_recent_info_free (info);
      
      return NULL;
    }
 
  return egg_recent_info_ref (info);
}

/**
 * egg_recent_manager_move_item:
 * @manager: a #EggRecentManager
 * @uri: the URI of a recently used resource
 * @new_uri: the new URI of the recently used resource, or %NULL to
 *    remove the item pointed by @uri in the list
 * @error: a return location for a #GError, or %NULL
 *
 * Changes the location of a recently used resource from @uri to @new_uri.
 * 
 * Please note that this function will not affect the resource pointed
 * by the URIs, but only the URI used in the recently used resources list.
 *
 * Return value: %TRUE on success.
 *
 * Since: 2.10
 */ 
gboolean
egg_recent_manager_move_item (EggRecentManager  *recent_manager,
			      const gchar       *uri,
			      const gchar       *new_uri,
			      GError           **error)
{
  EggRecentManagerPrivate *priv;
  GError *move_error;
  gboolean res;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), FALSE);
  g_return_val_if_fail (uri != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  
  priv = recent_manager->priv;

  if (!egg_bookmark_file_has_item (priv->recent_items, uri))
    {
      g_set_error (error, EGG_RECENT_MANAGER_ERROR,
      		   EGG_RECENT_MANAGER_ERROR_NOT_FOUND,
      		   _("Unable to find an item with URI '%s'"),
      		   uri);
      return FALSE;
    }
  
  move_error = NULL;
  res = egg_bookmark_file_move_item (priv->recent_items,
                                     uri, new_uri,
                                     &move_error);
  if (move_error)
    {
      g_propagate_error (error, move_error);
      return FALSE;
    }
  
  priv->is_dirty = TRUE;

  egg_recent_manager_changed (recent_manager);
  
  return TRUE;
}

/**
 * egg_recent_manager_get_items:
 * @recent_manager: a #EggRecentManager
 *
 * Gets the list of recently used resources.
 *
 * Return value: a list of newly allocated #EggRecentInfo objects. Use
 *   egg_recent_info_unref() on each item inside the list, and then
 *   free the list itself using g_list_free().
 *
 * Since: 2.10
 */
GList *
egg_recent_manager_get_items (EggRecentManager *recent_manager)
{
  EggRecentManagerPrivate *priv;
  GList *retval = NULL;
  gchar **uris;
  gsize uris_len, i;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), NULL);
  
  priv = recent_manager->priv;
  if (!priv->recent_items)
    return NULL;
  
  uris = egg_bookmark_file_get_uris (priv->recent_items, &uris_len);
  for (i = 0; i < uris_len; i++)
    {
      EggRecentInfo *info;
      gboolean res;
      
      info = egg_recent_info_new (uris[i]);
      res = build_recent_info (priv->recent_items, info);
      if (!res)
        {
          g_warning ("Unable to create a RecentInfo object for "
                     "item with URI `%s'",
                     uris[i]);
          egg_recent_info_free (info);
          continue;
        }
      
      retval = g_list_prepend (retval, info);
    }
  
  g_strfreev (uris);
    
  /* clamp the list, if a limit is present */
  if ((priv->limit != -1) &&
      (g_list_length (retval) > priv->limit))
    {
      GList *clamp, *l;
      
      clamp = g_list_nth (retval, priv->limit - 1);
      
      if (!clamp)
        return retval;
      
      l = clamp->next;
      clamp->next = NULL;
      
      g_list_foreach (l, (GFunc) egg_recent_info_free, NULL);
      g_list_free (l);
    }
  
  return retval;
}

static void
purge_recent_items_list (EggRecentManager  *manager,
			 GError           **error)
{
  EggRecentManagerPrivate *priv = manager->priv;

  if (!priv->recent_items)
    return;
  
  egg_bookmark_file_free (priv->recent_items);
  priv->recent_items = NULL;
      
  priv->recent_items = egg_bookmark_file_new ();
  priv->size = 0;
  priv->is_dirty = TRUE;
      
  /* emit the changed signal, to ensure that the purge is written */
  egg_recent_manager_changed (manager);
}

/**
 * egg_recent_manager_purge_items:
 * @recent_manager: a #EggRecentManager
 * @error: a return location for a #GError, or %NULL
 *
 * Purges every item from the recently used resources list.
 *
 * Return value: the number of items that have been removed from the
 *   recently used resources list.
 *
 * Since: 2.10
 */
gint
egg_recent_manager_purge_items (EggRecentManager  *recent_manager,
				GError           **error)
{
  EggRecentManagerPrivate *priv;
  gint count, purged;
  
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (recent_manager), -1);

  priv = recent_manager->priv;
  if (!priv->recent_items)
    return 0;
  
  count = egg_bookmark_file_get_size (priv->recent_items);
  if (!count)
    return 0;
  
  purge_recent_items_list (recent_manager, error);
  
  purged = count - egg_bookmark_file_get_size (priv->recent_items);

  return purged;
}

static void
egg_recent_manager_changed (EggRecentManager *recent_manager)
{
  g_signal_emit (recent_manager, signal_changed, 0);
}

/*****************
 * EggRecentInfo *
 *****************/
 
GType
egg_recent_info_get_type (void)
{
  static GType info_type = 0;
  
  if (!info_type)
    info_type = g_boxed_type_register_static ("EggRecentInfo",
    					      (GBoxedCopyFunc) egg_recent_info_ref,
    					      (GBoxedFreeFunc) egg_recent_info_unref);
  return info_type;
}

static EggRecentInfo *
egg_recent_info_new (const gchar *uri)
{
  EggRecentInfo *info;

  g_assert (uri != NULL);

  info = g_new0 (EggRecentInfo, 1);
  info->uri = g_strdup (uri);
  
  info->applications = NULL;
  info->apps_lookup = g_hash_table_new (g_str_hash, g_str_equal);
  
  info->groups = NULL;
  
  info->ref_count = 1;

  return info;
}

static void
egg_recent_info_free (EggRecentInfo *recent_info)
{
  if (!recent_info)
    return;

  g_free (recent_info->uri);
  g_free (recent_info->display_name);
  g_free (recent_info->description);
  g_free (recent_info->mime_type);
  
  if (recent_info->applications)
    {
      g_slist_foreach (recent_info->applications,
      		       (GFunc) recent_app_info_free,
      		       NULL);
      g_slist_free (recent_info->applications);
      
      recent_info->applications = NULL;
    }
  
  if (recent_info->apps_lookup)
    g_hash_table_destroy (recent_info->apps_lookup);

  if (recent_info->groups)
    {
      g_slist_foreach (recent_info->groups,
		       (GFunc) g_free,
		       NULL);
      g_slist_free (recent_info->groups);

      recent_info->groups = NULL;
    }
  
  if (recent_info->icon)
    g_object_unref (recent_info->icon);

  g_free (recent_info);
}

/**
 * egg_recent_info_ref:
 * @recent_info: a #EggRecentInfo
 *
 * Increases the reference count of @recent_info by one.
 *
 * Return value: the recent info object with its reference count increased
 *   by one.
 *
 * Since: 2.10
 */
EggRecentInfo *
egg_recent_info_ref (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, NULL);
  g_return_val_if_fail (recent_info->ref_count > 0, NULL);
  
  recent_info->ref_count += 1;
    
  return recent_info;
}

/**
 * egg_recent_info_unref:
 * @recent_info: a #EggRecentInfo
 *
 * Decreases the reference count of @recent_info by one.  If the reference
 * count reaches zero, @recent_info is deallocated, and the memory freed.
 *
 * Since: 2.10
 */
void
egg_recent_info_unref (EggRecentInfo *recent_info)
{
  g_return_if_fail (recent_info != NULL);
  g_return_if_fail (recent_info->ref_count > 0);

  recent_info->ref_count -= 1;
  
  if (recent_info->ref_count == 0)
    egg_recent_info_free (recent_info);
}

/**
 * egg_recent_info_get_uri:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the URI of the resource.
 *
 * Return value: the URI of the resource.  The returned string is
 *   owned by the recent manager, and should not be freed.
 *
 * Since: 2.10
 */
G_CONST_RETURN gchar *
egg_recent_info_get_uri (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  return recent_info->uri;
}

/**
 * egg_recent_info_get_display_name:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the name of the resource.  If none has been defined, the basename
 * of the resource is obtained.
 *
 * Return value: the display name of the resource.  The returned string
 *   is owned by the recent manager, and should not be freed.
 *
 * Since: 2.10
 */
G_CONST_RETURN gchar *
egg_recent_info_get_display_name (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  if (!recent_info->display_name)
    recent_info->display_name = egg_recent_info_get_short_name (recent_info);
  
  return recent_info->display_name;
}

/**
 * egg_recent_info_get_description:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the (short) description of the resource.
 *
 * Return value: the description of the resource.  The returned string
 *   is owned by the recent manager, and should not be freed.
 *
 * Since: 2.10
 **/
G_CONST_RETURN gchar *
egg_recent_info_get_description (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  return recent_info->description;
}

/**
 * egg_recent_info_get_mime_type:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the MIME type of the resource.
 *
 * Return value: the MIME type of the resource.  The returned string
 *   is owned by the recent manager, and should not be freed.
 *
 * Since: 2.10
 */
G_CONST_RETURN gchar *
egg_recent_info_get_mime_type (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  if (!recent_info->mime_type)
    recent_info->mime_type = g_strdup (EGG_RECENT_DEFAULT_MIME);
  
  return recent_info->mime_type;
}

/**
 * egg_recent_info_get_added:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the timestamp (seconds from system's Epoch) when the resource
 * was added to the recently used resources list.
 *
 * Return value: the number of seconds elapsed from system's Epoch when
 *   the resource was added to the list, or -1 on failure.
 *
 * Since: 2.10
 */
time_t
egg_recent_info_get_added (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, (time_t) -1);
  
  return recent_info->added;
}

/**
 * egg_recent_info_get_modified:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the timestamp (seconds from system's Epoch) when the resource
 * was last modified.
 *
 * Return value: the number of seconds elapsed from system's Epoch when
 *   the resource was last modified, or -1 on failure.
 *
 * Since: 2.10
 */
time_t
egg_recent_info_get_modified (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, (time_t) -1);
  
  return recent_info->modified;
}

/**
 * egg_recent_info_get_visited:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the timestamp (seconds from system's Epoch) when the resource
 * was last visited.
 *
 * Return value: the number of seconds elapsed from system's Epoch when
 *   the resource was last visited, or -1 on failure.
 *
 * Since: 2.10
 */
time_t
egg_recent_info_get_visited (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, (time_t) -1);
  
  return recent_info->visited;
}

/**
 * egg_recent_info_get_private_hint:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the value of the "private" flag.  Resources in the recently used
 * list that have this flag set to %TRUE should only be displayed by the
 * applications that have registered them.
 *
 * Return value: %TRUE if the private flag was found, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_get_private_hint (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, FALSE);
  
  return recent_info->is_private;
}


static RecentAppInfo *
recent_app_info_new (const gchar *app_name)
{
  RecentAppInfo *app_info;

  g_assert (app_name != NULL);
  
  app_info = g_new0 (RecentAppInfo, 1);
  app_info->name = g_strdup (app_name);
  app_info->exec = NULL;
  app_info->count = 1;
  app_info->stamp = time (NULL);
  
  return app_info;
}

static void
recent_app_info_free (RecentAppInfo *app_info)
{
  if (!app_info)
    return;
  
  if (app_info->name)
    g_free (app_info->name);
  
  if (app_info->exec)
    g_free (app_info->exec);
  
  g_free (app_info);
}

/**
 * egg_recent_info_get_application_info:
 * @recent_info: a #EggRecentInfo
 * @app_name: the name of the application that has registered this item
 * @app_exec: return location for the string containing the command line
 * @count: return location for the number of times this item was registered
 * @time: return location for the timestamp this item was last registered
 *    for this application
 *
 * Gets the data regarding the application that has registered the resource
 * pointed by @recent_info.
 *
 * If the command line contains any escape characters defined inside the
 * storage specification, they will be expanded.
 *
 * Return value: %TRUE if an application with @app_name has registered this
 *   resource inside the recently used list, or %FALSE otherwise.  You should
 *   free the returned command line using g_free().
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_get_application_info (EggRecentInfo  *recent_info,
				      const gchar    *app_name,
				      gchar         **app_exec,
				      guint          *count,
				      time_t         *time)
{
  RecentAppInfo *ai;
  
  g_return_val_if_fail (recent_info != NULL, FALSE);
  g_return_val_if_fail (app_name != NULL, FALSE);
  
  ai = (RecentAppInfo *) g_hash_table_lookup (recent_info->apps_lookup,
  					      app_name);
  if (!ai)
    {
      g_warning ("No registered application with name '%s' "
                 "for item with URI '%s' found",
                 app_name,
                 recent_info->uri);
      return FALSE;
    }
  
  if (app_exec)
    *app_exec = ai->exec;
  
  if (count)
    *count = ai->count;
  
  if (time)
    *time = ai->stamp;

  return TRUE;
}

/**
 * egg_recent_info_get_applications:
 * @recent_info: a #EggRecentInfo
 * @length: return location for the length of the returned list, or %NULL
 *
 * Retrieves the list of applications that have registered this resource.
 *
 * Return value: a newly allocated %NULL-terminated array of strings.
 *   Use g_strfreev() to free it.
 *
 * Since: 2.10
 */ 			      
gchar **
egg_recent_info_get_applications (EggRecentInfo *recent_info,
				  gsize         *length)
{
  GSList *l;
  gchar **retval;
  gsize n_apps, i;
  
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  if (!recent_info->applications)
    {
      if (length)
        *length = 0;
      
      return NULL;    
    }
  
  n_apps = g_slist_length (recent_info->applications);
  
  retval = g_new0 (gchar *, n_apps + 1);
  
  for (l = recent_info->applications, i = 0;
       l != NULL;
       l = l->next)
    {
      RecentAppInfo *ai = (RecentAppInfo *) l->data;
      
      g_assert (ai != NULL);
      
      retval[i++] = g_strdup (ai->name);
    }
  retval[i] = NULL;
  
  if (length)
    *length = i;
  
  return retval;
}

/**
 * egg_recent_info_has_application:
 * @recent_info: a #EggRecentInfo
 * @app_name: a string containing an application name
 *
 * Checks whether an application registered this resource using @app_name.
 *
 * Return value: %TRUE if an application with name @app_name was found,
 *   %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_has_application (EggRecentInfo *recent_info,
				 const gchar   *app_name)
{
  g_return_val_if_fail (recent_info != NULL, FALSE);
  g_return_val_if_fail (app_name != NULL, FALSE);
  
  return (NULL != g_hash_table_lookup (recent_info->apps_lookup, app_name));
}

/**
 * egg_recent_info_last_application:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the name of the last application that have registered the
 * recently used resource represented by @recent_info.
 *
 * Return value: an application name.  Use g_free() to free it.
 *
 * Since: 2.10
 */
gchar *
egg_recent_info_last_application (EggRecentInfo  *recent_info)
{
  GSList *l;
  time_t last_stamp = (time_t) -1;
  gchar *name = NULL;
  
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  for (l = recent_info->applications; l != NULL; l = l->next)
    {
      RecentAppInfo *ai = (RecentAppInfo *) l->data;
      
      if (ai->stamp > last_stamp)
        name = ai->name;
    }
  
  return g_strdup (name);
}

typedef struct
{
  gint size;
  GdkPixbuf *pixbuf;
} IconCacheElement;

static void
icon_cache_element_free (IconCacheElement *element)
{
  if (element->pixbuf)
    g_object_unref (element->pixbuf);
  g_free (element);
}

static void
icon_theme_changed (GtkIconTheme     *icon_theme)
{
  GHashTable *cache;

  /* Difference from the initial creation is that we don't
   * reconnect the signal
   */
  cache = g_hash_table_new_full (g_str_hash, g_str_equal,
				 (GDestroyNotify)g_free,
				 (GDestroyNotify)icon_cache_element_free);
  g_object_set_data_full (G_OBJECT (icon_theme), "egg-recent-icon-cache",
			  cache, (GDestroyNotify)g_hash_table_destroy);
}

/* TODO: use the GtkFileChooser's icon cache instead of our own to reduce
 * the memory footprint
 */
static GdkPixbuf *
get_cached_icon (const gchar *name,
		 gint         pixel_size)
{
  GtkIconTheme *icon_theme;
  GHashTable *cache;
  IconCacheElement *element;

  icon_theme = gtk_icon_theme_get_default ();
  cache = g_object_get_data (G_OBJECT (icon_theme), "egg-recent-icon-cache");

  if (!cache)
    {
      cache = g_hash_table_new_full (g_str_hash, g_str_equal,
				     (GDestroyNotify)g_free,
				     (GDestroyNotify)icon_cache_element_free);

      g_object_set_data_full (G_OBJECT (icon_theme), "egg-recent-icon-cache",
			      cache, (GDestroyNotify)g_hash_table_destroy);
      g_signal_connect (icon_theme, "changed",
			G_CALLBACK (icon_theme_changed), NULL);
    }

  element = g_hash_table_lookup (cache, name);
  if (!element)
    {
      element = g_new0 (IconCacheElement, 1);
      g_hash_table_insert (cache, g_strdup (name), element);
    }

  if (element->size != pixel_size)
    {
      if (element->pixbuf)
	g_object_unref (element->pixbuf);

      element->size = pixel_size;
      element->pixbuf = gtk_icon_theme_load_icon (icon_theme, name,
						  pixel_size, 0, NULL);
    }

  return element->pixbuf ? g_object_ref (element->pixbuf) : NULL;
}


static GdkPixbuf *
get_icon_for_mime_type (const char *mime_type,
			gint        pixel_size)
{
  const char *separator;
  GString *icon_name;
  GdkPixbuf *pixbuf;

  separator = strchr (mime_type, '/');
  if (!separator)
    return NULL; /* maybe we should return a GError with "invalid MIME-type" */

  icon_name = g_string_new ("gnome-mime-");
  g_string_append_len (icon_name, mime_type, separator - mime_type);
  g_string_append_c (icon_name, '-');
  g_string_append (icon_name, separator + 1);
  pixbuf = get_cached_icon (icon_name->str, pixel_size);
  g_string_free (icon_name, TRUE);
  if (pixbuf)
    return pixbuf;

  icon_name = g_string_new ("gnome-mime-");
  g_string_append_len (icon_name, mime_type, separator - mime_type);
  pixbuf = get_cached_icon (icon_name->str, pixel_size);
  g_string_free (icon_name, TRUE);

  return pixbuf;
}

static GdkPixbuf *
get_icon_fallback (const gchar *icon_name,
		   gint         size)
{
  GtkIconTheme *icon_theme;
  GdkPixbuf *retval;

  icon_theme = gtk_icon_theme_get_default ();
  
  retval = gtk_icon_theme_load_icon (icon_theme, icon_name,
  				     size,
  				     GTK_ICON_LOOKUP_USE_BUILTIN,
  				     NULL);
  g_assert (retval != NULL);
  
  return retval; 
}

/**
 * egg_recent_info_get_icon:
 * @recent_info: a #EggRecentInfo
 * @size: the size of the icon in pixels
 *
 * Retrieves the icon of size @size associated to the resource MIME type.
 *
 * Return value: a #GdkPixbuf containing the icon, or %NULL.
 *
 * Since: 2.10
 */
GdkPixbuf *
egg_recent_info_get_icon (EggRecentInfo *recent_info,
			  gint           size)
{
  GdkPixbuf *retval = NULL;
  
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  if (recent_info->mime_type)
    retval = get_icon_for_mime_type (recent_info->mime_type, size);

  /* this should never fail */  
  if (!retval)
    retval = get_icon_fallback (GTK_STOCK_FILE, size);
  
  return retval;
}

/**
 * egg_recent_info_is_local:
 * @recent_info: a #EggRecentInfo
 *
 * Checks whether the resource is local or not by looking at the
 * scheme of its URI.
 *
 * Return value: %TRUE if the resource is local.
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_is_local (EggRecentInfo *recent_info)
{
  g_return_val_if_fail (recent_info != NULL, FALSE);
  
  return g_str_has_prefix (recent_info->uri, "file://");
}

/**
 * egg_recent_info_exists:
 * @recent_info: a #EggRecentInfo
 *
 * Checks whether the resource pointed by @recent_info still exists.  At
 * the moment this check is done only on resources pointing to local files.
 *
 * Return value: %TRUE if the resource exists
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_exists (EggRecentInfo *recent_info)
{
  gchar *filename;
  struct stat stat_buf;
  gboolean retval = FALSE;
  
  g_return_val_if_fail (recent_info != NULL, FALSE);
  
  /* we guarantee only local resources */
  if (!egg_recent_info_is_local (recent_info))
    return FALSE;
  
  filename = g_filename_from_uri (recent_info->uri, NULL, NULL);
  if (filename)
    {
      if (stat (filename, &stat_buf) == 0)
        retval = TRUE;
     
      g_free (filename);
    }
  
  return retval;
}

/**
 * egg_recent_info_match:
 * @a: a #EggRecentInfo
 * @b: a #EggRecentInfo
 *
 * Checks whether two #EggRecentInfo structures point to the same
 * resource.
 *
 * Return value: %TRUE if both #EggRecentInfo structures point to se same
 *   resource, %FALSE otherwise.
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_match (EggRecentInfo *a,
		       EggRecentInfo *b)
{
  g_return_val_if_fail (a != NULL, FALSE);
  g_return_val_if_fail (b != NULL, FALSE);
  
  return (0 == strcmp (a->uri, b->uri));
}

/* taken from gnome-vfs-uri.c */
static const gchar *
get_method_string (const gchar *substring, gchar **method_string)
{
  const gchar *p;
  char *method;
	
  for (p = substring;
       g_ascii_isalnum (*p) || *p == '+' || *p == '-' || *p == '.';
       p++)
    ;

  if (*p == ':'
#ifdef G_OS_WIN32
                &&
      !(p == substring + 1 && g_ascii_isalpha (*substring))
#endif
							   )
    {
      /* Found toplevel method specification.  */
      method = g_strndup (substring, p - substring);
      *method_string = g_ascii_strdown (method, -1);
      g_free (method);
      p++;
    }
  else
    {
      *method_string = g_strdup ("file");
      p = substring;
    }
  
  return p;
}

/* Stolen from gnome_vfs_make_valid_utf8() */
static char *
make_valid_utf8 (const char *name)
{
  GString *string;
  const char *remainder, *invalid;
  int remaining_bytes, valid_bytes;

  string = NULL;
  remainder = name;
  remaining_bytes = name ? strlen (name) : 0;

  while (remaining_bytes != 0)
    {
      if (g_utf8_validate (remainder, remaining_bytes, &invalid))
        break;
      
      valid_bytes = invalid - remainder;
      
      if (string == NULL)
        string = g_string_sized_new (remaining_bytes);
      
      g_string_append_len (string, remainder, valid_bytes);
      g_string_append_c (string, '?');
      
      remaining_bytes -= valid_bytes + 1;
      remainder = invalid + 1;
    }
  
  if (string == NULL)
    return g_strdup (name);

  g_string_append (string, remainder);
  g_assert (g_utf8_validate (string->str, -1, NULL));

  return g_string_free (string, FALSE);
}

static gchar *
get_uri_shortname_for_display (const gchar *uri)
{
  gchar *name = NULL;
  gboolean validated = FALSE;

  if (g_str_has_prefix (uri, "file://"))
    {
      gchar *local_file;
      
      local_file = g_filename_from_uri (uri, NULL, NULL);
      
      if (local_file != NULL)
        {
          name = g_filename_display_basename (local_file);
          validated = TRUE;
        }
		
      g_free (local_file);
    } 
  else
    {
      gchar *method;
      gchar *local_file;
      const gchar *rest;
      
      rest = get_method_string (uri, &method);
      local_file = g_filename_display_basename (rest);
      
      name = g_strdup_printf ("%s: %s", method, local_file);
      
      g_free (local_file);
      g_free (method);
    }
  
  g_assert (name != NULL);
  
  if (!validated && !g_utf8_validate (name, -1, NULL))
    {
      gchar *utf8_name;
      
      utf8_name = make_valid_utf8 (name);
      g_free (name);
      
      name = utf8_name;
    }

  return name;
}

/**
 * egg_recent_info_get_short_name:
 * @recent_info: an #EggRecentInfo
 *
 * Computes a valid UTF-8 string that can be used as the name of the item in a
 * menu or list.  For example, calling this function on an item that refers to
 * "file:///foo/bar.txt" will yield "bar.txt".
 *
 * Return value: A newly-allocated string in UTF-8 encoding; free it with
 *   g_free().
 *
 * Since: 2.10
 */
gchar *
egg_recent_info_get_short_name (EggRecentInfo *recent_info)
{
  gchar *short_name;

  g_return_val_if_fail (recent_info != NULL, NULL);

  if (recent_info->uri == NULL)
    return NULL;

  short_name = get_uri_shortname_for_display (recent_info->uri);

  return short_name;
}

/**
 * egg_recent_info_get_uri_display:
 * @recent_info: a #EggRecentInfo
 *
 * Gets a displayable version of the resource's URI.
 *
 * Return value: a UTF-8 string containing the resource's URI or %NULL
 *
 * Since: 2.10
 */
gchar *
egg_recent_info_get_uri_display (EggRecentInfo *recent_info)
{
  gchar *filename, *filename_utf8;
  
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  filename = g_filename_from_uri (recent_info->uri, NULL, NULL);
  if (!filename)
    return NULL;
      
  filename_utf8 = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);
  g_free (filename);

  return filename_utf8;
}

/**
 * egg_recent_info_get_age:
 * @recent_info: a #EggRecentInfo
 *
 * Gets the number of days elapsed since the last update of the resource
 * pointed by @recent_info.
 *
 * Return value: a positive integer containing the number of days elapsed
 *   since the time this resource was last modified.  On failure, -1 is
 *   returned.
 *
 * Since: 2.10
 */
gint
egg_recent_info_get_age (EggRecentInfo *recent_info)
{
  time_t now, delta;
  gint retval;

  g_return_val_if_fail (recent_info != NULL, -1);

  now = time (NULL);
  
  delta = now - recent_info->modified;
  g_assert (delta >= 0);
  
  retval = (gint) (delta / (60 * 60 * 24));
  
  return retval;
}

/**
 * egg_recent_info_get_groups:
 * @recent_info: a #EggRecentInfo
 * @length: return location for the number of groups returned, or %NULL
 *
 * Returns all groups registered for the recently used item @recent_info.  The
 * array of returned group names will be %NULL terminated, so length might
 * optionally be %NULL.
 *
 * Return value: a newly allocated %NULL terminated array of strings.  Use
 *   g_strfreev() to free it.
 *
 * Since: 2.10
 */
gchar **
egg_recent_info_get_groups (EggRecentInfo *recent_info,
			    gsize         *length)
{
  GSList *l;
  gchar **retval;
  gsize n_groups, i;
  
  g_return_val_if_fail (recent_info != NULL, NULL);
  
  if (!recent_info->groups)
    {
      if (length)
        *length = 0;
      
      return NULL;
    }
  
  n_groups = g_slist_length (recent_info->groups);
  
  retval = g_new0 (gchar *, n_groups + 1);
  
  for (l = recent_info->groups, i = 0;
       l != NULL;
       l = l->next)
    {
      gchar *group_name = (gchar *) l->data;
      
      g_assert (group_name != NULL);
      
      retval[i++] = g_strdup (group_name);
    }
  retval[i] = NULL;
  
  if (length)
    *length = i;
  
  return retval;
}

/**
 * egg_recent_info_has_group:
 * @recent_info: a #EggRecentInfo
 * @group_name: name of a group
 *
 * Checks whether @group_name appears inside the groups registered for the
 * recently used item @recent_info.
 *
 * Return value: %TRUE if the group was found.
 *
 * Since: 2.10
 */
gboolean
egg_recent_info_has_group (EggRecentInfo *recent_info,
			   const gchar   *group_name)
{
  GSList *l;
  
  g_return_val_if_fail (recent_info != NULL, FALSE);
  g_return_val_if_fail (group_name != NULL, FALSE);

  if (!recent_info->groups)
    return FALSE;

  for (l = recent_info->groups; l != NULL; l = l->next)
    {
      gchar *g = (gchar *) l->data;

      if (strcmp (g, group_name) == 0)
        return TRUE;
    }

  return FALSE;
}
