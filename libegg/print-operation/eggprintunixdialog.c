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

#include "eggprintbackend.h"
#include "eggprintbackendcups.h"
#include "eggprintunixdialog.h"
#include "eggprintsettingwidget.h"

#define EXAMPLE_PAGE_AREA_SIZE 140

#define _(x) (x)

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

static void populate_dialog (EggPrintUnixDialog *dialog);

enum {
  PROP_0,
  EGG_PRINT_UNIX_DIALOG_PROP_PRINT_BACKEND
};

enum {
  LAST_SIGNAL
};

//static guint egg_print_unix_dialog_signals[LAST_SIGNAL] = { 0 };

enum {
  PRINTER_LIST_COL_ICON,
  PRINTER_LIST_COL_NAME,
  PRINTER_LIST_COL_STATE,
  PRINTER_LIST_COL_JOBS,
  PRINTER_LIST_COL_LOCATION,
  PRINTER_LIST_COL_PRINTER_OBJ,
  PRINTER_LIST_N_COLS
};

struct EggPrintUnixDialogPrivate
{
  GtkWidget *notebook;

  GtkWidget *printer_treeview;
  
  GtkTreeModel *printer_list;

  GtkWidget *all_pages_radio;
  GtkWidget *current_page_radio;
  GtkWidget *page_range_radio;
  GtkWidget *page_range_entry;
  
  GtkWidget *copies_spin;
  GtkWidget *collate_check;
  GtkWidget *reverse_check;
  EggPrintSettingWidget *pages_per_sheet;
  EggPrintSettingWidget *duplex;
  EggPrintSettingWidget *paper_size;
  EggPrintSettingWidget *paper_type;
  EggPrintSettingWidget *paper_source;
  EggPrintSettingWidget *output_tray;

  GtkWidget *advanced_vbox;
  
  EggPrintBackend *print_backend;
  
  EggPrintBackendSettingSet *settings;
};

G_DEFINE_TYPE (EggPrintUnixDialog, egg_print_unix_dialog, GTK_TYPE_DIALOG);

/* XPM */
static const char *collate_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #020202",
"@	c #FFFFFF",
"#	c #010101",
"$	c #070707",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"..+++++++++##++++++$@@@@@@@@@..   ..+++++++++##++++++$@@@@@@@@@..",
"..+++++++++##+++++#+@@@@@@@@@..   ..+++++++++##+++++#+@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@..@@@@..   ..@@@@@@@@@@@@@@@@++@@@..@@@@..",
"..@@@@@@@@@@@@@@@@++@@.@@.@@@..   ..@@@@@@@@@@@@@@@@++@@.@@.@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@.@@@..   ..@@@@@@@@@@@@@@@@++@@@@@.@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@.@@@@@..   ..@@@@@@@@@@@@@@@@++@@@.@@@@@..",
"..@@@@@@@@@@@@@@@@++@@.@@@@@@..   ..@@@@@@@@@@@@@@@@++@@.@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@....@@@..   ..@@@@@@@@@@@@@@@@++@@....@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@.@@@@.............   ..@@@@@@@@@@@.@@@@.............",
"..@@@@@@@@@@..@@@@.............   ..@@@@@@@@@@..@@@@.............",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@...@@@..              ..@@@@@@@@@@...@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"....................              ....................           ",
"....................              ....................           "};

/* XPM */
static const char *nocollate_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #020202",
"#	c #010101",
"$	c #070707",
"           ....................              ....................",
"           ....................              ....................",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"..@@@@@@@@@##@@@@@@$+++++++++..   ..@@@@@@@@@##@@@@@@$+++++++++..",
"..@@@@@@@@@##@@@@@#@+++++++++..   ..@@@@@@@@@##@@@@@#@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++..++++..",
"..++++++++++++++++@@+++..++++..   ..++++++++++++++++@@++.++.+++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++++.+++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++.+++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++.++++++..",
"..++++++++++++++++@@+++...+++..   ..++++++++++++++++@@++....+++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..+++++++++++.++++.............   ..++++++++++..++++.............",
"..++++++++++..++++.............   ..+++++++++.++.+++.............",
"..+++++++++++.++++..              ..++++++++++++.+++..           ",
"..+++++++++++.++++..              ..+++++++++++.++++..           ",
"..+++++++++++.++++..              ..++++++++++.+++++..           ",
"..+++++++++++.++++..              ..+++++++++.++++++..           ",
"..++++++++++...+++..              ..+++++++++....+++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"....................              ....................           ",
"....................              ....................           "};

/* XPM */
static const char *collate_reverse_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #020202",
"@	c #FFFFFF",
"#	c #010101",
"$	c #070707",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"..+++++++++##++++++$@@@@@@@@@..   ..+++++++++##++++++$@@@@@@@@@..",
"..+++++++++##+++++#+@@@@@@@@@..   ..+++++++++##+++++#+@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@..@@@@..   ..@@@@@@@@@@@@@@@@++@@@..@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@...@@@..   ..@@@@@@@@@@@@@@@@++@@@...@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@..@@@@.............   ..@@@@@@@@@@..@@@@.............",
"..@@@@@@@@@.@@.@@@.............   ..@@@@@@@@@.@@.@@@.............",
"..@@@@@@@@@@@@.@@@..              ..@@@@@@@@@@@@.@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@.@@@@@..              ..@@@@@@@@@@.@@@@@..           ",
"..@@@@@@@@@.@@@@@@..              ..@@@@@@@@@.@@@@@@..           ",
"..@@@@@@@@@....@@@..              ..@@@@@@@@@....@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"....................              ....................           ",
"....................              ....................           "};

/* XPM */
static const char *nocollate_reverse_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #020202",
"#	c #010101",
"$	c #070707",
"           ....................              ....................",
"           ....................              ....................",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"..@@@@@@@@@##@@@@@@$+++++++++..   ..@@@@@@@@@##@@@@@@$+++++++++..",
"..@@@@@@@@@##@@@@@#@+++++++++..   ..@@@@@@@@@##@@@@@#@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++..++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++.++.+++..   ..++++++++++++++++@@+++..++++..",
"..++++++++++++++++@@+++++.+++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@+++.+++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++.++++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++....+++..   ..++++++++++++++++@@+++...+++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++..++++.............   ..+++++++++++.++++.............",
"..+++++++++.++.+++.............   ..++++++++++..++++.............",
"..++++++++++++.+++..              ..+++++++++++.++++..           ",
"..+++++++++++.++++..              ..+++++++++++.++++..           ",
"..++++++++++.+++++..              ..+++++++++++.++++..           ",
"..+++++++++.++++++..              ..+++++++++++.++++..           ",
"..+++++++++....+++..              ..++++++++++...+++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"....................              ....................           ",
"....................              ....................           "};


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
  populate_dialog (dialog);
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
                   EggPrinter *printer, 
		   EggPrintUnixDialog *impl)
{
  GtkTreeIter iter;
  GtkTreeSelection *selection;

  gtk_list_store_append (GTK_LIST_STORE (impl->priv->printer_list), &iter);

  gtk_list_store_set (GTK_LIST_STORE (impl->priv->printer_list), &iter,
                      PRINTER_LIST_COL_ICON, NULL,
                      PRINTER_LIST_COL_NAME, egg_printer_get_name (printer),
                      PRINTER_LIST_COL_STATE, egg_printer_get_state_message (printer),
                      PRINTER_LIST_COL_JOBS, egg_printer_get_job_count (printer),
                      PRINTER_LIST_COL_LOCATION, egg_printer_get_location (printer),
                      PRINTER_LIST_COL_PRINTER_OBJ, printer,
                      -1);

  /* If this is the first printer, select it */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (impl->priv->printer_treeview));
  if (!gtk_tree_selection_get_selected (selection, NULL, NULL))
    gtk_tree_selection_select_iter (selection, &iter);
}

static void
_printer_removed_cb (EggPrintBackend *backend, 
                   EggPrinter *printer, 
		   EggPrintUnixDialog *impl)
{
}


static void
_printer_status_cb (EggPrintBackend *backend, 
                   EggPrinter *printer, 
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
_create_printer_list_model (EggPrintUnixDialog *dialog)
{
  GtkListStore *model;

  model = gtk_list_store_new (PRINTER_LIST_N_COLS,
                              GDK_TYPE_PIXBUF,
                              G_TYPE_STRING, 
                              G_TYPE_STRING, 
                              G_TYPE_INT, 
                              G_TYPE_STRING, 
                              G_TYPE_OBJECT);

  dialog->priv->printer_list = (GtkTreeModel *)model;
}

static GtkWidget *
combo_box_new_option (void)
{
  GtkWidget *combo_box;
  GtkCellRenderer *cell;
  GtkListStore *store;

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  combo_box = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell,
                                  "text", 0,
                                  NULL);

  return combo_box;
}

static void
option_combo_box_clear (GtkComboBox *combo)
{
  GtkTreeModel *model;
  GtkListStore *store;
  
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
  store = GTK_LIST_STORE (model);

  gtk_list_store_clear (store);
}
  
static void
option_combo_box_append (GtkWidget *combo,
			 const char *display_text,
			 const char *value)
{
  GtkTreeModel *model;
  GtkListStore *store;
  GtkTreeIter iter;
  
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
  store = GTK_LIST_STORE (model);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      0, display_text,
		      1, value,
		      -1);
}

struct ComboSet {
  GtkComboBox *combo;
  const char *value;
};

static gboolean
set_cb (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  struct ComboSet *set_data = data;
  gboolean found;
  char *value;
  
  gtk_tree_model_get (model, iter, 1, &value, -1);
  found = (strcmp (value, set_data->value) == 0);
  g_free (value);
  
  if (found)
    gtk_combo_box_set_active_iter (set_data->combo, iter);

  return found;
}

static void
option_combo_box_set (GtkWidget *combo,
		      const char *value)
{
  GtkTreeModel *model;
  GtkListStore *store;
  struct ComboSet set_data;
  
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
  store = GTK_LIST_STORE (model);

  set_data.combo = GTK_COMBO_BOX (combo);
  set_data.value = value;
  gtk_tree_model_foreach (model, set_cb, &set_data);
}


static GtkWidget *
wrap_in_frame (const char *label, GtkWidget *child)
{
  GtkWidget *frame, *alignment;
  
  frame = gtk_frame_new (label);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  
  alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment),
			     0, 0, 12, 0);
  gtk_container_add (GTK_CONTAINER (frame), alignment);

  gtk_container_add (GTK_CONTAINER (alignment), child);

  gtk_widget_show (frame);
  gtk_widget_show (alignment);
  
  return frame;
}

static void
foreach_setting_cb (EggPrintBackendSetting  *setting,
		    gpointer                 user_data)
{
  EggPrintUnixDialog *dialog = user_data;
  GtkWidget *widget, *hbox, *label;

  if (g_str_has_prefix (setting->name, "gtk-"))
    return;

  widget = egg_print_setting_widget_new (setting);
  gtk_widget_show (widget);

  if (egg_print_setting_widget_has_external_label (EGG_PRINT_SETTING_WIDGET (widget)))
    {
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_widget_show (hbox);
      gtk_box_pack_start (GTK_BOX (dialog->priv->advanced_vbox),
			  hbox, FALSE, FALSE, 0);

      label = egg_print_setting_widget_get_external_label (EGG_PRINT_SETTING_WIDGET (widget));
      gtk_widget_show (label);
      gtk_box_pack_start (GTK_BOX (hbox),
			  label, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (hbox),
			  widget, FALSE, FALSE, 0);
      
    }
  else
    gtk_box_pack_start (GTK_BOX (dialog->priv->advanced_vbox),
			widget, FALSE, FALSE, 0);

}


static void
setup_setting (EggPrintUnixDialog *dialog,
	       const char *setting_name,
	       EggPrintSettingWidget *widget)
{
  EggPrintBackendSetting *setting;
  
  setting = egg_print_backend_setting_set_lookup (dialog->priv->settings, setting_name);
  egg_print_setting_widget_set_source (widget, setting);
}

static void
update_dialog_from_settings (EggPrintUnixDialog *dialog)
{
  setup_setting (dialog, "gtk-n-up", dialog->priv->pages_per_sheet);
  setup_setting (dialog, "gtk-duplex", dialog->priv->duplex);
  setup_setting (dialog, "gtk-paper-size", dialog->priv->paper_size);
  setup_setting (dialog, "gtk-paper-type", dialog->priv->paper_type);
  setup_setting (dialog, "gtk-paper-source", dialog->priv->paper_source);
  setup_setting (dialog, "gtk-output-tray", dialog->priv->output_tray);

  egg_print_backend_setting_set_foreach (dialog->priv->settings,
					 foreach_setting_cb, dialog);
}

static void
selected_printer_changed (GtkTreeSelection *selection, EggPrintUnixDialog *dialog)
{
  EggPrinter *printer;
  EggPrintBackend *backend;
  GtkTreeIter iter;

  if (dialog->priv->settings)
    g_object_unref (dialog->priv->settings);

  dialog->priv->settings = NULL;  

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gtk_tree_model_get (dialog->priv->printer_list, &iter,
			  PRINTER_LIST_COL_PRINTER_OBJ, &printer,
			  -1);

      g_print ("selected new printer %p\n", printer);
  
      backend = egg_printer_get_backend (printer);

      /* TODO: Shouldn't the call was on the printer, not the backend */
      dialog->priv->settings =
	egg_print_backend_create_settings (backend, printer);
      
      g_object_unref (backend);
      g_object_unref (printer);
    }
  
  update_dialog_from_settings (dialog);
}

static void
create_main_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label, *hbox;
  GtkWidget *scrolled, *treeview, *frame, *table;
  GtkWidget *entry, *spinbutton;
  GtkWidget *radio, *check, *image;
  GdkPixbuf *pixbuf;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeSelection *selection;
  
  priv = dialog->priv;

  main_vbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (main_vbox);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
				       GTK_SHADOW_IN);
  gtk_widget_show (scrolled);
  gtk_box_pack_start (GTK_BOX (main_vbox), scrolled, TRUE, TRUE, 6);

  treeview = gtk_tree_view_new_with_model (priv->printer_list);
  priv->printer_treeview = treeview;
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), TRUE);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  g_signal_connect (selection, "changed", G_CALLBACK (selected_printer_changed), dialog);
  
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Printer"),
						     renderer,
						     "text",
						     PRINTER_LIST_COL_NAME,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Status"),
						     renderer,
						     "text",
						     PRINTER_LIST_COL_STATE,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);
  
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Location"),
						     renderer,
						     "text",
						     PRINTER_LIST_COL_LOCATION,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  gtk_widget_show (treeview);
  gtk_container_add (GTK_CONTAINER (scrolled), treeview);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 6);

  table = gtk_table_new (3, 2, FALSE);
  frame = wrap_in_frame (_("Print Pages"), table);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 6);
  gtk_widget_show (table);

  radio = gtk_radio_button_new_with_label (NULL, _("All"));
  priv->all_pages_radio = radio;
  gtk_widget_show (radio);
  gtk_table_attach (GTK_TABLE (table), radio,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);
  radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)),
					   _("Current"));
  priv->current_page_radio = radio;
  gtk_widget_show (radio);
  gtk_table_attach (GTK_TABLE (table), radio,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);
  radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)),
					   _("Range: "));
  priv->page_range_radio = radio;
  gtk_widget_show (radio);
  gtk_table_attach (GTK_TABLE (table), radio,
		    0, 1, 2, 3,  GTK_FILL, 0,
		    0, 0);
  entry = gtk_entry_new ();
  priv->page_range_entry = entry;
  gtk_widget_show (entry);
  gtk_table_attach (GTK_TABLE (table), entry,
		    1, 2, 2, 3,  GTK_FILL, 0,
		    0, 0);

  table = gtk_table_new (3, 2, FALSE);
  frame = wrap_in_frame (_("Copies"), table);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 6);
  gtk_widget_show (table);

  label = gtk_label_new (_("Copies:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);
  spinbutton = gtk_spin_button_new_with_range (1.0, 100.0, 1.0);
  priv->copies_spin = spinbutton;
  gtk_widget_show (spinbutton);
  gtk_table_attach (GTK_TABLE (table), spinbutton,
		    1, 2, 0, 1,  GTK_FILL, 0,
		    0, 0);

  check = gtk_check_button_new_with_mnemonic (_("_Collate"));
  priv->collate_check = check;
  gtk_widget_show (check);
  gtk_table_attach (GTK_TABLE (table), check,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);
  check = gtk_check_button_new_with_mnemonic (_("_Reverse"));
  priv->reverse_check = check;
  gtk_widget_show (check);
  gtk_table_attach (GTK_TABLE (table), check,
		    0, 1, 2, 3,  GTK_FILL, 0,
		    0, 0);

  image = gtk_image_new ();
  gtk_widget_show (image);
  gtk_table_attach (GTK_TABLE (table), image,
		    1, 2, 1, 3, GTK_FILL, 0,
		    0, 0);

  pixbuf = gdk_pixbuf_new_from_xpm_data (collate_xpm);
  gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
  g_object_unref (pixbuf);
  
  label = gtk_label_new (_("General"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    main_vbox, label);
  
}

static gboolean
draw_page_cb (GtkWidget	     *widget,
	      GdkEventExpose      *event,
	      EggPrintUnixDialog *dialog)
{
  cairo_t *cr;
  double ratio;
  int w, h, shadow_offset;
  
  
  cr = gdk_cairo_create (widget->window);

  ratio = 1.4142;

  w = (EXAMPLE_PAGE_AREA_SIZE - 3) / ratio;
  h = w * ratio;

  shadow_offset = 3;
  
  cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 0.5);
  cairo_rectangle (cr, shadow_offset + 1, shadow_offset + 1, w, h);
  cairo_fill (cr);
  
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_rectangle (cr, 1, 1, w, h);
  cairo_fill (cr);
  cairo_set_line_width (cr, 1.0);
  cairo_rectangle (cr, 0.5, 0.5, w+1, h+1);
  
  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
  cairo_stroke (cr);

  return TRUE;
}

static void
create_page_setup_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label, *hbox, *hbox2;
  GtkWidget *frame, *table, *widget;
  GtkWidget *combo, *spinbutton, *draw;
  
  priv = dialog->priv;

  main_vbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (main_vbox);

  hbox = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox, TRUE, TRUE, 6);

  table = gtk_table_new (5, 2, FALSE);
  frame = wrap_in_frame (_("Layout"), table);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 6);
  gtk_widget_show (table);

  label = gtk_label_new (_("Pages per sheet:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);

  widget = egg_print_setting_widget_new (NULL);
  priv->pages_per_sheet = EGG_PRINT_SETTING_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 0, 1,  GTK_FILL, 0,
		    0, 0);

  label = gtk_label_new (_("Two-sided:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);

  widget = egg_print_setting_widget_new (NULL);
  priv->duplex = EGG_PRINT_SETTING_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 1, 2,  GTK_FILL, 0,
		    0, 0);

  label = gtk_label_new (_("Scale:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 2, 3,  GTK_FILL, 0,
		    0, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_table_attach (GTK_TABLE (table), hbox2,
		    1, 2, 2, 3,  GTK_FILL, 0,
		    0, 0);
  
  spinbutton = gtk_spin_button_new_with_range (1.0, 1000.0, 1.0);
  gtk_spin_button_set_digits (GTK_SPIN_BUTTON (spinbutton), 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), 100.0);
  gtk_widget_show (spinbutton);
  gtk_box_pack_start (GTK_BOX (hbox2), spinbutton, FALSE, FALSE, 0);
  label = gtk_label_new ("%");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox2), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Orientation:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 3, 4,  GTK_FILL, 0,
		    0, 0);

  combo = gtk_combo_box_new_text ();
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 3, 4,  GTK_FILL, 0,
		    0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Portrait"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Landscape"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Reverse Portrait"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Reverse Landscape"));  
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  label = gtk_label_new (_("Only Print:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 4, 5,  GTK_FILL, 0,
		    0, 0);

  combo = gtk_combo_box_new_text ();
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 4, 5,  GTK_FILL, 0,
		    0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("All pages"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Even pages"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Odd pages"));  
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  table = gtk_table_new (4, 2, FALSE);
  frame = wrap_in_frame (_("Paper"), table);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 6);
  gtk_widget_show (table);

  label = gtk_label_new (_("Paper size:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);

  widget = egg_print_setting_widget_new (NULL);
  priv->paper_size = EGG_PRINT_SETTING_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 0, 1,  GTK_FILL, 0,
		    0, 0);

  label = gtk_label_new (_("Paper Type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);

  widget = egg_print_setting_widget_new (NULL);
  priv->paper_type = EGG_PRINT_SETTING_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 1, 2,  GTK_FILL, 0,
		    0, 0);

  label = gtk_label_new (_("Paper Source:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 2, 3,  GTK_FILL, 0,
		    0, 0);

  widget = egg_print_setting_widget_new (NULL);
  priv->paper_source = EGG_PRINT_SETTING_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 2, 3,  GTK_FILL, 0,
		    0, 0);

  label = gtk_label_new (_("Output Tray:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 3, 4,  GTK_FILL, 0,
		    0, 0);

  widget = egg_print_setting_widget_new (NULL);
  priv->output_tray = EGG_PRINT_SETTING_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 3, 4,  GTK_FILL, 0,
		    0, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox2, TRUE, TRUE, 6);

  draw = gtk_drawing_area_new ();
  gtk_widget_set_size_request (draw, 200, 200);
  g_signal_connect (draw, "expose_event", G_CALLBACK (draw_page_cb), dialog);
  gtk_widget_show (draw);

  gtk_box_pack_start (GTK_BOX (hbox2), draw, TRUE, FALSE, 6);
  
  label = gtk_label_new (_("Page Setup"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    main_vbox, label);
  
}

static void
create_job_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_table, *label;
  GtkWidget *frame, *table, *radio;
  GtkWidget *combo, *entry;
  
  priv = dialog->priv;

  main_table = gtk_table_new (2, 2, FALSE);
  gtk_widget_show (main_table);


  table = gtk_table_new (2, 2, FALSE);
  frame = wrap_in_frame (_("Job Details"), table);
  gtk_table_attach (GTK_TABLE (main_table), frame,
		    0, 1, 0, 1,  0, 0,
		    0, 0);
  gtk_widget_show (table);

  label = gtk_label_new (_("Priority:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);

  combo = gtk_combo_box_new_text ();
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 0, 1,  GTK_FILL, 0,
		    0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Urgent"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("High"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Medium"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Low"));  
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);

  label = gtk_label_new (_("Billing info:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_table_attach (GTK_TABLE (table), entry,
		    1, 2, 1, 2,  GTK_FILL, 0,
		    0, 0);


  table = gtk_table_new (2, 2, FALSE);
  frame = wrap_in_frame (_("Print Document"), table);
  gtk_table_attach (GTK_TABLE (main_table), frame,
		    0, 1, 1, 2,  0, 0,
		    0, 0);
  gtk_widget_show (table);

  radio = gtk_radio_button_new_with_label (NULL, _("Now"));
  gtk_widget_show (radio);
  gtk_table_attach (GTK_TABLE (table), radio,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);
  radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)),
					   _("At:"));
  gtk_widget_show (radio);
  gtk_table_attach (GTK_TABLE (table), radio,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);
  radio = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON (radio)),
					   _("On Hold"));
  gtk_widget_show (radio);
  gtk_table_attach (GTK_TABLE (table), radio,
		    0, 1, 2, 3,  GTK_FILL, 0,
		    0, 0);
  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_table_attach (GTK_TABLE (table), entry,
		    1, 2, 1, 2,  GTK_FILL, 0,
		    0, 0);
 

  table = gtk_table_new (2, 2, FALSE);
  frame = wrap_in_frame (_("Add Cover Page"), table);
  gtk_table_attach (GTK_TABLE (main_table), frame,
		    1, 2, 0, 1,  0, 0,
		    0, 0);
  gtk_widget_show (table);

  label = gtk_label_new (_("Before:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 0, 1,  GTK_FILL, 0,
		    0, 0);

  combo = gtk_combo_box_new_text ();
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 0, 1,  GTK_FILL, 0,
		    0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("None"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Standard"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Confidential"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Secret"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Classified"));  
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  label = gtk_label_new (_("After:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show (label);
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);

  combo = gtk_combo_box_new_text ();
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 1, 2,  GTK_FILL, 0,
		    0, 0);
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("None"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Standard"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Confidential"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Secret"));  
  gtk_combo_box_append_text (GTK_COMBO_BOX (combo), _("Classified"));  
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);


  label = gtk_label_new (_("Job"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    main_table, label);
}

static void
create_image_quality_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label;
  
  priv = dialog->priv;

  main_vbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (main_vbox);

  label = gtk_label_new ("Autogenerated image quality settings go here");
  gtk_widget_show (label);

  gtk_box_pack_start (GTK_BOX (main_vbox), label, TRUE, TRUE, 6);
  
  label = gtk_label_new (_("Image Quality"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    main_vbox, label);
}

static void
create_finishing_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label;
  
  priv = dialog->priv;

  main_vbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (main_vbox);

  label = gtk_label_new ("Autogenerated finishing settings go here");
  gtk_widget_show (label);

  gtk_box_pack_start (GTK_BOX (main_vbox), label, TRUE, TRUE, 6);
  
  label = gtk_label_new (_("Finishing"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    main_vbox, label);
}

static void
create_advanced_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label, *scrolled;
  
  priv = dialog->priv;

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_AUTOMATIC);
  gtk_widget_show (scrolled);

  main_vbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (main_vbox);
  

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					 main_vbox);
  
  label = gtk_label_new ("Autogenerated settings that are not on other pages go here");
  gtk_widget_show (label);

  gtk_box_pack_start (GTK_BOX (main_vbox), label, TRUE, TRUE, 6);

  dialog->priv->advanced_vbox = main_vbox;
  
  label = gtk_label_new (_("Advanced"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    scrolled, label);
}


static void
populate_dialog (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  
  g_return_if_fail (EGG_IS_PRINT_UNIX_DIALOG (dialog));
  
  priv = dialog->priv;
 
  _create_printer_list_model (dialog);

  priv->notebook = gtk_notebook_new ();

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), 
                      priv->notebook,
                      TRUE, TRUE, 10);

  create_main_page (dialog);
  create_page_setup_page (dialog);
  create_job_page (dialog);
  create_image_quality_page (dialog);
  create_finishing_page (dialog);
  create_advanced_page (dialog);
  
  gtk_widget_show (priv->notebook);
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
 * Since: 2.8
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

EggPrinter *
egg_print_unix_dialog_get_selected_printer (EggPrintUnixDialog *dialog)
{
  EggPrinter *printer;
  GtkTreeIter iter;
  GtkTreeSelection *selection;
  printer = NULL;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->printer_treeview));

  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    {
      gtk_tree_model_get (dialog->priv->printer_list,
                          &iter,
                          PRINTER_LIST_COL_PRINTER_OBJ, &printer,
                          -1); 
    }

  return printer; 
}
