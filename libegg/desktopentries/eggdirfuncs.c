/* Some helper functions that gdesktopentries needs
 * Copyright (C) 2004 Ray Strode
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

#include "eggdirfuncs.h"
#include "glib.h"

/**
 * egg_get_user_data_dir:
 * 
 * Returns the data directory of the user.
 * 
 * Return value: the value of the data directory of the user as 
 *               specified by the XDG Base Directory specification. 
 **/
gchar*
egg_get_user_data_dir (void)
{
  gchar *data_dir;  

  data_dir = (gchar *) g_getenv ("XDG_DATA_HOME");

  if (data_dir && data_dir[0])
    data_dir = g_strdup (data_dir);
  else
    data_dir = g_build_filename (g_get_home_dir (), ".local", "share", NULL);

  return data_dir;
}

/**
 * egg_get_user_configuration_dir:
 * 
 * Returns the configuration directory of the user.
 * 
 * Return value: the value of the configuration directory of the user
 *               as specified by the XDG Base Directory specification. 
 **/
gchar*
egg_get_user_configuration_dir (void)
{
  gchar *conf_dir;  

  conf_dir = (gchar *) g_getenv ("XDG_CONFIG_HOME");

  if (conf_dir && conf_dir[0])
    conf_dir = g_strdup (conf_dir);
  else
    conf_dir = g_build_filename (g_get_home_dir (), ".config", NULL);

  return conf_dir;
}

/**
 * egg_get_user_cache_dir:
 * 
 * Returns the cache directory of the user.
 * 
 * Return value: the value of the cache directory of the user
 *               as specified by the XDG Base Directory specification. 
 **/
gchar*
egg_get_user_cache_dir (void)
{
  gchar *cache_dir;  

  cache_dir = (gchar *) g_getenv ("XDG_CACHE_HOME");

  if (cache_dir && cache_dir[0])
    cache_dir = g_strdup (cache_dir);
  else
    cache_dir = g_build_filename (g_get_home_dir (), ".cache", NULL);

  return cache_dir;
}

/**
 * egg_get_secondary_data_dirs:
 * 
 * Returns a %NULL-terminated array of preference ordered data directories.
 * 
 * Return value: a %NULL-terminated array of preference ordered data 
 *               directories as specified by the XDG Base Directory 
 *               specification. Use g_strfreev to free it.
 **/
gchar**
egg_get_secondary_data_dirs (void)
{
  gchar *data_dirs, **data_dir_vector;

  data_dirs = (gchar *) g_getenv ("XDG_DATA_DIRS");

  if (!data_dirs || !data_dirs[0])
    data_dirs = "/usr/local/share/:/usr/share/";

  data_dir_vector = g_strsplit (data_dirs, ":", 0);

  return data_dir_vector;
}

/**
 * egg_get_secondary_configuration_dirs:
 * 
 * Returns a %NULL-terminated array of preference ordered configuration
 * directories.
 * 
 * Return value: a %NULL-terminated array of preference ordered 
 *               configuration directories as specified by the XDG 
 *               Base Directory specification. Use g_strfreev to free 
 *               it.
 **/
gchar**
egg_get_secondary_configuration_dirs (void)
{
  gchar *conf_dirs, **conf_dir_vector;

  conf_dirs = (gchar *) g_getenv ("XDG_CONFIG_DIRS");

  if (!conf_dirs || !conf_dirs[0])
    conf_dirs = "/etc/xdg";

  conf_dir_vector = g_strsplit (conf_dirs, ":", 0);

  return conf_dir_vector;
}

gchar **
egg_get_data_dirs (void)
{
  gchar **data_dirs = NULL, **secondary_data_dirs;
  gsize i, length;

  secondary_data_dirs = egg_get_secondary_data_dirs ();

  for (length = 0; secondary_data_dirs[length] != NULL; length++);

  data_dirs = g_renew (char *, secondary_data_dirs, length + 2);

  data_dirs[0] = egg_get_user_data_dir ();

  i = 0;
  while (i < length) {
    data_dirs[i + 1] = secondary_data_dirs[i];
    i++;
  }
  data_dirs[length + 1] = NULL;

  return data_dirs;
}

gchar **
egg_get_configuration_dirs (void)
{
  gchar **configuration_dirs = NULL, **secondary_configuration_dirs;
  gsize i, length;

  secondary_configuration_dirs = egg_get_secondary_configuration_dirs ();

  for (length = 0; secondary_configuration_dirs[length] != NULL; length++);

  configuration_dirs = g_renew (char *, secondary_configuration_dirs,
                                length + 2);

  configuration_dirs[0] = egg_get_user_configuration_dir ();

  i = 0;
  while (i < length) {
    configuration_dirs[i + 1] = secondary_configuration_dirs[i];
    i++;
  }
  configuration_dirs[length + 1] = NULL;

  return configuration_dirs;
}
