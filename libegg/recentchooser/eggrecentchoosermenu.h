/* GTK - The GIMP Toolkit
 * eggrecentchoosermenu.h - Recently used items menu widget
 * Copyright (C) 2005, Emmanuele Bassi
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

#ifndef __EGG_RECENT_CHOOSER_MENU_H__
#define __EGG_RECENT_CHOOSER_MENU_H__

#include <gtk/gtkmenu.h>
#include "eggrecentchooser.h"

G_BEGIN_DECLS

#define EGG_TYPE_RECENT_CHOOSER_MENU		(egg_recent_chooser_menu_get_type ())
#define EGG_RECENT_CHOOSER_MENU(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_CHOOSER_MENU, EggRecentChooserMenu))
#define EGG_IS_RECENT_CHOOSER_MENU(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_CHOOSER_MENU))
#define EGG_RECENT_CHOOSER_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_RECENT_CHOOSER_MENU, EggRecentChooserMenuClass))
#define EGG_IS_RECENT_CHOOSER_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_RECENT_CHOOSER_MENU))
#define EGG_RECENT_CHOOSER_MENU_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_RECENT_CHOOSER_MENU, EggRecentChooserMenuClass))

typedef struct _EggRecentChooserMenu		EggRecentChooserMenu;
typedef struct _EggRecentChooserMenuClass	EggRecentChooserMenuClass;
typedef struct _EggRecentChooserMenuPrivate	EggRecentChooserMenuPrivate;

struct _EggRecentChooserMenu
{
  /*< private >*/
  GtkMenu parent_instance;

  EggRecentChooserMenuPrivate *priv;
};

struct _EggRecentChooserMenuClass
{
  GtkMenuClass parent_class;
  
  /* padding for future expansion */
  void (* egg_recent1) (void);
  void (* egg_recent2) (void);
  void (* egg_recent3) (void);
  void (* egg_recent4) (void);
};

GType      egg_recent_chooser_menu_get_type         (void) G_GNUC_CONST;

GtkWidget *egg_recent_chooser_menu_new              (void);
GtkWidget *egg_recent_chooser_menu_new_for_manager  (EggRecentManager     *manager);

G_END_DECLS

#endif /* ! __EGG_RECENT_CHOOSER_MENU_H__ */
