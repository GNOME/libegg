/* EggPrinter
 * Copyright (C) 2006 John (J5) Palmieri  <johnp@redhat.com>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
//#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "eggprinter.h"
#include "eggprinter-private.h"
#include "eggprintbackend.h"

#define EGG_PRINTER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PRINTER, EggPrinterPrivate))

static void egg_printer_finalize     (GObject *object);

G_DEFINE_TYPE (EggPrinter, egg_printer, G_TYPE_OBJECT);

static void
egg_printer_class_init (EggPrinterClass *class)
{
  GObjectClass *object_class;
  object_class = (GObjectClass *) class;

  object_class->finalize = egg_printer_finalize;

  g_type_class_add_private (class, sizeof (EggPrinterPrivate));  
}

static void
egg_printer_init (EggPrinter *printer)
{
  printer->priv = EGG_PRINTER_GET_PRIVATE (printer); 
  printer->priv->backend_data = NULL;
  printer->priv->backend_data_destroy_notify = NULL;

  printer->priv->name = NULL;
  printer->priv->location = NULL;
  printer->priv->description = NULL;

  printer->priv->is_active = TRUE;
  printer->priv->is_new = TRUE;

  printer->priv->state_message = NULL;  
  printer->priv->job_count = 0;
}

static void
egg_printer_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  EggPrinter *printer = EGG_PRINTER (object);

  g_free (printer->priv->name);
  g_free (printer->priv->location);
  g_free (printer->priv->description);
  g_free (printer->priv->state_message);

  if (printer->priv->backend_data_destroy_notify)
    printer->priv->backend_data_destroy_notify (printer->priv->backend_data);

  if (G_OBJECT_CLASS (egg_printer_parent_class)->finalize)
    G_OBJECT_CLASS (egg_printer_parent_class)->finalize (object);
}

/**
 * egg_printer_new:
 *
 * Creates a new #EggPrinter.
 *
 * Return value: a new #EggPrinter
 *
 * Since: 2.8
 **/
EggPrinter *
egg_printer_new (void)
{
  GObject *result;
  
  result = g_object_new (EGG_TYPE_PRINTER,
                         NULL);

  return (EggPrinter *) result;
}

void
egg_printer_set_backend_data (EggPrinter *printer,
                                    void *data,
                                    GFreeFunc destroy_notify)
{
  EGG_IS_PRINTER (printer);

  printer->priv->backend_data = data;
  printer->priv->backend_data_destroy_notify = destroy_notify;
}

const gchar *
egg_printer_get_name (EggPrinter *printer)
{
  EGG_IS_PRINTER (printer);

  return printer->priv->name;
}

const gchar *
egg_printer_get_state_message (EggPrinter *printer)
{
  EGG_IS_PRINTER (printer);

  return printer->priv->state_message;
}

const gchar *
egg_printer_get_location (EggPrinter *printer)
{
  EGG_IS_PRINTER (printer);

  return printer->priv->location;
}


gint 
egg_printer_get_job_count (EggPrinter *printer)
{
  EGG_IS_PRINTER (printer);

  return printer->priv->job_count;
}

cairo_surface_t *
egg_printer_create_cairo_surface (EggPrinter *printer,
                                        double width,
                                        double height)
{
   EGG_IS_PRINTER (printer);

  return egg_print_backend_printer_create_cairo_surface (printer->priv->backend,
                                                         printer,
                                                         width,
                                                         height);
}

