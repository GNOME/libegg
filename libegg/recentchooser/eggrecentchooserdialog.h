/* GTK - The GIMP Toolkit
 * eggrecentchooserdialog.h: Recent files selector dialog
 * Copyright (C) 2005 Emmanuele Bassi
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

#ifndef __EGG_RECENT_CHOOSER_DIALOG_H__
#define __EGG_RECENT_CHOOSER_DIALOG_H__

#include <gtk/gtkdialog.h>
#include "eggrecentchooser.h"

G_BEGIN_DECLS

#define EGG_TYPE_RECENT_CHOOSER_DIALOG		  (egg_recent_chooser_dialog_get_type ())
#define EGG_RECENT_CHOOSER_DIALOG(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_CHOOSER_DIALOG, EggRecentChooserDialog))
#define EGG_IS_RECENT_CHOOSER_DIALOG(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_CHOOSER_DIALOG))
#define EGG_RECENT_CHOOSER_DIALOG_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_RECENT_CHOOSER_DIALOG, EggRecentChooserDialogClass))
#define EGG_IS_RECENT_CHOOSER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_RECENT_CHOOSER_DIALOG))
#define EGG_RECENT_CHOOSER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_RECENT_CHOOSER_DIALOG, EggRecentChooserDialogClass))

typedef struct _EggRecentChooserDialog        EggRecentChooserDialog;
typedef struct _EggRecentChooserDialogClass   EggRecentChooserDialogClass;

typedef struct _EggRecentChooserDialogPrivate EggRecentChooserDialogPrivate;


struct _EggRecentChooserDialog
{
  /*< private >*/
  GtkDialog parent_instance;
  
  EggRecentChooserDialogPrivate *priv;
};

struct _EggRecentChooserDialogClass
{
  GtkDialogClass parent_class;
};


GType      egg_recent_chooser_dialog_get_type        (void) G_GNUC_CONST;

GtkWidget *egg_recent_chooser_dialog_new             (const gchar      *title,
					              GtkWindow        *parent,
					              const gchar      *first_button_text,
					              ...) G_GNUC_NULL_TERMINATED;
GtkWidget *egg_recent_chooser_dialog_new_for_manager (const gchar      *title,
						      GtkWindow        *parent,
						      EggRecentManager *manager,
						      const gchar      *first_button_text,
						      ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* __EGG_RECENT_CHOOSER_DIALOG_H__ */
