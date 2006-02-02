/* eggiconchooserdialog.c
 * Copyright (C) 2004  James M. Cape  <jcape@ignore-your.tv>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdarg.h>

#include "gtkfilechooserembed.h"
#include "eggiconchooserutils.h"
#include "eggiconchooserwidget.h"
#include "eggiconchooserdialog.h"


/* **************** *
 *  Private Macros  *
 * **************** */

#define EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE(object) (EGG_ICON_CHOOSER_DIALOG (object)->priv)


/* *************** *
 *  Private Types  *
 * *************** */

struct _EggIconChooserDialogPrivate
{
  GtkWidget *widget;
  gchar *file_system;

  gint default_width;
  gint default_height;
  gboolean resize_horizontally : 1;
  gboolean resize_vertically   : 1;
};


/* ********************* *
 *  Function Prototypes  *
 * ********************* */

/* GObject */
static GObject *egg_icon_chooser_dialog_constructor  (GType                  type,
				                      guint                  n_params,
				                      GObjectConstructParam *params);
static void     egg_icon_chooser_dialog_set_property (GObject               *object,
						      guint                  param_id,
						      const GValue          *value,
						      GParamSpec            *pspec);
static void     egg_icon_chooser_dialog_get_property (GObject               *object,
						      guint                  param_id,
						      GValue                *value,
						      GParamSpec            *pspec);

/* GtkWidget */
static void egg_icon_chooser_dialog_style_set (GtkWidget *widget,
					       GtkStyle  *previous_style);
static void egg_icon_chooser_dialog_show_all  (GtkWidget *widget);
static void egg_icon_chooser_dialog_hide_all  (GtkWidget *widget);

/* GtkDialog */
static void egg_icon_chooser_dialog_response (GtkDialog *dialog,
					      gint       response_id,
					      gpointer   user_data);

/* Widget Callbacks */
static void widget_default_size_changed (GtkFileChooserEmbed *chooser_widget,
					 gpointer             user_data);
static void widget_item_activated       (EggIconChooser      *chooser_widget,
					 gpointer             user_data);


/* ******************* *
 *  GType Declaration  *
 * ******************* */

G_DEFINE_TYPE_WITH_CODE (EggIconChooserDialog, egg_icon_chooser_dialog, GTK_TYPE_DIALOG, ({\
  G_IMPLEMENT_INTERFACE (EGG_TYPE_ICON_CHOOSER, _egg_icon_chooser_delegate_iface_init) \
}));


/* ***************** *
 *  GType Functions  *
 * ***************** */

static void
egg_icon_chooser_dialog_class_init (EggIconChooserDialogClass *class)
{
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class = G_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);

  gobject_class->constructor = egg_icon_chooser_dialog_constructor;
  gobject_class->set_property = egg_icon_chooser_dialog_set_property;
  gobject_class->get_property = egg_icon_chooser_dialog_get_property;

  widget_class->style_set = egg_icon_chooser_dialog_style_set;
  widget_class->hide_all = egg_icon_chooser_dialog_hide_all;
  widget_class->show_all = egg_icon_chooser_dialog_show_all;

  _egg_icon_chooser_install_properties (gobject_class);

  g_type_class_add_private (class, sizeof (EggIconChooserDialogPrivate));
}

static void
egg_icon_chooser_dialog_init (EggIconChooserDialog *dialog)
{
  dialog->priv = G_TYPE_INSTANCE_GET_PRIVATE (dialog,
					      EGG_TYPE_ICON_CHOOSER_DIALOG,
					      EggIconChooserDialogPrivate);

  gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
  g_signal_connect (dialog, "response",
		    G_CALLBACK (egg_icon_chooser_dialog_response), NULL);
}


/* ******************* *
 *  GObject Functions  *
 * ******************* */

static GObject *
egg_icon_chooser_dialog_constructor (GType                  type,
				     guint                  n_params,
				     GObjectConstructParam *params)
{
  GObject *object;
  EggIconChooserDialogPrivate *priv;

  object = (*G_OBJECT_CLASS (egg_icon_chooser_dialog_parent_class)->constructor) (type,
										  n_params,
										  params);
  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (object);

  gtk_widget_push_composite_child ();

  if (priv->file_system)
    priv->widget = egg_icon_chooser_widget_new_with_backend (priv->file_system);
  else
    priv->widget = egg_icon_chooser_widget_new ();

  g_free (priv->file_system);
  priv->file_system = NULL;

  g_signal_connect (priv->widget, "file-activated",
                    G_CALLBACK (widget_item_activated), object);
  g_signal_connect (priv->widget, "icon-activated",
                    G_CALLBACK (widget_item_activated), object);
  g_signal_connect (priv->widget, "default-size-changed",
                    G_CALLBACK (widget_default_size_changed), object);

  _egg_icon_chooser_set_delegate (EGG_ICON_CHOOSER (object),
				  EGG_ICON_CHOOSER (priv->widget));
  _gtk_file_chooser_embed_initial_focus (GTK_FILE_CHOOSER_EMBED (priv->widget));

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (object)->vbox), priv->widget);
  gtk_widget_show (priv->widget);

  gtk_widget_pop_composite_child ();

  return object;
}

static void
egg_icon_chooser_dialog_set_property (GObject      *object,
				      guint         param_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  EggIconChooserDialogPrivate *priv;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (object);

  switch (param_id)
    {
    case EGG_ICON_CHOOSER_PROP_FILE_SYSTEM:
      priv->file_system = g_value_dup_string (value);
      break;
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_set_property (G_OBJECT (priv->widget), pspec->name, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
egg_icon_chooser_dialog_get_property (GObject    *object,
				      guint       param_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
  EggIconChooserDialogPrivate *priv;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (object);

  switch (param_id)
    {
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_get_property (G_OBJECT (priv->widget), pspec->name, value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}


/* ********************* *
 *  GtkWidget Functions  *
 * ********************* */

static void
egg_icon_chooser_dialog_style_set (GtkWidget *widget,
				   GtkStyle  *previous_style)
{
  GtkDialog *dialog;

  if (GTK_WIDGET_CLASS (egg_icon_chooser_dialog_parent_class)->style_set)
    (*GTK_WIDGET_CLASS (egg_icon_chooser_dialog_parent_class)->style_set) (widget,
									   previous_style);

  dialog = GTK_DIALOG (widget);

  /* Override the style properties with HIG-compliant spacings.  Ugh.
   * http://developer.gnome.org/projects/gup/hig/1.0/layout.html#layout-dialogs
   * http://developer.gnome.org/projects/gup/hig/1.0/windows.html#alert-spacing
   */

  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox), 12);
  gtk_box_set_spacing (GTK_BOX (dialog->vbox), 24);

  gtk_container_set_border_width (GTK_CONTAINER (dialog->action_area), 0);
  gtk_box_set_spacing (GTK_BOX (dialog->action_area), 6);
}

static void
egg_icon_chooser_dialog_show_all (GtkWidget *widget)
{
  gtk_widget_show (widget);
}

static void
egg_icon_chooser_dialog_hide_all (GtkWidget *widget)
{
  gtk_widget_hide (widget);
}


/* ********************* *
 *  GtkDialog Functions  *
 * ********************* */

static void
egg_icon_chooser_dialog_response (GtkDialog *dialog,
				  gint       response_id,
				  gpointer   user_data)
{
  EggIconChooserDialogPrivate *priv;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (dialog);

  /* Act only on response IDs we recognize */
  if (!(response_id == GTK_RESPONSE_ACCEPT
        || response_id == GTK_RESPONSE_OK
        || response_id == GTK_RESPONSE_YES
        || response_id == GTK_RESPONSE_APPLY))
    return;

  if (!_gtk_file_chooser_embed_should_respond (GTK_FILE_CHOOSER_EMBED (priv->widget)))
    g_signal_stop_emission_by_name (dialog, "response");

}


/* ******************** *
 *  Callback Functions  *
 * ******************** */

static void
update_geometry_hints (GtkWindow *dialog)
{
  EggIconChooserDialogPrivate *priv;
  GdkGeometry geometry;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (dialog);

  geometry.min_width = -1;
  geometry.min_height = -1;
  geometry.max_width = (priv->resize_horizontally ? G_MAXSHORT : -1);
  geometry.max_height = (priv->resize_vertically ? G_MAXSHORT : -1);

  gtk_window_set_geometry_hints (dialog, NULL, &geometry,
                                 (GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
}

static inline void
clamp_to_screen (GtkWidget *widget,
                 gint      *width,
                 gint      *height)
{
  GdkScreen *screen;
  int monitor_num;
  GdkRectangle monitor;

  g_return_if_fail (GTK_WIDGET_REALIZED (widget));

  screen = gtk_widget_get_screen (widget);
  monitor_num = gdk_screen_get_monitor_at_window (screen, widget->window);

  gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

  if (width)
    *width = MIN (*width, (monitor.width * 3) / 4);

  if (height)
    *height = MIN (*height, (monitor.height * 3) / 4);
}

static inline void
widget_realized_default_size_changed (GtkFileChooserEmbed *embed,
				      gpointer             user_data)
{
  EggIconChooserDialogPrivate *priv;
  GtkRequisition req;
  gint current_width, current_height;
  gint default_width, default_height;
  gint width, height;
  gint diff_x, diff_y;
  gboolean update_hints;
  gboolean resize_horizontally, resize_vertically;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (user_data);
  update_hints = FALSE;
  diff_x = 0;
  diff_y = 0;
  resize_horizontally = FALSE;
  resize_vertically = FALSE;

  gtk_widget_size_request (user_data, &req);
  gtk_window_get_size (user_data, &current_width, &current_height);
  _gtk_file_chooser_embed_get_default_size (embed, &default_width,
					    &default_height);

  width = (req.width - priv->widget->requisition.width) + default_width;
  height = (req.height - priv->widget->requisition.height) + default_height;

  _gtk_file_chooser_embed_get_resizable_hints (GTK_FILE_CHOOSER_EMBED (priv->widget),
                                               &resize_horizontally,
                                               &resize_vertically);
  resize_vertically = (!! resize_vertically);     /* normalize */
  resize_horizontally = (!! resize_horizontally);

  if (resize_horizontally && priv->resize_horizontally)
    {
      diff_x = default_width - priv->default_width;
      priv->default_width = default_width;
    }
  else if (resize_horizontally && ! priv->resize_horizontally)
    {
      /* We restore to the ideal size + any change in default_size (which is not
       * expected).  It would be nicer to store the older size to restore to in
       * the future. */
      diff_x = default_width - priv->default_width;
      diff_x += width - current_width;
      priv->default_width = default_width;
      update_hints = TRUE;
    }
  else
    {
      update_hints = TRUE;
    }

  if (resize_vertically && priv->resize_vertically)
    {
      diff_y = default_height - priv->default_height;
      priv->default_height = default_height;
    }
  else if (resize_vertically && ! priv->resize_vertically)
    {
      diff_y = default_height - priv->default_height;
      diff_y += height - current_height;
      priv->default_height = default_height;
      update_hints = TRUE;
    }
  else
    {
      update_hints = TRUE;
    }

  priv->resize_horizontally = (resize_horizontally != FALSE);
  priv->resize_vertically = (resize_vertically != FALSE);

  if (diff_x != 0 || diff_y != 0)
    {
      gint new_width = current_width + diff_x;
      gint new_height = current_height + diff_y;

      clamp_to_screen (user_data, &new_width, &new_height);

      gtk_window_resize (user_data, new_width, new_height);
    }

  /* Only store the size if we can resize in that direction. */
  if (update_hints)
    update_geometry_hints (user_data);
}

static inline void
widget_unrealized_default_size_changed (GtkFileChooserEmbed *embed,
				        gpointer             user_data)
{
  EggIconChooserDialogPrivate *priv;
  GtkRequisition req;
  gint width, height;
  gboolean resize_horizontally, resize_vertically;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (user_data);

  gtk_widget_size_request (user_data, &req);

  _gtk_file_chooser_embed_get_resizable_hints (embed,
					       &resize_horizontally,
					       &resize_vertically);
  _gtk_file_chooser_embed_get_default_size (embed, &(priv->default_width),
					    &(priv->default_height));

  priv->resize_horizontally = (resize_horizontally != FALSE);
  priv->resize_vertically = (resize_vertically != FALSE);

  width = priv->default_width + req.width - priv->widget->requisition.width;
  height = priv->default_height + req.height - priv->widget->requisition.height;

  gtk_window_set_default_size (user_data, width, height);
  update_geometry_hints (user_data);
}

static void
widget_default_size_changed (GtkFileChooserEmbed *embed,
			     gpointer             user_data)
{
  if (GTK_WIDGET_REALIZED (embed))
    widget_realized_default_size_changed (embed, user_data);
  else
    widget_unrealized_default_size_changed (embed, user_data);
}

static void
widget_item_activated (EggIconChooser *chooser,
		       gpointer        user_data)
{
  EggIconChooserDialogPrivate *priv;
  GList *list;

  priv = EGG_ICON_CHOOSER_DIALOG_GET_PRIVATE (user_data);

  if (gtk_window_activate_default (GTK_WINDOW (user_data)))
    return;

  for (list = gtk_container_get_children (GTK_CONTAINER (GTK_DIALOG (user_data)->action_area));
       list != NULL;
       list = g_list_remove_link (list, list))
    {
      GtkWidget *widget;
      gint *response_id;

      widget = list->data;

      /* This will break if GtkDialog changes the contents of the "ResponseData"
	 struct, so if you get funky shit, check gtkdialog.c. */
      response_id = g_object_get_data (list->data, "gtk-dialog-response-data");

      if (response_id &&
	  (*response_id == GTK_RESPONSE_ACCEPT ||
	   *response_id == GTK_RESPONSE_OK ||
	   *response_id == GTK_RESPONSE_YES ||
	   *response_id == GTK_RESPONSE_APPLY))
	{
	  gtk_widget_activate (widget);
	  break;
	}
    }

  /* Free any remaining items -- though if apps follow the HIG there shouldn't
     be any :-) */
  g_list_free (list);
}


/* ******************* *
 *  Utility Functions  *
 * ******************* */

static GtkWidget *
egg_icon_chooser_dialog_new_valist (const gchar *title,
				    GtkWindow   *parent,
				    const gchar *backend,
				    const gchar *first_button_text,
				    va_list      varargs)
{
  GtkWidget *retval;
  const gchar *button_text;
  gint response_id;

  retval = g_object_new (EGG_TYPE_ICON_CHOOSER_DIALOG,
			 "title", title,
			 "file-system-backend", backend,
			 NULL);

  if (parent)
    gtk_window_set_transient_for (GTK_WINDOW (retval), parent);
  
  button_text = first_button_text;
  while (button_text)
    {
      response_id = va_arg (varargs, gint);
      gtk_dialog_add_button (GTK_DIALOG (retval), button_text, response_id);
      button_text = va_arg (varargs, const gchar *);
    }

  return retval;
}


/* ************************************************************************** *
 *  Public API                                                                *
 * ************************************************************************** */

GtkWidget *
egg_icon_chooser_dialog_new (const gchar *title,
			     GtkWindow   *parent,
			     const gchar *first_button_text,
			     ...)
{
  GtkWidget *retval;
  va_list varargs;

  g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

  va_start (varargs, first_button_text);
  retval = egg_icon_chooser_dialog_new_valist (title, parent, NULL,
					       first_button_text, varargs);
  va_end (varargs);

  return retval;
}

GtkWidget *
egg_icon_chooser_dialog_new_with_backend (const gchar *title,
					  GtkWindow   *parent,
					  const gchar *file_system_backend,
					  const gchar *first_button_text,
					  ...)
{
  GtkWidget *retval;
  va_list varargs;

  g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

  va_start (varargs, first_button_text);
  retval = egg_icon_chooser_dialog_new_valist (title, parent,
					       file_system_backend,
					       first_button_text, varargs);
  va_end (varargs);

  return retval;
}
