/* eggfilesystem.h
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
#ifndef __EGG_FILE_SYSTEM_H__
#define __EGG_FILE_SYSTEM_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define EGG_TYPE_FILE_SYSTEM		(egg_file_system_get_type ())
#define EGG_FILE_SYSTEM(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_FILE_SYSTEM, EggFileSystem))
#define EGG_IS_FILE_SYSTEM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_FILE_SYSTEM))
#define EGG_FILE_SYSTEM_GET_IFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE ((obj), EGG_TYPE_FILE_SYSTEM, EggFileSystemIface))

#define EGG_TYPE_FILE_SYSTEM_ITEM       (egg_file_system_item_get_type ())

typedef struct _EggFileSystem            EggFileSystem;
typedef struct _EggFileSystemIface       EggFileSystemIface;
typedef struct _EggFileSystemItem        EggFileSystemItem;
typedef struct _EggFileSystemItemPrivate EggFileSystemItemPrivate;

typedef enum
{
  EGG_FILE_SYSTEM_FOLDER_ROOT,
  EGG_FILE_SYSTEM_FOLDER_HOME,
  EGG_FILE_SYSTEM_FOLDER_CURRENT
} EggFileSystemFolderType;

typedef enum
{
  EGG_FILE_SYSTEM_ITEM_FILE,
  EGG_FILE_SYSTEM_ITEM_FOLDER,
} EggFileSystemItemType;

typedef enum
{
  EGG_FILE_SYSTEM_ICON_REGULAR,
  EGG_FILE_SYSTEM_ICON_SMALL
} EggFileSystemIconSize;

struct _EggFileSystemItem
{
  EggFileSystem *system;
  gint ref_count;
  
  EggFileSystemItemPrivate *priv;
};

struct _EggFileSystemIface
{
  GTypeInterface g_iface;

  /* Virtual Table */
  EggFileSystemItem *   (* get_folder)           (EggFileSystem            *system,
						  EggFileSystemFolderType   folder_type,
						  GError                  **error);
  EggFileSystemItem *   (* new_folder_create)    (EggFileSystem            *system,
		  				  const gchar              *folder_name,
						  GError                  **error);
  gchar *               (* get_item_current_path)(EggFileSystem            *system,
		  				  EggFileSystemItem        *item,
						  GError                  **error);
  EggFileSystemItem *   (* get_item_by_name)     (EggFileSystem            *system,
						  const gchar              *folder_name,
						  GError                  **error);
  GList             *   (* get_item_children)    (EggFileSystem            *system,
						  EggFileSystemItem        *item,
						  GError                  **error);
  EggFileSystemItem *   (* get_item_parent)      (EggFileSystem            *system,
						  EggFileSystemItem        *item,
						  GError                  **error);
  gchar             *   (* get_item_name)        (EggFileSystem            *system,
						  EggFileSystemItem        *item);
  gchar             *   (* get_item_description) (EggFileSystem            *system,
						  EggFileSystemItem        *item);
  GdkPixbuf         *   (* get_item_icon)        (EggFileSystem            *system,
						  EggFileSystemItem        *item,
						  EggFileSystemIconSize     icon_size);
  EggFileSystemItemType (* get_item_type)        (EggFileSystem            *system,
						  EggFileSystemItem        *item);
  gboolean              (* item_is_root)         (EggFileSystem            *system,
						  EggFileSystemItem        *item);
  void                  (* free_item)            (EggFileSystem            *system,
						  EggFileSystemItem        *item);
};

typedef enum
{
  EGG_FILE_SYSTEM_ERROR_UNKNOWN_FOLDER,
  EGG_FILE_SYSTEM_ERROR_DOES_NOT_EXIST,
  EGG_FILE_SYSTEM_ERROR_IS_NOT_FOLDER
} EggFileSystemError;

#define EGG_FILE_SYSTEM_ERROR egg_file_system_error_quark ()

GQuark egg_file_system_error_quark   (void) G_GNUC_CONST;
GType  egg_file_system_get_type      (void) G_GNUC_CONST;
GType  egg_file_system_item_get_type (void) G_GNUC_CONST;

EggFileSystemItem *   egg_file_system_get_folder         (EggFileSystem            *system,
							  EggFileSystemFolderType   folder_type,
							  GError                  **error);
EggFileSystemItem *   egg_file_system_get_item_by_name   (EggFileSystem            *system,
							  const gchar              *item_name,
							  GError                  **error);
EggFileSystemItem *   egg_file_system_create_new_folder  (EggFileSystem            *system,
							  const gchar              *folder_name,
							  GError                  **error);

/* Item handling */
EggFileSystemItem *   egg_file_system_item_new             (EggFileSystem     *system);
EggFileSystemItem *   egg_file_system_item_ref             (EggFileSystemItem *item);
void                  egg_file_system_item_unref           (EggFileSystemItem *item);
gchar             *   egg_file_system_item_get_name        (EggFileSystemItem *item);
gchar             *   egg_file_system_item_get_current_path (EggFileSystemItem      *item,
							     GError                **error);
gchar             *   egg_file_system_item_get_description (EggFileSystemItem *item);
EggFileSystemItemType egg_file_system_item_get_item_type   (EggFileSystemItem *item);
GdkPixbuf         *   egg_file_system_item_get_icon        (EggFileSystemItem    *item,
							    EggFileSystemIconSize icon_size);
gboolean              egg_file_system_item_is_root         (EggFileSystemItem    *item);
EggFileSystemItem *   egg_file_system_item_get_parent      (EggFileSystemItem  *item,
							    GError            **error);
GList             *   egg_file_system_item_get_children    (EggFileSystemItem  *item,
							    GError            **error);


/* Item list handling */
void egg_file_system_item_list_free (GList *item_list);


G_END_DECLS

#endif /* __EGG_FILE_SYSTEM_H__ */
