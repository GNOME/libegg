/* Egg Libraries: eggiconchooser.c
 *
 * Copyright (c) 2004 James M. Cape.  <jcape@ignore-your.tv>
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; version 2.1 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib/gi18n.h>

#include <gtk/gtkenums.h>
#include <gtk/gtktypebuiltins.h>

#include "eggiconchoosertypebuiltins.h"
#include "eggiconchooserprivate.h"

enum
{
  ICON_SELECTION_CHANGED,
  ICON_ACTIVATED,
  FILE_SELECTION_CHANGED,
  FILE_ACTIVATED,
  LAST_SIGNAL
};


static guint signals[LAST_SIGNAL] = { 0 };


/* **************** *
 *  Initialization  *
 * **************** */

static void
egg_icon_chooser_class_init (gpointer g_iface)
{
  /**
   * EggIconChooser:file-system-backend:
   * 
   * The GtkFileSystem backend to use for the "Custom" pane.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_string ("file-system-backend",
							    _("File System Backend"),
							    _("The filesystem to use for the \"Custom\" pane."), NULL,
							    (G_PARAM_WRITABLE |
							     G_PARAM_CONSTRUCT_ONLY)));
  /**
   * EggIconChooser:context:
   * 
   * The icon theme context (category) to display.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_enum ("context",
							  _("Context"),
							  _("The current icon theme context."),
							  EGG_TYPE_ICON_CONTEXT,
							  EGG_ICON_CONTEXT_ALL,
							  G_PARAM_READWRITE));
  /**
   * EggIconChooser:select-multiple:
   * 
   * Whether or not the user can select multiple icons.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_boolean ("select-multiple",
							     _("Select Multiple"),
							     _("Whether or not the user can select multiple icons."), FALSE,
							     G_PARAM_READWRITE));
  /**
   * EggIconChooser:icon-size:
   * 
   * The desired icon size in pixels. If the value of this property is %-1, then
   * the current GTK+ theme's GTK_ICON_SIZE_DIALOG stock icon size will be used
   * (this is 48 pixels by default). If the icon size is between %0 and %12, the
   * actual size will be 12 pixels.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_int ("icon-size",
							 _("Icon Size"),
							 _("The desired icon size (in pixels)."),
							 -1, G_MAXINT, -1,
							 G_PARAM_READWRITE));
  /**
   * EggIconChooser:show-icon-name:
   * 
   * Whether or not icon names are visible in the chooser.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_boolean ("show-icon-name",
							     _("Show Icon Name"),
							     _("Whether or not to show icon names."),
							     FALSE,
							     G_PARAM_READWRITE));
  /**
   * EggIconChooser:allow-custom:
   * 
   * Whether or not the user can select the "Custom" group, which is used to
   * pick a file.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_boolean ("allow-custom",
							     _("Allow Custom"),
							     _("Whether or not the \"Custom\" group should be shown."),
							     TRUE,
							     G_PARAM_READWRITE));
  /**
   * EggIconChooser:custom-filter:
   * 
   * The GtkFileFilter to use when filtering the "Custom" group's file list.
   * 
   * Since: 2.8
   **/
  g_object_interface_install_property (g_iface,
				       g_param_spec_object ("custom-filter",
							    _("Custom Filter"),
							    _("The current custom filter."),
							    GTK_TYPE_FILE_FILTER,
							    G_PARAM_READWRITE));

  /**
   * EggIconChooser::icon-selection-changed:
   * @chooser:
   * 
   * #EggIconChooser widgets will emit this signal when the themed-icon
   * (non-"Custom") selection has changed. Applications which connect to this
   * signal should check if the "EggIconChooser:context" property is not
   * EGG_ICON_CHOOSER_CUSTOM before actually using this icon name.
   * 
   * Since: 2.8
   **/
  signals[ICON_SELECTION_CHANGED] = 
    g_signal_new ("icon-selection-changed",
		  G_TYPE_FROM_INTERFACE (g_iface),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggIconChooserIface, icon_selection_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
  /**
   * EggIconChooser::icon-activated:
   * @chooser:
   * 
   * #EggIconChooser widgets will emit this signal when the user activates
   * (double-clicks or hits Enter) a themed-icon selection has changed.
   * Applications which connect to this signal can then use the icon name.
   * 
   * Since: 2.8
   **/
  signals[ICON_ACTIVATED] = 
    g_signal_new ("icon-activated",
		  G_TYPE_FROM_INTERFACE (g_iface),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggIconChooserIface, icon_activated),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  /**
   * EggIconChooser::file-selection-changed:
   * @chooser:
   * 
   * #EggIconChooser widgets will emit this signal when the file ("Custom")
   * selection has changed. Applications which connect to this signal should
   * check to ensure the "EggIconChooser:context" property is set to
   * EGG_ICON_CHOOSER_CUSTOM before actually using the file selection, as this
   * signal is also emitted when an application calls
   * egg_icon_chooser_select_filename(), egg_icon_chooser_select_uri(), or
   * egg_icon_chooser_unselect_all().
   * 
   * Since: 2.8
   **/
  signals[FILE_SELECTION_CHANGED] = 
    g_signal_new ("file-selection-changed",
		  G_TYPE_FROM_INTERFACE (g_iface),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggIconChooserIface, file_selection_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
  signals[FILE_ACTIVATED] = 
    g_signal_new ("file-activated",
		  G_TYPE_FROM_INTERFACE (g_iface),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggIconChooserIface, file_activated),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}


/* ************************************************************************** *
 *  EggIconChooser Public API                                                 *
 * ************************************************************************** */

GType
egg_icon_chooser_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (type == G_TYPE_INVALID)
    {
      static const GTypeInfo info = {
	sizeof (EggIconChooserIface),	/* class_size */
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) egg_icon_chooser_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	0,
	0,			/* n_preallocs */
	NULL
      };

      type = g_type_register_static (G_TYPE_INTERFACE, "EggIconChooser", &info, 0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
    }

  return type;
}

/**
 * egg_icon_chooser_set_select_multiple:
 * @chooser: the chooser to modify.
 * @select_multiple: whether or not to allow selecting multiple icons.
 *
 * Changes whether @chooser will allow selecting multiple icons or not.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_set_select_multiple (EggIconChooser *chooser,
				       gboolean         select_multiple)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_object_set (chooser, "select-multiple", select_multiple, NULL);
}

/**
 * egg_icon_chooser_get_select_multiple:
 * @chooser: the chooser to examine.
 *
 * Retrieves whether or not @chooser will allow selecting multiple icons or not.
 * 
 * Returns: %TRUE if multiple selection is allowed, %FALSE if it is not.
 *
 * Since: 2.8
 **/
gboolean
egg_icon_chooser_get_select_multiple (EggIconChooser *chooser)
{
  gboolean retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), FALSE);

  g_object_get (chooser, "select-multiple", &retval, NULL);

  return retval;
}

/**
 * egg_icon_chooser_unselect_all:
 * @chooser: the chooser to examine.
 * 
 * Clears both the icon and file selections of @chooser.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_unselect_all (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->unselect_all != NULL);

  (*iface->unselect_all) (chooser);
}

/**
 * egg_icon_chooser_get_context:
 * @chooser: the chooser to examine.
 *
 * Retrieves the icon theme context that @chooser is currently showing.
 * 
 * Returns: the group that @chooser is currently showing.
 *
 * Since: 2.8
 **/
EggIconContext
egg_icon_chooser_get_context (EggIconChooser *chooser)
{
  EggIconContext retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser),
			EGG_ICON_CONTEXT_INVALID);

  g_object_get (chooser, "context", &retval, NULL);

  return retval;
}

/**
 * egg_icon_chooser_set_context:
 * @chooser: the chooser to modify.
 * @group: the group to use.
 *
 * Changes the group that @chooser is currently showing.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_set_context (EggIconChooser *chooser,
			      EggIconContext  context)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));
  g_return_if_fail (context > EGG_ICON_CONTEXT_INVALID &&
		    context < EGG_ICON_CONTEXT_LAST);

  g_object_set (chooser, "context", context, NULL);
}

/**
 * egg_icon_chooser_get_icon_size:
 * @chooser: the chooser to examine.
 *
 * Retrieves the pixel size that @chooser is using to render icons, or %-1
 * to indicate the theme-dependent #GTK_ICON_SIZE_DIALOG is being used.
 * 
 * Returns: the size in pixels that @chooser renders icons at, or %-1.
 *
 * Since: 2.8
 **/
gint
egg_icon_chooser_get_icon_size (EggIconChooser *chooser)
{
  gint retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), -1);

  g_object_get (chooser, "icon-size", &retval, NULL);

  return retval;
}

/**
 * egg_icon_chooser_set_icon_size:
 * @chooser: the chooser to modify.
 * @icon_size: the pixel size to render icons at.
 *
 * Changes the pixel size that @chooser renders icons at to @icon_size. If
 * @icon_size is equal to %-1, the chooser will use the theme-dependent
 * #GTK_ICON_SIZE_DIALOG.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_set_icon_size (EggIconChooser *chooser,
				gint            icon_size)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));
  g_return_if_fail (icon_size >= 12 || icon_size == -1);

  g_object_set (chooser, "icon-size", icon_size, NULL);
}

/**
 * egg_icon_chooser_get_show_icon_name:
 * @chooser: the chooser to examine.
 *
 * Retrieves whether or not @chooser will show the themed icon names.
 * 
 * Returns: whether or not @chooser will show the themed icon names.
 *
 * Since: 2.8
 **/
gboolean
egg_icon_chooser_get_show_icon_name (EggIconChooser *chooser)
{
  gboolean retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), -1);

  g_object_get (chooser, "show-icon-name", &retval, NULL);

  return retval;
}

/**
 * egg_icon_chooser_set_show_icon_name:
 * @chooser: the chooser to modify.
 * @show_names: the new value of the property.
 *
 * Changes whether or not @chooser will show the themed icon names under the
 * icons to @show_name.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_set_show_icon_name (EggIconChooser *chooser,
				     gboolean        show_name)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_object_set (chooser, "show-icon-name", show_name, NULL);
}

/**
 * egg_icon_chooser_get_icons:
 * @chooser: the chooser to examine.
 *
 * Retrieves a list of themed icon name strings corresponding to the
 * currently selected icons.
 *
 * Returns: a newly-allocated list of newly-allocated strings. The list
 *  contents should be freed with g_free(), and the list itself with
 *  g_slist_free().
 * 
 * Since: 1.0
 **/
GSList *
egg_icon_chooser_get_icons (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_icons != NULL, NULL);

  return (*iface->get_icons) (chooser);
}

/**
 * egg_icon_chooser_get_icon:
 * @chooser: the chooser to examine.
 *
 * Retrieves the themed icon name string corresponding to the selected icon.
 *
 * Returns: a newly-allocated icon name string, which should be freed with
 *  g_free().
 * 
 * Since: 2.8
 **/
gchar *
egg_icon_chooser_get_icon (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;
  GSList *list;
  gchar *retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_icons != NULL, NULL);

  list = (*iface->get_icons) (chooser);

  if (list != NULL)
    {
      retval = list->data;

      list = g_slist_remove_link (list, list);
      while (list != NULL)
	{
	  g_free (list->data);
	  list = g_slist_remove_link (list, list);
	}
    }
  else
    {
      retval = NULL;
    }

  return retval;
}

/**
 * egg_icon_chooser_select_icon:
 * @chooser: the chooser to examine.
 * @icon_name: the themed icon name to select.
 * 
 * Selects the icon corresponding to @icon_name.
 * 
 * Returns: %TRUE if @icon_name was selected, %FALSE if it was not.
 *
 * Since: 2.8
 **/
gboolean
egg_icon_chooser_select_icon (EggIconChooser *chooser,
			      const gchar    *icon_name)
{
  EggIconChooserIface *iface;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), FALSE);
  g_return_val_if_fail (icon_name != NULL, FALSE);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->select_icon != NULL, FALSE);

  return (*iface->select_icon) (chooser, icon_name);
}

/**
 * egg_icon_chooser_unselect_icon:
 * @chooser: the chooser to modify.
 * @icon_name: the path to unselect.
 * 
 * Removes @icon_name from @chooser's selected icons.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_unselect_icon (EggIconChooser *chooser,
			        const gchar    *icon_name)
{
  EggIconChooserIface *iface;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));
  g_return_if_fail (icon_name != NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->unselect_icon != NULL);

  (*iface->unselect_icon) (chooser, icon_name);
}

/**
 * egg_icon_chooser_set_allow_custom:
 * @chooser: the chooser to modify.
 * @allow_custom: whether or not to allow the custom group.
 *
 * Changes whether the custom group in @chooser can be selected.
 *
 * Since: 2.8
 **/
void
egg_icon_chooser_set_allow_custom (EggIconChooser *chooser,
				   gboolean         allow_custom)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_object_set (chooser, "allow-custom", allow_custom, NULL);
}

/**
 * egg_icon_chooser_get_allow_custom:
 * @chooser: the chooser to examine.
 *
 * Retrieves the whether or not the custom group is selectable in @chooser.
 * 
 * Returns: %TRUE if the custom group can be selected, %FALSE if it cannot.
 *
 * Since: 2.8
 **/
gboolean
egg_icon_chooser_get_allow_custom (EggIconChooser *chooser)
{
  gboolean retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), FALSE);

  g_object_get (chooser, "allow-custom", &retval, NULL);

  return retval;
}

/**
 * _egg_icon_chooser_get_file_system:
 * @chooser: the chooser to examine.
 * 
 * Retrieves a pointer to the file-system object used by @chooser's "custom"
 * group.
 * 
 * Returns: a pointer to a #GtkFileSystem object which should not be
 *  un-referenced.
 * 
 * Since: 2.8
 **/
GtkFileSystem *
_egg_icon_chooser_get_file_system (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_file_system != NULL, FALSE);

  return (*iface->get_file_system) (chooser);
}

/**
 * _egg_icon_chooser_select_path:
 * @chooser: the chooser to modify.
 * @path: the path to select.
 * @error: a pointer to a location to store errors in.
 * 
 * Selects @path in @chooser's file chooser group.
 * 
 * Returns: %TRUE if @path was selected, %FALSE if it was not.
 *
 * Since: 2.8
 **/
gboolean
_egg_icon_chooser_select_path (EggIconChooser     *chooser,
			       const GtkFilePath  *path,
			       GError            **error)
{
  EggIconChooserIface *iface;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->select_path != NULL, FALSE);

  return (*iface->select_path) (chooser, path, error);
}

/**
 * _egg_icon_chooser_unselect_path:
 * @chooser: the chooser to examine.
 * @path: the path to unselect.
 * 
 * Removes @path from @chooser's file selection.
 *
 * Since: 2.8
 **/
void
_egg_icon_chooser_unselect_path (EggIconChooser     *chooser,
			         const GtkFilePath  *path)
{
  EggIconChooserIface *iface;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));
  g_return_if_fail (path != NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->unselect_path != NULL);

  (*iface->unselect_path) (chooser, path);
}

/**
 * _egg_icon_chooser_get_paths:
 * @chooser: the chooser to examine.
 *
 * Retrieves a list of #GtkFilePath structures corresponding to the
 * current file-chooser selection (from the "Custom" group).
 *
 * Returns: a newly allocated list of #GtkFilePath structures which must be
 *  freed with gtk_file_paths_free().
 * 
 * Since: 2.8
 **/
GSList *
_egg_icon_chooser_get_paths (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_paths != NULL, NULL);

  return (*iface->get_paths) (chooser);
}

static GSList *
file_paths_to_strings (GtkFileSystem *fs,
                       GSList        *paths,
                       gchar *      (*convert_func) (GtkFileSystem *fs, const GtkFilePath *path))
{
  GSList *strings;

  strings = NULL;

  while (paths != NULL)
    {
      GtkFilePath *path;
      gchar *string;

      path = paths->data;
      string = (* convert_func) (fs, path);

      /* Eat the list */
      gtk_file_path_free (path);
      paths = g_slist_remove_link (paths, paths);

      if (string)
        strings = g_slist_prepend (strings, string);
    }

  return g_slist_reverse (strings);
}

/**
 * egg_icon_chooser_get_uris:
 * @chooser: the chooser to examine.
 *
 * Retrieves a list of UI strings corresponding to the current file-chooser
 * selection (from the "Custom" group).
 *
 * Returns: a newly-allocated list of newly-allocated strings.
 * 
 * Since: 2.8
 **/
GSList *
egg_icon_chooser_get_uris (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GSList *paths;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_file_system != NULL, NULL);
  g_return_val_if_fail (iface->get_paths != NULL, NULL);

  fs = (*iface->get_file_system) (chooser);
  paths = (*iface->get_paths) (chooser);

  return file_paths_to_strings (fs, paths, gtk_file_system_path_to_uri);
}

/**
 * egg_icon_chooser_get_uri:
 * @chooser: the chooser to examine.
 *
 * Retrieves the currently selected file-chooser URI (from the "Custom" group).
 * 
 * Returns: a newly-allocated string which must be freed with g_free().
 *
 * Since: 2.8
 **/
gchar *
egg_icon_chooser_get_uri (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GSList *paths;
  gchar *retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_file_system != NULL, NULL);
  g_return_val_if_fail (iface->get_paths != NULL, NULL);

  fs = (*iface->get_file_system) (chooser);
  paths = (*iface->get_paths) (chooser);

  if (paths != NULL)
    {
      retval = gtk_file_system_path_to_uri (fs, paths->data);
      gtk_file_paths_free (paths);
    }
  else
    {
      retval = NULL;
    }

  return retval;
}

gboolean
egg_icon_chooser_select_uri (EggIconChooser *chooser,
			     const gchar    *uri)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GtkFilePath *path;
  gboolean retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), FALSE);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->select_path != NULL, FALSE);
  g_return_val_if_fail (iface->get_file_system != NULL, FALSE);

  fs = (*iface->get_file_system) (chooser);

  path = gtk_file_system_uri_to_path (fs, uri);

  if (path != NULL)
    {
      retval = (*iface->select_path) (chooser, path, NULL);
      gtk_file_path_free (path);
    }
  else
    retval = FALSE;

  return retval;
}

void
egg_icon_chooser_unselect_uri (EggIconChooser *chooser,
			       const gchar    *uri)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GtkFilePath *path;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->unselect_path != NULL);
  g_return_if_fail (iface->get_file_system != NULL);

  fs = (*iface->get_file_system) (chooser);

  path = gtk_file_system_uri_to_path (fs, uri);

  if (path != NULL)
    {
      (*iface->unselect_path) (chooser, path);
      gtk_file_path_free (path);
    }
}

GSList *
egg_icon_chooser_get_filenames (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GSList *paths;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_file_system != NULL, NULL);
  g_return_val_if_fail (iface->get_paths != NULL, NULL);

  fs = (*iface->get_file_system) (chooser);
  paths = (*iface->get_paths) (chooser);

  return file_paths_to_strings (fs, paths, gtk_file_system_path_to_filename);
}

/**
 * egg_icon_chooser_get_filename:
 * @chooser: the chooser to examine.
 *
 * Retrieves the currently selected file-chooser local filename (from the
 * "Custom" group).
 * 
 * Returns: a newly-allocated string which must be freed with g_free().
 *
 * Since: 2.8
 **/
gchar *
egg_icon_chooser_get_filename (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GSList *paths;
  gchar *retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->get_file_system != NULL, NULL);
  g_return_val_if_fail (iface->get_paths != NULL, NULL);

  fs = (*iface->get_file_system) (chooser);
  paths = (*iface->get_paths) (chooser);

  if (paths != NULL)
    {
      retval = gtk_file_system_path_to_filename (fs, paths->data);
      gtk_file_paths_free (paths);
    }
  else
    {
      retval = NULL;
    }

  return retval;
}

gboolean
egg_icon_chooser_select_filename (EggIconChooser *chooser,
				  const gchar    *filename)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GtkFilePath *path;
  gboolean retval;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), FALSE);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->select_path != NULL, FALSE);
  g_return_val_if_fail (iface->get_file_system != NULL, FALSE);

  fs = (*iface->get_file_system) (chooser);

  path = gtk_file_system_filename_to_path (fs, filename);

  if (path != NULL)
    {
      retval = (*iface->select_path) (chooser, path, NULL);
      gtk_file_path_free (path);
    }
  else
    retval = FALSE;

  return retval;
}

void
egg_icon_chooser_unselect_filename (EggIconChooser *chooser,
				    const gchar    *filename)
{
  EggIconChooserIface *iface;
  GtkFileSystem *fs;
  GtkFilePath *path;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->unselect_path != NULL);
  g_return_if_fail (iface->get_file_system != NULL);

  fs = (*iface->get_file_system) (chooser);

  path = gtk_file_system_filename_to_path (fs, filename);

  if (path != NULL)
    {
      (*iface->unselect_path) (chooser, path);
      gtk_file_path_free (path);
    }
}


/**
 * egg_icon_chooser_list_custom_filters:
 * @chooser: the chooser to examine.
 *
 * Retrieves a list of #GtkFileFilter objects used by the custom group of
 * @chooser.
 * 
 * Returns: a list of filters which should be freed with g_slist_free(). The
 *  list data should not be un-referenced.
 *
 * Since: 2.6
 **/
GSList *
egg_icon_chooser_list_custom_filters (EggIconChooser *chooser)
{
  EggIconChooserIface *iface;

  g_return_val_if_fail (EGG_IS_ICON_CHOOSER (chooser), NULL);

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_val_if_fail (iface->list_filters != NULL, NULL);

  return (*iface->list_filters) (chooser);
}

/**
 * egg_icon_chooser_add_custom_filter:
 * @chooser: the chooser to modify.
 * @filter: the filter to add.
 *
 * Adds @filter to the custom group in @chooser.
 *
 * Since: 2.6
 **/
void
egg_icon_chooser_add_custom_filter (EggIconChooser *chooser,
				     GtkFileFilter   *filter)
{
  EggIconChooserIface *iface;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));
  g_return_if_fail (GTK_IS_FILE_FILTER (filter));

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->add_filter != NULL);

  (*iface->add_filter) (chooser, filter);
}

/**
 * egg_icon_chooser_remove_custom_filter:
 * @chooser: the chooser to modify.
 * @filter: the filter to remove.
 *
 * Removes @filter from the custom group in @chooser.
 *
 * Since: 2.6
 **/
void
egg_icon_chooser_remove_custom_filter (EggIconChooser *chooser,
					GtkFileFilter   *filter)
{
  EggIconChooserIface *iface;

  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));
  g_return_if_fail (GTK_IS_FILE_FILTER (filter));

  iface = EGG_ICON_CHOOSER_GET_IFACE (chooser);

  g_return_if_fail (iface->remove_filter != NULL);

  (*iface->remove_filter) (chooser, filter);
}


void
_egg_icon_chooser_icon_selection_changed (EggIconChooser *chooser)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_signal_emit (chooser, signals[ICON_SELECTION_CHANGED], 0);
}


void
_egg_icon_chooser_icon_activated (EggIconChooser *chooser)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_signal_emit (chooser, signals[ICON_ACTIVATED], 0);
}


void
_egg_icon_chooser_file_selection_changed (EggIconChooser *chooser)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_signal_emit (chooser, signals[FILE_SELECTION_CHANGED], 0);
}


void
_egg_icon_chooser_file_activated (EggIconChooser *chooser)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (chooser));

  g_signal_emit (chooser, signals[FILE_ACTIVATED], 0);
}
