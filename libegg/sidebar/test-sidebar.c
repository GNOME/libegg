
#include <gtk/gtk.h>
#include "eggsidebar.h"

int
main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *sidebar;
	EggSidebarButton *button;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (G_OBJECT (window), "destroy", gtk_main_quit, NULL);

	sidebar = egg_sidebar_new ();

	button = egg_sidebar_button_new ("button1");
	egg_sidebar_button_set (button, GTK_STOCK_COPY, "_Copy Time", TRUE);
	egg_sidebar_append (EGG_SIDEBAR (sidebar), button);

	button = egg_sidebar_button_new ("button2");
	egg_sidebar_button_set (button, GTK_STOCK_DELETE, "_Fun Fun Time", FALSE);
	egg_sidebar_append (EGG_SIDEBAR (sidebar), button);

	gtk_container_add (GTK_CONTAINER (window), sidebar);
	gtk_widget_show_all (window);

	gtk_main ();
	
	return 0;
}
