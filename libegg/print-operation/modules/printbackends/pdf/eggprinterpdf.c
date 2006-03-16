/* EggPrinterPdf
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
#include "eggprinterpdf.h"
#include "eggprinter-private.h"

static void egg_printer_pdf_finalize     (GObject *object);

G_DEFINE_TYPE (EggPrinterPdf, egg_printer_pdf, EGG_TYPE_PRINTER);

static void
egg_printer_pdf_class_init (EggPrinterPdfClass *class)
{
  GObjectClass *object_class;
  object_class = (GObjectClass *) class;

  object_class->finalize = egg_printer_pdf_finalize;

}

static void
egg_printer_pdf_init (EggPrinterPdf *printer)
{
  EggPrinter *parent;

  parent = EGG_PRINTER (printer);

  printer->file_option = NULL;

  parent->priv->has_details = TRUE;
  parent->priv->is_virtual = TRUE;
  
}

static void
egg_printer_pdf_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  EggPrinterPdf *printer = EGG_PRINTER_PDF (object);

  if (printer->file_option)
    g_object_unref (printer->file_option);

  if (G_OBJECT_CLASS (egg_printer_pdf_parent_class)->finalize)
    G_OBJECT_CLASS (egg_printer_pdf_parent_class)->finalize (object);
}

/**
 * egg_printer_pdf_new:
 *
 * Creates a new #EggPrinterPdf.
 *
 * Return value: a new #EggPrinterPdf
 *
 * Since: 2.10
 **/
EggPrinterPdf *
egg_printer_pdf_new (void)
{
  GObject *result;
  
  result = g_object_new (EGG_TYPE_PRINTER_PDF,
                         NULL);

  return (EggPrinterPdf *) result;
}

