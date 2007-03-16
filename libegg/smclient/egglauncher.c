/* egglauncher.c - Freedesktop.Org Desktop File Launching
 * Copyright (C) 2007 Novell, Inc.
 *
 * Based on gnome-desktop-item.c
 * Copyright (C) 1999, 2000 Red Hat Inc.
 * Copyright (C) 2001 George Lebl
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

#include "egglauncher.h"

#include <string.h>
#include <glib/gi18n.h>

#ifdef GDK_WINDOWING_X11
#define SN_API_NOT_YET_FROZEN
#include <libsn/sn-launcher.h>
#include <gdk/gdkx.h>
#endif

struct EggLauncher {
  GKeyFile        *desktop;
  char            *source;

  EggLauncherType  type;
  gboolean         accepts_multiple;
  gboolean         accepts_uris;

  GSList          *documents;
  GPtrArray       *env;
  GdkScreen       *screen;
  int              workspace;
  char            *directory;
  guint32          launch_time;
  gboolean         launch_time_set;
  gboolean         reap_child;
  GSpawnChildSetupFunc setup_func;
  gpointer         setup_data;

  GPid             pid;
  char            *startup_id;
};

#define EGG_LAUNCHER_SN_TIMEOUT_LENGTH (30 /* seconds */ * 1000)

static gboolean
exec_key_contains_code (const char *exec_key, const char *code)
{
  const char *found_code, *p;

  found_code = strstr (exec_key, code);
  if (!found_code)
    return FALSE;

  /* the code can't be preceded by an odd number of %-signs */
  for (p = found_code; p > exec_key && p[-1] == '%'; p--)
    ;
  return (found_code - p) % 2 == 0;
}

/**
 * egg_launcher_new:
 * @desktop_file: path to a Freedesktop-style Desktop file
 * @error: error pointer
 *
 * Creates a new #EggLauncher for @desktop_file.
 *
 * Return value: the new #EggLauncher, or %NULL on error.
 **/
EggLauncher *
egg_launcher_new (const char *desktop_file, GError **error)
{
  EggLauncher *launcher;
  GKeyFile *desktop;

  desktop = g_key_file_new ();
  if (!g_key_file_load_from_file (desktop, desktop_file, 0, error))
    {
      g_key_file_free (desktop);
      return NULL;
    }

  launcher = egg_launcher_new_from_key_file (desktop, desktop_file, error);
  if (!launcher)
    g_key_file_free (desktop);

  return launcher;
}

/**
 * egg_launcher_new_from_key_file:
 * @desktop: a #GKeyFile representing a desktop file
 * @source_uri: the path or URI that @desktop was loaded from, or %NULL
 * @error: error pointer
 *
 * Creates a new #EggLauncher for @desktop. Assumes ownership of
 * @desktop on success (meaning it will be freed when the launcher
 * is freed).
 *
 * Return value: the new #EggLauncher, or %NULL on error.
 **/
EggLauncher *
egg_launcher_new_from_key_file (GKeyFile    *desktop,
				const char  *source_uri,
				GError     **error)
{
  EggLauncher *launcher;
  char *type, *name, *exec;

  if (!g_key_file_has_group (desktop, EGG_LAUNCHER_DESKTOP_FILE_GROUP))
    {
      g_set_error (error, EGG_LAUNCHER_ERROR, EGG_LAUNCHER_ERROR_INVALID,
		   _("File is not a valid .desktop file"));
      return NULL;
    }

  if (g_key_file_has_key (desktop, EGG_LAUNCHER_DESKTOP_FILE_GROUP,
			  EGG_LAUNCHER_DESKTOP_FILE_KEY_VERSION, NULL))
    {
      double version;

      version = g_key_file_get_double (desktop, EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				       EGG_LAUNCHER_DESKTOP_FILE_KEY_VERSION,
				       NULL);
      if (version != 1.0)
	{
	  g_set_error (error, EGG_LAUNCHER_ERROR, EGG_LAUNCHER_ERROR_INVALID,
		       _("Invalid version '%f'"), version);
	  return NULL;
	}
    }

  name = g_key_file_get_string (desktop, EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				EGG_LAUNCHER_DESKTOP_FILE_KEY_NAME, error);
  if (!name)
    return NULL;
  g_free (name);

  type = g_key_file_get_string (desktop, EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				EGG_LAUNCHER_DESKTOP_FILE_KEY_TYPE, error);
  if (!type)
    return NULL;

  launcher = g_new0 (EggLauncher, 1);
  launcher->desktop = desktop;
  launcher->source = g_strdup (source_uri);

  /* initialize default values */
  launcher->workspace = -1;
  launcher->reap_child = TRUE;

  if (!strcmp (type, "Application"))
    launcher->type = EGG_LAUNCHER_TYPE_APPLICATION;
  else if (!strcmp (type, "Link"))
    launcher->type = EGG_LAUNCHER_TYPE_LINK;
  else if (!strcmp (type, "Directory"))
    launcher->type = EGG_LAUNCHER_TYPE_DIRECTORY;
  else
    launcher->type = EGG_LAUNCHER_TYPE_UNRECOGNIZED;

  if (launcher->type == EGG_LAUNCHER_TYPE_APPLICATION)
    {
      exec = g_key_file_get_string (launcher->desktop,
				    EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				    EGG_LAUNCHER_DESKTOP_FILE_KEY_EXEC,
				    NULL);

      if (exec)
	{
	  launcher->accepts_uris =
	    exec_key_contains_code (exec, "%u") ||
	    exec_key_contains_code (exec, "%U");
	  launcher->accepts_multiple =
	    exec_key_contains_code (exec, "%F") ||
	    exec_key_contains_code (exec, "%U");

	  g_free (exec);
	}
    }

  return launcher;
}

/**
 * egg_launcher_free:
 * @launcher: an #EggLauncher
 *
 * Frees @launcher.
 **/
void
egg_launcher_free (EggLauncher *launcher)
{
  int i;

  g_key_file_free (launcher->desktop);
  g_free (launcher->source);

  egg_launcher_clear_documents (launcher);

  if (launcher->env)
    {
      for (i = 0; i < launcher->env->len; i++)
	g_free (launcher->env->pdata[i]);
      g_ptr_array_free (launcher->env, TRUE);
    }

  if (launcher->screen)
    g_object_unref (launcher->screen);

  g_free (launcher->directory);
  g_free (launcher->startup_id);

  g_free (launcher);
}

GKeyFile *
egg_launcher_get_key_file (EggLauncher *launcher)
{
  return launcher->desktop;
}

/**
 * egg_launcher_get_launcher_type:
 * @launcher: an #EggLauncher
 *
 * Gets the desktop file type of @launcher.
 *
 * Return value: @launcher's type
 **/
EggLauncherType
egg_launcher_get_launcher_type (EggLauncher *launcher)
{
  return launcher->type;
}

/**
 * egg_launcher_can_exec:
 * @launcher: an #EggLauncher
 * @desktop_environment: the name of the running desktop environment,
 * or %NULL
 * @error: error pointer
 *
 * Tests if @launcher can be run. If @launcher has a "TryExec" key,
 * the binary it points to is searched for. Furthermore, if
 * @desktop_environment is non-%NULL, the "OnlyShowIn" and "NotShowIn"
 * keys are also checked to make sure that this launcher is
 * appropriate for the named environment.
 *
 * Return value: %TRUE if @launcher can be run.
 **/
gboolean
egg_launcher_can_exec (EggLauncher  *launcher,
		       const char   *desktop_environment)
{
  char *try_exec, *found_program;
  char **only_show_in, **not_show_in;
  gboolean found;
  int i;

  if (launcher->type == EGG_LAUNCHER_TYPE_LINK)
    return TRUE;
  else if (launcher->type != EGG_LAUNCHER_TYPE_APPLICATION)
    return FALSE;

  try_exec = g_key_file_get_string (launcher->desktop,
				    EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				    EGG_LAUNCHER_DESKTOP_FILE_KEY_TRY_EXEC,
				    NULL);
  if (try_exec)
    {
      found_program = g_find_program_in_path (try_exec);
      g_free (try_exec);

      if (!found_program)
	return FALSE;
      g_free (found_program);
    }

  if (desktop_environment)
    {
      only_show_in = g_key_file_get_string_list (launcher->desktop,
						 EGG_LAUNCHER_DESKTOP_FILE_GROUP,
						 EGG_LAUNCHER_DESKTOP_FILE_KEY_ONLY_SHOW_IN,
						 NULL, NULL);
      if (only_show_in)
	{
	  for (i = 0, found = FALSE; only_show_in[i] && !found; i++)
	    {
	      if (!strcmp (only_show_in[i], desktop_environment))
		found = TRUE;
	    }

	  g_strfreev (only_show_in);

	  if (!found)
	    return FALSE;
	}

      not_show_in = g_key_file_get_string_list (launcher->desktop,
						EGG_LAUNCHER_DESKTOP_FILE_GROUP,
						EGG_LAUNCHER_DESKTOP_FILE_KEY_NOT_SHOW_IN,
						NULL, NULL);
      if (not_show_in)
	{
	  for (i = 0, found = FALSE; not_show_in[i] && !found; i++)
	    {
	      if (!strcmp (not_show_in[i], desktop_environment))
		found = TRUE;
	    }

	  g_strfreev (not_show_in);

	  if (found)
	    return FALSE;
	}
    }

  return TRUE;
}

/**
 * egg_launcher_accepts_multiple:
 * @launcher: an #EggLauncher
 *
 * Tests if @launcher can accept multiple documents at once. If not,
 * and you want to open multiple documents, you must launch @launcher
 * with the first document, then call egg_launcher_clear_documents(),
 * add and launch the second document, etc.
 *
 * Return value: %TRUE or %FALSE
 **/
gboolean
egg_launcher_accepts_multiple (EggLauncher *launcher)
{
  return launcher->accepts_multiple;
}

/**
 * egg_launcher_accepts_uris:
 * @launcher: an #EggLauncher
 *
 * Tests if @launcher can accept (non-"file:") URIs as documents to
 * open.
 *
 * Return value: %TRUE or %FALSE
 **/
gboolean
egg_launcher_accepts_uris (EggLauncher *launcher)
{
  return launcher->accepts_uris;
}

/**
 * egg_launcher_add_document:
 * @launcher: an #EggLauncher
 * @document: a URI or absolute path to a document to open with @launcher
 *
 * Adds @document to the list of files to open when @launcher is
 * launched. @document must be acceptable according to
 * egg_launcher_accepts_multiple() and egg_launcher_accepts_uris().
 **/
void
egg_launcher_add_document (EggLauncher  *launcher,
			   const char   *document)
{
  g_return_if_fail (launcher->accepts_multiple || !launcher->documents);

  if (launcher->accepts_uris)
    {
      if (g_path_is_absolute (document))
	{
	  /* Convert path to URI */
	  launcher->documents = g_slist_prepend (launcher->documents,
						 g_filename_to_uri (document, NULL, NULL));
	}
      else
	{
	  /* Add URI */
	  launcher->documents = g_slist_prepend (launcher->documents,
						 g_strdup (document));
	}
    }
  else
    {
      if (g_path_is_absolute (document))
	{
	  /* Add path */
	  launcher->documents = g_slist_prepend (launcher->documents,
						 g_strdup (document));
	}
      else
	{
	  g_return_if_fail (launcher->accepts_uris || !strncmp (document, "file:", 5));
	  /* Convert URI to path */
	  launcher->documents = g_slist_prepend (launcher->documents,
						 g_filename_from_uri (document, NULL, NULL));
	}
    }
}

/**
 * egg_launcher_clear_documents:
 * @launcher: an #EggLauncher
 *
 * Clears @launcher's list of documents (allowing you to re-launch it
 * with a different set of documents).
 **/
void
egg_launcher_clear_documents (EggLauncher *launcher)
{
  GSList *docs;

  for (docs = launcher->documents; docs; docs = docs->next)
    g_free (docs->data);
  g_slist_free (launcher->documents);
  launcher->documents = NULL;
}

static void
append_quoted_word (GString    *str,
		    const char *s,
		    int         len,
		    gboolean    in_single_quotes,
		    gboolean    in_double_quotes)
{
  const char *p;

  if (!in_single_quotes && !in_double_quotes)
    g_string_append_c (str, '\'');
  else if (!in_single_quotes && in_double_quotes)
    g_string_append (str, "\"'");

  if (!strchr (s, '\''))
    g_string_append (str, s);
  else
    {
      for (p = s; *p != '\0' && len; p++, len--)
	{
	  if (*p == '\'')
	    g_string_append (str, "'\\''");
	  else
	    g_string_append_c (str, *p);
	}
    }

  if (!in_single_quotes && !in_double_quotes)
    g_string_append_c (str, '\'');
  else if (!in_single_quotes && in_double_quotes)
    g_string_append (str, "'\"");
}

static void
append_documents (GString  *str,
		  char      code,
		  GSList   *documents,
		  gboolean  in_single_quotes,
		  gboolean  in_double_quotes)
{
  /* egg_launcher_append_document() enforces the path-vs-URI and
   * single-vs-multiple distinctions, so all we need to worry about
   * here is %d/%D vs %n/%N vs anything else.
   */

  for (; documents; documents = documents->next)
    {
      char *doc = documents->data;
      char *slash;

      g_string_append (str, " ");

      if (code == 'd' || code == 'D')
	{
	  slash = strrchr (doc, '/');
	  if (slash && slash != doc)
	    {
	      append_quoted_word (str, doc, slash - doc,
				  in_single_quotes, in_double_quotes);
	    }
	  else
	    {
	      append_quoted_word (str, doc, -1,
				  in_single_quotes, in_double_quotes);
	    }
	}
      else if (code == 'n' || code == 'N')
	{
	  slash = strrchr (doc, '/');
	  if (slash)
	    {
	      append_quoted_word (str, slash + 1, -1,
				  in_single_quotes, in_double_quotes);
	    }
	  else
	    {
	      append_quoted_word (str, doc, -1,
				  in_single_quotes, in_double_quotes);
	    }
	}
      else
	{
	  append_quoted_word (str, doc, -1,
			      in_single_quotes, in_double_quotes);
	}
    }
}

static gboolean
do_percent_subst (EggLauncher *launcher,
		  char         code,
		  GString     *str,
		  gboolean     in_single_quotes,
		  gboolean     in_double_quotes)
{
  char *value;

  switch (code)
    {
    case '%':
      g_string_append_c (str, '%');
      break;

    case 'F':
    case 'U':
    case 'D':
    case 'N':
    case 'f':
    case 'u':
    case 'd':
    case 'n':
      append_documents (str, code, launcher->documents,
			in_single_quotes, in_double_quotes);
      break;

    case 'i':
      value = g_key_file_get_locale_string (launcher->desktop,
					    EGG_LAUNCHER_DESKTOP_FILE_GROUP,
					    EGG_LAUNCHER_DESKTOP_FILE_KEY_ICON,
					    NULL, NULL);
      if (value)
	{
	  g_string_append (str, "--icon ");
	  append_quoted_word (str, value, -1,
			      in_single_quotes, in_double_quotes);
	  g_free (value);
	}
      break;

    case 'c':
      value = g_key_file_get_locale_string (launcher->desktop,
					    EGG_LAUNCHER_DESKTOP_FILE_GROUP,
					    EGG_LAUNCHER_DESKTOP_FILE_KEY_NAME,
					    NULL, NULL);
      if (value)
	{
	  append_quoted_word (str, value, -1,
			      in_single_quotes, in_double_quotes);
	  g_free (value);
	}
      break;

    case 'k':
      if (launcher->source)
	{
	  append_quoted_word (str, launcher->source, -1,
			      in_single_quotes, in_double_quotes);
	}
      break;

    case 'v':
    case 'm':
      /* Deprecated; skip */
      break;

    default:
      return FALSE;
    }

  return TRUE;
}

static char *
get_application_command (EggLauncher  *launcher,
			 GError      **error)
{
  char *exec, *p, *command;
  gboolean escape, single_quot, double_quot;
  GString *gs;

  exec = g_key_file_get_string (launcher->desktop,
				EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				EGG_LAUNCHER_DESKTOP_FILE_KEY_EXEC,
				error);
  if (!exec)
    return NULL;

  /* Clean up malformed Exec keys */
  if (g_ascii_isspace (exec[0]))
    {
      g_warning ("Removing leading whitespace from Exec key '%s'", exec);
      g_strchug (exec);
    }
  p = exec + strlen (exec);
  if (p > exec && g_ascii_isspace (p[-1]))
    {
      g_warning ("Removing trailing whitespace from Exec key '%s'", exec);
      g_strchomp (exec);
      p = exec + strlen (exec);
    }
  if (p > exec && p[-1] == '&')
    {
      g_warning ("Removing trailing & from Exec key '%s'", exec);
      while (p > exec && (p[-1] == '&' || g_ascii_isspace (p[-1])))
	p--;
      *p = '\0';
    }
  if (!*exec)
    {
      g_set_error (error, EGG_LAUNCHER_ERROR,
		   EGG_LAUNCHER_ERROR_BAD_EXEC_STRING,
		   _("Bad command (Exec) to launch"));
      return NULL;
    }

  /* Build the command */
  gs = g_string_new (NULL);
  escape = single_quot = double_quot = FALSE;

  for (p = exec; *p != '\0'; p++)
    {
      if (escape)
	{
	  escape = FALSE;
	  g_string_append_c (gs, *p);
	}
      else if (*p == '\\')
	{
	  if (!single_quot)
	    escape = TRUE;
	  g_string_append_c (gs, *p);
	}
      else if (*p == '\'')
	{
	  g_string_append_c (gs, *p);
	  if (!single_quot && !double_quot)
	    single_quot = TRUE;
	  else if (single_quot)
	    single_quot = FALSE;
	}
      else if (*p == '"')
	{
	  g_string_append_c (gs, *p);
	  if (!single_quot && !double_quot)
	    double_quot = TRUE;
	  else if (double_quot)
	    double_quot = FALSE;
	}
      else if (*p == '%' && p[1])
	{
	  if (do_percent_subst (launcher, p[1], gs,
				single_quot, double_quot))
	    p++;
	}
      else
	g_string_append_c (gs, *p);
    }

  g_free (exec);
  command = g_string_free (gs, FALSE);

  /* Prepend "xdg-terminal " if needed */
  if (g_key_file_has_key (launcher->desktop,
			  EGG_LAUNCHER_DESKTOP_FILE_GROUP,
			  EGG_LAUNCHER_DESKTOP_FILE_KEY_TERMINAL,
			  NULL))
    {
      GError *terminal_error = NULL;
      gboolean use_terminal =
	g_key_file_get_boolean (launcher->desktop,
				EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				EGG_LAUNCHER_DESKTOP_FILE_KEY_TERMINAL,
				&terminal_error);
      if (terminal_error)
	{
	  g_free (command);
	  g_propagate_error (error, terminal_error);
	  return NULL;
	}

      if (use_terminal)
	{
	  gs = g_string_new ("xdg-terminal ");
	  append_quoted_word (gs, command, -1, FALSE, FALSE);
	  g_free (command);
	  command = g_string_free (gs, FALSE);
	}
    }

  return command;
}

static char *
get_link_command (EggLauncher  *launcher,
		  GError      **error)
{
  char *url;
  GString *gs;

  url = g_key_file_get_string (launcher->desktop,
			       EGG_LAUNCHER_DESKTOP_FILE_GROUP,
			       EGG_LAUNCHER_DESKTOP_FILE_KEY_URL,
			       error);
  if (!url)
    return NULL;

  gs = g_string_new ("xdg-open ");
  append_quoted_word (gs, url, -1, FALSE, FALSE);
  g_free (url);

  return g_string_free (gs, FALSE);
}

/**
 * egg_launcher_get_command:
 * @launcher: an #EggLauncher
 * @error: error pointer
 *
 * Gets the command that should be used to launch @launcher.
 *
 * Return value: the command, which must be freed with g_free().
 **/
char *
egg_launcher_get_command (EggLauncher  *launcher,
			  GError      **error)
{
  switch (launcher->type)
    {
    case EGG_LAUNCHER_TYPE_APPLICATION:
      return get_application_command (launcher, error);

    case EGG_LAUNCHER_TYPE_LINK:
      return get_link_command (launcher, error);

    default:
      g_set_error (error, EGG_LAUNCHER_ERROR,
		   EGG_LAUNCHER_ERROR_NOT_LAUNCHABLE,
		   _("Not a launchable item"));
      return NULL;
    }
}

/**
 * egg_launcher_clearenv:
 * @launcher: an #EggLauncher
 *
 * Clears the environment used by @launcher when launching the
 * application. (By default, the application inherits its parent's
 * complete environment.)
 **/
void
egg_launcher_clearenv (EggLauncher  *launcher)
{
  if (launcher->env)
    {
      int i;

      for (i = 0; i < launcher->env->len; i++)
	g_free (launcher->env->pdata[i]);

      g_ptr_array_free (launcher->env, TRUE);
    }

  launcher->env = g_ptr_array_new ();
}

/**
 * egg_launcher_setenv:
 * @launcher: an #EggLauncher
 * @key: the name of an environment variable
 * @value: the value to set @key to
 *
 * Adds the mapping @key = @value to @launcher's environment.
 **/
void
egg_launcher_setenv   (EggLauncher  *launcher,
		       const char   *key,
		       const char   *value)
{
  egg_launcher_unsetenv (launcher, key);
  g_ptr_array_add (launcher->env, g_strdup_printf ("%s=%s", key, value));
}

/**
 * egg_launcher_unsetenv:
 * @launcher: an #EggLauncher
 * @key: the name of an environment variable
 *
 * Removes @key from @launcher's environment
 **/
void
egg_launcher_unsetenv (EggLauncher  *launcher,
		       const char   *key)
{
  int klen = strlen (key);
  int i;

  /* Make sure env is initialized */
  if (!launcher->env)
    {
      extern char **environ;

      launcher->env = g_ptr_array_new ();

      for (i = 0; environ[i]; i++)
	g_ptr_array_add (launcher->env, g_strdup (environ[i]));
    }

  for (i = 0; i < launcher->env->len; i++)
    {
      char *ekey = (char *)launcher->env->pdata[i];

      if (!strncmp (ekey, key, klen) && ekey[klen] == '=')
	{
	  g_free (ekey);
	  g_ptr_array_remove_index_fast (launcher->env, i);
	  return;
	}
    }
}

/**
 * egg_launcher_set_screen:
 * @launcher: an #EggLauncher
 * @screen: the screen to launch the application on
 *
 * Sets @launcher to launch its application on @screen. The default
 * value, %NULL, means to launch the application on the default
 * screen.
 **/
void
egg_launcher_set_screen (EggLauncher  *launcher,
			 GdkScreen    *screen)
{
  if (launcher->screen)
    g_object_unref (launcher->screen);
  launcher->screen = screen;
  if (launcher->screen)
    g_object_ref (launcher->screen);
}

/**
 * egg_launcher_set_workspace:
 * @launcher: an #EggLauncher
 * @workspace: the workspace to launch the application on
 *
 * Sets @launcher to launch its application on @workspace. The default
 * value, %-1, means to launch the application in the current
 * workspace (or in the screen's default workspace if you use
 * egg_launcher_set_screen() as well).
 **/
void
egg_launcher_set_workspace (EggLauncher  *launcher,
			    int           workspace)
{
  launcher->workspace = workspace;
}

/**
 * egg_launcher_set_directory:
 * @launcher: an #EggLauncher
 * @dir: the directory to launch the application in
 *
 * Sets @launcher to launch its application with a current working
 * directory of @dir. The default value, %NULL, means to launch the
 * application in the current working directory.
 **/
void
egg_launcher_set_directory (EggLauncher  *launcher,
			    const char   *dir)
{
  g_free (launcher->directory);
  launcher->directory = g_strdup (dir);
}

/**
 * egg_launcher_set_launch_time:
 * @launcher: an #EggLauncher
 * @launch_time: timestamp of the event that caused the application
 * to be launched.
 *
 * Sets the launch time of @launcher. If the user interacts with
 * another window after @launch_time but before the launched
 * application creates its first window, the window manager may choose
 * to not give focus to the new application. Passing 0 for
 * @launch_time will explicitly request that the application not
 * receive focus. (FIXME: is that true?)
 **/
void
egg_launcher_set_launch_time (EggLauncher  *launcher,
			      guint32       launch_time)
{
  launcher->launch_time = launch_time;
  launcher->launch_time_set = TRUE;
}

/**
 * egg_launcher_set_reap_child:
 * @launcher: an #EggLauncher
 * @reap_child: whether or not to automatically reap the child process
 * when it exits
 *
 * Sets whether or not the child process will be automatically reaped
 * or not. The default value is %TRUE. If you set this to %FALSE, you
 * will need to call egg_launcher_get_pid() after launching the
 * application and ensure that the child process is reaped; see the
 * documentation for g_spawn_async() for more details.
 **/
void
egg_launcher_set_reap_child (EggLauncher  *launcher,
			     gboolean      reap_child)
{
  launcher->reap_child = reap_child;
}

/**
 * egg_launcher_set_setup_func:
 * @launcher: an #EggLauncher
 * @setup_func: the setup function
 * @setup_data: data to be passed to @setup_func
 *
 * Sets a setup function that will be called in the child process
 * after forking, but before execing the new application. This can be
 * used to set up the child process in various ways.
 *
 * (Note: the behavior is different on Windows: see the documentation
 * for g_spawn_async() for more details.)
 **/
void
egg_launcher_set_setup_func (EggLauncher  *launcher,
			     GSpawnChildSetupFunc setup_func,
			     gpointer      setup_data)
{
  launcher->setup_func = setup_func;
  launcher->setup_data = setup_data;
}

#ifdef GDK_WINDOWING_X11
static void
sn_error_trap_push (SnDisplay *display,
		    Display   *xdisplay)
{
  gdk_error_trap_push ();
}

static void
sn_error_trap_pop (SnDisplay *display,
		   Display   *xdisplay)
{
  gdk_error_trap_pop ();
}

static SnLauncherContext *
setup_startup_notification (EggLauncher *launcher, const char *launchee)
{
  GdkDisplay *display;
  GdkScreen *screen;
  SnDisplay *sn_display;
  SnLauncherContext *sn_context;
  char *startup_class = NULL;
  gboolean startup_notify;
  char *name, *description, *icon;
  guint32 launch_time;

  startup_class =
    g_key_file_get_string (launcher->desktop,
			   EGG_LAUNCHER_DESKTOP_FILE_GROUP,
			   EGG_LAUNCHER_DESKTOP_FILE_KEY_STARTUP_WM_CLASS,
			   NULL);

  if (startup_class)
    startup_notify = TRUE;
  else if (g_key_file_has_key (launcher->desktop,
			       EGG_LAUNCHER_DESKTOP_FILE_GROUP,
			       EGG_LAUNCHER_DESKTOP_FILE_KEY_STARTUP_NOTIFY,
			       NULL))
    {
      startup_notify =
	g_key_file_get_boolean (launcher->desktop,
				EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				EGG_LAUNCHER_DESKTOP_FILE_KEY_STARTUP_NOTIFY,
				NULL);
    }

  if (!startup_notify)
    return NULL;

  if (launcher->screen)
    {
      screen = launcher->screen;
      display = gdk_screen_get_display (launcher->screen);
    }
  else
    {
      display = gdk_display_get_default ();
      screen = gdk_display_get_default_screen (display);
    }

  sn_display = sn_display_new (GDK_DISPLAY_XDISPLAY (display),
			       sn_error_trap_push, sn_error_trap_pop);
  sn_context = sn_launcher_context_new (sn_display,
					gdk_screen_get_number (screen));
  sn_display_unref (sn_display);

  name = g_key_file_get_locale_string (launcher->desktop,
				       EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				       EGG_LAUNCHER_DESKTOP_FILE_KEY_NAME,
				       NULL, NULL);
  sn_launcher_context_set_name (sn_context, name);

  description = g_strdup_printf (_("Starting %s"), name);
  sn_launcher_context_set_description (sn_context, description);
  g_free (description);
		
  name = g_key_file_get_locale_string (launcher->desktop,
				       EGG_LAUNCHER_DESKTOP_FILE_GROUP,
				       EGG_LAUNCHER_DESKTOP_FILE_KEY_ICON,
				       NULL, NULL);
  if (icon != NULL)
    sn_launcher_context_set_icon_name (sn_context, icon);

  if (launcher->workspace != -1)
    sn_launcher_context_set_workspace (sn_context, launcher->workspace);

  if (startup_class != NULL)
    sn_launcher_context_set_wmclass (sn_context, startup_class);

  sn_launcher_context_set_binary_name (sn_context, launchee);

  if (launcher->launch_time_set)
    launch_time = launcher->launch_time;
  else
    {
      /* FIXME: we want to just not pass a launch time in this
       * case, but libsn doesn't let us do that.
       */
      launch_time = gdk_x11_display_get_user_time (display);
    }

  sn_launcher_context_initiate (sn_context, g_get_prgname (),
				launchee, launch_time);

  egg_launcher_setenv (launcher, "DESKTOP_STARTUP_ID",
		       sn_launcher_context_get_startup_id (sn_context));

  return sn_context;
}

static gboolean
startup_notification_timeout (void *sn_context)
{
  sn_launcher_context_complete (sn_context);
  sn_launcher_context_unref (sn_context);

  return FALSE;
}
#endif

/**
 * egg_launcher_launch:
 * @launcher: an #EggLauncher
 * @error: error pointer
 *
 * Launches @launcher according to the parameters that have been set
 * for it.
 *
 * You can use a given #EggLauncher multiple times, changing
 * parameters (such as the document list) in between launches.
 *
 * Return value: %TRUE if the application was successfully launched,
 * %FALSE on failure.
 **/
gboolean
egg_launcher_launch (EggLauncher  *launcher,
		     GError      **error)
{
  char *command;
  int argc;
  char **argv;
  gboolean success;
#ifdef GDK_WINDOWING_X11
  SnLauncherContext *sn_context;
#endif
  GSpawnFlags flags;

  command = egg_launcher_get_command (launcher, error);
  if (!command)
    return FALSE;

  if (!g_shell_parse_argv (command, &argc, &argv, error))
    {
      g_free (command);
      return FALSE;
  }
  g_free (command);
	
  if (launcher->screen)
    {
      char *display_name = gdk_screen_make_display_name (launcher->screen);
      egg_launcher_setenv (launcher, "DISPLAY", display_name);
      g_free (display_name);
    }

  flags = G_SPAWN_SEARCH_PATH;
  if (!launcher->reap_child)
    flags |= G_SPAWN_DO_NOT_REAP_CHILD;

#ifdef GDK_WINDOWING_X11
  sn_context = setup_startup_notification (launcher, argv[0]);
#endif

  success = g_spawn_async (launcher->directory,
			   argv,
			   launcher->env ? (char **)launcher->env->pdata : NULL,
			   flags,
			   launcher->setup_func,
			   launcher->setup_data,
			   &launcher->pid,
			   error);

#ifdef GDK_WINDOWING_X11
  if (sn_context)
    {
      if (success)
	{
	  launcher->startup_id =
	    g_strdup (sn_launcher_context_get_startup_id (sn_context));

	  g_timeout_add (EGG_LAUNCHER_SN_TIMEOUT_LENGTH,
			 startup_notification_timeout,
			 sn_context);
	  /* The timeout now owns sn_context */
	}
      else
	{
	  sn_launcher_context_complete (sn_context);
	  sn_launcher_context_unref (sn_context);
	}
    }
#endif

  g_strfreev (argv);
  return success;
}

/**
 * egg_launcher_get_pid:
 * @launcher: an #EggLauncher that has been successfully launched
 *
 * Retrieves the PID of the (most-recently-launched) child process.
 *
 * Return value: the PID
 **/
GPid
egg_launcher_get_pid (EggLauncher  *launcher)
{
  return launcher->pid;
}

/**
 * egg_launcher_get_startup_id:
 * @launcher: an #EggLauncher that has been successfully launched
 *
 * Retrieves the startup ID of the (most-recently-launched)
 * application, or %NULL if startup notification was not used when
 * launching @launcher.
 *
 * Return value: the startup ID
 **/
const char *
egg_launcher_get_startup_id (EggLauncher  *launcher)
{
  return launcher->startup_id;
}


GQuark
egg_launcher_error_quark (void)
{
  return g_quark_from_static_string ("egg-launcher-error-quark");
}
