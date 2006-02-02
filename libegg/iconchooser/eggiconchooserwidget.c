/* eggiconchooserwidget.c
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

#include "gtkfilechooserembed.h"

#include "eggiconchooserdefault.h"
#include "eggiconchooserutils.h"

#include "eggiconchooserwidget.h"


/* **************** *
 *  Private Macros  *
 * **************** */

#define EGG_ICON_CHOOSER_WIDGET_GET_PRIVATE(object) \
  (EGG_ICON_CHOOSER_WIDGET ((object))->priv)


/* *************** *
 *  Private Types  *
 * *************** */

struct _EggIconChooserWidgetPrivate
{
  gchar *file_system;
  GtkWidget *chooser;
};


/* ********************* *
 *  Function Prototypes  *
 * ********************* */

static GObject *egg_icon_chooser_widget_constructor  (GType                  type,
						      guint                  n_params,
						      GObjectConstructParam *params);
static void     egg_icon_chooser_widget_set_property (GObject               *object,
						      guint                  param_id,
						      const GValue          *value,
						      GParamSpec            *pspec);
static void     egg_icon_chooser_widget_get_property (GObject               *object,
						      guint                  param_id,
						      GValue                *value,
						      GParamSpec            *pspec);


/* ******************* *
 *  GType Declaration  *
 * ******************* */

G_DEFINE_TYPE_WITH_CODE (EggIconChooserWidget, egg_icon_chooser_widget, GTK_TYPE_VBOX, ({	   \
  G_IMPLEMENT_INTERFACE (EGG_TYPE_ICON_CHOOSER, _egg_icon_chooser_delegate_iface_init)	  	   \
  G_IMPLEMENT_INTERFACE (GTK_TYPE_FILE_CHOOSER_EMBED, _gtk_file_chooser_embed_delegate_iface_init) \
}));


/* ***************** *
 *  GType Functions  *
 * ***************** */

static void
egg_icon_chooser_widget_class_init (EggIconChooserWidgetClass *class)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (class);

  gobject_class->constructor = egg_icon_chooser_widget_constructor;
  gobject_class->set_property = egg_icon_chooser_widget_set_property;
  gobject_class->get_property = egg_icon_chooser_widget_get_property;

  _egg_icon_chooser_install_properties (gobject_class);

  g_type_class_add_private (class, sizeof (EggIconChooserWidgetPrivate));
}


static void
egg_icon_chooser_widget_init (EggIconChooserWidget *widget)
{
  widget->priv = G_TYPE_INSTANCE_GET_PRIVATE (widget, EGG_TYPE_ICON_CHOOSER_WIDGET,
					      EggIconChooserWidgetPrivate);
}


/* ******************* *
 *  GObject Functions  *
 * ******************* */

static GObject *
egg_icon_chooser_widget_constructor (GType                  type,
				     guint                  n_params,
				     GObjectConstructParam *params)
{
  GObject *object;
  EggIconChooserWidgetPrivate *priv;

  object = (*G_OBJECT_CLASS (egg_icon_chooser_widget_parent_class)->constructor) (type,
										  n_params,
										  params);

  priv = EGG_ICON_CHOOSER_WIDGET_GET_PRIVATE (object);

  if (priv->file_system)
    priv->chooser = g_object_new (EGG_TYPE_ICON_CHOOSER_DEFAULT,
				  "file-system-backend", priv->file_system,
				  NULL);
  else
    priv->chooser = g_object_new (EGG_TYPE_ICON_CHOOSER_DEFAULT, NULL);
  g_free (priv->file_system);
  priv->file_system = NULL;
  gtk_container_add (GTK_CONTAINER (object), priv->chooser);
  gtk_widget_show (priv->chooser);
  _egg_icon_chooser_set_delegate (EGG_ICON_CHOOSER (object),
				  EGG_ICON_CHOOSER (priv->chooser));
  _gtk_file_chooser_embed_set_delegate (GTK_FILE_CHOOSER_EMBED (object),
					GTK_FILE_CHOOSER_EMBED (priv->chooser));

  return object;
}

static void
egg_icon_chooser_widget_set_property (GObject      *object,
				      guint         param_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  EggIconChooserWidgetPrivate *priv;

  priv = EGG_ICON_CHOOSER_WIDGET_GET_PRIVATE (object);

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
      g_object_set_property (G_OBJECT (priv->chooser), pspec->name, value);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}

static void
egg_icon_chooser_widget_get_property (GObject    *object,
				      guint       param_id,
				      GValue     *value,
				      GParamSpec *pspec)
{
  EggIconChooserWidgetPrivate *priv;

  priv = EGG_ICON_CHOOSER_WIDGET_GET_PRIVATE (object);

  switch (param_id)
    {
    case EGG_ICON_CHOOSER_PROP_CONTEXT:
    case EGG_ICON_CHOOSER_PROP_SELECT_MULTIPLE:
    case EGG_ICON_CHOOSER_PROP_ICON_SIZE:
    case EGG_ICON_CHOOSER_PROP_SHOW_ICON_NAME:
    case EGG_ICON_CHOOSER_PROP_ALLOW_CUSTOM:
    case EGG_ICON_CHOOSER_PROP_CUSTOM_FILTER:
      g_object_get_property (G_OBJECT (priv->chooser), pspec->name, value);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
      break;
    }
}


/* ************************************************************************** *
 *  Public API                                                                *
 * ************************************************************************** */

/**
 * EggIconChooserWidget:
 * 
 * The widget structure for the EggIconChooserWidget. This should not be
 * accessed directly. Use the #EggIconChooser API.
 * 
 * Since: 2.8
 **/

/**
 * egg_icon_chooser_widget_new:
 * 
 * Creates a new #EggIconChooser as a widget.
 * 
 * Returns: the new widget.
 * 
 * Since: 2.8
 **/
GtkWidget *
egg_icon_chooser_widget_new (void)
{
  return g_object_new (EGG_TYPE_ICON_CHOOSER_WIDGET, NULL);
}


/**
 * egg_icon_chooser_widget_new_with_backend:
 * @file_system_backend: the name of the #GtkFileChooser backend to use.
 * 
 * Creates a new #EggIconChooser as a widget who's "custom" page will use
 * @file_system_backend.
 * 
 * Returns: the new widget.
 * 
 * Since: 2.8
 **/
GtkWidget *
egg_icon_chooser_widget_new_with_backend (const gchar *file_system_backend)
{
  return g_object_new (EGG_TYPE_ICON_CHOOSER_WIDGET,
		       "file-system-backend", file_system_backend,
		       NULL);
}
