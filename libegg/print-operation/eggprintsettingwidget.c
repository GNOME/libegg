/* EggPrintSettingWidget
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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eggintl.h"
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "eggprintsettingwidget.h"

#define EGG_PRINT_SETTING_WIDGET_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PRINT_SETTING_WIDGET, EggPrintSettingWidgetPrivate))

static void egg_print_setting_widget_finalize     (GObject *object);

static void deconstruct_widgets (EggPrintSettingWidget *widget);
static void construct_widgets (EggPrintSettingWidget *setting);
static void update_widgets (EggPrintSettingWidget *setting);

struct EggPrintSettingWidgetPrivate
{
  EggPrintBackendSetting *source;
  gulong		  source_changed_handler;
  
  GtkWidget *check;
  GtkWidget *combo;
  GtkWidget *entry;
  GtkWidget *image;
  GtkWidget *label;
};

enum {
  CHANGED,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_SOURCE,
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EggPrintSettingWidget, egg_print_setting_widget, GTK_TYPE_HBOX);

static void egg_print_setting_widget_set_property (GObject      *object,
						   guint         prop_id,
						   const GValue *value,
						   GParamSpec   *pspec);
static void egg_print_setting_widget_get_property (GObject      *object,
						   guint         prop_id,
						   GValue       *value,
						   GParamSpec   *pspec);

static void
egg_print_setting_widget_class_init (EggPrintSettingWidgetClass *class)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GObjectClass *) class;
  widget_class = (GtkWidgetClass *) class;

  object_class->finalize = egg_print_setting_widget_finalize;
  object_class->set_property = egg_print_setting_widget_set_property;
  object_class->get_property = egg_print_setting_widget_get_property;

  g_type_class_add_private (class, sizeof (EggPrintSettingWidgetPrivate));  

  signals[CHANGED] =
    g_signal_new ("changed",
		  G_TYPE_FROM_CLASS (class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggPrintSettingWidgetClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  g_object_class_install_property (object_class,
                                   PROP_SOURCE,
                                   g_param_spec_object ("source",
							P_("Source setting"),
							P_("The PrintBackendSetting backing this widget"),
							EGG_TYPE_PRINT_BACKEND_SETTING,
							GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));

}

static void
egg_print_setting_widget_init (EggPrintSettingWidget *widget)
{
  widget->priv = EGG_PRINT_SETTING_WIDGET_GET_PRIVATE (widget); 
}

static void
egg_print_setting_widget_finalize (GObject *object)
{
  EggPrintSettingWidget *setting;
  
  setting = EGG_PRINT_SETTING_WIDGET (object);

  if (setting->priv->source)
    {
      g_object_unref (setting->priv->source);
      setting->priv->source = NULL;
    }
  
  if (G_OBJECT_CLASS (egg_print_setting_widget_parent_class)->finalize)
    G_OBJECT_CLASS (egg_print_setting_widget_parent_class)->finalize (object);
}

static void
egg_print_setting_widget_set_property (GObject         *object,
				       guint            prop_id,
				       const GValue    *value,
				       GParamSpec      *pspec)
{
  EggPrintSettingWidget *setting;
  
  setting = EGG_PRINT_SETTING_WIDGET (object);

  switch (prop_id)
    {
    case PROP_SOURCE:
      egg_print_setting_widget_set_source (setting, g_value_get_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_print_setting_widget_get_property (GObject    *object,
				       guint       prop_id,
				       GValue     *value,
				       GParamSpec *pspec)
{
  EggPrintSettingWidget *setting;
  
  setting = EGG_PRINT_SETTING_WIDGET (object);

  switch (prop_id)
    {
    case PROP_SOURCE:
      g_value_set_object (value, setting->priv->source);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
emit_changed (EggPrintSettingWidget *widget)
{
  g_signal_emit (widget, signals[CHANGED], 0);
}

GtkWidget *
egg_print_setting_widget_new (EggPrintBackendSetting *source)
{
  return g_object_new (EGG_TYPE_PRINT_SETTING_WIDGET, "source", source, NULL);
}

static void
source_changed_cb (EggPrintBackendSetting *source,
		   EggPrintSettingWidget  *setting)
{
  update_widgets (setting);
  emit_changed (setting);
}

void
egg_print_setting_widget_set_source (EggPrintSettingWidget  *setting,
				     EggPrintBackendSetting *source)
{
  if (source)
    g_object_ref (source);
  
  if (setting->priv->source)
    {
      g_signal_handler_disconnect (setting->priv->source,
				   setting->priv->source_changed_handler);
      g_object_unref (setting->priv->source);
    }

  setting->priv->source = source;

  if (source)
    {
      setting->priv->source_changed_handler =
	g_signal_connect (source, "changed", G_CALLBACK (source_changed_cb), setting);
    }

  construct_widgets (setting);
  update_widgets (setting);

  g_object_notify (G_OBJECT (setting), "source");
}

static GtkWidget *
combo_box_new (void)
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
combo_box_append (GtkWidget *combo,
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
combo_box_set (GtkWidget *combo,
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

static char *
combo_box_get (GtkWidget *combo)
{
  GtkTreeModel *model;
  char *val;
  GtkTreeIter iter;
  
  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));

  val = NULL;
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
    gtk_tree_model_get (model, &iter,
			1, &val,
			-1);
  return val;
}


static void
deconstruct_widgets (EggPrintSettingWidget *widget)
{
  if (widget->priv->check)
    {
      gtk_widget_destroy (widget->priv->check);
      widget->priv->check = NULL;
    }
  
  if (widget->priv->combo)
    {
      gtk_widget_destroy (widget->priv->combo);
      widget->priv->combo = NULL;
    }
  
  if (widget->priv->entry)
    {
      gtk_widget_destroy (widget->priv->entry);
      widget->priv->entry = NULL;
    }

  if (widget->priv->image)
    {
      gtk_widget_destroy (widget->priv->image);
      widget->priv->image = NULL;
    }

  if (widget->priv->label)
    {
      gtk_widget_destroy (widget->priv->label);
      widget->priv->label = NULL;
    }
}

static void
check_toggled_cb (GtkToggleButton *toggle_button,
		  EggPrintSettingWidget *setting)
{
  g_signal_handler_block (setting->priv->source, setting->priv->source_changed_handler);
  egg_print_backend_setting_set_boolean (setting->priv->source,
					 gtk_toggle_button_get_active (toggle_button));
  g_signal_handler_unblock (setting->priv->source, setting->priv->source_changed_handler);
  emit_changed (setting);
}

static void
combo_changed_cb (GtkWidget *combo,
		  EggPrintSettingWidget *setting)
{
  char *value;
  
  g_signal_handler_block (setting->priv->source, setting->priv->source_changed_handler);
  value = combo_box_get (combo);
  if (value)
    egg_print_backend_setting_set (setting->priv->source, value);
  g_free (value);
  g_signal_handler_unblock (setting->priv->source, setting->priv->source_changed_handler);
  emit_changed (setting);
}


static void
construct_widgets (EggPrintSettingWidget *setting)
{
  EggPrintBackendSetting *source;
  char *text;
  int i;

  source = setting->priv->source;
  
  deconstruct_widgets (setting);
  
  if (source == NULL)
    {
      setting->priv->combo = combo_box_new ();
      combo_box_append (setting->priv->combo,_("Not available"), "None");
      gtk_combo_box_set_active (GTK_COMBO_BOX (setting->priv->combo), 0);
      gtk_widget_set_sensitive (setting->priv->combo, FALSE);
      gtk_widget_show (setting->priv->combo);
      gtk_box_pack_start (GTK_BOX (setting), setting->priv->combo, TRUE, TRUE, 0);
    }
  else switch (source->type)
    {
    case EGG_PRINT_BACKEND_SETTING_TYPE_BOOLEAN:
      setting->priv->check = gtk_check_button_new_with_mnemonic (source->display_text);
      g_signal_connect (setting->priv->check, "toggled", G_CALLBACK (check_toggled_cb), setting);
      gtk_widget_show (setting->priv->check);
      gtk_box_pack_start (GTK_BOX (setting), setting->priv->check, TRUE, TRUE, 0);
      break;
    case EGG_PRINT_BACKEND_SETTING_TYPE_PICKONE:
      setting->priv->combo = combo_box_new ();
      for (i = 0; i < source->num_choices; i++)
	  combo_box_append (setting->priv->combo,
			    source->choices_display[i],
			    source->choices[i]);
      gtk_widget_show (setting->priv->combo);
      gtk_box_pack_start (GTK_BOX (setting), setting->priv->combo, TRUE, TRUE, 0);
      g_signal_connect (setting->priv->combo, "changed", G_CALLBACK (combo_changed_cb), setting);

      text = g_strdup_printf ("%s: ", source->display_text);
      setting->priv->label = gtk_label_new_with_mnemonic (text);
      g_free (text);
      gtk_widget_show (setting->priv->label);
      break;
    case EGG_PRINT_BACKEND_SETTING_TYPE_STRING:
      break;
    default:
      break;
    }

  setting->priv->image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (setting), setting->priv->image, FALSE, FALSE, 0);
}

static void
update_widgets (EggPrintSettingWidget *setting)
{
  EggPrintBackendSetting *source;

  source = setting->priv->source;
  
  if (source == NULL)
    {
      gtk_widget_hide (setting->priv->image);
      return;
    }

  switch (source->type)
    {
    case EGG_PRINT_BACKEND_SETTING_TYPE_BOOLEAN:
      if (strcmp (source->value, "True") == 0)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (setting->priv->check), TRUE);
      else
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (setting->priv->check), FALSE);
      break;
    case EGG_PRINT_BACKEND_SETTING_TYPE_PICKONE:
      combo_box_set (setting->priv->combo, source->value);
      break;
    case EGG_PRINT_BACKEND_SETTING_TYPE_STRING:
      break;
    default:
      break;
    }

  if (source->has_conflict)
    gtk_widget_show (setting->priv->image);
  else
    gtk_widget_hide (setting->priv->image);
}

gboolean
egg_print_setting_widget_has_external_label (EggPrintSettingWidget  *setting)
{
  return setting->priv->label != NULL;
}

GtkWidget *
egg_print_setting_widget_get_external_label (EggPrintSettingWidget  *setting)
{
  return setting->priv->label;
}

const char *
egg_print_setting_widget_get_value (EggPrintSettingWidget  *setting)
{
  if (setting->priv->source)
    {
      return setting->priv->source->value;
    }
  
  return "";
}
