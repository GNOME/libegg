/* eggiconchooserdialog.h
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

#ifndef __EGG_ICON_CHOOSER_DIALOG_H__
#define __EGG_ICON_CHOOSER_DIALOG_H__ 1

#include <gtk/gtk.h>
#include "eggiconchooser.h"

G_BEGIN_DECLS


#define EGG_TYPE_ICON_CHOOSER_DIALOG \
  (egg_icon_chooser_dialog_get_type ())
#define EGG_ICON_CHOOSER_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_ICON_CHOOSER_DIALOG, EggIconChooserDialog))
#define EGG_IS_ICON_CHOOSER_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_ICON_CHOOSER_DIALOG))
#define EGG_ICON_CHOOSER_DIALOG_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_CAST ((obj), EGG_TYPE_ICON_CHOOSER_DIALOG, EggIconChooserDialogClass))
#define EGG_IS_ICON_CHOOSER_DIALOG_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_ICON_CHOOSER_DIALOG))
#define EGG_ICON_CHOOSER_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_ICON_CHOOSER_DIALOG, EggIconChooserDialogClass))


typedef struct _EggIconChooserDialog EggIconChooserDialog;
typedef struct _EggIconChooserDialogPrivate EggIconChooserDialogPrivate;
typedef struct _EggIconChooserDialogClass EggIconChooserDialogClass;

struct _EggIconChooserDialog
{
  GtkDialog parent;

  EggIconChooserDialogPrivate *priv;
};

struct _EggIconChooserDialogClass
{
  GtkDialogClass parent_class;

  void (*__egg_reserved1);
  void (*__egg_reserved2);
  void (*__egg_reserved3);
  void (*__egg_reserved4);
  void (*__egg_reserved5);
  void (*__egg_reserved6);
  void (*__egg_reserved7);
  void (*__egg_reserved8);
};

GType      egg_icon_chooser_dialog_get_type         (void) G_GNUC_CONST;

GtkWidget *egg_icon_chooser_dialog_new              (const gchar *title,
						     GtkWindow   *parent,
						     const gchar *first_button_text,
						     ...);
GtkWidget *egg_icon_chooser_dialog_new_with_backend (const gchar *title,
						     GtkWindow   *parent,
						     const gchar *filesystem_backend,
						     const gchar *first_button_text,
						     ...);

G_END_DECLS

#endif /* !__EGG_ICON_CHOOSER_DIALOG_H__ */
