/* eggcellrendererpixbuf.h
 * Copyright (C) 2000  Red Hat, Inc.,  Jonathan Blandford <jrb@redhat.com>
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

#ifndef __EGG_CELL_RENDERER_PIXBUF_H__
#define __EGG_CELL_RENDERER_PIXBUF_H__

#include <gtk/gtkcellrenderer.h>
#include <gtk/gtkenums.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_CELL_RENDERER_PIXBUF			(egg_cell_renderer_pixbuf_get_type ())
#define EGG_CELL_RENDERER_PIXBUF(obj)			(GTK_CHECK_CAST ((obj), GTK_TYPE_CELL_RENDERER_PIXBUF, EggCellRendererPixbuf))
#define EGG_CELL_RENDERER_PIXBUF_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CELL_RENDERER_PIXBUF, EggCellRendererPixbufClass))
#define GTK_IS_CELL_RENDERER_PIXBUF(obj)		(GTK_CHECK_TYPE ((obj), GTK_TYPE_CELL_RENDERER_PIXBUF))
#define GTK_IS_CELL_RENDERER_PIXBUF_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CELL_RENDERER_PIXBUF))
#define EGG_CELL_RENDERER_PIXBUF_GET_CLASS(obj)         (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_CELL_RENDERER_PIXBUF, EggCellRendererPixbufClass))

typedef struct _EggCellRendererPixbuf EggCellRendererPixbuf;
typedef struct _EggCellRendererPixbufClass EggCellRendererPixbufClass;

struct _EggCellRendererPixbuf
{
  GtkCellRenderer parent;

  GdkPixbuf *pixbuf;
  GdkPixbuf *pixbuf_expander_open;
  GdkPixbuf *pixbuf_expander_closed;

  gchar *stock_id;
  GtkIconSize stock_size;
  gchar *stock_detail;
};

struct _EggCellRendererPixbufClass
{
  GtkCellRendererClass parent_class;

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
};

GtkType          egg_cell_renderer_pixbuf_get_type (void);
GtkCellRenderer *egg_cell_renderer_pixbuf_new      (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EGG_CELL_RENDERER_PIXBUF_H__ */
