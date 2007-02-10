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
#include <gtk/gtk.h>
#define WIN32_LEAN_AND_MEAN
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

  gboolean in_filter;
  gboolean will_quit;
  gboolean will_quit_set;
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

static GdkFilterReturn egg_sm_client_win32_filter (GdkXEvent *xevent,
						   GdkEvent  *event,
						   gpointer   data);

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
  gdk_window_add_filter (NULL, egg_sm_client_win32_filter, client);
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

  win32->will_quit = will_quit;
  win32->will_quit_set = TRUE;
}

static gboolean
sm_client_win32_end_session (EggSMClient         *client,
			     EggSMClientEndStyle  style,
			     gboolean             request_confirmation)
{
  EggSMClientWin32 *win32 = (EggSMClientWin32 *)client;
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

  if (!request_confirmation)
    uFlags |= EWX_FORCE;

#ifdef SHTDN_REASON_FLAG_PLANNED
  ExitWindowsEx (uFlags, SHTDN_REASON_FLAG_PLANNED);
#else
  ExitWindowsEx (uFlags, 0);
#endif

  return TRUE;
}

static gboolean
will_quit (EggSMClientWin32 *win32)
{
  /* Will this really work? Or do we need to do something kinky like
   * arrange to have at least one WM_QUERYENDSESSION message delivered
   * to another thread and then have that thread wait for the main
   * thread to process the quit_requested signal asynchronously before
   * returning? FIXME
   */
  win32->will_quit_set = FALSE;

  egg_sm_client_quit_requested ((EggSMClient *)win32);
  while (!win32->will_quit_set)
    gtk_main_iteration ();

  return win32->will_quit;
}

static GdkFilterReturn
egg_sm_client_win32_filter (GdkXEvent *xevent,
			    GdkEvent  *event,
			    gpointer   data)
{
  EggSMClientWin32 *win32 = data;
  EggSMClient *client = data;
  MSG *msg = (MSG *)xevent;
  GdkFilterReturn retval;

  if (win32->in_filter)
    {
      g_message ("win32->in_filter");
      return GDK_FILTER_CONTINUE;
    }

  /* FIXME: I think these messages are delivered per-window, not
   * per-app, so we need to make sure we only act on them once. (Does
   * the win32 backend have client leader windows like x11?)
   */

  win32->in_filter = TRUE;

  switch (msg->message)
    {
    case WM_QUERYENDSESSION:
      /* If we return GDK_FILTER_CONTINUE, the event will eventually
       * get passed on to DefWindowProc, which will return TRUE,
       * allowing the logout to continue. If we return
       * GDK_FILTER_REMOVE, then inner_window_procedure will return
       * its default value, 0, aka FALSE, causing the logout to be
       * aborted.
       */
      retval = will_quit (win32) ? GDK_FILTER_CONTINUE : GDK_FILTER_REMOVE;
      break;

    case WM_ENDSESSION:
      if (msg->wParam)
	{
	  /* The session is ending */
#ifdef VISTA
	  if ((msg->lParam & ENDSESSION_CLOSEAPP) && win32->registered)
	    {
	      g_free (win32->state_dir);
	      win32->state_dir = egg_sm_client_save_state (client);
	      set_restart_info (win32);
	    }
#endif
	  egg_sm_client_quit (client);
	}
#ifdef VISTA
      else
	{
	  /* The session isn't ending */
	  egg_sm_client_quit_cancelled (client);
	}
#endif

      /* This will cause 0 to be returned, which is what we're supposed
       * to return, although the docs don't say what happens if you return
       * any other value.
       */
      retval = GDK_FILTER_REMOVE;
      break;

    default:
      retval = GDK_FILTER_CONTINUE;
      break;
    }

  win32->in_filter = FALSE;
  return retval;
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
