/*
 * egg-dock-tablabel.h
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

#ifndef __EGG_DOCK_TABLABEL_H__
#define __EGG_DOCK_TABLABEL_H__

#include <gtk/gtk.h>
#include <dock/egg-dock-item.h>


G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK_TABLABEL            (egg_dock_tablabel_get_type ())
#define EGG_DOCK_TABLABEL(obj)            (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_TABLABEL, EggDockTablabel))
#define EGG_DOCK_TABLABEL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_TABLABEL, EggDockTablabelClass))
#define EGG_IS_DOCK_TABLABEL(obj)         (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_TABLABEL))
#define EGG_IS_DOCK_TABLABEL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_TABLABEL))
#define EGG_DOCK_TABLABEL_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_TABLABEL, EggDockTablabelClass))

/* data types & structures */
typedef struct _EggDockTablabel      EggDockTablabel;
typedef struct _EggDockTablabelClass EggDockTablabelClass;
typedef struct _EggDockTablabelPrivate EggDockTablabelPrivate;

struct _EggDockTablabel {
    GtkBin          parent;

    guint           drag_handle_size;
    GtkWidget      *item;
    GdkWindow      *event_window;
    gboolean        active;

    GdkEventButton  drag_start_event;
    gboolean        pre_drag;
	EggDockTablabelPrivate *_priv;
};

struct _EggDockTablabelClass {
    GtkBinClass      parent_class;

    void            (*button_pressed_handle)  (EggDockTablabel *tablabel,
                                               GdkEventButton  *event);
	gpointer unused[2];
};

/* public interface */
 
GtkWidget     *egg_dock_tablabel_new           (EggDockItem *item);
GType          egg_dock_tablabel_get_type      (void);

void           egg_dock_tablabel_activate      (EggDockTablabel *tablabel);
void           egg_dock_tablabel_deactivate    (EggDockTablabel *tablabel);

G_END_DECLS

#endif
