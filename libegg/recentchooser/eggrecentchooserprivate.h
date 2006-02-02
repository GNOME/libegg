/* eggrecentprivatechooser.h - Interface definitions for recent selectors UI
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
 */
 
#ifndef __EGG_RECENT_CHOOSER_PRIVATE_H__
#define __EGG_RECENT_CHOOSER_PRIVATE_H__

#include <glib-object.h>

#include "eggrecentmanager.h"
#include "eggrecentchooser.h"

G_BEGIN_DECLS

EggRecentManager *_egg_recent_chooser_get_recent_manager (EggRecentChooser *chooser);

void              _egg_recent_chooser_item_activated     (EggRecentChooser *chooser);
void              _egg_recent_chooser_selection_changed  (EggRecentChooser *chooser);

G_END_DECLS
 
#endif /* ! __EGG_RECENT_CHOOSER_PRIVATE_H__ */
