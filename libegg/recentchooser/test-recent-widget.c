#include <stdio.h>
#include <gtk/gtk.h>

#include "eggrecentchooser.h"
#include "eggrecentchooserwidget.h"

static const gchar *sort_orders[] = {
  "None",
  "MRU",
  "LRU",
  "Custom",

  NULL,
};

static gint
sort_by_count (EggRecentInfo *info_a,
	       EggRecentInfo *info_b,
	       gpointer       user_data)
{
  gint count_a, count_b;
  gchar *app_name = (gchar *) user_data;

  if (egg_recent_info_has_application (info_a, app_name))
    egg_recent_info_get_application_info (info_a,
		    			  app_name,
					  NULL,
					  &count_a,
					  NULL);
  else
   count_a = 0;

  if (egg_recent_info_has_application (info_b, app_name))
    egg_recent_info_get_application_info (info_b,
		    			  app_name,
					  NULL,
					  &count_b,
					  NULL);
  else
    count_b = 0;

  return count_a < count_b;
}

static void
on_item_activated (EggRecentChooser *chooser,
		   gpointer          user_data)
{
  EggRecentInfo *info;
  gchar *uri, *last_used_by;
  gint last_used_time;
  
  info = egg_recent_chooser_get_current_item (chooser);
  if (!info)
    return;
  
  g_print ("Item activated: `%s'\n",
  	   egg_recent_info_get_display_name (info));
  
  uri = egg_recent_info_get_uri_display (info);
  
  last_used_by = egg_recent_info_last_application (info);
  last_used_time = egg_recent_info_get_age (info);

  g_print ("Item info:\n"
           "* URI: `%s'\n"
           "* Last Used by: `%s'\n"
	   "* Age: %d %s\n",
	   uri,
	   last_used_by,
	   last_used_time,
	   (last_used_time <= 1 ? "day" : "days"));
  
  g_free (uri);
  g_free (last_used_by);
}

static GtkWidget *
create_recent_widget (EggRecentManager  *manager,
		      gint               limit,
		      EggRecentSortType  sort_type)
{
  GtkWidget *recent;
  GtkWidget *label;
  GtkWidget *vbox;
  EggRecentFilter *filter;
  gchar *text;

  vbox = gtk_vbox_new (FALSE, 6);

  text = g_strdup_printf ("Using %s class\n"
		          "(instance: %p, limit: %d, sort: %s)",
			  g_type_name (G_OBJECT_TYPE (manager)),
		  	  manager,
			  limit,
			  sort_orders[sort_type]);
  
  label = gtk_label_new (text);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  g_free (text);
  
  recent = egg_recent_chooser_widget_new_for_manager (manager);
  
  egg_recent_chooser_set_limit (EGG_RECENT_CHOOSER (recent), limit);
  
  egg_recent_chooser_set_show_private (EGG_RECENT_CHOOSER (recent), FALSE);
  egg_recent_chooser_set_show_not_found (EGG_RECENT_CHOOSER (recent), FALSE);
  egg_recent_chooser_set_show_icons (EGG_RECENT_CHOOSER (recent), TRUE);
  
  egg_recent_chooser_set_sort_type (EGG_RECENT_CHOOSER (recent), sort_type);
  if (sort_type == EGG_RECENT_SORT_CUSTOM)
    egg_recent_chooser_set_sort_func (EGG_RECENT_CHOOSER (recent),
		    		      sort_by_count,
				      "populate-recent",
				      NULL);

  g_signal_connect (recent, "item-activated",
  		    G_CALLBACK (on_item_activated), NULL);

  filter = egg_recent_filter_new ();
  egg_recent_filter_set_name (filter, "All resources");
  egg_recent_filter_add_pattern (filter, "*");
  egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (recent), filter);

  /* make this filter the default */
  egg_recent_chooser_set_filter (EGG_RECENT_CHOOSER (recent), filter);
  
  filter = egg_recent_filter_new ();
  egg_recent_filter_set_name (filter, "Today");
  egg_recent_filter_add_age (filter, 1);
  egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (recent), filter);
  
  filter = egg_recent_filter_new ();
  egg_recent_filter_set_name (filter, "Since Yesterday");
  egg_recent_filter_add_age (filter, 2);
  egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (recent), filter);
  
  filter = egg_recent_filter_new ();
  egg_recent_filter_set_name (filter, "Since Last Week");
  egg_recent_filter_add_age (filter, 7);
  egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (recent), filter);

  filter = egg_recent_filter_new ();
  egg_recent_filter_set_name (filter, "Since Last Month");
  egg_recent_filter_add_age (filter, 31);
  egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (recent), filter);

  gtk_box_pack_end (GTK_BOX (vbox), recent, TRUE, TRUE, 0);
  
  return vbox;
}

int
main (int   argc,
      char *argv[])
{
  GtkWidget *window;
  GtkWidget *recent;
  GtkWidget *vbox, *hbox;
  GtkWidget *label;
  GtkWidget *button;
  EggRecentManager *manager;

  gtk_init (&argc, &argv);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_window_set_title (GTK_WINDOW (window), "RecentChooserWidget");
  gtk_container_set_border_width (GTK_CONTAINER (window), 12);

  vbox = gtk_vbox_new (FALSE, 18);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  label = gtk_label_new ("RecentChooser sharing test:\n"
		         "These chooser widgets share the same manager instance");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  
  hbox = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  
  manager = egg_recent_manager_new ();
  
  recent = create_recent_widget (manager, 4, EGG_RECENT_SORT_MRU);
  gtk_box_pack_start (GTK_BOX (hbox), recent, TRUE, TRUE, 0);
  
  recent = create_recent_widget (manager, 10, EGG_RECENT_SORT_CUSTOM);
  gtk_box_pack_start (GTK_BOX (hbox), recent, TRUE, TRUE, 0);
  
  button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_box_pack_end (GTK_BOX (vbox), button, FALSE, FALSE, 0);
  
  gtk_window_resize (GTK_WINDOW (window), 400, 300);

  gtk_widget_show_all (window);

  g_object_unref (manager);

  gtk_main ();

  return 0;
}
