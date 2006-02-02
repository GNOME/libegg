/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   James Willcox <jwillcox@cs.indiana.edu>
 */
#undef GTK_DISABLE_DEPRECATED
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <libgnomevfs/gnome-vfs.h>
#include <glade/glade.h>
#include "egg-recent.h"

typedef struct _MyApp MyApp;
struct _MyApp {
	GtkUIManager *manager;
	EggRecentModel *model;
	EggRecentViewUIManager *view;
	EggRecentModel *global_model;
	EggRecentViewUIManager *global_view;
	GtkWidget *filesel;
};

MyApp *app;


static void
open_ok_cb (GtkWidget *button, gpointer data)
{
	const gchar *filename;
	gchar *uri;

	gtk_widget_hide (app->filesel);

	filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (app->filesel));

	uri = gnome_vfs_get_uri_from_local_path (filename);
	egg_recent_model_add (app->model, uri);
	g_free (uri);
}

static void
open_cancel_cb (GtkWidget *button, gpointer data)
{
	gtk_widget_hide (app->filesel);
}

static void
open_cb (GtkAction *action, gpointer data)
{
	gtk_widget_show (app->filesel);
}

static void
quit_cb (GtkAction *action, gpointer data)
{
	gtk_main_quit ();
}

static const GtkActionEntry menu_entries[] =
{
	{ "File", NULL, N_("_File") },
	{ "List", NULL, N_("_List") },

	{ "FileNew", GTK_STOCK_NEW, N_("_New"), "<control>N",
	  N_("Create a new document"), G_CALLBACK (open_cb) },
	{ "FileOpen", GTK_STOCK_OPEN, N_("_Open..."), "<control>O",
	  N_("Open a file"), G_CALLBACK (open_cb) },
	{ "FileQuit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
	  N_("Quit the program"), G_CALLBACK (quit_cb) },

	{ "GlobalRecents", NULL, N_("_Global") },
};

static const gchar *ui_info =
"  <menubar name=\"MenuBar\">\n"
"    <menu name=\"FileMenu\" action=\"File\">\n"
"      <menuitem name=\"FileNewMenu\" action=\"FileNew\" />\n"
"      <menuitem name=\"FileOpenMenu\" action=\"FileOpen\" />\n"
"      <placeholder name=\"FileRecentsPlaceholder\"\n>"
"        <separator/>\n"
"      </placeholder>\n"
"      <separator/>\n"
"      <menuitem name=\"FileQuitMenu\" action=\"FileQuit\" />\n"
"    </menu>\n"
"    <menu name=\"ListMenu\" action=\"List\">\n"
"      <menu name='GlobalRecentsMenu' action='GlobalRecents'>\n"
"      </menu>\n"
"    </menu>\n"
"  </menubar>\n";

static void
local_clear_cb (GtkButton *button, gpointer data)
{
	egg_recent_model_clear (app->model);
}

static void
global_clear_cb (GtkButton *button, gpointer data)
{
	egg_recent_model_clear (app->global_model);
}

static void
local_limit_activate_cb (GtkEntry *entry, gpointer data)
{
	const gchar *text;
	gint limit;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	limit = atoi (text);

	g_print ("Setting limit: %d\n", limit);

	egg_recent_model_set_limit (app->model, limit);
}

static void
global_limit_activate_cb (GtkEntry *entry, gpointer data)
{
	const gchar *text;
	gint limit;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	limit = atoi (text);

	g_print ("Setting limit: %d\n", limit);

	egg_recent_model_set_limit (app->global_model, limit);
}

static void
open_recent_cb (GtkAction *action, gpointer data)
{
	EggRecentItem *item;
	gchar *uri;
	GnomeVFSURI *vfs_uri;
	gboolean global;

	global = (GPOINTER_TO_INT (data) == 1);

	item = egg_recent_view_uimanager_get_item (global ? app->global_view : app->view,
						   action);
	g_return_if_fail (item != NULL);

	uri = egg_recent_item_get_uri (item);
	vfs_uri = gnome_vfs_uri_new (uri);
	g_print ("Opening: %s\n", uri);

	if (gnome_vfs_uri_exists (vfs_uri))
		egg_recent_model_add (app->model, uri);
	else  {
		g_print ("Opening failed: %s\n", uri);
	}

	g_free (uri);
	gnome_vfs_uri_unref (vfs_uri);
}

static void
construct_window (MyApp *app, gchar *title)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkUIManager *manager;
	GtkWidget *menubar;
	GtkActionGroup *action_group;
	GladeXML *xml;
	GtkWidget *content;
	GtkWidget *local_clear;
	GtkWidget *local_limit;
	GtkWidget *global_clear;
	GtkWidget *global_limit;

	vbox = gtk_vbox_new (FALSE, 0);
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	manager = gtk_ui_manager_new ();
	app->manager = manager;

	gtk_window_add_accel_group (GTK_WINDOW (window),
				    gtk_ui_manager_get_accel_group (manager));

	if (!gtk_ui_manager_add_ui_from_string (manager, ui_info, -1, NULL)) {
		g_message ("building menus failed");
	}

	action_group = gtk_action_group_new ("MyActions");
	gtk_action_group_add_actions (action_group,
				      menu_entries,
				      G_N_ELEMENTS (menu_entries),
				      NULL);

	gtk_ui_manager_insert_action_group (manager, action_group, 0);

	menubar = gtk_ui_manager_get_widget (manager, "/MenuBar");
	gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

	xml = glade_xml_new ("./egg-recent-test.glade",
			     "content", NULL);
	content = glade_xml_get_widget (xml, "content");
	global_clear = glade_xml_get_widget (xml, "global_clear_button");
	global_limit = glade_xml_get_widget (xml, "global_limit");
	local_clear = glade_xml_get_widget (xml, "local_clear_button");
	local_limit = glade_xml_get_widget (xml, "local_limit");

	gtk_box_pack_end (GTK_BOX (vbox), content, TRUE, TRUE, 5);

	gtk_widget_show_all (window);

	g_signal_connect (G_OBJECT (window), "delete-event",
			  G_CALLBACK (gtk_main_quit), NULL);

	g_signal_connect (G_OBJECT (global_clear), "clicked",
			  G_CALLBACK (global_clear_cb), NULL);
	g_signal_connect (G_OBJECT (local_clear), "clicked",
			  G_CALLBACK (local_clear_cb), NULL);
	g_signal_connect (G_OBJECT (global_limit), "activate",
			  G_CALLBACK (global_limit_activate_cb), NULL);
	g_signal_connect (G_OBJECT (local_limit), "activate",
			  G_CALLBACK (local_limit_activate_cb), NULL);
}

static void
construct_filesel (void)
{
	GtkWidget *filesel;
	
	filesel = gtk_file_selection_new ("Open....");
	app->filesel = filesel;
	g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
			  "clicked", G_CALLBACK (open_ok_cb), NULL);
	g_signal_connect (G_OBJECT (GTK_FILE_SELECTION(filesel)->cancel_button),
			  "clicked", G_CALLBACK (open_cancel_cb), NULL);
}

static char *
tooltip_func (EggRecentItem *item, gpointer user_data)
{
	char *tip;
	char *uri_for_display;

	uri_for_display = egg_recent_item_get_uri_for_display (item);
	g_return_val_if_fail (uri_for_display != NULL, NULL);

	tip = g_strdup_printf (_("Open '%s'"), uri_for_display);

	g_free (uri_for_display);

	return tip;
}

int
main (int argc, char *argv[])
{
	EggRecentModel *global_model;
	EggRecentModel *model;
	EggRecentViewUIManager *view;
	gchar *appname;

	if (argc > 1) {
		appname = argv[1];
	}
	else
		appname = "egg-recent-test-uimanager";

	app = g_new (MyApp, 1);

	gtk_init (&argc, &argv);

	construct_window (app, appname);
	construct_filesel ();

	/* Set up the egg_recent stuff */
	model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	egg_recent_model_set_filter_mime_types (model, "text/*", NULL);
	egg_recent_model_set_filter_uri_schemes (model, "file", NULL);
	view = egg_recent_view_uimanager_new (app->manager,
					      "/MenuBar/FileMenu/FileRecentsPlaceholder",
					      G_CALLBACK (open_recent_cb),
					      GINT_TO_POINTER (0));
	egg_recent_view_uimanager_show_icons (view, FALSE);
	egg_recent_view_uimanager_set_tooltip_func (view,
						    tooltip_func,
						    NULL);
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), model);
	app->view = view;

	global_model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	view = egg_recent_view_uimanager_new (app->manager,
					      "/MenuBar/ListMenu/GlobalRecentsMenu",
					      G_CALLBACK (open_recent_cb),
					      GINT_TO_POINTER (1));
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), global_model);
	app->global_view = view;

	app->model = model;
	app->global_model = global_model;

	/* destroy the models with the view */
	g_object_unref (app->model);
	g_object_unref (app->global_model);
	
	gtk_main ();

	g_object_unref (app->view);
	g_object_unref (app->global_view);
	
	return 0;
}
