/* GTK - The GIMP Toolkit
 * eggprinteroption.c: Handling possible settings for a specific printer setting
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

#include "eggprinteroption.h"

/*****************************************
 *            EggPrinterOption           *
 *****************************************/

enum {
  CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EggPrinterOption, egg_printer_option, G_TYPE_OBJECT)

static void
egg_printer_option_finalize (GObject *object)
{
  EggPrinterOption *option = EGG_PRINTER_OPTION (object);
  int i;
  
  g_free (option->name);
  g_free (option->display_text);
  g_free (option->value);
  for (i = 0; i < option->num_choices; i++)
    {
      g_free (option->choices[i]);
      g_free (option->choices_display[i]);
    }
  g_free (option->choices);
  g_free (option->choices_display);
  g_free (option->group);
  
  G_OBJECT_CLASS (egg_printer_option_parent_class)->finalize (object);
}

static void
egg_printer_option_init (EggPrinterOption *option)
{
}

static void
egg_printer_option_class_init (EggPrinterOptionClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_printer_option_finalize;

  signals[CHANGED] =
    g_signal_new ("changed",
		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrinterOptionClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

EggPrinterOption *
egg_printer_option_new (const char *name, const char *display_text,
			       EggPrinterOptionType type)
{
  EggPrinterOption *option;

  option = g_object_new (EGG_TYPE_PRINTER_OPTION, NULL);

  option->name = g_strdup (name);
  option->display_text = g_strdup (display_text);
  option->type = type;
  
  return option;
}

static void
emit_changed (EggPrinterOption *option)
{
  g_signal_emit (option, signals[CHANGED], 0);
}

void
egg_printer_option_set (EggPrinterOption *option,
			const char *value)
{
  
  if ((option->value == NULL && value == NULL) ||
      (option->value != NULL && value != NULL &&
       strcmp (option->value, value) == 0))
    return;

  if (option->type == EGG_PRINTER_OPTION_TYPE_PICKONE &&
      value != NULL)
    {
      int i;
      
      for (i = 0; i < option->num_choices; i++)
	{
	  if (g_ascii_strcasecmp (value, option->choices[i]) == 0)
	    {
	      value = option->choices[i];
	      break;
	    }
	}

      if (i == option->num_choices)
	return; /* Not found in availible choices */
    }
  
  g_free (option->value);
  option->value = g_strdup (value);
  
  emit_changed (option);
}

void
egg_printer_option_set_boolean (EggPrinterOption *option,
				gboolean value)
{
  egg_printer_option_set (option, value ? "True" : "False");
}

void
egg_printer_option_set_has_conflict  (EggPrinterOption *option,
				      gboolean  has_conflict)
{
  has_conflict = has_conflict != 0;
  
  if (option->has_conflict == has_conflict)
    return;

  option->has_conflict = has_conflict;
  emit_changed (option);
}

void
egg_printer_option_clear_has_conflict (EggPrinterOption     *option)
{
  egg_printer_option_set_has_conflict  (option, FALSE);
}

void
egg_printer_option_allocate_choices (EggPrinterOption     *option,
				     int num)
{
  g_free (option->choices);
  g_free (option->choices_display);

  option->num_choices = num;
  if (num == 0)
    {
      option->choices = NULL;
      option->choices_display = NULL;
    }
  else
    {
      option->choices = g_new0 (char *, num);
      option->choices_display = g_new0 (char *, num);
    }
}

void
egg_printer_option_choices_from_array (EggPrinterOption   *option,
				       int                 num_choices,
				       char               *choices[],
				       char              *choices_display[])
{
  int i;
  
  egg_printer_option_allocate_choices (option, num_choices);
  for (i = 0; i < num_choices; i++)
    {
      option->choices[i] = g_strdup (choices[i]);
      option->choices_display[i] = g_strdup (choices_display[i]);
    }
}
