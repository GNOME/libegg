/* GTK - The GIMP Toolkit
 * eggprintbackend.h: Abstract printer backend interfaces
 * Copyright (C) 2006, Red Hat, Inc.
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

#include <config.h>
#include <gmodule.h>
#include <glib.h>
#include <string.h>

#include "eggprintbackendsettingset.h"

/*****************************************
 *             EggPrintBackendSettingSet    *
 *****************************************/

enum {
  CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EggPrintBackendSettingSet, egg_print_backend_setting_set, G_TYPE_OBJECT)

static void
egg_print_backend_setting_set_finalize (GObject *object)
{
  EggPrintBackendSettingSet *set = EGG_PRINT_BACKEND_SETTING_SET (object);

  g_hash_table_destroy (set->hash);
  
  G_OBJECT_CLASS (egg_print_backend_setting_set_parent_class)->finalize (object);
}

static void
egg_print_backend_setting_set_init (EggPrintBackendSettingSet *set)
{
  set->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
				     g_free, g_object_unref);
}

static void
egg_print_backend_setting_set_class_init (EggPrintBackendSettingSetClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_backend_setting_set_finalize;

  signals[CHANGED] =
    g_signal_new ("changed",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintBackendSettingSetClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}


static void
emit_changed (EggPrintBackendSettingSet *set)
{
  g_signal_emit (set, signals[CHANGED], 0);
}

EggPrintBackendSettingSet *
egg_print_backend_setting_set_new (void)
{
  return g_object_new (EGG_TYPE_PRINT_BACKEND_SETTING_SET, NULL);
}

void
egg_print_backend_setting_set_add (EggPrintBackendSettingSet *set,
				   EggPrintBackendSetting    *setting)
{
  g_hash_table_insert (set->hash, g_strdup (setting->name), g_object_ref (setting));
  g_signal_connect_object (setting, "changed", G_CALLBACK (emit_changed), set, G_CONNECT_SWAPPED);
}

EggPrintBackendSetting *
egg_print_backend_setting_set_lookup (EggPrintBackendSettingSet *set,
				      const char                *name)
{
  gpointer ptr;

  ptr = g_hash_table_lookup (set->hash, name);

  return EGG_PRINT_BACKEND_SETTING (ptr);
}

struct SettingSetForeach {
  EggPrintBackendSettingSetFunc func;
  const char *group;
  gpointer user_data;
};

static void
hash_foreach (gpointer       key,
	      gpointer       value,
	      gpointer       user_data)
{
  struct SettingSetForeach *data = user_data;
  EggPrintBackendSetting *setting;

  setting = EGG_PRINT_BACKEND_SETTING (value);
  if (data->group == NULL || strcmp (data->group, setting->group) == 0)
    data->func (setting, data->user_data);
}

void
egg_print_backend_setting_set_foreach (EggPrintBackendSettingSet *set,
				       EggPrintBackendSettingSetFunc func,
				       gpointer	    user_data)
{
  struct SettingSetForeach data;
  data.func = func;
  data.user_data =  user_data;
  data.group = NULL;
  
  g_hash_table_foreach (set->hash, hash_foreach, &data);
}



void
egg_print_backend_setting_set_clear_conflicts (EggPrintBackendSettingSet *set)
{
  egg_print_backend_setting_set_foreach (set,
					 (EggPrintBackendSettingSetFunc)egg_print_backend_setting_clear_has_conflict,
					 NULL);
}

static void
get_groups (EggPrintBackendSetting *setting, GList **list)
{
  if (g_list_find_custom (*list, setting->group, strcmp) != NULL)
    return;

  *list = g_list_prepend (*list, g_strdup (setting->group));
}
  


GList *
egg_print_backend_setting_set_get_groups (EggPrintBackendSettingSet     *set)
{
  GList *list = NULL;
  
  egg_print_backend_setting_set_foreach (set,
					 (EggPrintBackendSettingSetFunc)get_groups,
					 &list);

  return list;
}

void
egg_print_backend_setting_set_foreach_in_group (EggPrintBackendSettingSet     *set,
						const char                    *group,
						EggPrintBackendSettingSetFunc  func,
						gpointer                       user_data)
{
  struct SettingSetForeach data;
  data.func = func;
  data.user_data =  user_data;
  data.group = group;
  
  g_hash_table_foreach (set->hash, hash_foreach, &data);
}

