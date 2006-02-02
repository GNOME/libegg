/* GTK - The GIMP Toolkit
 * eggprintsettings.c: Print Settings
 * Copyright (C) 2005, Red Hat, Inc.
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

#define GTK_PARAM_READWRITE G_PARAM_READWRITE
#define P_(s) s

#include "eggprintsettings.h"
#include <string.h>

typedef struct _EggPrintSettingsClass EggPrintSettingsClass;

#define EGG_IS_PRINT_SETTINGS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_SETTINGS))
#define EGG_PRINT_SETTINGS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_SETTINGS, EggPrintSettingsClass))
#define EGG_PRINT_SETTINGS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_SETTINGS, EggPrintSettingsClass))

static int egg_print_settings_n_properties = 0;

struct _EggPrintSettings
{
  GObject parent_instance;

  GHashTable *properties;
};

struct _EggPrintSettingsClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (EggPrintSettings, egg_print_settings, G_TYPE_OBJECT)

static void
egg_print_settings_finalize (GObject *object)
{
  G_OBJECT_CLASS (egg_print_settings_parent_class)->finalize (object);
}

static void
egg_print_settings_set_property (GObject        *object,
				 guint           property_id,
				 const GValue   *value,
				 GParamSpec     *pspec)
{
  EggPrintSettings *settings = EGG_PRINT_SETTINGS (object);
  GValue *new_value;

  new_value = g_new (GValue, 1);
  memset (new_value, 0, sizeof (GValue));

  g_value_init (new_value, G_VALUE_TYPE (value));
  g_value_copy (value, new_value);

  g_hash_table_insert (settings->properties,
		       GUINT_TO_POINTER (property_id),
		       new_value);
}

static void
egg_print_settings_get_property (GObject        *object,
				 guint           property_id,
				 GValue         *value,
				 GParamSpec     *pspec)
{
  EggPrintSettings *settings = EGG_PRINT_SETTINGS (object);
  GValue *stored_value = g_hash_table_lookup (settings->properties,
					      GUINT_TO_POINTER (property_id));

  if (stored_value)
    g_value_copy (stored_value, value);
  else
    g_param_value_set_default (pspec, value);
}

static void
egg_print_settings_class_init (EggPrintSettingsClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_settings_finalize;
  gobject_class->set_property = egg_print_settings_set_property;
  gobject_class->get_property = egg_print_settings_get_property;

  egg_print_settings_install_property (g_param_spec_string ("output-filename",
							    P_("Output Filename"),
							    P_("Filename of output file to create when printing to a file"),
							    NULL,
							    GTK_PARAM_READWRITE));
}

static void
free_value (gpointer data)
{
  GValue *value = data;

  g_value_unset (value);
  g_free (value);
}
 
static void
egg_print_settings_init (EggPrintSettings *settings)
{
  settings->properties = g_hash_table_new_full (g_direct_hash,
						NULL,
						NULL,
						free_value);
}
  
EggPrintSettings *
egg_print_settings_new (void)
{
  return g_object_new (EGG_TYPE_PRINT_SETTINGS, NULL);
}

void
copy_property (gpointer key,
	       gpointer value,
	       gpointer data)
{
  egg_print_settings_set_property (data,
				   GPOINTER_TO_UINT(key), value,
				   NULL);
}


EggPrintSettings *
egg_print_settings_copy (EggPrintSettings *other)
{
  EggPrintSettings *settings;

  g_return_val_if_fail (EGG_IS_PRINT_SETTINGS (other), NULL);

  settings = egg_print_settings_new ();

  g_hash_table_foreach (other->properties,
			copy_property,
			settings);

  return settings;
}

void
egg_print_settings_install_property (GParamSpec *pspec)
{
  g_object_class_install_property (g_type_class_peek (EGG_TYPE_PRINT_SETTINGS),
				   ++egg_print_settings_n_properties,
				   pspec);
}
