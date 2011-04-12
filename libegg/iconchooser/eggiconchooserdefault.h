/* eggiconchooserdefault.h
 * Copyright (C) 2004  James M. Cape  <jcape@ignore-your.tv>
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

#ifndef __EGG_ICON_CHOOSER_DEFAULT_H__
#define __EGG_ICON_CHOOSER_DEFAULT_H__ 1

#include <gtk/gtk.h>

G_BEGIN_DECLS


#define EGG_TYPE_ICON_CHOOSER_DEFAULT    (_egg_icon_chooser_default_get_type ())
#define EGG_ICON_CHOOSER_DEFAULT(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_ICON_CHOOSER_DEFAULT, EggIconChooserDefault))
#define EGG_IS_ICON_CHOOSER_DEFAULT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_ICON_CHOOSER_DEFAULT))


typedef struct _EggIconChooserDefault EggIconChooserDefault;

GType _egg_icon_chooser_default_get_type (void) G_GNUC_CONST;

GtkWidget *_egg_icon_chooser_default_new (void);


G_END_DECLS

#endif /* __EGG_ICON_CHOOSER_DEFAULT_H__ */
