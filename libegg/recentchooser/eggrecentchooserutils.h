/* eggrecentchooserutils.h - Private utility functions for implementing a
 *                           EggRecentChooser interface
 *
 * Copyright (C) 2005 Emmanuele Bassi
 *
 * All rights reserved
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
 *
 * Based on gtkfilechooserutils.h:
 *	Copyright (C) 2003 Red Hat, Inc.
 */
 
#ifndef __EGG_RECENT_CHOOSER_UTILS_H__
#define __EGG_RECENT_CHOOSER_UTILS_H__

#include "eggrecentchooserprivate.h"

G_BEGIN_DECLS


#define EGG_RECENT_CHOOSER_DELEGATE_QUARK	(_egg_recent_chooser_delegate_get_quark ())

typedef enum {
  EGG_RECENT_CHOOSER_PROP_FIRST           = 0x3000,
  EGG_RECENT_CHOOSER_PROP_RECENT_MANAGER,
  EGG_RECENT_CHOOSER_PROP_SHOW_PRIVATE,
  EGG_RECENT_CHOOSER_PROP_SHOW_NOT_FOUND,
  EGG_RECENT_CHOOSER_PROP_SHOW_TIPS,
  EGG_RECENT_CHOOSER_PROP_SHOW_ICONS,
  EGG_RECENT_CHOOSER_PROP_SELECT_MULTIPLE,
  EGG_RECENT_CHOOSER_PROP_LIMIT,
  EGG_RECENT_CHOOSER_PROP_LOCAL_ONLY,
  EGG_RECENT_CHOOSER_PROP_SORT_TYPE,
  EGG_RECENT_CHOOSER_PROP_FILTER,
  EGG_RECENT_CHOOSER_PROP_LAST
} EggRecentChooserProp;

void   _egg_recent_chooser_install_properties  (GObjectClass          *klass);

void   _egg_recent_chooser_delegate_iface_init (EggRecentChooserIface *iface);
void   _egg_recent_chooser_set_delegate        (EggRecentChooser      *receiver,
						EggRecentChooser      *delegate);

GQuark _egg_recent_chooser_delegate_get_quark  (void) G_GNUC_CONST;


G_END_DECLS

#endif /* __EGG_RECENT_CHOOSER_UTILS_H__ */
