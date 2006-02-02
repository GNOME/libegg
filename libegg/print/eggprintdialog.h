/* GTK - The GIMP Toolkit
 * eggprintdialog.h: Print Dialog
 * Copyright (C) 2005, Red Hat, Inc.
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

#ifndef __EGG_PRINT_DIALOG_H__
#define __EGG_PRINT_DIALOG_H__

#include <gtk/gtkdialog.h>

#include <eggprintsettings.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_DIALOG            (egg_print_dialog_get_type ())
#define EGG_PRINT_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_DIALOG, EggPrintDialog))
#define EGG_PRINT_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_DIALOG, EggPrintDialogClass))
#define EGG_IS_PRINT_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_DIALOG))
#define EGG_IS_PRINT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_DIALOG))
#define EGG_PRINT_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_DIALOG, EggPrintDialogClass))

typedef struct _EggPrintDialogClass   EggPrintDialogClass;
typedef struct _EggPrintDialogPrivate EggPrintDialogPrivate;
typedef struct _EggPrintDialog        EggPrintDialog;

struct _EggPrintDialog
{
  GObject parent_instance;
  
  EggPrintDialogPrivate *priv;
};

struct _EggPrintDialogClass
{
  GObjectClass parent_class;
  
  void (*result)          (EggPrintDialog *dialog,
			   gint            response_id);
};

GType egg_print_dialog_get_type (void);

EggPrintDialog *egg_print_dialog_new (EggPrintSettings *settings);

gint egg_print_dialog_run       (EggPrintDialog *dialog);
void egg_print_dialog_run_async (EggPrintDialog *dialog);

EggPrintDialog *egg_print_dialog_get_settings (EggPrintDialog *dialog);


G_END_DECLS

#endif /* __EGG_PRINT_DIALOG_H__ */
