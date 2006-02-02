/* eggcellrendererpopup.h
 * Copyright (C) 2002  Anders Carlsson <andersca@gnu.org>
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

#ifndef __EGG_CELL_RENDERER_POPUP_H__
#define __EGG_CELL_RENDERER_POPUP_H__

#include <gtk/gtkcellrenderertext.h>

G_BEGIN_DECLS

#define EGG_TYPE_CELL_RENDERER_POPUP		(egg_cell_renderer_popup_get_type ())
#define EGG_CELL_RENDERER_POPUP(obj)		(EGG_CHECK_CAST ((obj), EGG_TYPE_CELL_RENDERER_POPUP, EggCellRendererPopup))
#define EGG_CELL_RENDERER_POPUP_CLASS(klass)	(EGG_CHECK_CLASS_CAST ((klass), EGG_TYPE_CELL_RENDERER_POPUP, EggCellRendererPopupClass))
#define EGG_IS_CELL_RENDERER_POPUP(obj)		(EGG_CHECK_TYPE ((obj), EGG_TYPE_CELL_RENDERER_POPUP))
#define EGG_IS_CELL_RENDERER_POPUP_CLASS(klass)	(EGG_CHECK_CLASS_TYPE ((klass), EGG_TYPE_CELL_RENDERER_POPUP))
#define EGG_CELL_RENDERER_POPUP_GET_CLASS(obj)   (EGG_CHECK_GET_CLASS ((obj), EGG_TYPE_CELL_RENDERER_POPUP, EggCellRendererPopupClass))

typedef struct _EggCellRendererPopup      EggCellRendererPopup;
typedef struct _EggCellRendererPopupClass EggCellRendererPopupClass;

struct _EggCellRendererPopup
{
  GtkCellRendererText parent;
};

struct _EggCellRendererPopupClass
{
  GtkCellRendererTextClass parent_class;
};

GType            egg_cell_renderer_popup_get_type (void);
GtkCellRenderer *egg_cell_renderer_popup_new      (void);

G_END_DECLS


#endif /* __GTK_CELL_RENDERER_POPUP_H__ */
