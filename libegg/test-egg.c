#include <gtk/gtk.h>

typedef GtkWidget *(* CreateWindowFunc) (void);

static GtkWidget *
progress_bar_test (void)
{
  GtkWidget *window;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  return window;
}

struct
{
  char *name;
  CreateWindowFunc create_func;
} entries[] =
{
  { "Progress Bar Cell", progress_bar_test },
};

static void
row_activated (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  CreateWindowFunc func;
  GtkWidget *window;
  char *str;
  
  model = gtk_tree_view_get_model (tree_view);
  
  gtk_tree_model_get_iter (model, &iter, path);

  gtk_tree_model_get (model, &iter,
		      0, &str,
		      1, &func,
		      -1);


  window = (*func) ();
  gtk_window_set_title (GTK_WINDOW (window), str);
  g_free (str);
  
  g_signal_connect (window, "delete_event",
		    G_CALLBACK (gtk_widget_destroy), NULL);
  gtk_widget_show_all (window);
}

gint
main (gint argc, gchar **argv)
{
  GtkWidget *dialog, *sw, *tree_view;
  GtkListStore *model;
  int i;
  GtkTreeIter iter;
  
  gtk_init (&argc, &argv);

  dialog = gtk_dialog_new_with_buttons ("Egg test",
					NULL,
					0,
					GTK_STOCK_CLOSE, GTK_RESPONSE_OK,
					NULL);
  gtk_window_set_default_size (GTK_WINDOW (dialog), 200, 200);
  g_signal_connect (dialog, "response",
		    G_CALLBACK (gtk_main_quit), NULL);
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_border_width (GTK_CONTAINER (sw), 8);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), sw, TRUE, TRUE, 0);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);

  for (i = 0; i < G_N_ELEMENTS (entries); i++)
    {
      gtk_list_store_append (model,
			     &iter);
      gtk_list_store_set (model,
			  &iter,
			  0, entries[i].name,
			  1, entries[i].create_func,
			  -1);
    }
  
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
  g_signal_connect (tree_view, "row_activated",
		    G_CALLBACK (row_activated), NULL);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view), -1,
					       "Tests", gtk_cell_renderer_text_new (),
					       "text", 0,
					       NULL);
  gtk_container_add (GTK_CONTAINER (sw), tree_view);
  
  gtk_widget_show_all (dialog);

  gtk_main ();

  return 0;
}
