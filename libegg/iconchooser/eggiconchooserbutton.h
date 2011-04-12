/* eggiconchooserbutton.h
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

#ifndef __EGG_ICON_CHOOSER_BUTTON_H__
#define __EGG_ICON_CHOOSER_BUTTON_H__ 1

#include <gtk/gtk.h>
#include "eggiconchooser.h"

G_BEGIN_DECLS

#define EGG_TYPE_ICON_CHOOSER_BUTTON \
  (egg_icon_chooser_button_get_type ())
#define EGG_ICON_CHOOSER_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_ICON_CHOOSER_BUTTON, EggIconChooserButton))
#define EGG_IS_ICON_CHOOSER_BUTTON(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_ICON_CHOOSER_BUTTON))
#define EGG_ICON_CHOOSER_BUTTON_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST ((obj), EGG_TYPE_ICON_CHOOSER_BUTTON, EggIconChooserButtonClass))
#define EGG_IS_ICON_CHOOSER_BUTTON_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_ICON_CHOOSER_BUTTON))
#define EGG_ICON_CHOOSER_BUTTON_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_ICON_CHOOSER_BUTTON, EggIconChooserButtonClass))


typedef struct _EggIconChooserButton EggIconChooserButton;
typedef struct _EggIconChooserButtonPrivate EggIconChooserButtonPrivate;
typedef struct _EggIconChooserButtonClass EggIconChooserButtonClass;

struct _EggIconChooserButton
{
  GtkVBox parent;

  EggIconChooserButtonPrivate *priv;
};

struct _EggIconChooserButtonClass
{
  GtkVBoxClass parent_class;

  void (*__egg_reserved1);
  void (*__egg_reserved2);
  void (*__egg_reserved3);
  void (*__egg_reserved4);
  void (*__egg_reserved5);
  void (*__egg_reserved6);
  void (*__egg_reserved7);
  void (*__egg_reserved8);
};

GType      egg_icon_chooser_button_get_type         (void) G_GNUC_CONST;

GtkWidget *egg_icon_chooser_button_new              (const gchar *title);
GtkWidget *egg_icon_chooser_button_new_with_backend (const gchar *title,
						     const gchar *file_system_backend);
GtkWidget *egg_icon_chooser_button_new_with_dialog  (GtkWidget   *dialog);

void                  egg_icon_chooser_button_set_title  (EggIconChooserButton *button,
						          const gchar          *title);
G_CONST_RETURN gchar *egg_icon_chooser_button_get_title  (EggIconChooserButton *button);

G_END_DECLS

#endif /* !__EGG_ICON_CHOOSER_BUTTON_H__ */
