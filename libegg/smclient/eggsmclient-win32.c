/*
 * Copyright (C) 2007 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "eggsmclient-private.h"
#include <gdk/gdk.h>

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>

#define EGG_TYPE_SM_CLIENT_WIN32            (egg_sm_client_win32_get_type ())
#define EGG_SM_CLIENT_WIN32(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SM_CLIENT_WIN32, EggSMClientWin32))
#define EGG_SM_CLIENT_WIN32_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_SM_CLIENT_WIN32, EggSMClientWin32Class))
#define EGG_IS_SM_CLIENT_WIN32(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SM_CLIENT_WIN32))
#define EGG_IS_SM_CLIENT_WIN32_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_SM_CLIENT_WIN32))
#define EGG_SM_CLIENT_WIN32_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SM_CLIENT_WIN32, EggSMClientWin32Class))

typedef struct _EggSMClientWin32        EggSMClientWin32;
typedef struct _EggSMClientWin32Class   EggSMClientWin32Class;

struct _EggSMClientWin32 {
  EggSMClient parent;

#ifdef VISTA
  gboolean registered;
  char *restart_args;
  char *state_dir;
#endif

  GAsyncQueue *msg_queue;
};

struct _EggSMClientWin32Class
{
  EggSMClientClass parent_class;

};

static void     sm_client_win32_startup (EggSMClient *client,
					 const char  *client_id);
#ifdef VISTA
static void     sm_client_win32_register_client (EggSMClient *client,
						 const char  *desktop_path);
static void     sm_client_win32_set_restart_command (EggSMClient  *client,
						     int           argc,
						     const char  **argv);
#endif
static void     sm_client_win32_will_quit (EggSMClient *client,
					   gboolean     will_quit);
static gboolean sm_client_win32_end_session (EggSMClient         *client,
					     EggSMClientEndStyle  style,
					     gboolean  request_confirmation);

static gpointer sm_client_thread (gpointer data);

G_DEFINE_TYPE (EggSMClientWin32, egg_sm_client_win32, EGG_TYPE_SM_CLIENT)

static void
egg_sm_client_win32_init (EggSMClientWin32 *win32)
{
  ;
}

static void
egg_sm_client_win32_class_init (EggSMClientWin32Class *klass)
{
  EggSMClientClass *sm_client_class = EGG_SM_CLIENT_CLASS (klass);

  sm_client_class->startup             = sm_client_win32_startup;
#ifdef VISTA
  sm_client_class->register_client     = sm_client_win32_register_client;
  sm_client_class->set_restart_command = sm_client_win32_set_restart_command;
#endif
  sm_client_class->will_quit           = sm_client_win32_will_quit;
  sm_client_class->end_session         = sm_client_win32_end_session;
}

EggSMClient *
egg_sm_client_win32_new (void)
{
  return g_object_new (EGG_TYPE_SM_CLIENT_WIN32, NULL);
}

static void
sm_client_win32_startup (EggSMClient *client,
			 const char  *client_id)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;

  /* FIXME: if we were resumed, we need to clean up the old state
   * dir. But not until after everyone has read their state back...
   */

  /* spawn another thread to listen for logout signals on */
  win32->msg_queue = g_async_queue_new ();
  g_thread_create (sm_client_thread, client, FALSE, NULL);
}

#ifdef VISTA
static void
sm_client_win32_register_client (EggSMClient *client,
				 const char  *desktop_path)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;

  win32->registered = TRUE;
  set_restart_info (win32);
}

static void
sm_client_win32_set_restart_command (EggSMClient  *client,
				     int           argc,
				     const char  **argv)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;
  GString *cmdline = g_string_new (NULL);

  g_return_if_fail (win32->registered == TRUE);

  g_free (win32->restart_args);

  /* RegisterApplicationRestart only cares about the part of the
   * command line after the executable name.
   */
  if (argc > 1)
    {
      int i;

      /* FIXME: what is the right way to quote the arguments? */
      for (i = 1; i < argc; i++)
	{
	  if (i > 1)
	    g_string_append_c (cmdline, ' ');
	  g_string_append (cmdline, argv[i]);
	}
    }

  win32->restart_args = g_string_free (cmdline, FALSE);
  set_restart_info (win32);
}
#endif

static void
sm_client_win32_will_quit (EggSMClient *client,
			   gboolean     will_quit)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;

  /* Can't push NULL onto a GAsyncQueue, so we add 1 to the value... */
  g_async_queue_push (win32->msg_queue, GINT_TO_POINTER (will_quit + 1));
}

static gboolean
sm_client_win32_end_session (EggSMClient         *client,
			     EggSMClientEndStyle  style,
			     gboolean             request_confirmation)
{
  UINT uFlags = EWX_LOGOFF;

  switch (style)
    {
    case EGG_SM_CLIENT_END_SESSION_DEFAULT:
    case EGG_SM_CLIENT_LOGOUT:
      uFlags = EWX_LOGOFF;
      break;
    case EGG_SM_CLIENT_REBOOT:
      uFlags = EWX_REBOOT;
      break;
    case EGG_SM_CLIENT_SHUTDOWN:
      uFlags = EWX_POWEROFF;
      break;
    }

  /* There's no way to make ExitWindowsEx() show a logout dialog, so
   * we ignore @request_confirmation.
   */

#ifdef SHTDN_REASON_FLAG_PLANNED
  ExitWindowsEx (uFlags, SHTDN_REASON_FLAG_PLANNED);
#else
  ExitWindowsEx (uFlags, 0);
#endif

  return TRUE;
}

#ifdef VISTA
static void
set_restart_info (EggSMClientWin32 *win32)
{
  PCWSTR cmdline;

  if (win32->state_dir)
    {
      char *restart_args =
	g_strdup_printf ("--sm-client-state-dir \"%s\"%s%s",
			 win32->state_dir,
			 *win32->restart_args ? " " : "",
			 win32->restart_args);

      /* FIXME: is this right? */
      cmdline = g_utf8_to_utf16 (restart_command, -1, NULL, NULL, NULL);
    }
  else if (*win32->restart_args)
    cmdline = g_utf8_to_utf16 (win32->restart_args, -1, NULL, NULL, NULL);
  else
    cmdline = NULL;

  RegisterApplicationRestart (cmdline, RESTART_NO_CRASH | RESTART_NO_HANG);
  g_free (cmdline);
}
#endif


/* callbacks from logout-listener thread */

static gboolean
emit_quit_requested (gpointer smclient)
{
  gdk_threads_enter ();
  egg_sm_client_quit_requested (smclient);
  gdk_threads_leave ();

  return FALSE;
}

static gboolean
emit_quit (gpointer smclient)
{
  EggSMClientWin32 *win32 = smclient;

  gdk_threads_enter ();
  egg_sm_client_quit (smclient);
  gdk_threads_leave ();

  g_async_queue_push (win32->msg_queue, GINT_TO_POINTER (1));
  return FALSE;
}

static gboolean
emit_quit_cancelled (gpointer smclient)
{
  EggSMClientWin32 *win32 = smclient;

  gdk_threads_enter ();
  egg_sm_client_quit_cancelled (smclient);
  gdk_threads_leave ();

  g_async_queue_push (win32->msg_queue, GINT_TO_POINTER (1));
  return FALSE;
}

#ifdef VISTA
static gboolean
emit_save_state (gpointer smclient)
{
  EggSMClientWin32 *win32 = smclient;

  g_free (win32->state_dir);
  gdk_threads_enter ();
  win32->state_dir = egg_sm_client_save_state (client);
  gdk_threads_leave ();
  set_restart_info (win32);

  g_async_queue_push (win32->msg_queue, GINT_TO_POINTER (1));
  return FALSE;
}
#endif

/* logout-listener thread */

static int
async_emit (EggSMClientWin32 *win32, GSourceFunc emitter)
{
  /* ensure message queue is empty */
  while (g_async_queue_try_pop (win32->msg_queue))
    ;

  /* Emit signal in the main thread and wait for a response */
  g_idle_add (emitter, win32);
  return GPOINTER_TO_INT (g_async_queue_pop (win32->msg_queue)) - 1;
}

LRESULT CALLBACK
sm_client_win32_window_procedure (HWND   hwnd,
				  UINT   message,
				  WPARAM wParam,
				  LPARAM lParam)
{
  EggSMClientWin32 *win32 =
    (EggSMClientWin32 *)GetWindowLongPtr (hwnd, GWLP_USERDATA);

  switch (message)
    {
    case WM_QUERYENDSESSION:
      return async_emit (win32, emit_quit_requested);

    case WM_ENDSESSION:
      if (wParam)
	{
	  /* The session is ending */
#ifdef VISTA
	  if ((lParam & ENDSESSION_CLOSEAPP) && win32->registered)
	    async_emit (win32, emit_save_state);
#endif
	  async_emit (win32, emit_quit);
	}
      else
	{
	  /* Nope, the session *isn't* ending */
	  async_emit (win32, emit_quit_cancelled);
	}
      return 0;

    default:
      return DefWindowProc (hwnd, message, wParam, lParam);
    }
}

static gpointer
sm_client_thread (gpointer smclient)
{
  HINSTANCE instance;
  WNDCLASSEXW wcl; 
  ATOM klass;
  HWND window;
  MSG msg;

  instance = GetModuleHandle (NULL);

  memset (&wcl, 0, sizeof (WNDCLASSEX));
  wcl.cbSize = sizeof (WNDCLASSEX);
  wcl.lpfnWndProc = sm_client_win32_window_procedure;
  wcl.hInstance = instance;
  wcl.lpszClassName = L"EggSmClientWindow";
  klass = RegisterClassEx (&wcl);

  window = CreateWindowEx (0, MAKEINTRESOURCE (klass),
			   L"EggSmClientWindow", 0,
			   10, 10, 50, 50, GetDesktopWindow (),
			   NULL, instance, NULL);
  SetWindowLongPtr (window, GWLP_USERDATA, (LONG_PTR)smclient);

  /* main loop */
  while (GetMessage (&msg, NULL, 0, 0))
    DispatchMessage (&msg);

  return NULL;
}
