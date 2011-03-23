#include <gtk/gtk.h>
#include "eggtreemultidnd.h"

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *tree_view;
  GtkTreeModel *model;
  GtkTreeIter iter;
  gint i;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));

  for (i = 0; i < 20; i++)
    {
      gchar *str;

      str = g_strdup_printf ("Row to test dragging %d", i);
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, str, -1);
      g_free (str);
    }
  
  tree_view = gtk_tree_view_new_with_model (model);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
						-1, "", gtk_cell_renderer_text_new (),
						"text", 0, NULL);

  gtk_container_add (GTK_CONTAINER (window), tree_view);
  gtk_widget_show_all (window);
  g_signal_connect (G_OBJECT (window), "destroy", gtk_main_quit, NULL);

  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view))), GTK_SELECTION_MULTIPLE);
  egg_tree_multi_drag_add_drag_support (GTK_TREE_VIEW (tree_view));
  
  gtk_main ();
  return 0;
}
