/* EGG - The GIMP Toolkit
 * eggprintoperation-unix.c: Print Operation Details for Unix and Unix like platforms
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

#include "eggprintoperation-private.h"
#include "eggprintcontext-private.h"
#include "eggprintmarshal.h"

#include "eggprintunixdialog.h"
#include "eggprintbackend.h"

typedef struct {
  EggPrinter *printer;  /* the printer to send the job to */
} EggPrintOperationUnix;

static void
unix_start_page (EggPrintOperation *op,
		 EggPrintContext *print_context,
		 EggPageSetup *page_setup)
{

}

static void
unix_end_page (EggPrintOperation *op,
	       EggPrintContext *print_context)
{
  cairo_t *cr;

  cr = egg_print_context_get_cairo (print_context);
  cairo_show_page (cr);
}

static void
unix_end_run (EggPrintOperation *op)
{
  EggPrintOperationUnix *op_unix = op->priv->platform_data;
  g_object_unref (G_OBJECT (op_unix->printer));

  g_free (op_unix);
  op->priv->platform_data = NULL;

  cairo_surface_destroy (op->priv->surface);
  op->priv->surface = NULL;
}

EggPrintOperationResult
egg_print_operation_platform_backend_run_dialog (EggPrintOperation *op,
						 GtkWindow *parent,
						 gboolean *do_print,
						 GError **error)
{
  GtkWidget *pd;
  EggPrintOperationResult result;
  
  result = EGG_PRINT_OPERATION_RESULT_CANCEL;
  
  pd = egg_print_unix_dialog_new ("Print...", parent, NULL);
  
  *do_print = FALSE; 
  if (gtk_dialog_run (GTK_DIALOG (pd)) == GTK_RESPONSE_ACCEPT)
    {
      EggPrintOperationUnix *op_unix;
      EggPrinter *printer;
      EggPageSetup *page_setup;
      double width, height;

      *do_print = TRUE;
      result = EGG_PRINT_OPERATION_RESULT_APPLY;

      printer = egg_print_unix_dialog_get_selected_printer (EGG_PRINT_UNIX_DIALOG (pd));

      g_object_ref (G_OBJECT (printer));

      if (op->priv->default_page_setup)
        page_setup = egg_page_setup_copy (op->priv->default_page_setup);
      else
        page_setup = egg_page_setup_new ();

      width = egg_page_setup_get_paper_width (page_setup, EGG_UNIT_POINTS);
      height = egg_page_setup_get_paper_height (page_setup, EGG_UNIT_POINTS);
      g_object_unref (page_setup); 

      op_unix = g_new (EggPrintOperationUnix, 1);
      op_unix->printer = printer;

      op->priv->surface = egg_printer_create_cairo_surface (printer,
                                                                  width, 
                                                                  height);

      op->priv->dpi_x = 72;
      op->priv->dpi_y = 72;
 
      op->priv->platform_data = op_unix;

      /* TODO: hook up to dialog elements */
      op->priv->manual_num_copies = 1;
      op->priv->manual_collation = FALSE;
    } 

  op->priv->start_page = unix_start_page;
  op->priv->end_page = unix_end_page;
  op->priv->end_run = unix_end_run;

  gtk_widget_destroy (pd);

  return result;
}

