#include "eggcomboboxtext.h"

#include <gtk/gtktreeselection.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkmain.h>

#include "eggentry.h"

#include <gdk/gdkkeysyms.h>

#include <string.h>


static void     egg_combo_box_text_class_init      (EggComboBoxTextClass *klass);
static void     egg_combo_box_text_init            (EggComboBoxText      *text);

static void     egg_combo_box_text_entry_changed   (GtkWidget            *entry,
		                                    gpointer              data);

static void     egg_combo_box_text_remove_grabs    (EggComboBoxText      *text);

static void     egg_combo_box_text_popped_up       (GtkWidget            *widget,
						    gpointer              data);

static gboolean egg_combo_box_text_button_pressed  (GtkWidget            *widget,
						    GdkEventButton       *event,
						    gpointer              data);
static gboolean egg_combo_box_text_button_released (GtkWidget            *widget,
						    GdkEventButton       *event,
						    gpointer              data);

static gboolean egg_combo_box_text_key_press       (GtkWidget            *widget,
						    GdkEventKey          *event,
						    gpointer              data);

GType
egg_combo_box_text_get_type (void)
{
  static GType combo_box_text_type = 0;

  if (!combo_box_text_type)
    {
      static const GTypeInfo combo_box_text_info =
        {
	  sizeof (EggComboBoxTextClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) egg_combo_box_text_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (EggComboBoxText),
	  0,
	  (GInstanceInitFunc) egg_combo_box_text_init
	};

      combo_box_text_type = g_type_register_static (EGG_TYPE_COMBO_BOX,
						    "EggComboBoxText",
						    &combo_box_text_info,
						    0);
    }

  return combo_box_text_type;
}

static void
egg_combo_box_text_class_init (EggComboBoxTextClass *klass)
{
}

static void
egg_combo_box_text_init (EggComboBoxText *textcombo)
{
  GtkTreeSelection *sel;

  textcombo->sample_index = -1;

  g_signal_connect (textcombo, "popped_up",
		    G_CALLBACK (egg_combo_box_text_popped_up), NULL);
  g_signal_connect (EGG_COMBO_BOX (textcombo)->button, "button_press_event",
		    G_CALLBACK (egg_combo_box_text_button_pressed), textcombo);

  textcombo->tree_view = gtk_tree_view_new ();
  g_signal_connect (textcombo->tree_view, "button_press_event",
		    G_CALLBACK (egg_combo_box_text_button_pressed), textcombo);
  g_signal_connect (textcombo->tree_view, "button_release_event",
		    G_CALLBACK (egg_combo_box_text_button_released), textcombo);
  g_signal_connect (textcombo->tree_view, "key_press_event",
		    G_CALLBACK (egg_combo_box_text_key_press), textcombo);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (textcombo->tree_view));

  gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (textcombo->tree_view),
				     FALSE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (textcombo->tree_view),
					       0, NULL, gtk_cell_renderer_text_new (),
					       NULL);
  gtk_widget_show (textcombo->tree_view);
}

/* callbacks */
static void
egg_combo_box_text_entry_changed (GtkWidget *entry,
		                  gpointer   data)
{
  EggComboBoxText *text = EGG_COMBO_BOX_TEXT (data);
  GtkTreeSelection *selection;
  GtkTreeIter iter;

  const gchar *str;

  gchar *normalized_string;
  gchar *case_normalized_string;

  str = egg_entry_get_text (EGG_ENTRY (text->entry));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (text->tree_view));

  /* empty entry? */
  if (!str || !strcmp ("", str))
    {
      gtk_tree_selection_unselect_all (selection);
      return;
    }

  /* iterate model */
  normalized_string = g_utf8_normalize (str, -1, G_NORMALIZE_ALL);
  case_normalized_string = g_utf8_casefold (str, -1);

  if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (text->store), &iter))
    goto unselect_out;

  do
    {
      gchar *item;
      gchar *normalized_item;
      gchar *case_normalized_item;
      gboolean out = FALSE;

      gtk_tree_model_get (GTK_TREE_MODEL (text->store), &iter,
	                  0, &item,
			  -1);

      if (!item)
	continue;

      normalized_item = g_utf8_normalize (item, -1, G_NORMALIZE_ALL);
      case_normalized_item = g_utf8_casefold (item, -1);

      if (!strncmp (case_normalized_string, case_normalized_item,
	            strlen (case_normalized_string)))
        {
	  GtkTreePath *path;

          path = gtk_tree_model_get_path (GTK_TREE_MODEL (text->store), &iter);
	  gtk_tree_view_set_cursor (GTK_TREE_VIEW (text->tree_view), path,
	                            NULL, FALSE);
	  gtk_tree_path_free (path);

	  out = TRUE;
	}

      g_free (normalized_item);
      g_free (case_normalized_item);
      g_free (item);

      /* ugh */
      if (out)
	goto out;
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (text->store), &iter));

  /* matched nothing, unselect */

unselect_out:
  gtk_tree_selection_unselect_all (selection);

out:
  g_free (normalized_string);
  g_free (case_normalized_string);
}

static void
egg_combo_box_text_remove_grabs (EggComboBoxText *text)
{
  EggComboBox *box = EGG_COMBO_BOX (text);

  if (GTK_WIDGET_HAS_GRAB (text->tree_view))
    gtk_grab_remove (text->tree_view);

  if (GTK_WIDGET_HAS_GRAB (box->popup_window))
    {
      gtk_grab_remove (box->popup_window);
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
}

static void
egg_combo_box_text_popped_up (GtkWidget *combobox,
			      gpointer   data)
{
  EggComboBox *box = EGG_COMBO_BOX (combobox);
  EggComboBoxText *text = EGG_COMBO_BOX_TEXT (combobox);

  if (!GTK_WIDGET_HAS_FOCUS (text->tree_view))
    {
      gdk_keyboard_grab (box->popup_window->window, FALSE, GDK_CURRENT_TIME);
      gtk_widget_grab_focus (text->tree_view);
    }
}

static gboolean
egg_combo_box_text_button_pressed (GtkWidget      *widget,
				   GdkEventButton *event,
				   gpointer        data)
{
  EggComboBox *box = EGG_COMBO_BOX (data);
  EggComboBoxText *text = EGG_COMBO_BOX_TEXT (data);

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (ewidget == text->tree_view)
    return TRUE;

  if (ewidget != box->button ||
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (box->button)))
    return FALSE;

  egg_combo_box_popup (box);

  gtk_grab_add (box->popup_window);
  gdk_pointer_grab (box->popup_window->window, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);

  /* FIXME: drag selection on treeview */
  gtk_grab_add (text->tree_view);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (box->button), TRUE);

  text->popup_in_progress = TRUE;

  return TRUE;
}

static gboolean
egg_combo_box_text_button_released (GtkWidget      *widget,
				    GdkEventButton *event,
				    gpointer        data)
{
  gboolean ret;
  GtkTreePath *path = NULL;

  EggComboBox *box = EGG_COMBO_BOX (data);
  EggComboBoxText *text = EGG_COMBO_BOX_TEXT (data);

  gboolean popup_in_progress = FALSE;

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (text->popup_in_progress)
    {
      popup_in_progress = TRUE;
      text->popup_in_progress = FALSE;
    }

  if (ewidget != text->tree_view)
    {
      if (ewidget == box->button &&
	  !popup_in_progress &&
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (box->button)))
        {
	  egg_combo_box_text_remove_grabs (text);
	  egg_combo_box_popdown (box);
	  return TRUE;
	}

      /* released outsite treeview */
      if (ewidget != box->button)
        {
          egg_combo_box_text_remove_grabs (text);
          egg_combo_box_popdown (box);

          return TRUE;
        }

      return FALSE;
    }

  /* drop grabs */
  egg_combo_box_text_remove_grabs (text);

  /* select something cool */
  ret = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
				       event->x, event->y,
				       &path,
				       NULL, NULL, NULL);

  if (!ret)
    return TRUE; /* clicked outside window? */

  egg_combo_box_text_set_sample (text, gtk_tree_path_get_indices (path)[0]);
  egg_combo_box_popdown (box);

  gtk_tree_path_free (path);

  return TRUE;
}

static gboolean
egg_combo_box_text_key_press (GtkWidget   *widget,
			      GdkEventKey *event,
			      gpointer     data)
{
  EggComboBoxText *text = EGG_COMBO_BOX_TEXT (data);

  if ((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter ||
       event->keyval == GDK_space || event->keyval == GDK_KP_Space) ||
      event->keyval == GDK_Escape)
    {
      if (event->keyval != GDK_Escape)
        {
	  gboolean ret;
	  GtkTreeIter iter;
	  GtkTreeModel *model = NULL;
	  GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (text->tree_view));

	  ret = gtk_tree_selection_get_selected (sel, &model, &iter);
	  if (ret)
	    {
	      GtkTreePath *path;

	      path = gtk_tree_model_get_path (model, &iter);
	      if (path)
	        {
	          egg_combo_box_text_set_sample (text, gtk_tree_path_get_indices (path)[0]);
	          gtk_tree_path_free (path);
		}
	    }
	}
      else
        {
	  GtkTreePath *path;

	  path = gtk_tree_path_new ();
	  gtk_tree_path_append_index (path, text->sample_index);

	  gtk_tree_view_set_cursor (GTK_TREE_VIEW (text->tree_view),
				    path, NULL, FALSE);

	  gtk_tree_path_free (path);
	}

      egg_combo_box_text_remove_grabs (text);
      egg_combo_box_popdown (EGG_COMBO_BOX (text));

      return TRUE;
    }

  return FALSE;
}

/* public API */
GtkWidget *
egg_combo_box_text_new_with_entry (EggEntry *entry)
{
  gint text_column;
  GList *list;
  GtkTreeModel *model;
  EggComboBoxText *textcombo;
  GtkTreeViewColumn *col;

  g_return_val_if_fail (EGG_IS_ENTRY (entry), NULL);
  g_return_val_if_fail (egg_entry_completion_enabled (entry), NULL);

  egg_entry_completion_get_model (entry, &model, &text_column);

  textcombo = EGG_COMBO_BOX_TEXT (g_object_new (egg_combo_box_text_get_type (), NULL));

  textcombo->entry = GTK_WIDGET (entry);
  g_signal_connect (textcombo->entry, "changed",
                    G_CALLBACK (egg_combo_box_text_entry_changed),
		    textcombo);

  gtk_tree_view_set_model (GTK_TREE_VIEW (textcombo->tree_view),
                           model);
  col = gtk_tree_view_get_column (GTK_TREE_VIEW (textcombo->tree_view), 0);
  list = gtk_tree_view_column_get_cell_renderers (col);
  gtk_tree_view_column_add_attribute (col, list->data,
                                      "text", text_column);
  g_list_free (list);

  textcombo->store = GTK_LIST_STORE (model);

  egg_combo_box_set_sample_widget (EGG_COMBO_BOX (textcombo),
				   textcombo->entry);
  egg_combo_box_set_popup_widget (EGG_COMBO_BOX (textcombo),
				  textcombo->tree_view);

  return GTK_WIDGET (textcombo);
}

GtkWidget *
egg_combo_box_text_new_with_model (GtkListStore *model,
                                   gint          text_column,
				   gint          history_max)
{
  EggEntry *entry;
  EggComboBoxText *textcombo;

  g_return_val_if_fail (GTK_IS_LIST_STORE (model), NULL);

  textcombo = EGG_COMBO_BOX_TEXT (g_object_new (egg_combo_box_text_get_type (), NULL));

  entry = EGG_ENTRY (egg_entry_new ());

  egg_entry_enable_completion (entry,
                               model, text_column, -1,
	                       NULL, NULL, NULL);
  egg_entry_history_set_max (entry, history_max);

  return egg_combo_box_text_new_with_entry (entry);
}

gint
egg_combo_box_text_get_index (EggComboBoxText *textcombo,
			      const gchar     *text)
{
  gint i;
  GtkTreeIter iter;

  g_return_val_if_fail (EGG_IS_COMBO_BOX_TEXT (textcombo), -1);
  g_return_val_if_fail (GTK_IS_TREE_MODEL (textcombo->store), -1);
  g_return_val_if_fail (text != NULL, -1);

  /* slow slow slow */

  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (textcombo->store), &iter);

  for (i = 0; i < textcombo->store->length; i++)
    {
      gint ret;
      GValue value = {0,};

      gtk_tree_model_get_value (GTK_TREE_MODEL (textcombo->store), &iter,
				0, &value);
      /* FIXME: use normalized strings here */
      ret = strcmp (text, g_value_get_string (&value));
      g_value_unset (&value);

      if (ret == 0)
	break;

      gtk_tree_model_iter_next (GTK_TREE_MODEL (textcombo->store), &iter);
    }

  if (i == textcombo->store->length)
    return -1;

  return i;
}

gint
egg_combo_box_text_get_length (EggComboBoxText *textcombo)
{
  g_return_val_if_fail (EGG_IS_COMBO_BOX_TEXT (textcombo), 0);

  return gtk_tree_model_iter_n_children (GTK_TREE_MODEL (textcombo->store), NULL);
}

gint
egg_combo_box_text_get_sample_index (EggComboBoxText *textcombo)
{
  g_return_val_if_fail (EGG_IS_COMBO_BOX_TEXT (textcombo), 0);

  return textcombo->sample_index;
}

gchar *
egg_combo_box_text_get_sample_text (EggComboBoxText *textcombo)
{
  GtkTreeIter iter;
  GValue value = {0,};
  gchar *ret;

  g_return_val_if_fail (EGG_IS_COMBO_BOX_TEXT (textcombo), NULL);
  g_return_val_if_fail (GTK_IS_TREE_MODEL (textcombo->store), NULL);

  if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (textcombo->store),
				      &iter, NULL,
				      textcombo->sample_index))
    return NULL;

  gtk_tree_model_get_value (GTK_TREE_MODEL (textcombo->store), &iter,
			    0, &value);

  ret = g_strdup (g_value_get_string (&value));
  g_value_unset (&value);

  return ret;
}

void
egg_combo_box_text_set_sample (EggComboBoxText *textcombo,
			       gint             index)
{
  GtkTreePath *path;

  g_return_if_fail (EGG_IS_COMBO_BOX_TEXT (textcombo));
  g_return_if_fail (index >= 0);
  g_return_if_fail (EGG_IS_ENTRY (textcombo->entry));
  g_return_if_fail (GTK_IS_TREE_MODEL (textcombo->store));
  g_return_if_fail (gtk_tree_model_iter_n_children (GTK_TREE_MODEL (textcombo->store), NULL));

  if (index == textcombo->sample_index)
    return;

  path = gtk_tree_path_new ();
  gtk_tree_path_append_index (path, index);

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (textcombo->tree_view), path,
			    NULL, FALSE);

  gtk_tree_path_free (path);

  textcombo->sample_index = index;
  egg_entry_set_text (EGG_ENTRY (textcombo->entry),
		      egg_combo_box_text_get_sample_text (textcombo));

  /* FIXME: need history hit? */

  g_signal_emit_by_name (textcombo, "changed", NULL, NULL);
}
