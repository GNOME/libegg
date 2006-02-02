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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "eggfilesystem-unix.h"

#ifndef EGG_COMPILATION
#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif
#else
#define _(x) x
#define N_(x) x
#endif

static void egg_file_system_unix_class_init (EggFileSystemUnixClass *klass);
static void egg_file_system_unix_init       (EggFileSystemUnix      *system);

static void egg_file_system_unix_file_system_init (EggFileSystemIface *iface);

static EggFileSystemItem *   egg_file_system_unix_get_folder           (EggFileSystem            *system,
									EggFileSystemFolderType   folder_type,
									GError                  **error);
static gchar *               egg_file_system_unix_get_current_path     (EggFileSystem            *system,
									EggFileSystemItem        *item,
									GError                  **error);
static EggFileSystemItem *   egg_file_system_unix_get_item_by_name     (EggFileSystem            *system,
									const gchar              *item_name,
									GError                  **error);
static EggFileSystemItem *   egg_file_system_unix_get_item_parent      (EggFileSystem            *system,
									EggFileSystemItem        *item,
									GError                  **error);
static GList *               egg_file_system_unix_get_item_children    (EggFileSystem            *system,
									EggFileSystemItem        *item,
									GError                  **error);
static gchar *               egg_file_system_unix_get_item_name        (EggFileSystem            *system,
									EggFileSystemItem        *item);
static gchar *               egg_file_system_unix_get_item_description (EggFileSystem            *system,
									EggFileSystemItem        *item);
static EggFileSystemItemType egg_file_system_unix_get_item_type        (EggFileSystem            *system,
									EggFileSystemItem        *item);
static GdkPixbuf *           egg_file_system_unix_get_item_icon        (EggFileSystem            *system,
									EggFileSystemItem        *item,
									EggFileSystemIconSize     icon_size);
static gboolean              egg_file_system_unix_item_is_root         (EggFileSystem            *system,
									EggFileSystemItem        *item);
static void                  egg_file_system_unix_free_item            (EggFileSystem            *system,
									EggFileSystemItem        *item);


     
struct _EggFileSystemItemPrivate
{
  EggFileSystemItemType type;
  gchar *path;
};

GType
egg_file_system_unix_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
	{	  
	  sizeof (EggFileSystemUnixClass),
	  NULL,		/* base_init */
	  NULL,		/* base_finalize */
	  (GClassInitFunc) egg_file_system_unix_class_init,
	  NULL,		/* class_finalize */
	  NULL,		/* class_data */
	  sizeof (EggFileSystemUnix),
	  0,            /* n_preallocs */
	  (GInstanceInitFunc) egg_file_system_unix_init
	};

      static const GInterfaceInfo file_system_info =
      {
	(GInterfaceInitFunc) egg_file_system_unix_file_system_init,
	NULL,
	NULL
      };
      
      object_type = g_type_register_static (G_TYPE_OBJECT, "EggFileSystemUnix", &object_info, 0);

      g_type_add_interface_static (object_type,
				   EGG_TYPE_FILE_SYSTEM,
				   &file_system_info);
    }

  return object_type;
  
}

static void
egg_file_system_unix_file_system_init (EggFileSystemIface *iface)
{
  iface->get_folder = egg_file_system_unix_get_folder;
  iface->get_item_current_path = egg_file_system_unix_get_current_path;
  iface->get_item_by_name = egg_file_system_unix_get_item_by_name;
  iface->get_item_name = egg_file_system_unix_get_item_name;
  iface->get_item_description = egg_file_system_unix_get_item_description;
  iface->get_item_type = egg_file_system_unix_get_item_type;
  iface->get_item_icon = egg_file_system_unix_get_item_icon;
  iface->get_item_children = egg_file_system_unix_get_item_children;
  iface->get_item_parent = egg_file_system_unix_get_item_parent;
  iface->item_is_root = egg_file_system_unix_item_is_root;
  iface->free_item = egg_file_system_unix_free_item;
}

static void
egg_file_system_unix_class_init (EggFileSystemUnixClass *klass)
{
}

static void
egg_file_system_unix_init (EggFileSystemUnix *system)
{
}

static EggFileSystemItem *
egg_file_system_unix_get_folder (EggFileSystem            *system,
				 EggFileSystemFolderType   folder_type,
				 GError                  **error)
{
  EggFileSystemItem *item;
  char *path;

  /* FIXME: Handle the case where the folder is a file (!) */
  switch (folder_type)
    {
    case EGG_FILE_SYSTEM_FOLDER_CURRENT:
      path = g_get_current_dir ();
      item = egg_file_system_get_item_by_name (system, path, error);
      g_free (path);
      return item;
    case EGG_FILE_SYSTEM_FOLDER_ROOT:
      return egg_file_system_get_item_by_name (system, "/", error);
    case EGG_FILE_SYSTEM_FOLDER_HOME:
      return egg_file_system_get_item_by_name (system, g_get_home_dir (), error);
    default:
      g_set_error (error,
		   EGG_FILE_SYSTEM_ERROR,
		   EGG_FILE_SYSTEM_ERROR_UNKNOWN_FOLDER,
		   _("Unknown folder type %d"),
		   folder_type);
      return NULL;
    }
  g_print ("in get folder!\n");
}

static gchar *
egg_file_system_unix_get_current_path (EggFileSystem            *system,
				       EggFileSystemItem        *item,
				       GError                  **error)
{
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM_UNIX (system), NULL);

  return g_strdup (item->priv->path);
}

static EggFileSystemItem *
egg_file_system_unix_get_item_by_name (EggFileSystem *system,
				       const gchar   *item_name,
				       GError      **error)
{
  EggFileSystemUnix *unix_system;
  EggFileSystemItem *item;
  struct stat file_info;
  
  unix_system = EGG_FILE_SYSTEM_UNIX (system);
  
  if (stat (item_name, &file_info) < 0)
    {
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

  if (S_ISDIR (file_info.st_mode))
    item->priv->type = EGG_FILE_SYSTEM_ITEM_FOLDER;
  else
    item->priv->type = EGG_FILE_SYSTEM_ITEM_FILE;
    
  return item;
}

static GList *
egg_file_system_unix_get_item_children (EggFileSystem      *system,
					EggFileSystemItem *item,
					GError           **error)
{
  GDir *dir;
  const char *name;
  char *full_name;
  GList *list = NULL;
  EggFileSystemItem *child;

  dir = g_dir_open (item->priv->path, 0, NULL);

  while (1)
    {
      name = g_dir_read_name (dir);

      if (!name)
	break;

      full_name = g_build_filename (item->priv->path, name, NULL);
      
      child = egg_file_system_unix_get_item_by_name (system, full_name, NULL);
      g_free (full_name);
      
      if (child)
	list = g_list_prepend (list, child);
    }

  g_dir_close (dir);

  return list;
}

static EggFileSystemItem *
egg_file_system_unix_get_item_parent (EggFileSystem     *system,
				      EggFileSystemItem *item,
				      GError           **error)
{
  gchar *path;
  EggFileSystemItem *parent;
  
  path = g_path_get_dirname (item->priv->path);
  
  parent = egg_file_system_unix_get_item_by_name (system, path, error);
  
  g_free (path);

  return parent;
}

static gchar *
egg_file_system_unix_get_item_name (EggFileSystem     *system,
				    EggFileSystemItem *item)
{
  return g_path_get_basename (item->priv->path);
}

static gchar *
egg_file_system_unix_get_item_description (EggFileSystem     *system,
					   EggFileSystemItem *item)
{
  switch (item->priv->type)
    {
    case EGG_FILE_SYSTEM_ITEM_FOLDER:
      return g_strdup ("Folder");
    case EGG_FILE_SYSTEM_ITEM_FILE:
      return g_strdup ("File");
    default:
      g_assert_not_reached ();
    }

  return NULL;
}

static EggFileSystemItemType
egg_file_system_unix_get_item_type (EggFileSystem     *system,
				    EggFileSystemItem *item)
{
  return item->priv->type;
}

static GdkPixbuf *
egg_file_system_unix_get_item_icon (EggFileSystem        *system,
				    EggFileSystemItem    *item,
				    EggFileSystemIconSize icon_size)
{
  static gboolean initialized = FALSE;
  static GdkPixbuf *file_icon, *folder_icon;

  if (!initialized)
    {
      file_icon = gdk_pixbuf_new_from_file ("/usr/share/pixmaps/nautilus/default/i-regular.png", NULL);
      folder_icon = gdk_pixbuf_new_from_file ("/usr/share/pixmaps/nautilus/default/i-directory.png", NULL);
      
      initialized = TRUE;
    }
  switch (item->priv->type)
    {
    case EGG_FILE_SYSTEM_ITEM_FILE:
      return g_object_ref (file_icon);
    case EGG_FILE_SYSTEM_ITEM_FOLDER:
      return g_object_ref (folder_icon);
      
    default:
      g_assert_not_reached ();
    }

  return NULL;
}

static gboolean
egg_file_system_unix_item_is_root (EggFileSystem     *system,
				   EggFileSystemItem *item)
{
  return (strcmp (item->priv->path, "/") == 0);
}

static void
egg_file_system_unix_free_item (EggFileSystem     *system,
				EggFileSystemItem *item)
{
  g_free (item->priv->path);
  g_free (item->priv);
  g_free (item);
}
