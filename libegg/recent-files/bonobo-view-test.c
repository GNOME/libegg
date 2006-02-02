/*
 * hello.c
 *
 * A hello world application using the Bonobo UI handler
 *
 * Authors:
 *	Michael Meeks    <michael@ximian.com>
 *	Murray Cumming   <murrayc@usa.net>
 *      Havoc Pennington <hp@redhat.com>
 *
 * Copyright (C) 1999 Red Hat, Inc.
 *               2001 Murray Cumming,
 *               2001 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include <stdlib.h>

#include <libbonoboui.h>
#include <glade/glade-xml.h>
#include <libgnomevfs/gnome-vfs.h>
#include "egg-recent.h"

/* Keep a list of all open application windows */
static GSList *app_list = NULL;

/* the EggRecentModel object */
static EggRecentModel *model;
static EggRecentModel *global_model;

static EggRecentViewBonobo *view;
static EggRecentViewBonobo *global_view;

#define EGG_RECENT_UI_XML "egg-recent-test.xml"

/*   A single forward prototype - try and keep these
 * to a minumum by ordering your code nicely */
static GtkWidget *hello_new (void);

static void
hello_on_menu_file_new (BonoboUIComponent *uic,
			gpointer           user_data,
			const gchar       *verbname)
{
	gtk_widget_show_all (hello_new ());
}

static void
show_nothing_dialog(GtkWidget* parent)
{
	GtkWidget* dialog;

	dialog = gtk_message_dialog_new (
		GTK_WINDOW (parent),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		_("This does nothing; it is only a demonstration."));

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
hello_on_menu_file_open (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	gchar *uri;

	uri = bonobo_file_selector_open (NULL, TRUE, "Open....", NULL, NULL);

	if (uri != NULL) {
		EggRecentItem *item = egg_recent_item_new_from_uri (uri);
		egg_recent_item_add_group (item, "Bonobo Test");
		egg_recent_model_add_full (model, item);
		egg_recent_item_unref (item);
	}

	g_free (uri);
}

static void
hello_on_menu_file_save (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}

static void
hello_on_menu_file_saveas (BonoboUIComponent *uic,
			      gpointer           user_data,
			      const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}

static void
hello_on_menu_file_exit (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	g_object_unref (view);
	g_object_unref (global_view);

	
	exit (0);
}	

static void
hello_on_menu_file_close (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)
{
	GtkWidget *app = user_data;

	/* Remove instance: */
	app_list = g_slist_remove (app_list, app);

	gtk_widget_destroy (app);

	if (!app_list)
		hello_on_menu_file_exit(uic, user_data, verbname);
}

static void
hello_on_menu_edit_undo (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}	

static void
hello_on_menu_edit_redo (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}	

static void
hello_on_menu_edit_clear (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	egg_recent_model_clear (model);
}

static void
local_clear_cb (GtkButton *button, gpointer data)
{

	egg_recent_model_clear (model);
}

static void
global_clear_cb (GtkButton *button, gpointer data)
{
	egg_recent_model_clear (global_model);
}

static void
local_limit_activate_cb (GtkEntry *entry, gpointer data)
{
	const gchar *text;
	gint limit;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	limit = atoi (text);

	g_print ("Setting limit: %d\n", limit);

	egg_recent_model_set_limit (model, limit);
}

static void
global_limit_activate_cb (GtkEntry *entry, gpointer data)
{
	const gchar *text;
	gint limit;

	text = gtk_entry_get_text (GTK_ENTRY (entry));
	limit = atoi (text);

	g_print ("Setting limit: %d\n", limit);

	egg_recent_model_set_limit (global_model, limit);
}

static void
hello_on_menu_help_about (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (
		GTK_WINDOW (user_data),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		_("BonoboUI-Hello."));

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

/*
 *   These verb names are standard, see libonobobui/doc/std-ui.xml
 * to find a list of standard verb names.
 *   The menu items are specified in Bonobo_Sample_Hello.xml and
 * given names which map to these verbs here.
 */
static const BonoboUIVerb hello_verbs [] = {
	BONOBO_UI_VERB ("FileNew",    hello_on_menu_file_new),
	BONOBO_UI_VERB ("FileOpen",   hello_on_menu_file_open),
	BONOBO_UI_VERB ("FileSave",   hello_on_menu_file_save),
	BONOBO_UI_VERB ("FileSaveAs", hello_on_menu_file_saveas),
	BONOBO_UI_VERB ("FileClose",  hello_on_menu_file_close),
	BONOBO_UI_VERB ("FileExit",   hello_on_menu_file_exit),

	BONOBO_UI_VERB ("EditUndo",   hello_on_menu_edit_undo),
	BONOBO_UI_VERB ("EditRedo",   hello_on_menu_edit_redo),
	BONOBO_UI_VERB ("EditClear",   hello_on_menu_edit_clear),

	BONOBO_UI_VERB ("HelpAbout",  hello_on_menu_help_about),

	BONOBO_UI_VERB_END
};

static void
open_recent_cb (GtkWidget *widget, const EggRecentItem *item, gpointer data)
{
	GnomeVFSURI *vfs_uri;
	gchar *uri;
	gboolean ret;

	uri = egg_recent_item_get_uri (item);

	g_return_if_fail (uri != NULL);
	
	/* beware, this doesn't appear to work for mailto: URIs at least, so
	 * this test app is a bit broken
	 */
	vfs_uri = gnome_vfs_uri_new (uri);

	g_return_if_fail (vfs_uri != NULL);
	
	g_print ("Opening: %s\n", uri);

	if (gnome_vfs_uri_exists (vfs_uri)) {
		EggRecentItem *item = egg_recent_item_new_from_uri (uri);
		egg_recent_item_add_group (item, "Bonobo Test");
		egg_recent_model_add_full (model, item);
		egg_recent_item_unref (item);
		ret = TRUE;
	} else  {
		/* in actual use, we would probably want to remove the file
		 * from the list
		 */
		g_print ("Opening failed: %s\n", uri);
	}

	gnome_vfs_uri_unref (vfs_uri);
	g_free (uri);
}

static char *
tooltip_cb (EggRecentItem *item, gpointer user_data)
{
	char *uri, *ret;

	uri = egg_recent_item_get_uri_utf8 (item);
	ret = g_strdup_printf ("Open '%s'", uri);

	g_free (uri);

	return ret;
}

static BonoboWindow *
hello_create_main_window (void)
{
	BonoboWindow      *win;
	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;

	win = BONOBO_WINDOW (bonobo_window_new ("egg-recent", _("Gnome Hello")));

	/* Create Container: */
	ui_container = bonobo_window_get_ui_container (win);

	/* This determines where the UI configuration info. will be stored */
	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (win),
					  "/hello-app/UIConfig/kvps");

	/* Create a UI component with which to communicate with the window */
	ui_component = bonobo_ui_component_new_default ();

	/* Associate the BonoboUIComponent with the container */
	bonobo_ui_component_set_container (
		ui_component, BONOBO_OBJREF (ui_container), NULL);

	/* NB. this creates a relative file name from the current dir,
	 * in production you want to pass the application's datadir
	 * see Makefile.am to see how HELLO_SRCDIR gets set. */
	bonobo_ui_util_set_ui (ui_component, "", /* data dir */
			       "./" EGG_RECENT_UI_XML,
			       "bonobo-hello", NULL);

	/* Associate our verb -> callback mapping with the BonoboWindow */
	/* All the callback's user_data pointers will be set to 'win' */
	bonobo_ui_component_add_verb_list_with_data (ui_component, hello_verbs, win);

	/* Setup the EggRecentModel stuff */
	model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	egg_recent_model_set_filter_groups (model, "Bonobo Test", NULL);

	/* try these if you want
	egg_recent_model_set_filter_mime_types (model, "text/plain", NULL);
	egg_recent_model_set_filter_uri_schemes (model, "mailto",
						   "http", "ftp", NULL);
	*/
	view = egg_recent_view_bonobo_new (ui_component,
				"/menu/File/EggRecentDocuments/");
	egg_recent_view_bonobo_set_tooltip_func (view, tooltip_cb, NULL);
	egg_recent_view_bonobo_show_icons (view, TRUE);
	egg_recent_view_set_model (EGG_RECENT_VIEW (view), model);

	/* destroy the model when no more views are using it */
	g_object_unref (model);

	
	/* Let's see the global history too */
	global_model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	global_view = egg_recent_view_bonobo_new (ui_component,
				"/menu/Global/EggRecentDocuments/");
	egg_recent_view_bonobo_set_tooltip_func (global_view,
						 tooltip_cb, NULL);
	egg_recent_view_bonobo_show_icons (global_view, TRUE);
	egg_recent_view_set_model (EGG_RECENT_VIEW (global_view), global_model);
	g_object_unref (global_model);
	
	g_signal_connect (G_OBJECT (view), "activate",
			  G_CALLBACK (open_recent_cb), NULL);

	g_signal_connect (G_OBJECT (global_view), "activate",
			  G_CALLBACK (open_recent_cb), NULL);

	return win;
}

static gint 
delete_event_cb (GtkWidget *window, GdkEventAny *e, gpointer user_data)
{
	hello_on_menu_file_close (NULL, window, NULL);

	/* Prevent the window's destruction, since we destroyed it 
	 * ourselves with hello_app_close() */
	return TRUE;
}

static GtkWidget *
hello_new (void)
{
	GtkWidget    *global_clear;
	GtkWidget    *local_clear;
	GtkWidget    *global_limit;
	GtkWidget    *local_limit;
	GtkWidget    *content;
	GladeXML     *xml;
	BonoboWindow *win;
	gchar *tmp;

	win = hello_create_main_window();


	xml = glade_xml_new ("./egg-recent-test.glade", "content", NULL);

	if (!xml) {
		g_warning (_("Could not find egg-recent-test.glade2."));
		return NULL;
	}

	content = glade_xml_get_widget (xml, "content");
	global_clear = glade_xml_get_widget (xml, "global_clear_button");
	global_limit = glade_xml_get_widget (xml, "global_limit");
	local_clear = glade_xml_get_widget (xml, "local_clear_button");
	local_limit = glade_xml_get_widget (xml, "local_limit");

	tmp = g_strdup_printf ("%d", egg_recent_model_get_limit (model));
	gtk_entry_set_text (GTK_ENTRY (local_limit), tmp);

	/*
	tmp = g_strdup_printf ("%d", egg_recent_model_get_limit (global_model));
	*/
	gtk_entry_set_text (GTK_ENTRY (global_limit), tmp);

	bonobo_window_set_contents (win, content);

	/* Connect to the delete_event: a close from the window manager */
	g_signal_connect (G_OBJECT (win),
			    "delete_event",
			    G_CALLBACK (delete_event_cb),
			    NULL);

	g_signal_connect (G_OBJECT (global_clear), "clicked",
			  G_CALLBACK (global_clear_cb), NULL);
	g_signal_connect (G_OBJECT (local_clear), "clicked",
			  G_CALLBACK (local_clear_cb), NULL);
	g_signal_connect (G_OBJECT (global_limit), "activate",
			  G_CALLBACK (global_limit_activate_cb), NULL);
	g_signal_connect (G_OBJECT (local_limit), "activate",
			  G_CALLBACK (local_limit_activate_cb), NULL);

	return GTK_WIDGET(win);
}

int 
main (int argc, char* argv[])
{
	GtkWidget *app;

	if (!bonobo_ui_init ("bonobo-hello", "1.0", &argc, argv))
		g_error (_("Cannot init libbonoboui code"));

	app = hello_new ();

	gtk_widget_show_all (GTK_WIDGET (app));

	bonobo_main ();

	return 0;
}
