/* EggPrintUnixDialog
 * Copyright (C) 2006 John (J5) Palmieri  <johnp@redhat.com>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "config.h"
//#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "eggprintunixdialog.h"
#include "eggprintbackend.h"
#include "eggprintbackendcups.h"

#define EGG_PRINT_UNIX_DIALOG_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PRINT_UNIX_DIALOG, EggPrintUnixDialogPrivate))

static void egg_print_unix_dialog_finalize     (GObject *object);
static void egg_print_unix_dialog_set_property (GObject      *object,
			                   guint         prop_id,
			                   const GValue *value,
			                   GParamSpec   *pspec);
static void egg_print_unix_dialog_get_property (GObject      *object,
			                   guint         prop_id,
			                   GValue *value,
			                   GParamSpec   *pspec);

static void _populate_dialog (EggPrintUnixDialog *dialog);

enum {
  PROP_0,
  EGG_PRINT_UNIX_DIALOG_PROP_PRINT_BACKEND
};

enum {
  LAST_SIGNAL
};

//static guint egg_print_unix_dialog_signals[LAST_SIGNAL] = { 0 };

struct EggPrintUnixDialogPrivate
{
  GtkWidget *main_hbox;
  GtkWidget *label_vbox;
  GtkWidget *input_vbox;
  
  GtkWidget *printer_label;
  GtkWidget *printer_select;

  GtkWidget *settings_label;
  GtkWidget *settings_select;
  
  GtkWidget *copies_label;
  GtkWidget *copies_spinner_hbox;
  GtkWidget *copies_spinner;

  GtkWidget *pages_label;
  GtkWidget *pages_all_radio;
  GtkWidget *pages_from_hbox;
  GtkWidget *pages_from_radio;
  GtkWidget *pages_from_start_entry;
  GtkWidget *pages_from_to_label;
  GtkWidget *pages_from_end_entry;

  EggPrintBackend *print_backend;
};


G_DEFINE_TYPE (EggPrintUnixDialog, egg_print_unix_dialog, GTK_TYPE_DIALOG);

static void
egg_print_unix_dialog_class_init (EggPrintUnixDialogClass *class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;

  object_class->finalize = egg_print_unix_dialog_finalize;
  object_class->set_property = egg_print_unix_dialog_set_property;
  object_class->get_property = egg_print_unix_dialog_get_property;

  g_type_class_add_private (class, sizeof (EggPrintUnixDialogPrivate));  

  gtk_settings_install_property (g_param_spec_string ("gtk-print-backend",
						      "Default print backend",
						      "Name of the EggPrintUnixDialog backend to use by default",
						      NULL,
						      GTK_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   EGG_PRINT_UNIX_DIALOG_PROP_PRINT_BACKEND,
                                   g_param_spec_string ("print-backend",
						      "Print backend",
						      "The EggPrintUnixDialog backend to use",
						      NULL,
						      GTK_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));


}

static void
egg_print_unix_dialog_init (EggPrintUnixDialog *dialog)
{
  dialog->priv = EGG_PRINT_UNIX_DIALOG_GET_PRIVATE (dialog); 
  dialog->priv->print_backend = NULL;
  _populate_dialog (dialog);
}

static void
egg_print_unix_dialog_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  /* EggPrintUnixDialog *dialog = EGG_PRINT_UNIX_DIALOG (object); */

  if (G_OBJECT_CLASS (egg_print_unix_dialog_parent_class)->finalize)
    G_OBJECT_CLASS (egg_print_unix_dialog_parent_class)->finalize (object);
}

static void
_printer_added_cb (EggPrintBackend *backend, 
                   const gchar *printer_name, 
		   EggPrintUnixDialog *impl)
{
  /* TODO: use a model instead of just text */
  gtk_combo_box_append_text (GTK_COMBO_BOX (impl->priv->printer_select),
                             printer_name);
}

static void
_printer_removed_cb (EggPrintBackend *backend, 
                   const gchar *printer_name, 
		   EggPrintUnixDialog *impl)
{
}


static void
_printer_status_cb (EggPrintBackend *backend, 
                   const gchar *printer_name, 
		   EggPrintUnixDialog *impl)
{
}


static void
_printer_list_initialize (EggPrintUnixDialog *impl)
{
  /* TODO: allow for multiple backends */
  g_return_if_fail (impl->priv->print_backend != NULL);

  g_signal_connect (impl->priv->print_backend, 
                    "printer-added", 
		    (GCallback) _printer_added_cb, 
		    impl);

  g_signal_connect (impl->priv->print_backend, 
                    "printer-removed", 
		    (GCallback) _printer_removed_cb, 
		    impl);

  g_signal_connect (impl->priv->print_backend, 
                    "printer-status-changed", 
		    (GCallback) _printer_status_cb, 
		    impl);

}

static void
_set_print_backend (EggPrintUnixDialog *impl,
		   const char *backend)
{
  if (impl->priv->print_backend)
    {
      /*TODO: clean up signal handlers
      g_signal_handler_disconnect */
      g_object_unref (impl->priv->print_backend);
    }

  impl->priv->print_backend = NULL;

/* TODO: allow for dynamicly loaded backends */
#if 0
  if (backend)
    impl->priv->print_backend = _egg_print_backend_create (backend);
  else
    {
      GtkSettings *settings = gtk_settings_get_default ();
      gchar *default_backend = NULL;

      g_object_get (settings, "gtk-print-backend", &default_backend, NULL);
      if (default_backend)
	{
	  impl->priv->print_backend = _egg_print_backend_create (default_backend);
	  g_free (default_backend);
	}
    }
#endif

  if (!impl->priv->print_backend)
    {
#if defined (G_OS_UNIX)
      impl->priv->print_backend = egg_print_backend_cups_new ();
#else
#error "No default filesystem implementation on the platform"
#endif
    }

  if (impl->priv->print_backend)
    _printer_list_initialize (impl);

}

static void
egg_print_unix_dialog_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)

{
  EggPrintUnixDialog *impl = EGG_PRINT_UNIX_DIALOG (object);

  switch (prop_id)
    {
    case EGG_PRINT_UNIX_DIALOG_PROP_PRINT_BACKEND:
      _set_print_backend (impl, g_value_get_string (value));
      break;
    

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_print_unix_dialog_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
  /* EggPrintUnixDialog *impl = EGG_PRINT_UNIX_DIALOG (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
_populate_dialog (EggPrintUnixDialog *dialog)
{
  GtkSizeGroup *printer_group;
  GtkSizeGroup *settings_group;
  GtkSizeGroup *copies_group;
  GtkSizeGroup *pages_group;
  EggPrintUnixDialogPrivate *priv;
  
  g_return_if_fail (EGG_IS_PRINT_UNIX_DIALOG (dialog));
  
  priv = dialog->priv;
  
  printer_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
  settings_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
  copies_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);
  pages_group = gtk_size_group_new (GTK_SIZE_GROUP_VERTICAL);

  priv->main_hbox = gtk_hbox_new (FALSE, 5);
  priv->label_vbox = gtk_vbox_new (FALSE, 5);
  priv->input_vbox = gtk_vbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), 
                      priv->main_hbox,
                      TRUE, TRUE, 10);
  
  gtk_box_pack_start (GTK_BOX (priv->main_hbox), 
                               priv->label_vbox,
			       FALSE, FALSE, 0);
			       
  gtk_box_pack_start (GTK_BOX (priv->main_hbox), 
                      priv->input_vbox,
		      TRUE, TRUE, 0);
  
  priv->printer_label = gtk_label_new ("Send To:");
  /* TODO: make printer selection model */
  priv->printer_select = gtk_combo_box_new_text (); 
  gtk_size_group_add_widget (printer_group, priv->printer_label);
  gtk_size_group_add_widget (printer_group, priv->printer_select);
  gtk_box_pack_start (GTK_BOX (priv->label_vbox), 
                      priv->printer_label,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->input_vbox), 
                      priv->printer_select,
		      FALSE, FALSE, 0);

  priv->settings_label = gtk_label_new ("Settings:");
  priv->settings_select = gtk_combo_box_new_text ();
  
  gtk_size_group_add_widget (settings_group, priv->settings_label);
  gtk_size_group_add_widget (settings_group, priv->settings_select);
  gtk_box_pack_start (GTK_BOX (priv->label_vbox), 
                      priv->settings_label,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->input_vbox), 
                      priv->settings_select,
		      FALSE, FALSE, 0);
  
  
  priv->copies_label = gtk_label_new ("Copies:");
  priv->copies_spinner = gtk_spin_button_new_with_range (1, 9999, 1);
  priv->copies_spinner_hbox = gtk_hbox_new (FALSE, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (priv->copies_spinner), 4);
  gtk_size_group_add_widget (copies_group, priv->copies_label);
  gtk_size_group_add_widget (copies_group, priv->copies_spinner_hbox);
  gtk_box_pack_start (GTK_BOX (priv->label_vbox), 
                      priv->copies_label,
                      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->input_vbox), 
                      priv->copies_spinner_hbox,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->copies_spinner_hbox), 
                      priv->copies_spinner,
		      FALSE, FALSE, 0);

  priv->pages_label = gtk_label_new ("Pages:");
  priv->pages_all_radio = gtk_radio_button_new_with_label (NULL, "All");
  priv->pages_from_hbox = gtk_hbox_new (FALSE, 5);
  priv->pages_from_radio = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (priv->pages_all_radio), 
                                                                        "From:");
  priv->pages_from_start_entry = gtk_entry_new ();
  priv->pages_from_to_label = gtk_label_new ("to:");
  priv->pages_from_end_entry = gtk_entry_new ();
  gtk_entry_set_width_chars (GTK_ENTRY (priv->pages_from_start_entry), 3);
  gtk_entry_set_width_chars (GTK_ENTRY (priv->pages_from_end_entry), 3);

  gtk_box_pack_start (GTK_BOX (priv->pages_from_hbox), 
                      priv->pages_from_radio,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->pages_from_hbox), 
                      priv->pages_from_start_entry,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->pages_from_hbox), 
                      priv->pages_from_to_label,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->pages_from_hbox), 
                      priv->pages_from_end_entry,
		      FALSE, FALSE, 0);
  gtk_size_group_add_widget (pages_group, priv->pages_label);
  gtk_size_group_add_widget (pages_group, priv->pages_all_radio);
  gtk_box_pack_start (GTK_BOX (priv->label_vbox), 
                      priv->pages_label,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->input_vbox), 
                      priv->pages_all_radio,
		      FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (priv->input_vbox), 
                      priv->pages_from_hbox,
		      FALSE, FALSE, 0);

  gtk_widget_show_all (GTK_DIALOG (dialog)->vbox);

  g_object_unref (printer_group);
  g_object_unref (settings_group);
  g_object_unref (copies_group);
  g_object_unref (pages_group);
}


/**
 * egg_print_unix_dialog_new:
 * @title: Title of the dialog, or %NULL
 * @parent: Transient parent of the dialog, or %NULL
 *
 * Creates a new #EggPrintUnixDialog.
 *
 * Return value: a new #EggPrintUnixDialog
 *
 * Since: 2.6
 **/
GtkWidget *
egg_print_unix_dialog_new (const gchar *title,
                      GtkWindow *parent,
		      const gchar *print_backend)
{
  GtkWidget *result;
  const gchar *_title = "Print";

  if (title)
    _title = title;
  
  result = g_object_new (EGG_TYPE_PRINT_UNIX_DIALOG,
                         "title", _title,
			 "has-separator", FALSE,
			 "print-backend", print_backend,
                         NULL);

  if (parent)
    gtk_window_set_transient_for (GTK_WINDOW (result), parent);

  gtk_dialog_add_buttons (GTK_DIALOG (result), 
                          GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                          GTK_STOCK_PRINT, GTK_RESPONSE_ACCEPT,
                          NULL);

  return result;
  
}

