/**
 * EVIL TEST CASE
 *
 * Do not copy any code from here (:. I won't mention my name here.
 */

#include <gtk/gtk.h>
#include <stdio.h>

#include "eggtreemodelunion.h"


typedef struct {
	GtkWidget *hbox;

	GtkWidget *tv;

	GtkWidget *vbox;
	GtkWidget *add;
	GtkWidget *delete;
} ViewBox;


static void
add_row (GtkWidget *button, gpointer data)
{
	char buf[16];
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (data));

	sprintf (buf, "%d", g_random_int ());

	if (GTK_IS_LIST_STORE (model)) {
		GtkListStore *l = GTK_LIST_STORE (model);

		gtk_list_store_append (l, &iter);
		gtk_list_store_set (l, &iter, 0, buf, -1);
	} else if (GTK_IS_TREE_STORE (model)) {
		GtkTreeIter parent;
		GtkTreeStore *l = GTK_TREE_STORE (model);
		GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (data));

		if (gtk_tree_selection_get_selected (sel, NULL, &parent))
			gtk_tree_store_append (l, &iter, &parent);
		else
			gtk_tree_store_append (l, &iter, NULL);

		gtk_tree_store_set (l, &iter, 0, buf, -1);
	}
}

static void
delete_row (GtkWidget *button, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model = NULL;
	GtkTreeSelection *sel;
	GtkTreeView *tv = GTK_TREE_VIEW (data);

	sel = gtk_tree_view_get_selection (tv);
	if (gtk_tree_selection_get_selected (sel, &model, &iter)) {
		if (GTK_IS_LIST_STORE (model))
			gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		else if (GTK_IS_TREE_STORE (model))
			gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
	}
}

static ViewBox *
create_view_box (GtkTreeModel *model)
{
	ViewBox *vb = g_new0 (ViewBox, 1);

	vb->hbox = gtk_hbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vb->hbox), 5);

	vb->tv = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (vb->tv), FALSE);
	gtk_widget_set_size_request (vb->tv, 80, 100);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (vb->tv),
			                             0, NULL,
						     gtk_cell_renderer_text_new (),
						     "text", 0, NULL);
	gtk_box_pack_start (GTK_BOX (vb->hbox), vb->tv, FALSE, FALSE, 0);

	vb->vbox = gtk_vbox_new (TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vb->hbox), vb->vbox, FALSE, FALSE, 0);

	vb->add = gtk_button_new_from_stock (GTK_STOCK_ADD);
	g_signal_connect (vb->add, "clicked",
			  G_CALLBACK (add_row), vb->tv);
	gtk_box_pack_start (GTK_BOX (vb->vbox), vb->add, TRUE, FALSE, 0);

	vb->delete = gtk_button_new_from_stock (GTK_STOCK_DELETE);
	g_signal_connect (vb->delete, "clicked",
			  G_CALLBACK (delete_row), GTK_TREE_VIEW (vb->tv));
	gtk_box_pack_start (GTK_BOX (vb->vbox), vb->delete, TRUE, FALSE, 0);

	return vb;
}

static GtkTreeModel *
create_model (GtkListStore **l1, GtkTreeStore **l2, GtkListStore **l3)
{
	GtkTreeIter iter, iter2;
	GtkTreeModel *u;

	*l1 = gtk_list_store_new (1, G_TYPE_STRING);
	*l2 = gtk_tree_store_new (1, G_TYPE_STRING);
	*l3 = gtk_list_store_new (1, G_TYPE_STRING);

	u = egg_tree_model_union_new (1, G_TYPE_STRING);

	egg_tree_model_union_append (EGG_TREE_MODEL_UNION (u),
			             GTK_TREE_MODEL (*l1));
	egg_tree_model_union_prepend (EGG_TREE_MODEL_UNION (u),
			              GTK_TREE_MODEL (*l2));
	egg_tree_model_union_insert (EGG_TREE_MODEL_UNION (u),
			             GTK_TREE_MODEL (*l3), 1);

	gtk_list_store_append (*l1, &iter);
	gtk_list_store_set (*l1, &iter, 0, "blaat", -1);

	gtk_list_store_append (*l1, &iter);
	gtk_list_store_set (*l1, &iter, 0, "foobar", -1);

	gtk_tree_store_append (*l2, &iter, NULL);
	gtk_tree_store_set (*l2, &iter, 0, "BLAAT", -1);

	gtk_tree_store_append (*l2, &iter2, &iter);
	gtk_tree_store_set (*l2, &iter2, 0, "ha ha ha", -1);

	gtk_tree_store_append (*l2, &iter, &iter2);
	gtk_tree_store_set (*l2, &iter, 0, "I hate you guys", -1);

	gtk_tree_store_append (*l2, &iter, NULL);
	gtk_tree_store_set (*l2, &iter, 0, "FOOBAR", -1);

	gtk_list_store_append (*l3, &iter);
	gtk_list_store_set (*l3, &iter, 0, "------", -1);


	return u;
}

int
main (int argc, char **argv)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *window;
	GtkWidget *sw;
	GtkWidget *treeview;
	GtkWidget *mainbox;
	GtkTreeModel *model;
	GtkListStore *l1, *l3;
	GtkTreeStore *l2;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Union test");
	g_signal_connect (window, "delete_event",
			  G_CALLBACK (gtk_main_quit), window);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (window), hbox);

	vbox = gtk_vbox_new (TRUE, 5);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	model = create_model (&l1, &l2, &l3);

	/* leakety leak */
	gtk_box_pack_start (GTK_BOX (vbox), create_view_box (GTK_TREE_MODEL (l1))->hbox,
			    TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), create_view_box (GTK_TREE_MODEL (l2))->hbox,
			    TRUE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), create_view_box (GTK_TREE_MODEL (l3))->hbox,
			    TRUE, FALSE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
			                GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_set_border_width (GTK_CONTAINER (sw), 5);
	gtk_box_pack_start (GTK_BOX (hbox), sw, TRUE, TRUE, 0);

	treeview = gtk_tree_view_new_with_model (model);
	gtk_widget_set_size_request (treeview, 90, -1);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
			                             0, NULL,
						     gtk_cell_renderer_text_new (),
						     "text", 0, NULL);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
	gtk_container_add (GTK_CONTAINER (sw), treeview);

	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}
