/* EGG - The GIMP Toolkit
 * eggprintsettings.h: Print Settings
 * Copyright (C) 2005, Red Hat, Inc.
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

#ifndef __EGG_PRINT_SETTINGS_H__
#define __EGG_PRINT_SETTINGS_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _EggPrintSettings EggPrintSettings;

#define EGG_TYPE_PRINT_SETTINGS    (egg_print_settings_get_type ())
#define EGG_PRINT_SETTINGS(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_SETTINGS, EggPrintSettings))
#define EGG_IS_PRINT_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_SETTINGS))

GType egg_print_settings_get_type (void);

EggPrintSettings *egg_print_settings_new (void);

EggPrintSettings *egg_print_settings_copy (EggPrintSettings *other);

void egg_print_settings_install_property (GParamSpec *pspec);

G_END_DECLS

#endif /* __EGG_PRINT_SETTINGS_H__ */
