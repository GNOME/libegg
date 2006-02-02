/*
 * egg-dock-item-grip.h
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2003 Biswapesh Chattopadhyay <biswapesh_chatterjee@tcscal.co.in>
 * 
 * Based on bonobo-dock-item-grip.  Original copyright notice follows.
 *
 * Author:
 *    Michael Meeks
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 */

#ifndef _EGG_DOCK_ITEM_GRIP_H_
#define _EGG_DOCK_ITEM_GRIP_H_

#include <gtk/gtkwidget.h>
#include <dock/egg-dock-item.h>

G_BEGIN_DECLS

#define EGG_TYPE_DOCK_ITEM_GRIP            (egg_dock_item_grip_get_type())
#define EGG_DOCK_ITEM_GRIP(obj)            \
    (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_ITEM_GRIP, EggDockItemGrip))
#define EGG_DOCK_ITEM_GRIP_CLASS(klass)    \
    (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_ITEM_GRIP, EggDockItemGripClass))
#define EGG_IS_DOCK_ITEM_GRIP(obj)         \
    (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_ITEM_GRIP))
#define EGG_IS_DOCK_ITEM_GRIP_CLASS(klass) \
    (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_ITEM_GRIP))
#define EGG_DOCK_ITEM_GRIP_GET_CLASS(obj)  \
    (GTK_CHECK_GET_CLASS ((obj), EGG_TYPE_DOCK_ITEM_GRIP, EggDockItemGripClass))

typedef struct _EggDockItemGripPrivate EggDockItemGripPrivate;

typedef struct {
    GtkWidget parent;
    EggDockItem *item;
	/* private */
	EggDockItemGripPrivate *_priv;
} EggDockItemGrip;

typedef struct {
    GtkWidgetClass parent_class;

    void (*activate) (EggDockItemGrip *grip);
	gpointer unused[2]; /* For future expansion without breaking ABI */
} EggDockItemGripClass;

GType      egg_dock_item_grip_get_type (void);
GtkWidget *egg_dock_item_grip_new      (EggDockItem *item);

G_END_DECLS

#endif /* _EGG_DOCK_ITEM_GRIP_H_ */
