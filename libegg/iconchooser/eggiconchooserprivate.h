/* Egg Libraries: eggiconchooserprivate.h
 * 
 * Copyright (c) 2004 James M. Cape <jcape@ignore-your.tv>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_ICON_CHOOSER_PRIVATE_H__
#define __EGG_ICON_CHOOSER_PRIVATE_H__ 1

#include "eggiconchooser.h"

#define GTK_FILE_SYSTEM_ENABLE_UNSUPPORTED
#include <gtk/gtkfilesystem.h>

G_BEGIN_DECLS


#define EGG_ICON_CHOOSER_GET_IFACE(obj) \
  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EGG_TYPE_ICON_CHOOSER, EggIconChooserIface))

typedef struct _EggIconChooserIface EggIconChooserIface;

struct _EggIconChooserIface
{
  GTypeInterface g_iface;

  /* Methods */
  void           (*unselect_all)           (EggIconChooser     *chooser);

  /* Icon Selection */
  gboolean       (*select_icon)            (EggIconChooser     *chooser,
					    const gchar        *icon_name);
  void           (*unselect_icon)          (EggIconChooser     *chooser,
					    const gchar        *icon_name);
  GSList        *(*get_icons)              (EggIconChooser     *chooser);

  /* File Selection */
  GtkFileSystem *(*get_file_system)        (EggIconChooser     *chooser);
  gboolean       (*select_path)            (EggIconChooser     *chooser,
					    const GtkFilePath  *path,
					    GError            **error);
  void           (*unselect_path)          (EggIconChooser     *chooser,
				            const GtkFilePath  *path);
  GSList        *(*get_paths)              (EggIconChooser     *chooser);

  /* File Filters */
  void	         (*add_filter)	           (EggIconChooser     *chooser,
					    GtkFileFilter      *filter);
  void	         (*remove_filter)	   (EggIconChooser     *chooser,
					    GtkFileFilter      *filter);
  GSList        *(*list_filters)           (EggIconChooser     *chooser);

  /* Signals */
  void           (*icon_selection_changed) (EggIconChooser     *chooser);
  void           (*icon_activated)         (EggIconChooser     *chooser);
  void           (*file_selection_changed) (EggIconChooser     *chooser);
  void           (*file_activated)         (EggIconChooser     *chooser);
};


GtkFileSystem *_egg_icon_chooser_get_file_system  (EggIconChooser *chooser);

gboolean  _egg_icon_chooser_select_path           (EggIconChooser    *chooser,
						   const GtkFilePath *path,
						   GError           **error);
void      _egg_icon_chooser_unselect_path         (EggIconChooser    *chooser,
						   const GtkFilePath *path);
GSList   *_egg_icon_chooser_get_paths             (EggIconChooser *chooser);

void     _egg_icon_chooser_icon_selection_changed (EggIconChooser *chooser);
void     _egg_icon_chooser_icon_activated         (EggIconChooser *chooser);
void     _egg_icon_chooser_file_selection_changed (EggIconChooser *chooser);
void     _egg_icon_chooser_file_activated         (EggIconChooser *chooser);


G_END_DECLS

#endif /* !__EGG_ICON_CHOOSER_PRIVATE_H__ */
