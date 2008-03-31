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

#ifndef __EGG_TOOL_ITEM_GROUP_H__
#define __EGG_TOOL_ITEM_GROUP_H__

#include <gtk/gtkcontainer.h>
#include <gtk/gtktoolitem.h>

G_BEGIN_DECLS

#define EGG_TYPE_TOOL_ITEM_GROUP           (egg_tool_item_group_get_type())
#define EGG_TOOL_ITEM_GROUP(obj)           (G_TYPE_CHECK_INSTANCE_CAST(obj, EGG_TYPE_TOOL_ITEM_GROUP, EggToolItemGroup))
#define EGG_TOOL_ITEM_GROUP_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST(cls, EGG_TYPE_TOOL_ITEM_GROUP, EggToolItemGroupClass))
#define EGG_IS_TOOL_ITEM_GROUP(obj)        (G_TYPE_CHECK_INSTANCE_TYPE(obj, EGG_TYPE_TOOL_ITEM_GROUP))
#define EGG_IS_TOOL_ITEM_GROUP_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE(obj, EGG_TYPE_TOOL_ITEM_GROUP))
#define EGG_TOOL_ITEM_GROUP_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_TOOL_ITEM_GROUP, EggToolItemGroupClass))

typedef struct _EggToolItemGroup        EggToolItemGroup;
typedef struct _EggToolItemGroupClass   EggToolItemGroupClass;
typedef struct _EggToolItemGroupPrivate EggToolItemGroupPrivate;

struct _EggToolItemGroup
{
  GtkContainer parent_instance;
  EggToolItemGroupPrivate *priv;
};

struct _EggToolItemGroupClass
{
  GtkContainerClass parent_class;
};

GType                 egg_tool_item_group_get_type          (void) G_GNUC_CONST;
GtkWidget*            egg_tool_item_group_new               (const gchar        *name);

void                  egg_tool_item_group_set_name          (EggToolItemGroup   *group,
                                                             const gchar        *name);
void                  egg_tool_item_group_set_collapsed      (EggToolItemGroup  *group,
                                                             gboolean            collapsed);
void                  egg_tool_item_group_set_ellipsize     (EggToolItemGroup   *group,
                                                             PangoEllipsizeMode  ellipsize);

G_CONST_RETURN gchar* egg_tool_item_group_get_name          (EggToolItemGroup   *group);
gboolean              egg_tool_item_group_get_collapsed     (EggToolItemGroup   *group);
PangoEllipsizeMode    egg_tool_item_group_get_ellipsize     (EggToolItemGroup   *group);

void                  egg_tool_item_group_insert            (EggToolItemGroup   *group,
                                                             GtkToolItem        *item,
                                                             gint                position);
void                  egg_tool_item_group_set_item_position (EggToolItemGroup   *group,
                                                             GtkToolItem        *item,
                                                             gint                position);
gint                  egg_tool_item_group_get_item_position (EggToolItemGroup   *group,
                                                             GtkToolItem        *item);

guint                 egg_tool_item_group_get_n_items       (EggToolItemGroup   *group);
GtkToolItem*          egg_tool_item_group_get_nth_item      (EggToolItemGroup   *group,
                                                             guint               index);
GtkToolItem*          egg_tool_item_group_get_drop_item     (EggToolItemGroup   *group,
                                                             gint                x,
                                                             gint                y);

G_END_DECLS

#endif /* __EGG_TOOL_ITEM_GROUP_H__ */ 
