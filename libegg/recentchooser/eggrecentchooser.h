/* GTK - The GIMP Toolkit
 * eggrecentchooser.h - Abstract interface for recent file selectors GUIs
 *
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

#ifndef __EGG_RECENT_CHOOSER_H__
#define __EGG_RECENT_CHOOSER_H__

#include <gtk/gtkwidget.h>

#include "eggrecentmanager.h"
#include "eggrecentfilter.h"

G_BEGIN_DECLS

#define EGG_TYPE_RECENT_CHOOSER			(egg_recent_chooser_get_type ())
#define EGG_RECENT_CHOOSER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_CHOOSER, EggRecentChooser))
#define EGG_IS_RECENT_CHOOSER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_CHOOSER))
#define EGG_RECENT_CHOOSER_GET_IFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE ((inst), EGG_TYPE_RECENT_CHOOSER, EggRecentChooserIface))

/**
 * EggRecentSortType:
 * @EGG_RECENT_SORT_NONE: Do not sort the returned list of recently used
 *   resources.
 * @EGG_RECENT_SORT_MRU: Sort the returned list with the most recently used
 *   items first.
 * @EGG_RECENT_SORT_LRU: Sort the returned list with the least recently used
 *   items first.
 * @EGG_RECENT_SORT_CUSTOM: Sort the returned list using a custom sorting
 *   function passed using egg_recent_manager_set_sort_func().
 *
 * Used to specify the sorting method to be applyed to the recently
 * used resource list.
 **/
typedef enum
{
  EGG_RECENT_SORT_NONE = 0,
  EGG_RECENT_SORT_MRU,
  EGG_RECENT_SORT_LRU,
  EGG_RECENT_SORT_CUSTOM
} EggRecentSortType;

typedef gint (*EggRecentSortFunc) (EggRecentInfo *a,
				   EggRecentInfo *b,
				   gpointer       user_data);


typedef struct _EggRecentChooser      EggRecentChooser; /* dummy */
typedef struct _EggRecentChooserIface EggRecentChooserIface;

#define EGG_RECENT_CHOOSER_ERROR	(egg_recent_chooser_error_quark ())

typedef enum
{
  EGG_RECENT_CHOOSER_ERROR_NOT_FOUND,
  EGG_RECENT_CHOOSER_ERROR_INVALID_URI
} EggRecentChooserError;

GQuark  egg_recent_chooser_error_quark (void);


struct _EggRecentChooserIface
{
  GTypeInterface base_iface;
  
  /*
   * Methods
   */
  gboolean          (* set_current_uri)    (EggRecentChooser  *chooser,
  					    const gchar       *uri,
  					    GError           **error);
  gchar *           (* get_current_uri)    (EggRecentChooser  *chooser);
  gboolean          (* select_uri)         (EggRecentChooser  *chooser,
  					    const gchar       *uri,
  					    GError           **error);
  void              (* unselect_uri)       (EggRecentChooser  *chooser,
                                            const gchar       *uri);
  void              (* select_all)         (EggRecentChooser  *chooser);
  void              (* unselect_all)       (EggRecentChooser  *chooser);
  GList *           (* get_items)          (EggRecentChooser  *chooser);
  EggRecentManager *(* get_recent_manager) (EggRecentChooser  *chooser);
  void              (* add_filter)         (EggRecentChooser  *chooser,
  					    EggRecentFilter   *filter);
  void              (* remove_filter)      (EggRecentChooser  *chooser,
  					    EggRecentFilter   *filter);
  GSList *          (* list_filters)       (EggRecentChooser  *chooser);
  void              (* set_sort_func)      (EggRecentChooser  *chooser,
  					    EggRecentSortFunc  sort_func,
  					    gpointer           data,
  					    GDestroyNotify     destroy);
  
  /*
   * Signals
   */
  void		    (* item_activated)     (EggRecentChooser  *chooser);
  void		    (* selection_changed)  (EggRecentChooser  *chooser);
};

GType   egg_recent_chooser_get_type    (void) G_GNUC_CONST;

/*
 * Configuration
 */
void              egg_recent_chooser_set_show_private    (EggRecentChooser  *chooser,
							  gboolean           show_private);
gboolean          egg_recent_chooser_get_show_private    (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_show_not_found  (EggRecentChooser  *chooser,
							  gboolean           show_not_found);
gboolean          egg_recent_chooser_get_show_not_found  (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_select_multiple (EggRecentChooser  *chooser,
							  gboolean           select_multiple);
gboolean          egg_recent_chooser_get_select_multiple (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_limit           (EggRecentChooser  *chooser,
							  gint               limit);
gint              egg_recent_chooser_get_limit           (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_local_only      (EggRecentChooser  *chooser,
							  gboolean           local_only);
gboolean          egg_recent_chooser_get_local_only      (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_show_tips       (EggRecentChooser  *chooser,
							  gboolean           show_tips);
gboolean          egg_recent_chooser_get_show_tips       (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_show_numbers    (EggRecentChooser  *chooser,
							  gboolean           show_numbers);
gboolean          egg_recent_chooser_get_show_numbers    (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_show_icons      (EggRecentChooser  *chooser,
							  gboolean           show_icons);
gboolean          egg_recent_chooser_get_show_icons      (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_sort_type       (EggRecentChooser  *chooser,
							  EggRecentSortType  sort_type);
EggRecentSortType egg_recent_chooser_get_sort_type       (EggRecentChooser  *chooser);
void              egg_recent_chooser_set_sort_func       (EggRecentChooser  *chooser,
							  EggRecentSortFunc  sort_func,
							  gpointer           sort_data,
							  GDestroyNotify     data_destroy);

/*
 * Items handling
 */
gboolean       egg_recent_chooser_set_current_uri  (EggRecentChooser  *chooser,
						    const gchar       *uri,
						    GError           **error);
gchar *        egg_recent_chooser_get_current_uri  (EggRecentChooser  *chooser);
EggRecentInfo *egg_recent_chooser_get_current_item (EggRecentChooser  *chooser);
gboolean       egg_recent_chooser_select_uri       (EggRecentChooser  *chooser,
						    const gchar       *uri,
						    GError           **error);
void           egg_recent_chooser_unselect_uri     (EggRecentChooser  *chooser,
					            const gchar       *uri);
void           egg_recent_chooser_select_all       (EggRecentChooser  *chooser);
void           egg_recent_chooser_unselect_all     (EggRecentChooser  *chooser);
GList *        egg_recent_chooser_get_items        (EggRecentChooser  *chooser);
gchar **       egg_recent_chooser_get_uris         (EggRecentChooser  *chooser,
						    gsize             *length);

/*
 * Filters
 */
void 		 egg_recent_chooser_add_filter    (EggRecentChooser *chooser,
			 			   EggRecentFilter  *filter);
void 		 egg_recent_chooser_remove_filter (EggRecentChooser *chooser,
						   EggRecentFilter  *filter);
GSList * 	 egg_recent_chooser_list_filters  (EggRecentChooser *chooser);
void 		 egg_recent_chooser_set_filter    (EggRecentChooser *chooser,
						   EggRecentFilter  *filter);
EggRecentFilter *egg_recent_chooser_get_filter    (EggRecentChooser *chooser);


G_END_DECLS

#endif /* __EGG_RECENT_CHOOSER_H__ */
