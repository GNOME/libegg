#include <gtk/gtk.h>
#include "eggdruid.h"
#include "eggdruidpagestandard.h"


static GtkWidget *
get_test_page (void)
{
  GtkWidget *retval;
  GtkWidget *button_hbox;
  GtkWidget *hbox, *hbox2;
  GtkWidget *button;
  GtkSizeGroup *size_group;


  retval = gtk_vbox_new (FALSE, 9);
  button_hbox = gtk_hbox_new (FALSE, 0);
  hbox = gtk_hbox_new (FALSE, 36);
  hbox2 = gtk_hbox_new (FALSE, 9);
  size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  /* Help */
  button = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (button_hbox), button, FALSE, FALSE, 0);

  /* Cancel */
  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

  /* Prev */
  button = gtk_button_new_from_stock (GTK_STOCK_GO_BACK);
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

  /* Next */
  button = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
  gtk_size_group_add_widget (size_group, button);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);

  /* Tie it together */
  gtk_box_pack_end (GTK_BOX (button_hbox), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (retval), button_hbox, FALSE, FALSE, 0);
  gtk_box_pack_end (GTK_BOX (retval), gtk_hseparator_new (), FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (retval), 9);
  return retval;
}

int
main (gint argc, gchar **argv)
{
  GtkWidget *window, *druid, *box;
  GdkPixbuf *sidebar_pixbuf, *header_pixbuf;
  EggDruidPage *page;
  GdkColor color;
  
  gtk_init (&argc, &argv);
  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 300);
  
  druid = egg_druid_new ();
  sidebar_pixbuf = gdk_pixbuf_new_from_file ("sidebar.png", NULL);
  egg_druid_set_sidebar_image (EGG_DRUID (druid), sidebar_pixbuf);
  egg_druid_set_sidebar_image_alignment (EGG_DRUID (druid), 1.0);

  gdk_color_parse ("#ce0000", &color);
  egg_druid_set_sidebar_color (EGG_DRUID (druid), &color);

  header_pixbuf = gdk_pixbuf_new_from_file ("logo.png", NULL);
  egg_druid_set_header_image (EGG_DRUID (druid), header_pixbuf);
  gdk_color_parse ("#ce0000", &color);
  egg_druid_set_header_color (EGG_DRUID (druid), &color);

  gdk_color_parse ("white", &color);
  egg_druid_set_header_text_color (EGG_DRUID (druid), &color);
  
  gtk_container_add (GTK_CONTAINER (window), druid);

  page = egg_druid_page_standard_new ();
  egg_druid_page_standard_set_title (EGG_DRUID_PAGE_STANDARD (page), "This is the title");
  
  box = get_test_page ();
  gtk_container_add (GTK_CONTAINER (page), box);
  gtk_widget_show_all (GTK_WIDGET (page));

  egg_druid_append_page (EGG_DRUID (druid), page);
  
  gtk_widget_show (druid);
  gtk_widget_show_all (window);
    
  gtk_main ();
  
  return 0;
}
