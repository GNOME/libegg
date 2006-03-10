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

#include "eggprinteroptionset.h"

/*****************************************
 *         EggPrinterOptionSet    *
 *****************************************/

enum {
  CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EggPrinterOptionSet, egg_printer_option_set, G_TYPE_OBJECT)

static void
egg_printer_option_set_finalize (GObject *object)
{
  EggPrinterOptionSet *set = EGG_PRINTER_OPTION_SET (object);

  g_hash_table_destroy (set->hash);
  g_ptr_array_foreach (set->array, (GFunc)g_object_unref, NULL);
  g_ptr_array_free (set->array, TRUE);
  
  G_OBJECT_CLASS (egg_printer_option_set_parent_class)->finalize (object);
}

static void
egg_printer_option_set_init (EggPrinterOptionSet *set)
{
  set->array = g_ptr_array_new ();
  set->hash = g_hash_table_new (g_str_hash, g_str_equal);
}

static void
egg_printer_option_set_class_init (EggPrinterOptionSetClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_printer_option_set_finalize;

  signals[CHANGED] =
    g_signal_new ("changed",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrinterOptionSetClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}


static void
emit_changed (EggPrinterOptionSet *set)
{
  g_signal_emit (set, signals[CHANGED], 0);
}

EggPrinterOptionSet *
egg_printer_option_set_new (void)
{
  return g_object_new (EGG_TYPE_PRINTER_OPTION_SET, NULL);
}

void
egg_printer_option_set_remove (EggPrinterOptionSet *set,
			       EggPrinterOption    *option)
{
  int i;
  
  for (i = 0; i < set->array->len; i++)
    {
      if (g_ptr_array_index (set->array, i) == option)
	{
	  g_ptr_array_remove_index (set->array, i);
	  g_hash_table_remove (set->hash, option->name);
	  g_signal_handlers_disconnect_by_func (option, emit_changed, set);

	  g_object_unref (option);
	  break;
	}
    }
}

void
egg_printer_option_set_add (EggPrinterOptionSet *set,
			    EggPrinterOption    *option)
{
  g_object_ref (option);
  
  if (egg_printer_option_set_lookup (set, option->name))
    egg_printer_option_set_remove (set, option);
    
  g_ptr_array_add (set->array, option);
  g_hash_table_insert (set->hash, option->name, option);
  g_signal_connect_object (option, "changed", G_CALLBACK (emit_changed), set, G_CONNECT_SWAPPED);
}

EggPrinterOption *
egg_printer_option_set_lookup (EggPrinterOptionSet *set,
			       const char          *name)
{
  gpointer ptr;

  ptr = g_hash_table_lookup (set->hash, name);

  return EGG_PRINTER_OPTION (ptr);
}

void
egg_printer_option_set_clear_conflicts (EggPrinterOptionSet *set)
{
  egg_printer_option_set_foreach (set,
				  (EggPrinterOptionSetFunc)egg_printer_option_clear_has_conflict,
				  NULL);
}

static int
safe_strcmp (const char *a, const char *b)
{
  if (a == NULL)
    a = "";
  if (b == NULL)
    b = "";

  return strcmp (a, b);
}

GList *
egg_printer_option_set_get_groups (EggPrinterOptionSet *set)
{
  EggPrinterOption *option;
  GList *list = NULL;
  int i;

  for (i = 0; i < set->array->len; i++)
    {
      option = g_ptr_array_index (set->array, i);

      if (g_list_find_custom (list, option->group, (GCompareFunc)safe_strcmp) == NULL)
	list = g_list_prepend (list, g_strdup (option->group));
    }

  return g_list_reverse (list);
}

void
egg_printer_option_set_foreach_in_group (EggPrinterOptionSet     *set,
					 const char              *group,
					 EggPrinterOptionSetFunc  func,
					 gpointer                 user_data)
{
  EggPrinterOption *option;
  int i;

  for (i = 0; i < set->array->len; i++)
    {
      option = g_ptr_array_index (set->array, i);

      if (group == NULL ||
	  (option->group != NULL && strcmp (group, option->group) == 0))
	func (option, user_data);
    }
}

void
egg_printer_option_set_foreach (EggPrinterOptionSet *set,
				EggPrinterOptionSetFunc func,
				gpointer user_data)
{
  egg_printer_option_set_foreach_in_group (set, NULL, func, user_data);
}

