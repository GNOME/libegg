/* EggEnumAction -- An action that creates combo boxes for enums
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

#ifndef __EGG_ENUM_ACTION_H__
#define __EGG_ENUM_ACTION_H__

#include <gtk/gtkaction.h>

G_BEGIN_DECLS

#define EGG_TYPE_ENUM_ACTION           (egg_enum_action_get_type())
#define EGG_ENUM_ACTION(obj)           (G_TYPE_CHECK_INSTANCE_CAST(obj, EGG_TYPE_ENUM_ACTION, EggEnumAction))
#define EGG_ENUM_ACTION_CLASS(cls)     (G_TYPE_CHECK_CLASS_CAST(cls, EGG_TYPE_ENUM_ACTION, EggEnumActionClass))
#define EGG_IS_ENUM_ACTION(obj)        (G_TYPE_CHECK_INSTANCE_TYPE(obj, EGG_TYPE_ENUM_ACTION))
#define EGG_IS_ENUM_ACTION_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE(obj, EGG_TYPE_ENUM_ACTION))
#define EGG_ENUM_ACTION_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), EGG_TYPE_ENUM_ACTION, EggEnumActionClass))

typedef struct _EggEnumAction        EggEnumAction;
typedef struct _EggEnumActionClass   EggEnumActionClass;
typedef struct _EggEnumActionPrivate EggEnumActionPrivate;

typedef void     (*EggEnumActionCallback)   (GEnumValue *enum_value,
                                             gpointer    user_data);
typedef gboolean (*EggEnumActionFilterFunc) (GEnumValue *enum_value,
                                             gpointer    user_data);

struct _EggEnumAction
{
  GtkAction parent_instance;
  EggEnumActionPrivate *priv;
};

struct _EggEnumActionClass
{
  GtkActionClass parent_class;
};

GType      egg_enum_action_get_type   (void) G_GNUC_CONST;
GtkAction* egg_enum_action_new        (const gchar             *name,
                                       const gchar             *label,
                                       const gchar             *tooltip,
                                       GType                    enum_type);

void       egg_enum_action_bind       (EggEnumAction           *action,
                                       GObject                 *object,
                                       const gchar             *property_name);
void       egg_enum_action_connect    (EggEnumAction           *action,
                                       EggEnumActionCallback    callback,
                                       gpointer                 data);

void       egg_enum_action_set_filter (EggEnumAction           *action,
                                       EggEnumActionFilterFunc  filter,
                                       gpointer                 user_data,
                                       GDestroyNotify           destroy_data);

G_END_DECLS

#endif /* __EGG_ENUM_ACTION_H__ */ 
