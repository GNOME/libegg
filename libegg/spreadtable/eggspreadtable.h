/* 
 * Copyright (C) 2010 Openismus GmbH
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

#ifndef __EGG_SPREAD_TABLE_H__
#define __EGG_SPREAD_TABLE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define EGG_TYPE_SPREAD_TABLE                  (egg_spread_table_get_type ())
#define EGG_SPREAD_TABLE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SPREAD_TABLE, EggSpreadTable))
#define EGG_SPREAD_TABLE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_SPREAD_TABLE, EggSpreadTableClass))
#define EGG_IS_SPREAD_TABLE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SPREAD_TABLE))
#define EGG_IS_SPREAD_TABLE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_SPREAD_TABLE))
#define EGG_SPREAD_TABLE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SPREAD_TABLE, EggSpreadTableClass))

typedef struct _EggSpreadTable            EggSpreadTable;
typedef struct _EggSpreadTablePrivate     EggSpreadTablePrivate;
typedef struct _EggSpreadTableClass       EggSpreadTableClass;


struct _EggSpreadTable
{
  GtkContainer parent_instance;

  /*< private >*/
  EggSpreadTablePrivate *priv;
};

struct _EggSpreadTableClass
{
  GtkContainerClass parent_class;

  gint    (* build_segments_for_size) (EggSpreadTable *table,
				       gint            for_size,
				       gint          **segments);

  void    ( *insert_child)            (EggSpreadTable *table,
				       GtkWidget      *child,
				       gint            index);
};


GType                 egg_spread_table_get_type                  (void) G_GNUC_CONST;


GtkWidget            *egg_spread_table_new                       (GtkOrientation  orientation,
								  guint           lines);

void                  egg_spread_table_insert_child              (EggSpreadTable *table,
								  GtkWidget      *child,
								  gint            index);

void                  egg_spread_table_reorder_child             (EggSpreadTable *table,
								  GtkWidget      *widget,
								  guint           index);

gint                 *egg_spread_table_get_segments              (EggSpreadTable *table);
gint                  egg_spread_table_build_segments_for_size   (EggSpreadTable *table,
								  gint            for_size,
								  gint          **segments);

guint                 egg_spread_table_get_child_line            (EggSpreadTable *table,
								  GtkWidget      *child,
								  gint            size);

void                  egg_spread_table_set_lines                 (EggSpreadTable *table,
								  guint           lines);
guint                 egg_spread_table_get_lines                 (EggSpreadTable *table);

void                  egg_spread_table_set_horizontal_spacing    (EggSpreadTable *table,
								  guint           spacing);
guint                 egg_spread_table_get_horizontal_spacing    (EggSpreadTable *table);

void                  egg_spread_table_set_vertical_spacing      (EggSpreadTable *table,
								  guint           spacing);
guint                 egg_spread_table_get_vertical_spacing      (EggSpreadTable *table);




G_END_DECLS


#endif /* __EGG_SPREAD_TABLE_H__ */
