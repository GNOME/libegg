#include "eggcombobox.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkbindings.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkmenu.h>
#include "eggmarshalers.h"

enum {
  CHANGED,
  POPPED_DOWN,
  POPPED_UP,
  POPUP,
  LAST_SIGNAL
};

static guint combo_signals[LAST_SIGNAL] = { 0 };

static void	egg_combo_box_class_init	(EggComboBoxClass *klass);
static void	egg_combo_box_init		(EggComboBox      *combobox);

static gboolean	egg_combo_box_real_popup	(EggComboBox	  *combobox);

static void     egg_combo_box_menu_position     (GtkMenu          *menu,
                                                 gint             *x,
			                         gint             *y,
			                         gboolean         *push_in,
			                         gpointer          user_data);

static void	egg_combo_box_toggled		(GtkWidget        *button,
						 gpointer          data);
static gboolean egg_combo_box_button_press      (GtkWidget        *button,
                                                 GdkEventButton   *event,
						 gpointer          user_data);


GType
egg_combo_box_get_type (void)
{
  static GType combo_box_type = 0;

  if (!combo_box_type)
    {
      static const GTypeInfo combo_box_info =
        {
	  sizeof (EggComboBoxClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) egg_combo_box_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (EggComboBox),
	  0,
	  (GInstanceInitFunc) egg_combo_box_init
	};

      combo_box_type = g_type_register_static (GTK_TYPE_HBOX,
					       "EggComboBox",
					       &combo_box_info, 0);
    }

  return combo_box_type;
}

static void
egg_combo_box_class_init (EggComboBoxClass *klass)
{
  GtkBindingSet *binding_set;

  binding_set = gtk_binding_set_by_class (klass);
  
  klass->popup = egg_combo_box_real_popup;
  
  combo_signals[CHANGED] =
    g_signal_new ("changed",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggComboBoxClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  combo_signals[POPPED_UP] = 
    g_signal_new ("popped_up",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggComboBoxClass, popped_up),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  combo_signals[POPPED_DOWN] = 
    g_signal_new ("popped_down",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (EggComboBoxClass, popped_down),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  combo_signals[POPUP] =
    g_signal_new ("popup",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (EggComboBoxClass, popup),
		  NULL, NULL,
		  _egg_marshal_BOOLEAN__VOID,
		  G_TYPE_BOOLEAN, 0);

  gtk_binding_entry_add_signal (binding_set, GDK_Down, GDK_MOD1_MASK, "popup", 0);
}

static void
egg_combo_box_init (EggComboBox *combobox)
{
  GtkWidget *arrow;

  combobox->popped_up = FALSE;

  combobox->button = gtk_toggle_button_new ();
  arrow = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (combobox->button), arrow);
  gtk_box_pack_start (GTK_BOX (combobox), combobox->button,
		      FALSE, FALSE, 0);
  gtk_widget_show_all (combobox->button);

  g_signal_connect (combobox->button, "toggled",
		    G_CALLBACK (egg_combo_box_toggled), combobox);
  g_signal_connect (combobox->button, "button_press_event",
                    G_CALLBACK (egg_combo_box_button_press), combobox);
}

static void
egg_combo_box_setup_popup (EggComboBox *combobox)
{
  if (combobox->popup)
    return;

  combobox->popup_window = gtk_window_new (GTK_WINDOW_POPUP);

  combobox->frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (combobox->frame),
			     GTK_SHADOW_NONE);
  gtk_container_add (GTK_CONTAINER (combobox->popup_window), combobox->frame);
  gtk_widget_show (combobox->frame);
}

static void
egg_combo_box_destroy_popup (EggComboBox *combobox)
{
  if (combobox->popup_window)
    gtk_widget_destroy (combobox->popup_window);
}

static void
egg_combo_box_toggled (GtkWidget *button,
		       gpointer   data)
{
  EggComboBox *combobox = EGG_COMBO_BOX (data);

  if (GTK_IS_MENU (combobox->popup_window))
    return;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
    /* popup the popup window */
    egg_combo_box_popup (combobox);
  else
    /* popdown */
    egg_combo_box_popdown (combobox);
}

static gboolean
egg_combo_box_button_press (GtkWidget      *button,
                            GdkEventButton *event,
			    gpointer        user_data)
{
  EggComboBox *combobox = EGG_COMBO_BOX (user_data);

  if (! GTK_IS_MENU (combobox->popup_window))
    return FALSE;

  if (event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
      gtk_menu_popup (GTK_MENU (combobox->popup_window),
                      NULL, NULL,
                      egg_combo_box_menu_position, combobox,
		      event->button, event->time);

      /* above code will handle the button for us, we will
       * emit the signal here ... eeew
       */
      g_signal_emit (combobox, combo_signals[POPPED_UP], 0);

      return TRUE;
    }

  return FALSE;
}

static gboolean
egg_combo_box_real_popup (EggComboBox *combobox)
{
  egg_combo_box_popup (combobox);

  return TRUE;
}

GtkWidget *
egg_combo_box_new ()
{
  EggComboBox *combobox;

  combobox = EGG_COMBO_BOX (g_object_new (egg_combo_box_get_type (), NULL));

  return GTK_WIDGET (combobox);
}

GtkWidget *
egg_combo_box_get_sample_widget (EggComboBox *combobox)
{
  g_return_val_if_fail (EGG_IS_COMBO_BOX (combobox), NULL);

  return combobox->sample;
}

void
egg_combo_box_set_sample_widget (EggComboBox *combobox,
				 GtkWidget   *sample)
{
  g_return_if_fail (EGG_IS_COMBO_BOX (combobox));
  g_return_if_fail (GTK_IS_WIDGET (sample));

  if (combobox->sample)
    gtk_container_remove (GTK_CONTAINER (combobox), combobox->sample);

  gtk_box_pack_start (GTK_BOX (combobox), sample, TRUE, TRUE, 0);
  gtk_box_reorder_child (GTK_BOX (combobox), sample, 0);
  g_object_ref (G_OBJECT (sample));
  combobox->sample = sample;
}

GtkWidget *
egg_combo_box_get_popup_widget (EggComboBox *combobox)
{
  g_return_val_if_fail (EGG_IS_COMBO_BOX (combobox), NULL);

  if (GTK_IS_MENU (combobox->popup_window))
    return combobox->popup_window;

  return combobox->popup;
}

static void
egg_combo_box_menu_show (GtkWidget *menu,
                         gpointer   user_data)
{
  EggComboBox *combobox = EGG_COMBO_BOX (user_data);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combobox->button),
                                TRUE);
}

static void
egg_combo_box_menu_hide (GtkWidget *menu,
                         gpointer   user_data)
{
  EggComboBox *combobox = EGG_COMBO_BOX (user_data);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combobox->button),
                                FALSE);
}

void
egg_combo_box_set_popup_widget (EggComboBox *combobox,
				GtkWidget   *popup)
{
  g_return_if_fail (EGG_IS_COMBO_BOX (combobox));
  g_return_if_fail (GTK_IS_WIDGET (popup));

  if (combobox->popup)
    {
      gtk_container_remove (GTK_CONTAINER (combobox->frame),
			    combobox->popup);
      g_object_unref (G_OBJECT (combobox->popup));
      combobox->popup = NULL;
    }

  if (GTK_IS_MENU (combobox->popup_window))
    combobox->popup_window = NULL;

  if (GTK_IS_MENU (popup))
    {
      if (combobox->popup_window)
	egg_combo_box_destroy_popup (combobox);

      combobox->popup_window = popup;

      g_signal_connect (popup, "show",
                        G_CALLBACK (egg_combo_box_menu_show), combobox);
      g_signal_connect (popup, "hide",
                        G_CALLBACK (egg_combo_box_menu_hide), combobox);

      /* FIXME: need to attach to widget? */

      return;
    }

  if (!combobox->popup_window)
    egg_combo_box_setup_popup (combobox);

  gtk_container_add (GTK_CONTAINER (combobox->frame), popup);
  gtk_widget_show (popup);
  g_object_ref (G_OBJECT (popup));
  combobox->popup = popup;
}

static void
egg_combo_box_menu_position (GtkMenu  *menu,
                             gint     *x,
			     gint     *y,
			     gboolean *push_in,
			     gpointer  user_data)
{
  gint sx, sy;
  gboolean deugh = FALSE;
  GtkRequisition req;
  EggComboBox *combobox = EGG_COMBO_BOX (user_data);

  /* FIXME: is using the size request here broken? */

  /* ugh */
  if (!combobox->sample)
    {
      combobox->sample = combobox->button;
      deugh = TRUE;
    }

  gdk_window_get_origin (combobox->sample->window, &sx, &sy);

  gtk_widget_size_request (GTK_WIDGET (menu), &req);

  *x = sx + combobox->sample->allocation.width - req.width;
  *y = sy + combobox->sample->allocation.height;

  if (GTK_WIDGET_NO_WINDOW (combobox->sample))
    {
      *x += combobox->sample->allocation.x;
      *y += combobox->sample->allocation.y;
    }

  if (deugh)
    combobox->sample = NULL;

  *push_in = TRUE;
}

void
egg_combo_box_popup (EggComboBox *combobox)
{
  gint x, y, width, height;

  g_return_if_fail (EGG_IS_COMBO_BOX (combobox));

  if (GTK_WIDGET_MAPPED (combobox->popup_window))
    return;

  if (GTK_IS_MENU (combobox->popup_window))
    {
      gtk_menu_popup (GTK_MENU (combobox->popup_window),
                      NULL, NULL,
                      egg_combo_box_menu_position, combobox,
		      0, 0);
      return;
    }

  /* size it */
  width = combobox->sample->allocation.width;
  height = combobox->sample->allocation.height;

  gdk_window_get_origin (combobox->sample->window,
			 &x, &y);
  gtk_widget_set_size_request (combobox->popup_window,
			       width, -1);

  if (GTK_WIDGET_NO_WINDOW (combobox->sample))
    {
      x += combobox->sample->allocation.x;
      y += combobox->sample->allocation.y;
    }

  gtk_window_move (GTK_WINDOW (combobox->popup_window),
		   x,
		   y + height);

  /* popup */
  gtk_widget_show_all (combobox->popup_window);

  gtk_widget_grab_focus (combobox->popup_window);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combobox->button), TRUE);

  g_signal_emit (combobox, combo_signals[POPPED_UP], 0);
}

void
egg_combo_box_popdown (EggComboBox *combobox)
{
  g_return_if_fail (EGG_IS_COMBO_BOX (combobox));

  if (GTK_IS_MENU (combobox->popup_window))
    {
      gtk_menu_popdown (GTK_MENU (combobox->popup_window));
      return;
    }

  gtk_widget_hide (combobox->popup_window);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (combobox->button), FALSE);

  g_signal_emit (combobox, combo_signals[POPPED_DOWN], 0);
}
