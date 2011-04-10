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
#include "egg-cell-renderer-text.h"

static void egg_cell_renderer_text_class_init    
                                   (EggCellRendererTextClass *cell_text_class);
#if GTK_CHECK_VERSION (3,0,0)
static void egg_cell_renderer_text_render (GtkCellRenderer     *cell,
					   cairo_t             *cr,
					   GtkWidget           *widget,
					   const GdkRectangle  *background_area,
					   const GdkRectangle  *cell_area,
					   GtkCellRendererState flags);
#else
static void egg_cell_renderer_text_render     (GtkCellRenderer          *cell,
					       GdkWindow                *window,
					       GtkWidget                *widget,
					       const GdkRectangle       *background_area,
					       const GdkRectangle       *cell_area,
					       const GdkRectangle       *expose_area,
					       GtkCellRendererState      flags);
#endif

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

#if GTK_CHECK_VERSION (3,0,0)
static void
egg_cell_renderer_text_render (GtkCellRenderer     *cell,
			       cairo_t             *cr,
			       GtkWidget           *widget,
			       const GdkRectangle  *background_area,
			       const GdkRectangle  *cell_area,
			       GtkCellRendererState flags)
{
	GtkCellRendererText *celltext = (GtkCellRendererText *) cell;
	GtkStateType state;
	gboolean background_set;

	if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
	{
		if (gtk_widget_has_focus (widget))
			state = GTK_STATE_SELECTED;
		else
			state = GTK_STATE_ACTIVE;
	}
	else
	{
		if (gtk_widget_get_state (widget) == GTK_STATE_INSENSITIVE)
			state = GTK_STATE_INSENSITIVE;
		else
			state = GTK_STATE_NORMAL;
	}

	g_object_get (celltext, "cell-background-set", &background_set, NULL);
	if (state == GTK_STATE_SELECTED && background_set)
	{
		GdkRGBA color;
		guint ypad;

		g_object_get (celltext, "cell-background-rgba", &color,
		                        "ypad", &ypad, NULL);

		gdk_cairo_set_source_rgba (cr, &color);

		cairo_rectangle (cr,
		                 background_area->x,
				 background_area->y + ypad,
				 background_area->width,
				 background_area->height - 2 * ypad);

		cairo_fill (cr);
	}

	GTK_CELL_RENDERER_CLASS (parent_class)->render (cell, cr, widget,
							background_area,
							cell_area, flags);
}
#else /* !GTK_CHECK_VERSION (3,0,0) */
static void
egg_cell_renderer_text_render (GtkCellRenderer     *cell,
			       GdkWindow           *window,
			       GtkWidget           *widget,
			       const GdkRectangle  *background_area,
			       const GdkRectangle  *cell_area,
			       const GdkRectangle  *expose_area,
			       GtkCellRendererState flags)

{
	GtkCellRendererText *celltext = (GtkCellRendererText *) cell;
	GtkStateType state;
	gboolean background_set;
	
	if ((flags & GTK_CELL_RENDERER_SELECTED) == GTK_CELL_RENDERER_SELECTED)
	{
#if GTK_CHECK_VERSION(2,18,0)
		if (gtk_widget_has_focus (widget))
#else
		if (GTK_WIDGET_HAS_FOCUS (widget))
#endif
			state = GTK_STATE_SELECTED;
		else
			state = GTK_STATE_ACTIVE;
	}
	else
	{
#if GTK_CHECK_VERSION(2,20,0)
		if (gtk_widget_get_state (widget) == GTK_STATE_INSENSITIVE)
#else
		if (GTK_WIDGET_STATE (widget) == GTK_STATE_INSENSITIVE)
#endif
			state = GTK_STATE_INSENSITIVE;
		else
			state = GTK_STATE_NORMAL;
	}

	g_object_get (celltext, "cell-background-set", &background_set, NULL);
	if (state == GTK_STATE_SELECTED && background_set)
	{
		GdkColor color;
		cairo_t *cr;
		guint ypad;
		
		g_object_get (celltext, "cell-background-gdk", &color,
		                        "ypad", &ypad, NULL);
		
		cr = gdk_cairo_create (window);

		gdk_cairo_set_source_color (cr, &color);

		cairo_rectangle (cr,
		                 background_area->x,
				 background_area->y + ypad,
				 background_area->width,
				 background_area->height - 2 * ypad);

		cairo_fill (cr);
		cairo_destroy (cr);
	}

	GTK_CELL_RENDERER_CLASS (parent_class)->render (cell, window, widget, background_area,
			       cell_area, expose_area, flags);
}
#endif /* GTK_CHECK_VERSION (3,0,0) */
