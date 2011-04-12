/* eggiconchooser.h
 * Copyright (C) 2003  James M. Cape  <jcape@ignore-your.tv>
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
 * Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_ICON_CHOOSER_H__
#define __EGG_ICON_CHOOSER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define EGG_TYPE_ICON_CHOOSER \
  (egg_icon_chooser_get_type ())
#define EGG_ICON_CHOOSER(object) \
  (G_TYPE_CHECK_INSTANCE_CAST ((object), EGG_TYPE_ICON_CHOOSER, EggIconChooser))
#define EGG_IS_ICON_CHOOSER(object) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((object), EGG_TYPE_ICON_CHOOSER))


typedef enum /* <prefix=EGG_ICON_CONTEXT> */
{
  EGG_ICON_CONTEXT_INVALID,
  EGG_ICON_CONTEXT_ALL,
  EGG_ICON_CONTEXT_FILE,

  EGG_ICON_CONTEXT_APPLICATIONS,
  EGG_ICON_CONTEXT_LOCATIONS,
  EGG_ICON_CONTEXT_EMBLEMS,
  EGG_ICON_CONTEXT_MIMETYPES,
  EGG_ICON_CONTEXT_STOCK,
  EGG_ICON_CONTEXT_UNCATEGORIZED,

  EGG_ICON_CONTEXT_LAST
}
EggIconContext;


typedef struct _EggIconChooser EggIconChooser;


GType           egg_icon_chooser_get_type (void) G_GNUC_CONST;

gboolean        egg_icon_chooser_get_select_multiple  (EggIconChooser *chooser);
void	        egg_icon_chooser_set_select_multiple  (EggIconChooser *chooser,
						       gboolean        select_multiple);

void	        egg_icon_chooser_unselect_all         (EggIconChooser *chooser);

EggIconContext  egg_icon_chooser_get_context          (EggIconChooser *chooser);
void            egg_icon_chooser_set_context          (EggIconChooser *chooser,
						       EggIconContext  context);

gint            egg_icon_chooser_get_icon_size        (EggIconChooser *chooser);
void            egg_icon_chooser_set_icon_size        (EggIconChooser *chooser,
						       gint            icon_size);

gboolean        egg_icon_chooser_get_show_icon_name   (EggIconChooser *chooser);
void            egg_icon_chooser_set_show_icon_name   (EggIconChooser *chooser,
						       gboolean        show_name);

GSList	       *egg_icon_chooser_get_icons            (EggIconChooser *chooser);
gchar          *egg_icon_chooser_get_icon             (EggIconChooser *chooser);
void            egg_icon_chooser_set_icon             (EggIconChooser *chooser,
						       const gchar    *icon);
gboolean        egg_icon_chooser_select_icon          (EggIconChooser *chooser,
						       const gchar    *icon_name);
void            egg_icon_chooser_unselect_icon        (EggIconChooser *chooser,
						       const gchar    *icon_name);

gboolean        egg_icon_chooser_get_allow_custom     (EggIconChooser *chooser);
void	        egg_icon_chooser_set_allow_custom     (EggIconChooser *chooser,
						       gboolean        allow_custom);

GSList	       *egg_icon_chooser_get_uris             (EggIconChooser *chooser);
gchar          *egg_icon_chooser_get_uri              (EggIconChooser *chooser);
void            egg_icon_chooser_set_uri              (EggIconChooser *chooser,
						       const gchar    *uri);
gboolean        egg_icon_chooser_select_uri           (EggIconChooser *chooser,
						       const gchar    *uri);
void            egg_icon_chooser_unselect_uri         (EggIconChooser *chooser,
						       const gchar    *uri);

GSList	       *egg_icon_chooser_get_filenames        (EggIconChooser *chooser);
gchar          *egg_icon_chooser_get_filename         (EggIconChooser *chooser);
void            egg_icon_chooser_set_filename         (EggIconChooser *chooser,
						       const gchar    *filename);
gboolean        egg_icon_chooser_select_filename      (EggIconChooser *chooser,
						       const gchar    *filename);
void            egg_icon_chooser_unselect_filename    (EggIconChooser *chooser,
						       const gchar    *filename);

GSList	       *egg_icon_chooser_list_custom_filters  (EggIconChooser *chooser);
void	        egg_icon_chooser_add_custom_filter    (EggIconChooser *chooser,
						       GtkFileFilter  *filter);
void	        egg_icon_chooser_remove_custom_filter (EggIconChooser *chooser,
						       GtkFileFilter  *filter);

GtkFileFilter  *egg_icon_chooser_get_custom_filter    (EggIconChooser *chooser);
void            egg_icon_chooser_set_custom_filter    (EggIconChooser *chooser,
						       GtkFileFilter  *filter);


G_END_DECLS

#endif /* __EGG_ICON_CHOOSER_H__ */
