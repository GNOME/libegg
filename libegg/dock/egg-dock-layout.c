/*
 *
 * This file is part of the GNOME Devtools Libraries.
 *
 * Copyright (C) 2002 Gustavo Gir�ldez <gustavo.giraldez@gmx.net>
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
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <util/egg-macros.h>

#include "egg-dock-layout.h"
#include "egg-dock-placeholder.h"


/* ----- Private variables ----- */

enum {
    PROP_0,
    PROP_MASTER,
    PROP_DIRTY
};

#define ROOT_ELEMENT         "dock-layout"
#define DEFAULT_LAYOUT       "__default__"
#define LAYOUT_ELEMENT_NAME  "layout"
#define NAME_ATTRIBUTE_NAME  "name"

#define LAYOUT_GLADE_FILE    "layout.glade"

enum {
    COLUMN_NAME,
    COLUMN_SHOW,
    COLUMN_LOCKED,
    COLUMN_ITEM
};

struct _EggDockLayoutPrivate {
    xmlDocPtr         doc;

    /* layout list models */
    GtkListStore     *items_model;
    GtkListStore     *layouts_model;

    /* idle control */
    gboolean          idle_save_pending;
};

typedef struct _EggDockLayoutUIData EggDockLayoutUIData;

struct _EggDockLayoutUIData {
    EggDockLayout    *layout;
    
    GtkWidget        *layout_entry;
    GtkWidget        *locked_check;
    GtkTreeSelection *selection;
};


/* ----- Private prototypes ----- */

static void     egg_dock_layout_class_init      (EggDockLayoutClass *klass);

static void     egg_dock_layout_instance_init   (EggDockLayout      *layout);

static void     egg_dock_layout_set_property    (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);

static void     egg_dock_layout_get_property    (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);

static void     egg_dock_layout_dispose         (GObject            *object);

static void     egg_dock_layout_build_doc       (EggDockLayout      *layout);

static xmlNodePtr egg_dock_layout_find_layout   (EggDockLayout      *layout, 
                                                 const gchar        *name);

static void     egg_dock_layout_build_models    (EggDockLayout      *layout);


/* ----- Private implementation ----- */

EGG_CLASS_BOILERPLATE (EggDockLayout, egg_dock_layout, GObject, G_TYPE_OBJECT);

static void
egg_dock_layout_class_init (EggDockLayoutClass *klass)
{
    GObjectClass *g_object_class = (GObjectClass *) klass;

    g_object_class->set_property = egg_dock_layout_set_property;
    g_object_class->get_property = egg_dock_layout_get_property;
    g_object_class->dispose = egg_dock_layout_dispose;

    g_object_class_install_property (
        g_object_class, PROP_MASTER,
        g_param_spec_object ("master", _("Master"),
                             _("EggDockMaster object which the layout object "
                               "is attached to"),
                             EGG_TYPE_DOCK_MASTER, 
                             G_PARAM_READWRITE));

    g_object_class_install_property (
        g_object_class, PROP_DIRTY,
        g_param_spec_boolean ("dirty", _("Dirty"),
                              _("True if the layouts have changed and need to be "
                                "saved to a file"),
                              FALSE,
                              G_PARAM_READABLE));
}

static void
egg_dock_layout_instance_init (EggDockLayout *layout)
{
    layout->master = NULL;
    layout->dirty = FALSE;
    layout->_priv = g_new0 (EggDockLayoutPrivate, 1);
    layout->_priv->idle_save_pending = FALSE;

    egg_dock_layout_build_models (layout);
}

static void
egg_dock_layout_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
    EggDockLayout *layout = EGG_DOCK_LAYOUT (object);

    switch (prop_id) {
        case PROP_MASTER:
            egg_dock_layout_attach (layout, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    };
}

static void
egg_dock_layout_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
    EggDockLayout *layout = EGG_DOCK_LAYOUT (object);

    switch (prop_id) {
        case PROP_MASTER:
            g_value_set_object (value, layout->master);
            break;
        case PROP_DIRTY:
            g_value_set_boolean (value, layout->dirty);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    };
}

static void
egg_dock_layout_dispose (GObject *object)
{
    EggDockLayout *layout;

    g_return_if_fail (object != NULL);
    g_return_if_fail (EGG_IS_DOCK_LAYOUT (object));

    layout = EGG_DOCK_LAYOUT (object);
    
    if (layout->master)
        egg_dock_layout_attach (layout, NULL);

    if (layout->_priv) {
        if (layout->_priv->idle_save_pending) {
            layout->_priv->idle_save_pending = FALSE;
            g_idle_remove_by_data (layout);
        }
        
        if (layout->_priv->doc) {
            xmlFreeDoc (layout->_priv->doc);
            layout->_priv->doc = NULL;
        }

        if (layout->_priv->items_model) {
            g_object_unref (layout->_priv->items_model);
            g_object_unref (layout->_priv->layouts_model);
            layout->_priv->items_model = NULL;
            layout->_priv->layouts_model = NULL;
        }

        g_free (layout->_priv);
        layout->_priv = NULL;
    }
}

static void
egg_dock_layout_build_doc (EggDockLayout *layout)
{
    g_return_if_fail (layout->_priv->doc == NULL);

    layout->_priv->doc = xmlNewDoc ("1.0");
    layout->_priv->doc->children = xmlNewDocNode (layout->_priv->doc, NULL, 
                                                  ROOT_ELEMENT, NULL);
}

static xmlNodePtr
egg_dock_layout_find_layout (EggDockLayout *layout, 
                             const gchar   *name)
{
    xmlNodePtr node;
    gboolean   found = FALSE;

    g_return_val_if_fail (layout != NULL, NULL);
    
    if (!layout->_priv->doc)
        return NULL;

    /* get document root */
    node = layout->_priv->doc->children;
    for (node = node->children; node; node = node->next) {
        gchar *layout_name;
        
        if (strcmp (node->name, LAYOUT_ELEMENT_NAME))
            /* skip non-layout element */
            continue;

        /* we want the first layout */
        if (!name)
            break;

        layout_name = xmlGetProp (node, NAME_ATTRIBUTE_NAME);
        if (!strcmp (name, layout_name))
            found = TRUE;
        xmlFree (layout_name);

        if (found)
            break;
    };
    return node;
}

static void
egg_dock_layout_build_models (EggDockLayout *layout)
{
    if (!layout->_priv->items_model) {
        layout->_priv->items_model = gtk_list_store_new (4, 
                                                         G_TYPE_STRING, 
                                                         G_TYPE_BOOLEAN,
                                                         G_TYPE_BOOLEAN,
                                                         G_TYPE_POINTER);
        gtk_tree_sortable_set_sort_column_id (
            GTK_TREE_SORTABLE (layout->_priv->items_model), 
            COLUMN_NAME, GTK_SORT_ASCENDING);
    }

    if (!layout->_priv->layouts_model) {
        layout->_priv->layouts_model = gtk_list_store_new (1, G_TYPE_STRING);
        gtk_tree_sortable_set_sort_column_id (
            GTK_TREE_SORTABLE (layout->_priv->layouts_model),
            COLUMN_NAME, GTK_SORT_ASCENDING);
    }
}

static void
build_list (EggDockObject *object, GList **list)
{
    /* add only items, not toplevels */
    if (EGG_IS_DOCK_ITEM (object))
        *list = g_list_prepend (*list, object);
}

static void
update_items_model (EggDockLayout *layout)
{
    GList *items, *l;
    GtkTreeIter iter;
    GtkListStore *store;
    gchar *long_name;
    gboolean locked;
    
    g_return_if_fail (layout != NULL);
    g_return_if_fail (layout->_priv->items_model != NULL);

    if (!layout->master)
        return;
    
    /* build items list */
    items = NULL;
    egg_dock_master_foreach (layout->master, (GFunc) build_list, &items);

    /* walk the current model */
    store = layout->_priv->items_model;
    
    /* update items model data after a layout load */
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter)) {
        gboolean valid = TRUE;
        
        while (valid) {
            EggDockItem *item;
            
            gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                                COLUMN_ITEM, &item,
                                -1);
            if (item) {
                /* look for the object in the items list */
                for (l = items; l && l->data != item; l = l->next);

                if (l) {
                    /* found, update data */
                    g_object_get (item, 
                                  "long_name", &long_name, 
                                  "locked", &locked, 
                                  NULL);
                    gtk_list_store_set (store, &iter, 
                                        COLUMN_NAME, long_name,
                                        COLUMN_SHOW, EGG_DOCK_OBJECT_ATTACHED (item),
                                        COLUMN_LOCKED, locked,
                                        -1);
                    g_free (long_name);

                    /* remove the item from the linked list and keep on walking the model */
                    items = g_list_delete_link (items, l);
                    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);

                } else {
                    /* not found, which means the item has been removed */
                    valid = gtk_list_store_remove (store, &iter);
                    
                }

            } else {
                /* not a valid row */
                valid = gtk_list_store_remove (store, &iter);
            }
        }
    }

    /* add any remaining objects */
    for (l = items; l; l = l->next) {
        EggDockObject *object = l->data;
        
        g_object_get (object, 
                      "long_name", &long_name, 
                      "locked", &locked, 
                      NULL);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 
                            COLUMN_ITEM, object,
                            COLUMN_NAME, long_name,
                            COLUMN_SHOW, EGG_DOCK_OBJECT_ATTACHED (object),
                            COLUMN_LOCKED, locked,
                            -1);
        g_free (long_name);
    }
    
    g_list_free (items);
}

static void
update_layouts_model (EggDockLayout *layout)
{
    GList *items, *l;
    GtkTreeIter iter;
    
    g_return_if_fail (layout != NULL);
    g_return_if_fail (layout->_priv->layouts_model != NULL);

    /* build layouts list */
    gtk_list_store_clear (layout->_priv->layouts_model);
    items = egg_dock_layout_get_layouts (layout, FALSE);
    for (l = items; l; l = l->next) {
        gtk_list_store_append (layout->_priv->layouts_model, &iter);
        gtk_list_store_set (layout->_priv->layouts_model, &iter,
                            COLUMN_NAME, l->data,
                            -1);
        g_free (l->data);
    };
    g_list_free (items);
}


/* ------- UI functions & callbacks ------ */

static void 
save_layout_cb (GtkWidget *w,
                gpointer   data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) data;

    EggDockLayout *layout = ui_data->layout;
    gchar         *name;
    
    g_return_if_fail (layout != NULL);
    
    /* get the entry text and use it as a name to save the layout */
    name = g_strdup (gtk_entry_get_text (GTK_ENTRY (ui_data->layout_entry)));
    g_strstrip (name);
    if (strlen (name) > 0) {
        gboolean exists;

        exists = (egg_dock_layout_find_layout (layout, name) != NULL);
        if (!exists && strcmp (name, DEFAULT_LAYOUT)) {
            GtkTreeIter iter;

            /* add the name to the model */
            gtk_list_store_append (layout->_priv->layouts_model, &iter);
            gtk_list_store_set (layout->_priv->layouts_model, &iter,
                                COLUMN_NAME, name,
                                -1);
        };
        egg_dock_layout_save_layout (layout, name);

    } else {
        GtkWidget *error_dialog, *parent;
        parent = gtk_widget_get_toplevel (w);
        error_dialog = gtk_message_dialog_new (
            parent ? GTK_WINDOW (parent) : NULL,
            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            _("You must provide a name for the layout"));
        gtk_dialog_run (GTK_DIALOG (error_dialog));
        gtk_widget_destroy (error_dialog);
    };
    g_free (name);
}

static void 
load_layout_cb (GtkWidget *w,
                gpointer   data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) data;

    GtkTreeModel  *model;
    GtkTreeIter    iter;
    EggDockLayout *layout = ui_data->layout;
    gchar         *name;

    g_return_if_fail (layout != NULL);
    
    if (gtk_tree_selection_get_selected (ui_data->selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            COLUMN_NAME, &name,
                            -1);
        egg_dock_layout_load_layout (layout, name);
        g_free (name);
    }
}

static void
delete_layout_cb (GtkWidget *w, gpointer data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) data;

    GtkTreeModel  *model;
    GtkTreeIter    iter;
    EggDockLayout *layout = ui_data->layout;
    gchar         *name;

    g_return_if_fail (layout != NULL);
    
    if (gtk_tree_selection_get_selected (ui_data->selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            COLUMN_NAME, &name,
                            -1);
        egg_dock_layout_delete_layout (layout, name);
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
        g_free (name);
    };
}

static void
show_toggled_cb (GtkCellRendererToggle *renderer,
                 gchar                 *path_str,
                 gpointer               data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) data;

    EggDockLayout *layout = ui_data->layout;
    GtkTreeModel  *model;
    GtkTreeIter    iter;
    GtkTreePath   *path = gtk_tree_path_new_from_string (path_str);
    gboolean       value;
    EggDockItem   *item;

    g_return_if_fail (layout != NULL);
    
    model = GTK_TREE_MODEL (layout->_priv->items_model);
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, 
                        COLUMN_SHOW, &value, 
                        COLUMN_ITEM, &item, 
                        -1);

    value = !value;
    if (value)
        egg_dock_item_show_item (item);
    else
        egg_dock_item_hide_item (item);

    gtk_tree_path_free (path);
}

static void
selection_changed_cb (GtkTreeSelection *selection,
                      gpointer          data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) data;

    GtkTreeModel  *model;
    GtkTreeIter    iter;
    gchar         *name;

    /* set the entry widget to the selected layout name */
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            COLUMN_NAME, &name,
                            -1);
        gtk_entry_set_text (GTK_ENTRY (ui_data->layout_entry), name);
        g_free (name);
    };
}

static void
all_locked_toggled_cb (GtkWidget *widget,
                       gpointer   data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) data;
    EggDockMaster       *master;
    gboolean             locked;
    
    g_return_if_fail (ui_data->layout != NULL);
    master = ui_data->layout->master;
    g_return_if_fail (master != NULL);

    locked = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    g_object_set (master, "locked", locked ? 1 : 0, NULL);
}

static void
layout_ui_destroyed (GtkWidget *widget,
                     gpointer   user_data)
{
    EggDockLayoutUIData *ui_data;
    
    /* widget is the GtkContainer */
    ui_data = g_object_get_data (G_OBJECT (widget), "ui_data");
    if (ui_data) {
        if (ui_data->layout) {
            if (ui_data->layout->master)
                /* disconnet the notify handler */
                g_signal_handlers_disconnect_matched (ui_data->layout->master,
                                                      G_SIGNAL_MATCH_DATA,
                                                      0, 0, NULL, NULL,
                                                      ui_data);
            
            g_object_remove_weak_pointer (G_OBJECT (ui_data->layout),
                                          (gpointer *) &ui_data->layout);
            ui_data->layout = NULL;
        }
        g_object_set_data (G_OBJECT (widget), "ui_data", NULL);
        g_free (ui_data);
    }
}

static void
master_locked_notify_cb (EggDockMaster *master,
                         GParamSpec    *pspec,
                         gpointer       user_data)
{
    EggDockLayoutUIData *ui_data = (EggDockLayoutUIData *) user_data;
    gint locked;

    g_object_get (master, "locked", &locked, NULL);
    if (locked == -1) {
        gtk_toggle_button_set_inconsistent (
            GTK_TOGGLE_BUTTON (ui_data->locked_check), TRUE);
    }
    else {
        gtk_toggle_button_set_inconsistent (
            GTK_TOGGLE_BUTTON (ui_data->locked_check), FALSE);
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (ui_data->locked_check), (locked == 1));
    }
}

static GladeXML *
load_interface (const gchar *top_widget)
{
    GladeXML *gui;
    gchar    *gui_file;

    /* load ui */
    gui_file = g_build_filename (EGG_GLADEDIR, LAYOUT_GLADE_FILE, NULL);
    gui = glade_xml_new (gui_file, top_widget, GETTEXT_PACKAGE);
    g_free (gui_file);
    if (!gui) {
        /* FIXME: pop up an error dialog */
        g_warning (_("Could not load layout user interface file '%s'"), 
                   LAYOUT_GLADE_FILE);
        return NULL;
    };
    return gui;
}

static GtkWidget *
egg_dock_layout_construct_items_ui (EggDockLayout *layout)
{
    GladeXML            *gui;
    GtkWidget           *container;
    GtkWidget           *items_list;
    GtkCellRenderer     *renderer;
    GtkTreeViewColumn   *column;

    EggDockLayoutUIData *ui_data;
    
    /* load the interface if it wasn't provided */
    gui = load_interface ("items_vbox");
    
    if (!gui)
        return NULL;
    
    /* get the container */
    container = glade_xml_get_widget (gui, "items_vbox");

    ui_data = g_new0 (EggDockLayoutUIData, 1);
    ui_data->layout = layout;
    g_object_add_weak_pointer (G_OBJECT (layout),
                               (gpointer *) &ui_data->layout);
    g_object_set_data (G_OBJECT (container), "ui_data", ui_data);
    
    /* get ui widget references */
    ui_data->locked_check = glade_xml_get_widget (gui, "locked_check");
    items_list = glade_xml_get_widget (gui, "items_list");

    /* locked check connections */
    g_signal_connect (ui_data->locked_check, "toggled",
                      (GCallback) all_locked_toggled_cb, ui_data);
    if (layout->master) {
        g_signal_connect (layout->master, "notify::locked",
                          (GCallback) master_locked_notify_cb, ui_data);
        /* force update now */
        master_locked_notify_cb (layout->master, NULL, ui_data);
    }
    
    /* set models */
    gtk_tree_view_set_model (GTK_TREE_VIEW (items_list),
                             GTK_TREE_MODEL (layout->_priv->items_model));

    /* construct list views */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled", 
                      G_CALLBACK (show_toggled_cb), ui_data);
    column = gtk_tree_view_column_new_with_attributes (_("Visible"),
                                                       renderer,
                                                       "active", COLUMN_SHOW,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (items_list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Item"),
                                                       renderer,
                                                       "text", COLUMN_NAME,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (items_list), column);

    /* connect signals */
    g_signal_connect (container, "destroy", (GCallback) layout_ui_destroyed, NULL);

    g_object_unref (gui);

    return container;
}

static GtkWidget *
egg_dock_layout_construct_layouts_ui (EggDockLayout *layout)
{
    GladeXML            *gui;
    GtkWidget           *container;
    GtkWidget           *layouts_list;
    GtkCellRenderer     *renderer;
    GtkTreeViewColumn   *column;

    EggDockLayoutUIData *ui_data;
    
    /* load the interface if it wasn't provided */
    gui = load_interface ("layouts_vbox");
    
    if (!gui)
        return NULL;
    
    /* get the container */
    container = glade_xml_get_widget (gui, "layouts_vbox");

    ui_data = g_new0 (EggDockLayoutUIData, 1);
    ui_data->layout = layout;
    g_object_add_weak_pointer (G_OBJECT (layout),
                               (gpointer *) &ui_data->layout);
    g_object_set_data (G_OBJECT (container), "ui_data", ui_data);
    
    /* get ui widget references */
    ui_data->layout_entry = glade_xml_get_widget (gui, "newlayout_entry");
    layouts_list = glade_xml_get_widget (gui, "layouts_list");

    /* set models */
    gtk_tree_view_set_model (GTK_TREE_VIEW (layouts_list),
                             GTK_TREE_MODEL (layout->_priv->layouts_model));

    /* construct list views */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Name"),
                                                       renderer,
                                                       "text", COLUMN_NAME,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (layouts_list), column);

    /* connect signals */
    ui_data->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (layouts_list));
    g_signal_connect (ui_data->selection, "changed",
                      G_CALLBACK (selection_changed_cb), ui_data);

    glade_xml_signal_connect_data (gui, "on_newlayout_entry_activate",
                                   GTK_SIGNAL_FUNC (save_layout_cb), ui_data);
    glade_xml_signal_connect_data (gui, "on_save_button_clicked",
                                   GTK_SIGNAL_FUNC (save_layout_cb), ui_data);
    glade_xml_signal_connect_data (gui, "on_load_button_clicked",
                                   GTK_SIGNAL_FUNC (load_layout_cb), ui_data);
    glade_xml_signal_connect_data (gui, "on_delete_button_clicked",
                                   GTK_SIGNAL_FUNC (delete_layout_cb), ui_data);

    g_signal_connect (container, "destroy", (GCallback) layout_ui_destroyed, NULL);

    g_object_unref (gui);

    return container;
}

static GtkWidget *
egg_dock_layout_construct_ui (EggDockLayout *layout)
{
    GtkWidget *container, *child;
    
    container = gtk_notebook_new ();
    gtk_widget_show (container);
    
    child = egg_dock_layout_construct_items_ui (layout);
    if (child)
        gtk_notebook_append_page (GTK_NOTEBOOK (container),
                                  child,
                                  gtk_label_new (_("Dock items")));
    
    child = egg_dock_layout_construct_layouts_ui (layout);
    if (child)
        gtk_notebook_append_page (GTK_NOTEBOOK (container),
                                  child,
                                  gtk_label_new (_("Saved layouts")));

    gtk_notebook_set_current_page (GTK_NOTEBOOK (container), 0);
    
    return container;
}

/* ----- Save & Load layout functions --------- */

#define EGG_DOCK_PARAM_CONSTRUCTION(p) \
    (((p)->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) != 0)

static EggDockObject * 
egg_dock_layout_setup_object (EggDockMaster *master,
                              xmlNodePtr     node,
                              gint          *n_after_params,
                              GParameter   **after_params)
{
    EggDockObject *object = NULL;
    GType          object_type;
    xmlChar       *object_name;
    GObjectClass  *object_class = NULL;

    GParamSpec   **props;
    gint           n_props, i;
    GParameter    *params = NULL;
    gint           n_params = 0;
    GValue         serialized = { 0, };
    
    object_name = xmlGetProp (node, EGG_DOCK_NAME_PROPERTY);
    if (object_name && strlen (object_name) > 0) {
        /* the object must already be bound to the master */
        object = egg_dock_master_get_object (master, object_name);

        xmlFree (object_name);
        object_type = object ? G_TYPE_FROM_INSTANCE (object) : G_TYPE_NONE;
    }
    else {
        /* the object should be automatic, so create it by
           retrieving the object type from the dock registry */
        object_type = egg_dock_object_type_from_nick (node->name);
        if (object_type == G_TYPE_NONE) {
            g_warning (_("While loading layout: don't know how to create "
                         "a dock object whose nick is '%s'"), node->name);
        }
    }
    
    if (object_type == G_TYPE_NONE || !G_TYPE_IS_CLASSED (object_type))
        return NULL;

    object_class = g_type_class_ref (object_type);
    props = g_object_class_list_properties (object_class, &n_props);
        
    /* create parameter slots */
    /* extra parameter is the master */
    params = g_new0 (GParameter, n_props + 1);
    *after_params = g_new0 (GParameter, n_props);
    *n_after_params = 0;
    
    /* initialize value used for transformations */
    g_value_init (&serialized, EGG_TYPE_DOCK_PARAM);
    
    for (i = 0; i < n_props; i++) {
        xmlChar *xml_prop;

        /* process all exported properties, skip
           EGG_DOCK_NAME_PROPERTY, since named items should
           already by in the master */
        if (!(props [i]->flags & EGG_DOCK_PARAM_EXPORT) ||
            !strcmp (props [i]->name, EGG_DOCK_NAME_PROPERTY))
            continue;

        /* get the property from xml if there is one */
        xml_prop = xmlGetProp (node, props [i]->name);
        if (xml_prop) {
            g_value_set_static_string (&serialized, xml_prop);
            
            if (!EGG_DOCK_PARAM_CONSTRUCTION (props [i]) &&
                (props [i]->flags & EGG_DOCK_PARAM_AFTER)) {
                (*after_params) [*n_after_params].name = props [i]->name;
                g_value_init (&((* after_params) [*n_after_params].value),
                              props [i]->value_type);
                g_value_transform (&serialized,
                                   &((* after_params) [*n_after_params].value));
                (*n_after_params)++;
            }
            else if (!object || (!EGG_DOCK_PARAM_CONSTRUCTION (props [i]) && object)) {
                params [n_params].name = props [i]->name;
                g_value_init (&(params [n_params].value), props [i]->value_type);
                g_value_transform (&serialized, &(params [n_params].value));
                n_params++;
            }
            xmlFree (xml_prop);
        }
    }
    g_value_unset (&serialized);
    g_free (props);

    if (!object) {
        params [n_params].name = EGG_DOCK_MASTER_PROPERTY;
        g_value_init (&params [n_params].value, EGG_TYPE_DOCK_MASTER);
        g_value_set_object (&params [n_params].value, master);
        n_params++;
        
        /* construct the object if we have to */
        /* set the master, so toplevels are created correctly and
           other objects are bound */
        object = g_object_newv (object_type, n_params, params);
    }
    else {
        /* set the parameters to the existing object */
        for (i = 0; i < n_params; i++)
            g_object_set_property (G_OBJECT (object),
                                   params [i].name,
                                   &params [i].value);
    }

    /* free the parameters (names are static/const strings) */
    for (i = 0; i < n_params; i++)
        g_value_unset (&params [i].value);
    g_free (params);

    /* finally unref object class */
    g_type_class_unref (object_class);

    return object;
}

static void
egg_dock_layout_recursive_build (EggDockMaster *master,
                                 xmlNodePtr     parent_node,
                                 EggDockObject *parent)
{
    EggDockObject *object;
    xmlNodePtr     node;
    
    g_return_if_fail (master != NULL && parent_node != NULL);

    /* if parent is NULL we should build toplevels */
    for (node = parent_node->children; node; node = node->next) {
        GParameter *after_params = NULL;
        gint        n_after_params = 0, i;

        object = egg_dock_layout_setup_object (master, node,
                                               &n_after_params,
                                               &after_params);
        
        if (object) {
            egg_dock_object_freeze (object);

            /* recurse here to catch placeholders */
            egg_dock_layout_recursive_build (master, node, object);
            
            if (EGG_IS_DOCK_PLACEHOLDER (object))
                /* placeholders are later attached to the parent */
                egg_dock_object_detach (object, FALSE);
            
            /* apply "after" parameters */
            for (i = 0; i < n_after_params; i++) {
                g_object_set_property (G_OBJECT (object),
                                       after_params [i].name,
                                       &after_params [i].value);
                /* unset and free the value */
                g_value_unset (&after_params [i].value);
            }
            g_free (after_params);
            
            /* add the object to the parent */
            if (parent) {
                if (EGG_IS_DOCK_PLACEHOLDER (object))
                    egg_dock_placeholder_attach (EGG_DOCK_PLACEHOLDER (object),
                                                 parent);
                else if (egg_dock_object_is_compound (parent)) {
                    gtk_container_add (GTK_CONTAINER (parent), GTK_WIDGET (object));
                    if (GTK_WIDGET_VISIBLE (parent))
                        gtk_widget_show (GTK_WIDGET (object));
                }
            }
            else {
                EggDockObject *controller = egg_dock_master_get_controller (master);
                if (controller != object && GTK_WIDGET_VISIBLE (controller))
                    gtk_widget_show (GTK_WIDGET (object));
            }
                
            /* call reduce just in case any child is missing */
            if (egg_dock_object_is_compound (object))
                egg_dock_object_reduce (object);

            egg_dock_object_thaw (object);
        }
    }
}

static void
_egg_dock_layout_foreach_detach (EggDockObject *object)
{
    egg_dock_object_detach (object, TRUE);
}

static void
egg_dock_layout_foreach_toplevel_detach (EggDockObject *object)
{
    gtk_container_foreach (GTK_CONTAINER (object),
                           (GtkCallback) _egg_dock_layout_foreach_detach,
                           NULL);
}

static void
egg_dock_layout_load (EggDockMaster *master, xmlNodePtr node)
{
    g_return_if_fail (master != NULL && node != NULL);

    /* start by detaching all items from the toplevels */
    egg_dock_master_foreach_toplevel (master, TRUE,
                                      (GFunc) egg_dock_layout_foreach_toplevel_detach,
                                      NULL);
    
    egg_dock_layout_recursive_build (master, node, NULL);
}

static void 
egg_dock_layout_foreach_object_save (EggDockObject *object,
                                     gpointer       user_data)
{
    struct {
        xmlNodePtr  where;
        GHashTable *placeholders;
    } *info = user_data, info_child;

    xmlNodePtr   node;
    gint         n_props, i;
    GParamSpec **props;
    GValue       attr = { 0, };
    
    g_return_if_fail (object != NULL && EGG_IS_DOCK_OBJECT (object));
    g_return_if_fail (info->where != NULL);
    
    node = xmlNewChild (info->where,
                        NULL,               /* ns */
                        egg_dock_object_nick_from_type (G_TYPE_FROM_INSTANCE (object)),
                        NULL);              /* contents */

    /* get object exported attributes */
    props = g_object_class_list_properties (G_OBJECT_GET_CLASS (object),
                                            &n_props);
    g_value_init (&attr, EGG_TYPE_DOCK_PARAM);
    for (i = 0; i < n_props; i++) {
        GParamSpec *p = props [i];

        if (p->flags & EGG_DOCK_PARAM_EXPORT) {
            GValue v = { 0, };
            
            /* export this parameter */
            /* get the parameter value */
            g_value_init (&v, p->value_type);
            g_object_get_property (G_OBJECT (object),
                                   p->name,
                                   &v);

            /* only save the object "name" if it is set
               (i.e. don't save the empty string) */
            if (strcmp (p->name, EGG_DOCK_NAME_PROPERTY) ||
                g_value_get_string (&v)) {
                if (g_value_transform (&v, &attr))
                    xmlSetProp (node, p->name, g_value_get_string (&attr));
            }
            
            /* free the parameter value */
            g_value_unset (&v);
        }
    }
    g_value_unset (&attr);
    g_free (props);

    info_child = *info;
    info_child.where = node;

    /* save placeholders for the object */
    if (info->placeholders && !EGG_IS_DOCK_PLACEHOLDER (object)) {
        GList *lph = g_hash_table_lookup (info->placeholders, object);
        for (; lph; lph = lph->next)
            egg_dock_layout_foreach_object_save (EGG_DOCK_OBJECT (lph->data),
                                                 (gpointer) &info_child);
    }
    
    /* recurse the object if appropiate */
    if (egg_dock_object_is_compound (object)) {
        gtk_container_foreach (GTK_CONTAINER (object),
                               (GtkCallback) egg_dock_layout_foreach_object_save,
                               (gpointer) &info_child);
    }
}

static void
add_placeholder (EggDockObject *object,
                 GHashTable    *placeholders)
{
    if (EGG_IS_DOCK_PLACEHOLDER (object)) {
        EggDockObject *host;
        GList *l;
        
        g_object_get (object, "host", &host, NULL);
        if (host) {
            l = g_hash_table_lookup (placeholders, host);
            /* add the current placeholder to the list of placeholders
               for that host */
            if (l)
                g_hash_table_steal (placeholders, host);
            
            l = g_list_prepend (l, object);
            g_hash_table_insert (placeholders, host, l);
            g_object_unref (host);
        }
    }
}

static void 
egg_dock_layout_save (EggDockMaster *master,
                      xmlNodePtr     where)
{
    struct {
        xmlNodePtr  where;
        GHashTable *placeholders;
    } info;
    
    GHashTable *placeholders;
    
    g_return_if_fail (master != NULL && where != NULL);

    /* build the placeholder's hash: the hash keeps lists of
     * placeholders associated to each object, so that we can save the
     * placeholders when we are saving the object (since placeholders
     * don't show up in the normal widget hierarchy) */
    placeholders = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                          NULL, (GDestroyNotify) g_list_free);
    egg_dock_master_foreach (master, (GFunc) add_placeholder, placeholders);
    
    /* save the layout recursively */
    info.where = where;
    info.placeholders = placeholders;
    
    egg_dock_master_foreach_toplevel (master, TRUE,
                                      (GFunc) egg_dock_layout_foreach_object_save,
                                      (gpointer) &info);

    g_hash_table_destroy (placeholders);
}


/* ----- Public interface ----- */

EggDockLayout *
egg_dock_layout_new (EggDock *dock)
{
    EggDockMaster *master = NULL;
    
    /* get the master of the given dock */
    if (dock)
        master = EGG_DOCK_OBJECT_GET_MASTER (dock);
    
    return g_object_new (EGG_TYPE_DOCK_LAYOUT,
                         "master", master,
                         NULL);
}

static gboolean
egg_dock_layout_idle_save (EggDockLayout *layout)
{
    /* save default layout */
    egg_dock_layout_save_layout (layout, NULL);
    
    layout->_priv->idle_save_pending = FALSE;
    
    return FALSE;
}

static void
egg_dock_layout_layout_changed_cb (EggDockMaster *master,
                                   EggDockLayout *layout)
{
    /* update model */
    update_items_model (layout);

    if (!layout->_priv->idle_save_pending) {
        g_idle_add ((GSourceFunc) egg_dock_layout_idle_save, layout);
        layout->_priv->idle_save_pending = TRUE;
    }
}

void
egg_dock_layout_attach (EggDockLayout *layout,
                        EggDockMaster *master)
{
    g_return_if_fail (layout != NULL);
    g_return_if_fail (master == NULL || EGG_IS_DOCK_MASTER (master));
    
    if (layout->master) {
        g_signal_handlers_disconnect_matched (layout->master, G_SIGNAL_MATCH_DATA,
                                              0, 0, NULL, NULL, layout);
        g_object_unref (layout->master);
    }
    
    gtk_list_store_clear (layout->_priv->items_model);
    
    layout->master = master;
    if (layout->master) {
        g_object_ref (layout->master);
        g_signal_connect (layout->master, "layout_changed",
                          (GCallback) egg_dock_layout_layout_changed_cb,
                          layout);
    }

    update_items_model (layout);
}

gboolean
egg_dock_layout_load_layout (EggDockLayout *layout,
                             const gchar   *name)
{
    xmlNodePtr  node;
    gchar      *layout_name;

    g_return_val_if_fail (layout != NULL, FALSE);
    
    if (!layout->_priv->doc || !layout->master)
        return FALSE;

    if (!name)
        layout_name = DEFAULT_LAYOUT;
    else
        layout_name = (gchar *) name;

    node = egg_dock_layout_find_layout (layout, layout_name);
    if (!node && !name)
        /* return the first layout if the default name failed to load */
        node = egg_dock_layout_find_layout (layout, NULL);

    if (node) {
        egg_dock_layout_load (layout->master, node);
        return TRUE;
    } else
        return FALSE;
}

void
egg_dock_layout_save_layout (EggDockLayout *layout,
                             const gchar   *name)
{
    xmlNodePtr  node;
    gchar      *layout_name;

    g_return_if_fail (layout != NULL);
    g_return_if_fail (layout->master != NULL);
    
    if (!layout->_priv->doc)
        egg_dock_layout_build_doc (layout);

    if (!name)
        layout_name = DEFAULT_LAYOUT;
    else
        layout_name = (gchar *) name;

    /* delete any previously node with the same name */
    node = egg_dock_layout_find_layout (layout, layout_name);
    if (node) {
        xmlUnlinkNode (node);
        xmlFreeNode (node);
    };

    /* create the new node */
    node = xmlNewChild (layout->_priv->doc->children, NULL, 
                        LAYOUT_ELEMENT_NAME, NULL);
    xmlSetProp (node, NAME_ATTRIBUTE_NAME, layout_name);

    /* save the layout */
    egg_dock_layout_save (layout->master, node);
    layout->dirty = TRUE;
    g_object_notify (G_OBJECT (layout), "dirty");
}

void
egg_dock_layout_delete_layout (EggDockLayout *layout,
                               const gchar   *name)
{
    xmlNodePtr node;

    g_return_if_fail (layout != NULL);

    /* don't allow the deletion of the default layout */
    if (!name || !strcmp (DEFAULT_LAYOUT, name))
        return;
    
    node = egg_dock_layout_find_layout (layout, name);
    if (node) {
        xmlUnlinkNode (node);
        xmlFreeNode (node);
        layout->dirty = TRUE;
        g_object_notify (G_OBJECT (layout), "dirty");
    }
}

void
egg_dock_layout_run_manager (EggDockLayout *layout)
{
    GtkWidget *dialog, *container;
    GtkWidget *parent = NULL;
    
    g_return_if_fail (layout != NULL);

    if (!layout->master)
        /* not attached to a dock yet */
        return;

    container = egg_dock_layout_construct_ui (layout);
    if (!container)
        return;

    parent = GTK_WIDGET (egg_dock_master_get_controller (layout->master));
    if (parent)
        parent = gtk_widget_get_toplevel (parent);
    
    dialog = gtk_dialog_new_with_buttons (_("Layout managment"),
                                          parent ? GTK_WINDOW (parent) : NULL,
                                          GTK_DIALOG_MODAL | GTK_DIALOG_NO_SEPARATOR,
                                          GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                          NULL);

    gtk_window_set_default_size (GTK_WINDOW (dialog), -1, 300);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), container);
    
    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}

gboolean
egg_dock_layout_load_from_file (EggDockLayout *layout,
                                const gchar   *filename)
{
    gboolean retval = FALSE;

    if (layout->_priv->doc) {
        xmlFreeDoc (layout->_priv->doc);
        layout->_priv->doc = NULL;
        layout->dirty = FALSE;
        g_object_notify (G_OBJECT (layout), "dirty");
    }

    /* FIXME: cannot open symlinks */
    if (g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
        layout->_priv->doc = xmlParseFile (filename);
        if (layout->_priv->doc) {
            xmlNodePtr root = layout->_priv->doc->children;
            /* minimum validation: test the root element */
            if (root && !strcmp (root->name, ROOT_ELEMENT)) {
                update_layouts_model (layout);
                retval = TRUE;
            } else {
                xmlFreeDoc (layout->_priv->doc);
                layout->_priv->doc = NULL;
            }		
        }
    }

    return retval;
}

gboolean
egg_dock_layout_save_to_file (EggDockLayout *layout,
                              const gchar   *filename)
{
    FILE     *file_handle;
    int       bytes;
    gboolean  retval = FALSE;

    g_return_val_if_fail (layout != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    /* if there is still no xml doc, create an empty one */
    if (!layout->_priv->doc)
        egg_dock_layout_build_doc (layout);

    file_handle = fopen (filename, "w");
    if (file_handle) {
        bytes = xmlDocDump (file_handle, layout->_priv->doc);
        if (bytes >= 0) {
            layout->dirty = FALSE;
            g_object_notify (G_OBJECT (layout), "dirty");
            retval = TRUE;
        };
        fclose (file_handle);
    };

    return retval;
}

gboolean
egg_dock_layout_is_dirty (EggDockLayout *layout)
{
    g_return_val_if_fail (layout != NULL, FALSE);

    return layout->dirty;
};

GList *
egg_dock_layout_get_layouts (EggDockLayout *layout,
                             gboolean       include_default)
{
    GList      *retval = NULL;
    xmlNodePtr  node;

    g_return_val_if_fail (layout != NULL, NULL);

    if (!layout->_priv->doc)
        return NULL;

    node = layout->_priv->doc->children;
    for (node = node->children; node; node = node->next) {
        gchar *name;

        if (strcmp (node->name, LAYOUT_ELEMENT_NAME))
            continue;

        name = xmlGetProp (node, NAME_ATTRIBUTE_NAME);
        if (include_default || strcmp (name, DEFAULT_LAYOUT))
            retval = g_list_prepend (retval, g_strdup (name));
        xmlFree (name);
    };
    retval = g_list_reverse (retval);

    return retval;
}

GtkWidget *
egg_dock_layout_get_ui (EggDockLayout *layout)
{
    GtkWidget *ui;

    g_return_val_if_fail (layout != NULL, NULL);
    ui = egg_dock_layout_construct_ui (layout);

    return ui;
}

GtkWidget *
egg_dock_layout_get_items_ui (EggDockLayout *layout)
{
    GtkWidget *ui;

    g_return_val_if_fail (layout != NULL, NULL);
    ui = egg_dock_layout_construct_items_ui (layout);

    return ui;
}

GtkWidget *
egg_dock_layout_get_layouts_ui (EggDockLayout *layout)
{
    GtkWidget *ui;

    g_return_val_if_fail (layout != NULL, NULL);
    ui = egg_dock_layout_construct_layouts_ui (layout);

    return ui;
}