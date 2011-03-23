#include <gtk/gtk.h>
#include "eggcolumnchooserdialog.h"

GtkWidget *tree_view;
                                                                                                           
enum
{
  COL1_STRING,
  COL2_STRING,
  N_COLUMNS
};
                                                                                                           
                                                                                                           
static void
setup_model (GtkTreeModel *model)
{
  GtkTreeIter iter;
  gint i;
                                                                                                           
  for (i = 0; i < 10; i++)
    {
      gchar *str1, *str2;
      GtkTreeIter iter2;
                                                                                                           
      str1 = g_strdup_printf ("Row %d", i);
      str2 = g_strdup_printf ("Col 2 %d", i);
      gtk_tree_store_append (GTK_TREE_STORE (model), &iter, NULL);
      gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
                          COL1_STRING, str1,
                          COL2_STRING, str2,
                          -1);
      gtk_tree_store_append (GTK_TREE_STORE (model), &iter2, &iter);
      gtk_tree_store_set (GTK_TREE_STORE (model), &iter2,
                          COL1_STRING, str1,
                          COL2_STRING, str2,
                          -1);
      gtk_tree_store_append (GTK_TREE_STORE (model), &iter2, &iter);
      gtk_tree_store_set (GTK_TREE_STORE (model), &iter2,
                          COL1_STRING, str1,
                          COL2_STRING, str2,
                          -1);
      g_free (str1);
      g_free (str2);
    }
}
                                                                                                           

int
main (int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
 
  GtkTreeModel *model;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
 
  gtk_init (&argc, &argv);
 
  model = (GtkTreeModel *) gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
  setup_model (model);
  tree_view = gtk_tree_view_new_with_model (model);
 
  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Column 1", cell,
                                                     "text", COL1_STRING,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  column = gtk_tree_view_column_new_with_attributes ("Column 2", cell,
                                                     "text", COL2_STRING,
                                                     NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  column = gtk_tree_view_column_new_with_attributes ("Column 3", cell,
                                                     "text", COL2_STRING,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 500);
  vbox = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
  gtk_container_add (GTK_CONTAINER (window), vbox);
                                                                                                           
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);

  gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
  gtk_widget_show_all (egg_column_chooser_dialog_new (tree_view));

  gtk_widget_show_all (window);
  gtk_main ();

  return 0;
}
