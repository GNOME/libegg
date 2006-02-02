/* GTK - The GIMP Toolkit
 * eggrecentchoosermenu.c - Recently used items menu widget
 * Copyright (C) 2005, Emmanuele Bassi
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
#include "config.h"
#endif

#include <string.h>

#include <glib/gi18n.h>

#include <gtk/gtkstock.h>
#include <gtk/gtkicontheme.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtksettings.h>
#include <gtk/gtkmenushell.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkseparatormenuitem.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkimage.h>
#include <gtk/gtktooltips.h>

#include "eggrecentmanager.h"
#include "eggrecentfilter.h"
#include "eggrecentchooser.h"
#include "eggrecentchooserutils.h"
#include "eggrecentchooserprivate.h"
#include "eggrecentchoosermenu.h"
#include "eggrecenttypebuiltins.h"

struct _EggRecentChooserMenuPrivate
{
  /* the recent manager object */
  EggRecentManager *manager;
  
  /* size of the icons of the menu items */  
  gint icon_size;

  /* RecentChooser properties */
  gint limit;  
  guint show_private : 1;
  guint show_not_found : 1;
  guint show_tips : 1;
  guint show_numbers : 1;
  guint show_icons : 1;
  guint local_only : 1;
  
  EggRecentSortType sort_type;
  EggRecentSortFunc sort_func;
  gpointer sort_data;
  GDestroyNotify sort_data_destroy;
  
  GSList *filters;
  EggRecentFilter *current_filter;
  
  gulong manager_changed_id;

  /* tooltips for our bookmark items*/
  GtkTooltips *tooltips;
};


#define FALLBACK_ICON_SIZE 	32
#define FALLBACK_ITEM_LIMIT 	10

#define EGG_RECENT_CHOOSER_MENU_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), EGG_TYPE_RECENT_CHOOSER_MENU, EggRecentChooserMenuPrivate))

static void     egg_recent_chooser_menu_finalize    (GObject                   *object);
static GObject *egg_recent_chooser_menu_constructor (GType                      type,
						     guint                      n_construct_properties,
						     GObjectConstructParam     *construct_params);

static void egg_recent_chooser_iface_init      (EggRecentChooserIface     *iface);

static void egg_recent_chooser_menu_set_property (GObject      *object,
						  guint         prop_id,
						  const GValue *value,
						  GParamSpec   *pspec);
static void egg_recent_chooser_menu_get_property (GObject      *object,
						  guint         prop_id,
						  GValue       *value,
						  GParamSpec   *pspec);

static gboolean          egg_recent_chooser_menu_set_current_uri    (EggRecentChooser  *chooser,
							             const gchar       *uri,
							             GError           **error);
static gchar *           egg_recent_chooser_menu_get_current_uri    (EggRecentChooser  *chooser);
static gboolean          egg_recent_chooser_menu_select_uri         (EggRecentChooser  *chooser,
								     const gchar       *uri,
								     GError           **error);
static void              egg_recent_chooser_menu_unselect_uri       (EggRecentChooser  *chooser,
								     const gchar       *uri);
static void              egg_recent_chooser_menu_select_all         (EggRecentChooser  *chooser);
static void              egg_recent_chooser_menu_unselect_all       (EggRecentChooser  *chooser);
static GList *           egg_recent_chooser_menu_get_items          (EggRecentChooser  *chooser);
static EggRecentManager *egg_recent_chooser_menu_get_recent_manager (EggRecentChooser  *chooser);
static void              egg_recent_chooser_menu_set_sort_func      (EggRecentChooser  *chooser,
								     EggRecentSortFunc  sort_func,
								     gpointer           sort_data,
								     GDestroyNotify     data_destroy);
static void              egg_recent_chooser_menu_add_filter         (EggRecentChooser  *chooser,
								     EggRecentFilter   *filter);
static void              egg_recent_chooser_menu_remove_filter      (EggRecentChooser  *chooser,
								     EggRecentFilter   *filter);
static GSList *          egg_recent_chooser_menu_list_filters       (EggRecentChooser  *chooser);
static void              egg_recent_chooser_menu_set_current_filter (EggRecentChooserMenu *menu,
								     EggRecentFilter      *filter);

static void              egg_recent_chooser_menu_map (GtkWidget *widget);
static void              egg_recent_chooser_menu_set_show_tips      (EggRecentChooserMenu *menu,
								     gboolean              show_tips);

static void     set_recent_manager (EggRecentChooserMenu *menu,
				    EggRecentManager     *manager);

static void     chooser_set_sort_type (EggRecentChooserMenu *menu,
				       EggRecentSortType     sort_type);

static gint     get_icon_size_for_widget (GtkWidget *widget);

static void     item_activate_cb   (GtkWidget        *widget,
			            gpointer          user_data);
static void     manager_changed_cb (EggRecentManager *manager,
				    gpointer          user_data);

G_DEFINE_TYPE_WITH_CODE (EggRecentChooserMenu,
			 egg_recent_chooser_menu,
			 GTK_TYPE_MENU,
			 G_IMPLEMENT_INTERFACE (EGG_TYPE_RECENT_CHOOSER,
				 		egg_recent_chooser_iface_init));


static void
egg_recent_chooser_iface_init (EggRecentChooserIface *iface)
{
  iface->set_current_uri = egg_recent_chooser_menu_set_current_uri;
  iface->get_current_uri = egg_recent_chooser_menu_get_current_uri;
  iface->select_uri = egg_recent_chooser_menu_select_uri;
  iface->unselect_uri = egg_recent_chooser_menu_unselect_uri;
  iface->select_all = egg_recent_chooser_menu_select_all;
  iface->unselect_all = egg_recent_chooser_menu_unselect_all;
  iface->get_items = egg_recent_chooser_menu_get_items;
  iface->get_recent_manager = egg_recent_chooser_menu_get_recent_manager;
  iface->set_sort_func = egg_recent_chooser_menu_set_sort_func;
  iface->add_filter = egg_recent_chooser_menu_add_filter;
  iface->remove_filter = egg_recent_chooser_menu_remove_filter;
  iface->list_filters = egg_recent_chooser_menu_list_filters;
}

static void
egg_recent_chooser_menu_class_init (EggRecentChooserMenuClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  gobject_class->constructor = egg_recent_chooser_menu_constructor;
  gobject_class->finalize = egg_recent_chooser_menu_finalize;
  gobject_class->set_property = egg_recent_chooser_menu_set_property;
  gobject_class->get_property = egg_recent_chooser_menu_get_property;
  
  widget_class->map = egg_recent_chooser_menu_map;
  
  _egg_recent_chooser_install_properties (gobject_class);
  
  g_type_class_add_private (klass, sizeof (EggRecentChooserMenuPrivate));
}

static void
egg_recent_chooser_menu_init (EggRecentChooserMenu *menu)
{
  EggRecentChooserMenuPrivate *priv;
  
  priv = EGG_RECENT_CHOOSER_MENU_GET_PRIVATE (menu);
  
  menu->priv = priv;
  
  priv->show_numbers = FALSE;
  priv->show_tips = FALSE;
  priv->show_not_found = FALSE;
  priv->show_private = FALSE;
  priv->local_only = TRUE;
  
  priv->limit = FALLBACK_ITEM_LIMIT;

  priv->sort_type = EGG_RECENT_SORT_NONE;
  
  priv->icon_size = FALLBACK_ICON_SIZE;
  
  priv->current_filter = NULL;
    
  priv->tooltips = gtk_tooltips_new ();
  g_object_ref (G_OBJECT (priv->tooltips));
  gtk_object_sink (GTK_OBJECT (priv->tooltips));
}

static void
egg_recent_chooser_menu_finalize (GObject *object)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (object);
  EggRecentChooserMenuPrivate *priv = menu->priv;
  
  g_signal_handler_disconnect (priv->manager, priv->manager_changed_id);
  priv->manager_changed_id = 0;
  g_object_unref (priv->manager);
  
  if (priv->sort_data_destroy)
    {
      priv->sort_data_destroy (priv->sort_data);
      
      priv->sort_data_destroy = NULL;
      priv->sort_data = NULL;
      priv->sort_func = NULL;
    }
  
  if (priv->tooltips)
    g_object_unref (priv->tooltips);
  
  if (priv->current_filter)
    g_object_unref (priv->current_filter);
  
  G_OBJECT_CLASS (egg_recent_chooser_menu_parent_class)->finalize;
}

static GObject *
egg_recent_chooser_menu_constructor (GType                  type,
				     guint                  n_construct_properties,
				     GObjectConstructParam *construct_params)
{
  EggRecentChooserMenu *menu;
  GObject *object;
  
  object = G_OBJECT_CLASS (egg_recent_chooser_menu_parent_class)->constructor (type,
		  							       n_construct_properties,
									       construct_params);
  menu = EGG_RECENT_CHOOSER_MENU (object);
  
  g_assert (menu->priv->manager);
  
  return object;
}

static void
egg_recent_chooser_menu_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (object);
  
  switch (prop_id)
    {
    case EGG_RECENT_CHOOSER_PROP_RECENT_MANAGER:
      set_recent_manager (menu, g_value_get_object (value));
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_PRIVATE:
      menu->priv->show_private = g_value_get_boolean (value);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_NOT_FOUND:
      menu->priv->show_not_found = g_value_get_boolean (value);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_TIPS:
      egg_recent_chooser_menu_set_show_tips (menu, g_value_get_boolean (value));
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_NUMBERS:
      menu->priv->show_numbers = g_value_get_boolean (value);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_ICONS:
      menu->priv->show_icons = g_value_get_boolean (value);
      break;
    case EGG_RECENT_CHOOSER_PROP_SELECT_MULTIPLE:
      g_warning ("%s: RecentChoosers of type `%s' do not support "
                 "selecting multiple items.",
                 G_STRFUNC,
                 G_OBJECT_TYPE_NAME (object));
      break;
    case EGG_RECENT_CHOOSER_PROP_LOCAL_ONLY:
      menu->priv->local_only = g_value_get_boolean (value);
      break;
    case EGG_RECENT_CHOOSER_PROP_LIMIT:
      menu->priv->limit = g_value_get_int (value);
      break;
    case EGG_RECENT_CHOOSER_PROP_SORT_TYPE:
      chooser_set_sort_type (menu, g_value_get_enum (value));
      break;
    case EGG_RECENT_CHOOSER_PROP_FILTER:
      egg_recent_chooser_menu_set_current_filter (menu, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_recent_chooser_menu_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (object);
  
  switch (prop_id)
    {
    case EGG_RECENT_CHOOSER_PROP_SHOW_TIPS:
      g_value_set_boolean (value, menu->priv->show_tips);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_NUMBERS:
      g_value_set_boolean (value, menu->priv->show_numbers);
      break;
    case EGG_RECENT_CHOOSER_PROP_LIMIT:
      g_value_set_int (value, menu->priv->limit);
      break;
    case EGG_RECENT_CHOOSER_PROP_LOCAL_ONLY:
      g_value_set_boolean (value, menu->priv->local_only);
    case EGG_RECENT_CHOOSER_PROP_SORT_TYPE:
      g_value_set_enum (value, menu->priv->sort_type);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_PRIVATE:
      g_value_set_boolean (value, menu->priv->show_private);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_NOT_FOUND:
      g_value_set_boolean (value, menu->priv->show_not_found);
      break;
    case EGG_RECENT_CHOOSER_PROP_SHOW_ICONS:
      g_value_set_boolean (value, menu->priv->show_icons);
      break;
    case EGG_RECENT_CHOOSER_PROP_SELECT_MULTIPLE:
      g_warning ("%s: Recent Choosers of type `%s' do not support "
                 "selecting multiple items.",
                 G_STRFUNC,
                 G_OBJECT_TYPE_NAME (object));
      break;
    case EGG_RECENT_CHOOSER_PROP_FILTER:
      g_value_set_object (value, menu->priv->current_filter);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
egg_recent_chooser_menu_set_current_uri (EggRecentChooser  *chooser,
					 const gchar       *uri,
					 GError           **error)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (chooser);
  GList *children, *l;
  GtkWidget *menu_item = NULL;
  gboolean found = FALSE;
  
  children = gtk_container_get_children (GTK_CONTAINER (menu));
  for (l = children; l != NULL; l = l->next)
    {
      EggRecentInfo *info;
      
      menu_item = GTK_WIDGET (l->data);
      
      info = g_object_get_data (G_OBJECT (menu_item), "egg-recent-info");
      if (!info)
        continue;
      
      if (0 == strcmp (uri, egg_recent_info_get_uri (info)))
        found = TRUE;
    }

  g_list_free (children);
  
  if (!found)  
    {
      g_set_error (error, EGG_RECENT_CHOOSER_ERROR,
      		   EGG_RECENT_CHOOSER_ERROR_NOT_FOUND,
      		   _("No recently used resource found with URI `%s'"),
      		   uri);
      return FALSE;
    }
  else
    {
      gtk_menu_shell_activate_item (GTK_MENU_SHELL (menu), menu_item, TRUE);
      
      return TRUE;
    }
}

static gchar *
egg_recent_chooser_menu_get_current_uri (EggRecentChooser  *chooser)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (chooser);
  GtkWidget *menu_item;
  EggRecentInfo *info;
  
  menu_item = gtk_menu_get_active (GTK_MENU (menu));
  if (!menu_item)
    return NULL;
  
  info = g_object_get_data (G_OBJECT (menu_item), "egg-recent-info");
  if (!info)
    return NULL;
  
  return g_strdup (egg_recent_info_get_uri (info));
}

static gboolean
egg_recent_chooser_menu_select_uri (EggRecentChooser  *chooser,
				    const gchar       *uri,
				    GError           **error)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (chooser);
  GList *children, *l;
  GtkWidget *menu_item = NULL;
  gboolean found = FALSE;
  
  children = gtk_container_get_children (GTK_CONTAINER (menu));
  for (l = children; l != NULL; l = l->next)
    {
      EggRecentInfo *info;
      
      menu_item = GTK_WIDGET (l->data);
      
      info = g_object_get_data (G_OBJECT (menu_item), "egg-recent-info");
      if (!info)
        continue;
      
      if (0 == strcmp (uri, egg_recent_info_get_uri (info)))
        found = TRUE;
    }

  g_list_free (children);
  
  if (!found)  
    {
      g_set_error (error, EGG_RECENT_CHOOSER_ERROR,
      		   EGG_RECENT_CHOOSER_ERROR_NOT_FOUND,
      		   _("No recently used resource found with URI `%s'"),
      		   uri);
      return FALSE;
    }
  else
    {
      gtk_menu_shell_select_item (GTK_MENU_SHELL (menu), menu_item);

      return TRUE;
    }
}

static void
egg_recent_chooser_menu_unselect_uri (EggRecentChooser *chooser,
				       const gchar     *uri)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (chooser);
  
  gtk_menu_shell_deselect (GTK_MENU_SHELL (menu));
}

static void
egg_recent_chooser_menu_select_all (EggRecentChooser *chooser)
{
  g_warning (_("This function is not implemented for "
               "widgets of class '%s'"),
             g_type_name (G_OBJECT_TYPE (chooser)));
}

static void
egg_recent_chooser_menu_unselect_all (EggRecentChooser *chooser)
{
  g_warning (_("This function is not implemented for "
               "widgets of class '%s'."),
             g_type_name (G_OBJECT_TYPE (chooser)));
}

static void
egg_recent_chooser_menu_set_sort_func (EggRecentChooser  *chooser,
				       EggRecentSortFunc  sort_func,
				       gpointer           sort_data,
				       GDestroyNotify     data_destroy)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (chooser);
  EggRecentChooserMenuPrivate *priv = menu->priv;
  
  if (priv->sort_data_destroy)
    {
      priv->sort_data_destroy (priv->sort_data);
      
      priv->sort_func = NULL;
      priv->sort_data = NULL;
      priv->sort_data_destroy = NULL;
    }
  
  if (sort_func)
    {
      priv->sort_func = sort_func;
      priv->sort_data = sort_data;
      priv->sort_data_destroy = data_destroy;
    }
}

static gint
sort_recent_items_mru (EggRecentInfo *a,
		       EggRecentInfo *b,
		       gpointer       unused)
{
  g_assert (a != NULL && b != NULL);
  
  return (egg_recent_info_get_modified (a) < egg_recent_info_get_modified (b));
}

static gint
sort_recent_items_lru (EggRecentInfo *a,
		       EggRecentInfo *b,
		       gpointer       unused)
{
  g_assert (a != NULL && b != NULL);
  
  return (egg_recent_info_get_modified (a) > egg_recent_info_get_modified (b));
}

/* our proxy sorting function */
static gint
sort_recent_items_proxy (gpointer *a,
			 gpointer *b,
			 gpointer  user_data)
{
  EggRecentInfo *info_a = (EggRecentInfo *) a;
  EggRecentInfo *info_b = (EggRecentInfo *) b;
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (user_data);

  if (menu->priv->sort_func)
    return (* menu->priv->sort_func) (info_a,
    				      info_b,
    				      menu->priv->sort_data);
  
  /* fallback */
  return 0;
}

static void
chooser_set_sort_type (EggRecentChooserMenu *menu,
		       EggRecentSortType     sort_type)
{
  if (menu->priv->sort_type == sort_type)
    return;

  menu->priv->sort_type = sort_type;
}


static GList *
egg_recent_chooser_menu_get_items (EggRecentChooser *chooser)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (chooser);
  EggRecentChooserMenuPrivate *priv;
  gint limit;
  EggRecentSortType sort_type;
  GList *items;
  GCompareDataFunc compare_func;
  gint length;
  
  priv = menu->priv;
  
  if (!priv->manager)
    return NULL;
  
  limit = egg_recent_chooser_get_limit (chooser);
  sort_type = egg_recent_chooser_get_sort_type (chooser);

  switch (sort_type)
    {
    case EGG_RECENT_SORT_NONE:
      compare_func = NULL;
      break;
    case EGG_RECENT_SORT_MRU:
      compare_func = (GCompareDataFunc) sort_recent_items_mru;
      break;
    case EGG_RECENT_SORT_LRU:
      compare_func = (GCompareDataFunc) sort_recent_items_lru;
      break;
    case EGG_RECENT_SORT_CUSTOM:
      compare_func = (GCompareDataFunc) sort_recent_items_proxy;
      break;
    default:
      g_assert_not_reached ();
      break;
    }
  
  items = egg_recent_manager_get_items (priv->manager);
  if (!items)
    return NULL;
  
  if (compare_func)  
    items = g_list_sort_with_data (items, compare_func, menu);
  
  length = g_list_length (items);
  if ((limit != -1) && (length > limit))
    {
      GList *clamp, *l;
      
      clamp = g_list_nth (items, limit - 1);

      l = clamp->next;
      clamp->next = NULL;      
      
      g_list_foreach (l, (GFunc) egg_recent_info_unref, NULL);
      g_list_free (l);
    }
  
  return items;
}

static EggRecentManager *
egg_recent_chooser_menu_get_recent_manager (EggRecentChooser *chooser)
{
  EggRecentChooserMenuPrivate *priv;
 
  priv = EGG_RECENT_CHOOSER_MENU (chooser)->priv;
  
  return priv->manager;
}

static void
egg_recent_chooser_menu_add_filter (EggRecentChooser *chooser,
				    EggRecentFilter  *filter)
{
  g_warning (_("This function is not implemented for "
               "widgets of class '%s'"),
             g_type_name (G_OBJECT_TYPE (chooser)));
}

static void
egg_recent_chooser_menu_remove_filter (EggRecentChooser *chooser,
				       EggRecentFilter  *filter)
{
  g_warning (_("This function is not implemented for "
               "widgets of class '%s'"),
             g_type_name (G_OBJECT_TYPE (chooser)));
}

static GSList *
egg_recent_chooser_menu_list_filters (EggRecentChooser  *chooser)
{
  g_warning (_("This function is not implemented for "
               "widgets of class '%s'"),
             g_type_name (G_OBJECT_TYPE (chooser)));

  return NULL;
}

static void
egg_recent_chooser_menu_set_current_filter (EggRecentChooserMenu *menu,
					    EggRecentFilter      *filter)
{
  EggRecentChooserMenuPrivate *priv;

  priv = menu->priv;
  
  if (priv->current_filter)
    g_object_unref (G_OBJECT (priv->current_filter));
  
  priv->current_filter = filter;
  g_object_ref (priv->current_filter);
  gtk_object_sink (GTK_OBJECT (priv->current_filter));
  
  g_object_notify (G_OBJECT (menu), "filter");
}

static gboolean
get_is_recent_filtered (EggRecentChooserMenu *menu,
			EggRecentInfo        *info)
{
  EggRecentChooserMenuPrivate *priv;
  EggRecentFilter *current_filter;
  EggRecentFilterInfo filter_info;
  EggRecentFilterFlags needed;
  gboolean retval;

  g_assert (info != NULL);

  priv = menu->priv;
  
  if (!priv->current_filter)
    return FALSE;
  
  current_filter = priv->current_filter;
  needed = egg_recent_filter_get_needed (current_filter);
  
  filter_info.contains = EGG_RECENT_FILTER_URI | EGG_RECENT_FILTER_MIME_TYPE;
  
  filter_info.uri = egg_recent_info_get_uri (info);
  filter_info.mime_type = egg_recent_info_get_mime_type (info);
  
  if (needed & EGG_RECENT_FILTER_DISPLAY_NAME)
    {
      filter_info.display_name = egg_recent_info_get_display_name (info);
      filter_info.contains |= EGG_RECENT_FILTER_DISPLAY_NAME;
    }
  else
    filter_info.uri = NULL;
  
  if (needed & EGG_RECENT_FILTER_APPLICATION)
    {
      filter_info.applications = (const gchar **) egg_recent_info_get_applications (info, NULL);
      filter_info.contains |= EGG_RECENT_FILTER_APPLICATION;
    }
  else
    filter_info.applications = NULL;

  if (needed & EGG_RECENT_FILTER_GROUP)
    {
      filter_info.groups = (const gchar **) egg_recent_info_get_groups (info, NULL);
      filter_info.contains |= EGG_RECENT_FILTER_GROUP;
    }
  else
    filter_info.groups = NULL;
  
  if (needed & EGG_RECENT_FILTER_AGE)
    {
      filter_info.age = egg_recent_info_get_age (info);
      filter_info.contains |= EGG_RECENT_FILTER_AGE;
    }
  else
    filter_info.age = -1;
  
  retval = egg_recent_filter_filter (current_filter, &filter_info);
  
  /* this we own */
  if (filter_info.applications)
    g_strfreev ((gchar **) filter_info.applications);
  
  return !retval;
}

/* taken from libeel/eel-strings.c */
static gchar *
escape_underscores (const gchar *string)
{
  gint underscores;
  const gchar *p;
  gchar *q;
  gchar *escaped;

  if (!string)
    return NULL;
	
  underscores = 0;
  for (p = string; *p != '\0'; p++)
    underscores += (*p == '_');

  if (underscores == 0)
    return g_strdup (string);

  escaped = g_new (char, strlen (string) + underscores + 1);
  for (p = string, q = escaped; *p != '\0'; p++, q++)
    {
      /* Add an extra underscore. */
      if (*p == '_')
        *q++ = '_';
      
      *q = *p;
    }
  
  *q = '\0';
	
  return escaped;
}

static void
egg_recent_chooser_menu_add_tip (EggRecentChooserMenu *menu,
				 EggRecentInfo        *info,
				 GtkWidget            *item)
{
  EggRecentChooserMenuPrivate *priv;
  gchar *path, *tip_text;

  g_assert (info != NULL);
  g_assert (item != NULL);

  priv = menu->priv;
  
  if (!priv->tooltips)
    return;
  
  path = egg_recent_info_get_uri_display (info);
  
  tip_text = g_strdup_printf (_("Open '%s'"), path);
 
  gtk_tooltips_set_tip (priv->tooltips,
  			item,
  			tip_text,
  			NULL);

  g_free (path);  
  g_free (tip_text);
}

static GtkWidget *
egg_recent_chooser_menu_create_item (EggRecentChooserMenu *menu,
				     EggRecentInfo        *info,
				     gint                  count)
{
  EggRecentChooserMenuPrivate *priv;
  gchar *label;
  GtkWidget *item, *image;
  GdkPixbuf *icon;

  g_assert (info != NULL);

  priv = menu->priv;

  if (priv->show_numbers)
    {
      gchar *name, *escaped;
      
      name = g_strdup (egg_recent_info_get_display_name (info));
      if (!name)
        name = g_strdup (_("Unknown item"));
      
      escaped = escape_underscores (name);
      
      /* avoid clashing mnemonics */
      if (count <= 10)
        label = g_strdup_printf ("_%d. %s", count, escaped);
      else
        label = g_strdup_printf ("%d. %s", count, escaped);
      
      item = gtk_image_menu_item_new_with_mnemonic (label);
      
      g_free (escaped);
      g_free (name);
    }
  else
    {
      label = g_strdup (egg_recent_info_get_display_name (info));
      item = gtk_image_menu_item_new_with_label (label);
    }
  
  if (priv->show_icons)
    {
      icon = egg_recent_info_get_icon (info, priv->icon_size);
        
      image = gtk_image_new_from_pixbuf (icon);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
    }
  
  if (!egg_recent_info_exists (info))
    {
      gtk_widget_set_sensitive (item, FALSE);
      
      goto out;
    }
  
  g_signal_connect (item, "activate",
  		    G_CALLBACK (item_activate_cb),
  		    menu);

out:
  g_free (label);

  return item;
}

/* removes the items we own from the menu */
static void
egg_recent_chooser_menu_dispose_items (EggRecentChooserMenu *menu)
{
  GList *children, *l;
 
  children = gtk_container_get_children (GTK_CONTAINER (menu));
  for (l = children; l != NULL; l = l->next)
    {
      GtkWidget *menu_item = GTK_WIDGET (l->data);
      gint mark = 0;
      
      /* check for our mark, in order to remove just the items we own */
      mark = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menu_item),
                                                 "egg-recent-menu-mark"));
      if (mark == 1)
        {
          EggRecentInfo *info;
          
          /* destroy the attached RecentInfo struct, if found */
          info = g_object_get_data (G_OBJECT (menu_item), "egg-recent-info");
          if (info)
            g_object_set_data_full (G_OBJECT (menu_item), "egg-recent-info",
            			    NULL, NULL);
          
          /* and finally remove the item from the menu */
          gtk_container_remove (GTK_CONTAINER (menu), menu_item);
        }
    }

  g_list_free (children);
}

/* GtkWidget::map method override
 *
 * We override this method in order to populate the menu with our
 * menu items linked to the recently used resources.
 */
static void
egg_recent_chooser_menu_map (GtkWidget *widget)
{
  EggRecentChooserMenu *menu = EGG_RECENT_CHOOSER_MENU (widget);
  EggRecentChooserMenuPrivate *priv = menu->priv;
  GList *items, *l;
  gint count;
  gboolean has_items = FALSE;
  
  if (GTK_WIDGET_CLASS (egg_recent_chooser_menu_parent_class)->map)
    GTK_WIDGET_CLASS (egg_recent_chooser_menu_parent_class)->map (widget);
  
  priv->icon_size = get_icon_size_for_widget (widget);
  
  /* dispose our menu items first */
  egg_recent_chooser_menu_dispose_items (menu);
 
  items = egg_recent_chooser_get_items (EGG_RECENT_CHOOSER (menu));
  
  count = g_list_length (items);
  for (l = items; l != NULL; l = l->prev)
    {
      EggRecentInfo *info = (EggRecentInfo *) l->data;
      GtkWidget *item;

      g_assert (info != NULL);
      
      /* skip non-local items on request */
      if (priv->local_only && !egg_recent_info_is_local (info))
        continue;
      
      /* skip private items on request */
      if (!priv->show_private && egg_recent_info_get_private_hint (info))
        continue;
      
      /* skip non-existing items on request */
      if (!priv->show_not_found && !egg_recent_info_exists (info))
        continue;

      /* filter items based on the currently set filter object */
      if (get_is_recent_filtered (menu, info))
        continue;
 
      item = egg_recent_chooser_menu_create_item (menu, info, count);
      if (!item)
        continue;
      
      egg_recent_chooser_menu_add_tip (menu, info, item);
      
      /* FIXME
       *
       * We should really place our items taking into account user
       * defined menu items; this would also remove the need of
       * reverting the scan order.
       */
      gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
      gtk_widget_show (item);
      
      /* mark the menu item as one of our own */
      g_object_set_data (G_OBJECT (item), "egg-recent-menu-mark",
      			 GINT_TO_POINTER (1));
      
      /* attach the RecentInfo object to the menu item, and own a reference
       * to it, so that it will be destroyed with the menu item when it's
       * not needed anymore.
       */
      g_object_set_data_full (G_OBJECT (item), "egg-recent-info",
      			      egg_recent_info_ref (info),
      			      (GDestroyNotify) egg_recent_info_unref);
      
      /* we have at least one item */
      if (!has_items)
        has_items = TRUE;
    }
  
  /* now, the RecentInfo objects are bound to the lifetime of the menu */
  if (items)
    {
      g_list_foreach (items,
  		      (GFunc) egg_recent_info_unref,
  		      NULL);
      g_list_free (items);
    }

  /* no recently used resources were found, or they were filtered out, so
   * we build an item stating that no recently used resources were found
   * (as night follows the day...).
   */
  if (!has_items)
    {
      GtkWidget *item;
      
      item = gtk_menu_item_new_with_label ("No items found");
      gtk_widget_set_sensitive (item, FALSE);
      
      /* we also mark this item, so that it gets removed when rebuilding
       * the menu on the next map event
       */
      g_object_set_data (G_OBJECT (item), "egg-recent-menu-mark",
      			 GINT_TO_POINTER (1));
      
      gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), item);
      gtk_widget_show (item);
    }
}

/* bounce activate signal from the recent menu item widget 
 * to the recent menu widget
 */
static void
item_activate_cb (GtkWidget *widget,
		  gpointer   user_data)
{
  EggRecentChooser *chooser = EGG_RECENT_CHOOSER (user_data);
  
  g_signal_emit_by_name (chooser, "item-activated");
}

/* we force a redraw if the manager changes when we are showing */
static void
manager_changed_cb (EggRecentManager *manager,
		    gpointer          user_data)
{
  gtk_widget_queue_draw (GTK_WIDGET (user_data));
}

static void
set_recent_manager (EggRecentChooserMenu *menu,
		    EggRecentManager     *manager)
{
  if (menu->priv->manager)
    g_signal_handler_disconnect (menu, menu->priv->manager_changed_id);
  
  menu->priv->manager = NULL;
  
  if (manager)
    {
      menu->priv->manager = manager;
      
      g_object_ref (menu->priv->manager);
    }
#if 0
  else
    {
      GtkSettings *settings = gtk_settings_get_default ();
      EggRecentManager *default_manager = NULL;
      
      g_object_get_data (settings, "egg-recent-manager", &default_manager, NULL);
      
      if (default_manager)
        {
          menu->priv->manager = default_manager;
        }
    }
#endif
  
  /* create a new recent manager object */
  if (!menu->priv->manager)
    menu->priv->manager = egg_recent_manager_new ();
  
  if (menu->priv->manager)
    menu->priv->manager_changed_id = g_signal_connect (menu->priv->manager, "changed",
      						       G_CALLBACK (manager_changed_cb),
      						       menu);
}

static gint
get_icon_size_for_widget (GtkWidget *widget)
{
  GtkSettings *settings;
  gint width, height;

  if (gtk_widget_has_screen (widget))
    settings = gtk_settings_get_for_screen (gtk_widget_get_screen (widget));
  else
    settings = gtk_settings_get_default ();

  if (gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_MENU,
                                         &width, &height))
    return MAX (width, height);

  return FALLBACK_ICON_SIZE;
}

static void
egg_recent_chooser_menu_set_show_tips (EggRecentChooserMenu *menu,
				       gboolean              show_tips)
{
  if (menu->priv->show_tips == show_tips)
    return;
  
  g_assert (menu->priv->tooltips != NULL);
  
  if (show_tips)
    gtk_tooltips_enable (menu->priv->tooltips);
  else
    gtk_tooltips_disable (menu->priv->tooltips);
  
  menu->priv->show_tips = show_tips;
}

/*
 * Public API
 */

/**
 * egg_recent_chooser_menu_new:
 *
 * Creates a new #EggRecentChooserMenu widget.
 *
 * This kind of widget shows the list of recently used resources as
 * a menu, each item as a menu item.  Each item inside the menu might
 * have an icon, representing its MIME type, and a number, for mnemonic
 * access.
 *
 * This widget implements the #EggRecentChooser interface.
 *
 * This widget creates its own #EggRecentManager object.  See the
 * egg_recent_chooser_menu_new_for_manager() function to know how to create
 * a #EggRecentChooserMenu widget bound to another #EggRecentManager object.
 *
 * Return value: a new #EggRecentChooserMenu
 *
 * Since: 2.10
 */
GtkWidget *
egg_recent_chooser_menu_new (void)
{
  return g_object_new (EGG_TYPE_RECENT_CHOOSER_MENU,
  		       "recent-manager", NULL,
  		       NULL);
}

/**
 * egg_recent_chooser_menu_new_for_manager:
 * @manager: a #EggRecentManager
 *
 * Creates a new #EggRecentChooserMenu widget using @manager as the underlying
 * recently used resources manager.
 *
 * This is useful if you have implemented your own recent manager, or if you
 * have a customized instance of a #EggRecentManager object (e.g. with your
 * own sorting and/or filtering functions), or if you wish to share a common
 * RecentManager object among multiple RecentChooser widgets.
 *
 * Return value: a new #EggRecentChooserMenu, bound to @manager.
 *
 * Since: 2.10
 */
GtkWidget *
egg_recent_chooser_menu_new_for_manager (EggRecentManager *manager)
{
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (manager), NULL);
  
  return g_object_new (EGG_TYPE_RECENT_CHOOSER_MENU,
  		       "recent-manager", manager,
  		       NULL);
}
