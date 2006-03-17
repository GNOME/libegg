/* GTK - The GIMP Toolkit
 * eggprintbackendlpr.c: Default implementation of EggPrintBackend 
 * for printing to lpr 
 * Copyright (C) 2003, Red Hat, Inc.
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <errno.h>
#include <cairo.h>
#include <cairo-ps.h>

#include <glib/gi18n-lib.h>

#include "eggprintoperation.h"

#include "eggprintbackend.h"
#include "eggprintbackendlpr.h"

#include "eggprinter.h"
#include "eggprinter-private.h"

#include "eggprinterlpr.h"

typedef struct _EggPrintBackendLprClass EggPrintBackendLprClass;

#define EGG_PRINT_BACKEND_LPR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_BACKEND_LPR, EggPrintBackendLprClass))
#define EGG_IS_PRINT_BACKEND_LPR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_BACKEND_LPR))
#define EGG_PRINT_BACKEND_LPR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_BACKEND_LPR, EggPrintBackendLprClass))

#define _LPR_MAX_CHUNK_SIZE 8192

static GType print_backend_lpr_type = 0;

struct _EggPrintBackendLprClass
{
  GObjectClass parent_class;
};

struct _EggPrintBackendLpr
{
  GObject parent_instance;

  EggPrinterLpr *printer;

  GHashTable *printers;
};

static GObjectClass *backend_parent_class;

static void                 egg_print_backend_lpr_class_init      (EggPrintBackendLprClass           *class);
static void                 egg_print_backend_lpr_iface_init      (EggPrintBackendIface              *iface);
static void                 egg_print_backend_lpr_init            (EggPrintBackendLpr                *impl);
static void                 egg_print_backend_lpr_finalize        (GObject                           *object);
static GList *              lpr_request_printer_list              (EggPrintBackend                   *print_backend);
static void                 lpr_printer_get_settings_from_options (EggPrinter                        *printer,
								   EggPrinterOptionSet               *options,
								   EggPrintSettings                  *settings);
static gboolean             lpr_printer_mark_conflicts            (EggPrinter                        *printer,
								   EggPrinterOptionSet               *options);
static EggPrinterOptionSet *lpr_printer_get_options               (EggPrinter                        *printer);
static void                 lpr_printer_prepare_for_print         (EggPrinter                        *printer,
								   EggPrintSettings                  *settings);
static void                 lpr_printer_get_hard_margins          (EggPrinter                        *printer,
                                                                   double                            *top,
                                                                   double                            *bottom,
                                                                   double                            *left,
                                                                   double                            *right);
static void                 lpr_printer_request_details           (EggPrinter                        *printer);
static GList *              lpr_printer_list_papers               (EggPrinter                        *printer);

static void
egg_print_backend_register_type (GTypeModule *module)
{
  if (!print_backend_lpr_type)
    {
      static const GTypeInfo print_backend_lpr_info =
      {
	sizeof (EggPrintBackendLprClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
	(GClassInitFunc) egg_print_backend_lpr_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
	sizeof (EggPrintBackendLpr),
	0,		/* n_preallocs */
	(GInstanceInitFunc) egg_print_backend_lpr_init,
      };

      static const GInterfaceInfo print_backend_info =
      {
	(GInterfaceInitFunc) egg_print_backend_lpr_iface_init, /* interface_init */
	NULL,			                              /* interface_finalize */
	NULL			                              /* interface_data */
      };

      print_backend_lpr_type = g_type_module_register_type (module,
                                                             G_TYPE_OBJECT,
						             "EggPrintBackendLpr",
						             &print_backend_lpr_info, 0);
      g_type_module_add_interface (module,
                                   print_backend_lpr_type,
		 		   EGG_TYPE_PRINT_BACKEND,
				   &print_backend_info);
    }


}

G_MODULE_EXPORT void 
pb_module_init (GTypeModule    *module)
{
  egg_print_backend_register_type (module);
}

G_MODULE_EXPORT void 
pb_module_exit (void)
{

}
  
G_MODULE_EXPORT EggPrintBackend * 
pb_module_create (void)
{
  return egg_print_backend_lpr_new ();
}

/*
 * EggPrintBackendLpr
 */
GType
egg_print_backend_lpr_get_type (void)
{
  return print_backend_lpr_type;
}

/**
 * egg_print_backend_lpr_new:
 *
 * Creates a new #EggPrintBackendLpr object. #EggPrintBackendLpr
 * implements the #EggPrintBackend interface with direct access to
 * the filesystem using Unix/Linux API calls
 *
 * Return value: the new #EggPrintBackendLpr object
 **/
EggPrintBackend *
egg_print_backend_lpr_new (void)
{
  return g_object_new (EGG_TYPE_PRINT_BACKEND_LPR, NULL);
}

static void
egg_print_backend_lpr_class_init (EggPrintBackendLprClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  backend_parent_class = g_type_class_peek_parent (class);

  gobject_class->finalize = egg_print_backend_lpr_finalize;
}

static cairo_status_t
_cairo_write (void *cache_fd_as_pointer,
              const unsigned char *data,
              unsigned int         length)
{
  cairo_status_t result;
  gint cache_fd;
  cache_fd = GPOINTER_TO_INT (cache_fd_as_pointer);
  
  result = CAIRO_STATUS_WRITE_ERROR;
  
  /* write out the buffer */
  if (write (cache_fd, data, length) != -1)
      result = CAIRO_STATUS_SUCCESS;
   
  return result;
}


static cairo_surface_t *
lpr_printer_create_cairo_surface (EggPrinter *printer,
				   gdouble width, 
				   gdouble height,
				   gint cache_fd)
{
  cairo_surface_t *surface;
  
  surface = cairo_ps_surface_create_for_stream  (_cairo_write, GINT_TO_POINTER (cache_fd), width, height);

  /* TODO: DPI from settings object? */
  cairo_ps_surface_set_dpi (surface, 300, 300);

  return surface;
}

static EggPrinter *
egg_print_backend_lpr_find_printer (EggPrintBackend *print_backend,
                                     const gchar *printer_name)
{
  EggPrintBackendLpr *lpr_print_backend;
  EggPrinterLpr *printer;

  lpr_print_backend = EGG_PRINT_BACKEND_LPR (print_backend);
  
  printer = NULL;
  if (strcmp (EGG_PRINTER (lpr_print_backend->printer)->priv->name, printer_name) == 0)
    printer = lpr_print_backend->printer;

  return (EggPrinter *) printer; 
}

typedef struct {
  EggPrintBackend *backend;
  EggPrintJobCompleteFunc callback;
  EggPrintJob *job;
  gint target_fd;
  gpointer user_data;
} _PrintStreamData;

static void
lpr_print_cb (EggPrintBackendLpr *print_backend,
              GError **error,
              gpointer user_data)
{
  _PrintStreamData *ps = (_PrintStreamData *) user_data;

  if (ps->callback)
    ps->callback (ps->job, ps->user_data, error);

  g_free (ps);
}

static gboolean
lpr_write (GIOChannel *source,
           GIOCondition con,
           gpointer user_data)
{
  gchar buf[_LPR_MAX_CHUNK_SIZE];
  gsize bytes_read;
  GError *error;
  _PrintStreamData *ps = (_PrintStreamData *) user_data;
  gint source_fd;

  error = NULL;

  source_fd = g_io_channel_unix_get_fd (source);

  bytes_read = read (source_fd,
                     buf,
                     _LPR_MAX_CHUNK_SIZE);

   

  if (bytes_read > 0)
    {
      if (write (ps->target_fd, buf, bytes_read) == -1)
        {
          error = g_error_new (EGG_PRINT_ERROR,
                           EGG_PRINT_ERROR_INTERNAL_ERROR, 
                           g_strerror (errno));
        }
    }
  else if (bytes_read == -1)
    {
      error = g_error_new (EGG_PRINT_ERROR,
                           EGG_PRINT_ERROR_INTERNAL_ERROR, 
                           g_strerror (errno));
    }

  if (bytes_read == 0 || error != NULL)
    {
      lpr_print_cb (EGG_PRINT_BACKEND_LPR (ps->backend), &error, user_data);

      return FALSE;
    }

  return TRUE;
}

static void
egg_print_backend_lpr_print_stream (EggPrintBackend *print_backend,
                                     EggPrintJob *job,
				     const gchar *title,
				     gint data_fd,
				     EggPrintJobCompleteFunc callback,
				     gpointer user_data)
{
  GError *error;
  EggPrinterLpr *lpr_printer;
  _PrintStreamData *ps;
  EggPrintSettings *settings;
  GIOChannel *save_channel;  
  gchar *filename;

  lpr_printer = EGG_PRINTER_LPR (egg_print_job_get_printer (job));
  settings = egg_print_job_get_settings (job);

  error = NULL;

  //egg_print_settings_foreach (settings, add_lpr_options, request);

#if 0  
  ps = g_new0 (_PrintStreamData, 1);
  ps->callback = callback;
  ps->user_data = user_data;
  ps->job = job;
#endif

 /* TODO: spawn lpr with pipes and pipe ps file to lpr */
}


static void
egg_print_backend_lpr_iface_init (EggPrintBackendIface *iface)
{
  iface->get_printer_list = lpr_request_printer_list;
  iface->find_printer = egg_print_backend_lpr_find_printer;
  iface->print_stream = egg_print_backend_lpr_print_stream;
  iface->printer_request_details = lpr_printer_request_details;
  iface->printer_create_cairo_surface = lpr_printer_create_cairo_surface;
  iface->printer_get_options = lpr_printer_get_options;
  iface->printer_mark_conflicts = lpr_printer_mark_conflicts;
  iface->printer_get_settings_from_options = lpr_printer_get_settings_from_options;
  iface->printer_prepare_for_print = lpr_printer_prepare_for_print;
  iface->printer_list_papers = lpr_printer_list_papers;
  iface->printer_get_hard_margins = lpr_printer_get_hard_margins;
}

static GList *
lpr_request_printer_list (EggPrintBackend *backend)
{
  GList *l;
  EggPrintBackendLpr *lpr_backend;

  l = NULL;

  lpr_backend = EGG_PRINT_BACKEND_LPR (backend);
  
  if (lpr_backend->printer)
    l = g_list_append (l, lpr_backend->printer);

  return l; 
}

static void
egg_print_backend_lpr_init (EggPrintBackendLpr *backend_lpr)
{
  EggPrinter *printer;
  
  backend_lpr->printer = egg_printer_lpr_new (); 

  printer = EGG_PRINTER (backend_lpr->printer);

  printer->priv->name = g_strdup ("Print to LPR");
  printer->priv->icon_name = g_strdup ("printer");
  printer->priv->is_active = TRUE;
  printer->priv->backend = EGG_PRINT_BACKEND (backend_lpr);
}

static void
egg_print_backend_lpr_finalize (GObject *object)
{
  EggPrintBackendLpr *backend_lpr;

  backend_lpr = EGG_PRINT_BACKEND_LPR (object);

  g_object_unref (backend_lpr->printer);

  backend_parent_class->finalize (object);
}

static void
lpr_printer_request_details (EggPrinter *printer)
{
}

static EggPrinterOptionSet *
lpr_printer_get_options (EggPrinter *printer)
{
  EggPrinterLpr *lpr_printer;
  EggPrinterOptionSet *set;
  EggPrinterOption *option;
  char *n_up[] = {"1", "2", "4", "6", "9", "16" };

  lpr_printer = EGG_PRINTER_LPR (printer);
  
  set = egg_printer_option_set_new ();

  option = egg_printer_option_new ("gtk-n-up", _("Pages Per Sheet"), EGG_PRINTER_OPTION_TYPE_PICKONE);
  egg_printer_option_choices_from_array (option, G_N_ELEMENTS (n_up),
					 n_up, n_up);
  egg_printer_option_set (option, "1");
  egg_printer_option_set_add (set, option);
  g_object_unref (option);

  option = egg_printer_option_new ("main-page-custom-input", _("Command Line Options"), EGG_PRINTER_OPTION_TYPE_ENTRY);
  option->group = g_strdup ("GtkPrintDialogExtention");
  egg_printer_option_set_add (set, option);
  
  if (lpr_printer->options)
    g_object_unref (lpr_printer->options);

  lpr_printer->options = option;

  return set;
}


static gboolean
lpr_printer_mark_conflicts  (EggPrinter          *printer,
			     EggPrinterOptionSet *options)
{
  return FALSE;
}

static void
lpr_printer_get_settings_from_options (EggPrinter *printer,
				       EggPrinterOptionSet *options,
				       EggPrintSettings *settings)
{
 
}

static void
lpr_printer_prepare_for_print (EggPrinter *printer,
			       EggPrintSettings *settings)
{
  EggPageSet page_set;
  double scale;
  
  /* TODO: paper size & orientation */

  if (egg_print_settings_get_collate (settings))
    egg_print_settings_set (settings, "manual-Collate", "True");

  if (egg_print_settings_get_reverse (settings))
    egg_print_settings_set (settings, "manual-OutputOrder", "Reverse");

  if (egg_print_settings_get_num_copies (settings) > 1)
    egg_print_settings_set_int (settings, "manual-copies",
				egg_print_settings_get_num_copies (settings));

  scale = egg_print_settings_get_scale (settings);
  if (scale != 100.0)
    egg_print_settings_set_double (settings, "manual-scale", scale);

  page_set = egg_print_settings_get_page_set (settings);
  if (page_set == EGG_PAGE_SET_EVEN)
    egg_print_settings_set (settings, "manual-page-set", "even");
  else if (page_set == EGG_PAGE_SET_ODD)
    egg_print_settings_set (settings, "manual-page-set", "odd");
}

static void
lpr_printer_get_hard_margins (EggPrinter *printer,
                              double *top,
                              double *bottom,
                              double *left,
                              double *right)
{
  *top = 0;
  *bottom = 0;
  *left = 0;
  *right = 0;
}

static GList *
lpr_printer_list_papers (EggPrinter *printer)
{
  return NULL;
}
