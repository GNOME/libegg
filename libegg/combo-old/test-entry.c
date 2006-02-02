#include <gtk/gtk.h>

#include "eggentry.h"

static GtkListStore *
create_tree_model ()
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE (gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING));

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "klaas", 1, GTK_STOCK_ADD, -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "sliff", 1, GTK_STOCK_DELETE, -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "sloff", 1, GTK_STOCK_PASTE, -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "bla die bla", 1, GTK_STOCK_CLEAR, -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "blaat", 1, GTK_STOCK_OPEN, -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "dum de dum", 1, GTK_STOCK_PREFERENCES, -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "chippas", 1, GTK_STOCK_STOP, -1);

	return store;
}

/* ATTENTION KIDS!! DO NOT TRY THIS AT HOME.
 * Please wait for the official function to appear in gtk+ 2.4.
 * Thanks, your mum.
 */

static void
reorder_hack (GtkTreeViewColumn *col,
              GtkCellRenderer   *cell,
	      gint               position)
{
	/* this is a stripped down version of gtk_tree_view_reorder_cell
	 * which will hopefully appear in gtk+ 2.4. Note that we can not
	 * do as many checks as internally in GtkTreeViewColumn.
	 */
	GList *link;
	struct CellInfo {
		GtkCellRenderer *cell;
		/* we don't need the other stuff in the struct */
	} *info;

	g_return_if_fail (GTK_IS_TREE_VIEW_COLUMN (col));
	g_return_if_fail (GTK_IS_CELL_RENDERER (cell));
	/* WARNING: check to check if the cell has really been packed in
	 * this column left out.
	 */
	g_return_if_fail (position >= 0);

	for (link = col->cell_list; link; link = link->next) {
		/* ok, the cell field in the cell info structs is
		 * (right now) the first field in the struct
		 */
		info = link->data;

		if (info->cell == cell)
			break;
	}
	g_return_if_fail (link != NULL);

	col->cell_list = g_list_remove_link (col->cell_list, link);
	col->cell_list = g_list_insert (col->cell_list, info, position);
}

int
main (int argc, char **argv)
{
	GtkWidget *window, *entry;
	GtkCellRenderer *rend;
	GtkTreeViewColumn *col;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (window, "delete_event", gtk_main_quit, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);

	entry = egg_entry_new ();
	egg_entry_enable_completion (EGG_ENTRY (entry),
			             create_tree_model (),
				     0, -1,
				     NULL, NULL, NULL);
	col = egg_entry_completion_get_column (EGG_ENTRY (entry));

	egg_entry_history_set_max (EGG_ENTRY (entry), 20);

	rend = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (col, rend, FALSE);
	/* API addition for gtk+ 2.4 -- disabled for now
	 * gtk_tree_view_column_reorder_cell (col, rend, 0);
	 */
	reorder_hack (col, rend, 0);
	gtk_tree_view_column_set_attributes (col, rend,
			                     "stock_id", 1,
					     NULL);

	gtk_container_add (GTK_CONTAINER (window), entry);

	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}
