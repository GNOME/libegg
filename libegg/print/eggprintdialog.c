/* GTK - The GIMP Toolkit
 * eggprintdialog.c: Print Dialog
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

#include "eggprintdialog.h"
#include <gtk/gtkbox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkstock.h>

#define EGG_PRINT_DIALOG_GET_PRIVATE(obj)(G_TYPE_INSTANCE_GET_PRIVATE ((obj), EGG_TYPE_PRINT_DIALOG, EggPrintDialogPrivate))

struct _EggPrintDialogPrivate
{
  EggPrintSettings *settings;
  GtkWidget *dialog;
  GtkWidget *entry;
};

G_DEFINE_TYPE (EggPrintDialog, egg_print_dialog, G_TYPE_OBJECT)

static void
egg_print_dialog_finalize (GObject *object)
{
  EggPrintDialog *print_dialog = EGG_PRINT_DIALOG (object);

  g_object_unref (print_dialog->priv->settings);
  
  G_OBJECT_CLASS (egg_print_dialog_parent_class)->finalize (object);
}

static void
egg_print_dialog_class_init (EggPrintDialogClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_dialog_finalize;
  
  g_type_class_add_private (gobject_class, sizeof (EggPrintDialogPrivate));
}

static void
egg_print_dialog_init (EggPrintDialog *dialog)
{
  dialog->priv = EGG_PRINT_DIALOG_GET_PRIVATE (dialog);

  dialog->priv->dialog = gtk_dialog_new_with_buttons ("Print dialog",
						      NULL,
						      0,
						      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						      GTK_STOCK_PRINT,  GTK_RESPONSE_OK,
						      NULL);
  dialog->priv->entry = gtk_entry_new();

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog->priv->dialog)->vbox),
		      dialog->priv->entry, FALSE, FALSE, 0);
  gtk_widget_show_all (GTK_DIALOG (dialog->priv->dialog)->vbox);
  
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog->priv->dialog), FALSE);
}
  
EggPrintDialog *
egg_print_dialog_new (EggPrintSettings *settings)
{
  EggPrintDialog *print_dialog;
  gchar *output_filename;

  g_return_val_if_fail (EGG_IS_PRINT_SETTINGS (settings), NULL);

  print_dialog = g_object_new (EGG_TYPE_PRINT_DIALOG, NULL);

  print_dialog->priv->settings = g_object_ref (settings);

  g_object_get (settings,
		"output-filename", &output_filename,
		NULL);

  if (output_filename)
    gtk_entry_set_text (GTK_ENTRY (print_dialog->priv->entry), output_filename);

  return print_dialog;
}

gint
egg_print_dialog_run (EggPrintDialog *dialog)
{
  int response;
  
  g_return_val_if_fail (EGG_IS_PRINT_DIALOG (dialog), 0);
  
  response = gtk_dialog_run (GTK_DIALOG (dialog->priv->dialog));

  if (response == GTK_RESPONSE_OK)
    {
      g_object_set (dialog->priv->settings,
		    "output-filename", gtk_entry_get_text (GTK_ENTRY (dialog->priv->entry)),
		    NULL);
    }

  return response;
}

void
egg_print_dialog_run_async (EggPrintDialog *dialog)
{
  g_return_if_fail (EGG_IS_PRINT_DIALOG (dialog));

  gtk_widget_show (dialog->priv->dialog);
}

