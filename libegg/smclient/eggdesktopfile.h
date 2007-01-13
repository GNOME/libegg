/* eggdesktopfile.h - Freedesktop.Org Desktop Files
 * based on gnome-desktop-item.h
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

#ifndef __EGG_DESKTOP_FILE_H__
#define __EGG_DESKTOP_FILE_H__

#include <glib/gkeyfile.h>

G_BEGIN_DECLS

typedef enum {
  EGG_DESKTOP_FILE_TYPE_UNRECOGNIZED,

  EGG_DESKTOP_FILE_TYPE_APPLICATION,
  EGG_DESKTOP_FILE_TYPE_LINK,
  EGG_DESKTOP_FILE_TYPE_DIRECTORY,
} EggDesktopFileType;

#define EGG_DESKTOP_FILE_GROUP			"Desktop Entry"

/* Standard Keys */
#define EGG_DESKTOP_FILE_KEY_TYPE		"Type" /* string */
#define EGG_DESKTOP_FILE_KEY_VERSION		"Version"  /* numeric */
#define EGG_DESKTOP_FILE_KEY_NAME		"Name" /* localestring */
#define EGG_DESKTOP_FILE_KEY_GENERIC_NAME	"GenericName" /* localestring */
#define EGG_DESKTOP_FILE_KEY_NO_DISPLAY		"NoDisplay" /* boolean */
#define EGG_DESKTOP_FILE_KEY_COMMENT		"Comment" /* localestring */
#define EGG_DESKTOP_FILE_KEY_ICON		"Icon" /* string */
#define EGG_DESKTOP_FILE_KEY_HIDDEN		"Hidden" /* boolean */
#define EGG_DESKTOP_FILE_KEY_ONLY_SHOW_IN	"OnlyShowIn" /* string */
#define EGG_DESKTOP_FILE_KEY_NOT_SHOW_IN	"NotShowIn" /* string */
#define EGG_DESKTOP_FILE_KEY_TRY_EXEC		"TryExec" /* string */
#define EGG_DESKTOP_FILE_KEY_EXEC		"Exec" /* string */
#define EGG_DESKTOP_FILE_KEY_PATH		"Path" /* string */
#define EGG_DESKTOP_FILE_KEY_TERMINAL		"Terminal" /* boolean */
#define EGG_DESKTOP_FILE_KEY_MIME_TYPE		"MimeType" /* regexp(s) */
#define EGG_DESKTOP_FILE_KEY_CATEGORIES		"Categories" /* string */
#define EGG_DESKTOP_FILE_KEY_STARTUP_NOTIFY	"StartupNotify" /* boolean */
#define EGG_DESKTOP_FILE_KEY_STARTUP_WM_CLASS	"StartupWMClass" /* string */
#define EGG_DESKTOP_FILE_KEY_URL		"URL" /* string */

typedef enum {
  EGG_DESKTOP_FILE_LAUNCH_ONLY_ONE        = 1 << 0,
  EGG_DESKTOP_FILE_LAUNCH_USE_CURRENT_DIR = 1 << 1,
  EGG_DESKTOP_FILE_LAUNCH_APPEND_URIS     = 1 << 2,
  EGG_DESKTOP_FILE_LAUNCH_APPEND_PATHS    = 1 << 3
} EggDesktopFileLaunchFlags;

#define EGG_DESKTOP_FILE_ERROR egg_desktop_file_error_quark()

GQuark egg_desktop_file_error_quark (void);

typedef enum {
  EGG_DESKTOP_FILE_ERROR_INVALID,

  EGG_DESKTOP_FILE_ERROR_NO_EXEC_STRING,
  EGG_DESKTOP_FILE_ERROR_BAD_EXEC_STRING,
  EGG_DESKTOP_FILE_ERROR_NO_URL,
  EGG_DESKTOP_FILE_ERROR_NOT_LAUNCHABLE,
  EGG_DESKTOP_FILE_ERROR_INVALID_TYPE,
} EggDesktopFileError;

typedef struct {
  GKeyFile           *keyfile;
  char               *source;
  EggDesktopFileType  type;
} EggDesktopFile;

EggDesktopFile *egg_desktop_file_new  (const char      *path,
				       GError         **error);
void            egg_desktop_file_free (EggDesktopFile  *desktop);

#define egg_desktop_file_has_key(desktop, key, error)                         \
	g_key_file_has_key (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,         \
			    key, error)
#define egg_desktop_file_get_value(desktop, key, error)                       \
	g_key_file_get_value (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,       \
			      key, error)
#define egg_desktop_file_set_value(desktop, key, value)                       \
	g_key_file_set_value (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,       \
			      key, value)
#define egg_desktop_file_get_string(desktop, key, error)                      \
	g_key_file_get_string (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,      \
			       key, error)
#define egg_desktop_file_set_string(desktop, key, string)                     \
	g_key_file_set_string (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,      \
			       key, string)
#define egg_desktop_file_get_locale_string(desktop, key, locale, error)       \
	g_key_file_get_locale_string (desktop->keyfile,                       \
				      EGG_DESKTOP_FILE_GROUP, key,            \
				      locale, error)
#define egg_desktop_file_set_locale_string(desktop, key, locale, string)      \
	g_key_file_set_locale_string (desktop->keyfile,                       \
				      EGG_DESKTOP_FILE_GROUP, key,            \
				      locale, string)
#define egg_desktop_file_get_boolean(desktop, key, error)                     \
	g_key_file_get_boolean (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,     \
				key, error)
#define egg_desktop_file_set_boolean(desktop, key, value)                     \
	g_key_file_set_boolean (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,     \
				key, value)
#define egg_desktop_file_get_double(desktop, key, error)                      \
	g_key_file_get_double (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,      \
			       key, error)
#define egg_desktop_file_set_double(desktop, key, value)                      \
	g_key_file_set_double (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,      \
			       key, value)
#define egg_desktop_file_get_string_list(desktop, key, length, error)         \
	g_key_file_get_string_list (desktop->keyfile,                         \
				    EGG_DESKTOP_FILE_GROUP, key, length,      \
				    error)
#define egg_desktop_file_set_string_list(desktop, key, list, length)          \
	g_key_file_set_string_list (desktop->keyfile,                         \
				    EGG_DESKTOP_FILE_GROUP, key, list,        \
				    length)
#define egg_desktop_file_remove_key(desktop, key, error)                      \
	g_key_file_remove_key (desktop->keyfile, EGG_DESKTOP_FILE_GROUP,      \
			       key, error)

char *egg_desktop_file_exec_line (EggDesktopFile  *desktop,
				  GError         **error);

G_END_DECLS

#endif /* __EGG_DESKTOP_FILE_H__ */
