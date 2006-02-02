#include <gtk/gtk.h>
#include "eggcellview.h"
#include "eggcellviewmenuitem.h"
#include "eggcombobox.h"
#include "eggcomboboxentry.h"

#include <string.h>
#include <stdio.h>

/**
 * oh yes, this test app surely has a lot of ugly code
 */

/* grid combo demo */
static GdkPixbuf *
create_color_pixbuf (const char *color)
{
	GdkPixbuf *pixbuf;
	GdkColor col;

	int x;
	int num;
	int rowstride;
	guchar *pixels, *p;

	if (!gdk_color_parse (color, &col))
		return NULL;

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB,
				 FALSE, 8,
				 16, 16);

	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
	p = pixels = gdk_pixbuf_get_pixels (pixbuf);

	num = gdk_pixbuf_get_width (pixbuf) *
		gdk_pixbuf_get_height (pixbuf);

	for (x = 0; x < num; x++) {
		p[0] = col.red / 65535 * 255;
		p[1] = col.green / 65535 * 255;
		p[2] = col.blue / 65535 * 255;
		p += 3;
	}

	return pixbuf;
}

static GtkWidget *
create_combo_box_grid_demo ()
{
	GtkWidget *combo;
	GtkTreeIter iter;
	GtkCellRenderer *cell = gtk_cell_renderer_pixbuf_new ();
	GtkListStore *store;

	store = gtk_list_store_new (1, GDK_TYPE_PIXBUF);

	/* workaround treeview bug (leaks like mad too) */
	gtk_list_store_append (store, &iter);
	gtk_tree_row_reference_new (GTK_TREE_MODEL (store),
			            gtk_tree_path_new_from_indices (0, -1));
	gtk_tree_row_reference_new (GTK_TREE_MODEL (store),
			            gtk_tree_path_new_from_indices (0, -1));
	gtk_list_store_remove (store, &iter);

	combo = egg_combo_box_new (GTK_TREE_MODEL (store));
	egg_combo_box_pack_start (EGG_COMBO_BOX (combo),
				  cell, TRUE);
	egg_combo_box_set_attributes (EGG_COMBO_BOX (combo),
				      cell, "pixbuf", 0, NULL);
	egg_combo_box_set_wrap_width (EGG_COMBO_BOX (combo),
				      3);

	/* first row */
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("red"),
			    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("green"),
			    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("blue"),
			    -1);

	/* second row */
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("yellow"),
			    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("black"),
			    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("white"),
			    -1);

	/* third row */
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("gray"),
			    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("snow"),
			    -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, create_color_pixbuf ("magenta"),
			    -1);

	egg_combo_box_set_active (EGG_COMBO_BOX (combo), 0);

	return combo;
}

/* blaat */
static GtkTreeModel *
create_blaat ()
{
	GdkPixbuf *pixbuf;
	GtkWidget *cellview;
	GtkTreeIter iter;
	GtkListStore *store;

	cellview = egg_cell_view_new ();

	store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	/* workaround treeview bug (leaks like mad too) */
	gtk_list_store_append (store, &iter);
	gtk_tree_row_reference_new (GTK_TREE_MODEL (store),
			            gtk_tree_path_new_from_indices (0, -1));
	gtk_tree_row_reference_new (GTK_TREE_MODEL (store),
			            gtk_tree_path_new_from_indices (0, -1));
	gtk_list_store_remove (store, &iter);

	pixbuf = gtk_widget_render_icon (cellview, GTK_STOCK_DIALOG_WARNING,
					 GTK_ICON_SIZE_BUTTON, NULL);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, pixbuf,
			    1, "gtk-stock-dialog-warning",
			    -1);

	pixbuf = gtk_widget_render_icon (cellview, GTK_STOCK_STOP,
					 GTK_ICON_SIZE_BUTTON, NULL);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, pixbuf,
			    1, "gtk-stock-stop",
			    -1);

	pixbuf = gtk_widget_render_icon (cellview, GTK_STOCK_NEW,
					 GTK_ICON_SIZE_BUTTON, NULL);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, pixbuf,
			    1, "gtk-stock-new",
			    -1);

	pixbuf = gtk_widget_render_icon (cellview, GTK_STOCK_CLEAR,
					 GTK_ICON_SIZE_BUTTON, NULL);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter,
			    0, pixbuf,
			    1, "gtk-stock-clear",
			    -1);

	gtk_widget_destroy (cellview);

	return GTK_TREE_MODEL (store);
}

static GtkTreeModel *
create_text_store ()
{
	GtkTreeIter iter;
	GtkListStore *store;

	store = GTK_LIST_STORE (gtk_list_store_new (1, G_TYPE_STRING));

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "dum de dum", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "la la la", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "la la la dum de dum la la la la la la boom de da la la", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "bloop", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "bleep", -1);

	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, "hoer", -1);

	return GTK_TREE_MODEL (store);
}

/* this is a hack, because this function is evil and only for
 * testing and stuff (and it won't be in the final API)
 */
void egg_combo_box_switch (EggComboBox *combo_box);

int
main (int argc, char **argv)
{
	GtkWidget *window, *cellview, *mainbox;
	GtkWidget *combobox, *comboboxtext, *comboboxgrid;
	GtkWidget *tmp, *boom;
	GtkCellRenderer *renderer;
	GdkPixbuf *pixbuf;
	GValue value = {0, };

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	g_signal_connect (window, "delete_event", gtk_main_quit, NULL);

	mainbox = gtk_vbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (window), mainbox);

	/* EGG CELL VIEW */
	tmp = gtk_frame_new ("EggCellView");
	gtk_box_pack_start (GTK_BOX (mainbox), tmp, FALSE, FALSE, 0);

	boom = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (boom), 5);
	gtk_container_add (GTK_CONTAINER (tmp), boom);

	cellview = egg_cell_view_new ();
	renderer = gtk_cell_renderer_pixbuf_new ();
	pixbuf = gtk_widget_render_icon (cellview, GTK_STOCK_DIALOG_WARNING,
					 GTK_ICON_SIZE_BUTTON, NULL);

	egg_cell_view_pack_start (EGG_CELL_VIEW (cellview),
				  renderer,
				  FALSE);
	g_value_init (&value, GDK_TYPE_PIXBUF);
	g_value_set_instance (&value, pixbuf);
	egg_cell_view_set_values (EGG_CELL_VIEW (cellview),
				  renderer,
				  "pixbuf", &value,
				  NULL);
	g_value_unset (&value);

	renderer = gtk_cell_renderer_text_new ();
	egg_cell_view_pack_start (EGG_CELL_VIEW (cellview),
				  renderer,
				  TRUE);
	g_value_init (&value, G_TYPE_STRING);
	g_value_set_string (&value, "la la la");
	egg_cell_view_set_values (EGG_CELL_VIEW (cellview),
				  renderer,
				  "text", &value,
				  NULL);
	g_value_unset (&value);
	gtk_container_add (GTK_CONTAINER (boom), cellview);

	/* EggComboBox */
	tmp = gtk_frame_new ("EggComboBox");
	gtk_box_pack_start (GTK_BOX (mainbox), tmp, FALSE, FALSE, 0);

	boom = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (boom), 5);
	gtk_container_add (GTK_CONTAINER (tmp), boom);

	combobox = egg_combo_box_new (create_blaat ());
	gtk_container_add (GTK_CONTAINER (boom), combobox);


	tmp = gtk_button_new_with_label ("Change style");
	g_signal_connect_swapped (tmp, "clicked", G_CALLBACK (egg_combo_box_switch), combobox);
	gtk_container_add (GTK_CONTAINER (boom), tmp);


	renderer = gtk_cell_renderer_pixbuf_new ();
	egg_combo_box_pack_start (EGG_COMBO_BOX (combobox),
				  renderer,
				  FALSE);
	egg_combo_box_set_attributes (EGG_COMBO_BOX (combobox), renderer, "pixbuf", 0, NULL);

	renderer = gtk_cell_renderer_text_new ();
	egg_combo_box_pack_start (EGG_COMBO_BOX (combobox),
				  renderer,
				  TRUE);
	egg_combo_box_set_attributes (EGG_COMBO_BOX (combobox), renderer, "text", 1, NULL);

	egg_combo_box_set_active (EGG_COMBO_BOX (combobox), 1);

	/* EggComboBox (grid mode) */
	tmp = gtk_frame_new ("EggComboBox (grid mode)");
	gtk_box_pack_start (GTK_BOX (mainbox), tmp, FALSE, FALSE, 0);

	boom = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (boom), 5);
	gtk_container_add (GTK_CONTAINER (tmp), boom);

	comboboxgrid = create_combo_box_grid_demo ();
	gtk_box_pack_start (GTK_BOX (boom), comboboxgrid, FALSE, FALSE, 0);

	/* EggComboBoxEntry */
	tmp = gtk_frame_new ("EggComboBoxEntry");
	gtk_box_pack_start (GTK_BOX (mainbox), tmp, FALSE, FALSE, 0);

	boom = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (boom), 5);
	gtk_container_add (GTK_CONTAINER (tmp), boom);

	comboboxtext = egg_combo_box_entry_new (create_text_store (), 0);
	gtk_container_add (GTK_CONTAINER (boom), comboboxtext);

	/* done */
	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}
