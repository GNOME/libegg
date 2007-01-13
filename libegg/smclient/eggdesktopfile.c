/* eggdesktopfile.c - Freedesktop.Org Desktop Files
 * based on gnome-desktop-item.c
 *
 * Copyright (C) 1999, 2000 Red Hat Inc.
 * Copyright (C) 2001 Sid Vicious        <-- FIXME
 * Copyright (C) 2007 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This libraray is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING.LIB. If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "eggdesktopfile.h"

#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>

EggDesktopFile *
egg_desktop_file_new (const char *path, GError **err)
{
  EggDesktopFile *desktop;
  char *type, *name;
  double version;

  desktop = g_new0 (EggDesktopFile, 1);
  desktop->keyfile = g_key_file_new ();
  desktop->source = g_strdup (path);

  if (!g_key_file_load_from_file (desktop->keyfile, desktop->source, 0, err))
    {
      egg_desktop_file_free (desktop);
      return NULL;
    }

  if (!g_key_file_has_group (desktop->keyfile, EGG_DESKTOP_FILE_GROUP))
    {
      g_set_error (err, EGG_DESKTOP_FILE_ERROR,
		   EGG_DESKTOP_FILE_ERROR_INVALID,
		   _("%s is not a .desktop file"), path);
      egg_desktop_file_free (desktop);
      return NULL;
    }

  if (egg_desktop_file_has_key (desktop, EGG_DESKTOP_FILE_KEY_VERSION, NULL))
    {
      version = egg_desktop_file_get_double (desktop,
					     EGG_DESKTOP_FILE_KEY_VERSION,
					     NULL);
      if (version != 1.0)
	{
	  g_set_error (err, EGG_DESKTOP_FILE_ERROR,
		       EGG_DESKTOP_FILE_ERROR_INVALID,
		       _("Invalid version '%f'"), version);
	  egg_desktop_file_free (desktop);
	  return NULL;
	}
    }

  name = egg_desktop_file_get_string (desktop, EGG_DESKTOP_FILE_KEY_NAME, err);
  if (!name)
    {
      egg_desktop_file_free (desktop);
      return NULL;
    }
  g_free (name);
  
  type = egg_desktop_file_get_string (desktop, EGG_DESKTOP_FILE_KEY_TYPE, err);
  if (!type)
    {
      egg_desktop_file_free (desktop);
      return NULL;
    }

  if (!strcmp (type, "Application"))
    desktop->type = EGG_DESKTOP_FILE_TYPE_APPLICATION;
  else if (!strcmp (type, "Link"))
    desktop->type = EGG_DESKTOP_FILE_TYPE_LINK;
  else if (!strcmp (type, "Directory"))
    desktop->type = EGG_DESKTOP_FILE_TYPE_DIRECTORY;
  else
    desktop->type = EGG_DESKTOP_FILE_TYPE_UNRECOGNIZED;

  g_free (type);

  return desktop;
}

void
egg_desktop_file_free (EggDesktopFile *desktop)
{
  g_key_file_free (desktop->keyfile);
  g_free (desktop->source);
  g_free (desktop);
}

GQuark
egg_desktop_file_error_quark (void)
{
  return g_quark_from_static_string ("egg-desktop-file-error-quark");
}

static void
append_string_quoted (GString    *gs,
		      const char *s,
		      gboolean    in_single_quotes,
		      gboolean    in_double_quotes)
{
  const char *p;
  const char *pre = "";
  const char *post = "";

  if (!in_single_quotes && !in_double_quotes)
    {
      pre = "'";
      post = "'";
    }
  else if (!in_single_quotes && in_double_quotes)
    {
      pre = "\"'";
      post = "'\"";
    }

  g_string_append (gs, pre);

  if (strchr (s, '\'') == NULL)
    g_string_append (gs, s);
  else
    {
      for (p = s; *p != '\0'; p++)
	{
	  if (*p == '\'')
	    g_string_append (gs, "'\\''");
	  else
	    g_string_append_c (gs, *p);
	}
    }

  g_string_append (gs, post);
}

typedef enum {
  URI_TO_STRING,
  URI_TO_LOCAL_PATH,
  URI_TO_LOCAL_DIRNAME,
  URI_TO_LOCAL_BASENAME
} ConversionType;

#if 0
static char *
convert_uri (GnomeVFSURI    *uri,
	     ConversionType  conversion)
{
  char *uri_str;
  char *local_path;
  char *retval = NULL;

  uri_str = gnome_vfs_uri_to_string (uri, GNOME_VFS_URI_HIDE_NONE);

  if (conversion == URI_TO_STRING)
    return uri_str;

  local_path = gnome_vfs_get_local_path_from_uri (uri_str);
  g_free (uri_str);

  if (!local_path)
    return NULL;

  switch (conversion) {
  case URI_TO_LOCAL_PATH:
    retval = local_path;
    break;
  case URI_TO_LOCAL_DIRNAME:
    retval = g_path_get_dirname (local_path);
    g_free (local_path);
    break;
  case URI_TO_LOCAL_BASENAME:
    retval = g_path_get_basename (local_path);
    g_free (local_path);
    break;
  default:
    g_assert_not_reached ();
  }

  return retval;
}
#endif

typedef enum {
  ADDED_NONE = 0,
  ADDED_SINGLE,
  ADDED_ALL
} AddedStatus;

static AddedStatus
append_all_converted (GString        *str,
		      ConversionType  conversion,
		      GSList         *args,
		      gboolean        in_single_quotes,
		      gboolean        in_double_quotes,
		      AddedStatus     added_status)
{
#if 0
  GSList *l;

  for (l = args; l; l = l->next)
    {
      char *converted;

      if (!(converted = convert_uri (l->data, conversion)))
	continue;

      g_string_append (str, " ");
      append_string_quoted (str, converted,
			    in_single_quotes,
			    in_double_quotes);
      g_free (converted);
    }
#endif

  return ADDED_ALL;
}

static AddedStatus
append_first_converted (GString         *str,
			ConversionType   conversion,
			GSList         **arg_ptr,
			gboolean         in_single_quotes,
			gboolean         in_double_quotes,
			AddedStatus      added_status)
{
#if 0
  GSList *l;
  char   *converted = NULL;

  for (l = *arg_ptr; l; l = l->next)
    {
      if ((converted = convert_uri (l->data, conversion)))
	break;

      *arg_ptr = l->next;
    }

  if (!converted)
    return added_status;

  append_string_quoted (str, converted, in_single_quotes, in_double_quotes);
  g_free (converted);
#endif

  return added_status != ADDED_ALL ? ADDED_SINGLE : added_status;
}

static gboolean
do_percent_subst (EggDesktopFile  *desktop,
		  const char   	  *arg,
		  GString      	  *str,
		  gboolean     	   in_single_quotes,
		  gboolean     	   in_double_quotes,
		  GSList       	  *args,
		  GSList         **arg_ptr,
		  AddedStatus  	  *added_status)
{
  char *cs;

  if (arg[0] != '%' || arg[1] == '\0')
    return FALSE;

  switch (arg[1])
    {
    case '%':
      g_string_append_c (str, '%');
      break;
    case 'U':
      *added_status = append_all_converted (str,
					    URI_TO_STRING,
					    args,
					    in_single_quotes,
					    in_double_quotes,
					    *added_status);
      break;
    case 'F':
      *added_status = append_all_converted (str,
					    URI_TO_LOCAL_PATH,
					    args,
					    in_single_quotes,
					    in_double_quotes,
					    *added_status);
      break;
    case 'N':
      *added_status = append_all_converted (str,
					    URI_TO_LOCAL_BASENAME,
					    args,
					    in_single_quotes,
					    in_double_quotes,
					    *added_status);
      break;
    case 'D':
      *added_status = append_all_converted (str,
					    URI_TO_LOCAL_DIRNAME,
					    args,
					    in_single_quotes,
					    in_double_quotes,
					    *added_status);
      break;
    case 'f':
      *added_status = append_first_converted (str,
					      URI_TO_LOCAL_PATH,
					      arg_ptr,
					      in_single_quotes,
					      in_double_quotes,
					      *added_status);
      break;
    case 'u':
      *added_status = append_first_converted (str,
					      URI_TO_STRING,
					      arg_ptr,
					      in_single_quotes,
					      in_double_quotes,
					      *added_status);
      break;
    case 'd':
      *added_status = append_first_converted (str,
					      URI_TO_LOCAL_DIRNAME,
					      arg_ptr,
					      in_single_quotes,
					      in_double_quotes,
					      *added_status);
      break;
    case 'n':
      *added_status = append_first_converted (str,
					      URI_TO_LOCAL_BASENAME,
					      arg_ptr,
					      in_single_quotes,
					      in_double_quotes,
					      *added_status);
      break;
    case 'm':
      /* Deprecated */
      cs = egg_desktop_file_get_locale_string (desktop, "MiniIcon", NULL, NULL);
      if (cs != NULL)
	{
	  g_string_append (str, "--miniicon ");
	  append_string_quoted (str, cs, in_single_quotes, in_double_quotes);
	  g_free (cs);
	}
      break;
    case 'i':
      cs = egg_desktop_file_get_locale_string (desktop, EGG_DESKTOP_FILE_KEY_ICON, NULL, NULL);
      if (cs != NULL)
	{
	  g_string_append (str, "--icon ");
	  append_string_quoted (str, cs, in_single_quotes, in_double_quotes);
	  g_free (cs);
	}
      break;
    case 'c':
      cs = egg_desktop_file_get_locale_string (desktop, EGG_DESKTOP_FILE_KEY_NAME, NULL, NULL);
      if (cs != NULL)
	{
	  append_string_quoted (str, cs, in_single_quotes, in_double_quotes);
	  g_free (cs);
	}
      break;
    case 'k':
      if (desktop->source != NULL)
	{
	  append_string_quoted (str, desktop->source, in_single_quotes, in_double_quotes);
	}
      break;
    case 'v':
      /* Deprecated; skip */
      break;
    default:
      /* Maintain special characters - e.g. "%20" */
      /* FIXME: why? "%F0" won't work, why should "%20"? */
      if (g_ascii_isdigit (arg [1])) 
	g_string_append_c (str, '%');
      return FALSE;
    }

  return TRUE;
}

static char *
expand_string (EggDesktopFile  *desktop,
	       const char      *s,
	       GSList          *args,
	       GSList         **arg_ptr,
	       AddedStatus     *added_status)
{
  const char *p;
  gboolean escape = FALSE;
  gboolean single_quot = FALSE;
  gboolean double_quot = FALSE;
  GString *gs = g_string_new (NULL);

  for (p = s; *p != '\0'; p++)
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
      else if (*p == '%')
	{
	  if (do_percent_subst (desktop, p, gs,
				single_quot, double_quot,
				args, arg_ptr,
				added_status))
	    p++;
	}
      else
	g_string_append_c (gs, *p);
    }
  return g_string_free (gs, FALSE);
}

char *
egg_desktop_file_exec_line (EggDesktopFile  *desktop,
			    GError         **error)
{
  AddedStatus astat;
  GSList *arg_ptr = NULL;
  char *exec, *expanded;

  exec = egg_desktop_file_get_string (desktop, EGG_DESKTOP_FILE_KEY_EXEC, error);
  if (!exec)
    return NULL;

  expanded = expand_string (desktop, exec, NULL, &arg_ptr, &astat);
  g_free (exec);
  return expanded;
}
