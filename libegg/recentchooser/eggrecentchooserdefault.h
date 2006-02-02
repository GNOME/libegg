/* GTK - The GIMP Toolkit
 * eggrecentchooserdefault.h
 * Copyright (C) 2005-2006 Emmanuele Bassi
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

#ifndef __EGG_RECENT_CHOOSER_DEFAULT_H__
#define __EGG_RECENT_CHOOSER_DEFAULT_H__

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS


#define EGG_TYPE_RECENT_CHOOSER_DEFAULT    (egg_recent_chooser_default_get_type ())
#define EGG_RECENT_CHOOSER_DEFAULT(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_CHOOSER_DEFAULT, EggRecentChooserDefault))
#define EGG_IS_RECENT_CHOOSER_DEFAULT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_CHOOSER_DEFAULT))


typedef struct _EggRecentChooserDefault        EggRecentChooserDefault;

GType      _egg_recent_chooser_default_get_type (void) G_GNUC_CONST;
GtkWidget *_egg_recent_chooser_default_new      (EggRecentManager *recent_manager);


G_END_DECLS

#endif /* __EGG_RECENT_CHOOSER_DEFAULT_H__ */
