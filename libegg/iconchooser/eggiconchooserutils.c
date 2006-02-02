/* GTK - The GIMP Toolkit
 * eggiconchooserutils.c: Private utility functions useful for
 *                         implementing a EggIconChooser interface
 * Copyright (C) 2003, Red Hat, Inc.
 * Copyright (C) 2003-2004, James M. Cape <jcape@ignore-your.tv>
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

#include "eggiconchooserutils.h"

/* Methods */
static void      delegate_add_filter             (EggIconChooser     *chooser,
						  GtkFileFilter      *filter);
static void      delegate_remove_filter          (EggIconChooser     *chooser,
						  GtkFileFilter      *filter);
static GSList   *delegate_list_filters           (EggIconChooser     *chooser);
static gboolean  delegate_select_icon            (EggIconChooser     *chooser,
						  const gchar        *icon_name);
static void      delegate_unselect_icon          (EggIconChooser     *chooser,
						  const gchar        *icon_name);
static GSList   *delegate_get_icons              (EggIconChooser     *chooser);
static GtkFileSystem *delegate_get_file_system   (EggIconChooser     *chooser);
static gboolean  delegate_select_path        	 (EggIconChooser     *chooser,
					          const GtkFilePath  *path,
						  GError            **error);
static void      delegate_unselect_path          (EggIconChooser     *chooser,
						  const GtkFilePath  *path);
static GSList   *delegate_get_paths              (EggIconChooser     *chooser);
static void      delegate_unselect_all           (EggIconChooser     *chooser);

/* Signals */
static void      delegate_notify                 (GObject        *receiver,
						  GParamSpec     *pspec,
						  gpointer        user_data);
static void      delegate_icon_selection_changed (EggIconChooser *receiver,
						  EggIconChooser *delegate);
static void      delegate_icon_activated         (EggIconChooser *receiver,
						  EggIconChooser *delegate);
static void      delegate_file_selection_changed (EggIconChooser *receiver,
						  EggIconChooser *delegate);
static void      delegate_file_activated         (EggIconChooser *receiver,
						  EggIconChooser *delegate);


/* ************************************************************************** *
 *  Library-Public API                                                        *
 * ************************************************************************** */

/**
 * _egg_icon_chooser_install_properties:
 * @klass: the class structure for a type deriving from #GObject
 * 
 * Installs the necessary properties for a class implementing
 * #EggIconChooser. A #GtkParamSpecOverride property is installed
 * for each property, using the values from the #EggIconChooserProp
 * enumeration. The caller must make sure itself that the enumeration
 * values don't collide with some other property values they
 * are using.
 **/
void
_egg_icon_chooser_install_properties (GObjectClass *klass)
{
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_FILE_SYSTEM,
				    "file-system-backend");
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_CONTEXT,
				    "context");
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE,
				    "select-multiple");
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_ICON_SIZE,
				    "icon-size");
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME,
				    "show-icon-name");
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM,
				    "allow-custom");
  g_object_class_override_property (klass,
				    EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER,
				    "custom-filter");
}


/**
 * _egg_icon_chooser_delegate_iface_init:
 * @iface: a #EggIconChoserIface structure
 * 
 * An interface-initialization function for use in cases where
 * an object is simply delegating the methods, signals of
 * the #EggIconChooser interface to another object.
 * _egg_icon_chooser_set_delegate() must be called on each
 * instance of the object so that the delegate object can
 * be found.
 **/
void
_egg_icon_chooser_delegate_iface_init (EggIconChooserIface *iface)
{
  iface->unselect_all = delegate_unselect_all;
  iface->select_icon = delegate_select_icon;
  iface->unselect_icon = delegate_unselect_icon;
  iface->get_icons = delegate_get_icons;
  iface->get_file_system = delegate_get_file_system;
  iface->select_path = delegate_select_path;
  iface->unselect_path = delegate_unselect_path;
  iface->get_paths = delegate_get_paths;
  iface->add_filter = delegate_add_filter;
  iface->remove_filter = delegate_remove_filter;
  iface->list_filters = delegate_list_filters;
}


/**
 * _egg_icon_chooser_set_delegate:
 * @receiver: a GOobject implementing #EggIconChooser
 * @delegate: another GObject implementing #EggIconChooser
 *
 * Establishes that calls on @receiver for #EggIconChooser
 * methods should be delegated to @delegate, and that
 * #EggIconChooser signals emitted on @delegate should be
 * forwarded to @receiver. Must be used in confunction with
 * _egg_icon_chooser_delegate_iface_init().
 **/
void
_egg_icon_chooser_set_delegate (EggIconChooser *receiver,
				EggIconChooser *delegate)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER (receiver));
  g_return_if_fail (EGG_IS_ICON_CHOOSER (delegate));
  
  g_object_set_qdata (G_OBJECT (receiver), EGG_ICON_CHOOSER_DELEGATE_QUARK,
		      delegate);

  g_signal_connect (delegate, "notify",
		    G_CALLBACK (delegate_notify), receiver);
  g_signal_connect (delegate, "icon-selection-changed",
		    G_CALLBACK (delegate_icon_selection_changed), receiver);
  g_signal_connect (delegate, "icon-activated",
		    G_CALLBACK (delegate_icon_activated), receiver);
  g_signal_connect (delegate, "file-selection-changed",
		    G_CALLBACK (delegate_file_selection_changed), receiver);
  g_signal_connect (delegate, "file-activated",
		    G_CALLBACK (delegate_file_activated), receiver);
}


/* ******************* *
 *  Utility Functions  *
 * ******************* */


static inline EggIconChooser *
get_delegate (EggIconChooser *receiver)
{
  return g_object_get_qdata (G_OBJECT (receiver), EGG_ICON_CHOOSER_DELEGATE_QUARK);
}


/* ****************** *
 *  Delegate Methods  *
 * ****************** */

static GtkFileSystem *
delegate_get_file_system (EggIconChooser *chooser)
{
  return _egg_icon_chooser_get_file_system (get_delegate (chooser));
}

static void
delegate_unselect_all (EggIconChooser *chooser)
{
  egg_icon_chooser_unselect_all (get_delegate (chooser));
}

static gboolean
delegate_select_icon (EggIconChooser *chooser,
		      const gchar     *icon_name)
{
  return egg_icon_chooser_select_icon (get_delegate (chooser), icon_name);
}

static void
delegate_unselect_icon (EggIconChooser *chooser,
			const gchar     *icon_name)
{
  egg_icon_chooser_unselect_icon (get_delegate (chooser), icon_name);
}

static GSList *
delegate_get_icons (EggIconChooser *chooser)
{
  return egg_icon_chooser_get_icons (get_delegate (chooser));
}


static gboolean
delegate_select_path (EggIconChooser     *chooser,
		      const GtkFilePath  *path,
		      GError            **error)
{
  return _egg_icon_chooser_select_path (get_delegate (chooser), path, error);
}

static void
delegate_unselect_path (EggIconChooser    *chooser,
			const GtkFilePath *path)
{
  _egg_icon_chooser_unselect_path (get_delegate (chooser), path);
}

static GSList *
delegate_get_paths (EggIconChooser *chooser)
{
  return _egg_icon_chooser_get_paths (get_delegate (chooser));
}

static void
delegate_add_filter (EggIconChooser *chooser,
		     GtkFileFilter   *filter)
{
  egg_icon_chooser_add_custom_filter (get_delegate (chooser), filter);
}

static void
delegate_remove_filter (EggIconChooser *chooser,
		        GtkFileFilter   *filter)
{
  egg_icon_chooser_remove_custom_filter (get_delegate (chooser), filter);
}

static GSList *
delegate_list_filters (EggIconChooser *chooser)
{
  return egg_icon_chooser_list_custom_filters (get_delegate (chooser));
}


/* ****************** *
 *  Delegate Signals  *
 * ****************** */

static void
delegate_notify (GObject    *receiver,
		 GParamSpec *pspec,
		 gpointer    user_data)
{
  gpointer iface;

  iface = g_type_interface_peek (g_type_class_peek (G_OBJECT_TYPE (receiver)),
				 EGG_TYPE_ICON_CHOOSER);
  if (g_object_interface_find_property (iface, pspec->name))
    g_object_notify (user_data, pspec->name);
}

static void
delegate_icon_selection_changed (EggIconChooser *receiver,
				 EggIconChooser *delegate)
{
  _egg_icon_chooser_icon_selection_changed (delegate);
}

static void
delegate_icon_activated (EggIconChooser *receiver,
			 EggIconChooser *delegate)
{
  _egg_icon_chooser_icon_activated (delegate);
}

static void
delegate_file_selection_changed (EggIconChooser *receiver,
				 EggIconChooser *delegate)
{
  _egg_icon_chooser_file_selection_changed (delegate);
}

static void
delegate_file_activated (EggIconChooser *receiver,
			 EggIconChooser *delegate)
{
  _egg_icon_chooser_file_activated (delegate);
}


GQuark
_egg_icon_chooser_get_delegate_quark (void)
{
  static GQuark quark = 0;

  if (G_UNLIKELY (quark == 0))
    quark = g_quark_from_static_string ("egg-icon-chooser-delegate");

  return quark;
}
