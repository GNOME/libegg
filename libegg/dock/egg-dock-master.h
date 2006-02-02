/*
 * egg-dock-master.h - Object which manages a dock ring
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

#ifndef __EGG_DOCK_MASTER_H__
#define __EGG_DOCK_MASTER_H__

#include <glib-object.h>
#include <gtk/gtktypeutils.h>
#include <dock/egg-dock-object.h>


G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK_MASTER             (egg_dock_master_get_type ())
#define EGG_DOCK_MASTER(obj)             (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_MASTER, EggDockMaster))
#define EGG_DOCK_MASTER_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_MASTER, EggDockMasterClass))
#define EGG_IS_DOCK_MASTER(obj)          (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_MASTER))
#define EGG_IS_DOCK_MASTER_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_MASTER))
#define EGG_DOCK_MASTER_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_MASTER, EggDockMasterClass))

/* data types & structures */
typedef struct _EggDockMaster        EggDockMaster;
typedef struct _EggDockMasterClass   EggDockMasterClass;
typedef struct _EggDockMasterPrivate EggDockMasterPrivate;

struct _EggDockMaster {
    GObject               object;

    GHashTable           *dock_objects;
    GList                *toplevel_docks;
    EggDockObject        *controller;      /* GUI root object */
    
    gint                  dock_number;     /* for toplevel dock numbering */
    
    EggDockMasterPrivate *_priv;
};

struct _EggDockMasterClass {
    GObjectClass parent_class;

    void (* layout_changed) (EggDockMaster *master);
	gpointer unused[2]; /* Future expansion without ABI breakage */
};

/* additional macros */

#define EGG_DOCK_OBJECT_GET_MASTER(object) \
    (EGG_DOCK_OBJECT (object)->master ? \
        EGG_DOCK_MASTER (EGG_DOCK_OBJECT (object)->master) : NULL)

/* public interface */
 
GType          egg_dock_master_get_type         (void);

void           egg_dock_master_add              (EggDockMaster *master,
                                                 EggDockObject *object);
void           egg_dock_master_remove           (EggDockMaster *master,
                                                 EggDockObject *object);
void           egg_dock_master_foreach          (EggDockMaster *master,
                                                 GFunc          function,
                                                 gpointer       user_data);

void           egg_dock_master_foreach_toplevel (EggDockMaster *master,
                                                 gboolean       include_controller,
                                                 GFunc          function,
                                                 gpointer       user_data);

EggDockObject *egg_dock_master_get_object       (EggDockMaster *master,
                                                 const gchar   *nick_name);

EggDockObject *egg_dock_master_get_controller   (EggDockMaster *master);
void           egg_dock_master_set_controller   (EggDockMaster *master,
                                                 EggDockObject *new_controller);

G_END_DECLS

#endif /* __EGG_DOCK_MASTER_H__ */
