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

#include "eggsmclient.h"

#include <string.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

EggSMClientEndStyle style = EGG_SM_CLIENT_END_SESSION_DEFAULT;
gboolean confirm = TRUE;
gboolean gui = FALSE;

static gboolean
logout_callback (const char *option_name, const char *value,
		 gpointer data, GError **error)
{
  style = EGG_SM_CLIENT_LOGOUT;
  return TRUE;
}

static gboolean
reboot_callback (const char *option_name, const char *value,
		 gpointer data, GError **error)
{
  style = EGG_SM_CLIENT_REBOOT;
  return TRUE;
}

static gboolean
shutdown_callback (const char *option_name, const char *value,
		   gpointer data, GError **error)
{
  style = EGG_SM_CLIENT_SHUTDOWN;
  return TRUE;
}

static const GOptionEntry options[] = {
  { "logout", 'l', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, logout_callback, N_("Logout (as opposed to rebooting or shutting down)"), NULL },
  { "reboot", 'r', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, reboot_callback, N_("Reboot"), NULL },
  { "shutdown", 's', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, shutdown_callback, N_("Shut down computer"), NULL },

  { "gui",  'g', 0, G_OPTION_ARG_NONE, &gui, N_("Use dialog boxes for errors"), NULL },
  { "no-confirmation",  'n', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &confirm, N_("Don't give the user a chance to confirm"), NULL },

  { NULL }
};

int
main (int argc, char **argv)
{
  GOptionContext *goption_context;
  GError *err = NULL;

  g_type_init ();
  g_thread_init (NULL);

  goption_context = g_option_context_new (_("- End the current session"));
  g_option_context_add_main_entries (goption_context, options, NULL);

  g_option_context_add_group (goption_context, gtk_get_option_group (FALSE));
  g_option_context_add_group (goption_context, egg_sm_client_get_option_group ());

  if (!g_option_context_parse (goption_context, &argc, &argv, &err))
    {
      g_printerr ("Could not parse arguments: %s\n", err->message);
      g_error_free (err);
      return 1;
    }

  if (!egg_sm_client_end_session (style, confirm))
    {
      const char *message = _("Could not connect to the session manager");

      if (gui && gtk_init_check (&argc, &argv))
	{
	  GtkWidget *dialog;

	  dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,
					   GTK_BUTTONS_OK, "%s", message);
	  gtk_dialog_run (GTK_DIALOG (dialog));
	  gtk_widget_destroy (dialog);
	} else
	  g_printerr ("%s\n", message);

      return 1;
    }

  return 0;
}
