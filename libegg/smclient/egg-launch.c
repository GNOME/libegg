/* egg-launch.c - Test program for egglauncher.c
 * Copyright (C) 2007 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING.LIB. If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <gtk/gtk.h>

#include "egglauncher.h"

char **setenv, *display, *directory;
gboolean clearenv;
int workspace = -1, launch_time = -1;

GOptionEntry entries[] = {
  { "setenv", 'e', 0, G_OPTION_ARG_STRING_ARRAY, &setenv,
    "Set an environment variable", "VAR=VALUE" },
  { "clearenv", 'E', 0, G_OPTION_ARG_NONE, &clearenv,
    "Clear the child's environment", NULL },
  { "child-display", 'd', 0, G_OPTION_ARG_STRING, &display,
    "The display to launch the application on", "DISPLAY" },
  { "workspace", 'w', 0, G_OPTION_ARG_INT, &workspace,
    "The workspace to launch the application on", "WORKSPACE" },
  { "directory", 'D', 0, G_OPTION_ARG_STRING, &directory,
    "The directory to launch the application in", "DIRECTORY" },
  { "time", 't', 0, G_OPTION_ARG_INT, &launch_time,
    "The launch time, for focus-stealing-prevention", "TIME" },
  { NULL }
};

int
main (int argc, char **argv)
{
  GOptionContext *ctx;
  EggLauncher *launcher;
  char *desktop_file, *type, *command;
  GError *error = NULL;
  int i;

  ctx = g_option_context_new ("DESKTOP-FILE [DOCUMENTS...] - test EggLauncher");
  g_option_context_add_main_entries (ctx, entries, NULL);
  g_option_context_add_group (ctx, gtk_get_option_group (TRUE));

  if (!g_option_context_parse (ctx, &argc, &argv, &error))
    {
      g_printerr ("Could not parse arguments: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s [OPTION...] DESKTOP-FILE [DOCUMENTS...]\n", argv[0]);
      return 1;
    }

  desktop_file = argv[1];
  launcher = egg_launcher_new (desktop_file, &error);
  if (!launcher)
    {
      fprintf (stderr, "Could not create launcher: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  if (!egg_launcher_can_exec (launcher, NULL))
    {
      fprintf (stderr, "Not launchable according to TryExec\n");
      return 1;
    }
  if (!egg_launcher_can_exec (launcher, "GNOME"))
    {
      fprintf (stderr, "Not launchable in GNOME\n");
      return 1;
    }

  if (clearenv)
    egg_launcher_clearenv (launcher);
  if (setenv)
    {
      for (i = 0; setenv[i]; i++)
	{
	  char **env = g_strsplit (setenv[i], "=", 2);
	  if (env[0] && env[1])
	    egg_launcher_setenv (launcher, env[0], env[1]);
	  g_strfreev (env);
	}
    }

  if (display)
    {
      GdkDisplay *gdk_display = gdk_display_open (display);

      if (!gdk_display)
	{
	  fprintf (stderr, "Could not open child display '%s'\n", display);
	  return 1;
	}
      egg_launcher_set_screen (launcher, gdk_display_get_default_screen (gdk_display));
    }

  if (workspace != -1)
    egg_launcher_set_workspace (launcher, workspace);

  if (directory)
    egg_launcher_set_directory (launcher, directory);

  if (launch_time != -1)
    egg_launcher_set_launch_time (launcher, launch_time);

  if (argc > 2)
    {
      for (i = 2; i < argc; i++)
	egg_launcher_add_document (launcher, argv[i]);
    }

  command = egg_launcher_get_command (launcher, &error);
  if (!command)
    {
      fprintf (stderr, "Could not get launch command: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  switch (egg_launcher_get_launcher_type (launcher))
    {
    case EGG_LAUNCHER_TYPE_APPLICATION:
      type = "Application";
      break;
    case EGG_LAUNCHER_TYPE_DIRECTORY:
      type = "Directory";
      break;
    case EGG_LAUNCHER_TYPE_LINK:
      type = "Link";
      break;
    default:
      type = "Unrecognized";
      break;
    }

  printf ("Type: %s\nCommand: %s\n", type, command);
  g_free (command);

  if (!egg_launcher_launch (launcher, &error))
    {
      fprintf (stderr, "Could not launch launcher: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  printf ("PID: %lu\nStartup ID: %s\n",
	  (unsigned long)egg_launcher_get_pid (launcher),
	  egg_launcher_get_startup_id (launcher));

  return 0;
}
