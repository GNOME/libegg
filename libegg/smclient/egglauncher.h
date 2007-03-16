/* egglauncher.h - Freedesktop.Org Desktop File Launching
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

#ifndef __EGG_LAUNCHER_H__
#define __EGG_LAUNCHER_H__

#include <gdk/gdk.h>

G_BEGIN_DECLS

typedef struct EggLauncher EggLauncher;

typedef enum {
	EGG_LAUNCHER_TYPE_UNRECOGNIZED,

	EGG_LAUNCHER_TYPE_APPLICATION,
	EGG_LAUNCHER_TYPE_LINK,
	EGG_LAUNCHER_TYPE_DIRECTORY,
} EggLauncherType;

EggLauncher     *egg_launcher_new               (const char   *desktop_file,
						 GError      **error);

EggLauncher     *egg_launcher_new_from_key_file (GKeyFile     *desktop,
						 const char   *source_uri,
						 GError      **error);

void             egg_launcher_free              (EggLauncher  *launcher);

GKeyFile        *egg_launcher_get_key_file      (EggLauncher  *launcher);

EggLauncherType  egg_launcher_get_launcher_type (EggLauncher  *launcher);

gboolean         egg_launcher_can_exec          (EggLauncher  *launcher,
						 const char   *desktop_environment);

gboolean         egg_launcher_accepts_multiple  (EggLauncher  *launcher);
gboolean         egg_launcher_accepts_uris      (EggLauncher  *launcher);

void             egg_launcher_add_document      (EggLauncher  *launcher,
						 const char   *document);
void             egg_launcher_clear_documents   (EggLauncher  *launcher);

char            *egg_launcher_get_command       (EggLauncher  *launcher,
						 GError      **error);

void             egg_launcher_clearenv          (EggLauncher  *launcher);
void             egg_launcher_setenv            (EggLauncher  *launcher,
						 const char   *key,
						 const char   *value);
void             egg_launcher_unsetenv          (EggLauncher  *launcher,
						 const char   *key);

void             egg_launcher_set_screen        (EggLauncher  *launcher,
						 GdkScreen    *screen);
void             egg_launcher_set_workspace     (EggLauncher  *launcher,
						 int           workspace);

void             egg_launcher_set_directory     (EggLauncher  *launcher,
						 const char   *dir);

void             egg_launcher_set_launch_time   (EggLauncher  *launcher,
						 guint32       launch_time);

void             egg_launcher_set_reap_child    (EggLauncher  *launcher,
						 gboolean      reap_child);

void             egg_launcher_set_setup_func    (EggLauncher  *launcher,
						 GSpawnChildSetupFunc setup_func,
						 gpointer      setup_data);

gboolean         egg_launcher_launch            (EggLauncher  *launcher,
						 GError      **error);

GPid             egg_launcher_get_pid           (EggLauncher  *launcher);

const char      *egg_launcher_get_startup_id    (EggLauncher  *launcher);


/* Standard Keys */
#define EGG_LAUNCHER_DESKTOP_FILE_GROUP			"Desktop Entry"

#define EGG_LAUNCHER_DESKTOP_FILE_KEY_TYPE		"Type"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_VERSION		"Version"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_NAME		"Name"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_GENERIC_NAME	"GenericName"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_NO_DISPLAY	"NoDisplay"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_COMMENT		"Comment"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_ICON		"Icon"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_HIDDEN		"Hidden"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_ONLY_SHOW_IN	"OnlyShowIn"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_NOT_SHOW_IN	"NotShowIn"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_TRY_EXEC		"TryExec"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_EXEC		"Exec"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_PATH		"Path"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_TERMINAL		"Terminal"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_MIME_TYPE		"MimeType"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_CATEGORIES	"Categories"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_STARTUP_NOTIFY	"StartupNotify"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_STARTUP_WM_CLASS	"StartupWMClass"
#define EGG_LAUNCHER_DESKTOP_FILE_KEY_URL		"URL"

/* Errors */
#define EGG_LAUNCHER_ERROR egg_launcher_error_quark()

GQuark egg_launcher_error_quark (void);

typedef enum {
	EGG_LAUNCHER_ERROR_INVALID,

	EGG_LAUNCHER_ERROR_NO_EXEC_STRING,
	EGG_LAUNCHER_ERROR_BAD_EXEC_STRING,
	EGG_LAUNCHER_ERROR_NO_URL,
	EGG_LAUNCHER_ERROR_NOT_LAUNCHABLE,
	EGG_LAUNCHER_ERROR_INVALID_TYPE,
} EggLauncherError;

G_END_DECLS

#endif /* __EGG_LAUNCHER_H__ */
