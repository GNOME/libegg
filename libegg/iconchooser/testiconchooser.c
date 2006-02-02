
#include <gtk/gtk.h>

#include "eggiconchooserbutton.h"
#include "prop-editor.h"

static void
win_style_set (GtkDialog *win,
	       GtkStyle  *old_style,
	       gpointer  user_data)
{
  gtk_container_set_border_width (GTK_CONTAINER (win->vbox), 12);
  gtk_box_set_spacing (GTK_BOX (win->vbox), 24);
  gtk_container_set_border_width (GTK_CONTAINER (win->action_area), 0);
  gtk_box_set_spacing (GTK_BOX (win->action_area), 6);
}

static gboolean
prop_editor_delete_event_cb (GtkWidget *widget,
			     gpointer   user_data)
{
  gtk_widget_hide (widget);
  return TRUE;
}

static void
properties_button_clicked_cb (GtkWidget *button,
			      GObject   *chooser)
{
  GtkWidget *prop_editor;

  prop_editor = g_object_get_data (chooser, "testiconchooser-prop-editor");

  if (prop_editor == NULL)
    {
      GtkWidget *toplevel;

      prop_editor = create_prop_editor (chooser, G_TYPE_INVALID);
      g_signal_connect (prop_editor, "delete-event",
			G_CALLBACK (prop_editor_delete_event_cb), NULL);

      toplevel = gtk_widget_get_toplevel (GTK_WIDGET (chooser));
      if (toplevel && GTK_WIDGET_TOPLEVEL (toplevel))
	gtk_window_set_transient_for (GTK_WINDOW (prop_editor),
				      GTK_WINDOW (toplevel));

      g_object_set_data_full (chooser, "testiconchooser-prop-editor",
			      prop_editor, (GDestroyNotify) gtk_widget_destroy);
    }

  gtk_window_present (GTK_WINDOW (prop_editor));
}


int
main (int argc,
      char *argv[])
{
  GtkWidget *win, *box, *chooser, *align, *properties_button;

  gtk_init (&argc, &argv);

  win = gtk_dialog_new_with_buttons ("Icon Chooser Test", NULL,
				     GTK_DIALOG_NO_SEPARATOR,
				     GTK_STOCK_QUIT, GTK_RESPONSE_OK,
				     NULL);
  g_signal_connect (win, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (win, "response", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect (win, "style-set", G_CALLBACK (win_style_set), NULL);

  box = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (win)->vbox), box);
  gtk_widget_show (box);

  chooser = egg_icon_chooser_button_new ("Pick An Icon - Icon Chooser Test");
  gtk_box_pack_start (GTK_BOX (box), chooser, FALSE, FALSE, 0);
  gtk_widget_show (chooser);

  align = gtk_alignment_new (0.0, 1.0, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (box), align, FALSE, FALSE, 0);
  gtk_widget_show (align);

  properties_button = gtk_button_new_from_stock (GTK_STOCK_PROPERTIES);
  g_signal_connect (properties_button, "clicked",
		    G_CALLBACK (properties_button_clicked_cb), chooser);
  gtk_container_add (GTK_CONTAINER (align), properties_button);
  gtk_widget_show (properties_button);

  gtk_window_present (GTK_WINDOW (win));

  gtk_main ();

  gtk_widget_destroy (win);

  return 0;
}
