/* GTK - The GIMP Toolkit
 * eggprintbackendsetting.h: printer setting
 * Copyright (C) 2006, Red Hat, Inc.
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

#ifndef __EGG_PRINT_BACKEND_SETTING_SET_H__
#define __EGG_PRINT_BACKEND_SETTING_SET_H__

/* This is a "semi-private" header; it is meant only for
 * alternate EggPrintDialog backend modules; no stability guarantees 
 * are made at this point
 */
#ifndef EGG_PRINT_BACKEND_ENABLE_UNSUPPORTED
#error "EggPrintBackend is not supported API for general use"
#endif

#include <glib-object.h>
#include "eggprintbackendsetting.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_BACKEND_SETTING_SET             (egg_print_backend_setting_set_get_type ())
#define EGG_PRINT_BACKEND_SETTING_SET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_BACKEND_SETTING_SET, EggPrintBackendSettingSet))
#define EGG_IS_PRINT_BACKEND_SETTING_SET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_BACKEND_SETTING_SET))

typedef struct _EggPrintBackendSettingSet       EggPrintBackendSettingSet;
typedef struct _EggPrintBackendSettingSetClass  EggPrintBackendSettingSetClass;

struct _EggPrintBackendSettingSet
{
  GObject parent_instance;

  /*< private >*/
  GPtrArray *array;
  GHashTable *hash;
};

struct _EggPrintBackendSettingSetClass
{
  GObjectClass parent_class;

  void (*changed) (EggPrintBackendSettingSet *setting);
};

typedef void (*EggPrintBackendSettingSetFunc) (EggPrintBackendSetting  *setting,
					       gpointer                 user_data);


GType   egg_print_backend_setting_set_get_type       (void) G_GNUC_CONST;

EggPrintBackendSettingSet *egg_print_backend_setting_set_new              (void);
void                       egg_print_backend_setting_set_add              (EggPrintBackendSettingSet     *set,
									   EggPrintBackendSetting        *setting);
void                       egg_print_backend_setting_set_remove           (EggPrintBackendSettingSet     *set,
									   EggPrintBackendSetting        *setting);
EggPrintBackendSetting *   egg_print_backend_setting_set_lookup           (EggPrintBackendSettingSet     *set,
									   const char                    *name);
void                       egg_print_backend_setting_set_foreach          (EggPrintBackendSettingSet     *set,
									   EggPrintBackendSettingSetFunc  func,
									   gpointer                       user_data);
void                       egg_print_backend_setting_set_clear_conflicts  (EggPrintBackendSettingSet     *set);
GList *                    egg_print_backend_setting_set_get_groups       (EggPrintBackendSettingSet     *set);
void                       egg_print_backend_setting_set_foreach_in_group (EggPrintBackendSettingSet     *set,
									   const char                    *group,
									   EggPrintBackendSettingSetFunc  func,
									   gpointer                       user_data);

G_END_DECLS

#endif /* __EGG_PRINT_BACKEND_SETTING_SET_H__ */
