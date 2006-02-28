/* GTK - The GIMP Toolkit
 * eggprintbackendcups.h: Default implementation of EggPrintBackend 
 * for the Common Unix Print System (CUPS)
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
#include <stdlib.h>

#include <config.h>
#include <cups/cups.h>
#include <cups/language.h>
#include <cups/http.h>
#include <cups/ipp.h>
#include <errno.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>

#include "eggprintbackend.h"
#include "eggprintbackendcups.h"

#include "eggprinter.h"
#include "eggprinter-private.h"

#include "eggprintercups.h"
#include "eggprintercups-private.h"

#include "eggcupsutils.h"

#define N_(x) (x)
#define _(x) (x)

typedef struct _EggPrintBackendCupsClass EggPrintBackendCupsClass;

#define EGG_PRINT_BACKEND_CUPS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_BACKEND_CUPS, EggPrintBackendCupsClass))
#define EGG_IS_PRINT_BACKEND_CUPS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_BACKEND_CUPS))
#define EGG_PRINT_BACKEND_CUPS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_BACKEND_CUPS, EggPrintBackendCupsClass))

#define _CUPS_MAX_ATTEMPTS 10 
#define _CUPS_MAX_CHUNK_SIZE 8192

#define _CUPS_MAP_ATTR_INT(attr, v, a) {if (!g_ascii_strcasecmp (attr->name, (a))) v = attr->values[0].integer;}
#define _CUPS_MAP_ATTR_STR(attr, v, a) {if (!g_ascii_strcasecmp (attr->name, (a))) v = g_strdup (attr->values[0].string.text);}


typedef void (* EggPrintCupsResponseCallbackFunc) (EggPrintBackend *print_backend,
                                                   ipp_t *response, 
                                                   gpointer user_data);

typedef enum 
{
  DISPATCH_SETUP,
  DISPATCH_REQUEST,
  DISPATCH_SEND,
  DISPATCH_CHECK,
  DISPATCH_READ,
  DISPATCH_ERROR
} EggPrintCupsDispatchState;

typedef struct 
{
  GSource source;

  http_t *http;
  EggCupsRequest *request;
  EggPrintBackendCups *backend;

} EggPrintCupsDispatchWatch;

struct _EggPrintBackendCupsClass
{
  GObjectClass parent_class;
};

struct _EggPrintBackendCups
{
  GObject parent_instance;

  GHashTable *printers;
};

static GObjectClass *backend_parent_class;

static void                       egg_print_backend_cups_class_init (EggPrintBackendCupsClass          *class);
static void                       egg_print_backend_cups_iface_init (EggPrintBackendIface              *iface);
static void                       egg_print_backend_cups_init       (EggPrintBackendCups               *impl);
static void                       egg_print_backend_cups_finalize   (GObject                           *object);
static void                       _cups_request_printer_list        (EggPrintBackendCups               *print_backend);
static void                       _cups_request_execute             (EggPrintBackendCups               *print_backend,
								     EggCupsRequest                    *request,
								     EggPrintCupsResponseCallbackFunc   callback,
								     gpointer                           user_data,
								     GDestroyNotify                     notify,
								     GError                           **err);
static void                       cups_printer_add_backend_settings (EggPrinter                        *printer,
								     EggPrintBackendSettingSet         *backend_settings,
								     EggPrinterSettings                *settings);
static gboolean                   cups_printer_mark_conflicts       (EggPrinter                        *printer,
								     EggPrintBackendSettingSet         *settings);
static EggPrintBackendSettingSet *cups_printer_get_backend_settings (EggPrinter                        *printer);
static void                       cups_printer_prepare_for_print    (EggPrinter                        *printer,
								     EggPrinterSettings                *settings);

static void
_cups_request_ppd (EggPrintBackend *print_backend,
                   EggPrinter      *printer);


/*
 * EggPrintBackendCups
 */
GType
egg_print_backend_cups_get_type (void)
{
  static GType print_backend_cups_type = 0;

  if (!print_backend_cups_type)
    {
      static const GTypeInfo print_backend_cups_info =
      {
	sizeof (EggPrintBackendCupsClass),
	NULL,		/* base_init */
	NULL,		/* base_finalize */
	(GClassInitFunc) egg_print_backend_cups_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data */
	sizeof (EggPrintBackendCups),
	0,		/* n_preallocs */
	(GInstanceInitFunc) egg_print_backend_cups_init,
      };

      static const GInterfaceInfo print_backend_info =
      {
	(GInterfaceInitFunc) egg_print_backend_cups_iface_init, /* interface_init */
	NULL,			                              /* interface_finalize */
	NULL			                              /* interface_data */
      };

      print_backend_cups_type = g_type_register_static (G_TYPE_OBJECT,
						      "EggPrintBackendCups",
						      &print_backend_cups_info, 0);
      g_type_add_interface_static (print_backend_cups_type,
				   EGG_TYPE_PRINT_BACKEND,
				   &print_backend_info);
    }

  return print_backend_cups_type;
}

/**
 * egg_print_backend_cups_new:
 *
 * Creates a new #EggPrintBackendCups object. #EggPrintBackendCups
 * implements the #EggPrintBackend interface with direct access to
 * the filesystem using Unix/Linux API calls
 *
 * Return value: the new #EggPrintBackendCups object
 **/
EggPrintBackend *
egg_print_backend_cups_new (void)
{
  return g_object_new (EGG_TYPE_PRINT_BACKEND_CUPS, NULL);
}

static void
egg_print_backend_cups_class_init (EggPrintBackendCupsClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  backend_parent_class = g_type_class_peek_parent (class);

  gobject_class->finalize = egg_print_backend_cups_finalize;
}

static cairo_status_t
_cairo_write_to_cups (void *cache_fd_as_pointer,
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
cups_printer_create_cairo_surface (EggPrinter *printer,
				   gdouble width, 
				   gdouble height,
				   gint cache_fd)
{
  cairo_surface_t *surface;
  
  /* TODO: check if it is a ps or pdf printer */
  
  surface = cairo_pdf_surface_create_for_stream  (_cairo_write_to_cups, GINT_TO_POINTER (cache_fd), width, height);

  /* TODO: DPI from settings object? */
  cairo_pdf_surface_set_dpi (surface, 300, 300);

  return surface;
}

static EggPrinter *
egg_print_backend_cups_find_printer (EggPrintBackend *print_backend,
                                     const gchar *printer_name)
{
  EggPrintBackendCups *cups_print_backend;

  cups_print_backend = EGG_PRINT_BACKEND_CUPS (print_backend);
  
  return (EggPrinter *) g_hash_table_lookup (cups_print_backend->printers, 
                                                  printer_name);  
}

typedef struct {
  EggPrintJobCompleteFunc callback;
  EggPrintJob *job;
  gpointer user_data;
} _PrintStreamData;

void
_cups_print_cb (EggPrintBackendCups *print_backend,
                ipp_t *request,
                gpointer user_data)
{
  GError *error = NULL;

  _PrintStreamData *ps = (_PrintStreamData *) user_data;

  if (ps->callback)
    ps->callback (ps->job, ps->user_data, &error);

  g_free (ps);
}

static void
add_cups_options (const char *key,
		  const char *value,
		  gpointer  user_data)
{
  EggCupsRequest *request = user_data;
  
  if (!g_str_has_prefix (key, "cups-"))
    return;

  key = key + strlen("cups-");

  egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_NAME,
				   key, NULL, value);
}

static void
egg_print_backend_cups_print_stream (EggPrintBackend *print_backend,
                                     EggPrintJob *job,
				     const gchar *title,
				     gint data_fd,
				     EggPrintJobCompleteFunc callback,
				     gpointer user_data)
{
  GError *error;
  EggPrinterCups *cups_printer;
  _PrintStreamData *ps;
  EggCupsRequest *request;
  EggPrinterSettings *settings;
  
  cups_printer = EGG_PRINTER_CUPS (egg_print_job_get_printer (job));
  settings = egg_print_job_get_settings (job);

  error = NULL;

  request = egg_cups_request_new (NULL,
                                  EGG_CUPS_POST,
                                  IPP_PRINT_JOB,
				  data_fd,
				  NULL,
				  cups_printer->priv->device_uri);

  egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
                                   NULL, cups_printer->priv->printer_uri);

  egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                                   NULL, cupsUser());

  if (title)
    egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_NAME, "job-name", NULL,
                                     title);

  egg_printer_settings_foreach (settings, add_cups_options, request);
  
  ps = g_new0 (_PrintStreamData, 1);
  ps->callback = callback;
  ps->user_data = user_data;
  ps->job = job;

  _cups_request_execute (EGG_PRINT_BACKEND_CUPS (print_backend),
                         request,
                         (EggPrintCupsResponseCallbackFunc) _cups_print_cb,
                         ps,
                         NULL,
                         &error);

  g_object_unref (settings);
  g_object_unref (cups_printer);
}


static void
egg_print_backend_cups_iface_init   (EggPrintBackendIface *iface)
{
  iface->find_printer = egg_print_backend_cups_find_printer;
  iface->print_stream = egg_print_backend_cups_print_stream;
  iface->printer_create_cairo_surface = cups_printer_create_cairo_surface;
  iface->printer_get_backend_settings = cups_printer_get_backend_settings;
  iface->printer_mark_conflicts = cups_printer_mark_conflicts;
  iface->printer_add_backend_settings = cups_printer_add_backend_settings;
  iface->printer_prepare_for_print = cups_printer_prepare_for_print;
}

static void
egg_print_backend_cups_init (EggPrintBackendCups *backend_cups)
{
  backend_cups->printers = g_hash_table_new_full (g_str_hash, 
                                                  g_str_equal, 
                                                 (GDestroyNotify) g_free,
                                                 (GDestroyNotify) g_object_unref);

  _cups_request_printer_list (backend_cups);
}

static void
egg_print_backend_cups_finalize (GObject *object)
{
  EggPrintBackendCups *backend_cups;

  backend_cups = EGG_PRINT_BACKEND_CUPS (object);

  if (backend_cups->printers)
    g_hash_table_unref (backend_cups->printers);

  backend_parent_class->finalize (object);
}

static gboolean
_cups_dispatch_watch_check (GSource *source)
{
  EggPrintCupsDispatchWatch *dispatch;

  dispatch = (EggPrintCupsDispatchWatch *) source;

  return egg_cups_request_read_write (dispatch->request);
}

static gboolean
_cups_dispatch_watch_prepare (GSource *source,
                              gint *timeout_)
{
  EggPrintCupsDispatchWatch *dispatch;

  dispatch = (EggPrintCupsDispatchWatch *) source;
 

  *timeout_ = 250;
  
  return egg_cups_request_read_write (dispatch->request);
}

static gboolean
_cups_dispatch_watch_dispatch (GSource *source,
                               GSourceFunc callback,
                               gpointer user_data)
{
  EggPrintCupsDispatchWatch *dispatch;
  EggPrintCupsResponseCallbackFunc ep_callback;  
  EggCupsResult *result;
  
  g_assert (callback != NULL);

  ep_callback = (EggPrintCupsResponseCallbackFunc) callback;
  
  dispatch = (EggPrintCupsDispatchWatch *) source;

  result = egg_cups_request_get_result (dispatch->request);

  if (egg_cups_result_is_error (result))
    g_warning (egg_cups_result_get_error_string (result));

  ep_callback (EGG_PRINT_BACKEND (dispatch->backend), egg_cups_result_get_response (result), user_data);

  g_source_unref (source); 
  return FALSE;
}

static void
_cups_dispatch_watch_finalize (GSource *source)
{
  EggPrintCupsDispatchWatch *dispatch;

  dispatch = (EggPrintCupsDispatchWatch *) source;

  egg_cups_request_free (dispatch->request);
}

static GSourceFuncs _cups_dispatch_watch_funcs = 
                              {_cups_dispatch_watch_prepare,
	                       _cups_dispatch_watch_check,
	                       _cups_dispatch_watch_dispatch,
	                       _cups_dispatch_watch_finalize};


static void
_cups_request_execute (EggPrintBackendCups *print_backend,
                       EggCupsRequest *request,
                       EggPrintCupsResponseCallbackFunc callback,
                       gpointer user_data,
                       GDestroyNotify notify,
                       GError **err)
{
  EggPrintCupsDispatchWatch *dispatch;
  
  dispatch = (EggPrintCupsDispatchWatch *) g_source_new (&_cups_dispatch_watch_funcs, 
                                                         sizeof (EggPrintCupsDispatchWatch));

  dispatch->request = request;
  dispatch->backend = print_backend;

  g_source_set_callback ((GSource *) dispatch, (GSourceFunc) callback, user_data, notify);

  g_source_attach ((GSource *) dispatch, NULL);
}

void
_cups_request_printer_info_cb (EggPrintBackendCups *print_backend,
                               ipp_t *response,
                               gpointer user_data)
{
  ipp_attribute_t *attr;
  gchar *printer_name;
  EggPrinterCups *cups_printer;
  EggPrinter *printer;

  char uri[HTTP_MAX_URI],	/* Printer URI */
       method[HTTP_MAX_URI],	/* Method/scheme name */
       username[HTTP_MAX_URI],	/* Username:password */
       hostname[HTTP_MAX_URI],	/* Hostname */
       resource[HTTP_MAX_URI];	/* Resource name */
  int  port;			/* Port number */


  g_assert (EGG_IS_PRINT_BACKEND_CUPS (print_backend));

  printer_name = (gchar *)user_data;
  cups_printer = (EggPrinterCups *) g_hash_table_lookup (print_backend->printers, printer_name);

  if (!cups_printer)
    return;
    
  printer = EGG_PRINTER (cups_printer);

  /* TODO: determine printer type and use correct icon */
  printer->priv->icon_name = g_strdup ("printer-inkjet");
  
  cups_printer->priv->device_uri = g_strdup_printf ("/printers/%s", printer_name);
  for (attr = response->attrs; attr != NULL; attr = attr->next) {
    if (!attr->name)
      continue;

    _CUPS_MAP_ATTR_STR (attr, printer->priv->location, "printer-location");
    _CUPS_MAP_ATTR_STR (attr, printer->priv->description, "printer-info");

    /*_CUPS_MAP_ATTR_STR (attr, cups_printer->priv->device_uri, "device-uri");*/
    _CUPS_MAP_ATTR_STR (attr, cups_printer->priv->printer_uri, "printer-uri-supported");
    _CUPS_MAP_ATTR_STR (attr, printer->priv->state_message, "printer-state-message");
    _CUPS_MAP_ATTR_INT (attr, cups_printer->priv->state, "printer-state");
    _CUPS_MAP_ATTR_INT (attr, printer->priv->job_count, "queued-job-count");

  }

  httpSeparate(cups_printer->priv->printer_uri, method, username, hostname,
	       &port, resource);

  gethostname(uri, sizeof(uri));

  if (strcasecmp(uri, hostname) == 0)
    strcpy(hostname, "localhost");

  cups_printer->priv->hostname = g_strdup (hostname);
  cups_printer->priv->port = port;

  if (printer->priv->is_new)
    {
      g_signal_emit_by_name (EGG_PRINT_BACKEND (print_backend), "printer-added", printer);
      printer->priv->is_new = FALSE;
    }
}


static void
_cups_request_printer_info (EggPrintBackendCups *print_backend,
                            const gchar *printer_name)
{
  GError *error;
  EggCupsRequest *request;
  gchar *printer_uri;

  error = NULL;

  request = egg_cups_request_new (NULL,
                                  EGG_CUPS_POST,
                                  IPP_GET_PRINTER_ATTRIBUTES,
				  0,
				  NULL,
				  NULL);

  printer_uri = g_strdup_printf ("ipp://localhost/printers/%s",
                                  printer_name);
  egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_URI,
                                   "printer-uri", NULL, printer_uri);

  g_free (printer_uri);

  _cups_request_execute (print_backend,
                         request,
                         (EggPrintCupsResponseCallbackFunc) _cups_request_printer_info_cb,
                         g_strdup (printer_name),
                         (GDestroyNotify) g_free,
                         &error);
 
}

void
_cups_request_printer_list_cb (EggPrintBackendCups *print_backend,
                               ipp_t *response,
                               gpointer user_data)
{
  ipp_attribute_t *attr;

  g_assert (EGG_IS_PRINT_BACKEND_CUPS (print_backend));

  attr = ippFindAttribute (response, "printer-name", IPP_TAG_NAME);
  while (attr) 
    {
      EggPrinterCups *cups_printer;
      EggPrinter *printer;
      
      cups_printer = (EggPrinterCups *) g_hash_table_lookup (print_backend->printers, 
                                                             attr->values[0].string.text);
      printer = cups_printer ? EGG_PRINTER (cups_printer) : NULL;

      if (!cups_printer)
        {
	  cups_printer = egg_printer_cups_new ();
	  printer = EGG_PRINTER (cups_printer);
          printer->priv->name = g_strdup (attr->values[0].string.text);
          printer->priv->backend = EGG_PRINT_BACKEND (print_backend);

          g_hash_table_insert (print_backend->printers,
                               g_strdup (printer->priv->name), 
                               cups_printer); 
        }
    
      _cups_request_printer_info (print_backend, egg_printer_get_name (printer));
      
      attr = ippFindNextAttribute (response, 
                                   "printer-name",
                                   IPP_TAG_NAME);
    }

}

static void
_cups_request_printer_list (EggPrintBackendCups *print_backend)
{
  GError *error;
  EggCupsRequest *request;

  error = NULL;

  request = egg_cups_request_new (NULL,
                                  EGG_CUPS_POST,
                                  CUPS_GET_PRINTERS,
				  0,
				  NULL,
				  NULL);

  _cups_request_execute (print_backend,
                         request,
                         (EggPrintCupsResponseCallbackFunc) _cups_request_printer_list_cb,
			 request,
			 NULL,
                         &error);

}

typedef struct 
{
  EggPrinterCups *printer;
  gint ppd_fd;
  gchar *ppd_filename;
} GetPPDData;

static void
get_ppd_data_free (GetPPDData *data)
{
  close (data->ppd_fd);
  unlink (data->ppd_filename);
  g_free (data->ppd_filename);
  g_object_unref (data->printer);
  g_free (data);
}

static void
_cups_request_ppd_cb (EggPrintBackendCups *print_backend,
                      ipp_t *response,
                      GetPPDData *data)
{
  data->printer->priv->ppd_file = ppdOpenFile (data->ppd_filename);
  _egg_printer_emit_settings_retrieved (EGG_PRINTER (data->printer));
}

static void
_cups_request_ppd (EggPrintBackend *print_backend,
                   EggPrinter      *printer)
{
  GError *error;
  EggPrinterCups *cups_printer;
  EggCupsRequest *request;
  gchar *resource;
  http_t *http;
  GetPPDData *data;
  
  cups_printer = EGG_PRINTER_CUPS (printer);

  error = NULL;

  http = httpConnectEncrypt(cups_printer->priv->hostname, 
                            cups_printer->priv->port,
                            cupsEncryption());

  data = g_new0 (GetPPDData, 1);

  data->ppd_fd = g_file_open_tmp ("eggprint_ppd_XXXXXX", 
                                  &data->ppd_filename, 
                                  &error);

  if (error != NULL)
    {
      g_warning ("%s", error->message);
      g_error_free (error);
      httpClose (http);
      g_free (data);
      return;
    }
    
  fchmod (data->ppd_fd, S_IRUSR | S_IWUSR);

  data->printer = g_object_ref (printer);

  resource = g_strdup_printf ("/printers/%s.ppd", printer->priv->name);
  request = egg_cups_request_new (http,
                                  EGG_CUPS_GET,
				  0,
                                  data->ppd_fd,
				  cups_printer->priv->hostname,
				  resource);

  g_free (resource);
  
  _cups_request_execute (EGG_PRINT_BACKEND_CUPS (print_backend),
                         request,
                         (EggPrintCupsResponseCallbackFunc) _cups_request_ppd_cb,
                         data,
                         (GDestroyNotify)get_ppd_data_free,
                         &error);
}

char *
ppd_text_to_utf8 (ppd_file_t *ppd_file, const char *text)
{
  const char *encoding = NULL;
  char *res;
  
  if (g_ascii_strcasecmp (ppd_file->lang_encoding, "UTF-8") == 0)
    {
      return g_strdup (text);
    }
  else if (g_ascii_strcasecmp (ppd_file->lang_encoding, "ISOLatin1") == 0)
    {
      encoding = "ISO-8859-1";
    }
  else if (g_ascii_strcasecmp (ppd_file->lang_encoding, "ISOLatin2") == 0)
    {
      encoding = "ISO-8859-2";
    }
  else if (g_ascii_strcasecmp (ppd_file->lang_encoding, "ISOLatin5") == 0)
    {
      encoding = "ISO-8859-5";
    }
  else if (g_ascii_strcasecmp (ppd_file->lang_encoding, "JIS83-RKSJ") == 0)
    {
      encoding = "SHIFT-JIS";
    }
  else if (g_ascii_strcasecmp (ppd_file->lang_encoding, "MacStandard") == 0)
    {
      encoding = "MACINTOSH";
    }
  else if (g_ascii_strcasecmp (ppd_file->lang_encoding, "WindowsANSI") == 0)
    {
      encoding = "WINDOWS-1252";
    }
  else 
    {
      /* Fallback, try iso-8859-1... */
      encoding = "ISO-8859-1";
    }

  res = g_convert (text, -1, "UTF-8", encoding, NULL, NULL, NULL);

  if (res == NULL)
    {
      g_warning ("unable to convert PPD text");
      res = g_strdup ("???");
    }
  
  return res;
}

/* TODO: Add more translations for common settings here */

static const struct {
  const char *keyword;
  const char *translation;
} cups_option_translations[] = {
  { "Duplex", N_("Two Sided") },
};


static const struct {
  const char *keyword;
  const char *choice;
  const char *translation;
} cups_choice_translations[] = {
  { "Duplex", "None", N_("One Sided") },
};

static const struct {
  const char *ppd_keyword;
  const char *name;
} setting_names[] = {
  {"Duplex", "gtk-duplex"},
  {"PageSize", "gtk-paper-size"},
  {"MediaType", "gtk-paper-type"},
  {"InputSlot", "gtk-paper-source"},
  {"OutputBin", "gtk-output-tray"},
};

/* keep sorted when changing */
static const char *color_option_whitelist[] = {
  "BRColorEnhancement",
  "BRColorMatching",
  "BRColorMatching",
  "BRColorMode",
  "BRGammaValue",
  "BRImprovedGray",
  "BlackSubstitution",
  "ColorModel",
  "HPCMYKInks",
  "HPCSGraphics",
  "HPCSImages",
  "HPCSText",
  "HPColorSmart",
  "RPSBlackMode",
  "RPSBlackOverPrint",
  "Rcmyksimulation",
};

/* keep sorted when changing */
static const char *color_group_whitelist[] = {
  "ColorPage",
  "FPColorWise1",
  "FPColorWise2",
  "FPColorWise3",
  "FPColorWise4",
  "FPColorWise5",
};
  
/* keep sorted when changing */
static const char *image_quality_option_whitelist[] = {
  "BRDocument",
  "BRHalfTonePattern",
  "BRNormalPrt",
  "BRPrintQuality",
  "BitsPerPixel",
  "Darkness",
  "Dithering",
  "EconoMode",
  "Economode",
  "HPEconoMode",
  "HPEdgeControl",
  "HPGraphicsHalftone",
  "HPHalftone",
  "HPLJDensity",
  "HPPhotoHalftone",
  "OutputMode",
  "REt",
  "RPSBitsPerPixel",
  "RPSDitherType",
  "Resolution",
  "ScreenLock",
  "Smoothing",
  "TonerSaveMode",
  "UCRGCRForImage",
};

/* keep sorted when changing */
static const char *image_quality_group_whitelist[] = {
  "FPImageQuality1",
  "FPImageQuality2",
  "FPImageQuality3",
  "ImageQualityPage",
};

/* keep sorted when changing */
static const char * finishing_option_whitelist[] = {
  "BindColor",
  "BindEdge",
  "BindType",
  "BindWhen",
  "Booklet",
  "FoldType",
  "FoldWhen",
  "HPStaplerOptions",
  "Jog",
  "Slipsheet",
  "Sorter",
  "StapleLocation",
  "StapleOrientation",
  "StapleWhen",
  "StapleX",
  "StapleY",
};

/* keep sorted when changing */
static const char *finishing_group_whitelist[] = {
  "FPFinishing1",
  "FPFinishing2",
  "FPFinishing3",
  "FPFinishing4",
  "FinishingPage",
};

/* keep sorted when changing */
static const char *cups_option_blacklist[] = {
  "Collate",
  "Copies", 
  "OutputOrder",
  "PageRegion",
};

char *
get_option_text (ppd_file_t *ppd_file, ppd_option_t *option)
{
  int i;
  char *utf8;
  
  for (i = 0; i < G_N_ELEMENTS (cups_option_translations); i++)
    {
      if (strcmp (cups_option_translations[i].keyword, option->keyword) == 0)
	return g_strdup (_(cups_option_translations[i].translation));
    }

  utf8 = ppd_text_to_utf8 (ppd_file, option->text);

  /* Some ppd files have spaces in the text before the colon */
  g_strchomp (utf8);
  
  return utf8;
}

char *
get_choice_text (ppd_file_t *ppd_file, ppd_choice_t *choice)
{
  int i;
  ppd_option_t *option = choice->option;
  const char *keyword = option->keyword;
  
  for (i = 0; i < G_N_ELEMENTS (cups_choice_translations); i++)
    {
      if (strcmp (cups_choice_translations[i].keyword, keyword) == 0 &&
	  strcmp (cups_choice_translations[i].choice, choice->choice) == 0)
	return g_strdup (_(cups_choice_translations[i].translation));
    }
  return ppd_text_to_utf8 (ppd_file, choice->text);
}

static gboolean
group_has_option (ppd_group_t *group, ppd_option_t *option)
{
  int i;

  if (group == NULL)
    return FALSE;
  
  if (group->num_options > 0 &&
      option >= group->options && option < group->options + group->num_options)
    return TRUE;
  
  for (i = 0; i < group->num_subgroups; i++)
    {
      if (group_has_option (&group->subgroups[i],option))
	return TRUE;
    }
  return FALSE;
}

static gboolean
value_is_off (const char *value)
{
  return  (strcasecmp (value, "None") == 0 ||
	   strcasecmp (value, "Off") == 0 ||
	   strcasecmp (value, "False") == 0);
}

static int
availible_choices (ppd_file_t *ppd,
		   ppd_option_t *option,
		   ppd_choice_t ***availible,
		   gboolean keep_if_only_one_option)
{
  ppd_option_t *other_option;
  int i, j;
  char *conflicts;
  ppd_const_t *constraint;
  const char *choice, *other_choice;
  ppd_option_t *option1, *option2;
  ppd_group_t *installed_options;
  int num_conflicts;
  gboolean all_default;

  if (availible)
    *availible = NULL;

  conflicts = g_new0 (char, option->num_choices);

  installed_options = NULL;
  for (i = 0; i < ppd->num_groups; i++)
    {
      if (strcmp (ppd->groups[i].name, "InstallableOptions") == 0)
	{
	  installed_options = &ppd->groups[i];
	  break;
	}
    }

  for (i = ppd->num_consts, constraint = ppd->consts; i > 0; i--, constraint++)
    {
      option1 = ppdFindOption (ppd, constraint->option1);
      if (option1 == NULL)
	continue;

      option2 = ppdFindOption (ppd, constraint->option2);
      if (option2 == NULL)
	continue;

      if (option == option1)
	{
	  choice = constraint->choice1;
	  other_option = option2;
	  other_choice = constraint->choice2;
	}
      else if (option == option2)
	{
	  choice = constraint->choice2;
	  other_option = option1;
	  other_choice = constraint->choice1;
	}
      else
	continue;

      /* We only care of conflicts with installed_options */
      if (!group_has_option (installed_options, other_option))
	continue;

      if (*other_choice == 0)
	{
	  /* Conflict only if the installed option is not off */
	  if (value_is_off (other_option->defchoice))
	    continue;
	}
      /* Conflict if the installed option has the specified default */
      else if (strcasecmp (other_choice, other_option->defchoice) != 0)
	continue;

      if (*choice == 0)
	{
	  /* Conflict with all non-off choices */
	  for (j = 0; j < option->num_choices; j++)
	    {
	      if (!value_is_off (option->choices[j].choice))
		conflicts[j] = 1;
	    }
	}
      else
	{
	  for (j = 0; j < option->num_choices; j++)
	    {
	      if (strcasecmp (option->choices[j].choice, choice) == 0)
		conflicts[j] = 1;
	    }
	}
    }

  num_conflicts = 0;
  all_default = TRUE;
  for (j = 0; j < option->num_choices; j++)
    {
      if (conflicts[j])
	num_conflicts++;
      else if (strcmp (option->choices[j].choice, option->defchoice) != 0)
	all_default = FALSE;
    }

  if (all_default && !keep_if_only_one_option)
    return 0;
  
  if (num_conflicts == option->num_choices)
    return 0;
    
  if (availible)
    {
      *availible = g_new (ppd_choice_t *, option->num_choices - num_conflicts);

      i = 0;
      for (j = 0; j < option->num_choices; j++)
	{
	  if (!conflicts[j]) {
	    (*availible)[i++] = &option->choices[j];
	  }
	}
    }
  
  return option->num_choices - num_conflicts;
}

static EggPrintBackendSetting *
create_pickone_setting (ppd_file_t *ppd_file,
			ppd_option_t *option,
			const char *gtk_name)
{
  EggPrintBackendSetting *setting;
  ppd_choice_t **availible;
  char *label;
  int n_choices;
  int i;

  g_assert (option->ui == PPD_UI_PICKONE);
  
  setting = NULL;

  n_choices = availible_choices (ppd_file, option, &availible, g_str_has_prefix (gtk_name, "gtk-"));
  if (n_choices > 0)
    {
      label = get_option_text (ppd_file, option);
      setting = egg_print_backend_setting_new (gtk_name, label,
					       EGG_PRINT_BACKEND_SETTING_TYPE_PICKONE);
      g_free (label);
      
      egg_print_backend_setting_allocate_choices (setting, n_choices);
      for (i = 0; i < n_choices; i++)
	{
	  setting->choices[i] = g_strdup (availible[i]->choice);
	  setting->choices_display[i] = get_choice_text (ppd_file, availible[i]);
	}
      egg_print_backend_setting_set (setting, option->defchoice);
    }
  else
    g_warning ("Ignoring pickone %s\n", option->text);
  g_free (availible);

  return setting;
}

static EggPrintBackendSetting *
create_boolean_setting (ppd_file_t *ppd_file,
			ppd_option_t *option,
			const char *gtk_name)
{
  EggPrintBackendSetting *setting;
  ppd_choice_t **availible;
  char *label;
  int n_choices;

  g_assert (option->ui == PPD_UI_BOOLEAN);
  
  setting = NULL;

  n_choices = availible_choices (ppd_file, option, &availible, g_str_has_prefix (gtk_name, "gtk-"));
  if (n_choices == 2)
    {
      label = get_option_text (ppd_file, option);
      setting = egg_print_backend_setting_new (gtk_name, label,
					       EGG_PRINT_BACKEND_SETTING_TYPE_BOOLEAN);
      g_free (label);
      
      egg_print_backend_setting_allocate_choices (setting, 2);
      setting->choices[0] = g_strdup ("True");
      setting->choices_display[0] = g_strdup ("True");
      setting->choices[1] = g_strdup ("True");
      setting->choices_display[1] = g_strdup ("True");
      
      egg_print_backend_setting_set (setting, option->defchoice);
    }
  else
    g_warning ("Ignoring boolean %s\n", option->text);
  g_free (availible);

  return setting;
}

char *
get_setting_name (const char *keyword)
{
  int i;

  for (i = 0; i < G_N_ELEMENTS (setting_names); i++)
    if (strcmp (setting_names[i].ppd_keyword, keyword) == 0)
      return g_strdup (setting_names[i].name);

  return g_strdup_printf ("cups-%s", keyword);
}

static int
strptr_cmp(const void *a, const void *b)
{
  char **aa = (char **)a;
  char **bb = (char **)b;
  return strcmp(*aa, *bb);
}


static gboolean
string_in_table (char *str, const char *table[], int table_len)
{
  return bsearch (&str, table, table_len, sizeof (char *), (void *)strptr_cmp) != NULL;
}

#define STRING_IN_TABLE(_str, _table) (string_in_table (_str, _table, G_N_ELEMENTS (_table)))

static void
handle_option (EggPrintBackendSettingSet *set,
	       ppd_file_t *ppd_file,
	       ppd_option_t *option,
	       ppd_group_t *toplevel_group)
{
  EggPrintBackendSetting *setting;
  char *name;

  if (STRING_IN_TABLE (option->keyword, cups_option_blacklist))
    return;
  
  name = get_setting_name (option->keyword);

  setting = NULL;
  if (option->ui == PPD_UI_PICKONE)
    {
      setting = create_pickone_setting (ppd_file, option, name);
    }
  else if (option->ui == PPD_UI_BOOLEAN)
    {
      setting = create_boolean_setting (ppd_file, option, name);
    }
  else
    g_warning ("Ignored pickmany setting %s\n", option->text);
  
  
  if (setting)
    {
      if (STRING_IN_TABLE (toplevel_group->name,
			   color_group_whitelist) ||
	  STRING_IN_TABLE (option->keyword,
			   color_option_whitelist))
	{
	  setting->group = g_strdup ("ColorPage");
	}
      else if (STRING_IN_TABLE (toplevel_group->name,
				image_quality_group_whitelist) ||
	       STRING_IN_TABLE (option->keyword,
				image_quality_option_whitelist))
	{
	  setting->group = g_strdup ("ImageQualityPage");
	}
      else if (STRING_IN_TABLE (toplevel_group->name,
				finishing_group_whitelist) ||
	       STRING_IN_TABLE (option->keyword,
				finishing_option_whitelist))
	{
	  setting->group = g_strdup ("FinishingPage");
	}
      else
	{
	  setting->group = g_strdup (toplevel_group->text);
	}
      egg_print_backend_setting_set_add (set, setting);
    }
  
  g_free (name);
}

static void
handle_group (EggPrintBackendSettingSet *set,
	      ppd_file_t *ppd_file,
	      ppd_group_t *group,
	      ppd_group_t *toplevel_group)
{
  int i;

  /* Ignore installable options */
  if (strcmp (toplevel_group->name, "InstallableOptions") == 0)
    return;
  
  for (i = 0; i < group->num_options; i++)
    handle_option (set, ppd_file, &group->options[i], toplevel_group);

  for (i = 0; i < group->num_subgroups; i++)
    handle_group (set, ppd_file, &group->subgroups[i], toplevel_group);

}

static EggPrintBackendSettingSet *
cups_printer_get_backend_settings (EggPrinter *printer)
{
  EggPrintBackendSettingSet *set;
  EggPrintBackendSetting *setting;
  ppd_file_t *ppd_file;
  int i;
  char *n_up[] = {"1", "2", "4", "6", "9", "16" };

  set = egg_print_backend_setting_set_new ();

  /* Cups specific, non-ppd related settings */

  setting = egg_print_backend_setting_new ("gtk-n-up", _("Pages Per Sheet"), EGG_PRINT_BACKEND_SETTING_TYPE_PICKONE);
  egg_print_backend_setting_choices_from_array (setting, G_N_ELEMENTS (n_up),
						n_up, n_up);
  egg_print_backend_setting_set (setting, "1");
  egg_print_backend_setting_set_add (set, setting);
  g_object_unref (setting);


  /* Printer (ppd) specific settings */
  
  ppd_file = egg_printer_cups_get_ppd (EGG_PRINTER_CUPS (printer));

  if (ppd_file)
    {
      ppdMarkDefaults (ppd_file);

      for (i = 0; i < ppd_file->num_groups; i++)
        handle_group (set, ppd_file, &ppd_file->groups[i], &ppd_file->groups[i]);
    }
  else
    { 
       _cups_request_ppd (printer->priv->backend, printer);
    }

  return set;
}


static void
mark_option_from_set (EggPrintBackendSettingSet *set,
		      ppd_file_t *ppd_file,
		      ppd_option_t *option)
{
  EggPrintBackendSetting *setting;
  char *name = get_setting_name (option->keyword);

  setting = egg_print_backend_setting_set_lookup (set, name);

  if (setting)
    ppdMarkOption (ppd_file, option->keyword, setting->value);
  
  g_free (name);
}


static void
mark_group_from_set (EggPrintBackendSettingSet *set,
		     ppd_file_t *ppd_file,
		     ppd_group_t *group)
{
  int i;

  for (i = 0; i < group->num_options; i++)
    mark_option_from_set (set, ppd_file, &group->options[i]);

  for (i = 0; i < group->num_subgroups; i++)
    mark_group_from_set (set, ppd_file, &group->subgroups[i]);
}

static void
set_conflicts_from_option (EggPrintBackendSettingSet *set,
			   ppd_file_t *ppd_file,
			   ppd_option_t *option)
{
  EggPrintBackendSetting *setting;
  char *name;
  if (option->conflicted)
    {
      name = get_setting_name (option->keyword);
      setting = egg_print_backend_setting_set_lookup (set, name);

      if (setting)
	{
	  egg_print_backend_setting_set_has_conflict (setting, TRUE);
	}
      else
	g_warning ("conflict for option %s ignored", option->keyword);
      
      g_free (name);
    }
}

static void
set_conflicts_from_group (EggPrintBackendSettingSet *set,
			  ppd_file_t *ppd_file,
			  ppd_group_t *group)
{
  int i;

  for (i = 0; i < group->num_options; i++)
    set_conflicts_from_option (set, ppd_file, &group->options[i]);

  for (i = 0; i < group->num_subgroups; i++)
    set_conflicts_from_group (set, ppd_file, &group->subgroups[i]);
}

static gboolean
cups_printer_mark_conflicts  (EggPrinter                *printer,
			      EggPrintBackendSettingSet *settings)
{
  ppd_file_t *ppd_file;
  int num_conflicts;
  int i;
  
  ppd_file = egg_printer_cups_get_ppd (EGG_PRINTER_CUPS (printer));
  ppdMarkDefaults (ppd_file);

  for (i = 0; i < ppd_file->num_groups; i++)
    mark_group_from_set (settings, ppd_file, &ppd_file->groups[i]);

  num_conflicts = ppdConflicts (ppd_file);

  if (num_conflicts > 0)
    {
      for (i = 0; i < ppd_file->num_groups; i++)
	set_conflicts_from_group (settings, ppd_file, &ppd_file->groups[i]);
    }
 
  return num_conflicts > 0;
}

struct BackendSettingData {
  EggPrinter *printer;
  EggPrintBackendSettingSet *backend_settings;
  EggPrinterSettings *settings;
  ppd_file_t *ppd_file;
};

typedef struct {
  const char *cups;
  const char *standard;
} NameMapping;

static void
map_cups_settings (const char *value,
		   NameMapping table[],
		   int n_elements,
		   EggPrinterSettings *settings,
		   const char *standard_name,
		   const char *cups_name)
{
  int i;
  char *name;

  for (i = 0; i < n_elements; i++)
    {
      if (table[i].cups == NULL && table[i].standard == NULL)
	{
	  egg_printer_settings_set (settings,
				    standard_name,
				    value);
	  break;
	}
      else if (table[i].cups == NULL && table[i].standard != NULL)
	{
	  if (value_is_off (value))
	    {
	      egg_printer_settings_set (settings,
					standard_name,
					table[i].standard);
	      break;
	    }
	}
      else if (strcmp (table[i].cups, value) == 0)
	{
	  egg_printer_settings_set (settings,
				    standard_name,
				    table[i].standard);
	  break;
	}
    }

  /* Always set the corresponding cups-specific setting */
  name = g_strdup_printf ("cups-%s", cups_name);
  egg_printer_settings_set (settings, name, value);
  g_free (name);
}
      

static void
foreach_backend_setting (EggPrintBackendSetting  *setting,
			 gpointer                 user_data)
{
  struct BackendSettingData *data = user_data;
  EggPrinterSettings *settings = data->settings;
  const char *value;

  value = setting->value;

  /* TODO: paper size, margin */
  
  if (strcmp (setting->name, "gtk-paper-source") == 0)
    {
      NameMapping map[] = {
	{ "Lower", "lower"},
	{ "Middle", "middle"},
	{ "Upper", "upper"},
	{ "Rear", "rear"},
	{ "Envelope", "envelope"},
	{ "Cassette", "cassette"},
	{ "LargeCapacity", "large-capacity"},
	{ "AnySmallFormat", "small-format"},
	{ "AnyLargeFormat", "large-format"},
	{ NULL, NULL}
      };
      map_cups_settings (value, map, G_N_ELEMENTS (map),
			 settings, EGG_PRINTER_SETTINGS_DEFAULT_SOURCE, "InputSlot");
    }
  else if (strcmp (setting->name, "gtk-output-tray") == 0)
    {
      NameMapping map[] = {
	{ "Upper", "upper"},
	{ "Lower", "lower"},
	{ "Rear", "rear"},
	{ NULL, NULL}
      };
      map_cups_settings (value, map, G_N_ELEMENTS (map),
			 settings, EGG_PRINTER_SETTINGS_OUTPUT_BIN, "OutputBin");
    }
  else if (strcmp (setting->name, "gtk-duplex") == 0)
    {
      NameMapping map[] = {
	{ "DuplexTumble", "vertical" },
	{ "DuplexNoTumble", "horizontal" },
	{ NULL, "simplex" }
      };
      map_cups_settings (value, map, G_N_ELEMENTS (map),
			 settings, EGG_PRINTER_SETTINGS_DUPLEX, "Duplex");
    }
  else if (strcmp (setting->name, "cups-OutputMode") == 0)
    {
      NameMapping map[] = {
	{ "Standard", "normal" },
	{ "Normal", "normal" },
	{ "Draft", "draft" },
	{ "Fast", "draft" },
      };
      map_cups_settings (value, map, G_N_ELEMENTS (map),
			 settings, EGG_PRINTER_SETTINGS_QUALITY, "OutputMode");
    }
  else if (strcmp (setting->name, "cups-Resolution") == 0)
    {
      int res = atoi (value);
      /* TODO: What if resolution is on XXXxYYYdpi form? */
      if (res != 0)
	egg_printer_settings_set_resolution (settings, res);
      egg_printer_settings_set (settings, setting->name, value);
    }
  else if (strcmp (setting->name, "cups-MediaType") == 0)
    {
      NameMapping map[] = {
	{ "Transparency", "transparency"},
	{ "Standard", "stationery"},
	{ NULL, NULL}
      };
      map_cups_settings (value, map, G_N_ELEMENTS (map),
			 settings, EGG_PRINTER_SETTINGS_MEDIA_TYPE, "MediaType");
    }
  else if (strcmp (setting->name, "gtk-n-up") == 0)
    {
      egg_printer_settings_set (settings, "cups-number-up", value);
    }
  else if (g_str_has_prefix (setting->name, "cups-"))
    {
      egg_printer_settings_set (settings, setting->name, value);
    }
}


static void
cups_printer_add_backend_settings (EggPrinter *printer,
				   EggPrintBackendSettingSet *backend_settings,
				   EggPrinterSettings *settings)
{
  struct BackendSettingData data;

  data.printer = printer;
  data.backend_settings = backend_settings;
  data.settings = settings;
  data.ppd_file = egg_printer_cups_get_ppd (EGG_PRINTER_CUPS (printer));
  
  egg_print_backend_setting_set_foreach (backend_settings, 
					 foreach_backend_setting, &data);
}

static void
cups_printer_prepare_for_print (EggPrinter *printer,
				EggPrinterSettings *settings)
{
  EggPageSet page_set;
  double scale;
  
  /* TODO: paper size & orientation */

  if (egg_printer_settings_get_collate (settings))
    egg_printer_settings_set (settings, "cups-Collate", "True");

  if (egg_printer_settings_get_reverse (settings))
    egg_printer_settings_set (settings, "cups-OutputOrder", "Reverse");

  if (egg_printer_settings_get_num_copies (settings) > 1)
    egg_printer_settings_set_int (settings, "cups-copies",
				  egg_printer_settings_get_num_copies (settings));

  scale = egg_printer_settings_get_scale (settings);
  if (scale != 100.0)
    egg_printer_settings_set_double (settings, "manual-scale", scale);

  page_set = egg_printer_settings_get_page_set (settings);
  if (page_set == EGG_PAGE_SET_EVEN)
    egg_printer_settings_set (settings, "cups-page-set", "even");
  else if (page_set == EGG_PAGE_SET_ODD)
    egg_printer_settings_set (settings, "cups-page-set", "odd");
}

