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
  GtkWidget *label;
  GtkWidget *align;

  parent = EGG_PRINTER (printer);

  /* TODO: make this a gtkfilechooserentry once we move to GTK */
  printer->fileentry = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (printer->fileentry), "newprintout.pdf");

  printer->filebutton = gtk_file_chooser_button_new ("Print to PDF", 
                                                     GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

  printer->filechooser = gtk_table_new (2, 2, FALSE);

  align = gtk_alignment_new (0, 0.5, 0, 0);
  label = gtk_label_new ("Name:");
  gtk_container_add (GTK_CONTAINER (align), label);
 
  gtk_table_attach (GTK_TABLE (printer->filechooser), align,
                    0, 1, 0, 1, GTK_FILL, 0,
                    0, 0);

  gtk_table_attach (GTK_TABLE (printer->filechooser), printer->fileentry,
                    1, 2, 0, 1, GTK_FILL, 0,
                    0, 0);

  align = gtk_alignment_new (0, 0.5, 0, 0);
  label = gtk_label_new ("Save in folder:");
  gtk_container_add (GTK_CONTAINER (align), label);
  
  gtk_table_attach (GTK_TABLE (printer->filechooser), align,
                    0, 1, 1, 2, GTK_FILL, 0,
                    0, 0);

  gtk_table_attach (GTK_TABLE (printer->filechooser), printer->filebutton,
                    1, 2, 1, 2, GTK_FILL, 0,
                    0, 0);

  
  /* this is part of our custom widget API so keep a ref around
     so we don't get destroyed by the dialog */
  gtk_widget_ref (printer->filechooser);

  parent->priv->has_details = TRUE;
  parent->priv->is_virtual = TRUE;
  
}

static void
egg_printer_pdf_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  EggPrinterPdf *printer = EGG_PRINTER_PDF (object);

  if (printer->filechooser)
    gtk_widget_destroy (printer->filechooser);

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

