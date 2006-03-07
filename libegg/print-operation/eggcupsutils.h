/* eggcupsutils.h 
 * Copyright (C) 2006 John (J5) Palmieri <johnp@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#ifndef __EGG_CUPS_UTILS_H__
#define __EGG_CUPS_UTILS_H__

#include <glib.h>
#include <cups/cups.h>
#include <cups/language.h>
#include <cups/http.h>
#include <cups/ipp.h>

G_BEGIN_DECLS

typedef struct _EggCupsRequest  EggCupsRequest;
typedef struct _EggCupsResult   EggCupsResult;

typedef enum
{
  EGG_CUPS_POST,
  EGG_CUPS_GET
} EggCupsRequestType;


/** 
 * Direction we should be polling the http socket on.
 * We are either reading or writting at each state.
 * This makes it easy for mainloops to connect to poll.
 */
typedef enum
{
  EGG_CUPS_HTTP_IDLE,
  EGG_CUPS_HTTP_READ,
  EGG_CUPS_HTTP_WRITE
} EggCupsPollState;


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
  EggCupsPollState poll_state;

  gint own_http : 1; 
};

#define EGG_CUPS_REQUEST_START 0
#define EGG_CUPS_REQUEST_DONE 500

/* POST states */
enum 
{
  EGG_CUPS_POST_CONNECT = EGG_CUPS_REQUEST_START,
  EGG_CUPS_POST_SEND,
  EGG_CUPS_POST_WRITE_REQUEST,
  EGG_CUPS_POST_WRITE_DATA,
  EGG_CUPS_POST_CHECK,
  EGG_CUPS_POST_READ_RESPONSE,
  EGG_CUPS_POST_DONE = EGG_CUPS_REQUEST_DONE
};

/* GET states */
enum
{
  EGG_CUPS_GET_CONNECT = EGG_CUPS_REQUEST_START,
  EGG_CUPS_GET_SEND,
  EGG_CUPS_GET_CHECK,
  EGG_CUPS_GET_READ_DATA,
  EGG_CUPS_GET_DONE = EGG_CUPS_REQUEST_DONE
};

EggCupsRequest *egg_cups_request_new (http_t *connection,
                                      EggCupsRequestType req_type, 
                                      gint operation_id,
                                      gint data_fd,
                                      const char *server,
                                      const char *resource);

void            egg_cups_request_ipp_add_string (EggCupsRequest *request,
                                                 ipp_tag_t group,
                                                 ipp_tag_t tag,
                                                 const char *name,
                                                 const char *charset,
                                                 const char *value);
                                             
gboolean        egg_cups_request_read_write (EggCupsRequest *request);

EggCupsPollState egg_cups_request_get_poll_state (EggCupsRequest *request);

void            egg_cups_request_free (EggCupsRequest *request);

EggCupsResult  *egg_cups_request_get_result (EggCupsRequest *request);

gboolean        egg_cups_request_is_done (EggCupsRequest *request);

void            egg_cups_request_encode_option (EggCupsRequest *request,
                                                const gchar *option,
				                const gchar *value);

gboolean        egg_cups_result_is_error (EggCupsResult *result);

ipp_t          *egg_cups_result_get_response (EggCupsResult *result);

const char     *egg_cups_result_get_error_string (EggCupsResult *result);

G_END_DECLS
#endif 
