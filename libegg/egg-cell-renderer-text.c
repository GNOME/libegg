/* egg-cell-renderer-text.c
 * Copyright (C) 2002  Andreas J. Guelzow <aguelzow@taliesin.ca>
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "egg-cell-renderer-text.h"

static void egg_cell_renderer_text_class_init    
                                   (EggCellRendererTextClass *cell_text_class);
static void egg_cell_renderer_text_render     (GtkCellRenderer          *cell,
					       GdkWindow                *window,
					       GtkWidget                *widget,
					       GdkRectangle             *background_area,
					       GdkRectangle             *cell_area,
					       GdkRectangle             *expose_area,
					       guint                     flags);

static GtkCellRendererTextClass *parent_class = NULL;

GType
egg_cell_renderer_text_get_type (void)
{
  static GType cell_text_type = 0;

  if (!cell_text_type)
    {
      static const GTypeInfo cell_text_info =
      {
        sizeof (EggCellRendererTextClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
        (GClassInitFunc)egg_cell_renderer_text_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
        sizeof (GtkCellRendererText),
	0,              /* n_preallocs */
        (GInstanceInitFunc) NULL,
      };

      cell_text_type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, "EggCellRendererText", &cell_text_info, 0);
    }

  return cell_text_type;
}

static void
egg_cell_renderer_text_class_init (EggCellRendererTextClass *class)
{
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS  (class);
	GObjectClass *object_class = G_OBJECT_CLASS (class);

	parent_class = g_type_class_peek_parent (object_class);

	cell_class->render = egg_cell_renderer_text_render;
	
}


GtkCellRenderer *
egg_cell_renderer_text_new (void)
{
  return GTK_CELL_RENDERER (g_object_new (EGG_TYPE_CELL_RENDERER_TEXT, NULL));
}
static void
egg_cell_renderer_text_render (GtkCellRenderer    *cell,
			       GdkWindow          *window,
			       GtkWidget          *widget,
			       GdkRectangle       *background_area,
			       GdkRectangle       *cell_area,
			       GdkRectangle       *expose_area,
			       guint               flags)

{
	GtkCellRendererText *celltext = (GtkCellRendererText *) cell;
	GtkStateType state;
	
	if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
	{
		if (GTK_WIDGET_HAS_FOCUS (widget))
			state = GTK_STATE_SELECTED;
		else
			state = GTK_STATE_ACTIVE;
	}
	else
	{
		if (GTK_WIDGET_STATE (widget) == GTK_STATE_INSENSITIVE)
			state = GTK_STATE_INSENSITIVE;
		else
			state = GTK_STATE_NORMAL;
	}

	if ( state == GTK_STATE_SELECTED  && celltext->background_set)
	{
		GdkColor color;
		GdkGC *gc;
		
		color.red = celltext->background.red;
		color.green = celltext->background.green;
		color.blue = celltext->background.blue;
		
		gc = gdk_gc_new (window);
		
		gdk_gc_set_rgb_fg_color (gc, &color);
		
		gdk_draw_rectangle (window,
				    gc,
				    TRUE,
				    background_area->x,
				    background_area->y + cell->ypad,
				    background_area->width,
				    background_area->height - 2 * cell->ypad);
		
		g_object_unref (G_OBJECT (gc));
	}

	GTK_CELL_RENDERER_CLASS (parent_class)->render (cell, window, widget, background_area,
			       cell_area, expose_area, flags);
}
