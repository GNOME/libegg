/* GTK - The GIMP Toolkit
 * eggrecentchooserdialog.c: Recent files selector dialog
 * Copyright (C) 2005 Emmanuele Bassi
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include "eggrecentchooserdialog.h"
#include "eggrecentchooserwidget.h"
#include "eggrecentchooserutils.h"
#include "eggrecentmanager.h"
#include "eggrecenttypebuiltins.h"

#include <stdarg.h>

struct _EggRecentChooserDialogPrivate
{
  EggRecentManager *manager;
  
  GtkWidget *chooser;
};

#define EGG_RECENT_CHOOSER_DIALOG_GET_PRIVATE(obj)	(EGG_RECENT_CHOOSER_DIALOG (obj)->priv)

static void egg_recent_chooser_dialog_class_init (EggRecentChooserDialogClass *klass);
static void egg_recent_chooser_dialog_init       (EggRecentChooserDialog      *dialog);
static void egg_recent_chooser_dialog_finalize   (GObject                     *object);

static GObject *egg_recent_chooser_dialog_constructor (GType                  type,
						       guint                  n_construct_properties,
						       GObjectConstructParam *construct_params);

static void egg_recent_chooser_dialog_set_property (GObject      *object,
						    guint         prop_id,
						    const GValue *value,
						    GParamSpec   *pspec);
static void egg_recent_chooser_dialog_get_property (GObject      *object,
						    guint         prop_id,
						    GValue       *value,
						    GParamSpec   *pspec);

static void egg_recent_chooser_dialog_map       (GtkWidget *widget);
static void egg_recent_chooser_dialog_unmap     (GtkWidget *widget);
static void egg_recent_chooser_dialog_style_set (GtkWidget *widget,
						 GtkStyle  *old_style);


G_DEFINE_TYPE_WITH_CODE (EggRecentChooserDialog,
			 egg_recent_chooser_dialog,
			 GTK_TYPE_DIALOG,
			 G_IMPLEMENT_INTERFACE (EGG_TYPE_RECENT_CHOOSER,
		       				_egg_recent_chooser_delegate_iface_init));

static void
egg_recent_chooser_dialog_class_init (EggRecentChooserDialogClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  
  gobject_class->set_property = egg_recent_chooser_dialog_set_property;
  gobject_class->get_property = egg_recent_chooser_dialog_get_property;
  gobject_class->constructor = egg_recent_chooser_dialog_constructor;
  gobject_class->finalize = egg_recent_chooser_dialog_finalize;
  
  widget_class->map = egg_recent_chooser_dialog_map;
  widget_class->unmap = egg_recent_chooser_dialog_unmap;
  widget_class->style_set = egg_recent_chooser_dialog_style_set;
  
  _egg_recent_chooser_install_properties (gobject_class);
  
  g_type_class_add_private (klass, sizeof (EggRecentChooserDialogPrivate));
}

static void
egg_recent_chooser_dialog_init (EggRecentChooserDialog *dialog)
{
  EggRecentChooserDialogPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE (dialog,
  								     EGG_TYPE_RECENT_CHOOSER_DIALOG,
  								     EggRecentChooserDialogPrivate);
  
  dialog->priv = priv;
  
  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
}

/* we intercept the EggRecentChooser::item_activated signal and try to
 * make the dialog emit a valid response signal
 */
static void
egg_recent_chooser_item_activated_cb (EggRecentChooser *chooser,
				      gpointer          user_data)
{
  EggRecentChooserDialog *dialog;
  GList *children, *l;

  dialog = EGG_RECENT_CHOOSER_DIALOG (user_data);

  if (gtk_window_activate_default (GTK_WINDOW (dialog)))
    return;
  
  children = gtk_container_get_children (GTK_CONTAINER (GTK_DIALOG (dialog)->action_area));
  
  for (l = children; l; l = l->next)
    {
      GtkWidget *widget;
      gint response_id;
      
      widget = GTK_WIDGET (l->data);
      response_id = gtk_dialog_get_response_for_widget (GTK_DIALOG (dialog), widget);
      
      if (response_id == GTK_RESPONSE_ACCEPT ||
          response_id == GTK_RESPONSE_OK     ||
          response_id == GTK_RESPONSE_YES    ||
          response_id == GTK_RESPONSE_APPLY)
        {
          g_list_free (children);
	  
          gtk_dialog_response (GTK_DIALOG (dialog), response_id);

          return;
        }
    }
  
  g_list_free (children);
}

static GObject *
egg_recent_chooser_dialog_constructor (GType                  type,
				       guint                  n_construct_properties,
				       GObjectConstructParam *construct_params)
{
  GObject *object;
  EggRecentChooserDialogPrivate *priv;
  
  object = G_OBJECT_CLASS (egg_recent_chooser_dialog_parent_class)->constructor (type,
		  							         n_construct_properties,
										 construct_params);
  priv = EGG_RECENT_CHOOSER_DIALOG_GET_PRIVATE (object);
  
  gtk_widget_push_composite_child ();
  
  if (priv->manager)
    priv->chooser = g_object_new (EGG_TYPE_RECENT_CHOOSER_WIDGET,
  				  "recent-manager", priv->manager,
  				  NULL);
  else
    priv->chooser = g_object_new (EGG_TYPE_RECENT_CHOOSER_WIDGET, NULL);
  
  g_signal_connect (priv->chooser, "item-activated",
  		    G_CALLBACK (egg_recent_chooser_item_activated_cb),
  		    object);
  
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (object)->vbox),
                      priv->chooser, TRUE, TRUE, 0);
  gtk_widget_show (priv->chooser);
  
  _egg_recent_chooser_set_delegate (EGG_RECENT_CHOOSER (object),
  				    EGG_RECENT_CHOOSER (priv->chooser));
  
  gtk_widget_pop_composite_child ();
  
  return object;
}

static void
egg_recent_chooser_dialog_set_property (GObject      *object,
					guint         prop_id,
					const GValue *value,
					GParamSpec   *pspec)
{
  EggRecentChooserDialogPrivate *priv;
  
  priv = EGG_RECENT_CHOOSER_DIALOG_GET_PRIVATE (object);
  
  switch (prop_id)
    {
    case EGG_RECENT_CHOOSER_PROP_RECENT_MANAGER:
      if (priv->manager)
        g_object_unref (priv->manager);
      priv->manager = g_value_get_object (value);
      if (priv->manager)
        g_object_ref (priv->manager);
      break;
    default:
      g_object_set_property (G_OBJECT (priv->chooser), pspec->name, value);
      break;
    }
}

static void
egg_recent_chooser_dialog_get_property (GObject      *object,
					guint         prop_id,
					GValue       *value,
					GParamSpec   *pspec)
{
  EggRecentChooserDialogPrivate *priv;
  
  priv = EGG_RECENT_CHOOSER_DIALOG_GET_PRIVATE (object);
  
  g_object_get_property (G_OBJECT (priv->chooser), pspec->name, value);
}

static void
egg_recent_chooser_dialog_finalize (GObject *object)
{
  EggRecentChooserDialog *dialog = EGG_RECENT_CHOOSER_DIALOG (object);
  
  if (dialog->priv->manager)
    g_object_unref (G_OBJECT (dialog->priv->manager));
  
  G_OBJECT_CLASS (egg_recent_chooser_dialog_parent_class)->finalize (object);
}

static void
egg_recent_chooser_dialog_map (GtkWidget *widget)
{
  EggRecentChooserDialog *dialog = EGG_RECENT_CHOOSER_DIALOG (widget);
  EggRecentChooserDialogPrivate *priv = dialog->priv;
  
  if (!GTK_WIDGET_MAPPED (priv->chooser))
    gtk_widget_map (priv->chooser);

  GTK_WIDGET_CLASS (egg_recent_chooser_dialog_parent_class)->map (widget);
}

static void
egg_recent_chooser_dialog_unmap (GtkWidget *widget)
{
  EggRecentChooserDialog *dialog = EGG_RECENT_CHOOSER_DIALOG (widget);
  EggRecentChooserDialogPrivate *priv = dialog->priv;
  
  GTK_WIDGET_CLASS (egg_recent_chooser_dialog_parent_class)->unmap (widget);
  
  gtk_widget_unmap (priv->chooser);
}

/* taken from gtkfilechooserdialog.c */
static void
egg_recent_chooser_dialog_style_set (GtkWidget *widget,
				     GtkStyle  *old_style)
{
  GtkDialog *dialog;

  dialog = GTK_DIALOG (widget);

  /* Override the style properties with HIG-compliant spacings.  Ugh.
   * http://developer.gnome.org/projects/gup/hig/1.0/layout.html#layout-dialogs
   * http://developer.gnome.org/projects/gup/hig/1.0/windows.html#alert-spacing
   */

  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox), 12);
  gtk_box_set_spacing (GTK_BOX (dialog->vbox), 24);

  gtk_container_set_border_width (GTK_CONTAINER (dialog->action_area), 0);
  gtk_box_set_spacing (GTK_BOX (dialog->action_area), 6);
  
  if (GTK_WIDGET_CLASS (egg_recent_chooser_dialog_parent_class)->style_set)
    GTK_WIDGET_CLASS (egg_recent_chooser_dialog_parent_class)->style_set (widget, old_style);
}

static GtkWidget *
egg_recent_chooser_dialog_new_valist (const gchar      *title,
				      GtkWindow        *parent,
				      EggRecentManager *manager,
				      const gchar      *first_button_text,
				      va_list           varargs)
{
  GtkWidget *result;
  const char *button_text = first_button_text;
  gint response_id;
  
  result = g_object_new (EGG_TYPE_RECENT_CHOOSER_DIALOG,
                         "title", title,
                         "recent-manager", manager,
                         NULL);
  
  if (parent)
    gtk_window_set_transient_for (GTK_WINDOW (result), parent);
  
  while (button_text)
    {
      response_id = va_arg (varargs, gint);
      gtk_dialog_add_button (GTK_DIALOG (result), button_text, response_id);
      button_text = va_arg (varargs, const gchar *);
    }
  
  return result;
}

/**
 * egg_recent_chooser_dialog_new:
 * @title: Title of the dialog, or %NULL
 * @parent: Transient parent of the dialog, or %NULL,
 * @first_button_text: stock ID or text to go in the first button, or %NULL
 * @Varargs: response ID for the first button, then additional (button, id)
 *   pairs, ending with %NULL
 *
 * Creates a new #EggRecentChooserDialog.  This function is analogous to
 * gtk_dialog_new_with_buttons().
 *
 * Return value: a new #EggRecentChooserDialog
 *
 * Since: 2.10
 */
GtkWidget *
egg_recent_chooser_dialog_new (const gchar *title,
			       GtkWindow   *parent,
			       const gchar *first_button_text,
			       ...)
{
  GtkWidget *result;
  va_list varargs;
  
  va_start (varargs, first_button_text);
  result = egg_recent_chooser_dialog_new_valist (title,
  						 parent,
  						 NULL,
  						 first_button_text,
  						 varargs);
  va_end (varargs);
  
  return result;
}

/**
 * egg_recent_chooser_dialog_new_for_manager:
 * @title: Title of the dialog, or %NULL
 * @parent: Transient parent of the dialog, or %NULL,
 * @manager: a #EggRecentManager
 * @first_button_text: stock ID or text to go in the first button, or %NULL
 * @Varargs: response ID for the first button, then additional (button, id)
 *   pairs, ending with %NULL
 *
 * Creates a new #EggRecentChooserDialog with a specified recent manager.
 *
 * This is useful if you have implemented your own recent manager, or if you
 * have a customized instance of a #EggRecentManager object (e.g. with your
 * own sorting and/or filtering functions).
 *
 * Return value: a new #EggRecentChooserDialog
 *
 * Since: 2.10
 */
GtkWidget *
egg_recent_chooser_dialog_new_for_manager (const gchar      *title,
			                   GtkWindow        *parent,
			                   EggRecentManager *manager,
			                   const gchar      *first_button_text,
			                   ...)
{
  GtkWidget *result;
  va_list varargs;
  
  va_start (varargs, first_button_text);
  result = egg_recent_chooser_dialog_new_valist (title,
  						 parent,
  						 manager,
  						 first_button_text,
  						 varargs);
  va_end (varargs);
  
  return result;
}
