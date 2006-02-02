/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* eggtreeviewstate.h
 * Copyright (C) 2001 Anders Carlsson, Jonathan Blanford
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_TREE_VIEW_STATE_H__
#define __EGG_TREE_VIEW_STATE_H__

#include <gtk/gtktreeview.h>

gboolean egg_tree_view_state_apply_from_string (GtkTreeView *tree_view, const gchar *string, GError **error);

void egg_tree_view_state_add_cell_renderer_type (GType type);

#endif /* __EGG_TREE_VIEW_STATE_H__ */
