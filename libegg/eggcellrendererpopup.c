/* eggcellrendererpopup.c
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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "eggcellrendererpopup.h"

static void             egg_cell_renderer_popup_class_init    (EggCellRendererPopupClass *cell_popup_class);
static GtkCellEditable *egg_cell_renderer_popup_start_editing (GtkCellRenderer          *cell,
							      GdkEvent                 *event,
							      GtkWidget                *widget,
							      const gchar              *path,
							      GdkRectangle             *background_area,
							      GdkRectangle             *cell_area,
							      GtkCellRendererState      flags);


GType
egg_cell_renderer_popup_get_type (void)
{
  static GType cell_popup_type = 0;

  if (!cell_popup_type)
    {
      static const GTypeInfo cell_popup_info =
      {
        sizeof (EggCellRendererPopupClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
        (GClassInitFunc)egg_cell_renderer_popup_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
        sizeof (GtkCellRendererText),
	0,              /* n_preallocs */
        (GInstanceInitFunc) NULL,
      };

      cell_popup_type = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT, "EggCellRendererPopup", &cell_popup_info, 0);
    }

  return cell_popup_type;
}

static void
egg_cell_renderer_popup_init (EggCellRendererPopup *cell_popup)
{
}

static void
egg_cell_renderer_popup_class_init (EggCellRendererPopupClass *cell_popup_class)
{
}


GtkCellRenderer *
egg_cell_renderer_popup_new (void)
{
  return GTK_CELL_RENDERER (g_object_new (EGG_TYPE_CELL_RENDERER_POPUP, NULL));
}
