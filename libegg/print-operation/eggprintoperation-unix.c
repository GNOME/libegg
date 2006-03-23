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
#include "eggprintmarshal.h"

#include "eggprintunixdialog.h"
#include "eggpagesetupunixdialog.h"
#include "eggprintbackend.h"
#include "eggprinter.h"
#include "eggprintjob.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
  EggPrintJob *job;         /* the job we are sending to the printer */
  GtkWindow *parent;        /* parent window just in case we need to throw error dialogs */
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
_op_unix_free (EggPrintOperationUnix *op_unix)
{
  if (op_unix->job)
    g_object_unref (G_OBJECT (op_unix->job));

  g_free (op_unix);
}

static void
unix_finish_send  (EggPrintJob *job,
                   void *user_data, 
                   GError **error)
{
  EggPrintOperationUnix *op_unix;
  GtkWindow *parent;

  op_unix = (EggPrintOperationUnix *) user_data;

  parent = op_unix->parent;

  _op_unix_free (op_unix);

  if (error != NULL && *error != NULL)
    {
      GtkWidget *edialog;
      GError *err = *error;

      edialog = gtk_message_dialog_new (parent, 
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_ERROR,
                                        GTK_BUTTONS_CLOSE,
                                        "Error printing: %s",
                                        err->message);

      gtk_dialog_run (GTK_DIALOG (edialog));
      gtk_widget_destroy (edialog);
    }
}

static void
unix_end_run (EggPrintOperation *op)
{
  EggPrintOperationUnix *op_unix = op->priv->platform_data;
 
  /* TODO: Check for error */
  egg_print_job_send (g_object_ref (op_unix->job),
                      unix_finish_send, 
                      op_unix, NULL);

  op->priv->platform_data = NULL;
}

EggPrintOperationResult
_egg_print_operation_platform_backend_run_dialog (EggPrintOperation *op,
						  GtkWindow *parent,
						  gboolean *do_print,
						  GError **error)
{
  GtkWidget *pd;
  EggPrintOperationResult result;
  EggPageSetup *page_setup;
  
  result = EGG_PRINT_OPERATION_RESULT_CANCEL;

  if (op->priv->default_page_setup)
    page_setup = egg_page_setup_copy (op->priv->default_page_setup);
  else
    page_setup = egg_page_setup_new ();

  pd = egg_print_unix_dialog_new (NULL, parent);

  if (op->priv->print_settings)
    egg_print_unix_dialog_set_settings (EGG_PRINT_UNIX_DIALOG (pd),
					op->priv->print_settings);

  egg_print_unix_dialog_set_page_setup (EGG_PRINT_UNIX_DIALOG (pd), page_setup);
  
  *do_print = FALSE; 
  if (gtk_dialog_run (GTK_DIALOG (pd)) == GTK_RESPONSE_ACCEPT)
    {
      EggPrintOperationUnix *op_unix;
      EggPrinter *printer;
      EggPrintSettings *settings, *settings_copy;

      result = EGG_PRINT_OPERATION_RESULT_APPLY;
      
      printer = egg_print_unix_dialog_get_selected_printer (EGG_PRINT_UNIX_DIALOG (pd));
      if (printer == NULL)
	goto out;
      
      *do_print = TRUE;

      settings = egg_print_unix_dialog_get_settings (EGG_PRINT_UNIX_DIALOG (pd));

      /* We save a copy to return to the user to avoid exposing
	 the extra settings preparint the printer job adds. */
      settings_copy = egg_print_settings_copy (settings);
      egg_print_operation_set_print_settings (op, settings_copy);
      g_object_unref (settings_copy);

      op_unix = g_new (EggPrintOperationUnix, 1);
      op_unix->job = egg_printer_prep_job (printer,
					   settings,
					   page_setup,
                                           "Title",
					   error);
      g_object_unref (settings);
    
      if (error != NULL && *error != NULL)
        {
	  *do_print = FALSE;
	  _op_unix_free (op_unix);
	  result = EGG_PRINT_OPERATION_RESULT_ERROR;
	  goto out;
	}

      op_unix->parent = parent;

      op->priv->surface = egg_print_job_get_surface (op_unix->job);

      op->priv->dpi_x = 72;
      op->priv->dpi_y = 72;
 
      op->priv->platform_data = op_unix;

      /* TODO: hook up to dialog elements */
      op->priv->manual_num_copies =
	egg_print_settings_get_int_with_default (settings, "manual-num-copies", 1);
      op->priv->manual_collation =
	egg_print_settings_get_bool (settings, "manual-collate");
      op->priv->manual_scale =
	egg_print_settings_get_double_with_default (settings, "manual-scale", 100.0);
      op->priv->manual_orientation =
	egg_print_settings_get_bool (settings, "manual-orientation");
    } 

  op->priv->start_page = unix_start_page;
  op->priv->end_page = unix_end_page;
  op->priv->end_run = unix_end_run;

 out:
  g_object_unref (page_setup); 
  
  gtk_widget_destroy (pd);

  return result;
}

EggPageSetup *
egg_print_run_page_setup_dialog (GtkWindow        *parent,
				 EggPageSetup     *page_setup,
				 EggPrintSettings *settings)
{
  GtkWidget *dialog;
  EggPageSetup *new_page_setup;
  
  dialog = egg_page_setup_unix_dialog_new (NULL, parent);
  if (page_setup)
    egg_page_setup_unix_dialog_set_page_setup (EGG_PAGE_SETUP_UNIX_DIALOG (dialog),
					       page_setup);
  egg_page_setup_unix_dialog_set_print_settings (EGG_PAGE_SETUP_UNIX_DIALOG (dialog),
						 settings);
  gtk_dialog_run (GTK_DIALOG (dialog));

  new_page_setup = egg_page_setup_unix_dialog_get_page_setup (EGG_PAGE_SETUP_UNIX_DIALOG (dialog));

  gtk_widget_destroy (dialog);

  return new_page_setup;
}

