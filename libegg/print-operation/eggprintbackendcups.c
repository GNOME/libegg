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
  ipp_t *request;
  ipp_t *response;
  gchar *resource;
  EggPrintBackendCups *backend;

  gint attempts;
  EggPrintCupsDispatchState state;

  gint data_fd;                    /* Open file descriptor of the data we wish to send */
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

static void egg_print_backend_cups_class_init   (EggPrintBackendCupsClass *class);
static void egg_print_backend_cups_iface_init   (EggPrintBackendIface     *iface);
static void egg_print_backend_cups_init         (EggPrintBackendCups      *impl);
static void egg_print_backend_cups_finalize     (GObject                  *object);
static EggPrintBackendSettingSet * egg_print_backend_cups_create_settings (EggPrintBackend *print_backend,
									   EggPrinter *printer);
static gboolean egg_print_backend_cups_mark_conflicts  (EggPrintBackend           *print_backend,
							EggPrinter                *printer,
							EggPrintBackendSettingSet *settings);
static void _cups_request_printer_list (EggPrintBackendCups *print_backend);
static ipp_t * _cups_request_new     (int operation_id);
static void    _cups_request_execute (EggPrintBackendCups *print_backend,
                                      ipp_t               *request,
		                      gint                 data_fd,
                                      const char          *server, 
                                      const char          *path,
                                      EggPrintCupsResponseCallbackFunc callback,
                                      gpointer             user_data,
                                      GDestroyNotify       notify,
                                      GError             **err);


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
  if (write (cache_fd, data, length) != -1); 
    result = CAIRO_STATUS_SUCCESS;

  return result;
}


static cairo_surface_t *
egg_print_backend_cups_printer_create_cairo_surface (EggPrintBackend *backend, 
                                                     EggPrinter *printer,
                                                     gdouble width, 
                                                     gdouble height,
						     gint cache_fd)
{
  cairo_surface_t *surface;
  EggPrintBackendCups *cups_backend;
  
  /* TODO: check if it is a ps or pdf printer */
  
  cups_backend = EGG_PRINT_BACKEND_CUPS (backend);
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
  EggPrinterSendCompleteFunc callback;
  EggPrinter *printer;
  gpointer user_data;
} _PrintStreamData;

void
_cups_print_cb (EggPrintBackendCups *print_backend,
                               ipp_t *response,
                               gpointer user_data)
{
  GError *error = NULL;

  _PrintStreamData *ps = (_PrintStreamData *) user_data;

  if (ps->callback)
    ps->callback (ps->printer, ps->user_data, &error);

  g_free (ps);
}

static void
egg_print_backend_cups_print_stream (EggPrintBackend *print_backend,
                                     EggPrinter *printer,
				     const gchar *title,
				     gint data_fd,
				     EggPrinterSendCompleteFunc callback,
				     gpointer user_data)
{
  GError *error;
  ipp_t *request;
  EggPrinterCups *cups_printer;
  _PrintStreamData *ps;
  
  cups_printer = EGG_PRINTER_CUPS (printer);

  error = NULL;

  request = _cups_request_new (IPP_PRINT_JOB);

  ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_URI, "printer-uri",
               NULL, cups_printer->priv->printer_uri);

  ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
               NULL, cupsUser());

  if (title)
    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "job-name", NULL,
                 title);

  /* TODO: encode options here */

  ps = g_new0 (_PrintStreamData, 1);
  ps->callback = callback;
  ps->user_data = user_data;
  ps->printer = printer;

  g_message ("P: %s, D: %s", cups_printer->priv->printer_uri, cups_printer->priv->device_uri);
  _cups_request_execute (EGG_PRINT_BACKEND_CUPS (print_backend),
                         request,
			 data_fd,
                         NULL, 
                         cups_printer->priv->device_uri,
                         (EggPrintCupsResponseCallbackFunc) _cups_print_cb,
                         ps,
                         NULL,
                         &error);

}


static void
egg_print_backend_cups_iface_init   (EggPrintBackendIface *iface)
{
  iface->printer_create_cairo_surface = egg_print_backend_cups_printer_create_cairo_surface;
  iface->find_printer = egg_print_backend_cups_find_printer;
  iface->print_stream = egg_print_backend_cups_print_stream;
  iface->create_settings = egg_print_backend_cups_create_settings;
  iface->mark_conflicts = egg_print_backend_cups_mark_conflicts;
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

static ipp_t *
_cups_request_new (int operation_id)
{
	ipp_t *request;
	cups_lang_t *language;
	
	language = cupsLangDefault ();
	request = ippNew ();
	request->request.op.operation_id = operation_id;
	request->request.op.request_id = 1;
	
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_CHARSET,
		     "attributes-charset", 
		     NULL, "utf-8");
	
	ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE,
		     "attributes-natural-language", 
		     NULL, language->language);
	cupsLangFree (language);

	return request;
}

static gboolean
_cups_dispatch_watch_check (GSource *source)
{
  EggPrintCupsDispatchWatch *dispatch;

  dispatch = (EggPrintCupsDispatchWatch *) source;

  dispatch->attempts++;
  if (dispatch->attempts > _CUPS_MAX_ATTEMPTS)
    {
      g_warning ("Max attempts reached");
      goto fail;
    }

  switch (dispatch->state) 
    {
    gchar length[255];
    http_status_t http_status;
    ipp_state_t ipp_status;
    struct stat data_info;

    case DISPATCH_SETUP:
      if (dispatch->data_fd != 0)
        {
          fstat (dispatch->data_fd, &data_info);
	  sprintf (length, "%lu", (unsigned long)ippLength(dispatch->request) + data_info.st_size);

	}
      else
	{
          sprintf (length, "%lu", (unsigned long)ippLength(dispatch->request));
        }
	
      httpClearFields(dispatch->http);
      httpSetField(dispatch->http, HTTP_FIELD_CONTENT_LENGTH, length);
      httpSetField(dispatch->http, HTTP_FIELD_CONTENT_TYPE, "application/ipp");
      httpSetField(dispatch->http, HTTP_FIELD_AUTHORIZATION, dispatch->http->authstring);

      if (httpPost(dispatch->http, dispatch->resource))
        {
          if (httpReconnect(dispatch->http))
            {
              g_warning ("failed to reconnect");
              goto fail;
            }
          break;
        }
        
      dispatch->attempts = 0;

      dispatch->state = DISPATCH_REQUEST;
      dispatch->request->state = IPP_IDLE;

    case DISPATCH_REQUEST:
      ipp_status = ippWrite(dispatch->http, dispatch->request);

      if (ipp_status == IPP_ERROR)
        {
          g_warning ("We got an error writting to the socket");
          goto fail;
        }
 
     if (ipp_status != IPP_DATA)
        {
          /* write attempts don't count
             since we will get an error
             if anything goes wrong */
          dispatch->attempts--;
          break;
        } 

    case DISPATCH_SEND:
      dispatch->state = DISPATCH_SEND;
      http_status = HTTP_CONTINUE;

      if (dispatch->data_fd != 0)
        {
	  ssize_t bytes;
	  char buffer[_CUPS_MAX_CHUNK_SIZE];

          if (httpCheck (dispatch->http))
            http_status = httpUpdate(dispatch->http);

          if (http_status == HTTP_CONTINUE || http_status == HTTP_OK)
            {
	      /* send data */
              bytes = read(dispatch->data_fd, buffer, _CUPS_MAX_CHUNK_SIZE);
	   
	      if (bytes == 0)
	        {
                  dispatch->state = DISPATCH_CHECK;
		  break;
		}
	   
	      if (httpWrite(dispatch->http, buffer, (int)bytes) < bytes)
	        goto fail;
	   
              /* only count an attempt on recoverable errors */
              dispatch->attempts--;
              break;
            }
        }

    case DISPATCH_CHECK:

      dispatch->state = DISPATCH_CHECK;
      http_status = HTTP_CONTINUE;

      if (httpCheck (dispatch->http))
            http_status = httpUpdate(dispatch->http);

      if (http_status == HTTP_CONTINUE)
        {
	  /* only count an attempt on recoverable errors */
          dispatch->attempts--;
          break;
        }
      else if (http_status == HTTP_UNAUTHORIZED)
        {
          /* TODO: prompt for auth */
          goto fail;
        }
      else if (http_status == HTTP_ERROR)
        {
#ifdef G_OS_WIN32
          if (dispatch->http->error != WSAENETDOWN && 
              dispatch->http->error != WSAENETUNREACH)
#else
          if (dispatch->http->error != ENETDOWN && 
              dispatch->http->error != ENETUNREACH)
#endif /* G_OS_WIN32 */
            break; 
          else
            {
              goto fail;
            }
        }
/* TODO: detect ssl in configure.ac */
#if HAVE_SSL
      else if (http_status == HTTP_UPGRADE_REQUIRED)
        {
          /* Flush any error message... */
          httpFlush(dispatch->http);

          /* Reconnect... */
          httpReconnect(dispatch->http);

          /* Upgrade with encryption... */
          httpEncryption(dispatch->http, HTTP_ENCRYPT_REQUIRED);

          break;
        }
#endif 
      else if (http_status != HTTP_OK)
        {
          goto fail;
        }
    case DISPATCH_READ:
      /* We do as many reads as needed
         until we get an error or success */
      dispatch->attempts = 0;
      dispatch->state = DISPATCH_READ;

      if (dispatch->response == NULL)
        dispatch->response = ippNew();

      ipp_status = ippRead(dispatch->http, 
                           dispatch->response);

      if (ipp_status == IPP_ERROR)
        {
          ippDelete(dispatch->response);
          dispatch->response = NULL;
          
          goto fail;
        }
      else if (ipp_status == IPP_DATA)
        goto success;

    case DISPATCH_ERROR:
      break;
    }
    
  /* continue to next iteration */
  return FALSE;

 fail:
   dispatch->state = DISPATCH_ERROR;
 success:
   //httpFlush(dispatch->http);
   ippDelete(dispatch->request);
   dispatch->request = NULL;

   return TRUE;
}

static gboolean
_cups_dispatch_watch_prepare (GSource *source,
                              gint *timeout_)
{
  EggPrintCupsDispatchWatch *dispatch;

  dispatch = (EggPrintCupsDispatchWatch *) source;
 

  *timeout_ = 250;
  /* TODO: Add the ability to send a file */
  
  return FALSE;
}

static gboolean
_cups_dispatch_watch_dispatch (GSource *source,
                               GSourceFunc callback,
                               gpointer user_data)
{
  EggPrintCupsDispatchWatch *dispatch;
  EggPrintCupsResponseCallbackFunc ep_callback;  

  g_assert (callback != NULL);

  ep_callback = (EggPrintCupsResponseCallbackFunc) callback;
  
  dispatch = (EggPrintCupsDispatchWatch *) source;

  /* TODO: send in an error */
  if (dispatch->state == DISPATCH_ERROR)
    g_warning ("error in dispatch");

  ep_callback (EGG_PRINT_BACKEND (dispatch->backend), dispatch->response, user_data);

  g_source_unref (source); 
  return FALSE;
}

static void
_cups_dispatch_watch_finalize (GSource *source)
{
  EggPrintCupsDispatchWatch *dispatch;

  dispatch = (EggPrintCupsDispatchWatch *) source;

  g_free (dispatch->resource);

  if (dispatch->response)
    ippDelete (dispatch->response);
  httpClose (dispatch->http);
}

static GSourceFuncs _cups_dispatch_watch_funcs = 
                              {_cups_dispatch_watch_prepare,
	                       _cups_dispatch_watch_check,
	                       _cups_dispatch_watch_dispatch,
	                       _cups_dispatch_watch_finalize};


static void
_cups_request_execute (EggPrintBackendCups *print_backend,
                       ipp_t *request,
		       gint data_fd,
                       const char *server, 
                       const char *path,
                       EggPrintCupsResponseCallbackFunc callback,
                       gpointer user_data,
                       GDestroyNotify notify,
                       GError **err)
{
  http_t *http;
  EggPrintCupsDispatchWatch *dispatch;
  
  if (!server)
    server = cupsServer();

  if (!path)
    path = "/";

  http = httpConnectEncrypt (server, ippPort(), cupsEncryption());
  httpBlocking (http, 0);

  dispatch = (EggPrintCupsDispatchWatch *) g_source_new (&_cups_dispatch_watch_funcs, 
                                                         sizeof (EggPrintCupsDispatchWatch));

  dispatch->http = http;
  dispatch->request = request;
  dispatch->backend = print_backend;
  dispatch->resource = strdup (path);
  dispatch->attempts = 0;
  dispatch->state = DISPATCH_SETUP;
  dispatch->response = NULL;
  dispatch->data_fd = data_fd;

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

  g_assert (EGG_IS_PRINT_BACKEND_CUPS (print_backend));

  printer_name = (gchar *)user_data;
  cups_printer = (EggPrinterCups *) g_hash_table_lookup (print_backend->printers, printer_name);

  if (!cups_printer)
    return;
    
  printer = EGG_PRINTER (cups_printer);
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
  ipp_t *request;
  gchar *printer_uri;

  error = NULL;

  request = _cups_request_new (IPP_GET_PRINTER_ATTRIBUTES);

  printer_uri = g_strdup_printf ("ipp://localhost/printers/%s",
                                  printer_name);
  ippAddString (request, IPP_TAG_OPERATION, IPP_TAG_URI,
                "printer-uri", NULL, printer_uri);

  g_free (printer_uri);

  _cups_request_execute (print_backend,
                         request,
			 0,
                         NULL, 
                         NULL,
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
  ipp_t *request;

  error = NULL;

  request = _cups_request_new (CUPS_GET_PRINTERS);

  _cups_request_execute (print_backend,
                         request,
			 0,
                         NULL, 
                         NULL,
                         (EggPrintCupsResponseCallbackFunc) _cups_request_printer_list_cb,
                         NULL,
                         NULL,
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
  "EconoMode",
  "HPEconoMode",
  "HPEdgeControl",
  "HPGraphicsHalftone",
  "HPHalftone",
  "HPPhotoHalftone",
  "OutputMode",
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
egg_print_backend_cups_create_settings (EggPrintBackend *print_backend,
					EggPrinter *printer)
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
  
  ppd_file = ppdOpenFile ("test.ppd");
  ppdMarkDefaults (ppd_file);

  for (i = 0; i < ppd_file->num_groups; i++)
    handle_group (set, ppd_file, &ppd_file->groups[i], &ppd_file->groups[i]);

  ppdClose (ppd_file);
  
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
egg_print_backend_cups_mark_conflicts  (EggPrintBackend           *print_backend,
					EggPrinter                *printer,
					EggPrintBackendSettingSet *settings)
{
  ppd_file_t *ppd_file;
  int num_conflicts;
  int i;
  
  ppd_file = ppdOpenFile("test.ppd");
  ppdMarkDefaults (ppd_file);

  for (i = 0; i < ppd_file->num_groups; i++)
    mark_group_from_set (settings, ppd_file, &ppd_file->groups[i]);

  num_conflicts = ppdConflicts (ppd_file);

  if (num_conflicts > 0)
    {
      for (i = 0; i < ppd_file->num_groups; i++)
	set_conflicts_from_group (settings, ppd_file, &ppd_file->groups[i]);
    }

  
  ppdClose (ppd_file);

  return num_conflicts > 0;
}
