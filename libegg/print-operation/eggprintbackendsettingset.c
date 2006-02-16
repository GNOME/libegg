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
  g_ptr_array_foreach (set->array, (GFunc)g_object_unref, NULL);
  g_ptr_array_free (set->array, TRUE);
  
  G_OBJECT_CLASS (egg_print_backend_setting_set_parent_class)->finalize (object);
}

static void
egg_print_backend_setting_set_init (EggPrintBackendSettingSet *set)
{
  set->array = g_ptr_array_new ();
  set->hash = g_hash_table_new (g_str_hash, g_str_equal);
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
egg_print_backend_setting_set_remove (EggPrintBackendSettingSet *set,
				      EggPrintBackendSetting    *setting)
{
  int i;
  
  for (i = 0; i < set->array->len; i++)
    {
      if (g_ptr_array_index (set->array, i) == setting)
	{
	  g_ptr_array_remove_index (set->array, i);
	  g_hash_table_remove (set->hash, setting->name);
	  g_signal_handlers_disconnect_by_func (setting, emit_changed, set);

	  g_object_unref (setting);
	  break;
	}
    }
}

void
egg_print_backend_setting_set_add (EggPrintBackendSettingSet *set,
				   EggPrintBackendSetting    *setting)
{
  g_object_ref (setting);
  
  if (egg_print_backend_setting_set_lookup (set, setting->name))
    egg_print_backend_setting_set_remove (set, setting);
    
  g_ptr_array_add (set->array, setting);
  g_hash_table_insert (set->hash, setting->name, setting);
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



void
egg_print_backend_setting_set_clear_conflicts (EggPrintBackendSettingSet *set)
{
  egg_print_backend_setting_set_foreach (set,
					 (EggPrintBackendSettingSetFunc)egg_print_backend_setting_clear_has_conflict,
					 NULL);
}

GList *
egg_print_backend_setting_set_get_groups (EggPrintBackendSettingSet     *set)
{
  EggPrintBackendSetting *setting;
  GList *list = NULL;
  int i;

  for (i = 0; i < set->array->len; i++)
    {
      setting = g_ptr_array_index (set->array, i);

      if (g_list_find_custom (list, setting->group, (GCompareFunc)strcmp) == NULL)
	list = g_list_prepend (list, g_strdup (setting->group));
    }

  return list;
}

void
egg_print_backend_setting_set_foreach_in_group (EggPrintBackendSettingSet     *set,
						const char                    *group,
						EggPrintBackendSettingSetFunc  func,
						gpointer                       user_data)
{
  EggPrintBackendSetting *setting;
  int i;

  for (i = 0; i < set->array->len; i++)
    {
      setting = g_ptr_array_index (set->array, i);

      if (group == NULL || strcmp (group, setting->group) == 0)
	func (setting, user_data);
    }
}

void
egg_print_backend_setting_set_foreach (EggPrintBackendSettingSet *set,
				       EggPrintBackendSettingSetFunc func,
				       gpointer	    user_data)
{
  egg_print_backend_setting_set_foreach_in_group (set, NULL, func, user_data);
}

