/* EggPageSetupUnixDialog 
 * Copyright (C) 2006 Alexander Larsson <alexl@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <string.h>
#include <locale.h>
#include <langinfo.h>

#include "eggintl.h"
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "eggpagesetupunixdialog.h"
#include "eggprintbackendcups.h"
#include "eggprinter-private.h"
#include "eggpapersize.h"

struct EggPageSetupUnixDialogPrivate
{
  GtkListStore *printer_list;
  GtkListStore *paper_size_list;
  EggPrintBackend *print_backend;

  GtkWidget *printer_combo;
  GtkWidget *paper_size_combo;
  GtkWidget *paper_size_label;

  GtkWidget *portrait_radio;
  GtkWidget *landscape_radio;
  GtkWidget *reverse_portrait_radio;
  GtkWidget *reverse_landscape_radio;

  guint request_details_tag;
  
  EggPrintSettings *print_settings;
  /* These are stored in mm */
  double top_margin, bottom_margin, left_margin, right_margin;
};

enum {
  PROP_0,
  PROP_PRINT_BACKEND
};

enum {
  PRINTER_LIST_COL_NAME,
  PRINTER_LIST_COL_PRINTER_OBJ,
  PRINTER_LIST_N_COLS
};

enum {
  PAPER_SIZE_LIST_COL_NAME,
  PAPER_SIZE_LIST_COL_PAPER_SIZE,
  PAPER_SIZE_LIST_N_COLS
};

G_DEFINE_TYPE (EggPageSetupUnixDialog, egg_page_setup_unix_dialog, GTK_TYPE_DIALOG);

#define EGG_PAGE_SETUP_UNIX_DIALOG_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PAGE_SETUP_UNIX_DIALOG, EggPageSetupUnixDialogPrivate))

static void egg_page_setup_unix_dialog_finalize     (GObject                *object);
static void egg_page_setup_unix_dialog_set_property (GObject                *object,
						     guint                   prop_id,
						     const GValue           *value,
						     GParamSpec             *pspec);
static void egg_page_setup_unix_dialog_get_property (GObject                *object,
						     guint                   prop_id,
						     GValue                 *value,
						     GParamSpec             *pspec);
static void populate_dialog                         (EggPageSetupUnixDialog *dialog);
static void fill_paper_sizes_from_printer           (EggPageSetupUnixDialog *dialog,
						     EggPrinter             *printer);

static const char * const common_paper_sizes[] = {
  "na_letter",
  "na_legal",
  "iso_a4",
  "iso_a5",
  "roc_16k",
  "iso_b5",
  "jis_b5",
  "na_number-10",
  "iso_dl",
  "jpn_chou3",
  "na_ledger",
  "iso_a3",
};

static EggUnit
get_default_user_units (void)
{
  /* Translate to the default units to use for presenting
   * lengths to the user. Translate to default:inch if you
   * want inches, otherwise translate to default:mm.
   * Do *not* translate it to "predefinito:mm", if it
   * it isn't default:mm or default:inch it will not work 
   */
  char *e = _("default:mm");
  
#ifdef HAVE__NL_MEASUREMENT_MEASUREMENT
  char *imperial = NULL;
  
  imperial = nl_langinfo(_NL_MEASUREMENT_MEASUREMENT);
  if (imperial && imperial[0] == 2 )
    return EGG_UNIT_INCH;  /* imperial */
  if (imperial && imperial[0] == 1 )
    return EGG_UNIT_MM;  /* metric */
#endif
  
  if (strcmp (e, "default:inch")==0) {
    return EGG_UNIT_INCH;
  } else if (strcmp (e, "default:mm")) {
    g_warning ("Whoever translated default:mm did so wrongly.\n");
  }
  return EGG_UNIT_MM;
}

static void
egg_page_setup_unix_dialog_class_init (EggPageSetupUnixDialogClass *class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;

  object_class->finalize = egg_page_setup_unix_dialog_finalize;
  object_class->set_property = egg_page_setup_unix_dialog_set_property;
  object_class->get_property = egg_page_setup_unix_dialog_get_property;

  g_type_class_add_private (class, sizeof (EggPageSetupUnixDialogPrivate));  

  /* TODO: This is installed twice atm */
  gtk_settings_install_property (g_param_spec_string ("gtk-print-backend",
						      P_("Default print backend"),
						      P_("Name of the printbackend to use by default"),
						      NULL,
						      GTK_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                                   PROP_PRINT_BACKEND,
                                   g_param_spec_string ("print-backend",
							P_("Print backend"),
							P_("The print backend to use"),
							NULL,
							GTK_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
}

static void
egg_page_setup_unix_dialog_init (EggPageSetupUnixDialog *dialog)
{
  GtkTreeIter iter;
  
  dialog->priv = EGG_PAGE_SETUP_UNIX_DIALOG_GET_PRIVATE (dialog); 
  dialog->priv->print_backend = NULL;

  dialog->priv->printer_list = gtk_list_store_new (PRINTER_LIST_N_COLS,
						   G_TYPE_STRING, 
						   G_TYPE_OBJECT);

  gtk_list_store_append (dialog->priv->printer_list, &iter);
  gtk_list_store_set (dialog->priv->printer_list, &iter,
                      PRINTER_LIST_COL_NAME, _("<b>Any Printer</b>\nFor portable documents"),
                      PRINTER_LIST_COL_PRINTER_OBJ, NULL,
                      -1);
  
  dialog->priv->paper_size_list = gtk_list_store_new (PAPER_SIZE_LIST_N_COLS,
						      G_TYPE_STRING,
						      EGG_TYPE_PAPER_SIZE);

  populate_dialog (dialog);
}

static void
egg_page_setup_unix_dialog_finalize (GObject *object)
{
  EggPageSetupUnixDialog *dialog = EGG_PAGE_SETUP_UNIX_DIALOG (object);
  
  g_return_if_fail (object != NULL);

  if (dialog->priv->request_details_tag)
    {
      g_source_remove (dialog->priv->request_details_tag);
      dialog->priv->request_details_tag = 0;
    }
  
  if (dialog->priv->printer_list)
    {
      g_object_unref (dialog->priv->printer_list);
      dialog->priv->printer_list = NULL;
    }

  if (dialog->priv->paper_size_list)
    {
      g_object_unref (dialog->priv->paper_size_list);
      dialog->priv->paper_size_list = NULL;
    }

  if (dialog->priv->print_settings)
    {
      g_object_unref (dialog->priv->print_settings);
      dialog->priv->print_settings = NULL;
    }
  
  if (G_OBJECT_CLASS (egg_page_setup_unix_dialog_parent_class)->finalize)
    G_OBJECT_CLASS (egg_page_setup_unix_dialog_parent_class)->finalize (object);
}

static void
_printer_added_cb (EggPrintBackend *backend, 
                   EggPrinter *printer, 
		   EggPageSetupUnixDialog *dialog)
{
  GtkTreeIter iter;
  char *str;
  
  str = g_strdup_printf ("<b>%s</b>\n%s",
			 egg_printer_get_name (printer),
			 egg_printer_get_location (printer));
  
  gtk_list_store_append (dialog->priv->printer_list, &iter);
  gtk_list_store_set (dialog->priv->printer_list, &iter,
                      PRINTER_LIST_COL_NAME, str,
                      PRINTER_LIST_COL_PRINTER_OBJ, printer,
                      -1);
  g_free (str);
}

static void
_printer_removed_cb (EggPrintBackend *backend, 
                   EggPrinter *printer, 
		   EggPageSetupUnixDialog *impl)
{
}


static void
_printer_status_cb (EggPrintBackend *backend, 
                   EggPrinter *printer, 
		   EggPageSetupUnixDialog *impl)
{
}

static void
_printer_list_initialize (EggPageSetupUnixDialog *impl)
{
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
_set_print_backend (EggPageSetupUnixDialog *impl,
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
egg_page_setup_unix_dialog_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)

{
  EggPageSetupUnixDialog *impl = EGG_PAGE_SETUP_UNIX_DIALOG (object);

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
egg_page_setup_unix_dialog_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
  /* EggPageSetupUnixDialog *impl = EGG_PAGE_SETUP_UNIX_DIALOG (object); */

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
paper_size_row_is_separator (GtkTreeModel *model,
			     GtkTreeIter  *iter,
			     gpointer      data)
{
  char *name;

  gtk_tree_model_get (model, iter, 0, &name, -1);
  g_free (name);
  
  return name == NULL;
}

static EggPaperSize *
get_current_paper_size (EggPageSetupUnixDialog *dialog)
{
  EggPaperSize *current_paper_size;
  GtkComboBox *combo_box;
  GtkTreeIter iter;

  current_paper_size = NULL;
  
  combo_box = GTK_COMBO_BOX (dialog->priv->paper_size_combo);
  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    gtk_tree_model_get (GTK_TREE_MODEL (dialog->priv->paper_size_list), &iter,
			PAPER_SIZE_LIST_COL_PAPER_SIZE, &current_paper_size, -1);

  return current_paper_size;
}

static void
set_paper_size (EggPageSetupUnixDialog *dialog,
		EggPaperSize *paper_size)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  EggPaperSize *list_paper_size;

  model = GTK_TREE_MODEL (dialog->priv->paper_size_list);

  if (gtk_tree_model_get_iter_first (model, &iter))
    {
      do
	{
	  gtk_tree_model_get (GTK_TREE_MODEL (dialog->priv->paper_size_list), &iter,
			      PAPER_SIZE_LIST_COL_PAPER_SIZE, &list_paper_size, -1);
	  if (list_paper_size == NULL)
	    continue;
	  
	  if (egg_paper_size_is_equal (paper_size, list_paper_size))
	    {
	      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (dialog->priv->paper_size_combo),
					     &iter);
	      egg_paper_size_free (list_paper_size);
	      return;
	    }
	      
	  egg_paper_size_free (list_paper_size);
	  
	} while (gtk_tree_model_iter_next (model, &iter));
    }

  /* TODO: Maybe we should create a new row if the
   * paper size is not in the list?
   */
}

static void
fill_paper_sizes_from_printer (EggPageSetupUnixDialog *dialog,
			       EggPrinter *printer)
{
  GList *list, *l;
  EggPaperSize *paper_size, *current_paper_size;
  GtkComboBox *combo_box;
  GtkTreeIter iter;
  int current;
  int i;

  current_paper_size = get_current_paper_size (dialog);
  current = -1;
  gtk_list_store_clear (dialog->priv->paper_size_list);

  if (printer == NULL)
    {
      for (i = 0; i < G_N_ELEMENTS (common_paper_sizes); i++)
	{
	  paper_size = egg_paper_size_new (common_paper_sizes[i]);
	  gtk_list_store_append (dialog->priv->paper_size_list, &iter);
	  gtk_list_store_set (dialog->priv->paper_size_list, &iter,
			      PAPER_SIZE_LIST_COL_NAME,
			      egg_paper_size_get_display_name (paper_size),
			      PAPER_SIZE_LIST_COL_PAPER_SIZE, paper_size,
			      -1);
	  if (current_paper_size &&
	      egg_paper_size_is_equal (current_paper_size, paper_size))
	    current = i;
	  egg_paper_size_free (paper_size);
	}
    }
  else
    {
      list = _egg_printer_get_paper_sizes (printer);
      /* TODO: We should really sort this list so interesting size
	 are at the top */
      i = 0;
      for (l = list; l != NULL; l = l->next)
	{
	  paper_size = l->data;
	  gtk_list_store_append (dialog->priv->paper_size_list, &iter);
	  gtk_list_store_set (dialog->priv->paper_size_list, &iter,
			      PAPER_SIZE_LIST_COL_NAME,
			      egg_paper_size_get_display_name (paper_size),
			      PAPER_SIZE_LIST_COL_PAPER_SIZE, paper_size,
			      -1);
	  if (current_paper_size &&
	      egg_paper_size_is_equal (current_paper_size, paper_size))
	    current = i;
	  i++;
	  egg_paper_size_free (paper_size);
	}
    }

  gtk_list_store_append (dialog->priv->paper_size_list, &iter);
  gtk_list_store_set (dialog->priv->paper_size_list, &iter,
                      0, NULL,
                      -1);
  gtk_list_store_append (dialog->priv->paper_size_list, &iter);
  gtk_list_store_set (dialog->priv->paper_size_list, &iter,
                      0, _("Manage Custom Size..."),
                      -1);
  
  combo_box = GTK_COMBO_BOX (dialog->priv->paper_size_combo);
  if (current > 0)
    gtk_combo_box_set_active (combo_box, current);
  else
    /* TODO: pick better default */
    gtk_combo_box_set_active (combo_box, 0);
      
  if (current_paper_size)
    egg_paper_size_free (current_paper_size);
}

static void
printer_changed_finished_callback (EggPrinter *printer,
				   gboolean success,
				   EggPageSetupUnixDialog *dialog)
{
  dialog->priv->request_details_tag = 0;
  
  if (success)
    fill_paper_sizes_from_printer (dialog, printer);

}

static void
printer_changed_callback (GtkComboBox *combo_box,
			  EggPageSetupUnixDialog *dialog)
{
  EggPrinter *printer;
  GtkTreeIter iter;

  if (dialog->priv->request_details_tag)
    {
      g_source_remove (dialog->priv->request_details_tag);
      dialog->priv->request_details_tag = 0;
    }
  
  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    {
      gtk_tree_model_get (gtk_combo_box_get_model (combo_box), &iter,
			  PRINTER_LIST_COL_PRINTER_OBJ, &printer, -1);


      if (printer == NULL || _egg_printer_has_details (printer))
	fill_paper_sizes_from_printer (dialog, printer);
      else
	{
	  dialog->priv->request_details_tag =
	    g_signal_connect (printer, "details-acquired",
			      G_CALLBACK (printer_changed_finished_callback), dialog);
	  _egg_printer_request_details (printer);
	}

      /* TODO: Add format-for-printer to print_settings */
    }
}

/* We do this munging because we don't want to show zero digits
   after the decimal point, and not to many such digits if they
   are nonzero. I wish printf let you specify max precision for %f... */
static char *
double_to_string (double d, EggUnit unit)
{
  char *val, *p;
  struct lconv *locale_data;
  const char *decimal_point;
  int decimal_point_len;

  locale_data = localeconv ();
  decimal_point = locale_data->decimal_point;
  decimal_point_len = strlen (decimal_point);
  
  /* Max two decimal digits for inch, max one for mm */
  if (unit == EGG_UNIT_INCH)
    val = g_strdup_printf ("%.2f", d);
  else
    val = g_strdup_printf ("%.1f", d);
  
  if (strstr (val, decimal_point))
    {
      p = val + strlen (val) - 1;
      while (*p == '0')
        p--;
      if (p - val > decimal_point_len &&
	  strncmp (p - (decimal_point_len - 1), decimal_point, decimal_point_len) == 0)
        p -= decimal_point_len;
      p[1] = '\0';
    }
  return val;
}

static void
paper_size_changed (GtkComboBox *combo_box, EggPageSetupUnixDialog *dialog)
{
  GtkTreeIter iter;
  EggPaperSize *paper_size;
  EggUnit unit;
  double w, h;
  char *str, *ws, *hs;
  GtkLabel *label;

  label = GTK_LABEL (dialog->priv->paper_size_label);
   
  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    {
      gtk_tree_model_get (gtk_combo_box_get_model (combo_box),
			  &iter, PAPER_SIZE_LIST_COL_PAPER_SIZE, &paper_size, -1);

      if (paper_size == NULL)
	{
	  GtkWidget *error;
	  /* TODO: implement custom sizes */
	  error = gtk_message_dialog_new (GTK_WINDOW (dialog),
					  GTK_DIALOG_MODAL,
					  GTK_MESSAGE_INFO,
					  GTK_BUTTONS_CLOSE,
					  "Custom size dialog not implemented yet");
	  gtk_dialog_run (GTK_DIALOG (error));
	  gtk_widget_destroy (error);
	  return;
	}
      
      unit = get_default_user_units ();
      w = egg_paper_size_get_width (paper_size, unit);
      h = egg_paper_size_get_height (paper_size, unit);

      ws = double_to_string (w, unit);
      hs = double_to_string (h, unit);
      
      if (unit == EGG_UNIT_MM)
	str = g_strdup_printf ("%s x %s mm", ws, hs);
      else
	str = g_strdup_printf ("%s x %s inch", ws, hs);

      g_free (ws);
      g_free (hs);
      
      gtk_label_set_text (label, str);
      g_free (str);
      egg_paper_size_free (paper_size);
    }
  else
    gtk_label_set_text (label, "");

  /* TODO: set margins */
}

static void
populate_dialog (EggPageSetupUnixDialog *dialog)
{
  EggPageSetupUnixDialogPrivate *priv;
  GtkWidget *table, *label, *combo, *radio_button;
  GtkCellRenderer *cell;
  
  g_return_if_fail (EGG_IS_PAGE_SETUP_UNIX_DIALOG (dialog));
  
  priv = dialog->priv;

  table = gtk_table_new (3, 5, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
		      table, TRUE, TRUE, 6);
  gtk_widget_show (table);

  label = gtk_label_new_with_mnemonic (_("_Format for:"));
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 0, 1,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (label);

  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (dialog->priv->printer_list));
  dialog->priv->printer_combo = combo;
  
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                  "markup", PRINTER_LIST_COL_NAME,
                                  NULL);

  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 3, 0, 1,
		    GTK_FILL | GTK_EXPAND, 0, 0, 0);
  gtk_widget_show (combo);

  label = gtk_label_new_with_mnemonic (_("_Paper size:"));
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 1, 2,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (label);

  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (dialog->priv->paper_size_list));
  dialog->priv->paper_size_combo = combo;
  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combo), 
					paper_size_row_is_separator, NULL, NULL);
  
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                  "text", PAPER_SIZE_LIST_COL_NAME,
                                  NULL);

  gtk_table_attach (GTK_TABLE (table), combo,
		    1, 3, 1, 2,
		    GTK_FILL | GTK_EXPAND, 0, 0, 0);
  gtk_widget_show (combo);

  gtk_table_set_row_spacing (GTK_TABLE (table), 1, 0);
  
  label = gtk_label_new_with_mnemonic ("");
  dialog->priv->paper_size_label = label;
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_misc_set_padding (GTK_MISC (label), 12, 4);
  gtk_table_attach (GTK_TABLE (table), label,
		    1, 3, 2, 3,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (label);

  label = gtk_label_new_with_mnemonic (_("_Orientation:"));
  gtk_table_attach (GTK_TABLE (table), label,
		    0, 1, 3, 4,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (label);

  radio_button = gtk_radio_button_new_with_label (NULL, "port");
  dialog->priv->portrait_radio = radio_button;
  gtk_table_attach (GTK_TABLE (table), radio_button,
		    1, 2, 3, 4,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (radio_button);

  radio_button = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON(radio_button)),
						  "land");
  dialog->priv->landscape_radio = radio_button;
  gtk_table_attach (GTK_TABLE (table), radio_button,
		    2, 3, 3, 4,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (radio_button);

  gtk_table_set_row_spacing (GTK_TABLE (table), 3, 0);
  
  radio_button = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON(radio_button)),
						  "rport");
  dialog->priv->reverse_portrait_radio = radio_button;
  gtk_table_attach (GTK_TABLE (table), radio_button,
		    1, 2, 4, 5,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (radio_button);

  radio_button = gtk_radio_button_new_with_label (gtk_radio_button_get_group (GTK_RADIO_BUTTON(radio_button)),
						  "rland");
  dialog->priv->reverse_landscape_radio = radio_button;
  gtk_table_attach (GTK_TABLE (table), radio_button,
		    2, 3, 4, 5,
		    GTK_FILL, 0, 0, 0);
  gtk_widget_show (radio_button);


  g_signal_connect (dialog->priv->paper_size_combo, "changed", G_CALLBACK (paper_size_changed), dialog);
  g_signal_connect (dialog->priv->printer_combo, "changed", G_CALLBACK (printer_changed_callback), dialog);
  gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->priv->printer_combo), 0);
}

GtkWidget *
egg_page_setup_unix_dialog_new (const gchar *title,
				GtkWindow *parent,
				const gchar *print_backend)
{
  GtkWidget *result;

  if (title == NULL)
    title = _("Page Setup");
  
  result = g_object_new (EGG_TYPE_PAGE_SETUP_UNIX_DIALOG,
                         "title", title,
			 "has-separator", FALSE,
			 "print-backend", print_backend,
                         NULL);

  if (parent)
    gtk_window_set_transient_for (GTK_WINDOW (result), parent);

  gtk_dialog_add_buttons (GTK_DIALOG (result), 
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OK, GTK_RESPONSE_OK,
                          NULL);

  return result;
}

static EggPageOrientation
get_orientation (EggPageSetupUnixDialog *dialog)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->portrait_radio)))
    return EGG_PAGE_ORIENTATION_PORTRAIT;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->landscape_radio)))
    return EGG_PAGE_ORIENTATION_LANDSCAPE;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_portrait_radio)))
    return EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_landscape_radio)))
    return EGG_PAGE_ORIENTATION_REVERSE_LANDSCAPE;
  return EGG_PAGE_ORIENTATION_PORTRAIT;
}

static void
set_orientation (EggPageSetupUnixDialog *dialog, EggPageOrientation orientation)
{
  switch (orientation)
    {
    case EGG_PAGE_ORIENTATION_PORTRAIT:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->portrait_radio), TRUE);
      break;
    case EGG_PAGE_ORIENTATION_LANDSCAPE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->landscape_radio), TRUE);
      break;
    case EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_portrait_radio), TRUE);
      break;
    case EGG_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->priv->reverse_landscape_radio), TRUE);
      break;
    }
}


void
egg_page_setup_unix_dialog_set_page_setup (EggPageSetupUnixDialog *dialog,
					   EggPageSetup           *page_setup)
{
  if (page_setup)
    {
      set_paper_size (dialog, egg_page_setup_get_paper_size (page_setup));
      set_orientation (dialog, egg_page_setup_get_orientation (page_setup));
      dialog->priv->top_margin = egg_page_setup_get_top_margin (page_setup, EGG_UNIT_MM);
      dialog->priv->bottom_margin = egg_page_setup_get_bottom_margin (page_setup, EGG_UNIT_MM);
      dialog->priv->left_margin = egg_page_setup_get_left_margin (page_setup, EGG_UNIT_MM);
      dialog->priv->right_margin = egg_page_setup_get_right_margin (page_setup, EGG_UNIT_MM);
    }
  
}

EggPageSetup *
egg_page_setup_unix_dialog_get_page_setup (EggPageSetupUnixDialog *dialog)
{
  EggPageSetup *page_setup;
  EggPaperSize *paper_size;
  
  page_setup = egg_page_setup_new ();

  paper_size = get_current_paper_size (dialog);
  if (paper_size == NULL)
    paper_size = egg_paper_size_new (NULL);
  egg_page_setup_set_paper_size (page_setup, paper_size);
  egg_paper_size_free (paper_size);

  egg_page_setup_set_orientation (page_setup, get_orientation (dialog));

  egg_page_setup_set_top_margin (page_setup, dialog->priv->top_margin, EGG_UNIT_MM);
  egg_page_setup_set_bottom_margin (page_setup, dialog->priv->bottom_margin, EGG_UNIT_MM);
  egg_page_setup_set_left_margin (page_setup, dialog->priv->left_margin, EGG_UNIT_MM);
  egg_page_setup_set_right_margin (page_setup, dialog->priv->right_margin, EGG_UNIT_MM);

  return page_setup;
}

void
egg_page_setup_unix_dialog_set_print_settings (EggPageSetupUnixDialog *dialog,
					       EggPrintSettings       *print_settings)
{
  if (dialog->priv->print_settings)
    g_object_unref (dialog->priv->print_settings);

  dialog->priv->print_settings = print_settings;

  if (print_settings)
    g_object_ref (print_settings);

  /* TODO: Set format-for-printer if set. Delayed... */
}

EggPrintSettings *
egg_page_setup_unix_dialog_get_print_settings (EggPageSetupUnixDialog *dialog)
{
  return dialog->priv->print_settings;
}
