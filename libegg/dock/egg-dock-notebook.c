/*
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2003 Biswapesh Chattopadhyay <biswapesh_chatterjee@tcscal.co.in>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <util/eggintl.h>
#include <gtk/gtknotebook.h>
#include <util/egg-macros.h>

#include "egg-dock-notebook.h"
#include "egg-dock-tablabel.h"

struct _EggDockNotebookPrivate {
	gpointer unused;
};

/* Private prototypes */

static void  egg_dock_notebook_class_init    (EggDockNotebookClass *klass);
static void  egg_dock_notebook_instance_init (EggDockNotebook      *notebook);
static void  egg_dock_notebook_set_property  (GObject              *object,
                                              guint                 prop_id,
                                              const GValue         *value,
                                              GParamSpec           *pspec);
static void  egg_dock_notebook_get_property  (GObject              *object,
                                              guint                 prop_id,
                                              GValue               *value,
                                              GParamSpec           *pspec);

static void  egg_dock_notebook_destroy       (GtkObject    *object);

static void  egg_dock_notebook_add           (GtkContainer *container,
					      GtkWidget    *widget);
static void  egg_dock_notebook_forall        (GtkContainer *container,
					      gboolean      include_internals,
					      GtkCallback   callback,
					      gpointer      callback_data);
static GType egg_dock_notebook_child_type    (GtkContainer *container);

static void  egg_dock_notebook_dock          (EggDockObject    *object,
                                              EggDockObject    *requestor,
                                              EggDockPlacement  position,
                                              GValue           *other_data);

static void  egg_dock_notebook_switch_page_cb  (GtkNotebook     *nb,
                                                GtkNotebookPage *page,
                                                gint             page_num,
                                                gpointer         data);

static void  egg_dock_notebook_set_orientation (EggDockItem     *item,
                                                GtkOrientation   orientation);
					       
static gboolean egg_dock_notebook_child_placement (EggDockObject    *object,
                                                   EggDockObject    *child,
                                                   EggDockPlacement *placement);

static void     egg_dock_notebook_present         (EggDockObject    *object,
                                                   EggDockObject    *child);

static gboolean egg_dock_notebook_reorder         (EggDockObject    *object,
                                                   EggDockObject    *requestor,
                                                   EggDockPlacement  new_position,
                                                   GValue           *other_data);


/* Class variables and definitions */

enum {
    PROP_0,
    PROP_PAGE
};


/* ----- Private functions ----- */

EGG_CLASS_BOILERPLATE (EggDockNotebook, egg_dock_notebook, EggDockItem, EGG_TYPE_DOCK_ITEM) ;

static void
egg_dock_notebook_class_init (EggDockNotebookClass *klass)
{
    GObjectClass       *g_object_class;
    GtkObjectClass     *gtk_object_class;
    GtkWidgetClass     *widget_class;
    GtkContainerClass  *container_class;
    EggDockObjectClass *object_class;
    EggDockItemClass   *item_class;

    g_object_class = G_OBJECT_CLASS (klass);
    gtk_object_class = GTK_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);
    container_class = GTK_CONTAINER_CLASS (klass);
    object_class = EGG_DOCK_OBJECT_CLASS (klass);
    item_class = EGG_DOCK_ITEM_CLASS (klass);

    g_object_class->set_property = egg_dock_notebook_set_property;
    g_object_class->get_property = egg_dock_notebook_get_property;
    
    gtk_object_class->destroy = egg_dock_notebook_destroy;

    container_class->add = egg_dock_notebook_add;
    container_class->forall = egg_dock_notebook_forall;
    container_class->child_type = egg_dock_notebook_child_type;
    
    object_class->is_compound = TRUE;
    object_class->dock = egg_dock_notebook_dock;
    object_class->child_placement = egg_dock_notebook_child_placement;
    object_class->present = egg_dock_notebook_present;
    object_class->reorder = egg_dock_notebook_reorder;
    
    item_class->has_grip = TRUE;
    item_class->set_orientation = egg_dock_notebook_set_orientation;    
    
    g_object_class_install_property (
        g_object_class, PROP_PAGE,
        g_param_spec_int ("page", _("Page"),
                          _("The index of the current page"),
                          0, G_MAXINT,
                          0,
                          G_PARAM_READWRITE |
                          EGG_DOCK_PARAM_EXPORT | EGG_DOCK_PARAM_AFTER));
}

static void 
egg_dock_notebook_notify_cb (GObject    *g_object,
                             GParamSpec *pspec,
                             gpointer    user_data) 
{
    g_return_if_fail (user_data != NULL && EGG_IS_DOCK_NOTEBOOK (user_data));

    /* chain the notify signal */
    g_object_notify (G_OBJECT (user_data), pspec->name);
}

static gboolean 
egg_dock_notebook_button_cb (GtkWidget      *widget,
                             GdkEventButton *event,
                             gpointer        user_data)
{
    if (event->type == GDK_BUTTON_PRESS)
        EGG_DOCK_ITEM_SET_FLAGS (user_data, EGG_DOCK_USER_ACTION);
    else
        EGG_DOCK_ITEM_UNSET_FLAGS (user_data, EGG_DOCK_USER_ACTION);

    return FALSE;
}
    
static void
egg_dock_notebook_instance_init (EggDockNotebook *notebook)
{
    EggDockItem *item;

    item = EGG_DOCK_ITEM (notebook);

    /* create the container notebook */
    item->child = gtk_notebook_new ();
    gtk_widget_set_parent (item->child, GTK_WIDGET (notebook));
    if (item->orientation == GTK_ORIENTATION_HORIZONTAL)
        gtk_notebook_set_tab_pos (GTK_NOTEBOOK (item->child), GTK_POS_TOP);
    else
      {
	if (gtk_widget_get_direction (GTK_WIDGET (item->child)) == GTK_TEXT_DIR_RTL)
	  
	  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (item->child), GTK_POS_RIGHT);
	else
	  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (item->child), GTK_POS_LEFT);
      }
    g_signal_connect (item->child, "switch_page",
                      (GCallback) egg_dock_notebook_switch_page_cb, (gpointer) item);
    g_signal_connect (item->child, "notify::page",
                      (GCallback) egg_dock_notebook_notify_cb, (gpointer) item);
    g_signal_connect (item->child, "button-press-event",
                      (GCallback) egg_dock_notebook_button_cb, (gpointer) item);
    g_signal_connect (item->child, "button-release-event",
                      (GCallback) egg_dock_notebook_button_cb, (gpointer) item);
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (item->child), TRUE);
    gtk_widget_show (item->child);
}

static void 
egg_dock_notebook_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    EggDockItem *item = EGG_DOCK_ITEM (object);

    switch (prop_id) {
        case PROP_PAGE:
            if (item->child && GTK_IS_NOTEBOOK (item->child)) {
                gtk_notebook_set_current_page (GTK_NOTEBOOK (item->child),
                                               g_value_get_int (value));
            }
            
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void 
egg_dock_notebook_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    EggDockItem *item = EGG_DOCK_ITEM (object);

    switch (prop_id) {
        case PROP_PAGE:
            if (item->child && GTK_IS_NOTEBOOK (item->child)) {
                g_value_set_int (value, gtk_notebook_get_current_page
                                 (GTK_NOTEBOOK (item->child)));
            }
            
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}


static void
egg_dock_notebook_destroy (GtkObject *object)
{
    EggDockItem *item = EGG_DOCK_ITEM (object);

    /* we need to call the virtual first, since in EggDockDestroy our
       children dock objects are detached */
    EGG_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));

    /* after that we can remove the GtkNotebook */
    if (item->child) {
        gtk_widget_unparent (item->child);
        item->child = NULL;
    };
}

static void
egg_dock_notebook_switch_page_cb (GtkNotebook     *nb,
                                  GtkNotebookPage *page,
                                  gint             page_num,
                                  gpointer         data)
{
    EggDockNotebook *notebook;
    GtkWidget       *tablabel;

    notebook = EGG_DOCK_NOTEBOOK (data);

    /* deactivate old tablabel */
    if (nb->cur_page) {
        tablabel = gtk_notebook_get_tab_label (
            nb, gtk_notebook_get_nth_page (
                nb, gtk_notebook_get_current_page (nb)));
        if (tablabel && EGG_IS_DOCK_TABLABEL (tablabel))
            egg_dock_tablabel_deactivate (EGG_DOCK_TABLABEL (tablabel));
    };

    /* activate new label */
    tablabel = gtk_notebook_get_tab_label (
        nb, gtk_notebook_get_nth_page (nb, page_num));
    if (tablabel && EGG_IS_DOCK_TABLABEL (tablabel))
        egg_dock_tablabel_activate (EGG_DOCK_TABLABEL (tablabel));

    if (EGG_DOCK_ITEM_USER_ACTION (notebook) &&
        EGG_DOCK_OBJECT (notebook)->master)
        g_signal_emit_by_name (EGG_DOCK_OBJECT (notebook)->master,
                               "layout_changed");
}

static void
egg_dock_notebook_add (GtkContainer *container,
		       GtkWidget    *widget)
{
    g_return_if_fail (container != NULL && widget != NULL);
    g_return_if_fail (EGG_IS_DOCK_NOTEBOOK (container));
    g_return_if_fail (EGG_IS_DOCK_ITEM (widget));

    egg_dock_object_dock (EGG_DOCK_OBJECT (container),
                          EGG_DOCK_OBJECT (widget),
                          EGG_DOCK_CENTER,
                          NULL);
}

static void
egg_dock_notebook_forall (GtkContainer *container,
			  gboolean      include_internals,
			  GtkCallback   callback,
			  gpointer      callback_data)
{
    EggDockItem *item;

    g_return_if_fail (container != NULL);
    g_return_if_fail (EGG_IS_DOCK_NOTEBOOK (container));
    g_return_if_fail (callback != NULL);

    if (include_internals) {
        /* use EggDockItem's forall */
        EGG_CALL_PARENT (GTK_CONTAINER_CLASS, forall, 
                           (container, include_internals, callback, callback_data));
    }
    else {
        item = EGG_DOCK_ITEM (container);
        if (item->child)
            gtk_container_foreach (GTK_CONTAINER (item->child), callback, callback_data);
    }
}

static GType
egg_dock_notebook_child_type (GtkContainer *container)
{
    return EGG_TYPE_DOCK_ITEM;
}
    
static void
egg_dock_notebook_dock_child (EggDockObject *requestor,
                              gpointer       user_data)
{
    struct {
        EggDockObject    *object;
        EggDockPlacement  position;
        GValue           *other_data;
    } *data = user_data;

    egg_dock_object_dock (data->object, requestor, data->position, data->other_data);
}

static void
egg_dock_notebook_dock (EggDockObject    *object,
                        EggDockObject    *requestor,
                        EggDockPlacement  position,
                        GValue           *other_data)
{
    g_return_if_fail (EGG_IS_DOCK_NOTEBOOK (object));
    g_return_if_fail (EGG_IS_DOCK_ITEM (requestor));

    /* we only add support for EGG_DOCK_CENTER docking strategy here... for the rest
       use our parent class' method */
    if (position == EGG_DOCK_CENTER) {
        /* we can only dock simple (not compound) items */
        if (egg_dock_object_is_compound (requestor)) {
            struct {
                EggDockObject    *object;
                EggDockPlacement  position;
                GValue           *other_data;
            } data;

            egg_dock_object_freeze (requestor);
            
            data.object = object;
            data.position = position;
            data.other_data = other_data;
             
            gtk_container_foreach (GTK_CONTAINER (requestor),
                                   (GtkCallback) egg_dock_notebook_dock_child, &data);

            egg_dock_object_thaw (requestor);
        }
        else {
            EggDockItem *item = EGG_DOCK_ITEM (object);
            EggDockItem *requestor_item = EGG_DOCK_ITEM (requestor);
            GtkWidget   *label;
            gint         position = -1;
            
            label = egg_dock_item_get_tablabel (requestor_item);
            if (!label) {
                label = egg_dock_tablabel_new (requestor_item);
                egg_dock_item_set_tablabel (requestor_item, label);
            }
            if (EGG_IS_DOCK_TABLABEL (label)) {
                egg_dock_tablabel_deactivate (EGG_DOCK_TABLABEL (label));
                /* hide the item grip, as we will use the tablabel's */
                egg_dock_item_hide_grip (requestor_item);
            }

            if (other_data && G_VALUE_HOLDS (other_data, G_TYPE_INT))
                position = g_value_get_int (other_data);
            
            gtk_notebook_insert_page (GTK_NOTEBOOK (item->child), 
                                      GTK_WIDGET (requestor), label,
                                      position);
            EGG_DOCK_OBJECT_SET_FLAGS (requestor, EGG_DOCK_ATTACHED);
        }
    }
    else
        EGG_CALL_PARENT (EGG_DOCK_OBJECT_CLASS, dock,
                           (object, requestor, position, other_data));
}

static void
egg_dock_notebook_set_orientation (EggDockItem    *item,
				   GtkOrientation  orientation)
{
    if (item->child && GTK_IS_NOTEBOOK (item->child)) {
        if (orientation == GTK_ORIENTATION_HORIZONTAL)
            gtk_notebook_set_tab_pos (GTK_NOTEBOOK (item->child), GTK_POS_TOP);
        else 
	  {
	    if (gtk_widget_get_direction (GTK_WIDGET (item->child)) == GTK_TEXT_DIR_RTL)
	      
	      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (item->child), GTK_POS_RIGHT);
	    else
	      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (item->child), GTK_POS_LEFT);
	  }
    }

    EGG_CALL_PARENT (EGG_DOCK_ITEM_CLASS, set_orientation, (item, orientation));
}

static gboolean 
egg_dock_notebook_child_placement (EggDockObject    *object,
                                   EggDockObject    *child,
                                   EggDockPlacement *placement)
{
    EggDockItem      *item = EGG_DOCK_ITEM (object);
    EggDockPlacement  pos = EGG_DOCK_NONE;
    
    if (item->child) {
        GList *children, *l;

        children = gtk_container_get_children (GTK_CONTAINER (item->child));
        for (l = children; l; l = l->next) {
            if (l->data == (gpointer) child) {
                pos = EGG_DOCK_CENTER;
                break;
            }
        }
        g_list_free (children);
    }

    if (pos != EGG_DOCK_NONE) {
        if (placement)
            *placement = pos;
        return TRUE;
    }
    else
        return FALSE;
}

static void
egg_dock_notebook_present (EggDockObject *object,
                           EggDockObject *child)
{
    EggDockItem *item = EGG_DOCK_ITEM (object);
    int i;
    
    i = gtk_notebook_page_num (GTK_NOTEBOOK (item->child),
                               GTK_WIDGET (child));
    if (i >= 0)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (item->child), i);

    EGG_CALL_PARENT (EGG_DOCK_OBJECT_CLASS, present, (object, child));
}

static gboolean 
egg_dock_notebook_reorder (EggDockObject    *object,
                           EggDockObject    *requestor,
                           EggDockPlacement  new_position,
                           GValue           *other_data)
{
    EggDockItem *item = EGG_DOCK_ITEM (object);
    gint         current_position, new_pos = -1;
    gboolean     handled = FALSE;
    
    if (item->child && new_position == EGG_DOCK_CENTER) {
        current_position = gtk_notebook_page_num (GTK_NOTEBOOK (item->child),
                                                  GTK_WIDGET (requestor));
        if (current_position >= 0) {
            handled = TRUE;
    
            if (other_data && G_VALUE_HOLDS (other_data, G_TYPE_INT))
                new_pos = g_value_get_int (other_data);
            
            gtk_notebook_reorder_child (GTK_NOTEBOOK (item->child), 
                                        GTK_WIDGET (requestor),
                                        new_pos);
        }
    }
    return handled;
}

/* ----- Public interface ----- */

GtkWidget *
egg_dock_notebook_new (void)
{
    EggDockNotebook *notebook;

    notebook = EGG_DOCK_NOTEBOOK (g_object_new (EGG_TYPE_DOCK_NOTEBOOK, NULL));
    EGG_DOCK_OBJECT_UNSET_FLAGS (notebook, EGG_DOCK_AUTOMATIC);
    
    return GTK_WIDGET (notebook);
}
