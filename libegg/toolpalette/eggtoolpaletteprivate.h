/* EggToolPalette -- A tool palette with categories and DnD support
 * Copyright (C) 2008  Openismus GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *      Mathias Hasselmann
 */

#ifndef __EGG_TOOL_PALETTE_PRIVATE_H__
#define __EGG_TOOL_PALETTE_PRIVATE_H__

#include "eggtoolpalette.h"
#include "eggtoolitemgroup.h"
#include <gtk/gtk.h>

void _egg_tool_palette_get_item_size           (EggToolPalette   *palette,
                                                GtkRequisition   *item_size,
                                                gboolean          homogeneous_only,
                                                gint             *requested_rows);
void _egg_tool_palette_child_set_drag_source   (GtkWidget        *widget,
                                                gpointer          data);
void _egg_tool_palette_set_expanding_child     (EggToolPalette   *palette,
                                                GtkWidget        *widget);

void _egg_tool_item_group_palette_reconfigured (EggToolItemGroup *group);
void _egg_tool_item_group_item_size_request    (EggToolItemGroup *group,
                                                GtkRequisition   *item_size,
                                                gboolean          homogeneous_only,
                                                gint             *requested_rows);
gint _egg_tool_item_group_get_height_for_width (EggToolItemGroup *group,
                                                gint              width);
gint _egg_tool_item_group_get_width_for_height (EggToolItemGroup *group,
                                                gint              height);
void _egg_tool_item_group_paint                (EggToolItemGroup *group,
                                                cairo_t          *cr);
gint _egg_tool_item_group_get_size_for_limit   (EggToolItemGroup *group,
                                                gint              limit,
                                                gboolean          vertical,
                                                gboolean          animation);

#undef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
/* #define HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090 */

#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
GtkSizeGroup *_egg_tool_palette_get_size_group (EggToolPalette   *palette);
#endif

#endif /* __EGG_TOOL_PALETTE_PRIVATE_H__ */
