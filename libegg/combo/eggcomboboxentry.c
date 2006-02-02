/* eggcomboboxentry.c
 * Copyright (C) 2002, 2003  Kristian Rietveld <kris@gtk.org>
 *
 * <FIXME put GTK LGPL header here>
 */

#include "eggcomboboxentry.h"

#include <gtk/gtkentry.h>
#include <gtk/gtkcellrenderertext.h>


static void egg_combo_box_entry_active_changed   (EggComboBox *combo_box,
                                                  gpointer     user_data);
static void egg_combo_box_entry_contents_changed (GtkEntry    *entry,
                                                  gpointer     user_data);

GType
egg_combo_box_entry_get_type (void)
{
  static GType combo_box_entry_type = 0;

  if (!combo_box_entry_type)
    {
      static const GTypeInfo combo_box_entry_info =
        {
          sizeof (EggComboBoxEntryClass),
          NULL, /* base_init */
          NULL, /* base_finalize */
          NULL, /* class_init */
          NULL, /* class_finalize */
          NULL, /* class_data */
          sizeof (EggComboBoxEntry),
          0,
          NULL /* instance_init */
        };

      combo_box_entry_type = g_type_register_static (EGG_TYPE_COMBO_BOX,
                                                     "EggComboBoxEntry",
                                                     &combo_box_entry_info,
                                                     0);
    }

  return combo_box_entry_type;
}

static void
egg_combo_box_entry_active_changed (EggComboBox *combo_box,
                                    gpointer     user_data)
{
  gint index;
  EggComboBoxEntry *entry = EGG_COMBO_BOX_ENTRY (combo_box);

  index = egg_combo_box_get_active (combo_box);

  g_signal_handlers_block_by_func (entry->entry,
                                   egg_combo_box_entry_contents_changed,
                                   combo_box);

  if (index < 0)
    gtk_entry_set_text (GTK_ENTRY (entry->entry), "");
  else
    {
      gchar *str = NULL;
      GtkTreeIter iter;

      gtk_tree_model_iter_nth_child (combo_box->model, &iter, NULL, index);
      gtk_tree_model_get (combo_box->model, &iter,
                          entry->text_column, &str,
                          -1);

      gtk_entry_set_text (GTK_ENTRY (entry->entry), str);

      g_free (str);
    }

  g_signal_handlers_unblock_by_func (entry->entry,
                                     egg_combo_box_entry_contents_changed,
                                     combo_box);
}

static void
egg_combo_box_entry_contents_changed (GtkEntry *entry,
                                      gpointer  user_data)
{
  EggComboBox *combo_box = EGG_COMBO_BOX (user_data);

  g_signal_handlers_block_by_func (combo_box,
                                   egg_combo_box_entry_active_changed,
                                   NULL);
  egg_combo_box_set_active (combo_box, -1);
  g_signal_handlers_unblock_by_func (combo_box,
                                     egg_combo_box_entry_active_changed,
                                     NULL);
}

/* public API */
GtkWidget *
egg_combo_box_entry_new (GtkTreeModel *model,
                         gint          text_column)
{
  GtkWidget *ret;
  GtkCellRenderer *renderer;

  g_return_val_if_fail (GTK_IS_TREE_MODEL (model), NULL);
  g_return_val_if_fail (text_column >= 0, NULL);
  g_return_val_if_fail (text_column < gtk_tree_model_get_n_columns (model), NULL);

  ret = g_object_new (egg_combo_box_entry_get_type (),
                      "model", model,
                      NULL);

  EGG_COMBO_BOX_ENTRY (ret)->entry = gtk_entry_new ();
  gtk_container_add (GTK_CONTAINER (ret),
                     EGG_COMBO_BOX_ENTRY (ret)->entry);

  EGG_COMBO_BOX_ENTRY (ret)->text_column = text_column;
  renderer = gtk_cell_renderer_text_new ();
  egg_combo_box_pack_start (EGG_COMBO_BOX (ret), renderer, TRUE);
  egg_combo_box_set_attributes (EGG_COMBO_BOX (ret), renderer,
                                "text", text_column,
                                NULL);

  g_signal_connect (EGG_COMBO_BOX_ENTRY (ret)->entry, "changed",
                    G_CALLBACK (egg_combo_box_entry_contents_changed), ret);
  g_signal_connect (ret, "changed",
                    G_CALLBACK (egg_combo_box_entry_active_changed), NULL);

  return ret;
}
