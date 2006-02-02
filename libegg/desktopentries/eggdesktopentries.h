/* eggdesktopentries.h - desktop entry file parser
 *
 *  Copyright 2004  Ray Strode <halfline@hawaii.rr.com>
 *
 * GLib is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * GLib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GLib; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_DESKTOP_ENTRIES_H__
#define __EGG_DESKTOP_ENTRIES_H__

#include <glib/gerror.h>

G_BEGIN_DECLS

typedef enum
{
  EGG_DESKTOP_ENTRIES_ERROR_UNKNOWN_ENCODING,
  EGG_DESKTOP_ENTRIES_ERROR_BAD_START_GROUP,
  EGG_DESKTOP_ENTRIES_ERROR_PARSE,
  EGG_DESKTOP_ENTRIES_ERROR_NO_FILE,
  EGG_DESKTOP_ENTRIES_ERROR_KEY_NOT_FOUND,
  EGG_DESKTOP_ENTRIES_ERROR_GROUP_NOT_FOUND,
  EGG_DESKTOP_ENTRIES_ERROR_INVALID_VALUE
} EggDesktopEntriesError;

#define EGG_DESKTOP_ENTRIES_ERROR egg_desktop_entries_error_quark()

GQuark egg_desktop_entries_error_quark (void);

typedef struct _EggDesktopEntries EggDesktopEntries;

typedef enum
{
  EGG_DESKTOP_ENTRIES_NONE                     = 0,
  EGG_DESKTOP_ENTRIES_DISCARD_COMMENTS         = 1 << 0,
  EGG_DESKTOP_ENTRIES_DISCARD_TRANSLATIONS     = 1 << 1,
  EGG_DESKTOP_ENTRIES_GENERATE_LOOKUP_MAP      = 1 << 2
} EggDesktopEntriesFlags;

EggDesktopEntries         *egg_desktop_entries_new           (EggDesktopEntriesFlags   flags,
                                                              GError                 **error);
EggDesktopEntries         *egg_desktop_entries_new_from_file (const gchar             *file,
                                                              gchar * const legal_start_groups[],
                                                              EggDesktopEntriesFlags   flags,
                                                              GError             **error);
EggDesktopEntries         *egg_desktop_entries_new_from_data (const gchar         *data,
                                                              gsize                length,
                                                              gchar * const legal_start_groups[],
                                                              EggDesktopEntriesFlags   flags,
                                                              GError             **error);
EggDesktopEntries         *egg_desktop_entries_new_from_data_dirs (const gchar             *file,
                                                                   gchar                  **full_path,
                                                                   gchar * const            legal_start_groups[],
                                                                   EggDesktopEntriesFlags   flags,
                                                                   GError                 **error);

void                   egg_desktop_entries_free            (EggDesktopEntries  *entries);

void                   egg_desktop_entries_keep_locales    (EggDesktopEntries *entries,
                                                            gchar * const      locales[]);

gchar                 *egg_desktop_entries_to_data   (EggDesktopEntries   *entries,
                                                      gsize             *length,
                                                      GError           **error);

const gchar           *egg_desktop_entries_get_start_group (EggDesktopEntries *entries);

gchar                **egg_desktop_entries_get_groups (EggDesktopEntries  *entries,
                                                       gsize          *length);
gchar                **egg_desktop_entries_get_keys   (EggDesktopEntries  *entries,
                                                       const gchar    *group_name,
                                                       gsize            *length,
                                                       GError        **error);
gboolean               egg_desktop_entries_has_group  (EggDesktopEntries  *entries,
                                                       const gchar     *group);

gboolean               egg_desktop_entries_has_key  (EggDesktopEntries  *entries,
                                                     const gchar     *group,
                                                     const gchar     *key);

gchar                 *egg_desktop_entries_get_value         (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              GError        **error);

void                  egg_desktop_entries_set_value         (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              const gchar    *value,
                                                              GError        **error);

gchar                 *egg_desktop_entries_get_string        (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              GError        **error);
void                  egg_desktop_entries_set_string        (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              const gchar    *value,
                                                              GError        **error);

gchar                 *egg_desktop_entries_get_locale_string (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              const gchar    *locale,
                                                              GError        **error);
void                   egg_desktop_entries_set_locale_string (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              const gchar    *locale,
                                                              const gchar    *value,
                                                              GError        **error);

gboolean               egg_desktop_entries_get_boolean       (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              GError        **error);
void                   egg_desktop_entries_set_boolean       (EggDesktopEntries  *entries,
                                                              const gchar    *group,
                                                              const gchar    *key,
                                                              gboolean        value,
                                                              GError        **error);

gint                   egg_desktop_entries_get_integer       (EggDesktopEntries  *entries,
                                                              const gchar        *group,
                                                              const gchar        *key,
                                                              GError            **error);
void                   egg_desktop_entries_set_integer       (EggDesktopEntries  *entries,
                                                              const gchar        *group,
                                                              const gchar        *key,
                                                              gint                value,
                                                              GError            **error);

gchar                **egg_desktop_entries_get_string_list        (EggDesktopEntries  *entries,
                                                                   const gchar        *group,
                                                                   const gchar        *key,
                                                                   gsize              *length,
                                                                   GError            **error);
void                   egg_desktop_entries_set_string_list        (EggDesktopEntries  *entries,
                                                                   const gchar        *group,
                                                                   const gchar        *key,
                                                                   gchar * const       value[],
                                                                   gsize               length,
                                                                   GError            **error);

gchar                **egg_desktop_entries_get_locale_string_list (EggDesktopEntries  *entries,
                                                                   const gchar        *group,
                                                                   const gchar        *key,
                                                                   const gchar        *locale,
                                                                   gsize              *length,
                                                                   GError            **error);
void                   egg_desktop_entries_set_locale_string_list (EggDesktopEntries  *entries,
                                                                   const gchar        *group,
                                                                   const gchar        *key,
                                                                   const gchar        *locale,
                                                                   gchar * const       value[], 
                                                                   gsize               length,
                                                                   GError            **error);

gboolean              *egg_desktop_entries_get_boolean_list       (EggDesktopEntries  *entries,
                                                                   const gchar    *group,
                                                                   const gchar    *key,
                                                                   gsize          *length,
                                                                   GError        **error);
void                   egg_desktop_entries_set_boolean_list       (EggDesktopEntries  *entries,
                                                                   const gchar    *group,
                                                                   const gchar    *key,
                                                                   gboolean       value[],
                                                                   gsize          length,
                                                                   GError        **error);

gint                  *egg_desktop_entries_get_integer_list       (EggDesktopEntries  *entries,
                                                                   const gchar    *group,
                                                                   const gchar    *key,
                                                                   gsize          *length,
                                                                   GError        **error);
void                   egg_desktop_entries_set_integer_list       (EggDesktopEntries  *entries,
                                                                   const gchar    *group,
                                                                   const gchar    *key,
                                                                   gint           value[],
                                                                   gsize          length,
                                                                   GError        **error);


void                    egg_desktop_entries_remove_key (EggDesktopEntries *entries,
                                                        const gchar   *group,
                                                        const gchar   *key);

void                    egg_desktop_entries_remove_group (EggDesktopEntries *entries,
                                                          const gchar     *group_name);

G_END_DECLS
#endif /* __EGG_DESKTOP_ENTRIES_H__ */
