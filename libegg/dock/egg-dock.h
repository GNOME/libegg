/*
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

#ifndef __EGG_DOCK_H__
#define __EGG_DOCK_H__

#include <gtk/gtk.h>
#include <dock/egg-dock-object.h>
#include <dock/egg-dock-item.h>
#include <dock/egg-dock-placeholder.h>

G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK            (egg_dock_get_type ())
#define EGG_DOCK(obj)            (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK, EggDock))
#define EGG_DOCK_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK, EggDockClass))
#define EGG_IS_DOCK(obj)         (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK))
#define EGG_IS_DOCK_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK))
#define EGG_DOCK_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK, EggDockClass))

/* data types & structures */
typedef struct _EggDock        EggDock;
typedef struct _EggDockClass   EggDockClass;
typedef struct _EggDockPrivate EggDockPrivate;

struct _EggDock {
    EggDockObject    object;

    EggDockObject   *root;

    EggDockPrivate  *_priv;
};

struct _EggDockClass {
    EggDockObjectClass parent_class;

    void  (* layout_changed)  (EggDock *dock);    /* proxy signal for the master */
	gpointer unused[2]; /* Future expansion without ABI breakage */
};

/* additional macros */
#define EGG_DOCK_IS_CONTROLLER(dock)  \
    (egg_dock_master_get_controller (EGG_DOCK_OBJECT_GET_MASTER (dock)) == \
     EGG_DOCK_OBJECT (dock))

/* public interface */
 
GtkWidget     *egg_dock_new               (void);

GtkWidget     *egg_dock_new_from          (EggDock          *original,
                                           gboolean          floating);

GType          egg_dock_get_type          (void);

void           egg_dock_add_item          (EggDock          *dock,
                                           EggDockItem      *item,
                                           EggDockPlacement  place);

void           egg_dock_add_floating_item (EggDock        *dock,
                                           EggDockItem    *item,
                                           gint            x,
                                           gint            y,
                                           gint            width,
                                           gint            height);

EggDockItem   *egg_dock_get_item_by_name  (EggDock     *dock,
                                           const gchar *name);

EggDockPlaceholder *egg_dock_get_placeholder_by_name (EggDock     *dock,
                                                      const gchar *name);

GList         *egg_dock_get_named_items   (EggDock    *dock);

EggDock       *egg_dock_object_get_toplevel (EggDockObject *object);

void           egg_dock_xor_rect            (EggDock       *dock,
                                             GdkRectangle  *rect);

G_END_DECLS

#endif
