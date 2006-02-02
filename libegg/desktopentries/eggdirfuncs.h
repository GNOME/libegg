/* Some helper functions that gdesktopentries needs
 * Copyright (C) 2004 Ray Strode
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __EGG_DIR_FUNCS_H__
#define __EGG_DIR_FUNCS_H__

#include "glib.h"

G_BEGIN_DECLS

gchar* egg_get_user_data_dir (void);
gchar* egg_get_user_configuration_dir (void);
gchar* egg_get_user_cache_dir (void);
gchar** egg_get_secondary_data_dirs (void);
gchar** egg_get_secondary_configuration_dirs (void);
gchar** egg_get_data_dirs (void);
gchar** egg_get_configuration_dirs (void);

G_END_DECLS

#endif
