/**
 * egg-dock-item-grip.c
 * Copyright (C) 2002 Gustavo Gir�ldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2003 Biswapesh Chattopadhyay <biswapesh_chatterjee@tcscal.co.in>
 *
 * Based on bonobo-dock-item-grip.  Original copyright notice follows.
 *
 * Author:
 *    Michael Meeks
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <util/eggintl.h>
#include <string.h>
#include <glib-object.h>
#include <atk/atk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <util/egg-macros.h>
#include "egg-dock-item.h"
#include "egg-dock-item-grip.h"
#include "egg-dock.h"

/* Keep this for future API/ABI compatibility - Biswa */
struct _EggDockItemPrivate {
	gpointer unused;
};

enum {
    ACTIVATE,
    LAST_SIGNAL
};
static guint signals [LAST_SIGNAL];

EGG_CLASS_BOILERPLATE (EggDockItemGrip, egg_dock_item_grip,
			 GtkWidget, GTK_TYPE_WIDGET);

static gint
egg_dock_item_grip_expose (GtkWidget      *widget,
			   GdkEventExpose *event)
{
    GdkRectangle *clip = &event->area;
    GdkRectangle *rect = &widget->allocation;
    EggDockItemGrip *grip = (EggDockItemGrip *) widget;
    GtkShadowType shadow = GTK_SHADOW_OUT;

    gtk_paint_handle (widget->style,
                      widget->window,
#if GTK_CHECK_VERSION(2,20,0)
                      gtk_widget_get_state (widget),
#else
                      GTK_WIDGET_STATE (widget),
#endif
                      shadow,
                      clip, widget, "dockitem",
                      rect->x, rect->y, rect->width, rect->height,
                      grip->item->orientation);

#if GTK_CHECK_VERSION(2,18,0)
    if (gtk_widget_has_focus (widget)) {
#else
    if (GTK_WIDGET_HAS_FOCUS (widget)) {
#endif
        gint focus_width;
        gint focus_pad;
        GdkRectangle focus;

        gtk_widget_style_get (GTK_WIDGET (widget),
                              "focus-line-width", &focus_width,
                              "focus-padding", &focus_pad,
                              NULL);

        focus = *rect;
        focus.x += widget->style->xthickness + focus_pad;
        focus.y += widget->style->ythickness + focus_pad;
        focus.width -= 2 * (widget->style->xthickness + focus_pad);
        focus.height -= 2 * (widget->style->xthickness + focus_pad);

        gtk_paint_focus (widget->style, widget->window,
#if GTK_CHECK_VERSION(2,20,0)
                         gtk_widget_get_state (widget),
#else
                         GTK_WIDGET_STATE (widget),
#endif
                         clip, widget, "dockitem",
                         focus.x, focus.y,
                         focus.width, focus.height);
    }

    return FALSE;
}

static AtkObject *
egg_dock_item_grip_get_accessible (GtkWidget *widget)
{
    return NULL;
}

static void
egg_dock_item_grip_activate (EggDockItemGrip *grip)
{
}

static void
egg_dock_item_grip_dispose (GObject *object)
{
    EGG_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
egg_dock_item_grip_instance_init (EggDockItemGrip *grip)
{
    gtk_widget_set_can_focus (GTK_WIDGET (grip), TRUE);
    gtk_widget_set_has_window (GTK_WIDGET (grip), FALSE);
}

static gint
egg_dock_item_grip_key_press_event (GtkWidget   *widget,
                                    GdkEventKey *event)
{
    return GTK_WIDGET_CLASS (parent_class)->key_press_event (widget, event);
}

static void
egg_dock_item_grip_class_init (EggDockItemGripClass *klass)
{
    GtkBindingSet  *binding_set;
    GObjectClass   *gobject_class = (GObjectClass *) klass;
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

    parent_class = g_type_class_peek_parent (klass);

    gobject_class->dispose = egg_dock_item_grip_dispose;

    widget_class->expose_event = egg_dock_item_grip_expose;
    widget_class->get_accessible = egg_dock_item_grip_get_accessible;
    widget_class->key_press_event = egg_dock_item_grip_key_press_event;

    klass->activate = egg_dock_item_grip_activate;

    binding_set = gtk_binding_set_by_class (klass);

    signals [ACTIVATE] =
        g_signal_new ("activate",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                      G_STRUCT_OFFSET (
                          EggDockItemGripClass, activate),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
    widget_class->activate_signal = signals [ACTIVATE];

    gtk_binding_entry_add_signal (binding_set, GDK_Return, 0,
                                  "activate", 0);
    gtk_binding_entry_add_signal (binding_set, GDK_KP_Enter, 0,
                                  "activate", 0);
}

GtkWidget *
egg_dock_item_grip_new (EggDockItem *item)
{
    EggDockItemGrip *grip = g_object_new (EGG_TYPE_DOCK_ITEM_GRIP, NULL);

    grip->item = item;

    return GTK_WIDGET (grip);
}
