/* egg-launch.c - Test program for eggdesktopfile.c
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

#include "eggdesktopfile.h"

char **setenv, *display, *directory;
int workspace = -1, launch_time = 0;

GOptionEntry entries[] = {
  { "setenv", 'e', 0, G_OPTION_ARG_STRING_ARRAY, &setenv,
    "Set an environment variable", "VAR=VALUE" },
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
  EggDesktopFile *desktop_file;
  char *desktop_file_path, *command;
  const char *type;
  GError *error = NULL;
  GdkDisplay *gdk_display;
  GdkScreen *screen;
  GSList *documents;
  int i;
  pid_t pid;
  char *startup_id;

  ctx = g_option_context_new ("DESKTOP-FILE [DOCUMENTS...] - test EggDesktopFile");
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

  desktop_file_path = argv[1];
  desktop_file = egg_desktop_file_new (desktop_file_path, &error);
  if (!desktop_file)
    {
      fprintf (stderr, "Could not parse desktop file: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  if (!egg_desktop_file_can_launch (desktop_file, NULL))
    {
      fprintf (stderr, "Not launchable according to TryExec\n");
      return 1;
    }
  if (!egg_desktop_file_can_launch (desktop_file, "GNOME"))
    {
      fprintf (stderr, "Not launchable in GNOME\n");
      return 1;
    }

  if (!setenv)
    {
      setenv = g_new (char *, 1);
      setenv[0] = NULL;
    }

  if (display)
    {
      gdk_display = gdk_display_open (display);
      if (!gdk_display)
	{
	  fprintf (stderr, "Could not open child display '%s'\n", display);
	  return 1;
	}
    }
  else
    gdk_display = gdk_display_get_default ();
  screen = gdk_display_get_default_screen (gdk_display);

  documents = NULL;
  for (i = 2; i < argc; i++)
    documents = g_slist_prepend (documents, argv[i]);

  command = egg_desktop_file_parse_exec (desktop_file, documents, &error);
  if (!command)
    {
      fprintf (stderr, "Could not get launch command: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  switch (egg_desktop_file_get_desktop_file_type (desktop_file))
    {
    case EGG_DESKTOP_FILE_TYPE_APPLICATION:
      type = "Application";
      break;
    case EGG_DESKTOP_FILE_TYPE_DIRECTORY:
      type = "Directory";
      break;
    case EGG_DESKTOP_FILE_TYPE_LINK:
      type = "Link";
      break;
    case EGG_DESKTOP_FILE_TYPE_UNRECOGNIZED:
    default:
      type = "Unrecognized";
      break;
    }

  printf ("Type: %s\nCommand: %s\n", type, command);
  g_free (command);

  if (!egg_desktop_file_launch (desktop_file, documents, &error,
				EGG_DESKTOP_FILE_LAUNCH_PUTENV, setenv,
				EGG_DESKTOP_FILE_LAUNCH_SCREEN, screen,
				EGG_DESKTOP_FILE_LAUNCH_WORKSPACE, workspace,
				EGG_DESKTOP_FILE_LAUNCH_DIRECTORY, directory,
				EGG_DESKTOP_FILE_LAUNCH_TIME, (guint32)launch_time,
				EGG_DESKTOP_FILE_LAUNCH_RETURN_PID, &pid,
				EGG_DESKTOP_FILE_LAUNCH_RETURN_STARTUP_ID, &startup_id,
				NULL))
    {
      fprintf (stderr, "Could not launch desktop_file: %s\n", error->message);
      g_error_free (error);
      return 1;
    }

  printf ("PID: %lu\nStartup ID: %s\n", (unsigned long)pid, startup_id);

  return 0;
}
