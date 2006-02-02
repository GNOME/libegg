/*
 * Copyright (C) 2003  Sebastian Rittau <srittau@jroger.in-berlin.de>
 * $Id$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <libintl.h>
#include <locale.h>

#include <gtk/gtk.h>

#include "egg-datetime.h"


#ifndef _
#define _(x) dgettext(GETTEXT_PACKAGE, (x))
#endif


static void
show_date_toggled_cb (GtkToggleButton *toggle, EggDateTime *edt)
{
	gboolean active = gtk_toggle_button_get_active (toggle);
	EggDateTimeDisplayMode mode = egg_datetime_get_display_mode (edt);

	if (active)
		mode |= EGG_DATETIME_DISPLAY_DATE;
	else
		mode &= ~EGG_DATETIME_DISPLAY_DATE;

	egg_datetime_set_display_mode (edt, mode);
}

static void
show_time_toggled_cb (GtkToggleButton *toggle, EggDateTime *edt)
{
	gboolean active = gtk_toggle_button_get_active (toggle);
	EggDateTimeDisplayMode mode = egg_datetime_get_display_mode (edt);

	if (active)
		mode |= EGG_DATETIME_DISPLAY_TIME;
	else
		mode &= ~EGG_DATETIME_DISPLAY_TIME;

	egg_datetime_set_display_mode (edt, mode);
}

static void
lazy_mode_toggled_cb (GtkToggleButton *toggle, EggDateTime *edt)
{
	gboolean active = gtk_toggle_button_get_active (toggle);

	egg_datetime_set_lazy (edt, active);
}

static void
gui (int argc, char *argv[], GMainLoop *main_loop)
{
	GtkWidget *win;
	GtkWidget *main_box, *hbox, *vbox;
	GtkWidget *label, *check;
	GtkWidget *edt;
	PangoAttrList *attrs;
	PangoAttribute *attr;

	gtk_init (&argc, &argv);

	win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (win), _("EggDateTime Test"));
	gtk_container_set_border_width (GTK_CONTAINER (win), 6);
	g_signal_connect_swapped (G_OBJECT (win), "delete_event",
				  G_CALLBACK (g_main_loop_quit), main_loop);

	main_box = gtk_vbox_new (FALSE, 6);
	gtk_widget_show (main_box);
	gtk_container_add (GTK_CONTAINER (win), main_box);

	/* Date/Time widget */

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (main_box), hbox, FALSE, FALSE, 0);

	edt = egg_datetime_new ();
	gtk_widget_show (edt);
	gtk_box_pack_start (GTK_BOX (hbox), edt, FALSE, FALSE, 0);

	/* Options */

	attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
	attr->start_index = 0;
	attr->end_index   = G_MAXUINT;
	attrs = pango_attr_list_new ();
	pango_attr_list_insert (attrs, attr);

	label = gtk_label_new (_("Options"));
	gtk_label_set_attributes (GTK_LABEL (label), attrs);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (main_box), label, FALSE, FALSE, 6);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (main_box), hbox, FALSE, FALSE, 0);

	label = gtk_label_new ("");
	gtk_widget_set_size_request (label, 12, -1);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	vbox = gtk_vbox_new (TRUE, 6);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);

	check = gtk_check_button_new_with_mnemonic (_("Show _date"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check), TRUE);
	gtk_widget_show (check);
	g_signal_connect (G_OBJECT (check), "toggled",
			  G_CALLBACK (show_date_toggled_cb), edt);
	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, 0);

	check = gtk_check_button_new_with_mnemonic (_("Show _time"));
	gtk_widget_show (check);
	g_signal_connect (G_OBJECT (check), "toggled",
			  G_CALLBACK (show_time_toggled_cb), edt);
	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, 0);

	check = gtk_check_button_new_with_mnemonic (_("_Lazy mode"));
	gtk_widget_show (check);
	g_signal_connect (G_OBJECT (check), "toggled",
			  G_CALLBACK (lazy_mode_toggled_cb), edt);
	gtk_box_pack_start (GTK_BOX (vbox), check, FALSE, FALSE, 0);

	gtk_widget_show (win);
}

int
main (int argc, char *argv[])
{
        GMainLoop *main_loop;
 
	/* i18n */

	setlocale (LC_ALL, "");
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

	/* GUI Setup */

        main_loop = g_main_loop_new (NULL, FALSE);
	gui (argc, argv, main_loop);

	/* Main loop */

        g_main_loop_run (main_loop);
	g_main_loop_unref (main_loop);

	return 0;
}
