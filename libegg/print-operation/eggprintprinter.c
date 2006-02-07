/* EggPrintPrinter
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

#include "eggprintprinter.h"
#include "eggprintprinter-private.h"

#define EGG_PRINT_PRINTER_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PRINT_PRINTER, EggPrintPrinterPrivate))

static void egg_print_printer_finalize     (GObject *object);

G_DEFINE_TYPE (EggPrintPrinter, egg_print_printer, G_TYPE_OBJECT);

static void
egg_print_printer_class_init (EggPrintPrinterClass *class)
{
  GObjectClass *object_class;
  object_class = (GObjectClass *) class;

  object_class->finalize = egg_print_printer_finalize;

  g_type_class_add_private (class, sizeof (EggPrintPrinterPrivate));  
}

static void
egg_print_printer_init (EggPrintPrinter *printer)
{
  printer->priv = EGG_PRINT_PRINTER_GET_PRIVATE (printer); 
  printer->priv->backend_data = NULL;
  printer->priv->backend_data_destroy_notify = NULL;

  printer->name;
  printer->location;
  printer->description;

  printer->is_active = TRUE;
  printer->is_new = TRUE;

  printer->state_message = NULL;  
  printer->job_count = 0;


}

static void
egg_print_printer_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  EggPrintPrinter *printer = EGG_PRINT_PRINTER (object);

  g_free (printer->name);
  g_free (printer->location);
  g_free (printer->description);
  g_free (printer->state_message);

  if (printer->priv->backend_data_destroy_notify)
    printer->priv->backend_data_destroy_notify (printer->priv->backend_data);

  if (G_OBJECT_CLASS (egg_print_printer_parent_class)->finalize)
    G_OBJECT_CLASS (egg_print_printer_parent_class)->finalize (object);
}

/**
 * egg_print_printer_new:
 *
 * Creates a new #EggPrintPrinter.
 *
 * Return value: a new #EggPrintPrinter
 *
 * Since: 2.8
 **/
EggPrintPrinter *
egg_print_printer_new (void)
{
  GObject *result;
  
  result = g_object_new (EGG_TYPE_PRINT_PRINTER,
                         NULL);

  return (EggPrintPrinter *) result;
}

void
egg_print_printer_set_backend_data (EggPrintPrinter *printer,
                                    void *data,
                                    GFreeFunc destroy_notify)
{
  EGG_IS_PRINT_PRINTER (printer);

  printer->priv->backend_data = data;
  printer->priv->backend_data_destroy_notify = destroy_notify;
}

