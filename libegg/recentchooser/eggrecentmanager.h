/* GTK - The GIMP Toolkit
 * eggrecentmanager.h: a manager for the recently used resources
 *
 * Copyright (C) 2005 Emmanuele Bassi
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

#ifndef __EGG_RECENT_MANAGER_H__
#define __EGG_RECENT_MANAGER_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <time.h>

G_BEGIN_DECLS

#define EGG_TYPE_RECENT_INFO			(egg_recent_info_get_type ())

#define EGG_TYPE_RECENT_MANAGER			(egg_recent_manager_get_type ())
#define EGG_RECENT_MANAGER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_MANAGER, EggRecentManager))
#define EGG_IS_RECENT_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_MANAGER))
#define EGG_RECENT_MANAGER_CLASS(klass) 	(G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_RECENT_MANAGER, EggRecentManagerClass))
#define EGG_IS_RECENT_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_RECENT_MANAGER))
#define EGG_RECENT_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_RECENT_MANAGER, EggRecentManagerClass))

typedef struct _EggRecentInfo		EggRecentInfo;
typedef struct _EggRecentData		EggRecentData;
typedef struct _EggRecentManager	EggRecentManager;
typedef struct _EggRecentManagerClass	EggRecentManagerClass;
typedef struct _EggRecentManagerPrivate EggRecentManagerPrivate;

/**
 * EggRecentData:
 *
 * @display_name: a UTF-8 encoded string, containing the name of the recently
 *   used resource to be displayed, or %NULL;
 * @description: a UTF-8 encoded string, containing a short description of
 *   the resource, or %NULL;
 * @mime_type: the MIME type of the resource;
 * @app_name: the name of the application that is registering this recently
 *   used resource;
 * @app_exec: command line used to launch this resource; may contain the
 *   "%f" and "%u" escape characters which will be expanded to the resource
 *   file path and URI respectively when the command line is retrieved;
 * @groups: a vector of strings containing groups names;
 * @is_private: whether this resource should be displayed only by the
 *   applications that have registered it or not.
 *
 * Meta-data to be passed to egg_recent_manager_add_full() when
 * registering a recently used resource.
 **/
struct _EggRecentData
{
  gchar *display_name;
  gchar *description;
  
  gchar *mime_type;
  
  gchar *app_name;
  gchar *app_exec;
  
  gchar **groups;
  
  gboolean is_private;
};

struct _EggRecentManager
{
  /*< private >*/
  GObject parent_instance;
  
  EggRecentManagerPrivate *priv;
};

struct _EggRecentManagerClass
{
  /*< private >*/
  GObjectClass parent_class;
  
  void (*changed) (EggRecentManager *recent_manager);
  
  /* padding for future expansion */
  void (*_egg_recent1) (void);
  void (*_egg_recent2) (void);
  void (*_egg_recent3) (void);
  void (*_egg_recent4) (void);
};

/**
 * EggRecentManagerError:
 * @EGG_RECENT_MANAGER_ERROR_NOT_FOUND: the URI specified does not exists in
 *   the recently used resources list.
 * @EGG_RECENT_MANAGER_ERROR_INVALID_URI: the URI specified is not valid.
 * @EGG_RECENT_MANAGER_ERROR_INVALID_MIME: the MIME type specified is not
 *   valid.
 * @EGG_RECENT_MANAGER_ERROR_INVALID_ENCODING: the supplied string is not
 *   UTF-8 encoded.
 * @EGG_RECENT_MANAGER_ERROR_NOT_REGISTERED: no application has registered
 *   the specified item.
 * @EGG_RECENT_MANAGER_ERROR_READ: failure while reading the recently used
 *   resources file.
 * @EGG_RECENT_MANAGER_ERROR_WRITE: failure while writing the recently used
 *   resources file.
 * @EGG_RECENT_MANAGER_ERROR_UNKNOWN: unspecified error.
 *
 * Error codes for EggRecentManager operations
 **/
typedef enum
{
  EGG_RECENT_MANAGER_ERROR_NOT_FOUND,
  EGG_RECENT_MANAGER_ERROR_INVALID_URI,
  EGG_RECENT_MANAGER_ERROR_INVALID_MIME,
  EGG_RECENT_MANAGER_ERROR_INVALID_ENCODING,
  EGG_RECENT_MANAGER_ERROR_NOT_REGISTERED,
  EGG_RECENT_MANAGER_ERROR_BAD_EXEC_STRING,
  EGG_RECENT_MANAGER_ERROR_READ,
  EGG_RECENT_MANAGER_ERROR_WRITE,
  EGG_RECENT_MANAGER_ERROR_UNKNOWN
} EggRecentManagerError;

#define EGG_RECENT_MANAGER_ERROR	(egg_recent_manager_error_quark ())
GQuark 	egg_recent_manager_error_quark (void);


GType 		  egg_recent_manager_get_type    (void) G_GNUC_CONST;

EggRecentManager *egg_recent_manager_new         (void);
gboolean          egg_recent_manager_add_item    (EggRecentManager     *manager,
						  const gchar          *uri,
						  GError              **error);
gboolean          egg_recent_manager_add_full    (EggRecentManager     *manager,
						  const gchar          *uri,
						  const EggRecentData  *recent_data,
						  GError              **error);
gboolean          egg_recent_manager_remove_item (EggRecentManager     *manager,
						  const gchar          *uri,
						  GError              **error);
EggRecentInfo *   egg_recent_manager_lookup_item (EggRecentManager     *manager,
						  const gchar          *uri,
						  GError              **error);
gboolean          egg_recent_manager_has_item    (EggRecentManager     *manager,
						  const gchar          *uri);
gboolean          egg_recent_manager_move_item   (EggRecentManager     *manager,
						  const gchar          *uri,
						  const gchar          *new_uri,
						  GError              **error);
void              egg_recent_manager_set_limit   (EggRecentManager     *manager,
						  gint                  limit);
gint              egg_recent_manager_get_limit   (EggRecentManager     *manager);
GList *           egg_recent_manager_get_items   (EggRecentManager     *manager);
gint              egg_recent_manager_purge_items (EggRecentManager     *manager,
						  GError              **error);


GType	              egg_recent_info_get_type             (void) G_GNUC_CONST;

EggRecentInfo *       egg_recent_info_ref                  (EggRecentInfo  *info);
void                  egg_recent_info_unref                (EggRecentInfo  *info);

G_CONST_RETURN gchar *egg_recent_info_get_uri              (EggRecentInfo  *info);
G_CONST_RETURN gchar *egg_recent_info_get_display_name     (EggRecentInfo  *info);
G_CONST_RETURN gchar *egg_recent_info_get_description      (EggRecentInfo  *info);
G_CONST_RETURN gchar *egg_recent_info_get_mime_type        (EggRecentInfo  *info);
time_t                egg_recent_info_get_added            (EggRecentInfo  *info);
time_t                egg_recent_info_get_modified         (EggRecentInfo  *info);
time_t                egg_recent_info_get_visited          (EggRecentInfo  *info);
gboolean              egg_recent_info_get_private_hint     (EggRecentInfo  *info);
gboolean              egg_recent_info_get_application_info (EggRecentInfo  *info,
							    const gchar    *app_name,
							    gchar         **app_exec,
							    guint          *count,
							    time_t         *time);
gchar **              egg_recent_info_get_applications     (EggRecentInfo  *info,
							    gsize          *length) G_GNUC_MALLOC;
gchar *               egg_recent_info_last_application     (EggRecentInfo  *info) G_GNUC_MALLOC;
gboolean              egg_recent_info_has_application      (EggRecentInfo  *info,
							    const gchar    *app_name);
gchar **              egg_recent_info_get_groups           (EggRecentInfo  *info,
							    gsize          *length) G_GNUC_MALLOC;
gboolean              egg_recent_info_has_group            (EggRecentInfo  *info,
							    const gchar    *group_name);
GdkPixbuf *           egg_recent_info_get_icon             (EggRecentInfo  *info,
							    gint            size);
gchar *               egg_recent_info_get_short_name       (EggRecentInfo  *info) G_GNUC_MALLOC;
gchar *               egg_recent_info_get_uri_display      (EggRecentInfo  *info) G_GNUC_MALLOC;
gint                  egg_recent_info_get_age              (EggRecentInfo  *info);
gboolean              egg_recent_info_is_local             (EggRecentInfo  *info);
gboolean              egg_recent_info_exists               (EggRecentInfo  *info);
gboolean              egg_recent_info_match                (EggRecentInfo  *info_a,
							    EggRecentInfo  *info_b);

G_END_DECLS

#endif /* __EGG_RECENT_MANAGER_H__ */
