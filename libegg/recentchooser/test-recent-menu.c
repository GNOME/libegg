#include <gtk/gtk.h>
#include <glade/glade.h>
#include <stdio.h>
#include <stdlib.h>

#include "eggrecentchooser.h"
#include "eggrecentchoosermenu.h"
#include "eggrecentchooserdialog.h"

/* this callback is fired each time a user clicks on a menu item */
static void
on_item_activated (EggRecentChooser *chooser,
		   gpointer          user_data)
{
  GtkWidget *window = GTK_WIDGET (user_data);
  GtkWidget *label;
  EggRecentInfo *info;
  const gchar *uri;
  const gchar *mime_type;
  gchar *last_application;
  gchar *exec_cmd;
  gchar *text;
  guint count;
  GString *string;

  info = egg_recent_chooser_get_current_item (chooser);
  if (!info)
    {
      g_warning ("Unable to retrieve the current_item, aborting...\n");
      return;
    }
  
  label = g_object_get_data (G_OBJECT (window), "display-label");
  
  string = g_string_new ("Selected item:\n");
  
  uri = egg_recent_info_get_uri (info);
  g_string_append_printf (string, "\tURI: '%s'\n", uri);
  
  mime_type = egg_recent_info_get_mime_type (info);
  g_string_append_printf (string, "\tMIME Type: '%s'\n", mime_type);
  
  last_application = egg_recent_info_last_application (info);
  g_string_append_printf (string, "\tLast used by: '%s'\n", last_application);

  egg_recent_info_get_application_info (info, last_application,
		  			&exec_cmd,
					&count,
					NULL);
  g_string_append_printf (string, "\t  Command line: '%s'\n"
				  "\t  Registered '%d' times",
				  exec_cmd,
				  count);
  g_free (exec_cmd);
  g_free (last_application);
  
  egg_recent_info_unref (info);
  
  text = g_string_free (string, FALSE);
  
  gtk_label_set_text (GTK_LABEL (label), text);
  
  g_free (text);
}

static void
on_more_activate (GtkWidget *menu_item,
		  gpointer   user_data)
{

  GtkWidget *chooser = NULL;
  GtkWidget *window;
  GtkWidget *label;
  EggRecentManager *manager;
  EggRecentFilter *filter;
  
  window = GTK_WIDGET (user_data);

  manager = g_object_get_data (G_OBJECT (window), "recent-manager");
  chooser = g_object_get_data (G_OBJECT (window), "document-history");
  label = g_object_get_data (G_OBJECT (window), "display-label");
  
  if (!chooser)
    {
      chooser = egg_recent_chooser_dialog_new_for_manager ("Document History",
		      					   GTK_WINDOW (window),
							   manager,
							   GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL,
							   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							   NULL);
      
      /* show the most recently used items first */  
      egg_recent_chooser_set_sort_type (EGG_RECENT_CHOOSER (chooser),
		  		        EGG_RECENT_SORT_MRU);

      egg_recent_chooser_set_show_icons (EGG_RECENT_CHOOSER (chooser), TRUE);
      
      /* we want all the items */
      egg_recent_chooser_set_limit (EGG_RECENT_CHOOSER (chooser), -1);
      
      /* create and add the filters */
      
      /* Pattern based filter; this filter allows everything to pass */
      filter = egg_recent_filter_new ();
      egg_recent_filter_set_name (filter, "All resources");
      egg_recent_filter_add_pattern (filter, "*");
      egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (chooser), filter);
      /* this is also our default filter */
      egg_recent_chooser_set_filter (EGG_RECENT_CHOOSER (chooser), filter);
      
      /* Age based filter */
      filter = egg_recent_filter_new ();
      egg_recent_filter_set_name (filter, "Today's resources");
      egg_recent_filter_add_age (filter, 1);
      egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (chooser), filter);
      
      /* MIME type based filter */
      filter = egg_recent_filter_new ();
      egg_recent_filter_set_name (filter, "All PDF files");
      egg_recent_filter_add_mime_type (filter, "application/pdf");
      egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (chooser), filter);

      /* Application based filter */
      filter = egg_recent_filter_new ();
      egg_recent_filter_set_name (filter, "All files opened by Gedit");
      egg_recent_filter_add_application (filter, "Gedit");
      egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (chooser), filter);

      /* Group based filter */
      filter = egg_recent_filter_new ();
      egg_recent_filter_set_name (filter, "All music files");
      egg_recent_filter_add_group (filter, "Audio");
      egg_recent_chooser_add_filter (EGG_RECENT_CHOOSER (chooser), filter);
      
      /* let the dialog hang around */
      g_object_set_data_full (G_OBJECT (window), "document-history",
		              chooser,
			      (GDestroyNotify) gtk_widget_destroy);
    }

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
      /* use the "item_activated" callback */
      on_item_activated (EGG_RECENT_CHOOSER (chooser), window);      
    }

  gtk_widget_hide (chooser);
}

static void
on_clear_activate (GtkWidget *menu_item,
		   gpointer   user_data)
{
  g_message ("in on_clear_activate");
}

enum {
  TEXT_URI_LIST = 0
};

static void
on_drag_data_received (GtkWidget        *widget,
		       GdkDragContext   *context,
		       gint              x,
		       gint              y,
		       GtkSelectionData *data,
		       guint             info,
		       guint             time_,
		       gpointer          user_data)
{
  if (info == TEXT_URI_LIST)
    {
      gchar **uris = gtk_selection_data_get_uris (data);
      gsize i;

      for (i = 0; i < g_strv_length (uris); i++)
        {
          g_print ("Received URI := '%s'\n", uris[i]);
	}

      g_strfreev (uris);

      gtk_drag_finish (context, TRUE, FALSE, time_);
    }
  else
    gtk_drag_finish (context, FALSE, FALSE, time_);
}

static const GtkTargetEntry entries[] = {
  { "text/uri-list", 0, TEXT_URI_LIST },
};

int
main (int argc, char *argv[])
{
  GtkWidget *main_window;
  GtkWidget *open_recent;
  GtkWidget *recent_menu;
  GtkWidget *menu_item;
  GtkWidget *image;
  GtkWidget *label;
  GladeXML *xml;
  EggRecentManager *manager;
  
  gtk_init (&argc, &argv);

  manager = egg_recent_manager_new ();
  
  xml = glade_xml_new ("test-recent-menu.glade", "main_window", NULL);
  main_window = glade_xml_get_widget (xml, "main_window");
  open_recent = glade_xml_get_widget (xml, "open_recent1");
  label = glade_xml_get_widget (xml, "label1");

  g_object_set_data (G_OBJECT (main_window), "recent-manager", manager);

  gtk_drag_dest_set (label,
		     GTK_DEST_DEFAULT_ALL,
		     entries,
		     G_N_ELEMENTS (entries),
		     GDK_ACTION_COPY);
  
  g_signal_connect (main_window, "drag-data-received",
		    G_CALLBACK (on_drag_data_received), manager);
  g_signal_connect (main_window, "destroy",
  		    G_CALLBACK (gtk_main_quit), NULL);
  
  gtk_label_set_text (GTK_LABEL (label), "No item selected");
  g_object_set_data (G_OBJECT (main_window), "display-label", label);

  /* the recently used resources sub-menu */
  recent_menu = egg_recent_chooser_menu_new_for_manager (manager);
  egg_recent_chooser_set_limit (EGG_RECENT_CHOOSER (recent_menu), 4);
  egg_recent_chooser_set_sort_type (EGG_RECENT_CHOOSER (recent_menu),
		                    EGG_RECENT_SORT_MRU);
  egg_recent_chooser_set_show_tips (EGG_RECENT_CHOOSER (recent_menu), TRUE);
  egg_recent_chooser_set_show_icons (EGG_RECENT_CHOOSER (recent_menu), TRUE);
  g_signal_connect (recent_menu, "item-activated",
  		    G_CALLBACK (on_item_activated), main_window);

  /* a EggRecentChooserMenu is a GtkMenu, so you can append items to it */
  menu_item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (recent_menu), menu_item);
  gtk_widget_show (menu_item);

  /* this menu item launches the EggRecentChooserDialog widget */
  image = gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
  menu_item = gtk_image_menu_item_new_with_mnemonic ("Document _History");
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
  g_signal_connect (menu_item, "activate",
		    G_CALLBACK (on_more_activate), main_window);
  gtk_menu_shell_append (GTK_MENU_SHELL (recent_menu), menu_item);
  gtk_widget_show (menu_item);
  
  image = gtk_image_new_from_stock (GTK_STOCK_CLEAR, GTK_ICON_SIZE_MENU);
  menu_item = gtk_image_menu_item_new_with_mnemonic ("_Clear Recent Documents");
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item), image);
  g_signal_connect (menu_item, "activate",
  		    G_CALLBACK (on_clear_activate), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (recent_menu), menu_item);
  gtk_widget_show (menu_item);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (open_recent), recent_menu);
  
  gtk_widget_show_all (main_window);
  
  gtk_main ();
  
  return 0;
}
