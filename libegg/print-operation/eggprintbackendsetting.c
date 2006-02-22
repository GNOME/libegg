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
#include <string.h>

#include "eggprintbackendsetting.h"

/*****************************************
 *             EggPrintBackendSetting    *
 *****************************************/

enum {
  CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EggPrintBackendSetting, egg_print_backend_setting, G_TYPE_OBJECT)

static void
egg_print_backend_setting_finalize (GObject *object)
{
  EggPrintBackendSetting *setting = EGG_PRINT_BACKEND_SETTING (object);
  int i;
  
  g_free (setting->name);
  g_free (setting->display_text);
  g_free (setting->value);
  for (i = 0; i < setting->num_choices; i++)
    {
      g_free (setting->choices[i]);
      g_free (setting->choices_display[i]);
    }
  g_free (setting->choices);
  g_free (setting->choices_display);
  g_free (setting->group);
  
  G_OBJECT_CLASS (egg_print_backend_setting_parent_class)->finalize (object);
}

static void
egg_print_backend_setting_init (EggPrintBackendSetting *setting)
{
}

static void
egg_print_backend_setting_class_init (EggPrintBackendSettingClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_backend_setting_finalize;

  signals[CHANGED] =
    g_signal_new ("changed",
		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintBackendSettingClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

EggPrintBackendSetting *
egg_print_backend_setting_new (const char *name, const char *display_text,
			       EggPrintBackendSettingType type)
{
  EggPrintBackendSetting *setting;

  setting = g_object_new (EGG_TYPE_PRINT_BACKEND_SETTING, NULL);

  setting->name = g_strdup (name);
  setting->display_text = g_strdup (display_text);
  setting->type = type;
  
  return setting;
}

static void
emit_changed (EggPrintBackendSetting *setting)
{
  g_signal_emit (setting, signals[CHANGED], 0);
}

void
egg_print_backend_setting_set (EggPrintBackendSetting *setting,
			       const char *value)
{
  if ((setting->value == NULL && value == NULL) ||
      (setting->value != NULL && value != NULL &&
       strcmp (setting->value, value) == 0))
    return;
  
  g_free (setting->value);
  setting->value = g_strdup (value);
  
  emit_changed (setting);
}

void
egg_print_backend_setting_set_boolean (EggPrintBackendSetting *setting,
				       gboolean value)
{
  egg_print_backend_setting_set (setting, value ? "True" : "False");
}

void
egg_print_backend_setting_set_has_conflict  (EggPrintBackendSetting *setting,
					     gboolean                has_conflict)
{
  has_conflict = has_conflict != 0;
  
  if (setting->has_conflict == has_conflict)
    return;

  setting->has_conflict = has_conflict;
  emit_changed (setting);
}

void
egg_print_backend_setting_clear_has_conflict (EggPrintBackendSetting     *setting)
{
  egg_print_backend_setting_set_has_conflict  (setting, FALSE);
}

void
egg_print_backend_setting_allocate_choices (EggPrintBackendSetting     *setting,
					    int                         num)
{
  g_free (setting->choices);
  g_free (setting->choices_display);

  setting->num_choices = num;
  if (num == 0)
    {
      setting->choices = NULL;
      setting->choices_display = NULL;
    }
  else
    {
      setting->choices = g_new0 (char *, num);
      setting->choices_display = g_new0 (char *, num);
    }
}

void
egg_print_backend_setting_choices_from_array (EggPrintBackendSetting   *setting,
					      int                       num_choices,
					      char                     *choices[],
					      char                     *choices_display[])
{
  int i;
  
  egg_print_backend_setting_allocate_choices (setting, num_choices);
  for (i = 0; i < num_choices; i++)
    {
      setting->choices[i] = g_strdup (choices[i]);
      setting->choices_display[i] = g_strdup (choices_display[i]);
    }
}
