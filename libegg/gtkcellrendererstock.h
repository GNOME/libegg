/* gtkcellrendererstock.h
 * Copyright (C) 2002  Kristian Rietveld <kris@gtk.org>
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

#ifndef __GTK_CELL_RENDERER_STOCK_H__
#define __GTK_CELL_RENDERER_STOCK_H__

#include <gtk/gtkcellrenderer.h>
#include <gtk/gtkenums.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_CELL_RENDERER_STOCK                   (gtk_cell_renderer_stock_get_type ())
#define GTK_CELL_RENDERER_STOCK(obj)                   (GTK_CHECK_CAST ((obj), GTK_TYPE_CELL_RENDERER_STOCK, GtkCellRendererStock))
#define GTK_CELL_RENDERER_STOCK_CLASS(klass)           (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CELL_RENDERER_STOCK, GtkCellRendererStockClass))
#define GTK_IS_CELL_RENDERER_STOCK(obj)                (GTK_CHECK_TYPE ((obj), GTK_TYPE_CELL_RENDERER_STOCK))
#define GTK_IS_CELL_RENDERER_STOCK_CLASS(klass)        (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CELL_RENDERER_STOCK))
#define GTK_CELL_RENDERER_STOCK_GET_CLASS(obj)         (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_CELL_RENDERER_STOCK, GtkCellRendererStockClass))

typedef struct _GtkCellRendererStock GtkCellRendererStock;
typedef struct _GtkCellRendererStockClass GtkCellRendererStockClass;

struct _GtkCellRendererStock
{
  GtkCellRenderer parent;

  /*< private >*/
  GdkPixbuf *pixbuf;
  gchar *stock_id;
  GtkIconSize size;
  gchar *detail;
};

struct _GtkCellRendererStockClass
{
  GtkCellRendererClass parent_class;
};

GType            gtk_cell_renderer_stock_get_type (void);
GtkCellRenderer *gtk_cell_renderer_stock_new      (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_CELL_RENDERER_STOCK_H__ */
