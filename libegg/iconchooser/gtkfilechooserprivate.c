/* GTK - The GIMP Toolkit
 * gtkfilechooser.c: Abstract interface for file selector GUIs
 * Copyright (C) 2003, Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include "gtkfilechooserprivate.h"


/**
 * _gtk_file_chooser_set_current_folder_path:
 * @chooser: a #GtkFileChooser
 * @path: the #GtkFilePath for the new folder
 * @error: location to store error, or %NULL.
 * 
 * Sets the current folder for @chooser from a #GtkFilePath.
 * Internal function, see gtk_file_chooser_set_current_folder_uri().
 *
 * Return value: %TRUE if the folder could be changed successfully, %FALSE
 * otherwise.
 *
 * Since: 2.4
 **/
gboolean
_gtk_file_chooser_set_current_folder_path (GtkFileChooser    *chooser,
					   const GtkFilePath *path,
					   GError           **error)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->set_current_folder (chooser, path, error);
}

/**
 * _gtk_file_chooser_get_current_folder_path:
 * @chooser: a #GtkFileChooser
 * 
 * Gets the current folder of @chooser as #GtkFilePath.
 * See gtk_file_chooser_get_current_folder_uri().
 * 
 * Return value: the #GtkFilePath for the current folder.
 * Free with gtk_file_path_free().
 *
 * Since: 2.4
 */
GtkFilePath *
_gtk_file_chooser_get_current_folder_path (GtkFileChooser *chooser)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->get_current_folder (chooser);  
}

/**
 * _gtk_file_chooser_select_path:
 * @chooser: a #GtkFileChooser
 * @path: the path to select
 * @error: location to store error, or %NULL
 * 
 * Selects the file referred to by @path. An internal function. See
 * _gtk_file_chooser_select_uri().
 *
 * Return value: %TRUE if both the folder could be changed and the path was
 * selected successfully, %FALSE otherwise.
 *
 * Since: 2.4
 **/
gboolean
_gtk_file_chooser_select_path (GtkFileChooser    *chooser,
			       const GtkFilePath *path,
			       GError           **error)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->select_path (chooser, path, error);
}

/**
 * _gtk_file_chooser_unselect_path:
 * @chooser: a #GtkFileChooser
 * @path: the filename to path
 * 
 * Unselects the file referred to by @path. An internal
 * function. See _gtk_file_chooser_unselect_uri().
 *
 * Since: 2.4
 **/
void
_gtk_file_chooser_unselect_path (GtkFileChooser    *chooser,
				 const GtkFilePath *path)
{
  g_return_if_fail (GTK_IS_FILE_CHOOSER (chooser));

  GTK_FILE_CHOOSER_GET_IFACE (chooser)->unselect_path (chooser, path);
}

/**
 * _gtk_file_chooser_get_paths:
 * @chooser: a #GtkFileChooser
 * 
 * Lists all the selected files and subfolders in the current folder of @chooser
 * as #GtkFilePath. An internal function, see gtk_file_chooser_get_uris().
 * 
 * Return value: a #GSList containing a #GtkFilePath for each selected
 *   file and subfolder in the current folder.  Free the returned list
 *   with g_slist_free(), and the paths with gtk_file_path_free().
 *
 * Since: 2.4
 **/
GSList *
_gtk_file_chooser_get_paths (GtkFileChooser *chooser)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->get_paths (chooser);
}

/**
 * _gtk_file_chooser_get_file_system:
 * @chooser: a #GtkFileChooser
 * 
 * Gets the #GtkFileSystem of @chooser; this is an internal
 * implementation detail, used for conversion between paths
 * and filenames and URIs.
 * 
 * Return value: the file system for @chooser.
 *
 * Since: 2.4
 **/
GtkFileSystem *
_gtk_file_chooser_get_file_system (GtkFileChooser *chooser)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->get_file_system (chooser);
}


/**
 * gtk_file_chooser_get_preview_filename:
 * @chooser: a #GtkFileChooser
 * 
 * Gets the filename that should be previewed in a custom preview
 * Internal function, see gtk_file_chooser_get_preview_uri().
 * 
 * Return value: the #GtkFilePath for the file to preview, or %NULL if no file
 *  is selected. Free with gtk_file_path_free().
 *
 * Since: 2.4
 **/
GtkFilePath *
_gtk_file_chooser_get_preview_path (GtkFileChooser *chooser)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), NULL);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->get_preview_path (chooser);
}

/**
 * _gtk_file_chooser_add_shortcut_folder:
 * @chooser: a #GtkFileChooser
 * @path: path of the folder to add
 * @error: location to store error, or %NULL
 * 
 * Adds a folder to be displayed with the shortcut folders in a file chooser.
 * Internal function, see gtk_file_chooser_add_shortcut_folder().
 * 
 * Return value: %TRUE if the folder could be added successfully, %FALSE
 * otherwise.
 *
 * Since: 2.4
 **/
gboolean
_gtk_file_chooser_add_shortcut_folder (GtkFileChooser    *chooser,
				       const GtkFilePath *path,
				       GError           **error)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->add_shortcut_folder (chooser, path, error);
}

/**
 * _gtk_file_chooser_remove_shortcut_folder:
 * @chooser: a #GtkFileChooser
 * @path: path of the folder to remove
 * @error: location to store error, or %NULL
 * 
 * Removes a folder from the shortcut folders in a file chooser.  Internal
 * function, see gtk_file_chooser_remove_shortcut_folder().
 * 
 * Return value: %TRUE if the folder could be removed successfully, %FALSE
 * otherwise.
 *
 * Since: 2.4
 **/
gboolean
_gtk_file_chooser_remove_shortcut_folder (GtkFileChooser    *chooser,
					  const GtkFilePath *path,
					  GError           **error)
{
  g_return_val_if_fail (GTK_IS_FILE_CHOOSER (chooser), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);

  return GTK_FILE_CHOOSER_GET_IFACE (chooser)->remove_shortcut_folder (chooser, path, error);
}
