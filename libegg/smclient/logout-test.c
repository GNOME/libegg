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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

GtkWidget *main_window, *logout_dialog;

static void
dialog_response (GtkDialog *dialog, int response, gpointer user_data)
{
  EggSMClient *client = user_data;

  gtk_widget_destroy (GTK_WIDGET (dialog));
  egg_sm_client_will_quit (client, (response != GTK_RESPONSE_NO));
}

static void
quit_requested (EggSMClient *client, gpointer user_data)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_QUESTION,
				   GTK_BUTTONS_YES_NO,
				   "Really log out?");
  g_signal_connect (dialog, "response",
		    G_CALLBACK (dialog_response), client);
  gtk_widget_show (GTK_WIDGET (dialog));
}

static void
quit_cancelled (EggSMClient *client, gpointer user_data)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_INFO,
				   GTK_BUTTONS_OK,
				   "Logout cancelled");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
quit (EggSMClient *client, gpointer user_data)
{
  gtk_main_quit ();
}

static void
save_state (EggSMClient *client, const char *state_dir, gpointer user_data)
{
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
				   GTK_DIALOG_MODAL,
				   GTK_MESSAGE_INFO,
				   GTK_BUTTONS_OK,
				   "Save state");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
window_destroyed (GtkWidget *window, GdkEvent *event, gpointer user_data)
{
  gtk_main_quit ();
}

int
main (int argc, char **argv)
{
  EggSMClient *client;
  GOptionContext *goption_context;
  GError *err = NULL;
  GtkWidget *label;

  g_type_init ();

  goption_context = g_option_context_new (_("- Test logout functionality"));
  g_option_context_add_group (goption_context, gtk_get_option_group (TRUE));
  g_option_context_add_group (goption_context, egg_sm_client_get_option_group ());

  if (!g_option_context_parse (goption_context, &argc, &argv, &err))
    {
      g_printerr ("Could not parse arguments: %s\n", err->message);
      g_error_free (err);
      return 1;
    }

  client = egg_sm_client_get ();
  g_signal_connect (client, "quit-requested",
		    G_CALLBACK (quit_requested), NULL);
  g_signal_connect (client, "quit-cancelled",
		    G_CALLBACK (quit_cancelled), NULL);
  g_signal_connect (client, "quit",
		    G_CALLBACK (quit), NULL);
  g_signal_connect (client, "save-state",
		    G_CALLBACK (save_state), NULL);

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (main_window), "Logout Test");
  g_signal_connect (main_window, "destroy_event",
		    G_CALLBACK (window_destroyed), NULL);

  label = gtk_label_new ("Logout test running...");
  gtk_container_add (GTK_CONTAINER (main_window), label);

  gtk_widget_show_all (main_window);

  gtk_main ();
  return 0;
}
