/* egg-cell-renderer-text.h
 * Copyright (C) 2002  Andreas J. Guelzow <aguelzow@taliesin.ca>
 *
 * A version of gtk-cell-renderer-text useful for the times when we 
 * use colored text and/or colored backgrounds 
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

#ifndef __EGG_CELL_RENDERER_TEXT_H__
#define __EGG_CELL_RENDERER_TEXT_H__

#include <gtk/gtkcellrenderertext.h>

G_BEGIN_DECLS

#define EGG_TYPE_CELL_RENDERER_TEXT		(egg_cell_renderer_text_get_type ())
#define EGG_CELL_RENDERER_TEXT(obj)		(GTK_CHECK_CAST ((obj), EGG_TYPE_CELL_RENDERER_TEXT, EggCellRendererText))
#define EGG_CELL_RENDERER_TEXT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_CELL_RENDERER_TEXT, EggCellRendererTextClass))
#define EGG_IS_CELL_RENDERER_TEXT(obj)		(GTK_CHECK_TYPE ((obj), EGG_TYPE_CELL_RENDERER_TEXT))
#define EGG_IS_CELL_RENDERER_TEXT_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_CELL_RENDERER_TEXT))
#define EGG_CELL_RENDERER_TEXT_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), EGG_TYPE_CELL_RENDERER_TEXT, EggCellRendererTextClass))

typedef struct _EggCellRendererText      EggCellRendererText;
typedef struct _EggCellRendererTextClass EggCellRendererTextClass;

struct _EggCellRendererText
{
  GtkCellRendererText parent;
};

struct _EggCellRendererTextClass
{
  GtkCellRendererTextClass parent_class;
};

GType            egg_cell_renderer_text_get_type (void);
GtkCellRenderer *egg_cell_renderer_text_new      (void);

G_END_DECLS


#endif /* __GTK_CELL_RENDERER_TEXT_H__ */
