/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* mp-column-model.h
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

#ifndef __EGG_COLUMN_MODEL_H__
#define __EGG_COLUMN_MODEL_H__

#include <gtk/gtktreeview.h>

typedef struct _EggColumnModel EggColumnModel;
typedef struct _EggColumnModelClass EggColumnModelClass;

struct _EggColumnModel {
	GObject parent_instance;

	GtkTreeView *tree_view;
	GList *columns;
	gint stamp;
};

struct _EggColumnModelClass {
	GObjectClass parent_class;
};

GType egg_column_model_get_type (void);
EggColumnModel *egg_column_model_new (GtkTreeView *tree_view);

void egg_column_model_set_column_visible (EggColumnModel *model, GtkTreeIter *iter, gboolean visible);
gboolean egg_column_model_get_column_visible (EggColumnModel *model, GtkTreeIter *iter);

gboolean egg_column_model_is_column_first (EggColumnModel *model, GtkTreeIter *iter);
gboolean egg_column_model_is_column_last (EggColumnModel *model, GtkTreeIter *iter);

void egg_column_model_move_down_column (EggColumnModel *model, GtkTreeIter *iter);
void egg_column_model_move_up_column (EggColumnModel *model, GtkTreeIter *iter);

#endif /* __EGG_COLUMN_MODEL_H__ */
