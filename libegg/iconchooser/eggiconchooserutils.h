/* eggiconchooserutils.h
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

#ifndef __EGG_ICON_CHOOSER_UTILS_H__
#define __EGG_ICON_CHOOSER_UTILS_H__

#include "eggiconchooserprivate.h"

G_BEGIN_DECLS

#define EGG_ICON_CHOOSER_DELEGATE_QUARK	(_egg_icon_chooser_get_delegate_quark ())

typedef enum {
  EGG_ICON_CHOOSER_PROP_FIRST            = 0x2000,
  EGG_ICON_CHOOSER_PROP_FILE_SYSTEM,
  EGG_ICON_CHOOSER_PROP_CONTEXT,
  EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE,
  EGG_ICON_CHOOSER_PROP_ICON_SIZE,
  EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME,
  EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM,
  EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER,
  EGG_ICON_CHOOSER_PROP_LAST
} EggIonChooserProp;

void _egg_icon_chooser_install_properties  (GObjectClass *klass);

void _egg_icon_chooser_delegate_iface_init (EggIconChooserIface *iface);
void _egg_icon_chooser_set_delegate        (EggIconChooser *receiver,
					    EggIconChooser *delegate);

GQuark _egg_icon_chooser_get_delegate_quark (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __EGG_ICON_CHOOSER_UTILS_H__ */
