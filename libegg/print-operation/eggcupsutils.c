/* GTK - The GIMP Toolkit
 * eggcupsutils.h: Statemachine implementation of POST and GET 
 * cup calls which can be used to create a non-blocking cups API
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

#include "eggcupsutils.h"

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

typedef void (*EggCupsRequestStateFunc) (EggCupsRequest *request);

static void _post_send          (EggCupsRequest *request);
static void _post_write_request (EggCupsRequest *request);
static void _post_write_data    (EggCupsRequest *request);
static void _post_check         (EggCupsRequest *request);
static void _post_read_response (EggCupsRequest *request);

static void _get_send           (EggCupsRequest *request);
static void _get_check          (EggCupsRequest *request);
static void _get_read_data      (EggCupsRequest *request);

struct _EggCupsRequest 
{
  EggCupsRequestType type;

  http_t *http;
  http_status_t last_status;
  ipp_t *ipp_request;

  gchar *server;
  gchar *resource;
  gint data_fd;
  gint attempts;

  EggCupsResult *result;

  gint state;

  gint own_http : 1; 
};

struct _EggCupsResult
{
  gchar *error_msg;
  ipp_t *ipp_response;

  gint is_error : 1;
  gint is_ipp_response : 1;
};

#define REQUEST_START 0
#define REQUEST_DONE 500

#define _EGG_CUPS_MAX_ATTEMPTS 10 
#define _EGG_CUPS_MAX_CHUNK_SIZE 8192

/* POST states */
enum 
{
  POST_SEND = REQUEST_START,
  POST_WRITE_REQUEST,
  POST_WRITE_DATA,
  POST_CHECK,
  POST_READ_RESPONSE,
  POST_DONE = REQUEST_DONE
};

EggCupsRequestStateFunc post_states[] = {_post_send,
                                         _post_write_request,
                                         _post_write_data,
                                         _post_check,
                                         _post_read_response};

/* GET states */
enum
{
  GET_SEND = REQUEST_START,
  GET_CHECK,
  GET_READ_DATA,
  GET_DONE = REQUEST_DONE
};

EggCupsRequestStateFunc get_states[] = {_get_send,
                                        _get_check,
                                        _get_read_data};
                                        
static void
egg_cups_result_set_error (EggCupsResult *result, 
                           const char *error_msg)
{
  result->is_ipp_response = FALSE;

  result->is_error = TRUE;
  result->error_msg = g_strdup (error_msg);
}

EggCupsRequest *
egg_cups_request_new (http_t *connection,
                      EggCupsRequestType req_type, 
                      gint operation_id,
                      gint data_fd,
                      const char *server,
                      const char *resource)
{
  EggCupsRequest *request;
  cups_lang_t *language;
  
  request = g_new0 (EggCupsRequest, 1);
  request->result = g_new0 (EggCupsResult, 1);

  request->result->error_msg = NULL;
  request->result->ipp_response = NULL;

  request->result->is_error = FALSE;
  request->result->is_ipp_response = FALSE;

  request->type = req_type;
  request->state = REQUEST_START;

   if (server)
    request->server = g_strdup (server);
  else
    request->server = g_strdup (cupsServer());


  if (resource)
    request->resource = g_strdup (resource);
  else
    request->resource = g_strdup ("/");
 
  if (connection != NULL)
    {
      request->http = connection;
      request->own_http = FALSE;
    }
  else
    {
      request->http = httpConnectEncrypt (request->server, ippPort(), cupsEncryption());
      httpBlocking (request->http, 0);
      request->own_http = TRUE;
    }

  request->last_status = HTTP_CONTINUE;

  request->attempts = 0;
  request->data_fd = data_fd;

  request->ipp_request = ippNew();
  request->ipp_request->request.op.operation_id = operation_id;
  request->ipp_request->request.op.request_id = 1;

  language = cupsLangDefault ();

  egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_CHARSET,
                                   "attributes-charset", 
                                   NULL, "utf-8");
	
  egg_cups_request_ipp_add_string (request, IPP_TAG_OPERATION, IPP_TAG_LANGUAGE,
                                   "attributes-natural-language", 
                                   NULL, language->language);

  cupsLangFree (language);

  return request;
}

static void
egg_cups_result_free (EggCupsResult *result)
{
  g_free (result->error_msg);

  if (result->ipp_response)
    ippDelete (result->ipp_response);

  g_free (result);
}

void
egg_cups_request_free (EggCupsRequest *request)
{
  if (request->own_http)
    if (request->http)
      httpClose (request->http);
  
  if (request->ipp_request)
    ippDelete (request->ipp_request);

  g_free (request->server);
  g_free (request->resource);

  egg_cups_result_free (request->result);

  g_free (request);
}

gboolean 
egg_cups_request_read_write (EggCupsRequest *request)
{
  if (request->type == EGG_CUPS_POST)
    post_states[request->state](request);
  else if (request->type == EGG_CUPS_GET)
    get_states[request->state](request);

  if (request->state == REQUEST_DONE)
    {
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

EggCupsResult *
egg_cups_request_get_result (EggCupsRequest *request)
{
  return request->result;
}

void            
egg_cups_request_ipp_add_string (EggCupsRequest *request,
                                 ipp_tag_t group,
                                 ipp_tag_t tag,
                                 const char *name,
                                 const char *charset,
                                 const char *value)
{
  ippAddString (request->ipp_request,
                group,
                tag,
                name,
                charset,
                value);
}


static void 
_post_send (EggCupsRequest *request)
{
  gchar length[255];
  struct stat data_info;

  if (request->data_fd != 0)
    {
      fstat (request->data_fd, &data_info);
      sprintf (length, "%lu", (unsigned long)ippLength(request->ipp_request) + data_info.st_size);
    }
  else
    {
      sprintf (length, "%lu", (unsigned long)ippLength(request->ipp_request));
    }
	
  httpClearFields(request->http);
  httpSetField(request->http, HTTP_FIELD_CONTENT_LENGTH, length);
  httpSetField(request->http, HTTP_FIELD_CONTENT_TYPE, "application/ipp");
  httpSetField(request->http, HTTP_FIELD_AUTHORIZATION, request->http->authstring);

  if (httpPost(request->http, request->resource))
    {
      if (httpReconnect(request->http))
        {
          g_warning ("failed Post");
          request->state = POST_DONE;

          egg_cups_result_set_error (request->result, "Failed Post");
        }

      request->attempts++;
      return;    
    }
        
    request->attempts = 0;

    request->state = POST_WRITE_REQUEST;
    request->ipp_request->state = IPP_IDLE;
}

static void 
_post_write_request (EggCupsRequest *request)
{
  ipp_state_t ipp_status;
  ipp_status = ippWrite(request->http, request->ipp_request);

  if (ipp_status == IPP_ERROR)
    {
      g_warning ("We got an error writting to the socket");
      request->state = POST_DONE;
     
      egg_cups_result_set_error (request->result, ippErrorString (cupsLastError ()));
      return;
    }

  if (ipp_status == IPP_DATA)
    {
      if (request->data_fd != 0)
        request->state = POST_WRITE_DATA;
      else
        request->state = POST_CHECK;
    }
}

static void 
_post_write_data (EggCupsRequest *request)
{
  ssize_t bytes;
  char buffer[_EGG_CUPS_MAX_CHUNK_SIZE];
  http_status_t http_status;

  if (httpCheck (request->http))
    http_status = httpUpdate(request->http);
  else
    http_status = request->last_status;

  request->last_status = http_status;

  if (http_status == HTTP_CONTINUE || http_status == HTTP_OK)
    {
      /* send data */
      bytes = read(request->data_fd, buffer, _EGG_CUPS_MAX_CHUNK_SIZE);
          
      if (bytes == 0)
        {
          request->state = POST_CHECK;
          request->attempts = 0;
          return;
        }

      if (httpWrite(request->http, buffer, (int)bytes) < bytes)
        {
          g_warning ("We got an error writting to the socket");
          request->state = POST_DONE;
     
          egg_cups_result_set_error (request->result, "Error writting to socket in Post");
          return;
        }
    }
   else
    {
      request->attempts++;
    }
}

static void 
_post_check (EggCupsRequest *request)
{
  http_status_t http_status;

  http_status = request->last_status;

  if (http_status == HTTP_CONTINUE)
    {
      goto again; 
    }
  else if (http_status == HTTP_UNAUTHORIZED)
    {
      /* TODO: callout for auth */
      g_warning ("NOT IMPLEMENTED: We need to prompt for authorization");
      request->state = POST_DONE;
     
      egg_cups_result_set_error (request->result, "Can't prompt for authorization");
      return;
    }
  else if (http_status == HTTP_ERROR)
    {
#ifdef G_OS_WIN32
      if (request->http->error != WSAENETDOWN && 
          request->http->error != WSAENETUNREACH)
#else
      if (request->http->error != ENETDOWN && 
          request->http->error != ENETUNREACH)
#endif /* G_OS_WIN32 */
        {
          goto again;
        }
      else
        {
          g_warning ("Error status");
          request->state = POST_DONE;
     
          egg_cups_result_set_error (request->result, "Unknown HTTP error");
          return;
        }
    }
/* TODO: detect ssl in configure.ac */
#if HAVE_SSL
  else if (http_status == HTTP_UPGRADE_REQUIRED)
    {
      /* Flush any error message... */
      httpFlush (request->http);

      /* Reconnect... */
      httpReconnect (request->http);

      /* Upgrade with encryption... */
      httpEncryption(request->http, HTTP_ENCRYPT_REQUIRED);
  
      goto again;
    }
#endif 
  else if (http_status != HTTP_OK)
    {
      g_warning ("Error status");
      request->state = POST_DONE;
     
      egg_cups_result_set_error (request->result, "HTTP Error");
      return;
    }
  else
    {
      request->state = POST_READ_RESPONSE;
      return;
    }

 again:
  request->attempts++;
  http_status = HTTP_CONTINUE;

  if (httpCheck (request->http))
    http_status = httpUpdate (request->http);

  request->last_status = http_status;
}

static void 
_post_read_response (EggCupsRequest *request)
{
  ipp_state_t ipp_status;

  if (request->result->ipp_response == NULL)
    request->result->ipp_response = ippNew();

  ipp_status = ippRead (request->http, 
                        request->result->ipp_response);

  if (ipp_status == IPP_ERROR)
    {
      g_warning ("Error reading response");
      egg_cups_result_set_error (request->result, ippErrorString (cupsLastError()));
      
      ippDelete (request->result->ipp_response);
      request->result->ipp_response = NULL;

      request->state = POST_DONE; 
    }
  else if (ipp_status == IPP_DATA)
    {
      request->state = POST_DONE;
    }
}

static void 
_get_send (EggCupsRequest *request)
{
  if (request->data_fd == 0)
    {
      egg_cups_result_set_error (request->result, "Get requires an open file descriptor");
      request->state = GET_DONE;

      return;
    }

  httpClearFields(request->http);
  httpSetField(request->http, HTTP_FIELD_AUTHORIZATION, request->http->authstring);

  if (httpGet(request->http, request->resource))
    {
      if (httpReconnect(request->http))
        {
          g_warning ("failed Get");
          request->state = GET_DONE;

          egg_cups_result_set_error (request->result, "Failed Get");
        }

      request->attempts++;
      return;    
    }
        
  request->attempts = 0;

  request->state = GET_CHECK;
  request->ipp_request->state = IPP_IDLE;
}

static void 
_get_check (EggCupsRequest *request)
{
  http_status_t http_status;

  http_status = request->last_status;

  if (http_status == HTTP_CONTINUE)
    {
      goto again; 
    }
  else if (http_status == HTTP_UNAUTHORIZED)
    {
      /* TODO: callout for auth */
      g_warning ("NOT IMPLEMENTED: We need to prompt for authorization in a non blocking manner");
      request->state = GET_DONE;
     
      egg_cups_result_set_error (request->result, "Can't prompt for authorization");
      return;
    }
/* TODO: detect ssl in configure.ac */
#if HAVE_SSL
  else if (http_status == HTTP_UPGRADE_REQUIRED)
    {
      /* Flush any error message... */
      httpFlush (request->http);

      /* Reconnect... */
      httpReconnect (request->http);

      /* Upgrade with encryption... */
      httpEncryption(request->http, HTTP_ENCRYPT_REQUIRED);
  
      goto again;
    }
#endif 
  else if (http_status != HTTP_OK)
    {
      g_warning ("Error status");
      request->state = POST_DONE;
     
      egg_cups_result_set_error (request->result, "HTTP Error");

      httpFlush(request->http);

      return;
    }
  else
    {
      request->state = GET_READ_DATA;
      return;
    }

 again:
  request->attempts++;
  http_status = HTTP_CONTINUE;

  if (httpCheck (request->http))
    http_status = httpUpdate (request->http);

  request->last_status = http_status;

}

static void 
_get_read_data (EggCupsRequest *request)
{
  char buffer[_EGG_CUPS_MAX_CHUNK_SIZE];
  int bytes;
  
  bytes = httpRead(request->http, buffer, sizeof(buffer));

  if (bytes == 0)
    {
      request->state = GET_DONE;
      return;
    }
    
  if (write (request->data_fd, buffer, bytes) == -1)
    {
      char *error_msg;

      request->state = POST_DONE;
    
      error_msg = strerror (errno);
      egg_cups_result_set_error (request->result, error_msg ? error_msg:""); 
    }
}

gboolean
egg_cups_request_is_done (EggCupsRequest *request)
{
  return (request->state == REQUEST_DONE);
}

gboolean
egg_cups_result_is_error (EggCupsResult *result)
{
  return result->is_error;
}

ipp_t *
egg_cups_result_get_response (EggCupsResult *result)
{
  return result->ipp_response;
}

const char *
egg_cups_result_get_error_string (EggCupsResult *result)
{
  return result->error_msg; 
}

