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

#ifndef __EGG_PRINT_BACKEND_SETTING_H__
#define __EGG_PRINT_BACKEND_SETTING_H__

/* This is a "semi-private" header; it is meant only for
 * alternate EggPrintDialog backend modules; no stability guarantees 
 * are made at this point
 */
#ifndef EGG_PRINT_BACKEND_ENABLE_UNSUPPORTED
#error "EggPrintBackend is not supported API for general use"
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_BACKEND_SETTING             (egg_print_backend_setting_get_type ())
#define EGG_PRINT_BACKEND_SETTING(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_BACKEND_SETTING, EggPrintBackendSetting))
#define EGG_IS_PRINT_BACKEND_SETTING(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_BACKEND_SETTING))

typedef struct _EggPrintBackendSetting       EggPrintBackendSetting;
typedef struct _EggPrintBackendSettingClass  EggPrintBackendSettingClass;

#define EGG_PRINT_BACKEND_SETTING_GROUP_IMAGE_QUALITY "ImageQuality"
#define EGG_PRINT_BACKEND_SETTING_GROUP_FINISHING "Finishing"

typedef enum {
  EGG_PRINT_BACKEND_SETTING_TYPE_BOOLEAN,
  EGG_PRINT_BACKEND_SETTING_TYPE_PICKONE,
  EGG_PRINT_BACKEND_SETTING_TYPE_STRING
} EggPrintBackendSettingType;

struct _EggPrintBackendSetting
{
  GObject parent_instance;

  char *name;
  char *display_text;
  EggPrintBackendSettingType type;

  char *value;
  
  int num_choices;
  char **choices;
  char **choices_display;
  
  gboolean has_conflict;
  char *group;
};

struct _EggPrintBackendSettingClass
{
  GObjectClass parent_class;

  void (*changed) (EggPrintBackendSetting *setting);


  /* Padding for future expansion */
  void (*_egg_reserved1) (void);
  void (*_egg_reserved2) (void);
  void (*_egg_reserved3) (void);
  void (*_egg_reserved4) (void);
  void (*_egg_reserved5) (void);
  void (*_egg_reserved6) (void);
  void (*_egg_reserved7) (void);
};

GType   egg_print_backend_setting_get_type       (void) G_GNUC_CONST;

EggPrintBackendSetting *egg_print_backend_setting_new              (const char                 *name,
								    const char                 *display_text,
								    EggPrintBackendSettingType  type);
void                    egg_print_backend_setting_set              (EggPrintBackendSetting     *setting,
								    const char                 *value);
void                    egg_print_backend_setting_set_has_conflict (EggPrintBackendSetting     *setting,
								    gboolean                    has_conflict);
void                    egg_print_backend_setting_clear_has_conflict (EggPrintBackendSetting     *setting);
void                    egg_print_backend_setting_set_boolean      (EggPrintBackendSetting     *setting,
								    gboolean                    value);

void                    egg_print_backend_setting_allocate_choices (EggPrintBackendSetting     *setting,
								    int                         num);
void                    egg_print_backend_setting_choices_from_array (EggPrintBackendSetting   *setting,
								      int                       num_choices,
								      char                     *choices[],
								      char                     *choices_display[]);


G_END_DECLS

#endif /* __EGG_PRINT_BACKEND_SETTING_H__ */


