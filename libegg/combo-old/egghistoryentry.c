#include "egghistoryentry.h"

#include <gtk/gtktreeselection.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkbindings.h>

#include <gdk/gdkkeysyms.h>

#include "eggmarshalers.h"


enum {
  ACTIVE_CHANGED,
  POPPED_DOWN,
  POPPED_UP,
  POPUP,
  LAST_SIGNAL
};

static guint entry_signals[LAST_SIGNAL] = { 0 };


static void     egg_history_entry_class_init     (EggHistoryEntryClass *klass);
static void     egg_history_entry_init           (EggHistoryEntry *entry);

static void     egg_history_entry_remove_grabs   (EggHistoryEntry *entry);

static gboolean egg_history_entry_real_popup     (EggHistoryEntry *entry);
static void     egg_history_entry_popup          (EggHistoryEntry *entry);
static void     egg_history_entry_popdown        (EggHistoryEntry *entry);

static gboolean egg_history_entry_button_press   (GtkWidget *widget,
                                                  GdkEventButton *event,
                                                  gpointer data);
static gboolean egg_history_entry_button_release (GtkWidget *widget,
                                                  GdkEventButton *event,
                                                  gpointer data);
static gboolean egg_history_entry_key_press      (GtkWidget *widget,
                                                  GdkEventKey *event,
                                                  gpointer data);

GType
egg_history_entry_get_type (void)
{
  static GType history_entry_type = 0;

  if (!history_entry_type)
    {
      static const GTypeInfo history_entry_info =
        {
          sizeof (EggHistoryEntryClass),
          NULL, /* base_init */
          NULL, /* base_finalize */
          (GClassInitFunc) egg_history_entry_class_init,
          NULL, /* class_finalize */
          NULL, /* class_data */
          sizeof (EggHistoryEntry),
          0,
          (GInstanceInitFunc) egg_history_entry_init
        };

      history_entry_type = g_type_register_static (GTK_TYPE_ENTRY,
                                                   "EggHistoryEntry",
                                                   &history_entry_info,
                                                   0);
    }

  return history_entry_type;
}

static void
egg_history_entry_class_init (EggHistoryEntryClass *klass)
{
  GtkBindingSet *binding_set;

  binding_set = gtk_binding_set_by_class (klass);

  klass->popup = egg_history_entry_real_popup;

  entry_signals[ACTIVE_CHANGED] =
    g_signal_new ("active_changed",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggHistoryEntryClass, active_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  entry_signals[POPPED_UP] =
    g_signal_new ("popped_up",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggHistoryEntryClass, popped_up),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  entry_signals[POPPED_DOWN] =
    g_signal_new ("popped_down",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (EggHistoryEntryClass, popped_down),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  entry_signals[POPUP] =
    g_signal_new ("popup",
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (EggHistoryEntryClass, popup),
                  NULL, NULL,
                  _egg_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  /* FIXME: is this broken? */
  gtk_binding_entry_add_signal (binding_set, GDK_Down, GDK_MOD1_MASK, "popup", 0);
}

static void
egg_history_entry_init (EggHistoryEntry *entry)
{
  GtkTreeSelection *sel = NULL;

  gtk_entry_enable_popdown_button (GTK_ENTRY (entry));

  entry->popped_up = FALSE;

  entry->sample_index = -1;

  g_signal_connect (GTK_ENTRY (entry)->popdown_button, "button_press_event",
                    G_CALLBACK (egg_history_entry_button_press), entry);

  entry->popup_window = gtk_window_new (GTK_WINDOW_POPUP);

  entry->popup_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (entry->popup_frame), GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (entry->popup_window), entry->popup_frame);
  gtk_widget_show (entry->popup_frame);

  entry->tree_view = gtk_tree_view_new ();
  gtk_container_add (GTK_CONTAINER (entry->popup_frame), entry->tree_view);
  g_signal_connect (entry->tree_view, "button_press_event",
                    G_CALLBACK (egg_history_entry_button_press), entry);
  g_signal_connect (entry->tree_view, "button_release_event",
                    G_CALLBACK (egg_history_entry_button_release), entry);
  g_signal_connect (entry->tree_view, "key_press_event",
                    G_CALLBACK (egg_history_entry_key_press), entry);

  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry->tree_view));

  gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (entry->tree_view),
                                     FALSE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (entry->tree_view),
                                               0, NULL,
                                               gtk_cell_renderer_text_new (),
                                               NULL);

  gtk_widget_show (entry->tree_view);
}

/* FIXME: need destroy/finalize */

/* callbacks and helpers */
static void
egg_history_entry_remove_grabs (EggHistoryEntry *entry)
{
  if (GTK_WIDGET_HAS_GRAB (entry->tree_view))
    gtk_grab_remove (entry->tree_view);

  if (GTK_WIDGET_HAS_GRAB (entry->popup_window))
    {
      gtk_grab_remove (entry->popup_window);
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
}

static gboolean
egg_history_entry_real_popup (EggHistoryEntry *entry)
{
  egg_history_entry_popup (entry);

  return TRUE;
}

static void
egg_history_entry_popup (EggHistoryEntry *entry)
{
  gint x, y, width, height;

  if (GTK_WIDGET_MAPPED (entry->popup_window))
    return;

  /* size it */
  gdk_window_get_geometry (GTK_ENTRY (entry)->text_area,
                           NULL, NULL,
                           &width, &height, NULL);
  gdk_window_get_origin (GTK_WIDGET (entry)->window, &x, &y);
  gtk_widget_set_size_request (entry->popup_window, width, -1);

  if (GTK_WIDGET_NO_WINDOW (GTK_WIDGET (entry)))
    {
      x += GTK_WIDGET (entry)->allocation.x;
      y += GTK_WIDGET (entry)->allocation.y;
    }

  gtk_window_move (GTK_WINDOW (entry->popup_window), x, y + height);

  /* popup */
  gtk_widget_show_all (entry->popup_window);

  gtk_widget_grab_focus (entry->popup_window);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (GTK_ENTRY (entry)->popdown_button), TRUE);

  g_signal_emit (entry, entry_signals[POPPED_UP], 0);
}

static void
egg_history_entry_popdown (EggHistoryEntry *entry)
{
  gtk_widget_hide (entry->popup_window);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (GTK_ENTRY (entry)->popdown_button), FALSE);

  g_signal_emit (entry, entry_signals[POPPED_DOWN], 0);
}

static void
egg_history_entry_set_sample (EggHistoryEntry *entry,
                              GtkTreePath     *path)
{
  GtkTreeIter iter;
  gchar *text = NULL;

  gtk_tree_model_get_iter (entry->model, &iter, path);
  gtk_tree_model_get (entry->model, &iter,
                      entry->text_column, &text,
                      -1);

  gtk_entry_set_text (GTK_ENTRY (entry), text);

  gtk_tree_view_set_cursor (GTK_TREE_VIEW (entry->tree_view),
                            path, NULL, FALSE);

  entry->sample_index = gtk_tree_path_get_indices (path)[0];

  g_free (text);

  g_signal_emit (entry, entry_signals[ACTIVE_CHANGED], 0);
}

static gboolean
egg_history_entry_button_press (GtkWidget      *widget,
                                GdkEventButton *event,
                                gpointer        data)
{
  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);
  EggHistoryEntry *entry = EGG_HISTORY_ENTRY (data);

  if (ewidget == entry->tree_view)
    return TRUE;

  if (ewidget != GTK_ENTRY (entry)->popdown_button ||
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GTK_ENTRY (entry)->popdown_button)))
    return FALSE;

  egg_history_entry_popup (entry);

  gtk_grab_add (entry->popup_window);
  gdk_pointer_grab (entry->popup_window->window, TRUE,
                    GDK_BUTTON_PRESS_MASK |
                    GDK_BUTTON_RELEASE_MASK |
                    GDK_POINTER_MOTION_MASK,
                    NULL, NULL, GDK_CURRENT_TIME);

  gtk_grab_add (entry->tree_view);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (GTK_ENTRY (entry)->popdown_button), TRUE);

  entry->popup_in_progress = TRUE;

  return TRUE;
}

static gboolean
egg_history_entry_button_release (GtkWidget      *widget,
                                  GdkEventButton *event,
                                  gpointer        data)
{
  gboolean ret;
  GtkTreePath *path = NULL;

  EggHistoryEntry *entry = EGG_HISTORY_ENTRY (data);

  gboolean popup_in_progress = FALSE;

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (entry->popup_in_progress)
    {
      popup_in_progress = TRUE;
      entry->popup_in_progress = FALSE;
    }

  if (ewidget != entry->tree_view)
    {
      if (ewidget == GTK_ENTRY (entry)->popdown_button &&
          !popup_in_progress &&
          gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (GTK_ENTRY (entry)->popdown_button)))
        {
          egg_history_entry_remove_grabs (entry);
          egg_history_entry_popdown (entry);

          return TRUE;
        }

      /* release outside treeview */
      if (ewidget != GTK_ENTRY (entry)->popdown_button)
        {
          egg_history_entry_remove_grabs (entry);
          egg_history_entry_popdown (entry);

          return TRUE;
        }

      return FALSE;
    }

  /* drop grabs */
  egg_history_entry_remove_grabs (entry);

  /* select something cool */
  ret = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
                                       event->x, event->y,
                                       &path,
                                       NULL, NULL, NULL);

  if (!ret)
    return TRUE; /* clicked outside window? */

  egg_history_entry_set_sample (entry, path);
  egg_history_entry_popdown (entry);

  gtk_tree_path_free (path);

  return TRUE;
}

static gboolean
egg_history_entry_key_press (GtkWidget   *widget,
                             GdkEventKey *event,
                             gpointer     data)
{
  EggHistoryEntry *entry = EGG_HISTORY_ENTRY (data);

  if ((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter ||
       event->keyval == GDK_space || event->keyval == GDK_KP_Space) ||
      event->keyval == GDK_Escape)
    {
      if (event->keyval != GDK_Escape)
        {
          gboolean ret;
          GtkTreeIter iter;
          GtkTreeModel *model = NULL;
          GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (entry->tree_view));

          ret = gtk_tree_selection_get_selected (sel, &model, &iter);
          if (ret)
            {
              GtkTreePath *path;

              path = gtk_tree_model_get_path (model, &iter);
              if (path)
                {
                  egg_history_entry_set_sample (entry, path);
                  gtk_tree_path_free (path);
                }
            }
        }
      else
        {
          GtkTreePath *path;

          path = gtk_tree_path_new ();
          gtk_tree_path_append_index (path, entry->sample_index);

          gtk_tree_view_set_cursor (GTK_TREE_VIEW (entry->tree_view),
                                    path, NULL, FALSE);

          gtk_tree_path_free (path);
        }

      egg_history_entry_remove_grabs (entry);
      egg_history_entry_popdown (entry);

      return TRUE;
    }

  return FALSE;
}

/* public API */
GtkWidget *
egg_history_entry_new (GtkTreeModel *model,
                       gint          text_column)
{
  GList *list;
  GtkWidget *ret;
  EggHistoryEntry *entry;
  GtkTreeViewColumn *col;

  g_return_val_if_fail (GTK_IS_TREE_MODEL (model), NULL);

  ret = GTK_WIDGET (g_object_new (egg_history_entry_get_type (), NULL));

  entry = EGG_HISTORY_ENTRY (ret);

  gtk_tree_view_set_model (GTK_TREE_VIEW (entry->tree_view),
                           model);
  col = gtk_tree_view_get_column (GTK_TREE_VIEW (entry->tree_view), 0);
  list = gtk_tree_view_column_get_cell_renderers (col);
  gtk_tree_view_column_add_attribute (col, list->data,
                                      "text", text_column);
  g_list_free (list);

  /* the treeview refs for us */
  entry->model = model;

  return ret;
}
