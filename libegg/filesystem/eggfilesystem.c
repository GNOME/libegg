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

#include "eggfilesystem.h"

#ifndef EGG_COMPILATION
#ifndef _
#define _(x) dgettext (GETTEXT_PACKAGE, x)
#define N_(x) x
#endif
#else
#define _(x) x
#define N_(x) x
#endif

GType
egg_file_system_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
	{	  
	  sizeof (EggFileSystemIface),
	  NULL,		/* base_init */
	  NULL,		/* base_finalize */
	  NULL,
	  NULL,		/* class_finalize */
	  NULL,		/* class_data */
	  0, 
	  0,              /* n_preallocs */
	  NULL
	};

      object_type = g_type_register_static (G_TYPE_INTERFACE, "EggFileSystem", &object_info, 0);
    }

  return object_type;
  
}

GType
egg_file_system_item_get_type (void)
{
  static GType boxed_type = 0;

  if (!boxed_type)
    {
      boxed_type = g_boxed_type_register_static ("EggFileSystemItem",
						 (GBoxedCopyFunc)egg_file_system_item_ref,
						 (GBoxedFreeFunc)egg_file_system_item_unref);
    }

  return boxed_type;
}

/* Error quark */
GQuark
egg_file_system_error_quark (void)
{
  static GQuark q = 0;
  if (q == 0)
    q = g_quark_from_static_string ("egg-file-system-error-quark");

  return q;
}

EggFileSystemItem *
egg_file_system_get_folder (EggFileSystem          *system,
			    EggFileSystemFolderType folder_type,
			    GError                **error)
{
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (system)->get_folder != NULL, NULL);

  return (* EGG_FILE_SYSTEM_GET_IFACE (system)->get_folder) (system, folder_type, error);
}

gchar *
egg_file_system_item_get_current_path (EggFileSystemItem      *item,
				       GError                **error)
{
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_current_path != NULL, NULL);

  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_current_path) (item->system, item, error);
}

EggFileSystemItem *
egg_file_system_get_item_by_name (EggFileSystem *system,
				  const gchar   *item_name,
				  GError       **error)
{
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (system)->get_item_by_name != NULL, NULL);

  return (* EGG_FILE_SYSTEM_GET_IFACE (system)->get_item_by_name) (system, item_name, error);
}

EggFileSystemItem *
egg_file_system_create_new_folder (EggFileSystem            *system,
				   const gchar              *folder_name,
				   GError                  **error)
{
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (system)->new_folder_create != NULL, NULL);

  return (* EGG_FILE_SYSTEM_GET_IFACE (system)->new_folder_create) (system, folder_name, error);
}

EggFileSystemItem *
egg_file_system_item_new (EggFileSystem *system)
{
  EggFileSystemItem *item;

  item = g_new (EggFileSystemItem, 1);
  item->ref_count = 1;
  item->system = system;
  item->priv = NULL;

  return item;
}

gchar *
egg_file_system_item_get_name (EggFileSystemItem *item)
{
  g_return_val_if_fail (item != NULL, NULL);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), NULL);
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_name != NULL, NULL);
  
  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_name) (item->system, item);
}

gchar *
egg_file_system_item_get_description (EggFileSystemItem *item)
{
  g_return_val_if_fail (item != NULL, NULL);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), NULL);
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_description != NULL, NULL);
  
  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_description) (item->system, item);
}


GList *
egg_file_system_item_get_children  (EggFileSystemItem *item,
				    GError           **error)
{
  g_return_val_if_fail (item != NULL, NULL);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), NULL);
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_children != NULL, NULL);

  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_children) (item->system, item, error);
}

EggFileSystemItem *
egg_file_system_item_get_parent (EggFileSystemItem  *item,
				 GError            **error)
{
  g_return_val_if_fail (item != NULL, NULL);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), NULL);
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_parent != NULL, NULL);

  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_parent) (item->system, item, error);
}

EggFileSystemItemType
egg_file_system_item_get_item_type (EggFileSystemItem *item)
{
  g_return_val_if_fail (item != NULL, 0);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), 0);  
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_type != NULL, 0);
  
  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_type) (item->system, item);
}

GdkPixbuf *
egg_file_system_item_get_icon (EggFileSystemItem    *item,
			       EggFileSystemIconSize icon_size)
{
  g_return_val_if_fail (item != NULL, 0);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), 0);  
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_icon != NULL, 0);
  
  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->get_item_icon) (item->system, item, icon_size);
}

gboolean
egg_file_system_item_is_root (EggFileSystemItem *item)
{
  g_return_val_if_fail (item != NULL, 0);
  g_return_val_if_fail (EGG_IS_FILE_SYSTEM (item->system), 0);  
  g_return_val_if_fail (EGG_FILE_SYSTEM_GET_IFACE (item->system)->item_is_root != NULL, 0);
  
  return (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->item_is_root) (item->system, item);
}

EggFileSystemItem *
egg_file_system_item_ref (EggFileSystemItem *item)
{
  g_return_val_if_fail (item != NULL, NULL);

  item->ref_count += 1;

  return item;
}

void
egg_file_system_item_unref (EggFileSystemItem *item)
{
  g_return_if_fail (item != NULL);

  item->ref_count -= 1;

  if (item->ref_count == 0)
    (* EGG_FILE_SYSTEM_GET_IFACE (item->system)->free_item) (item->system, item);
}

void
egg_file_system_item_list_free (GList *item_list)
{
  GList *p;

  p = item_list;

  while (p)
    {
      egg_file_system_item_unref (p->data);
      
      p = p->next;
    }

  g_list_free (item_list);
}
