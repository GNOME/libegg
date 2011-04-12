/* eggiconchooserbutton.c
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

#include <glib/gi18n.h>

#include <string.h>

#include <gtk/gtk.h>

#include "eggiconchooserutils.h"
#include "eggiconchooserdialog.h"

#include "eggiconchooserbutton.h"


/* **************** *
 *  Private Macros  *
 * **************** */

#define EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE(object) (EGG_ICON_CHOOSER_BUTTON (object)->priv)

#define MINIMUM_ICON_SIZE	16
#define FALLBACK_ICON_SIZE	48


/* ********************** *
 *  Private Enumerations  *
 * ********************** */

enum
{
  PROP_0,
  PROP_DIALOG,
  PROP_TITLE
};


/* *************** *
 *  Private Types  *
 * *************** */

typedef struct _EggIconChooserData EggIconChooserData;
struct _EggIconChooserData
{
  gchar *title;

  GtkFileSystem *fs;
  gchar *icon;
  gchar *file;

  guint8 context : 5;
};

struct _EggIconChooserButtonPrivate
{
  GtkWidget *button;
  GtkWidget *image;
  GtkWidget *dialog;
  gchar *file_system;

  guint8 icon_is_file   : 1;
  guint8 icon_is_themed : 1;
  guint8 active         : 1;
};


/* ********************* *
 *  Function Prototypes  *
 * ********************* */

/* GObject */
static GObject *egg_icon_chooser_button_constructor  (GType                  type,
						      guint                  n_params,
						      GObjectConstructParam *params);
static void     egg_icon_chooser_button_set_property (GObject               *object,
						      guint                  param_id,
						      const GValue          *value,
						      GParamSpec            *pspec);
static void     egg_icon_chooser_button_get_property (GObject               *object,
						      guint                  param_id,
						      GValue                *value,
						      GParamSpec            *pspec);

/* GtkObject */
static void egg_icon_chooser_button_destroy (GtkObject *object);

/* GtkWidget */
static void egg_icon_chooser_button_style_set      (GtkWidget *widget,
						    GtkStyle  *previous_style);
static void egg_icon_chooser_button_screen_changed (GtkWidget *widget,
						    GdkScreen *previous_screen);
static void egg_icon_chooser_button_show_all       (GtkWidget *widget);
static void egg_icon_chooser_button_hide_all       (GtkWidget *widget);
static void egg_icon_chooser_button_show           (GtkWidget *widget);
static void egg_icon_chooser_button_hide           (GtkWidget *widget);

/* Button Callbacks */
static void button_clicked_cb (GtkButton *button,
			       gpointer   user_data);

/* Dialog Callbacks */
static void     dialog_notify_cb         (GObject        *object,
					  GParamSpec     *pspec,
					  gpointer        user_data);
static gboolean dialog_delete_event_cb   (GtkWidget      *dialog,
					  GdkEvent       *event,
					  gpointer        user_data);
static void     dialog_response_cb       (GtkDialog      *dialog,
					  gint            response,
					  gpointer        user_data);
static void     dialog_file_activated_cb (EggIconChooser *dialog,
					  gpointer        user_data);
static void     dialog_icon_activated_cb (EggIconChooser *dialog,
					  gpointer        user_data);

/* Utility Functions */
static void reload_icon (EggIconChooserButton *button);


/* ******************* *
 *  GType Declaration  *
 * ******************* */

G_DEFINE_TYPE_WITH_CODE (EggIconChooserButton, egg_icon_chooser_button, GTK_TYPE_VBOX, ({\
  G_IMPLEMENT_INTERFACE (EGG_TYPE_ICON_CHOOSER, _egg_icon_chooser_delegate_iface_init) \
}));


/* ***************** *
 *  GType Functions  *
 * ***************** */

static void
egg_icon_chooser_button_class_init (EggIconChooserButtonClass *class)
{
  GObjectClass *gobject_class;
  GtkObjectClass *gtkobject_class;
  GtkWidgetClass *widget_class;

  gobject_class = G_OBJECT_CLASS (class);
  gtkobject_class = GTK_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);

  gobject_class->constructor = egg_icon_chooser_button_constructor;
  gobject_class->set_property = egg_icon_chooser_button_set_property;
  gobject_class->get_property = egg_icon_chooser_button_get_property;

  gtkobject_class->destroy = egg_icon_chooser_button_destroy;

  widget_class->style_set = egg_icon_chooser_button_style_set;
  widget_class->screen_changed = egg_icon_chooser_button_screen_changed;
  widget_class->show_all = egg_icon_chooser_button_show_all;
  widget_class->hide_all = egg_icon_chooser_button_hide_all;
  widget_class->show = egg_icon_chooser_button_show;
  widget_class->hide = egg_icon_chooser_button_hide;

  _egg_icon_chooser_install_properties (gobject_class);

  /**
   * EggIconChooserButton:dialog
   * 
   * The EggIconChooserDialog (or subclass) to use for the button's underlying
   * dialog widget.
   * 
   * Since: 2.8
   **/
  g_object_class_install_property (gobject_class,
				   PROP_DIALOG,
				   g_param_spec_object ("dialog",
						        _("Dialog"),
						        _("The EggIconChooserDialog (or subclass) to use."),
							EGG_TYPE_ICON_CHOOSER_DIALOG,
							(G_PARAM_WRITABLE |
							 G_PARAM_CONSTRUCT_ONLY)));
  /**
   * EggIconChooserButton:title
   * 
   * The window title of the button's chooser dialog.
   * 
   * Since: 2.8
   **/
  g_object_class_install_property (gobject_class,
				   PROP_TITLE,
				   g_param_spec_string ("title",
						        _("Title"),
						        _("The title of the chooser dialog."),
							NULL,
							G_PARAM_READWRITE));

  g_type_class_add_private (class, sizeof (EggIconChooserButton));
}

static void
egg_icon_chooser_button_init (EggIconChooserButton *button)
{
  button->priv = G_TYPE_INSTANCE_GET_PRIVATE (button,
					      EGG_TYPE_ICON_CHOOSER_BUTTON,
					      EggIconChooserButtonPrivate);

  gtk_widget_push_composite_child ();

  button->priv->button = gtk_button_new ();
  g_signal_connect (button->priv->button, "clicked",
		    G_CALLBACK (button_clicked_cb), button);
  gtk_box_pack_start (GTK_BOX (button), button->priv->button, FALSE, FALSE, 0);
  gtk_widget_show (button->priv->button);

  button->priv->image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (button->priv->button), button->priv->image);
  gtk_widget_show (button->priv->image);

  gtk_widget_pop_composite_child ();
}


/* ******************* *
 *  GObject Functions  *
 * ******************* */

static GObject *
egg_icon_chooser_button_constructor (GType                  type,
				     guint                  n_params,
				     GObjectConstructParam *params)
{
  GObject *object;
  EggIconChooserButtonPrivate *priv;

  object = (*G_OBJECT_CLASS (egg_icon_chooser_button_parent_class)->constructor) (type,
										  n_params,
										  params);
  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (object);

  if (!priv->dialog)
    {
      if (priv->file_system)
	priv->dialog = g_object_new (EGG_TYPE_ICON_CHOOSER_DIALOG,
				     "file-system-backend", priv->file_system,
				     NULL);
      else
	priv->dialog = g_object_new (EGG_TYPE_ICON_CHOOSER_DIALOG, NULL);

      gtk_dialog_add_button (GTK_DIALOG (priv->dialog),
			     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
      gtk_dialog_add_button (GTK_DIALOG (priv->dialog),
			     GTK_STOCK_OK, GTK_RESPONSE_OK);

      gtk_dialog_set_alternative_button_order (GTK_DIALOG (priv->dialog),
					       GTK_RESPONSE_OK,
					       GTK_RESPONSE_CANCEL,
					       -1);
    }
  g_free (priv->file_system);
  priv->file_system = NULL;

  g_signal_connect (priv->dialog, "delete-event",
		    G_CALLBACK (dialog_delete_event_cb), object);
  g_signal_connect (priv->dialog, "file-activated",
		    G_CALLBACK (dialog_file_activated_cb), object);
  g_signal_connect (priv->dialog, "icon-activated",
		    G_CALLBACK (dialog_icon_activated_cb), object);
  g_signal_connect (priv->dialog, "response",
		    G_CALLBACK (dialog_response_cb), object);
  g_signal_connect (priv->dialog, "notify",
		    G_CALLBACK (dialog_notify_cb), object);

  g_object_set_qdata (object, EGG_ICON_CHOOSER_DELEGATE_QUARK, priv->dialog);
  g_object_add_weak_pointer (G_OBJECT (priv->dialog),
			     (gpointer *) &priv->dialog);

  return object;
}

static void
egg_icon_chooser_button_set_property (GObject      *object,
				      guint         param_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  EggIconChooserButtonPrivate *priv;

  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_DIALOG:
      priv->dialog = g_value_get_object (value);
      break;

    case EGG_ICON_CHOOSER_PROP_FILE_SYSTEM:
      priv->file_system = g_value_dup_string (value);
      break;

    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
      /* FIXME: Show icon name in button? */
    case PROP_TITLE:
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_set_property (G_OBJECT (priv->dialog), pspec->name, value);
      break;

    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
      g_critical ("%s: Icon Choosers of type `%s' do not support changing the \"%s\" property.",
		  G_STRLOC, G_OBJECT_TYPE_NAME (object), pspec->name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
egg_icon_chooser_button_get_property (GObject    *object,
				      guint       param_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
  EggIconChooserButtonPrivate *priv;

  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (object);

  switch (param_id)
    {
    case PROP_TITLE:
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_get_property (G_OBJECT (priv->dialog), pspec->name, value);
      break;

    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
      g_value_set_boolean (value, FALSE);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}


/* ********************* *
 *  GtkObject Functions  *
 * ********************* */

static void
egg_icon_chooser_button_destroy (GtkObject * object)
{
  EggIconChooserButtonPrivate *priv;

  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (object);

  if (priv->dialog != NULL)
    gtk_widget_destroy (priv->dialog);

  if (GTK_OBJECT_CLASS (egg_icon_chooser_button_parent_class)->destroy != NULL)
    (*GTK_OBJECT_CLASS (egg_icon_chooser_button_parent_class)->destroy) (object);
}


/* ********************* *
 *  GtkWidget Functions  *
 * ********************* */

static void
egg_icon_chooser_button_style_set (GtkWidget *widget,
				   GtkStyle  *previous_style)
{
  if (GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->style_set)
    (*GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->style_set) (widget,
									   previous_style);

  reload_icon (EGG_ICON_CHOOSER_BUTTON (widget));
}

static void
egg_icon_chooser_button_screen_changed (GtkWidget *widget,
					GdkScreen *previous_screen)
{
  if (GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->screen_changed)
    (*GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->screen_changed) (widget,
									        previous_screen);

  reload_icon (EGG_ICON_CHOOSER_BUTTON (widget));
}

static void
egg_icon_chooser_button_show_all (GtkWidget *widget)
{
  gtk_widget_show (widget);
}

static void
egg_icon_chooser_button_hide_all (GtkWidget *widget)
{
  gtk_widget_hide (widget);
}

static void
egg_icon_chooser_button_show (GtkWidget *widget)
{
  EggIconChooserButtonPrivate *priv;

  if (GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->show)
    (*GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->show) (widget);

  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (widget);

  if (priv->active)
    button_clicked_cb (GTK_BUTTON (priv->button), widget);
}

static void
egg_icon_chooser_button_hide (GtkWidget *widget)
{
  gtk_widget_hide (EGG_ICON_CHOOSER_BUTTON (widget)->priv->dialog);

  if (GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->hide)
    (*GTK_WIDGET_CLASS (egg_icon_chooser_button_parent_class)->hide) (widget);
}


/* ****************** *
 *  Button Callbacks  *
 * ****************** */

static void
button_clicked_cb (GtkButton *button,
		   gpointer   user_data)
{
  EggIconChooserButtonPrivate *priv;

  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (user_data);

#if GTK_CHECK_VERSION(2,20,0)
  if (!gtk_widget_get_visible (priv->dialog))
#else
  if (!GTK_WIDGET_VISIBLE (priv->dialog))
#endif
    {
      GtkWidget *toplevel;

      toplevel = gtk_widget_get_toplevel (GTK_WIDGET (button));

      if (GTK_WIDGET_TOPLEVEL (toplevel) && GTK_IS_WINDOW (toplevel))
        {
          if (GTK_WINDOW (toplevel) != gtk_window_get_transient_for (GTK_WINDOW (priv->dialog)))
 	    gtk_window_set_transient_for (GTK_WINDOW (priv->dialog),
					  GTK_WINDOW (toplevel));
	      
	  gtk_window_set_modal (GTK_WINDOW (priv->dialog),
				gtk_window_get_modal (GTK_WINDOW (toplevel)));
	}
    }

  gtk_window_present (GTK_WINDOW (priv->dialog));
}


/* ****************** *
 *  Dialog Callbacks  *
 * ****************** */

static void
dialog_notify_cb (GObject    *dialog,
		  GParamSpec *pspec,
		  gpointer    user_data)
{
  EggIconChooserButton *button;
  gboolean need_emit;

  button = EGG_ICON_CHOOSER_BUTTON (user_data);
  need_emit = FALSE;

  if (strcmp (pspec->name, "icon-size") == 0)
    {
      reload_icon (button);
      need_emit = TRUE;
    }
  else
    need_emit = (strcmp (pspec->name, "context") == 0 ||
		 strcmp (pspec->name, "allow-custom") == 0 ||
		 strcmp (pspec->name, "custom-filter") == 0);
  /* Additional properties ("select-multiple", "file-system-backend") should
     never get changed */

  if (need_emit)
    g_object_notify (user_data, pspec->name);
}

static gboolean
dialog_delete_event_cb (GtkWidget *dialog,
			GdkEvent  *event,
			gpointer   user_data)
{
  g_signal_emit_by_name (dialog, "response", GTK_RESPONSE_DELETE_EVENT);

  return TRUE;
}

static void
dialog_response_cb (GtkDialog *dialog,
		    gint       response,
		    gpointer   user_data)
{
  EggIconChooserButtonPrivate *priv;
  EggIconContext context;

  priv = EGG_ICON_CHOOSER_BUTTON_GET_PRIVATE (user_data);

  if (response == GTK_RESPONSE_OK ||
      response == GTK_RESPONSE_ACCEPT ||
      response == GTK_RESPONSE_APPLY)
    {
      context = egg_icon_chooser_get_context (EGG_ICON_CHOOSER (dialog));

      if (context == EGG_ICON_CONTEXT_FILE)
	{
	  priv->icon_is_file = TRUE;
	  _egg_icon_chooser_file_selection_changed (user_data);
	  _egg_icon_chooser_file_activated (user_data);
	}
      else
	{
	  priv->icon_is_file = FALSE;
	  _egg_icon_chooser_icon_selection_changed (user_data);
	  _egg_icon_chooser_icon_activated (user_data);
	}

      reload_icon (user_data);
    }

  gtk_widget_hide (GTK_WIDGET (dialog));
}

static void
dialog_file_activated_cb (EggIconChooser *dialog,
			  gpointer        user_data)
{
  EggIconChooserButton *button;

  button = EGG_ICON_CHOOSER_BUTTON (user_data);

  button->priv->icon_is_file = TRUE;
  reload_icon (button);
}

static void
dialog_icon_activated_cb (EggIconChooser *dialog,
			  gpointer        user_data)
{
  EggIconChooserButton *button;

  button = EGG_ICON_CHOOSER_BUTTON (user_data);

  button->priv->icon_is_file = FALSE;
  reload_icon (button);
}


/* ******************* *
 *  Utility Functions  *
 * ******************* */

static inline gint
get_icon_size_for_widget (GtkWidget *widget)
{
  GtkSettings *settings;
  gint width, height;

  if (gtk_widget_has_screen (widget))
    settings = gtk_settings_get_for_screen (gtk_widget_get_screen (widget));
  else
    settings = gtk_settings_get_default ();

  if (gtk_icon_size_lookup_for_settings (settings, GTK_ICON_SIZE_DIALOG, &width, &height))
    return MAX (width, height);

  return FALLBACK_ICON_SIZE;
}

static inline GtkIconTheme *
get_icon_theme_for_widget (GtkWidget *widget)
{
  GtkIconTheme *theme;

  theme = NULL;

  if (gtk_widget_has_screen (widget))
    theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (widget));

  if (!theme)
    theme = gtk_icon_theme_get_default ();
  
  return theme;
}

static void
reload_icon (EggIconChooserButton *button)
{
  GtkIconTheme *theme;
  GdkPixbuf *pixbuf;
  gchar *target;
  gint icon_size;

  icon_size = egg_icon_chooser_get_icon_size (EGG_ICON_CHOOSER (button->priv->dialog));

  if (icon_size < 0)
    icon_size = get_icon_size_for_widget (GTK_WIDGET (button));
  else
    icon_size = MAX (icon_size, MINIMUM_ICON_SIZE);

  gtk_widget_set_size_request (button->priv->image, icon_size + 4, icon_size + 4);

  if (button->priv->icon_is_file)
    {
      theme = NULL;

      target = egg_icon_chooser_get_filename (EGG_ICON_CHOOSER (button->priv->dialog));
      if (target)
	pixbuf = gdk_pixbuf_new_from_file_at_size (target, icon_size, icon_size, NULL);
      else
	pixbuf = NULL;
      button->priv->icon_is_themed = (!pixbuf);
    }
  else
    {
      theme = get_icon_theme_for_widget (GTK_WIDGET (button));

      target = egg_icon_chooser_get_icon (EGG_ICON_CHOOSER (button->priv->dialog));
      if (target)
	pixbuf = gtk_icon_theme_load_icon (theme, target, icon_size, 0, NULL);
      else
	pixbuf = NULL;

      button->priv->icon_is_themed = TRUE;
    }

  g_free (target);

  if (!pixbuf)
    {
      if (!theme)
	theme = get_icon_theme_for_widget (GTK_WIDGET (button));

      pixbuf = gtk_icon_theme_load_icon (theme, "stock_unknown",
					 icon_size, 0, NULL);
    }

  gtk_image_set_from_pixbuf (GTK_IMAGE (button->priv->image), pixbuf);
  g_object_unref (pixbuf);
}


/* ************************************************************************** *
 *  PUBLIC API                                                                *
 * ************************************************************************** */

GtkWidget *
egg_icon_chooser_button_new (const gchar *title)
{
  return g_object_new (EGG_TYPE_ICON_CHOOSER_BUTTON,
		       "title", title,
		       NULL);
}

GtkWidget *
egg_icon_chooser_button_new_with_backend (const gchar *title,
					  const gchar *file_system_backend)
{
  return g_object_new (EGG_TYPE_ICON_CHOOSER_BUTTON,
		       "title", title,
		       "file-system-backend", file_system_backend,
		       NULL);
}

GtkWidget *
egg_icon_chooser_button_new_with_dialog (GtkWidget *dialog)
{
  g_return_val_if_fail (EGG_IS_ICON_CHOOSER_DIALOG (dialog), NULL);

  return g_object_new (EGG_TYPE_ICON_CHOOSER_BUTTON,
		       "dialog", dialog,
		       NULL);
}

void
egg_icon_chooser_button_set_title (EggIconChooserButton *button,
				   const gchar          *title)
{
  g_return_if_fail (EGG_IS_ICON_CHOOSER_BUTTON (button));

  gtk_window_set_title (GTK_WINDOW (button->priv->dialog), title);
}

G_CONST_RETURN gchar *
egg_icon_chooser_button_get_title (EggIconChooserButton *button)
{
  g_return_val_if_fail (EGG_IS_ICON_CHOOSER_BUTTON (button), NULL);

  return gtk_window_get_title (GTK_WINDOW (button->priv->dialog));
}
