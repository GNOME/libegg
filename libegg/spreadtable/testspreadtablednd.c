/* testspreadtable.c
 * Copyright (C) 2010 Openismus GmbH
 *
 * Author:
 *      Tristan Van Berkom <tristanvb@openismus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <eggspreadtablednd.h>

#define INITIAL_HSPACING        2
#define INITIAL_VSPACING        2
#define INITIAL_LINES           3
#define INITIAL_HALIGN          GTK_ALIGN_FILL

static GtkWidget *paper = NULL;
static GtkAlign   child_halign     = INITIAL_HALIGN;
static gboolean   child_accepts_drops = TRUE;
static gboolean   parent_accepts_drops = TRUE;


static void
populate_spread_table_wrappy (EggSpreadTable *spread_table)
{
  GList *children, *l;
  GtkWidget *widget, *frame;
  gsize i;

  const gchar *strings[] = {
    "These are", "some wrappy label", "texts", "of various", "lengths.",
    "They should always be", "shown", "consecutively. Except it's",
    "hard to say", "where exactly the", "label", "will wrap", "and where exactly",
    "the actual", "container", "will wrap.", "This label is really really really long !",
    "Let's add some more", "labels to the",
    "mix. Just to", "make sure we", "got something to work", "with here."
  };

  /* Remove all children first */
  children = gtk_container_get_children (GTK_CONTAINER (paper));
  for (l = children; l; l = l->next)
    {
      GtkWidget *child = l->data;

      gtk_container_remove (GTK_CONTAINER (paper), child);
    }
  g_list_free (children);

  for (i = 0; i < G_N_ELEMENTS (strings); i++)
    {
      widget = gtk_label_new (strings[i]);
      frame  = gtk_frame_new (NULL);
      gtk_widget_show (widget);
      gtk_widget_show (frame);

      gtk_container_add (GTK_CONTAINER (frame), widget);

      gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
      gtk_label_set_line_wrap_mode (GTK_LABEL (widget), PANGO_WRAP_WORD);
      gtk_label_set_width_chars (GTK_LABEL (widget), 10);

      gtk_widget_set_halign (frame, child_halign);

      egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), frame, -1);
    }

  widget = gtk_label_new("labeltest");
  gtk_widget_show (widget);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);

  widget = gtk_label_new("selectable");
  gtk_label_set_selectable (GTK_LABEL (widget), TRUE);
  gtk_widget_show (widget);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);

  widget = gtk_button_new_with_label("buttontest");
  gtk_widget_show (widget);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);

  widget = gtk_font_button_new();
  gtk_widget_show (widget);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);

  widget = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0.0, 100.0, 1.0);
  gtk_widget_show (widget);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);

#ifdef WE_WANT_A_HUGE_APPCHOOSER_IN_THE_TEST
  widget = gtk_app_chooser_widget_new ("image/*");
  gtk_widget_show (widget);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);
#endif

  /* Weird behaviour */
  widget = gtk_scrolled_window_new (NULL, NULL);
  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (frame, 200, 200);
  gtk_widget_show (widget);
  gtk_widget_show (frame);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (widget), frame);
  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (spread_table), widget, -1);
}

static void
orientation_changed (GtkComboBox   *box,
                     EggSpreadTable  *paper)
{
  GtkOrientation orientation = gtk_combo_box_get_active (box);

  gtk_orientable_set_orientation (GTK_ORIENTABLE (paper), orientation);
}

static void
lines_changed (GtkSpinButton *button,
	       G_GNUC_UNUSED gpointer       data)
{
  gint lines = gtk_spin_button_get_value_as_int (button);

  egg_spread_table_set_lines (EGG_SPREAD_TABLE (paper), lines);
}

static void
spacing_changed (GtkSpinButton *button,
                 gpointer       data)
{
  GtkOrientation orientation = GPOINTER_TO_INT (data);
  gint           state = gtk_spin_button_get_value_as_int (button);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    egg_spread_table_set_horizontal_spacing (EGG_SPREAD_TABLE (paper), state);
  else
    egg_spread_table_set_vertical_spacing (EGG_SPREAD_TABLE (paper), state);
}



static void
halign_changed (GtkComboBox   *box,
                     EggSpreadTable  *paper)
{
  child_halign = gtk_combo_box_get_active (box);

  populate_spread_table_wrappy (EGG_SPREAD_TABLE (paper));
}


static gboolean
parent_drop_possible (EggSpreadTableDnd *table,
		      GtkWidget         *child,
		      gboolean          *drop_possible,
		      gpointer           unused)
{
  *drop_possible = parent_accepts_drops;

  return TRUE;
}

static gboolean
child_drop_possible (EggSpreadTableDnd *table,
		     GtkWidget         *child,
		     gboolean          *drop_possible,
		     gpointer           unused)
{
  *drop_possible = child_accepts_drops;

  return TRUE;
}

static void
drag_enabled_changed (GtkComboBox       *box,
		      EggSpreadTableDnd *table)
{
  egg_spread_table_dnd_set_drag_enabled (table, gtk_combo_box_get_active (box));
}

static void
drop_disable_toggled (GtkToggleButton *button,
		      EggSpreadTableDnd *table)
{
  egg_spread_table_dnd_set_drop_enabled (table, !gtk_toggle_button_get_active (button));
}

static void
set_boolean (GtkToggleButton *button,
	     gboolean        *value)
{
  *value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
}

static GtkWidget *
create_window (void)
{
  GtkWidget *window;
  GtkWidget *hbox, *vbox, *widget;
  GtkWidget *swindow, *frame, *expander;
  GtkWidget *paper_cntl, *items_cntl;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  hbox   = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  vbox   = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  gtk_widget_show (vbox);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (window), hbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

  frame = gtk_frame_new ("SpreadTable");
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

  swindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gtk_widget_show (swindow);
  gtk_container_add (GTK_CONTAINER (frame), swindow);

  paper = egg_spread_table_dnd_new (GTK_ORIENTATION_VERTICAL, INITIAL_LINES);
  egg_spread_table_set_vertical_spacing (EGG_SPREAD_TABLE (paper), INITIAL_VSPACING);
  egg_spread_table_set_horizontal_spacing (EGG_SPREAD_TABLE (paper), INITIAL_HSPACING);
  gtk_widget_show (paper);

  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (swindow), paper);

  /* Add SpreadTable test control frame */
  expander = gtk_expander_new ("SpreadTable controls");
  gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
  paper_cntl = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_show (paper_cntl);
  gtk_widget_show (expander);
  gtk_container_add (GTK_CONTAINER (expander), paper_cntl);
  gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);

  /* Add Orientation control */
  widget = gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Horizontal");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Vertical");
  gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 1);
  gtk_widget_show (widget);

  gtk_widget_set_tooltip_text (widget, "Set the spread_table orientation");
  gtk_box_pack_start (GTK_BOX (paper_cntl), widget, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (widget), "changed",
                    G_CALLBACK (orientation_changed), paper);


  /* Add horizontal/vertical spacing controls */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_widget_show (hbox);

  widget = gtk_label_new ("H Spacing");
  gtk_widget_show (widget);
  gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

  widget = gtk_spin_button_new_with_range (0, 30, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), INITIAL_HSPACING);
  gtk_widget_show (widget);

  gtk_widget_set_tooltip_text (widget, "Set the horizontal spacing between children");
  gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (widget), "changed",
                    G_CALLBACK (spacing_changed), GINT_TO_POINTER (GTK_ORIENTATION_HORIZONTAL));
  g_signal_connect (G_OBJECT (widget), "value-changed",
                    G_CALLBACK (spacing_changed), GINT_TO_POINTER (GTK_ORIENTATION_HORIZONTAL));

  gtk_box_pack_start (GTK_BOX (paper_cntl), hbox, FALSE, FALSE, 0);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_widget_show (hbox);

  widget = gtk_label_new ("V Spacing");
  gtk_widget_show (widget);
  gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

  widget = gtk_spin_button_new_with_range (0, 30, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), INITIAL_VSPACING);
  gtk_widget_show (widget);

  gtk_widget_set_tooltip_text (widget, "Set the vertical spacing between children");
  gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (widget), "changed",
                    G_CALLBACK (spacing_changed), GINT_TO_POINTER (GTK_ORIENTATION_VERTICAL));
  g_signal_connect (G_OBJECT (widget), "value-changed",
                    G_CALLBACK (spacing_changed), GINT_TO_POINTER (GTK_ORIENTATION_VERTICAL));

  gtk_box_pack_start (GTK_BOX (paper_cntl), hbox, FALSE, FALSE, 0);


  /* Add widget-drop-possible controls */
  widget = gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Disable Drag");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Enable Drag");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Full Drag");
  gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 1);
  gtk_widget_show (widget);

  gtk_widget_set_tooltip_text (widget, "Set whether you can drag widgets from the table");
  gtk_box_pack_start (GTK_BOX (paper_cntl), widget, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (widget), "changed",
                    G_CALLBACK (drag_enabled_changed), paper);

  /* Add widget-drop-possible controls */
  widget = gtk_toggle_button_new_with_label ("Disable Drop");
  gtk_widget_show (widget);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
  gtk_box_pack_start (GTK_BOX (paper_cntl), widget, FALSE, FALSE, 0);
  g_signal_connect (widget, "toggled", G_CALLBACK (drop_disable_toggled), paper);
  gtk_widget_set_tooltip_text (widget, "Set whether you can drop widgets on the table");

  /* Add widget-drop-possible controls */
  widget = gtk_toggle_button_new_with_label ("parent accept drop");
  gtk_widget_show (widget);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
  gtk_box_pack_start (GTK_BOX (paper_cntl), widget, FALSE, FALSE, 0);
  g_signal_connect (widget, "toggled", G_CALLBACK (set_boolean), &parent_accepts_drops);
  gtk_widget_set_tooltip_text (widget, "Set whether the parent will return TRUE from "
			       "the \"widget-drop-possible\" signal");

  widget = gtk_toggle_button_new_with_label ("child accept drop");
  gtk_widget_show (widget);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
  gtk_box_pack_start (GTK_BOX (paper_cntl), widget, FALSE, FALSE, 0);
  g_signal_connect (widget, "toggled", G_CALLBACK (set_boolean), &child_accepts_drops);
  gtk_widget_set_tooltip_text (widget, "Set whether the child will return TRUE from "
			       "the \"widget-drop-possible\" signal");

  /* Add lines controls */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
  gtk_widget_show (hbox);

  widget = gtk_label_new ("Lines");
  gtk_widget_show (widget);
  gtk_box_pack_start (GTK_BOX (hbox), widget, TRUE, TRUE, 0);

  widget = gtk_spin_button_new_with_range (1, 30, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), INITIAL_LINES);
  gtk_widget_show (widget);

  gtk_widget_set_tooltip_text (widget, "Set the horizontal spacing between children");
  gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (widget), "changed",
                    G_CALLBACK (lines_changed), NULL);
  g_signal_connect (G_OBJECT (widget), "value-changed",
                    G_CALLBACK (lines_changed), NULL);

  gtk_box_pack_start (GTK_BOX (paper_cntl), hbox, FALSE, FALSE, 0);


  /* Add test items control frame */
  expander = gtk_expander_new ("Test item controls");
  gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
  items_cntl = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_show (items_cntl);
  gtk_widget_show (expander);
  gtk_container_add (GTK_CONTAINER (expander), items_cntl);
  gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 0);

  /* Add child halign control */
  widget = gtk_combo_box_text_new ();
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Fill");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Start");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "End");
  gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), "Center");
  gtk_combo_box_set_active (GTK_COMBO_BOX (widget), INITIAL_HALIGN);
  gtk_widget_show (widget);

  gtk_widget_set_tooltip_text (widget, "Set the children's halign property");
  gtk_box_pack_start (GTK_BOX (items_cntl), widget, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (widget), "changed",
                    G_CALLBACK (halign_changed), paper);

  populate_spread_table_wrappy (EGG_SPREAD_TABLE (paper));

  /* Embed another dnd spread table */
  widget = egg_spread_table_dnd_new (GTK_ORIENTATION_VERTICAL, INITIAL_LINES);
  egg_spread_table_set_vertical_spacing (EGG_SPREAD_TABLE (widget), INITIAL_VSPACING);
  egg_spread_table_set_horizontal_spacing (EGG_SPREAD_TABLE (widget), INITIAL_HSPACING);

  frame  = gtk_frame_new (NULL);
  gtk_widget_show (widget);
  gtk_widget_show (frame);
  gtk_widget_set_size_request (widget, 40, 40);
  gtk_container_add (GTK_CONTAINER (frame), widget);

  egg_spread_table_dnd_insert_child (EGG_SPREAD_TABLE_DND (paper), frame, 5);

  gtk_window_set_default_size (GTK_WINDOW (window), 500, 400);


  /* Signals to control drop allowed or not */
  g_signal_connect (G_OBJECT (paper), "widget-drop-possible",
                    G_CALLBACK (parent_drop_possible), NULL);
  g_signal_connect (G_OBJECT (widget), "widget-drop-possible",
                    G_CALLBACK (child_drop_possible), NULL);


  return window;
}

int
main (int argc, char *argv[])
{
  GtkWidget *window;

  gtk_init (&argc, &argv);

  window = create_window ();

  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_main_quit), window);

  gtk_widget_show (window);

  gtk_main ();

  return 0;
}
