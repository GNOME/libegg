/* GTK - The GIMP Toolkit
 * eggprintoperation.c: Print Operation
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
#include <cairo-pdf.h>

#define EGG_PRINT_OPERATION_GET_PRIVATE(obj)(G_TYPE_INSTANCE_GET_PRIVATE ((obj), EGG_TYPE_PRINT_OPERATION, EggPrintOperationPrivate))

enum {
  BEGIN_PRINT,
  REQUEST_PAGE_SETUP,
  DRAW_PAGE,
  END_PRINT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };
static int job_nr = 1;

G_DEFINE_TYPE (EggPrintOperation, egg_print_operation, G_TYPE_OBJECT)

GQuark     
egg_print_error_quark (void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("egg-print-error-quark");
  return quark;
}
     
static void
egg_print_operation_finalize (GObject *object)
{
  EggPrintOperation *print_operation = EGG_PRINT_OPERATION (object);

  if (print_operation->priv->default_page_setup)
    g_object_unref (print_operation->priv->default_page_setup);
  
  if (print_operation->priv->print_settings)
    g_object_unref (print_operation->priv->print_settings);
  
  g_free (print_operation->priv->pdf_target);
  g_free (print_operation->priv->job_name);
  G_OBJECT_CLASS (egg_print_operation_parent_class)->finalize (object);
}

static void
egg_print_operation_init (EggPrintOperation *operation)
{
  const char *appname;

  operation->priv = EGG_PRINT_OPERATION_GET_PRIVATE (operation);

  operation->priv->default_page_setup = NULL;
  operation->priv->print_settings = NULL;
  operation->priv->nr_of_pages = -1;
  operation->priv->current_page = -1;
  operation->priv->use_full_page = FALSE;
  operation->priv->show_dialog = TRUE;
  operation->priv->pdf_target = NULL;

  operation->priv->unit = EGG_UNIT_PIXEL;

  appname = g_get_application_name ();
  if (appname == NULL)
    appname = "Gtk+ application";

  operation->priv->job_name = g_strdup_printf ("%s job #%d",
					       appname, job_nr++);
}

static void
egg_print_operation_class_init (EggPrintOperationClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_operation_finalize;
  
  g_type_class_add_private (gobject_class, sizeof (EggPrintOperationPrivate));


  signals[BEGIN_PRINT] =
    g_signal_new ("begin_print",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintOperationClass, begin_print),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, EGG_TYPE_PRINT_CONTEXT);
  signals[REQUEST_PAGE_SETUP] =
    g_signal_new ("request_page_setup",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintOperationClass, request_page_setup),
		  NULL, NULL,
		  _egg_marshal_VOID__OBJECT_INT_OBJECT,
		  G_TYPE_NONE, 3,
		  EGG_TYPE_PRINT_CONTEXT,
		  G_TYPE_INT,
		  EGG_TYPE_PAGE_SETUP);
  signals[DRAW_PAGE] =
    g_signal_new ("draw_page",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintOperationClass, draw_page),
		  NULL, NULL,
		  _egg_marshal_VOID__OBJECT_INT,
		  G_TYPE_NONE, 2,
		  EGG_TYPE_PRINT_CONTEXT,
		  G_TYPE_INT);
  signals[END_PRINT] =
    g_signal_new ("end_print",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintOperationClass, end_print),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__OBJECT,
		  G_TYPE_NONE, 1, EGG_TYPE_PRINT_CONTEXT);
}

EggPrintOperation *
egg_print_operation_new ()
{
  EggPrintOperation *print_operation;

  print_operation = g_object_new (EGG_TYPE_PRINT_OPERATION, NULL);
  
  return print_operation;
}

void
egg_print_operation_set_default_page_setup (EggPrintOperation  *op,
					    EggPageSetup    *default_page_setup)
{
  g_return_if_fail (op != NULL);

  if (default_page_setup)
    g_object_ref (default_page_setup);

  if (op->priv->default_page_setup)
    g_object_unref (op->priv->default_page_setup);

  op->priv->default_page_setup = default_page_setup;
}

EggPageSetup *
egg_print_operation_get_default_page_setup (EggPrintOperation  *op)
{
  g_return_val_if_fail (op != NULL, NULL);

  return op->priv->default_page_setup;
}

void
egg_print_operation_set_print_settings (EggPrintOperation  *op,
					EggPrintSettings *print_settings)
{
  g_return_if_fail (op != NULL);
  g_return_if_fail (print_settings != NULL);

  if (print_settings)
    g_object_ref (print_settings);

  if (op->priv->print_settings)
    g_object_unref (op->priv->print_settings);
  
  op->priv->print_settings = print_settings;
}

EggPrintSettings *
egg_print_operation_get_print_settings (EggPrintOperation  *op)
{
  g_return_val_if_fail (op != NULL, NULL);

  if (op->priv->print_settings)
    return g_object_ref (op->priv->print_settings);
  
  return NULL;
}

void
egg_print_operation_set_job_name (EggPrintOperation  *op,
				  const char         *job_name)
{
  g_return_if_fail (g_utf8_validate (job_name, -1, NULL));

  g_free (op->priv->job_name);
  op->priv->job_name = g_strdup (job_name);
}

void
egg_print_operation_set_nr_of_pages (EggPrintOperation  *op,
				     int                 n_pages)
{
  g_return_if_fail (op != NULL);

  op->priv->nr_of_pages = n_pages;
}

void
egg_print_operation_set_current_page (EggPrintOperation  *op,
				      int                 current_page)
{
  g_return_if_fail (op != NULL);

  op->priv->current_page = current_page;
}

void
egg_print_operation_set_use_full_page (EggPrintOperation  *op,
				       gboolean            full_page)
{
  g_return_if_fail (op != NULL);

  op->priv->use_full_page = full_page;
}

void
egg_print_operation_set_unit (EggPrintOperation  *op,
			      EggUnit             unit)
{
  op->priv->unit = unit;
}

void
egg_print_operation_set_show_dialog (EggPrintOperation  *op,
				     gboolean            show_dialog)
{
  g_return_if_fail (op != NULL);

  op->priv->show_dialog = show_dialog;
}

void
egg_print_operation_set_pdf_target (EggPrintOperation  *op,
				    const char *        filename)
{
  g_return_if_fail (op != NULL);

  g_free (op->priv->pdf_target);
  op->priv->pdf_target = g_strdup (filename);
}

/* Creates the initial page setup used for printing unless the
 * app overrides this on a per-page basis using request_page_setup.
 *
 * Data is taken from, in order, if existing:
 *
 * PrintSettings returned from the print dialog
 *  (initial dialog values are set from default_page_setup
     if unset in app specified print_settings)
 * default_page_setup
 * per-locale default setup
 */
static EggPageSetup *
create_page_setup (EggPrintOperation  *op)
{
  EggPageSetup *page_setup;
  EggPrintSettings *settings;
  
  if (op->priv->default_page_setup)
    page_setup = egg_page_setup_copy (op->priv->default_page_setup);
  else
    page_setup = egg_page_setup_new ();

  settings = op->priv->print_settings;
  if (settings)
    {
      EggPaperSize *paper_size;
      
      if (egg_print_settings_has_key (settings, EGG_PRINT_SETTINGS_ORIENTATION))
	egg_page_setup_set_orientation (page_setup,
					egg_print_settings_get_orientation (settings));


      paper_size = egg_print_settings_get_paper_size (settings);
      if (paper_size)
	egg_page_setup_set_paper_size (page_setup, paper_size);
      egg_paper_size_free (paper_size);

      /* TODO: Margins? */
    }
  
  return page_setup;
}

static void 
pdf_start_page (EggPrintOperation *op,
		EggPrintContext *print_context,
		EggPageSetup *page_setup)
{
  /* TODO: Set up page size, not supported in cairo yet */
}

static void
pdf_end_page (EggPrintOperation *op,
	      EggPrintContext *print_context)
{
  cairo_t *cr;

  cr = egg_print_context_get_cairo (print_context);
  cairo_show_page (cr);
}

static void
pdf_end_run (EggPrintOperation *op)
{
  cairo_surface_destroy (op->priv->surface);
  op->priv->surface = NULL;
}

static EggPrintOperationResult
run_pdf (EggPrintOperation *op,
	 GtkWindow *parent,
	 gboolean *do_print,
	 GError **error)
{
  EggPageSetup *page_setup;
  double width, height;
  /* This will be overwritten later by the non-default size, but
     we need to pass some size: */
  
  page_setup = create_page_setup (op);
  width = egg_page_setup_get_paper_width (page_setup, EGG_UNIT_POINTS);
  height = egg_page_setup_get_paper_height (page_setup, EGG_UNIT_POINTS);
  g_object_unref (page_setup);
  
  op->priv->surface = cairo_pdf_surface_create (op->priv->pdf_target,
						width, height);
  /* TODO: DPI from settings object? */
  cairo_pdf_surface_set_dpi (op->priv->surface, 300, 300);
  
  op->priv->dpi_x = 72;
  op->priv->dpi_y = 72;

  op->priv->manual_num_copies = 1;
  op->priv->manual_collation = FALSE;
  
  *do_print = TRUE;
  
  op->priv->start_page = pdf_start_page;
  op->priv->end_page = pdf_end_page;
  op->priv->end_run = pdf_end_run;
  
  return EGG_PRINT_OPERATION_RESULT_APPLY; 
}

static EggPrintOperationResult
run_print_dialog (EggPrintOperation *op,
		  GtkWindow *parent,
		  gboolean *do_print,
		  GError **error)
{
  if (op->priv->pdf_target != NULL)
    return run_pdf (op, parent, do_print, error);

  /* This does:
   * Open print dialog 
   * set print settings on dialog
   * run dialog, if show_dialog set
   * extract print settings from dialog
   * create cairo surface and data for print job
   * return correct result val
   */
  return egg_print_operation_platform_backend_run_dialog (op, parent,
							  do_print,
							  error);
}

EggPrintOperationResult
egg_print_operation_run (EggPrintOperation  *op,
			 GtkWindow *parent,
			 GError **error)
{
  int page, range;
  EggPageSetup *initial_page_setup, *page_setup;
  EggPrintContext *print_context;
  cairo_t *cr;
  gboolean do_print;
  int uncollated_copies, collated_copies;
  int i, j;
  EggPageRange *ranges;
  int num_ranges;
  EggPrintOperationResult result;

  result = run_print_dialog (op, parent, &do_print, error);
  if (!do_print)
    return result;
  
  if (op->priv->manual_collation)
    {
      uncollated_copies = 1;
      collated_copies = op->priv->manual_num_copies;
    }
  else
    {
      uncollated_copies = op->priv->manual_num_copies;
      collated_copies = 1;
    }

  print_context = _egg_print_context_new (op);

  initial_page_setup = create_page_setup (op);
  _egg_print_context_set_page_setup (print_context, initial_page_setup);

  g_signal_emit (op, signals[BEGIN_PRINT], 0, print_context);
  
  g_return_val_if_fail (op->priv->nr_of_pages != -1, FALSE);

  if (op->priv->print_settings == NULL)
    {
      ranges = g_new (EggPageRange, 1);
      num_ranges = 1;
      ranges[0].start = 0;
      ranges[0].end = op->priv->nr_of_pages - 1;
    }
  else
    {
      EggPrintPages print_pages = egg_print_settings_get_print_pages (op->priv->print_settings);
      ranges = NULL;
      if (print_pages == EGG_PRINT_PAGES_RANGES)
	ranges = egg_print_settings_get_page_ranges (op->priv->print_settings,
						     &num_ranges);
      if (ranges == NULL)
	{
	  ranges = g_new (EggPageRange, 1);
	  num_ranges = 1;
	  if (print_pages == EGG_PRINT_PAGES_CURRENT &&
	      op->priv->current_page != -1)
	    {
	      /* Current */
	      ranges[0].start = op->priv->current_page;
	      ranges[0].end = op->priv->current_page;
	    }
	  else
	    {
	      /* All */
	      ranges[0].start = 0;
	      ranges[0].end = op->priv->nr_of_pages - 1;
	    }
	}
    }
  
  for (i = 0; i < uncollated_copies; i++)
    {
      for (range = 0; range < num_ranges; range ++)
	{
	  for (page = ranges[range].start; page <= ranges[range].end; page ++)
	    {
	      for (j = 0; j < collated_copies; j++)
		{
		  page_setup = egg_page_setup_copy (initial_page_setup);
		  g_signal_emit (op, signals[REQUEST_PAGE_SETUP], 0, print_context, page, page_setup);
		  
		  _egg_print_context_set_page_setup (print_context, page_setup);
		  op->priv->start_page (op, print_context, page_setup);
		  
		  g_object_unref (page_setup);
		  
		  cr = egg_print_context_get_cairo (print_context);
		  
		  cairo_save (cr);
		  
		  if (!op->priv->use_full_page)
		    _egg_print_context_translate_into_margin (print_context);
		  
		  g_signal_emit (op, signals[DRAW_PAGE], 0, 
				 print_context, page);
		  
		  op->priv->end_page (op, print_context);
		  
		  cairo_restore (cr);
		}
	    }
	}
    }
  
  g_signal_emit (op, signals[END_PRINT], 0, print_context);

  g_object_unref (print_context);
  g_object_unref (initial_page_setup);

  cairo_surface_finish (op->priv->surface);
  op->priv->end_run (op);

  return EGG_PRINT_OPERATION_RESULT_APPLY;
}
