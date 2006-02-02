/*
 *
 * egg-dock-item.h
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2003 Biswapesh Chattopadhyay <biswapesh_chatterjee@tcscal.co.in>
 *
 * Based on GnomeDockItem/BonoboDockItem.  Original copyright notice follows.
 *
 * Copyright (C) 1998 Ettore Perazzoli
 * Copyright (C) 1998 Elliot Lee
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald 
 * All rights reserved.
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
 */

#ifndef __EGG_DOCK_ITEM_H__
#define __EGG_DOCK_ITEM_H__

#include <dock/egg-dock-object.h>

G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK_ITEM            (egg_dock_item_get_type ())
#define EGG_DOCK_ITEM(obj)            (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_ITEM, EggDockItem))
#define EGG_DOCK_ITEM_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_ITEM, EggDockItemClass))
#define EGG_IS_DOCK_ITEM(obj)         (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_ITEM))
#define EGG_IS_DOCK_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_ITEM))
#define EGG_DOCK_ITEM_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_ITEM, EggDockItemClass))

/* data types & structures */
typedef enum {
    EGG_DOCK_ITEM_BEH_NORMAL           = 0,
    EGG_DOCK_ITEM_BEH_NEVER_FLOATING   = 1 << 0,
    EGG_DOCK_ITEM_BEH_NEVER_VERTICAL   = 1 << 1,
    EGG_DOCK_ITEM_BEH_NEVER_HORIZONTAL = 1 << 2,
    EGG_DOCK_ITEM_BEH_LOCKED           = 1 << 3,
    EGG_DOCK_ITEM_BEH_CANT_DOCK_TOP    = 1 << 4,
    EGG_DOCK_ITEM_BEH_CANT_DOCK_BOTTOM = 1 << 5,
    EGG_DOCK_ITEM_BEH_CANT_DOCK_LEFT   = 1 << 6,
    EGG_DOCK_ITEM_BEH_CANT_DOCK_RIGHT  = 1 << 7,
    EGG_DOCK_ITEM_BEH_CANT_DOCK_CENTER = 1 << 8
} EggDockItemBehavior;

typedef enum {
    EGG_DOCK_IN_DRAG             = 1 << EGG_DOCK_OBJECT_FLAGS_SHIFT,
    EGG_DOCK_IN_PREDRAG          = 1 << (EGG_DOCK_OBJECT_FLAGS_SHIFT + 1),
    /* for general use: indicates the user has started an action on
       the dock item */
    EGG_DOCK_USER_ACTION         = 1 << (EGG_DOCK_OBJECT_FLAGS_SHIFT + 2)
} EggDockItemFlags;

typedef struct _EggDockItem        EggDockItem;
typedef struct _EggDockItemClass   EggDockItemClass;
typedef struct _EggDockItemPrivate EggDockItemPrivate;

struct _EggDockItem {
    EggDockObject        object;

    GtkWidget           *child;
    EggDockItemBehavior  behavior;
    GtkOrientation       orientation;

    guint                resize : 1;

    gint                 dragoff_x, dragoff_y;    /* these need to be
                                                     accesible from
                                                     outside */
    EggDockItemPrivate  *_priv;
};

struct _EggDockItemClass {
    EggDockObjectClass  parent_class;

    gboolean            has_grip;
    
    /* virtuals */
    void     (* dock_drag_begin)  (EggDockItem    *item);
    void     (* dock_drag_motion) (EggDockItem    *item,
                                   gint            x,
                                   gint            y);
    void     (* dock_drag_end)    (EggDockItem    *item,
                                   gboolean        cancelled);
                                   
    void     (* set_orientation)  (EggDockItem    *item,
                                   GtkOrientation  orientation);
	gpointer unused[2]; /* Future expansion without breaking ABI */
};

/* additional macros */
#define EGG_DOCK_ITEM_FLAGS(item)     (EGG_DOCK_OBJECT (item)->flags)
#define EGG_DOCK_ITEM_IN_DRAG(item) \
    ((EGG_DOCK_ITEM_FLAGS (item) & EGG_DOCK_IN_DRAG) != 0)
#define EGG_DOCK_ITEM_IN_PREDRAG(item) \
    ((EGG_DOCK_ITEM_FLAGS (item) & EGG_DOCK_IN_PREDRAG) != 0)
#define EGG_DOCK_ITEM_USER_ACTION(item) \
    ((EGG_DOCK_ITEM_FLAGS (item) & EGG_DOCK_USER_ACTION) != 0)
   
#define EGG_DOCK_ITEM_SET_FLAGS(item,flag) \
    G_STMT_START { (EGG_DOCK_ITEM_FLAGS (item) |= (flag)); } G_STMT_END
#define EGG_DOCK_ITEM_UNSET_FLAGS(item,flag) \
    G_STMT_START { (EGG_DOCK_ITEM_FLAGS (item) &= ~(flag)); } G_STMT_END

#define EGG_DOCK_ITEM_HAS_GRIP(item) (EGG_DOCK_ITEM_GET_CLASS (item)->has_grip)

/* public interface */
 
GtkWidget     *egg_dock_item_new               (const gchar         *name,
                                                const gchar         *long_name,
                                                EggDockItemBehavior  behavior);

GType          egg_dock_item_get_type          (void);

void           egg_dock_item_dock_to           (EggDockItem      *item,
                                                EggDockItem      *target,
                                                EggDockPlacement  position,
                                                gint              docking_param);

void           egg_dock_item_set_orientation   (EggDockItem    *item,
                                                GtkOrientation  orientation);

GtkWidget     *egg_dock_item_get_tablabel      (EggDockItem *item);
void           egg_dock_item_set_tablabel      (EggDockItem *item,
                                                GtkWidget   *tablabel);
void           egg_dock_item_hide_grip         (EggDockItem *item);
void           egg_dock_item_show_grip         (EggDockItem *item);

/* bind and unbind items to a dock */
void           egg_dock_item_bind              (EggDockItem *item,
                                                GtkWidget   *dock);

void           egg_dock_item_unbind            (EggDockItem *item);

void           egg_dock_item_hide_item         (EggDockItem *item);

void           egg_dock_item_show_item         (EggDockItem *item);

void           egg_dock_item_lock              (EggDockItem *item);

void           egg_dock_item_unlock            (EggDockItem *item);

void        egg_dock_item_set_default_position (EggDockItem      *item,
                                                EggDockObject    *reference);

void        egg_dock_item_preferred_size       (EggDockItem      *item,
                                                GtkRequisition   *req);


G_END_DECLS

#endif
