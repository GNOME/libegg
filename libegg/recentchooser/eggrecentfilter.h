/* GTK - The GIMP Toolkit
 * eggrecentfilter.h - Filter object for recently used resources
 * Copyright (C) 2005, Emmanuele Bassi
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_RECENT_FILTER_H__
#define __EGG_RECENT_FILTER_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define EGG_TYPE_RECENT_FILTER		(egg_recent_filter_get_type ())
#define EGG_RECENT_FILTER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_FILTER, EggRecentFilter))
#define EGG_IS_RECENT_FILTER(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_FILTER))

typedef struct _EggRecentFilter		EggRecentFilter;
typedef struct _EggRecentFilterInfo	EggRecentFilterInfo;

typedef enum {
  EGG_RECENT_FILTER_URI          = 1 << 0,
  EGG_RECENT_FILTER_DISPLAY_NAME = 1 << 1,
  EGG_RECENT_FILTER_MIME_TYPE    = 1 << 2,
  EGG_RECENT_FILTER_APPLICATION  = 1 << 3,
  EGG_RECENT_FILTER_GROUP        = 1 << 4,
  EGG_RECENT_FILTER_AGE          = 1 << 5
} EggRecentFilterFlags;

typedef gboolean (*EggRecentFilterFunc) (const EggRecentFilterInfo *filter_info,
					 gpointer                   user_data);

struct _EggRecentFilterInfo
{
  EggRecentFilterFlags contains;

  const gchar *uri;
  const gchar *display_name;
  const gchar *mime_type;
  const gchar **applications;
  const gchar **groups;
  
  gint age;
};

GType                 egg_recent_filter_get_type (void) G_GNUC_CONST;

EggRecentFilter *     egg_recent_filter_new      (void);
void                  egg_recent_filter_set_name (EggRecentFilter *filter,
						  const gchar     *name);
G_CONST_RETURN gchar *egg_recent_filter_get_name (EggRecentFilter *filter);

void egg_recent_filter_add_mime_type   (EggRecentFilter      *filter,
					const gchar          *mime_type);
void egg_recent_filter_add_pattern     (EggRecentFilter      *filter,
					const gchar          *pattern);
void egg_recent_filter_add_application (EggRecentFilter      *filter,
					const gchar          *application);
void egg_recent_filter_add_group       (EggRecentFilter      *filter,
					const gchar          *group);
void egg_recent_filter_add_age         (EggRecentFilter      *filter,
					gint                  days);
void egg_recent_filter_add_custom      (EggRecentFilter      *filter,
					EggRecentFilterFlags  needed,
					EggRecentFilterFunc   func,
					gpointer              data,
					GDestroyNotify        data_destroy);

EggRecentFilterFlags egg_recent_filter_get_needed (EggRecentFilter           *filter);
gboolean             egg_recent_filter_filter     (EggRecentFilter           *filter,
						   const EggRecentFilterInfo *filter_info);

G_END_DECLS

#endif /* ! __EGG_RECENT_FILTER_H__ */
