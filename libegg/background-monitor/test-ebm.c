/* 
 * (c) Copyright 2002 Ian McKellar <yakk@yakk.net>
 */

#include "egg-background-monitor.h"
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

typedef struct {
	EggBackgroundMonitor *monitor;
	GtkWidget *image;
	GtkWidget *image2;
} Foo;


static gboolean 
update_widget_background (gpointer user_data)
{
	GdkPixbuf *pixbuf;
	Foo *foo = user_data;

	pixbuf = egg_background_monitor_get_widget_background (foo->monitor,
			foo->image);
	gtk_image_set_from_pixbuf (GTK_IMAGE (foo->image), pixbuf);
	/* FIXME: unref pixbuf? */

	pixbuf = egg_background_monitor_get_widget_background (foo->monitor,
			foo->image2);
	gtk_image_set_from_pixbuf (GTK_IMAGE (foo->image2), pixbuf);
	/* FIXME: unref pixbuf? */

	return FALSE;
}

static void
background_changed (EggBackgroundMonitor *monitor, gpointer user_data) {
	Foo *foo = user_data;

	update_widget_background (foo);
}

static gboolean
configure_event (GtkWidget *window, GdkEventConfigure *event, Foo *foo) 
{
	background_changed (foo->monitor, foo);

	return TRUE;
}

int
main (int argc, char **argv) 
{
	Foo foo;
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *button;

	gtk_init (&argc, &argv);

	foo.monitor = egg_background_monitor_new ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 100, 100);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	foo.image = gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_DIALOG);
	gtk_container_add (GTK_CONTAINER (vbox), foo.image);
	/*gtk_widget_set_usize (foo.image, 20, 20);*/
	gtk_widget_show (foo.image);

	button = gtk_button_new_with_label ("hello nasty");
	gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (vbox), button);

	foo.image2 = gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_DIALOG);
	gtk_container_add (GTK_CONTAINER (vbox), foo.image2);
	/*gtk_widget_set_usize (foo.image2, 20, 20);*/
	gtk_widget_show (foo.image2);

	gtk_widget_show (vbox);
	gtk_widget_show (window);

	g_signal_connect (G_OBJECT (foo.monitor), "changed",
			(GCallback) background_changed, &foo);
	g_signal_connect (G_OBJECT (window), "configure-event",
			(GCallback) configure_event, &foo);

	gtk_main ();

	return 0;
}
