/* gtkcellrendererstock.c
 * Copyright (C) 2002  Kristian Rietveld <kris@gtk.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * code contains parts of gtkcellrendererpixbuf.c,
 * Copyright (C) 2000  Red Hat, Inc.,  Jonathan Blandford <jrb@redhat.com>
 */

#include "gtkcellrendererstock.h"

static void	gtk_cell_renderer_stock_init		(GtkCellRendererStock      *cellstock);
static void	gtk_cell_renderer_stock_class_init	(GtkCellRendererStockClass *class);
static void	gtk_cell_renderer_stock_get_property	(GObject                   *object,
							 guint                      prop_id,
							 GValue                    *value,
							 GParamSpec                *pspec);
static void	gtk_cell_renderer_stock_set_property	(GObject                   *object,
							 guint                      prop_id,
							 const GValue              *value,
							 GParamSpec                *pspec);
static void	gtk_cell_renderer_stock_create_pixbuf	(GtkCellRendererStock      *cellstock,
							 GtkWidget                 *widget);
static void	gtk_cell_renderer_stock_get_size	(GtkCellRenderer           *cell,
							 GtkWidget                 *widget,
							 GdkRectangle              *cell_area,
							 gint                      *x_offset,
							 gint                      *y_offset,
							 gint                      *width,
							 gint                      *height);
static void	gtk_cell_renderer_stock_render		(GtkCellRenderer	   *cell,
							 GdkWindow                 *window,
							 GtkWidget                 *widget,
							 GdkRectangle              *background_area,
							 GdkRectangle              *cell_area,
							 GdkRectangle              *expose_area,
							 guint                      flags);

enum
{
    PROP_ZERO,
    PROP_STOCK_ID,
    PROP_SIZE,
    PROP_DETAIL
};

GType
gtk_cell_renderer_stock_get_type (void)
{
  static GType cell_stock_type = 0;

  if (!cell_stock_type)
    {
      static const GTypeInfo cell_stock_info =
        {
	  sizeof (GtkCellRendererStockClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) gtk_cell_renderer_stock_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (GtkCellRendererStock),
	  0, /* n_preallocs */
	  (GInstanceInitFunc) gtk_cell_renderer_stock_init
	};

      cell_stock_type = g_type_register_static (GTK_TYPE_CELL_RENDERER, "GtkCellRendererStock", &cell_stock_info, 0);
    }

  return cell_stock_type;
}

static void
gtk_cell_renderer_stock_init (GtkCellRendererStock *cellstock)
{
  cellstock->size = GTK_ICON_SIZE_MENU;
}

static void
gtk_cell_renderer_stock_class_init (GtkCellRendererStockClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS (class);

  object_class->get_property = gtk_cell_renderer_stock_get_property;
  object_class->set_property = gtk_cell_renderer_stock_set_property;

  cell_class->get_size = gtk_cell_renderer_stock_get_size;
  cell_class->render = gtk_cell_renderer_stock_render;

  /* FIXME: the following property thingies need translation stuff once
   * there are in GTK+
   */
  g_object_class_install_property (object_class,
				   PROP_STOCK_ID,
				   g_param_spec_string ("stock_id",
						        "Stock ID",
						        "The stock ID of the stock icon to render",
						        NULL,
						        G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
				   PROP_SIZE,
				   g_param_spec_enum ("size",
						      "Size",
						      "The size of the rendered icon",
						      GTK_TYPE_ICON_SIZE,
						      GTK_ICON_SIZE_MENU,
						      G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
				   PROP_DETAIL,
				   g_param_spec_string ("detail",
							"Detail",
							"Render detail to pass to the theme engine",
							NULL,
							G_PARAM_READWRITE));
}

static void
gtk_cell_renderer_stock_get_property (GObject    *object,
				      guint       prop_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
  GtkCellRendererStock *cellstock = GTK_CELL_RENDERER_STOCK (object);

  switch (prop_id)
    {
      case PROP_STOCK_ID:
	g_value_set_string (value, cellstock->stock_id);
	break;

      case PROP_SIZE:
	g_value_set_enum (value, cellstock->size);
	break;

      case PROP_DETAIL:
	g_value_set_string (value, cellstock->detail);
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
gtk_cell_renderer_stock_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  GtkCellRendererStock *cellstock = GTK_CELL_RENDERER_STOCK (object);

  switch (prop_id)
    {
      case PROP_STOCK_ID:
	if (cellstock->stock_id)
	  g_free (cellstock->stock_id);
	cellstock->stock_id = g_strdup (g_value_get_string (value));
	g_object_notify (G_OBJECT (object), "stock_id");
	break;

      case PROP_SIZE:
	cellstock->size = g_value_get_enum (value);
	g_object_notify (G_OBJECT (object), "size");
	break;

      case PROP_DETAIL:
	if (cellstock->detail)
	  g_free (cellstock->detail);
	cellstock->detail = g_strdup (g_value_get_string (value));
	g_object_notify (G_OBJECT (object), "detail");
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	return;
    }

  if (cellstock->pixbuf)
    {
      g_object_unref (cellstock->pixbuf);
      cellstock->pixbuf = NULL;
    }
}

GtkCellRenderer *
gtk_cell_renderer_stock_new (void)
{
  return GTK_CELL_RENDERER (g_object_new (gtk_cell_renderer_stock_get_type (), NULL));
}

static void
gtk_cell_renderer_stock_create_pixbuf (GtkCellRendererStock *cellstock,
                                       GtkWidget            *widget)
{
  if (cellstock->pixbuf)
    g_object_unref (G_OBJECT (cellstock->pixbuf));

  cellstock->pixbuf = gtk_widget_render_icon (widget,
      					      cellstock->stock_id,
					      cellstock->size,
					      cellstock->detail);
}

static void
gtk_cell_renderer_stock_get_size (GtkCellRenderer *cell,
    				  GtkWidget       *widget,
				  GdkRectangle    *cell_area,
				  gint            *x_offset,
				  gint            *y_offset,
				  gint            *width,
				  gint            *height)
{
  GtkCellRendererStock *cellstock = (GtkCellRendererStock *) cell;
  gint pixbuf_width = 0;
  gint pixbuf_height = 0;
  gint calc_width;
  gint calc_height;

  if (!cellstock->pixbuf)
    gtk_cell_renderer_stock_create_pixbuf (cellstock, widget);

  pixbuf_width = gdk_pixbuf_get_width (cellstock->pixbuf);
  pixbuf_height = gdk_pixbuf_get_height (cellstock->pixbuf);

  calc_width = (gint) GTK_CELL_RENDERER (cellstock)->xpad * 2 + pixbuf_width;
  calc_height = (gint) GTK_CELL_RENDERER (cellstock)->ypad * 2 + pixbuf_height;

  if (x_offset) *x_offset = 0;
  if (y_offset) *y_offset = 0;

  if (cell_area && pixbuf_width > 0 && pixbuf_height > 0)
    {
      if (x_offset)
        {
          *x_offset = GTK_CELL_RENDERER (cellstock)->xalign * (cell_area->width - calc_width - (2 * GTK_CELL_RENDERER (cellstock)->xpad));
          *x_offset = MAX (*x_offset, 0) + GTK_CELL_RENDERER (cellstock)->xpad;
	}
      if (y_offset)
        {
          *y_offset = GTK_CELL_RENDERER (cellstock)->yalign * (cell_area->height - calc_height - (2 * GTK_CELL_RENDERER (cellstock)->ypad));
          *y_offset = MAX (*y_offset, 0) + GTK_CELL_RENDERER (cellstock)->ypad;
	}
    }

  if (calc_width)
    *width = calc_width;
  
  if (height)
    *height = calc_height;
}

static void
gtk_cell_renderer_stock_render (GtkCellRenderer      *cell,
				GdkWindow            *window,
				GtkWidget            *widget,
				GdkRectangle         *background_area,
				GdkRectangle         *cell_area,
				GdkRectangle         *expose_area,
				guint                 flags)
{
  GtkCellRendererStock *cellstock = GTK_CELL_RENDERER_STOCK (cell);
  GdkRectangle pix_rect;
  GdkRectangle draw_rect;

  gtk_cell_renderer_stock_get_size (cell, widget, cell_area,
      				    &pix_rect.x,
      				    &pix_rect.y,
      				    &pix_rect.width,
      				    &pix_rect.height);

  pix_rect.x += cell_area->x;
  pix_rect.y += cell_area->y;
  pix_rect.width -= cell->xpad * 2;
  pix_rect.height -= cell->ypad * 2;

  if (gdk_rectangle_intersect (cell_area, &pix_rect, &draw_rect))
    gdk_pixbuf_render_to_drawable_alpha (cellstock->pixbuf,
                                         window,
                                         /* pixbuf 0, 0 is at pix_rect.x, pix_rect.y */
                                         draw_rect.x - pix_rect.x,
                                         draw_rect.y - pix_rect.y,
                                         draw_rect.x,
                                         draw_rect.y,
                                         draw_rect.width,
                                         draw_rect.height,
                                         GDK_PIXBUF_ALPHA_FULL,
                                         0,
                                         GDK_RGB_DITHER_NORMAL,
                                         0, 0);
}
