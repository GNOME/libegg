/* EggPrinterLpr
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

#include "config.h"
#include "eggprinterlpr.h"
#include "eggprinter-private.h"

static void egg_printer_lpr_finalize     (GObject *object);

G_DEFINE_TYPE (EggPrinterLpr, egg_printer_lpr, EGG_TYPE_PRINTER);

static void
egg_printer_lpr_class_init (EggPrinterLprClass *class)
{
  GObjectClass *object_class;
  object_class = (GObjectClass *) class;

  object_class->finalize = egg_printer_lpr_finalize;

}

static void
egg_printer_lpr_init (EggPrinterLpr *printer)
{
  EggPrinter *parent;

  parent = EGG_PRINTER (printer);

  printer->options = NULL;

  parent->priv->has_details = TRUE;
  parent->priv->is_virtual = TRUE;
  
}

static void
egg_printer_lpr_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  EggPrinterLpr *printer = EGG_PRINTER_LPR (object);

  if (printer->options)
    g_object_unref (printer->options);

  if (G_OBJECT_CLASS (egg_printer_lpr_parent_class)->finalize)
    G_OBJECT_CLASS (egg_printer_lpr_parent_class)->finalize (object);
}

/**
 * egg_printer_lpr_new:
 *
 * Creates a new #EggPrinterLpr.
 *
 * Return value: a new #EggPrinterLpr
 *
 * Since: 2.10
 **/
EggPrinterLpr *
egg_printer_lpr_new (void)
{
  GObject *result;
  
  result = g_object_new (EGG_TYPE_PRINTER_LPR,
                         NULL);

  return (EggPrinterLpr *) result;
}

