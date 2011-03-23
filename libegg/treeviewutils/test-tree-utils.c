#include <gtk/gtk.h>
#include <stdio.h>
#include "eggtreeviewstate.h"
#include "eggcellrendererkeys.h"

#if 0
#include "eggnodestate.h"
#endif

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

#if 0
static void
node_state_save (GtkWidget *button, GtkTreeView *tree_view)
{
	gboolean ret;

	ret = egg_node_state_save_to_file (tree_view, "treeviewnodestate.xml");
}

static void
node_state_restore (GtkWidget *button, GtkTreeView *tree_view)
{
	gboolean ret;

	ret = egg_node_state_restore_from_file (tree_view,
						"treeviewnodestate.xml");
}

static GtkWidget *
node_state_test (void)
{
	GtkWidget *window, *sw, *tv, *hbox, *vbox, *button;
	GtkTreeStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *rend;
	gint i;

	/* create window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

	store = gtk_tree_store_new (1, G_TYPE_STRING);
	tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

	column = gtk_tree_view_column_new ();
	rend = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, rend,
					 TRUE);
	gtk_tree_view_column_set_attributes (column, rend,
					     "text", 0,
					     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

	gtk_container_add (GTK_CONTAINER (sw), tv);

	hbox = gtk_hbox_new (TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	button = gtk_button_new_with_label ("Save");
	g_signal_connect (button, "clicked", G_CALLBACK (node_state_save), tv);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

	button = gtk_button_new_with_label ("Restore");
	g_signal_connect (button, "clicked", G_CALLBACK (node_state_restore),
			  tv);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

	/* fill the model */
	for (i = 0; i < 6; i++) {
		GtkTreeIter iter;
		gchar text[16];
		gint j;

		sprintf (text, "%d", i);

		gtk_tree_store_append (store, &iter, NULL);
		gtk_tree_store_set (store, &iter, 0, text, -1);

		for (j = 0; j < 3; j++) {
			gint k;
			GtkTreeIter iter2;

			sprintf (text, "%d:%d", i, j);

			gtk_tree_store_append (store, &iter2, &iter);
			gtk_tree_store_set (store, &iter2, 0, text, -1);

			for (k = 0; k < 2; k++) {
				GtkTreeIter iter3;

				sprintf (text, "%d:%d:%d", i, j, k);

				gtk_tree_store_append (store, &iter3, &iter2);
				gtk_tree_store_set (store, &iter3,
						    0, text, -1);
			}
		}
	}

	/* done */

	return window;
}

#endif


static void
accel_edited_callback (GtkCellRendererText *cell,
                       const char          *path_string,
                       guint                keyval,
                       GdkModifierType      mask,
                       guint                hardware_keycode,
                       gpointer             data)
{
  GtkTreeModel *model = (GtkTreeModel *)data;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  GtkTreeIter iter;

  gtk_tree_model_get_iter (model, &iter, path);

  g_print ("%u %d %u\n", keyval, mask, hardware_keycode);
  
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      0, (gint)mask,
		      1, keyval,
		      -1);
  gtk_tree_path_free (path);
}

static GtkWidget *
key_test (void)
{
	GtkWidget *window, *sw, *tv;
	GtkListStore *store;
	GtkTreeViewColumn *column;
	GtkCellRenderer *rend;
	gint i;

	/* create window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (window), sw);

	store = gtk_list_store_new (2, G_TYPE_INT, G_TYPE_UINT);
	tv = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_container_add (GTK_CONTAINER (sw), tv);
	column = gtk_tree_view_column_new ();
	rend = egg_cell_renderer_keys_new ();
	g_object_set (G_OBJECT (rend), "accel_mode", EGG_CELL_RENDERER_KEYS_MODE_X, NULL);
	g_signal_connect (G_OBJECT (rend),
			  "keys_edited",
			  G_CALLBACK (accel_edited_callback),
			  store);

	g_object_set (G_OBJECT (rend), "editable", TRUE, NULL);
	gtk_tree_view_column_pack_start (column, rend,
					 TRUE);
	gtk_tree_view_column_set_attributes (column, rend,
					     "accel_mask", 0,
					     "accel_key", 1,
					     NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

	for (i = 0; i < 10; i++) {
		GtkTreeIter iter;

		gtk_list_store_append (store, &iter);
	}

	/* done */

	return window;
}

struct
{
  char *name;
  CreateWindowFunc create_func;
} entries[] =
{
  { "EggCellRendererKeys", key_test },
#if 0
  { "EggNodeState", node_state_test },
#endif
  { "Progress Bar Cell", progress_bar_test },
  { "Tree View State", state_test },
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
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  gtk_container_set_border_width (GTK_CONTAINER (sw), 8);
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
