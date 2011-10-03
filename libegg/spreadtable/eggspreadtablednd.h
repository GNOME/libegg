/* 
 * Copyright (C) 2011 Openismus GmbH
 *
 * Authors:
 *      Tristan Van Berkom <tristanvb@openismus.com>
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

#ifndef __EGG_SPREAD_TABLE_DND_H__
#define __EGG_SPREAD_TABLE_DND_H__

#include <gtk/gtk.h>
#include "eggspreadtable.h"

G_BEGIN_DECLS


#define EGG_TYPE_SPREAD_TABLE_DND            (egg_spread_table_dnd_get_type ())
#define EGG_SPREAD_TABLE_DND(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SPREAD_TABLE_DND, EggSpreadTableDnd))
#define EGG_SPREAD_TABLE_DND_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_SPREAD_TABLE_DND, EggSpreadTableDndClass))
#define EGG_IS_SPREAD_TABLE_DND(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SPREAD_TABLE_DND))
#define EGG_IS_SPREAD_TABLE_DND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_SPREAD_TABLE_DND))
#define EGG_SPREAD_TABLE_DND_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SPREAD_TABLE_DND, EggSpreadTableDndClass))

typedef struct _EggSpreadTableDnd            EggSpreadTableDnd;
typedef struct _EggSpreadTableDndPrivate     EggSpreadTableDndPrivate;
typedef struct _EggSpreadTableDndClass       EggSpreadTableDndClass;

/**
 * EggDragEnableMode:
 * @EGG_DRAG_DISABLED: Children cannot be dragged.
 * @EGG_DRAG_ENABLED:  Children can be dragged if the needed mouse events 
 *                     are not handled by those said children.
 * @EGG_DRAG_FULL:     All mouse events on children are consumed for the
 *                     purpose of starting drag and drop operations on children.
 *
 * Constants that control whether child widgets can be dragged from
 * an #EggSpreadTableDnd.
 *
 */
typedef enum {
  EGG_DRAG_DISABLED = 0,
  EGG_DRAG_ENABLED,
  EGG_DRAG_FULL
} EggDragEnableMode;

/**
 * EggSpreadTableDnd:
 */
struct _EggSpreadTableDnd
{
  /*< private >*/
  EggSpreadTable parent_instance;
  EggSpreadTableDndPrivate *priv;
};

/**
 * EggSpreadTableDndClass:
 * @widget_drop_possible: A signal to determine whether @widget can be dropped into @table
 */
struct _EggSpreadTableDndClass
{
  /*< private >*/
  EggSpreadTableClass parent_class;

  /*< public >*/
  gboolean  (* widget_drop_possible) (EggSpreadTableDnd *table,
				      GtkWidget         *widget,
				      gboolean          *drop_possible);
};

GType                 egg_spread_table_dnd_get_type              (void) G_GNUC_CONST;
GtkWidget            *egg_spread_table_dnd_new                   (GtkOrientation     orientation,
								  guint              lines);

void                  egg_spread_table_dnd_insert_child          (EggSpreadTableDnd *table,
								  GtkWidget         *child,
								  gint               index);
void                  egg_spread_table_dnd_remove_child          (EggSpreadTableDnd *table,
								  GtkWidget         *child);

void                  egg_spread_table_dnd_set_drag_enabled      (EggSpreadTableDnd *table,
								  EggDragEnableMode  drag_enabled);
EggDragEnableMode     egg_spread_table_dnd_get_drag_enabled      (EggSpreadTableDnd *table);
void                  egg_spread_table_dnd_set_drop_enabled      (EggSpreadTableDnd *table,
								  gboolean           drop_enabled);
gboolean              egg_spread_table_dnd_get_drop_enabled      (EggSpreadTableDnd *table);

G_END_DECLS


#endif /* __EGG_SPREAD_TABLE_H__ */
