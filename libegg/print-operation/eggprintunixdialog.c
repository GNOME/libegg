/* EggPrintUnixDialog
 * Copyright (C) 2006 John (J5) Palmieri  <johnp@redhat.com>
 * Copyright (C) 2006 Alexander Larsson <alexl@redhat.com>
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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "eggintl.h"
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "eggprintbackend.h"
#include "eggprintbackendcups.h"
#include "eggprintunixdialog.h"
#include "eggprinteroptionwidget.h"

#define EXAMPLE_PAGE_AREA_SIZE 140

#define EGG_PRINT_UNIX_DIALOG_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PRINT_UNIX_DIALOG, EggPrintUnixDialogPrivate))

static void egg_print_unix_dialog_finalize     (GObject            *object);
static void egg_print_unix_dialog_set_property (GObject            *object,
						guint               prop_id,
						const GValue       *value,
						GParamSpec         *pspec);
static void egg_print_unix_dialog_get_property (GObject            *object,
						guint               prop_id,
						GValue             *value,
						GParamSpec         *pspec);
static void populate_dialog                    (EggPrintUnixDialog *dialog);
static void unschedule_idle_mark_conflicts     (EggPrintUnixDialog *dialog);
static void selected_printer_changed           (GtkTreeSelection   *selection,
						EggPrintUnixDialog *dialog);

enum {
  PROP_0,
  PROP_PRINT_BACKEND
};

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
  GtkWidget *collate_image;
  GtkWidget *orientation_combo;
  GtkWidget *page_layout_preview;
  GtkWidget *scale_spin;
  GtkWidget *page_set_combo;
  EggPrinterOptionWidget *pages_per_sheet;
  EggPrinterOptionWidget *duplex;
  EggPrinterOptionWidget *paper_size;
  EggPrinterOptionWidget *paper_type;
  EggPrinterOptionWidget *paper_source;
  EggPrinterOptionWidget *output_tray;

  GtkWidget *conflicts_widget;

  GtkWidget *finishing_table;
  GtkWidget *finishing_page;
  GtkWidget *image_quality_table;
  GtkWidget *image_quality_page;
  GtkWidget *color_table;
  GtkWidget *color_page;

  GtkWidget *advanced_vbox;
  GtkWidget *advanced_page;
  
  EggPrintBackend *print_backend;
  
  EggPrinter *current_printer;
  guint request_details_tag;
  EggPrinterOptionSet *options;
  gulong options_changed_handler;
  gulong mark_conflicts_id;

  gint current_page;
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


/* This should be called by gtk_init(), but is called as-needed atm */
void
_egg_print_unix_init (void)
{
  static gboolean initialized = FALSE;

  if (initialized)
    return;

  gtk_settings_install_property (g_param_spec_string ("gtk-print-backend",
						      P_("Default print backend"),
						      P_("Name of the printbackend to use by default"),
						      NULL,
						      GTK_PARAM_READWRITE));
}

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

  _egg_print_unix_init ();

  g_object_class_install_property (object_class,
                                   PROP_PRINT_BACKEND,
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
  dialog->priv->current_page = -1;
  populate_dialog (dialog);
}

static void
egg_print_unix_dialog_finalize (GObject *object)
{
  EggPrintUnixDialog *dialog = EGG_PRINT_UNIX_DIALOG (object);
  
  g_return_if_fail (object != NULL);

  unschedule_idle_mark_conflicts (dialog);

  if (dialog->priv->request_details_tag)
    {
      g_source_remove (dialog->priv->request_details_tag);
      dialog->priv->request_details_tag = 0;
    }
  
  if (dialog->priv->current_printer)
    {
      g_object_unref (dialog->priv->current_printer);
      dialog->priv->current_printer = NULL;
    }

  if (dialog->priv->printer_list)
    {
      g_object_unref (dialog->priv->printer_list);
      dialog->priv->printer_list = NULL;
    }
  
  if (dialog->priv->options)
    {
      g_object_unref (dialog->priv->options);
      dialog->priv->options = NULL;
    }
  
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
                      PRINTER_LIST_COL_ICON, egg_printer_get_icon_name (printer),
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
    case PROP_PRINT_BACKEND:
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
                              G_TYPE_STRING,
                              G_TYPE_STRING, 
                              G_TYPE_STRING, 
                              G_TYPE_INT, 
                              G_TYPE_STRING, 
                              G_TYPE_OBJECT);

  dialog->priv->printer_list = (GtkTreeModel *)model;
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
setup_option (EggPrintUnixDialog *dialog,
	      const char *option_name,
	      EggPrinterOptionWidget *widget)
{
  EggPrinterOption *option;

  option = egg_printer_option_set_lookup (dialog->priv->options, option_name);
  egg_printer_option_widget_set_source (widget, option);
}

static void
add_option_to_table (EggPrinterOption *option,
		     gpointer          user_data)
{
  GtkTable *table;
  GtkWidget *label, *widget;
  int row;

  table = GTK_TABLE (user_data);
  
  if (g_str_has_prefix (option->name, "gtk-"))
    return;
  
  widget = egg_printer_option_widget_new (option);
  gtk_widget_show (widget);

  row = table->nrows;
  gtk_table_resize (table, table->nrows + 1, table->ncols + 1);
  
  if (egg_printer_option_widget_has_external_label (EGG_PRINTER_OPTION_WIDGET (widget)))
    {
      label = egg_printer_option_widget_get_external_label (EGG_PRINTER_OPTION_WIDGET (widget));
      gtk_widget_show (label);

      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
      
      gtk_table_attach (table, label,
			0, 1, row - 1 , row,  GTK_FILL, 0, 0, 0);
      
      gtk_table_attach (table, widget,
			1, 2, row - 1, row,  GTK_FILL, 0, 0, 0);
    }
  else
    gtk_table_attach (table, widget,
		      0, 2, row - 1, row,  GTK_FILL, 0, 0, 0);
}


static void
setup_page_table (EggPrinterOptionSet *options,
		  const char *group,
		  GtkWidget *table,
		  GtkWidget *page)
{
  egg_printer_option_set_foreach_in_group (options, group,
					   add_option_to_table,
					   table);
  if (GTK_TABLE (table)->nrows == 1)
    gtk_widget_hide (page);
  else
    gtk_widget_show (page);
}
	     
static void
update_dialog_from_settings (EggPrintUnixDialog *dialog)
{
  GList *groups, *l;
  char *group;
  GtkWidget *table, *frame;
  gboolean has_advanced;
  
  setup_option (dialog, "gtk-n-up", dialog->priv->pages_per_sheet);
  setup_option (dialog, "gtk-duplex", dialog->priv->duplex);
  setup_option (dialog, "gtk-paper-size", dialog->priv->paper_size);
  setup_option (dialog, "gtk-paper-type", dialog->priv->paper_type);
  setup_option (dialog, "gtk-paper-source", dialog->priv->paper_source);
  setup_option (dialog, "gtk-output-tray", dialog->priv->output_tray);

  setup_page_table (dialog->priv->options,
		    "ImageQualityPage",
		    dialog->priv->image_quality_table,
		    dialog->priv->image_quality_page);
  
  setup_page_table (dialog->priv->options,
		    "FinishingPage",
		    dialog->priv->finishing_table,
		    dialog->priv->finishing_page);

  setup_page_table (dialog->priv->options,
		    "ColorPage",
		    dialog->priv->color_table,
		    dialog->priv->color_page);

  /* Put the rest of the groups in the advanced page */
  groups = egg_printer_option_set_get_groups (dialog->priv->options);

  has_advanced = FALSE;
  for (l = groups; l != NULL; l = l->next)
    {
      group = l->data;

      if (group == NULL)
	continue;
      
      if (strcmp (group, "ImageQualityPage") == 0 ||
	  strcmp (group, "ColorPage") == 0 ||
	  strcmp (group, "FinishingPage") == 0)
	continue;

      table = gtk_table_new (1, 2, FALSE);
      
      egg_printer_option_set_foreach_in_group (dialog->priv->options,
					       group,
					       add_option_to_table,
					       table);
      if (GTK_TABLE (table)->nrows == 1)
	gtk_widget_destroy (table);
      else
	{
	  has_advanced = TRUE;
	  frame = wrap_in_frame (group, table);
	  gtk_widget_show (table);
	  gtk_widget_show (frame);
	  
	  gtk_box_pack_start (GTK_BOX (dialog->priv->advanced_vbox),
			      frame, FALSE, FALSE, 0);
	}
    }

  if (has_advanced)
    gtk_widget_show (dialog->priv->advanced_page);
  else
    gtk_widget_hide (dialog->priv->advanced_page);

  
  g_list_foreach (groups, (GFunc) g_free, NULL);
  g_list_free (groups);
}

static void
mark_conflicts (EggPrintUnixDialog *dialog)
{
  EggPrinter *printer;
  gboolean have_conflict;

  have_conflict = FALSE;

  printer = dialog->priv->current_printer;

  if (printer)
    {

      g_signal_handler_block (dialog->priv->options,
			      dialog->priv->options_changed_handler);
      
      egg_printer_option_set_clear_conflicts (dialog->priv->options);
      
      have_conflict = _egg_printer_mark_conflicts (printer,
						   dialog->priv->options);
      
      g_signal_handler_unblock (dialog->priv->options,
				dialog->priv->options_changed_handler);
    }

  if (have_conflict)
    gtk_widget_show (dialog->priv->conflicts_widget);
  else
    gtk_widget_hide (dialog->priv->conflicts_widget);
}

static gboolean
mark_conflicts_callback (gpointer data)
{
  EggPrintUnixDialog *dialog = data;

  dialog->priv->mark_conflicts_id = 0;

  mark_conflicts (dialog);

  return FALSE;
}

static void
unschedule_idle_mark_conflicts (EggPrintUnixDialog *dialog)
{
  if (dialog->priv->mark_conflicts_id != 0)
    {
      g_source_remove (dialog->priv->mark_conflicts_id);
      dialog->priv->mark_conflicts_id = 0;
    }
}

static void
schedule_idle_mark_conflicts (EggPrintUnixDialog *dialog)
{
  if (dialog->priv->mark_conflicts_id != 0)
    return;

  dialog->priv->mark_conflicts_id = g_idle_add (mark_conflicts_callback,
						dialog);
}

static void
clear_per_printer_ui (EggPrintUnixDialog *dialog)
{
  gtk_container_foreach (GTK_CONTAINER (dialog->priv->finishing_table),
			 (GtkCallback)gtk_widget_destroy,
			 NULL);
  gtk_table_resize (GTK_TABLE (dialog->priv->finishing_table), 1, 2);
  gtk_container_foreach (GTK_CONTAINER (dialog->priv->image_quality_table),
			 (GtkCallback)gtk_widget_destroy,
			 NULL);
  gtk_table_resize (GTK_TABLE (dialog->priv->image_quality_table), 1, 2);
  gtk_container_foreach (GTK_CONTAINER (dialog->priv->color_table),
			 (GtkCallback)gtk_widget_destroy,
			 NULL);
  gtk_table_resize (GTK_TABLE (dialog->priv->color_table), 1, 2);
  gtk_container_foreach (GTK_CONTAINER (dialog->priv->advanced_vbox),
			 (GtkCallback)gtk_widget_destroy,
			 NULL);
}

static void
printer_details_acquired (EggPrinter *printer,
			  gboolean success,
			  EggPrintUnixDialog *dialog)
{
  dialog->priv->request_details_tag = 0;
  
  if (success)
    {
      GtkTreeSelection *selection;
      selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (dialog->priv->printer_treeview));
      
      selected_printer_changed (selection, dialog);
    }
}

static void
selected_printer_changed (GtkTreeSelection *selection,
			  EggPrintUnixDialog *dialog)
{
  EggPrinter *printer;
  GtkTreeIter iter;

  if (dialog->priv->request_details_tag)
    {
      g_source_remove (dialog->priv->request_details_tag);
      dialog->priv->request_details_tag = 0;
    }
  
  printer = NULL;
  if (gtk_tree_selection_get_selected (selection, NULL, &iter))
    gtk_tree_model_get (dialog->priv->printer_list, &iter,
			PRINTER_LIST_COL_PRINTER_OBJ, &printer,
			-1);

  if (printer != NULL && !_egg_printer_has_details (printer))
    {
      dialog->priv->request_details_tag =
	g_signal_connect (printer, "details-acquired",
			  G_CALLBACK (printer_details_acquired), dialog);
      _egg_printer_request_details (printer);
      return;
    }
  
  if (printer == dialog->priv->current_printer)
    {
      if (printer)
	g_object_unref (printer);
      return;
    }

  if (dialog->priv->options)
    {
      g_object_unref (dialog->priv->options);
      dialog->priv->options = NULL;  

      clear_per_printer_ui (dialog);
    }

  if (dialog->priv->current_printer)
    {
      g_object_unref (dialog->priv->current_printer);
    }

  dialog->priv->current_printer = printer;

  dialog->priv->options = _egg_printer_get_options (printer);
  
  dialog->priv->options_changed_handler = 
    g_signal_connect_swapped (dialog->priv->options, "changed", G_CALLBACK (schedule_idle_mark_conflicts), dialog);
  
  update_dialog_from_settings (dialog);
}

static void
update_collate_icon (GtkToggleButton *toggle_button,
		     EggPrintUnixDialog *dialog)
{
  GdkPixbuf *pixbuf;
  gboolean collate, reverse;
  const char **xpm;

  collate = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->collate_check));
  reverse = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_check));

  if (collate)
    {
      if (reverse)
	xpm = collate_reverse_xpm;
      else
	xpm = collate_xpm;
    }
  else
    {
      if (reverse)
	xpm = nocollate_reverse_xpm;
      else
	xpm = nocollate_xpm;
    }
  
  pixbuf = gdk_pixbuf_new_from_xpm_data (xpm);
  gtk_image_set_from_pixbuf (GTK_IMAGE (dialog->priv->collate_image), pixbuf);
  g_object_unref (pixbuf);
}

static void
create_main_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label, *hbox;
  GtkWidget *scrolled, *treeview, *frame, *table;
  GtkWidget *entry, *spinbutton;
  GtkWidget *radio, *check, *image;
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
 
  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes (_(""),
						     renderer,
						     "icon-name",
						     PRINTER_LIST_COL_ICON,
						     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

 
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
  if (dialog->priv->current_page == -1)
    gtk_widget_set_sensitive (radio, FALSE);    
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
  g_signal_connect (check, "toggled", G_CALLBACK (update_collate_icon), dialog);
  gtk_widget_show (check);
  gtk_table_attach (GTK_TABLE (table), check,
		    0, 1, 1, 2,  GTK_FILL, 0,
		    0, 0);
  check = gtk_check_button_new_with_mnemonic (_("_Reverse"));
  g_signal_connect (check, "toggled", G_CALLBACK (update_collate_icon), dialog);
  priv->reverse_check = check;
  gtk_widget_show (check);
  gtk_table_attach (GTK_TABLE (table), check,
		    0, 1, 2, 3,  GTK_FILL, 0,
		    0, 0);

  image = gtk_image_new ();
  dialog->priv->collate_image = image;
  gtk_widget_show (image);
  gtk_table_attach (GTK_TABLE (table), image,
		    1, 2, 1, 3, GTK_FILL, 0,
		    0, 0);

  update_collate_icon (NULL, dialog);
  
  label = gtk_label_new (_("General"));
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    main_vbox, label);
  
}

static gboolean
is_range_separator (char c)
{
  return (c == ',' || c == ';' || c == ':');
}

static EggPageRange *
dialog_get_page_ranges (EggPrintUnixDialog *dialog,
			int *n_ranges_out)
{
  int i, n_ranges;
  const char *text, *p;
  char *next;
  EggPageRange *ranges;
  int start, end;
  
  text = gtk_entry_get_text (GTK_ENTRY (dialog->priv->page_range_entry));

  n_ranges = 1;
  p = text;
  while (*p)
    {
      if (is_range_separator (*p))
	n_ranges++;
      p++;
    }

  ranges = g_new0 (EggPageRange, n_ranges);
  
  i = 0;
  p = text;
  while (*p)
    {
      start = (int)strtol (p, &next, 10);
      if (start < 1)
	start = 1;
      end = start;

      if (next != p)
	{
	  p = next;

	  if (*p == '-')
	    {
	      p++;
	      end = (int)strtol (p, NULL, 10);
	      if (end < start)
		end = start;
	    }
	}

      ranges[i].start = start;
      ranges[i].end = end;
      i++;

      /* Skip until end or separator */
      while (*p && !is_range_separator (*p))
	p++;

      /* if not at end, skip separator */
      if (*p)
	p++;
    }

  *n_ranges_out = i;
  
  return ranges;
}

static EggPrintPages
dialog_get_print_pages (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv = dialog->priv;
  
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->all_pages_radio)))
    return EGG_PRINT_PAGES_ALL;
  else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->current_page_radio)))
    return EGG_PRINT_PAGES_CURRENT;
  else
    return EGG_PRINT_PAGES_RANGES;
}

static double
dialog_get_scale (EggPrintUnixDialog *dialog)
{
  return gtk_spin_button_get_value (GTK_SPIN_BUTTON (dialog->priv->scale_spin));
}

static EggPageSet
dialog_get_page_set (EggPrintUnixDialog *dialog)
{
  return (EggPageOrientation)gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->priv->page_set_combo));
}

static int
dialog_get_n_copies (EggPrintUnixDialog *dialog)
{
  return gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (dialog->priv->copies_spin));
}

static gboolean
dialog_get_collate (EggPrintUnixDialog *dialog)
{
  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->collate_check));
}

static gboolean
dialog_get_reverse (EggPrintUnixDialog *dialog)
{
  return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_check));
}


static EggPageOrientation
dialog_get_orientation (EggPrintUnixDialog *dialog)
{
  return (EggPageOrientation)gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->priv->orientation_combo));
}

static int 
dialog_get_pages_per_sheet (EggPrintUnixDialog *dialog)
{
  const char *val;
  int num;

  val = egg_printer_option_widget_get_value (dialog->priv->pages_per_sheet);

  num = 1;
  
  if (val)
    {
      num = atoi(val);
      if (num < 1)
	num = 1;
    }
  
  return num;
}


static gboolean
draw_page_cb (GtkWidget	     *widget,
	      GdkEventExpose      *event,
	      EggPrintUnixDialog *dialog)
{
  cairo_t *cr;
  double ratio;
  int w, h, tmp, shadow_offset;
  int pages_x, pages_y, i, x, y, layout_w, layout_h;
  double page_width, page_height;
  EggPageOrientation orientation;
  gboolean landscape;
  PangoLayout *layout;
  PangoFontDescription *font;
  char *text;
  
  orientation = dialog_get_orientation (dialog);
  landscape =
    (orientation == EGG_PAGE_ORIENTATION_LANDSCAPE) ||
    (orientation == EGG_PAGE_ORIENTATION_REVERSE_LANDSCAPE);
  
  cr = gdk_cairo_create (widget->window);
  
  ratio = 1.4142;

  w = (EXAMPLE_PAGE_AREA_SIZE - 3) / ratio;
  h = w * ratio;

  switch (dialog_get_pages_per_sheet (dialog)) {
  default:
  case 1:
    pages_x = 1; pages_y = 1;
    break;
  case 2:
    landscape = !landscape;
    pages_x = 1; pages_y = 2;
    break;
  case 4:
    pages_x = 2; pages_y = 2;
    break;
  case 6:
    landscape = !landscape;
    pages_x = 2; pages_y = 3;
    break;
  case 9:
    pages_x = 3; pages_y = 3;
    break;
  case 16:
    pages_x = 4; pages_y = 4;
    break;
  }

  if (landscape)
    {
      tmp = w;
      w = h;
      h = tmp;

      tmp = pages_x;
      pages_x = pages_y;
      pages_y = tmp;
    }
  
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

  i = 1;

  page_width = (double)w / pages_x;
  page_height = (double)h / pages_y;

  layout  = pango_cairo_create_layout (cr);

  font = pango_font_description_new ();
  pango_font_description_set_family (font, "sans");
  pango_font_description_set_absolute_size (font, page_height * 0.4 * PANGO_SCALE);
  pango_layout_set_font_description (layout, font);
  pango_font_description_free (font);

  pango_layout_set_width (layout, page_width * PANGO_SCALE);
  pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
  
  for (y = 0; y < pages_y; y++)
    {
      for (x = 0; x < pages_x; x++)
	{
	  text = g_strdup_printf ("%d", i++);
	  pango_layout_set_text (layout, text, -1);
	  g_free (text);
	  pango_layout_get_size (layout, &layout_w, &layout_h);
	  cairo_save (cr);
	  cairo_translate (cr,
			   x * page_width,
			   y * page_height + (page_height - layout_h / 1024.0) / 2
			   );
	  
	  pango_cairo_show_layout (cr, layout);
	  cairo_restore (cr);
	}
    }
  
  

  
  return TRUE;
}

static void
redraw_page_layout_preview (EggPrintUnixDialog *dialog)
{
  if (dialog->priv->page_layout_preview)
    gtk_widget_queue_draw (dialog->priv->page_layout_preview);
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

  widget = egg_printer_option_widget_new (NULL);
  g_signal_connect_swapped (widget, "changed", G_CALLBACK (redraw_page_layout_preview), dialog);
  priv->pages_per_sheet = EGG_PRINTER_OPTION_WIDGET (widget);
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

  widget = egg_printer_option_widget_new (NULL);
  priv->duplex = EGG_PRINTER_OPTION_WIDGET (widget);
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
  priv->scale_spin = spinbutton;
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
  g_signal_connect_swapped (combo, "changed",
			    G_CALLBACK (redraw_page_layout_preview), dialog);
  priv->orientation_combo = combo;
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 3, 4,  GTK_FILL, 0,
		    0, 0);
  /* In enum order: */
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
  priv->page_set_combo = combo;
  gtk_widget_show (combo);
  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 2, 4, 5,  GTK_FILL, 0,
		    0, 0);
  /* In enum order */
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

  widget = egg_printer_option_widget_new (NULL);
  priv->paper_size = EGG_PRINTER_OPTION_WIDGET (widget);
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

  widget = egg_printer_option_widget_new (NULL);
  priv->paper_type = EGG_PRINTER_OPTION_WIDGET (widget);
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

  widget = egg_printer_option_widget_new (NULL);
  priv->paper_source = EGG_PRINTER_OPTION_WIDGET (widget);
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

  widget = egg_printer_option_widget_new (NULL);
  priv->output_tray = EGG_PRINTER_OPTION_WIDGET (widget);
  gtk_widget_show (widget);
  gtk_table_attach (GTK_TABLE (table), widget,
		    1, 2, 3, 4,  GTK_FILL, 0,
		    0, 0);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (main_vbox), hbox2, TRUE, TRUE, 6);

  draw = gtk_drawing_area_new ();
  dialog->priv->page_layout_preview = draw;
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
create_optional_page (EggPrintUnixDialog *dialog,
		      const char *text,
		      GtkWidget **table_out,
		      GtkWidget **page_out)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *table, *label, *scrolled;
  
  priv = dialog->priv;

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_AUTOMATIC);
  gtk_widget_show (scrolled);
  
  table = gtk_table_new (1, 2, FALSE);
  gtk_widget_show (table);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					 table);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN(scrolled)->child),
				GTK_SHADOW_NONE);
  
  label = gtk_label_new (text);
  gtk_widget_show (label);
  
  gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook),
			    scrolled, label);

  *table_out = table;
  *page_out = scrolled;
}

static void
create_advanced_page (EggPrintUnixDialog *dialog)
{
  EggPrintUnixDialogPrivate *priv;
  GtkWidget *main_vbox, *label, *scrolled;
  
  priv = dialog->priv;

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  priv->advanced_page = scrolled;
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				  GTK_POLICY_NEVER,
				  GTK_POLICY_AUTOMATIC);
  gtk_widget_show (scrolled);

  main_vbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (main_vbox);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled),
					 main_vbox);
  gtk_viewport_set_shadow_type (GTK_VIEWPORT (GTK_BIN(scrolled)->child),
				GTK_SHADOW_NONE);
  
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
  GtkWidget *hbox, *conflict_hbox, *image, *label;
  
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
  create_optional_page (dialog, _("Image Quality"),
			&dialog->priv->image_quality_table,
			&dialog->priv->image_quality_page);
  create_optional_page (dialog, _("Color"),
			&dialog->priv->color_table,
			&dialog->priv->color_page);
  create_optional_page (dialog, _("Finishing"),
			&dialog->priv->finishing_table,
			&dialog->priv->finishing_page);
  create_advanced_page (dialog);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
                    FALSE, TRUE, 0);
  
  conflict_hbox = gtk_hbox_new (FALSE, 0);
  image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU);
  gtk_widget_show (image);
  gtk_box_pack_start (GTK_BOX (conflict_hbox), image, FALSE, TRUE, 0);
  label = gtk_label_new (_("Some of the settings in the dialog conflict"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (conflict_hbox), label, FALSE, TRUE, 0);
  dialog->priv->conflicts_widget = conflict_hbox;

  gtk_box_pack_start (GTK_BOX (hbox), conflict_hbox,
		      FALSE, FALSE, 0);

  /* Reparent the action area into the hbox. This is so we can have the
   * conflict warning on the same row, but not make the buttons the same
   * width as the warning (which the buttonbox does).
   */
  g_object_ref (GTK_DIALOG (dialog)->action_area);
  gtk_container_remove (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox),
			GTK_DIALOG (dialog)->action_area);
  gtk_box_pack_end (GTK_BOX (hbox), GTK_DIALOG (dialog)->action_area,
		    FALSE, FALSE, 0);
  g_object_unref (GTK_DIALOG (dialog)->action_area);
  
  gtk_widget_show (dialog->priv->notebook);
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
  if (dialog->priv->current_printer)
    return g_object_ref (dialog->priv->current_printer);
  
  return NULL; 
}

void
egg_print_unix_dialog_set_current_page (EggPrintUnixDialog *dialog,
					int                 current_page)
{
  dialog->priv->current_page = current_page;

  if (dialog->priv->current_page_radio)
    gtk_widget_set_sensitive (dialog->priv->current_page_radio, current_page != -1);
}


EggPrintSettings *
egg_print_unix_dialog_get_settings (EggPrintUnixDialog *dialog)
{
  EggPrintSettings *settings;
  EggPrintPages print_pages;

  settings = egg_print_settings_new ();

  if (dialog->priv->current_printer)
    egg_print_settings_set_printer (settings,
				    egg_printer_get_name (dialog->priv->current_printer));
  else
    egg_print_settings_set_printer (settings, "default");
  

  egg_print_settings_set_orientation (settings,
				      dialog_get_orientation (dialog));

  egg_print_settings_set_collate (settings,
				  dialog_get_collate (dialog));
  
  egg_print_settings_set_reverse (settings,
				  dialog_get_reverse (dialog));
  
  egg_print_settings_set_num_copies (settings,
				     dialog_get_n_copies (dialog));
  

  egg_print_settings_set_scale (settings,
				dialog_get_scale (dialog));
  
  egg_print_settings_set_page_set (settings,
				   dialog_get_page_set (dialog));
  
  print_pages = dialog_get_print_pages (dialog);
  egg_print_settings_set_print_pages (settings, print_pages);

  if (print_pages == EGG_PRINT_PAGES_RANGES)
    {
      EggPageRange *ranges;
      int n_ranges;

      ranges = dialog_get_page_ranges (dialog, &n_ranges);

      egg_print_settings_set_page_ranges  (settings, ranges, n_ranges);
      g_free (ranges);
    }

  /* TODO: prio, billing, print when, cover page (before/after)
     How to handle? */

  if (dialog->priv->current_printer)
    _egg_printer_get_settings_from_options (dialog->priv->current_printer,
					    dialog->priv->options,
					    settings);
  
  return settings;
}
