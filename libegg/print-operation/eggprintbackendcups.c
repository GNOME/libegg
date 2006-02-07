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

typedef struct _EggPrintBackendCupsClass EggPrintBackendCupsClass;

#define EGG_PRINT_BACKEND_CUPS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_BACKEND_CUPS, EggPrintBackendCupsClass))
#define EGG_IS_PRINT_BACKEND_CUPS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_BACKEND_CUPS))
#define EGG_PRINT_BACKEND_CUPS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_BACKEND_CUPS, EggPrintBackendCupsClass))

#define _CUPS_MAX_ATTEMPTS 10 

#define _CUPS_MAP_ATTR_INT(attr, v, a) {if (!g_ascii_strcasecmp (attr->name, (a))) v = attr->values[0].integer;}
#define _CUPS_MAP_ATTR_STR(attr, v, a) {if (!g_ascii_strcasecmp (attr->name, (a))) v = g_strdup (attr->values[0].string.text);}


typedef void (* EggPrintCupsResponseCallbackFunc) (EggPrintBackend *print_backend,
                                                   ipp_t *response, 
                                                   gpointer user_data);

typedef enum 
{
  DISPATCH_SETUP,
  DISPATCH_SEND,
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
} EggPrintCupsDispatchWatch;

typedef struct
{
  gchar *device_uri;
  gchar *printer_uri;
  ipp_pstate_t state;
} CupsPrinter;

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

static void _cups_request_printer_list (EggPrintBackendCups *print_backend);

static void
_free_cups_printer (CupsPrinter *printer)
{
  g_free (printer->device_uri);
  g_free (printer->printer_uri);

  g_free (printer);
}

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
_cairo_write_to_cups (EggPrintBackendCups *backend,
                      const unsigned char *data,
                      unsigned int         length)
{
  cairo_status_t result;
  
  /* TODO: Hookup to CUPS */
  result = CAIRO_STATUS_WRITE_ERROR;
  
  /* for now just print out the buffer */
  printf ("%.*s", length, data);

  result = CAIRO_STATUS_SUCCESS;

  return result;
}


static cairo_surface_t *
egg_print_backend_cups_printer_create_cairo_surface (EggPrintBackend *backend, 
                                                     EggPrinter *printer,
                                                     gdouble width, 
                                                     gdouble height)
{
  cairo_surface_t *surface;
  EggPrintBackendCups *cups_backend;
  
  /* TODO: check if it is a ps or pdf printer */
  
  cups_backend = EGG_PRINT_BACKEND_CUPS (backend);
  
  surface = cairo_pdf_surface_create_for_stream  (_cairo_write_to_cups, cups_backend, width, height);

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

static void
egg_print_backend_cups_iface_init   (EggPrintBackendIface *iface)
{
  iface->printer_create_cairo_surface = egg_print_backend_cups_printer_create_cairo_surface;
  iface->find_printer = egg_print_backend_cups_find_printer;
}

static void
egg_print_backend_cups_init (EggPrintBackendCups *backend_cups)
{
  backend_cups->printers = g_hash_table_new_full (g_str_hash, 
                                                  g_str_equal, 
                                                 (GDestroyNotify) g_free,
                                                 (GDestroyNotify) _free_cups_printer);

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

    case DISPATCH_SETUP:
      sprintf (length, "%lu", (unsigned long)ippLength(dispatch->request));

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
      dispatch->state = DISPATCH_SEND;
      dispatch->request->state = IPP_IDLE;

    case DISPATCH_SEND:

      ipp_status = ippWrite(dispatch->http, dispatch->request);

      if (ipp_status == IPP_ERROR)
        {
          g_warning ("We got an error writting to the socket");
          goto fail;
        }
      else if (ipp_status != IPP_DATA)
        {
          /* write attempts don't count
             since we will get an error
             if anything goes wrong */
          dispatch->attempts--;
          break;
        }
    
      http_status = HTTP_CONTINUE;

      if (httpCheck (dispatch->http))
        {
          http_status = httpUpdate(dispatch->http);
        }

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
  CupsPrinter *cups_printer;
  EggPrinter *printer;

  g_assert (EGG_IS_PRINT_BACKEND_CUPS (print_backend));

  printer_name = (gchar *)user_data;
  printer = (EggPrinter *) g_hash_table_lookup (print_backend->printers, printer_name);

  if (!printer)
    return;

  cups_printer = (CupsPrinter *) printer->priv->backend_data;

  for (attr = response->attrs; attr != NULL; attr = attr->next) {
    if (!attr->name)
      continue;

    _CUPS_MAP_ATTR_STR (attr, printer->priv->location, "printer-location");
    _CUPS_MAP_ATTR_STR (attr, printer->priv->description, "printer-info");

    _CUPS_MAP_ATTR_STR (attr, cups_printer->device_uri, "device-uri");
    _CUPS_MAP_ATTR_STR (attr, cups_printer->printer_uri, "printer-uri-supported");
    _CUPS_MAP_ATTR_STR (attr, printer->priv->state_message, "printer-state-message");
    _CUPS_MAP_ATTR_INT (attr, cups_printer->state, "printer-state");
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
      EggPrinter *printer;
      CupsPrinter *cups_printer;

      printer = (EggPrinter *) g_hash_table_lookup (print_backend->printers, 
                                                         attr->values[0].string.text);

      if (!printer)
        {
	  printer = egg_printer_new ();
          cups_printer = g_new0 (CupsPrinter, 1);
          printer->priv->name = g_strdup (attr->values[0].string.text);
          printer->priv->backend = EGG_PRINT_BACKEND (print_backend);

          egg_printer_set_backend_data (printer,
                                              cups_printer,
                                              (GFreeFunc) _free_cups_printer);

          g_hash_table_insert (print_backend->printers,
                               g_strdup (printer->priv->name), 
                               printer); 
        }
    
      _cups_request_printer_info (print_backend, printer->priv->name);
      
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
                         NULL, 
                         NULL,
                         (EggPrintCupsResponseCallbackFunc) _cups_request_printer_list_cb,
                         NULL,
                         NULL,
                         &error);

}
