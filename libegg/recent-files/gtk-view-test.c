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

static void open_cb (void);
static void open_ok_cb (GtkWidget *button, gpointer data);
static void open_cancel_cb (GtkWidget *button, gpointer data);
static void dump_list_cb (GtkWidget *button, gpointer data);
static void local_clear_cb (GtkButton *button, gpointer data);
static void leading_sep_cb (GtkWidget *button, gpointer data);
static void trailing_sep_cb (GtkWidget *button, gpointer data);


typedef struct _MyApp MyApp;
struct _MyApp {
	EggRecentModel *model;
	EggRecentViewGtk *view;
	EggRecentModel *global_model;
	EggRecentViewGtk *global_view;
	GtkItemFactory *menu;
	GtkWidget *filesel;

	GtkWidget *leading_check;
	GtkWidget *trailing_check;
};

MyApp *app;

static GtkItemFactoryEntry menu_items[] = {
	{ "/_File",              NULL, NULL,          0, "<Branch>" },
	{ "/_File/foo",          NULL, NULL,          0, "<Tearoff>" },
	{ "/_File/_New",         NULL, open_cb,       0, "<StockItem>", GTK_STOCK_NEW },
	{ "/_File/_Open",        NULL, open_cb,       0, "<StockItem>", GTK_STOCK_OPEN },
	{ "/_File/_Leading Separator", NULL, leading_sep_cb,0, "<CheckItem>"},
	{ "/_File/_Trailing Separator", NULL, trailing_sep_cb,0, "<CheckItem>"},
	{ "/_File/_Quit",        NULL, gtk_main_quit, 0, "<StockItem>", GTK_STOCK_QUIT },
	{ "/_List",              NULL, NULL,          0, "<Branch>" },
	{ "/_List/foo",          NULL, NULL,          0, "<Tearoff>" },
	{ "/_List/Global",    NULL, NULL,          0, "<Branch>" },
	{ "/_List/_Dump",        NULL, dump_list_cb,  0, "<StockItem>", GTK_STOCK_PASTE },
	{ "/_List/_Clear",       NULL, local_clear_cb, 0, "<StockItem>", GTK_STOCK_DELETE },
};

static void
leading_sep_cb (GtkWidget *widget, gpointer data)
{
	gboolean state;

	state = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (app->leading_check));
	
	egg_recent_view_gtk_set_leading_sep (app->view, state);
}

static void
trailing_sep_cb (GtkWidget *widget, gpointer data)
{
	gboolean state;

	state = gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (app->trailing_check));
	
	egg_recent_view_gtk_set_trailing_sep (app->view, state);
}

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
dump_list_cb (GtkWidget *widget, gpointer data)
{
	GList *list = egg_recent_model_get_list (app->model);
	GList *global_list = egg_recent_model_get_list (app->global_model);
	gint i = 1;

	g_print ("\nDumping URIs for this app:\n");
	for (; list; list = list->next) {
		gchar *uri = egg_recent_item_get_uri_for_display ((EggRecentItem *) list->data);
		g_print ("%d: %s\n", i++, uri);

		g_free (uri);
	}
	g_print ("\n");

	i=1;
	g_print ("\nDumping Global URIs:\n");
	for (; global_list; global_list = global_list->next) {
		gchar *uri = egg_recent_item_get_uri_for_display ((EggRecentItem *) global_list->data);
		g_print ("%d: %s\n", i++, uri);

		g_free (uri);
	}
	g_print ("\n");

	EGG_RECENT_ITEM_LIST_UNREF (list);
	EGG_RECENT_ITEM_LIST_UNREF (global_list);
}

static gboolean
open_recent_cb (EggRecentView *view, EggRecentItem *item, gpointer data)
{
	gchar *uri;
	GnomeVFSURI *vfs_uri;
	gboolean ret = TRUE;

	uri = egg_recent_item_get_uri (item);
	vfs_uri = gnome_vfs_uri_new (uri);
	g_print ("Opening: %s\n", uri);

	if (gnome_vfs_uri_exists (vfs_uri))
		egg_recent_model_add (app->model, uri);
	else  {
		g_print ("Opening failed: %s\n", uri);
		ret = FALSE;
	}

	g_free (uri);
	gnome_vfs_uri_unref (vfs_uri);

	return ret;
}

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
open_cb (void)
{
	gtk_widget_show (app->filesel);
}

static void
construct_window (MyApp *app, gchar *title)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
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

	accel_group = gtk_accel_group_new ();
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>",
					     accel_group);
	app->menu = item_factory;
	    
	g_object_ref (G_OBJECT (item_factory));
	g_object_set_data_full (G_OBJECT (window),
				"<main>",
				item_factory,
				(GDestroyNotify) g_object_unref);

	gtk_item_factory_create_items (item_factory, G_N_ELEMENTS (menu_items),
				       menu_items, window);

	gtk_box_pack_start (GTK_BOX (vbox),
			    gtk_item_factory_get_widget (item_factory,"<main>"),
			    FALSE, FALSE, 0);

	xml = glade_xml_new ("./egg-recent-test.glade",
						"content", NULL);
	content = glade_xml_get_widget (xml, "content");
	global_clear = glade_xml_get_widget (xml, "global_clear_button");
	global_limit = glade_xml_get_widget (xml, "global_limit");
	local_clear = glade_xml_get_widget (xml, "local_clear_button");
	local_limit = glade_xml_get_widget (xml, "local_limit");

	/*
	tmp = g_strdup_printf ("%d", egg_recent_model_get_limit (app->recent));
	gtk_entry_set_text (GTK_ENTRY (local_limit), tmp);
	*/

	app->leading_check = gtk_item_factory_get_widget (item_factory,
						"/File/Leading Separator");
	app->trailing_check = gtk_item_factory_get_widget (item_factory,
						"/File/Trailing Separator");
	
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

static void
tooltip_cb (GtkTooltips *tooltips, GtkWidget *menu_item,
	    EggRecentItem *item, gpointer user_data)
{
	gchar *uri, *tip;

	uri = egg_recent_item_get_uri_for_display (item);
	tip = g_strdup_printf ("Open '%s'", uri);

	gtk_tooltips_set_tip (tooltips, menu_item, tip, NULL);

	g_free (tip);
	g_free (uri);
}

int
main (int argc, char *argv[])
{
	EggRecentModel *global_model;
	EggRecentModel *model;
	EggRecentViewGtk *view;
	gchar *appname;


	if (argc > 1) {
		appname = argv[1];
	}
	else
		appname = "egg-recent-test-gtk";
	
	app = g_new (MyApp, 1);

	gtk_init (&argc, &argv);

	construct_window (app, appname);
	construct_filesel ();

	/* Set up the egg_recent stuff */
	model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	egg_recent_model_set_filter_mime_types (model, "text/*", NULL);
	egg_recent_model_set_filter_uri_schemes (model, "file", NULL);
	view = egg_recent_view_gtk_new (gtk_item_factory_get_widget (app->menu, "/File"),
					gtk_item_factory_get_widget (app->menu, "/File/Open"));
	egg_recent_view_gtk_set_tooltip_func (view, tooltip_cb, NULL);
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), model);
	app->view = view;

	g_signal_connect (G_OBJECT (view), "activate",
			  G_CALLBACK (open_recent_cb), NULL);
	
	global_model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	view = egg_recent_view_gtk_new (gtk_item_factory_get_widget (app->menu, "/List/Global"),
					NULL);
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), global_model);
	app->global_view = view;
	
	g_signal_connect (G_OBJECT (view), "activate",
			  G_CALLBACK (open_recent_cb), NULL);

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
