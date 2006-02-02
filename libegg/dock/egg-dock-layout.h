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


#ifndef __EGG_DOCK_LAYOUT_H__
#define __EGG_DOCK_LAYOUT_H__

#include <glib-object.h>
#include <dock/egg-dock-master.h>
#include <dock/egg-dock.h>

G_BEGIN_DECLS

/* standard macros */
#define	EGG_TYPE_DOCK_LAYOUT		  (egg_dock_layout_get_type ())
#define EGG_DOCK_LAYOUT(object)		  (GTK_CHECK_CAST ((object), EGG_TYPE_DOCK_LAYOUT, EggDockLayout))
#define EGG_DOCK_LAYOUT_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_LAYOUT, EggDockLayoutClass))
#define EGG_IS_DOCK_LAYOUT(object)	  (GTK_CHECK_TYPE ((object), EGG_TYPE_DOCK_LAYOUT))
#define EGG_IS_DOCK_LAYOUT_CLASS(klass)	  (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_LAYOUT))
#define	EGG_DOCK_LAYOUT_GET_CLASS(object) (GTK_CHECK_GET_CLASS ((object), EGG_TYPE_DOCK_LAYOUT, EggDockLayoutClass))

/* data types & structures */
typedef struct _EggDockLayout EggDockLayout;
typedef struct _EggDockLayoutClass EggDockLayoutClass;
typedef struct _EggDockLayoutPrivate EggDockLayoutPrivate;

struct _EggDockLayout {
    GObject               g_object;

    gboolean              dirty;
    EggDockMaster        *master;

    EggDockLayoutPrivate *_priv;
};

struct _EggDockLayoutClass {
    GObjectClass  g_object_class;
	gpointer unused[2]; /* Future expansion without ABI breakage */
};


/* public interface */
 
GType            egg_dock_layout_get_type       (void);

EggDockLayout   *egg_dock_layout_new            (EggDock       *dock);

void             egg_dock_layout_attach         (EggDockLayout *layout,
                                                 EggDockMaster *master);

gboolean         egg_dock_layout_load_layout    (EggDockLayout *layout,
                                                 const gchar   *name);

void             egg_dock_layout_save_layout    (EggDockLayout *layout,
                                                 const gchar   *name);

void             egg_dock_layout_delete_layout  (EggDockLayout *layout,
                                                 const gchar   *name);

GList           *egg_dock_layout_get_layouts    (EggDockLayout *layout,
                                                 gboolean       include_default);

void             egg_dock_layout_run_manager    (EggDockLayout *layout);

gboolean         egg_dock_layout_load_from_file (EggDockLayout *layout,
                                                 const gchar   *filename);

gboolean         egg_dock_layout_save_to_file   (EggDockLayout *layout,
                                                 const gchar   *filename);

gboolean         egg_dock_layout_is_dirty       (EggDockLayout *layout);

GtkWidget       *egg_dock_layout_get_ui         (EggDockLayout *layout);
GtkWidget       *egg_dock_layout_get_items_ui   (EggDockLayout *layout);
GtkWidget       *egg_dock_layout_get_layouts_ui (EggDockLayout *layout);

G_END_DECLS

#endif
