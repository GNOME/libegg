/* eggrecentchooserutils.h - Private utility functions for implementing a
 *                           EggRecentChooser interface
 *
 * Copyright (C) 2005 Emmanuele Bassi
 *
 * All rights reserved
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
 *
 * Based on gtkfilechooserutils.c:
 *	Copyright (C) 2003 Red Hat, Inc.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "eggrecentchooserutils.h"

/* Methods */
static void      delegate_set_sort_func              (EggRecentChooser  *chooser,
						      EggRecentSortFunc  sort_func,
						      gpointer           sort_data,
						      GDestroyNotify     data_destroy);
static void      delegate_add_filter                 (EggRecentChooser  *chooser,
						      EggRecentFilter   *filter);
static void      delegate_remove_filter              (EggRecentChooser  *chooser,
						      EggRecentFilter   *filter);
static GSList   *delegate_list_filters               (EggRecentChooser  *chooser);
static gboolean  delegate_select_uri                 (EggRecentChooser  *chooser,
						      const gchar       *uri,
						      GError           **error);
static void      delegate_unselect_uri               (EggRecentChooser  *chooser,
						      const gchar       *uri);
static GList    *delegate_get_items                  (EggRecentChooser  *chooser);
static EggRecentManager *delegate_get_recent_manager (EggRecentChooser  *chooser);
static void      delegate_select_all                 (EggRecentChooser  *chooser);
static void      delegate_unselect_all               (EggRecentChooser  *chooser);
static gboolean  delegate_set_current_uri            (EggRecentChooser  *chooser,
						      const gchar       *uri,
						      GError           **error);
static gchar *   delegate_get_current_uri            (EggRecentChooser  *chooser);

/* Signals */
static void      delegate_notify            (GObject          *object,
					     GParamSpec       *pspec,
					     gpointer          user_data);
static void      delegate_selection_changed (EggRecentChooser *receiver,
					     gpointer          user_data);
static void      delegate_item_activated    (EggRecentChooser *receiver,
					     gpointer          user_data);

/**
 * _egg_recent_chooser_install_properties:
 * @klass: the class structure for a type deriving from #GObject
 *
 * Installs the necessary properties for a class implementing
 * #EggRecentChooser. A #GtkParamSpecOverride property is installed
 * for each property, using the values from the #GtkRecentChooserProp
 * enumeration. The caller must make sure itself that the enumeration
 * values don't collide with some other property values they
 * are using.
 */
void
_egg_recent_chooser_install_properties (GObjectClass *klass)
{
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_RECENT_MANAGER,
  				    "recent-manager");  				    
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_SHOW_PRIVATE,
  				    "show-private");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_SHOW_NUMBERS,
  				    "show-numbers");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_SHOW_TIPS,
  				    "show-tips");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_SHOW_ICONS,
  				    "show-icons");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_SHOW_NOT_FOUND,
  				    "show-not-found");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_SELECT_MULTIPLE,
  				    "select-multiple");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_LIMIT,
  				    "limit");
  g_object_class_override_property (klass,
		  		    EGG_RECENT_CHOOSER_PROP_LOCAL_ONLY,
				    "local-only");
  g_object_class_override_property (klass,
		  		    EGG_RECENT_CHOOSER_PROP_SORT_TYPE,
				    "sort-type");
  g_object_class_override_property (klass,
  				    EGG_RECENT_CHOOSER_PROP_FILTER,
  				    "filter");
}

/**
 * _egg_recent_chooser_delegate_iface_init:
 * @iface: a #EggRecentChooserIface
 *
 * An interface-initialization function for use in cases where
 * an object is simply delegating the methods, signals of
 * the #EggRecentChooser interface to another object.
 * _egg_recent_chooser_set_delegate() must be called on each
 * instance of the object so that the delegate object can
 * be found.
 */
void
_egg_recent_chooser_delegate_iface_init (EggRecentChooserIface *iface)
{
  iface->set_current_uri = delegate_set_current_uri;
  iface->get_current_uri = delegate_get_current_uri;
  iface->select_uri = delegate_select_uri;
  iface->unselect_uri = delegate_unselect_uri;
  iface->select_all = delegate_select_all;
  iface->unselect_all = delegate_unselect_all;
  iface->get_items = delegate_get_items;
  iface->get_recent_manager = delegate_get_recent_manager;
  iface->set_sort_func = delegate_set_sort_func;
  iface->add_filter = delegate_add_filter;
  iface->remove_filter = delegate_remove_filter;
  iface->list_filters = delegate_list_filters;
}

/**
 * _egg_recent_chooser_set_delegate:
 * @receiver: a #GObject implementing #EggRecentChooser
 * @delegate: another #GObject implementing #EggRecentChooser
 *
 * Establishes that calls on @receiver for #EggRecentChooser
 * methods should be delegated to @delegate, and that
 * #EggRecentChooser signals emitted on @delegate should be
 * forwarded to @receiver. Must be used in conjunction with
 * _egg_recent_chooser_delegate_iface_init().
 */
void
_egg_recent_chooser_set_delegate (EggRecentChooser *receiver,
				  EggRecentChooser *delegate)
{
  g_return_if_fail (EGG_IS_RECENT_CHOOSER (receiver));
  g_return_if_fail (EGG_IS_RECENT_CHOOSER (delegate));
  
  g_object_set_data (G_OBJECT (receiver),
  		    "egg-recent-chooser-delegate", delegate);
  
  g_signal_connect (delegate, "notify",
  		    G_CALLBACK (delegate_notify), receiver);
  g_signal_connect (delegate, "selection-changed",
  		    G_CALLBACK (delegate_selection_changed), receiver);
  g_signal_connect (delegate, "item-activated",
  		    G_CALLBACK (delegate_item_activated), receiver);
}

GQuark
_egg_recent_chooser_delegate_get_quark (void)
{
  static GQuark quark = 0;
  
  if (G_UNLIKELY (quark == 0))
    quark = g_quark_from_static_string ("egg-recent-chooser-delegate");
  
  return quark;
}

static EggRecentChooser *
get_delegate (EggRecentChooser *receiver)
{
  return g_object_get_qdata (G_OBJECT (receiver),
  			     EGG_RECENT_CHOOSER_DELEGATE_QUARK);
}

static void
delegate_set_sort_func (EggRecentChooser  *chooser,
			EggRecentSortFunc  sort_func,
			gpointer           sort_data,
			GDestroyNotify     data_destroy)
{
  egg_recent_chooser_set_sort_func (get_delegate (chooser),
		  		    sort_func,
				    sort_data,
				    data_destroy);
}

static void
delegate_add_filter (EggRecentChooser *chooser,
		     EggRecentFilter  *filter)
{
  egg_recent_chooser_add_filter (get_delegate (chooser), filter);
}

static void
delegate_remove_filter (EggRecentChooser *chooser,
			EggRecentFilter  *filter)
{
  egg_recent_chooser_remove_filter (get_delegate (chooser), filter);
}

static GSList *
delegate_list_filters (EggRecentChooser *chooser)
{
  return egg_recent_chooser_list_filters (get_delegate (chooser));
}

static gboolean
delegate_select_uri (EggRecentChooser  *chooser,
		     const gchar       *uri,
		     GError           **error)
{
  return egg_recent_chooser_select_uri (get_delegate (chooser), uri, error);
}

static void
delegate_unselect_uri (EggRecentChooser *chooser,
		       const gchar      *uri)
{
  return egg_recent_chooser_unselect_uri (get_delegate (chooser), uri);
}

static GList *
delegate_get_items (EggRecentChooser *chooser)
{
  return egg_recent_chooser_get_items (get_delegate (chooser));
}

static EggRecentManager *
delegate_get_recent_manager (EggRecentChooser *chooser)
{
  return _egg_recent_chooser_get_recent_manager (get_delegate (chooser));
}

static void
delegate_select_all (EggRecentChooser *chooser)
{
  egg_recent_chooser_select_all (get_delegate (chooser));
}

static void
delegate_unselect_all (EggRecentChooser *chooser)
{
  egg_recent_chooser_unselect_all (get_delegate (chooser));
}

static gboolean
delegate_set_current_uri (EggRecentChooser  *chooser,
			  const gchar       *uri,
			  GError           **error)
{
  return egg_recent_chooser_set_current_uri (get_delegate (chooser), uri, error);
}

static gchar *
delegate_get_current_uri (EggRecentChooser *chooser)
{
  return egg_recent_chooser_get_current_uri (get_delegate (chooser));
}

static void
delegate_notify (GObject    *object,
		 GParamSpec *pspec,
		 gpointer    user_data)
{
  gpointer iface;

  iface = g_type_interface_peek (g_type_class_peek (G_OBJECT_TYPE (object)),
				 egg_recent_chooser_get_type ());
  if (g_object_interface_find_property (iface, pspec->name))
    g_object_notify (user_data, pspec->name);
}

static void
delegate_selection_changed (EggRecentChooser *receiver,
			    gpointer          user_data)
{
  g_signal_emit_by_name (user_data, "selection-changed");
}

static void
delegate_item_activated (EggRecentChooser *receiver,
			 gpointer          user_data)
{
  g_signal_emit_by_name (user_data, "item-activated");
}
