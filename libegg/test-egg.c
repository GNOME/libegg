#include <gtk/gtk.h>
#include "eggtreeviewstate.h"
#include "gtkcellrendererstock.h"

typedef GtkWidget *(* CreateWindowFunc) (void);

const char state_string[] = ""
"<treeview_state>"
"  <treeview headers_visible=\"true\" search_column=\"0\">"
"    <column title=\"Test first\" fixed_width=\"150\" resizable=\"true\" sizing=\"fixed\">"
"      <cell text=\"Sliff sloff\" type=\"GtkCellRendererText\" />"
"    </column>"
"    <column title=\"Test\" reorderable=\"true\" sizing=\"autosize\">"
"      <cell type=\"GtkCellRendererToggle\" expand=\"false\" active=\"model:1\"/>"
"      <cell type=\"GtkCellRendererText\" text=\"model:0\"/>"
"    </column>"
"  </treeview>"
"</treeview_state>";

static GtkWidget *
state_test (void)
{
  GtkWidget *window, *sw, *view;
  GtkListStore *store;
  GtkTreeIter iter;
  GError *error = NULL;

  egg_tree_view_state_add_cell_renderer_type (GTK_TYPE_CELL_RENDERER_TOGGLE);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_BOOLEAN);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      0, "Test string",
		      1, TRUE,
		      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
		      0, "Another string",
		      1, FALSE,
		      -1);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), sw);
  view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

  if (!egg_tree_view_state_apply_from_string (GTK_TREE_VIEW (view), state_string, &error))
    {
      g_print ("error: %s\n", error->message);
    }
  
  gtk_container_add (GTK_CONTAINER (sw), view);
  
  return window;
}

static GtkWidget *
progress_bar_test (void)
{
  GtkWidget *window;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  return window;
}

static GtkWidget *
cellstock_test (void)
{
  GtkWidget *window, *sw, *tv;
  GtkListStore *store;
  GtkTreeViewColumn *column;
  GtkCellRenderer *rend;
  GtkTreeIter iter;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), sw);

  /*** NOTE
   * The GtkCellRendererStock uses GTK_ICON_SIZE_MENU as default. If you
   * like that, there is no need to create a separate column to pass the
   * size
   */

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tv), FALSE);

  column = gtk_tree_view_column_new ();
  rend = gtk_cell_renderer_stock_new ();
  gtk_tree_view_column_pack_start (column, rend, FALSE);
  gtk_tree_view_column_set_attributes (column, rend,
				       "stock_id", 0,
				       "size", 1,
				       NULL);

  rend = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, rend, TRUE);
  gtk_tree_view_column_set_attributes (column, rend,
				       "text", 0,
				       NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

  gtk_container_add (GTK_CONTAINER (sw), tv);

  /* fill the list */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, GTK_STOCK_NEW,
		      1, GTK_ICON_SIZE_LARGE_TOOLBAR, -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, GTK_STOCK_QUIT,
		      1, GTK_ICON_SIZE_BUTTON, -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, GTK_STOCK_CUT,
		      1, GTK_ICON_SIZE_DND, -1);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, GTK_STOCK_COPY,
		      1, GTK_ICON_SIZE_SMALL_TOOLBAR, -1);

  return window;
}

struct
{
  char *name;
  CreateWindowFunc create_func;
} entries[] =
{
  { "Progress Bar Cell", progress_bar_test },
  { "Tree View State", state_test },
  { "GtkCellRendererStock", cellstock_test }
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
  gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);
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
