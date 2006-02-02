#include "eggsplinner.h"

#include <gtk/gtktreeselection.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkhscale.h>
#include <gtk/gtkmain.h>

#include "eggentry.h"

#include <gdk/gdkkeysyms.h>

#include <string.h>


static void     egg_splinner_class_init      (EggSplinnerClass *klass);
static void     egg_splinner_init            (EggSplinner      *text);


static void     egg_splinner_entry_changed   (GtkWidget            *entry,
					      gpointer              data);

static void     egg_splinner_remove_grabs    (EggSplinner      *text);

static void     egg_splinner_popped_up       (GtkWidget            *widget,
					      gpointer              data);

static gboolean egg_splinner_button_pressed  (GtkWidget            *widget,
					      GdkEventButton       *event,
					      gpointer              data);
static gboolean egg_splinner_button_released (GtkWidget            *widget,
					      GdkEventButton       *event,
					      gpointer              data);

static gboolean egg_splinner_key_press       (GtkWidget            *widget,
					      GdkEventKey          *event,
					      gpointer              data);


GType
egg_splinner_get_type (void)
{
  static GType splinner_type = 0;

  if (!splinner_type)
    {
      static const GTypeInfo splinner_info =
        {
	  sizeof (EggSplinnerClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) egg_splinner_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (EggSplinner),
	  0,
	  (GInstanceInitFunc) egg_splinner_init
	};

      splinner_type = g_type_register_static (EGG_TYPE_COMBO_BOX,
					      "EggSplinner",
					      &splinner_info,
					      0);
    }

  return splinner_type;
}

static void
egg_splinner_class_init (EggSplinnerClass *klass)
{
}

static void
egg_splinner_init (EggSplinner *textcombo)
{
  g_signal_connect (textcombo, "popped_up",
		    G_CALLBACK (egg_splinner_popped_up), NULL);
  g_signal_connect (EGG_COMBO_BOX (textcombo)->button, "button_press_event",
		    G_CALLBACK (egg_splinner_button_pressed), textcombo);

  textcombo->entry = egg_entry_new ();
  g_signal_connect (textcombo->entry, "changed",
		    G_CALLBACK (egg_splinner_entry_changed), textcombo);

  textcombo->range = gtk_hscale_new_with_range (0.0, 100.0, 5.0);
  g_signal_connect (textcombo->range, "button_press_event",
		    G_CALLBACK (egg_splinner_button_pressed), textcombo);
  g_signal_connect (textcombo->range, "button_release_event",
		    G_CALLBACK (egg_splinner_button_released), textcombo);
  gtk_scale_set_draw_value (GTK_SCALE (textcombo->range), FALSE);
}



/* callbacks */
static void
egg_splinner_entry_changed (GtkWidget *entry,
		                  gpointer   data)
{
  g_print ("egg_splinner_entry_changed\n");
}

static void
egg_splinner_remove_grabs (EggSplinner *splinner)
{
  EggComboBox *box = EGG_COMBO_BOX (splinner);

  if (GTK_WIDGET_HAS_GRAB (splinner->range))
    gtk_grab_remove (splinner->range);

  if (GTK_WIDGET_HAS_GRAB (box->popup_window))
    {
      gtk_grab_remove (box->popup_window);
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
}

static void
egg_splinner_popped_up (GtkWidget *combobox,
			      gpointer   data)
{

}

static gboolean
egg_splinner_button_pressed (GtkWidget      *widget,
			     GdkEventButton *event,
			     gpointer        data)
{
  EggComboBox *box = EGG_COMBO_BOX (data);
  EggSplinner *splinner = EGG_SPLINNER (data);

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (ewidget == splinner->range)
    return TRUE;

  if (ewidget != box->button ||
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (box->button)))
    return FALSE;

  egg_combo_box_popup (box);

  gtk_grab_add (box->popup_window);
  gdk_pointer_grab (box->popup_window->window, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);

  /* FIXME: drag selection on treeview */
  gtk_grab_add (splinner->range);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (box->button), TRUE);

  splinner->popup_in_progress = TRUE;

  return TRUE;
}

static gboolean
egg_splinner_button_released (GtkWidget      *widget,
			      GdkEventButton *event,
			      gpointer        data)
{
  EggComboBox *box = EGG_COMBO_BOX (data);
  EggSplinner *splinner = EGG_SPLINNER (data);

  gboolean popup_in_progress = FALSE;

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (splinner->popup_in_progress)
    {
      popup_in_progress = TRUE;
      splinner->popup_in_progress = FALSE;
    }

  if (ewidget != splinner->range)
    {
      if (ewidget == box->button &&
	  !popup_in_progress &&
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (box->button)))
        {
	  egg_splinner_remove_grabs (splinner);
	  egg_combo_box_popdown (box);
	  return TRUE;
	}

      /* released outsite treeview */
      if (ewidget != box->button)
        {
          egg_splinner_remove_grabs (splinner);
          egg_combo_box_popdown (box);

          return TRUE;
        }

      return FALSE;
    }

  /* drop grabs */
  egg_splinner_remove_grabs (splinner);
  egg_combo_box_popdown (box);

  return TRUE;
}


/* public API */
GtkWidget *
egg_splinner_new ()
{
  EggSplinner *textcombo;

  textcombo = EGG_SPLINNER (g_object_new (egg_splinner_get_type (), NULL));

  egg_combo_box_set_sample_widget (EGG_COMBO_BOX (textcombo),
				   textcombo->entry);
  egg_combo_box_set_popup_widget (EGG_COMBO_BOX (textcombo),
				  textcombo->range);

  return GTK_WIDGET (textcombo);
}
