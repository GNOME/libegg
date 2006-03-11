/* eggbookmarkfile.h: parsing and building desktop bookmarks
 *
 * Copyright (C) 2005-2006 Emmanuele Bassi
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
 */

#ifndef __EGG_BOOKMARK_FILE_H__
#define __EGG_BOOKMARK_FILE_H__

#include <glib/gerror.h>
#include <time.h>

G_BEGIN_DECLS

/* GError enumeration
 */
#define EGG_BOOKMARK_FILE_ERROR	(egg_bookmark_file_error_quark ())

typedef enum
{
  EGG_BOOKMARK_FILE_ERROR_INVALID_URI,
  EGG_BOOKMARK_FILE_ERROR_INVALID_VALUE,
  EGG_BOOKMARK_FILE_ERROR_APP_NOT_REGISTERED,
  EGG_BOOKMARK_FILE_ERROR_URI_NOT_FOUND,
  EGG_BOOKMARK_FILE_ERROR_DUPLICATE_URI,
  EGG_BOOKMARK_FILE_ERROR_READ,
  EGG_BOOKMARK_FILE_ERROR_UNKNOWN_ENCODING,
  EGG_BOOKMARK_FILE_ERROR_WRITE,
  EGG_BOOKMARK_FILE_ERROR_FILE_NOT_FOUND
} EggBookmarkFileError;

GQuark egg_bookmark_file_error_quark (void);

/*
 * EggBookmarkFile
 */
typedef struct _EggBookmarkFile EggBookmarkFile;

EggBookmarkFile *egg_bookmark_file_new                 (void);
void             egg_bookmark_file_free                (EggBookmarkFile  *bookmark);

gboolean         egg_bookmark_file_load_from_file      (EggBookmarkFile  *bookmark,
							const gchar      *filename,
							GError          **error);
gboolean         egg_bookmark_file_load_from_data      (EggBookmarkFile  *bookmark,
							const gchar      *data,
							gsize             length,
							GError          **error);
gboolean         egg_bookmark_file_load_from_data_dirs (EggBookmarkFile  *bookmark,
							const gchar      *file,
							gchar           **full_path,
							GError          **error);
gchar *          egg_bookmark_file_to_data             (EggBookmarkFile  *bookmark,
							gsize            *length,
							GError          **error) G_GNUC_MALLOC;
gboolean         egg_bookmark_file_to_file             (EggBookmarkFile  *bookmark,
							const gchar      *filename,
							GError          **error);

void             egg_bookmark_file_set_title           (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *title);
gchar *          egg_bookmark_file_get_title           (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							GError          **error) G_GNUC_MALLOC;
void             egg_bookmark_file_set_description     (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *description);
gchar *          egg_bookmark_file_get_description     (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							GError          **error) G_GNUC_MALLOC;
void             egg_bookmark_file_set_mime_type       (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *mime_type);
gchar *          egg_bookmark_file_get_mime_type       (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							GError          **error) G_GNUC_MALLOC;
void             egg_bookmark_file_set_groups          (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar     **groups,
							gsize             length);
void             egg_bookmark_file_add_group           (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *group);
gboolean         egg_bookmark_file_has_group           (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *group,
							GError          **error);
gchar **         egg_bookmark_file_get_groups          (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							gsize            *length,
							GError          **error) G_GNUC_MALLOC;
void             egg_bookmark_file_add_application     (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *name,
							const gchar      *exec);
gboolean         egg_bookmark_file_has_application     (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *name,
							GError          **error);
gchar **         egg_bookmark_file_get_applications    (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        gsize            *length,
						        GError          **error) G_GNUC_MALLOC;
gboolean         egg_bookmark_file_set_app_info        (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *name,
							const gchar      *exec,
							gint              count,
							time_t            stamp,
							GError          **error);
gboolean         egg_bookmark_file_get_app_info        (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *name,
							gchar           **exec,
							guint            *count,
							time_t           *stamp,
							GError          **error);
void             egg_bookmark_file_set_is_private      (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        gboolean          is_private);
gboolean         egg_bookmark_file_get_is_private      (EggBookmarkFile	 *bookmark,
						        const gchar      *uri,
						        GError          **error);
void             egg_bookmark_file_set_icon            (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *href,
							const gchar      *mime_type);
gboolean         egg_bookmark_file_get_icon            (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							gchar           **href,
							gchar           **mime_type,
							GError          **error);
void             egg_bookmark_file_set_added           (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        time_t            added);
time_t           egg_bookmark_file_get_added           (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        GError          **error);
void             egg_bookmark_file_set_modified	       (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        time_t            modified);
time_t           egg_bookmark_file_get_modified        (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        GError          **error);
void             egg_bookmark_file_set_visited         (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
						        time_t            visited);
time_t           egg_bookmark_file_get_visited         (EggBookmarkFile  *bookmark,
						        const gchar      *uri, 
						        GError          **error);
gboolean         egg_bookmark_file_has_item            (EggBookmarkFile  *bookmark,
						        const gchar      *uri);
gint             egg_bookmark_file_get_size            (EggBookmarkFile  *bookmark);
gchar **         egg_bookmark_file_get_uris            (EggBookmarkFile  *bookmark,
						        gsize            *length) G_GNUC_MALLOC;
gboolean         egg_bookmark_file_remove_group        (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *group,
							GError          **error);
gboolean         egg_bookmark_file_remove_application  (EggBookmarkFile  *bookmark,
							const gchar      *uri,
							const gchar      *name,
							GError          **error);
void             egg_bookmark_file_remove_item         (EggBookmarkFile  *bookmark,
						        const gchar      *uri,
							GError          **error);
gboolean         egg_bookmark_file_move_item           (EggBookmarkFile  *bookmark,
						        const gchar      *old_uri,
						        const gchar      *new_uri,
						        GError          **error);

G_END_DECLS

#endif /* __EGG_BOOKMARK_FILE_H__ */
