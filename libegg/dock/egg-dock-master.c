/*
 *
 * egg-dock-master.c - Object which manages a dock ring
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
#include <util/egg-macros.h>
#include <gdkconfig.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include "egg-dock-master.h"
#include "egg-dock.h"
#include "egg-dock-item.h"
#include <util/eggmarshalers.h>


/* ----- Private prototypes ----- */

static void     egg_dock_master_class_init    (EggDockMasterClass *klass);
static void     egg_dock_master_instance_init (EggDockMaster      *master);

static void     egg_dock_master_dispose       (GObject            *g_object);
static void     egg_dock_master_set_property  (GObject            *object,
                                               guint               prop_id,
                                               const GValue       *value,
                                               GParamSpec         *pspec);
static void     egg_dock_master_get_property  (GObject            *object,
                                               guint               prop_id,
                                               GValue             *value,
                                               GParamSpec         *pspec);

static void     _egg_dock_master_remove       (EggDockObject      *object,
                                               EggDockMaster      *master);

static void     egg_dock_master_drag_begin    (EggDockItem        *item, 
                                               gpointer            data);
static void     egg_dock_master_drag_end      (EggDockItem        *item,
                                               gboolean            cancelled,
                                               gpointer            data);
static void     egg_dock_master_drag_motion   (EggDockItem        *item, 
                                               gint                x, 
                                               gint                y,
                                               gpointer            data);

static void     _egg_dock_master_foreach      (gpointer            key,
                                               gpointer            value,
                                               gpointer            user_data);

static void     egg_dock_master_xor_rect      (EggDockMaster      *master);

static void     egg_dock_master_layout_changed (EggDockMaster     *master);

/* ----- Private data types and variables ----- */

enum {
    PROP_0,
    PROP_DEFAULT_TITLE,
    PROP_LOCKED
};

enum {
    LAYOUT_CHANGED,
    LAST_SIGNAL
};

struct _EggDockMasterPrivate {
    gint            number;             /* for naming nameless manual objects */
    gchar          *default_title;
    
    GdkGC          *root_xor_gc;
    gboolean        rect_drawn;
    EggDock        *rect_owner;
    
    EggDockRequest *drag_request;

    /* source id for the idle handler to emit a layout_changed signal */
    guint           idle_layout_changed_id;

    /* hashes to quickly calculate the overall locked status: i.e.
     * if size(unlocked_items) == 0 then locked = 1
     * else if size(locked_items) == 0 then locked = 0
     * else locked = -1
     */
    GHashTable     *locked_items;
    GHashTable     *unlocked_items;
};

#define COMPUTE_LOCKED(master)                                          \
    (g_hash_table_size ((master)->_priv->unlocked_items) == 0 ? 1 :     \
     (g_hash_table_size ((master)->_priv->locked_items) == 0 ? 0 : -1))

static guint master_signals [LAST_SIGNAL] = { 0 };


/* ----- Private interface ----- */

EGG_CLASS_BOILERPLATE (EggDockMaster, egg_dock_master, GObject, G_TYPE_OBJECT);

static void
egg_dock_master_class_init (EggDockMasterClass *klass)
{
    GObjectClass      *g_object_class;

    g_object_class = G_OBJECT_CLASS (klass);

    g_object_class->dispose = egg_dock_master_dispose;
    g_object_class->set_property = egg_dock_master_set_property;
    g_object_class->get_property = egg_dock_master_get_property;

    g_object_class_install_property (
        g_object_class, PROP_DEFAULT_TITLE,
        g_param_spec_string ("default_title", _("Default title"),
                             _("Default title for newly created floating docks"),
                             NULL,
                             G_PARAM_READWRITE));
    
    g_object_class_install_property (
        g_object_class, PROP_LOCKED,
        g_param_spec_int ("locked", _("Locked"),
                          _("If is set to 1, all the dock items bound to the master "
                            "are locked; if it's 0, all are unlocked; -1 indicates "
                            "inconsistency among the items"),
                          -1, 1, 0,
                          G_PARAM_READWRITE));

    master_signals [LAYOUT_CHANGED] = 
        g_signal_new ("layout_changed", 
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (EggDockMasterClass, layout_changed),
                      NULL, /* accumulator */
                      NULL, /* accu_data */
                      _egg_marshal_VOID__VOID,
                      G_TYPE_NONE, /* return type */
                      0);

    klass->layout_changed = egg_dock_master_layout_changed;
}

static void
egg_dock_master_instance_init (EggDockMaster *master)
{
    master->dock_objects = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, NULL);
    master->toplevel_docks = NULL;
    master->controller = NULL;
    master->dock_number = 1;
    
    master->_priv = g_new0 (EggDockMasterPrivate, 1);
    master->_priv->number = 1;

    master->_priv->locked_items = g_hash_table_new (g_direct_hash, g_direct_equal);
    master->_priv->unlocked_items = g_hash_table_new (g_direct_hash, g_direct_equal);
}

static void
_egg_dock_master_remove (EggDockObject *object,
                         EggDockMaster *master)
{
    g_return_if_fail (master != NULL && object != NULL);

    if (EGG_IS_DOCK (object)) {
        GList *found_link;

        found_link = g_list_find (master->toplevel_docks, object);
        if (found_link)
            master->toplevel_docks = g_list_delete_link (master->toplevel_docks,
                                                         found_link);
        if (object == master->controller) {
            GList *last;
            EggDockObject *new_controller = NULL;
            
            /* now find some other non-automatic toplevel to use as a
               new controller.  start from the last dock, since it's
               probably a non-floating and manual */
            last = g_list_last (master->toplevel_docks);
            while (last) {
                if (!EGG_DOCK_OBJECT_AUTOMATIC (last->data)) {
                    new_controller = EGG_DOCK_OBJECT (last->data);
                    break;
                }
                last = last->prev;
            };

            if (new_controller) {
                /* the new controller gets the ref (implicitly of course) */
                master->controller = new_controller;
            } else {
                master->controller = NULL;
                /* no controller, no master */
                g_object_unref (master);
            }
        }
    }
    /* disconnect dock object signals */
    g_signal_handlers_disconnect_matched (object, G_SIGNAL_MATCH_DATA, 
                                          0, 0, NULL, NULL, master);

    /* unref the object from the hash if it's there */
    if (object->name) {
        EggDockObject *found_object;
        found_object = g_hash_table_lookup (master->dock_objects, object->name);
        if (found_object == object) {
            g_hash_table_remove (master->dock_objects, object->name);
            g_object_unref (object);
        }
    }
}

static void
ht_foreach_build_slist (gpointer  key,
                        gpointer  value,
                        GSList  **slist)
{
    *slist = g_slist_prepend (*slist, value);
}

static void
egg_dock_master_dispose (GObject *g_object)
{
    EggDockMaster *master;
    
    g_return_if_fail (EGG_IS_DOCK_MASTER (g_object));

    master = EGG_DOCK_MASTER (g_object);

    if (master->toplevel_docks) {
        g_list_foreach (master->toplevel_docks,
                        (GFunc) egg_dock_object_unbind, NULL);
        g_list_free (master->toplevel_docks);
        master->toplevel_docks = NULL;
    }
    
    if (master->dock_objects) {
        GSList *alive_docks = NULL;
        g_hash_table_foreach (master->dock_objects,
                              (GHFunc) ht_foreach_build_slist, &alive_docks);
        while (alive_docks) {
            egg_dock_object_unbind (EGG_DOCK_OBJECT (alive_docks->data));
            alive_docks = g_slist_delete_link (alive_docks, alive_docks);
        }
        
        g_hash_table_destroy (master->dock_objects);
        master->dock_objects = NULL;
    }
    
    if (master->_priv) {
        if (master->_priv->idle_layout_changed_id)
            g_source_remove (master->_priv->idle_layout_changed_id);
        
        if (master->_priv->root_xor_gc) {
            g_object_unref (master->_priv->root_xor_gc);
            master->_priv->root_xor_gc = NULL;
        }
        if (master->_priv->drag_request) {
            if (G_IS_VALUE (&master->_priv->drag_request->extra))
                g_value_unset (&master->_priv->drag_request->extra);
            g_free (master->_priv->drag_request);
            master->_priv->drag_request = NULL;
        }
        g_free (master->_priv->default_title);
        master->_priv->default_title = NULL;

        g_hash_table_destroy (master->_priv->locked_items);
        master->_priv->locked_items = NULL;
        g_hash_table_destroy (master->_priv->unlocked_items);
        master->_priv->unlocked_items = NULL;
        
        g_free (master->_priv);
        master->_priv = NULL;
    }

    EGG_CALL_PARENT (G_OBJECT_CLASS, dispose, (g_object));
}

static void 
foreach_lock_unlock (EggDockItem *item,
                     gboolean     locked)
{
    if (!EGG_IS_DOCK_ITEM (item))
        return;
    
    g_object_set (item, "locked", locked, NULL);
    if (egg_dock_object_is_compound (EGG_DOCK_OBJECT (item)))
        gtk_container_foreach (GTK_CONTAINER (item),
                               (GtkCallback) foreach_lock_unlock,
                               (gpointer) locked);
}

static void
egg_dock_master_lock_unlock (EggDockMaster *master,
                             gboolean       locked)
{
    GList *l;
    
    for (l = master->toplevel_docks; l; l = l->next) {
        EggDock *dock = EGG_DOCK (l->data);
        if (dock->root)
            foreach_lock_unlock (EGG_DOCK_ITEM (dock->root), locked);
    }

    /* just to be sure hidden items are set too */
    egg_dock_master_foreach (master,
                             (GFunc) foreach_lock_unlock,
                             (gpointer) locked);
}

static void
egg_dock_master_set_property  (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    EggDockMaster *master = EGG_DOCK_MASTER (object);

    switch (prop_id) {
        case PROP_DEFAULT_TITLE:
            g_free (master->_priv->default_title);
            master->_priv->default_title = g_value_dup_string (value);
            break;
        case PROP_LOCKED:
            if (g_value_get_int (value) >= 0)
                egg_dock_master_lock_unlock (master, (g_value_get_int (value) > 0));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
egg_dock_master_get_property  (GObject      *object,
                               guint         prop_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
    EggDockMaster *master = EGG_DOCK_MASTER (object);

    switch (prop_id) {
        case PROP_DEFAULT_TITLE:
            g_value_set_string (value, master->_priv->default_title);
            break;
        case PROP_LOCKED:
            g_value_set_int (value, COMPUTE_LOCKED (master));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
egg_dock_master_drag_begin (EggDockItem *item,
                            gpointer     data)
{
    EggDockMaster  *master;
    EggDockRequest *request;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (item != NULL);

    master = EGG_DOCK_MASTER (data);

    if (!master->_priv->drag_request)
        master->_priv->drag_request = g_new0 (EggDockRequest, 1);

    request = master->_priv->drag_request;
    
    /* Set the target to itself so it won't go floating with just a click. */
    request->applicant = EGG_DOCK_OBJECT (item);
    request->target = EGG_DOCK_OBJECT (item);
    request->position = EGG_DOCK_FLOATING;
    if (G_IS_VALUE (&request->extra))
        g_value_unset (&request->extra);

    master->_priv->rect_drawn = FALSE;
    master->_priv->rect_owner = NULL;
}

static void
egg_dock_master_drag_end (EggDockItem *item, 
                          gboolean     cancelled,
                          gpointer     data)
{
    EggDockMaster  *master;
    EggDockRequest *request;
    
    g_return_if_fail (data != NULL);
    g_return_if_fail (item != NULL);

    master = EGG_DOCK_MASTER (data);
    request = master->_priv->drag_request;
    
    g_return_if_fail (EGG_DOCK_OBJECT (item) == request->applicant);
    
    /* Erase previously drawn rectangle */
    if (master->_priv->rect_drawn)
        egg_dock_master_xor_rect (master);
    
    /* cancel conditions */
    if (cancelled || request->applicant == request->target)
        return;

    /* Enforce behaviour restrictions */
    if (request->position == EGG_DOCK_FLOATING &&
        item->behavior | EGG_DOCK_ITEM_BEH_NEVER_FLOATING)
        return;

    /* dock object to the requested position */
    egg_dock_object_dock (request->target,
                          request->applicant,
                          request->position,
                          &request->extra);
    
    g_signal_emit (master, master_signals [LAYOUT_CHANGED], 0);
}

static void
egg_dock_master_drag_motion (EggDockItem *item, 
                             gint         root_x, 
                             gint         root_y,
                             gpointer     data)
{
    EggDockMaster  *master;
    EggDockRequest  my_request, *request;
    GdkWindow      *window;
    gint            win_x, win_y;
    gint            x, y;
    EggDock        *dock = NULL;
    gboolean        may_dock = FALSE;
    
    g_return_if_fail (item != NULL && data != NULL);

    master = EGG_DOCK_MASTER (data);
    request = master->_priv->drag_request;

    g_return_if_fail (EGG_DOCK_OBJECT (item) == request->applicant);
    
    my_request = *request;

    /* first look under the pointer */
    window = gdk_window_at_pointer (&win_x, &win_y);
    if (window) {
        GtkWidget *widget;
        /* ok, now get the widget who owns that window and see if we can
           get to a EggDock by walking up the hierarchy */
        gdk_window_get_user_data (window, (gpointer *) &widget);
        if (GTK_IS_WIDGET (widget)) {
            while (widget && !EGG_IS_DOCK (widget))
                widget = widget->parent;
            if (widget) {
                gint win_w, win_h;
                
                /* verify that the pointer is still in that dock
                   (the user could have moved it) */
                gdk_window_get_geometry (widget->window,
                                         NULL, NULL, &win_w, &win_h, NULL);
                gdk_window_get_origin (widget->window, &win_x, &win_y);
                if (root_x >= win_x && root_x < win_x + win_w &&
                    root_y >= win_y && root_y < win_y + win_h)
                    dock = EGG_DOCK (widget);
            }
        }
    }

    if (dock) {
        /* translate root coordinates into dock object coordinates
           (i.e. widget coordinates) */
        gdk_window_get_origin (GTK_WIDGET (dock)->window, &win_x, &win_y);
        x = root_x - win_x;
        y = root_y - win_y;
        may_dock = egg_dock_object_dock_request (EGG_DOCK_OBJECT (dock),
                                                 x, y, &my_request);
    }
    else {
        GList *l;
        
        /* try to dock the item in all the docks in the ring in turn */
        for (l = master->toplevel_docks; l; l = l->next) {
            dock = EGG_DOCK (l->data);
            /* translate root coordinates into dock object coordinates
               (i.e. widget coordinates) */
            gdk_window_get_origin (GTK_WIDGET (dock)->window, &win_x, &win_y);
            x = root_x - win_x;
            y = root_y - win_y;
            may_dock = egg_dock_object_dock_request (EGG_DOCK_OBJECT (dock),
                                                     x, y, &my_request);
            if (may_dock)
                break;
        }
    }
    
    if (!may_dock) {
        GtkRequisition req;
        
        dock = NULL;
        my_request.target = EGG_DOCK_OBJECT (
            egg_dock_object_get_toplevel (request->applicant));
        my_request.position = EGG_DOCK_FLOATING;

        egg_dock_item_preferred_size (EGG_DOCK_ITEM (request->applicant), &req);
        my_request.rect.width = req.width;
        my_request.rect.height = req.height;

        my_request.rect.x = root_x - EGG_DOCK_ITEM (request->applicant)->dragoff_x;
        my_request.rect.y = root_y - EGG_DOCK_ITEM (request->applicant)->dragoff_y;

        /* setup extra docking information */
        if (G_IS_VALUE (&my_request.extra))
            g_value_unset (&my_request.extra);

        g_value_init (&my_request.extra, GDK_TYPE_RECTANGLE);
        g_value_set_boxed (&my_request.extra, &my_request.rect);
    };

    if (!(my_request.rect.x == request->rect.x &&
          my_request.rect.y == request->rect.y &&
          my_request.rect.width == request->rect.width &&
          my_request.rect.height == request->rect.height &&
          dock == master->_priv->rect_owner)) {

        /* erase the previous rectangle */
        if (master->_priv->rect_drawn)
            egg_dock_master_xor_rect (master);
    }

    /* set the new values */
    *request = my_request;
    master->_priv->rect_owner = dock;
    
    /* draw the previous rectangle */
    if (~master->_priv->rect_drawn)
        egg_dock_master_xor_rect (master);
}

static void
_egg_dock_master_foreach (gpointer key,
                          gpointer value,
                          gpointer user_data)
{
    struct {
        GFunc    function;
        gpointer user_data;
    } *data = user_data;

    (* data->function) (GTK_WIDGET (value), data->user_data);
}

static void
egg_dock_master_xor_rect (EggDockMaster *master)
{
    gint8         dash_list [2];
    GdkWindow    *window;
    GdkRectangle *rect;
    
    if (!master->_priv || !master->_priv->drag_request)
        return;
    
    master->_priv->rect_drawn = ~master->_priv->rect_drawn;
    
    if (master->_priv->rect_owner) {
        egg_dock_xor_rect (master->_priv->rect_owner,
                           &master->_priv->drag_request->rect);
        return;
    }
    
    rect = &master->_priv->drag_request->rect;

#ifdef GDK_WINDOWING_X11
    window = gdk_window_lookup (gdk_x11_get_default_root_xwindow ());
#else
    window = gdk_get_default_root_window (); /* ??? */
#endif

    if (!master->_priv->root_xor_gc) {
        GdkGCValues values;

        values.function = GDK_INVERT;
        values.subwindow_mode = GDK_INCLUDE_INFERIORS;
        master->_priv->root_xor_gc = gdk_gc_new_with_values (
            window, &values, GDK_GC_FUNCTION | GDK_GC_SUBWINDOW);
    };

    gdk_gc_set_line_attributes (master->_priv->root_xor_gc, 1,
                                GDK_LINE_ON_OFF_DASH,
                                GDK_CAP_NOT_LAST,
                                GDK_JOIN_BEVEL);
    
    dash_list[0] = 1;
    dash_list[1] = 1;
    gdk_gc_set_dashes (master->_priv->root_xor_gc, 1, dash_list, 2);

    gdk_draw_rectangle (window, master->_priv->root_xor_gc, 0, 
                        rect->x, rect->y,
                        rect->width, rect->height);

    gdk_gc_set_dashes (master->_priv->root_xor_gc, 0, dash_list, 2);

    gdk_draw_rectangle (window, master->_priv->root_xor_gc, 0, 
                        rect->x + 1, rect->y + 1,
                        rect->width - 2, rect->height - 2);
}

static void
egg_dock_master_layout_changed (EggDockMaster *master)
{
    g_return_if_fail (EGG_IS_DOCK_MASTER (master));

    /* emit "layout_changed" on the controller to notify the user who
     * normally shouldn't have access to us */
    if (master->controller)
        g_signal_emit_by_name (master->controller, "layout_changed");

    /* remove the idle handler if there is one */
    if (master->_priv->idle_layout_changed_id) {
        g_source_remove (master->_priv->idle_layout_changed_id);
        master->_priv->idle_layout_changed_id = 0;
    }
}

static gboolean
idle_emit_layout_changed (gpointer user_data)
{
    EggDockMaster *master = user_data;

    g_return_val_if_fail (master && EGG_IS_DOCK_MASTER (master), FALSE);

    master->_priv->idle_layout_changed_id = 0;
    g_signal_emit (master, master_signals [LAYOUT_CHANGED], 0);
    
    return FALSE;
}

static void 
item_dock_cb (EggDockObject    *object,
              EggDockObject    *requestor,
              EggDockPlacement  position,
              GValue           *other_data,
              gpointer          user_data)
{
    EggDockMaster *master = user_data;
    
    g_return_if_fail (requestor && EGG_IS_DOCK_OBJECT (requestor));
    g_return_if_fail (master && EGG_IS_DOCK_MASTER (master));

    /* here we are in fact interested in the requestor, since it's
     * assumed that object will not change its visibility... for the
     * requestor, however, could mean that it's being shown */
    if (!EGG_DOCK_OBJECT_IN_REFLOW (requestor) &&
        !EGG_DOCK_OBJECT_AUTOMATIC (requestor)) {
        if (!master->_priv->idle_layout_changed_id)
            master->_priv->idle_layout_changed_id =
                g_idle_add (idle_emit_layout_changed, master);
    }
}

static void 
item_detach_cb (EggDockObject *object,
                gboolean       recursive,
                gpointer       user_data)
{
    EggDockMaster *master = user_data;
    
    g_return_if_fail (object && EGG_IS_DOCK_OBJECT (object));
    g_return_if_fail (master && EGG_IS_DOCK_MASTER (master));

    if (!EGG_DOCK_OBJECT_IN_REFLOW (object) &&
        !EGG_DOCK_OBJECT_AUTOMATIC (object)) {
        if (!master->_priv->idle_layout_changed_id)
            master->_priv->idle_layout_changed_id =
                g_idle_add (idle_emit_layout_changed, master);
    }
}

static void
item_notify_cb (EggDockObject *object,
                GParamSpec    *pspec,
                gpointer       user_data)
{
    EggDockMaster *master = user_data;
    gint locked = COMPUTE_LOCKED (master);
    gboolean item_locked;
    
    g_object_get (object, "locked", &item_locked, NULL);

    if (item_locked) {
        g_hash_table_remove (master->_priv->unlocked_items, object);
        g_hash_table_insert (master->_priv->locked_items, object, NULL);
    } else {
        g_hash_table_remove (master->_priv->locked_items, object);
        g_hash_table_insert (master->_priv->unlocked_items, object, NULL);
    }
    
    if (COMPUTE_LOCKED (master) != locked)
        g_object_notify (G_OBJECT (master), "locked");
}

/* ----- Public interface ----- */

void
egg_dock_master_add (EggDockMaster *master,
                     EggDockObject *object)
{
    g_return_if_fail (master != NULL && object != NULL);

    if (!EGG_DOCK_OBJECT_AUTOMATIC (object)) {
        EggDockObject *found_object;
        
        /* create a name for the object if it doesn't have one */
        if (!object->name)
            /* directly set the name, since it's a construction only
               property */
            object->name = g_strdup_printf ("__dock_%u", master->_priv->number++);
        
        /* add the object to our hash list */
        if ((found_object = g_hash_table_lookup (master->dock_objects, object->name))) {
            g_warning (_("master %p: unable to add object %p[%s] to the hash.  "
                         "There already is an item with that name (%p)."),
                       master, object, object->name, found_object);
        }
        else {
            g_object_ref (object);
            gtk_object_sink (GTK_OBJECT (object));
            g_hash_table_insert (master->dock_objects, g_strdup (object->name), object);
        }
    }
    
    if (EGG_IS_DOCK (object)) {
        gboolean floating;
        
        /* if this is the first toplevel we are adding, name it controller */
        if (!master->toplevel_docks)
            /* the dock should already have the ref */
            master->controller = object;
        
        /* add dock to the toplevel list */
        g_object_get (object, "floating", &floating, NULL);
        if (floating)
            master->toplevel_docks = g_list_prepend (master->toplevel_docks, object);
        else
            master->toplevel_docks = g_list_append (master->toplevel_docks, object);

        /* we are interested in the dock request this toplevel
         * receives to update the layout */
        g_signal_connect (object, "dock",
                          G_CALLBACK (item_dock_cb), master);

    }
    else if (EGG_IS_DOCK_ITEM (object)) {
        /* we need to connect the item's signals */
        g_signal_connect (object, "dock_drag_begin",
                          G_CALLBACK (egg_dock_master_drag_begin), master);
        g_signal_connect (object, "dock_drag_motion",
                          G_CALLBACK (egg_dock_master_drag_motion), master);
        g_signal_connect (object, "dock_drag_end",
                          G_CALLBACK (egg_dock_master_drag_end), master);
        g_signal_connect (object, "dock",
                          G_CALLBACK (item_dock_cb), master);
        g_signal_connect (object, "detach",
                          G_CALLBACK (item_detach_cb), master);

        /* register to "locked" notification if the item has a grip,
         * and add the item to the corresponding hash */
        if (EGG_DOCK_ITEM_HAS_GRIP (object)) {
            g_signal_connect (object, "notify::locked",
                              G_CALLBACK (item_notify_cb), master);
            item_notify_cb (object, NULL, master);
        }
        
        /* post a layout_changed emission if the item is not automatic
         * (since it should be added to the items model) */
        if (!EGG_DOCK_OBJECT_AUTOMATIC (object)) {
            if (!master->_priv->idle_layout_changed_id)
                master->_priv->idle_layout_changed_id =
                    g_idle_add (idle_emit_layout_changed, master);
        }
    }
}

void
egg_dock_master_remove (EggDockMaster *master,
                        EggDockObject *object)
{
    g_return_if_fail (master != NULL && object != NULL);

    /* remove from locked/unlocked hashes and property change if
     * that's the case */
    if (EGG_IS_DOCK_ITEM (object) && EGG_DOCK_ITEM_HAS_GRIP (object)) {
        gint locked = COMPUTE_LOCKED (master);
        if (g_hash_table_remove (master->_priv->locked_items, object) ||
            g_hash_table_remove (master->_priv->unlocked_items, object)) {
            if (COMPUTE_LOCKED (master) != locked)
                g_object_notify (G_OBJECT (master), "locked");
        }
    }
        
    /* ref the master, since removing the controller could cause master disposal */
    g_object_ref (master);
    
    /* all the interesting stuff happens in _egg_dock_master_remove */
    _egg_dock_master_remove (object, master);

    /* post a layout_changed emission if the item is not automatic
     * (since it should be removed from the items model) */
    if (!EGG_DOCK_OBJECT_AUTOMATIC (object)) {
        if (!master->_priv->idle_layout_changed_id)
            master->_priv->idle_layout_changed_id =
                g_idle_add (idle_emit_layout_changed, master);
    }
    
    /* balance ref count */
    g_object_unref (master);
}

void
egg_dock_master_foreach (EggDockMaster *master,
                         GFunc          function,
                         gpointer       user_data)
{
    struct {
        GFunc    function;
        gpointer user_data;
    } data;

    g_return_if_fail (master != NULL && function != NULL);

    data.function = function;
    data.user_data = user_data;
    g_hash_table_foreach (master->dock_objects, _egg_dock_master_foreach, &data);
}

void
egg_dock_master_foreach_toplevel (EggDockMaster *master,
                                  gboolean       include_controller,
                                  GFunc          function,
                                  gpointer       user_data)
{
    GList *l;
    
    g_return_if_fail (master != NULL && function != NULL);

    for (l = master->toplevel_docks; l; ) {
        EggDockObject *object = EGG_DOCK_OBJECT (l->data);
        l = l->next;
        if (object != master->controller || include_controller)
            (* function) (GTK_WIDGET (object), user_data);
    }
}

EggDockObject *
egg_dock_master_get_object (EggDockMaster *master,
                            const gchar   *nick_name)
{
    gpointer *found;
    
    g_return_val_if_fail (master != NULL, NULL);

    if (!nick_name)
        return NULL;

    found = g_hash_table_lookup (master->dock_objects, nick_name);

    return found ? EGG_DOCK_OBJECT (found) : NULL;
}

EggDockObject *
egg_dock_master_get_controller (EggDockMaster *master)
{
    g_return_val_if_fail (master != NULL, NULL);

    return master->controller;
}

void
egg_dock_master_set_controller (EggDockMaster *master,
                                EggDockObject *new_controller)
{
    g_return_if_fail (master != NULL);

    if (new_controller) {
        if (EGG_DOCK_OBJECT_AUTOMATIC (new_controller))
            g_warning (_("The new dock controller %p is automatic.  Only manual "
                         "dock objects should be named controller."), new_controller);
        
        /* check that the controller is in the toplevel list */
        if (!g_list_find (master->toplevel_docks, new_controller))
            egg_dock_master_add (master, new_controller);
        master->controller = new_controller;

    } else {
        master->controller = NULL;
        /* no controller, no master */
        g_object_unref (master);
    }
}
