/* GTK - The GIMP Toolkit
 * eggrecentchooserwidget.c: embeddable recently used resources chooser widget
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

#include "eggrecentchooserwidget.h"
#include "eggrecentchooserdefault.h"
#include "eggrecentchooserutils.h"
#include "eggrecenttypebuiltins.h"

struct _EggRecentChooserWidgetPrivate
{
  EggRecentManager *manager;
  
  GtkWidget *chooser;
};

#define EGG_RECENT_CHOOSER_WIDGET_GET_PRIVATE(obj)	(EGG_RECENT_CHOOSER_WIDGET (obj)->priv)

static GObject *egg_recent_chooser_widget_constructor  (GType                  type,
						        guint                  n_params,
						        GObjectConstructParam *params);
static void     egg_recent_chooser_widget_set_property (GObject               *object,
						        guint                  prop_id,
						        const GValue          *value,
						        GParamSpec            *pspec);
static void     egg_recent_chooser_widget_get_property (GObject               *object,
						        guint                  prop_id,
						        GValue                *value,
						        GParamSpec            *pspec);
static void     egg_recent_chooser_widget_finalize     (GObject               *object);


G_DEFINE_TYPE_WITH_CODE (EggRecentChooserWidget,
		         egg_recent_chooser_widget,
			 GTK_TYPE_VBOX,
			 G_IMPLEMENT_INTERFACE (EGG_TYPE_RECENT_CHOOSER,
						_egg_recent_chooser_delegate_iface_init));

static void
egg_recent_chooser_widget_class_init (EggRecentChooserWidgetClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->constructor = egg_recent_chooser_widget_constructor;
  gobject_class->set_property = egg_recent_chooser_widget_set_property;
  gobject_class->get_property = egg_recent_chooser_widget_get_property;
  gobject_class->finalize = egg_recent_chooser_widget_finalize;

  _egg_recent_chooser_install_properties (gobject_class);

  g_type_class_add_private (klass, sizeof (EggRecentChooserWidgetPrivate));
}


static void
egg_recent_chooser_widget_init (EggRecentChooserWidget *widget)
{
  widget->priv = G_TYPE_INSTANCE_GET_PRIVATE (widget, EGG_TYPE_RECENT_CHOOSER_WIDGET,
					      EggRecentChooserWidgetPrivate);
}

static GObject *
egg_recent_chooser_widget_constructor (GType                  type,
				       guint                  n_params,
				       GObjectConstructParam *params)
{
  GObject *object;
  EggRecentChooserWidgetPrivate *priv;

  object = G_OBJECT_CLASS (egg_recent_chooser_widget_parent_class)->constructor (type,
										 n_params,
										 params);

  priv = EGG_RECENT_CHOOSER_WIDGET_GET_PRIVATE (object);
  priv->chooser = _egg_recent_chooser_default_new (priv->manager);
  
  
  gtk_container_add (GTK_CONTAINER (object), priv->chooser);
  gtk_widget_show (priv->chooser);
  _egg_recent_chooser_set_delegate (EGG_RECENT_CHOOSER (object),
				    EGG_RECENT_CHOOSER (priv->chooser));

  return object;
}

static void
egg_recent_chooser_widget_set_property (GObject      *object,
				        guint         prop_id,
				        const GValue *value,
				        GParamSpec   *pspec)
{
  EggRecentChooserWidgetPrivate *priv;

  priv = EGG_RECENT_CHOOSER_WIDGET_GET_PRIVATE (object);
  
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
egg_recent_chooser_widget_get_property (GObject    *object,
				        guint       prop_id,
				        GValue     *value,
				        GParamSpec *pspec)
{
  EggRecentChooserWidgetPrivate *priv;

  priv = EGG_RECENT_CHOOSER_WIDGET_GET_PRIVATE (object);

  g_object_get_property (G_OBJECT (priv->chooser), pspec->name, value);
}

static void
egg_recent_chooser_widget_finalize (GObject *object)
{
  EggRecentChooserWidgetPrivate *priv;
  
  priv = EGG_RECENT_CHOOSER_WIDGET_GET_PRIVATE (object);
  
  if (priv->manager)
    g_object_unref (G_OBJECT (priv->manager));
  
  G_OBJECT_CLASS (egg_recent_chooser_widget_parent_class)->finalize (object);
}

/*
 * Public API
 */

/**
 * egg_recent_chooser_widget_new:
 * 
 * Creates a new #EggRecentChooserWidget object.  This is an embeddable widget
 * used to access the recently used resources list.
 *
 * Return value: a new #EggRecentChooserWidget
 *
 * Since: 2.10
 */
GtkWidget *
egg_recent_chooser_widget_new (void)
{
  return g_object_new (EGG_TYPE_RECENT_CHOOSER_WIDGET, NULL);
}

/**
 * egg_recent_chooser_widget_new_for_manager:
 * @manager: a #EggRecentManager
 *
 * Creates a new #EggRecentChooserWidget with a specified recent manager.
 *
 * This is useful if you have implemented your own recent manager, or if you
 * have a customized instance of a #EggRecentManager object (e.g. with your
 * own sorting and/or filtering functions).
 *
 * Return value: a new #EggRecentChooserWidget
 *
 * Since: 2.10
 */
GtkWidget *
egg_recent_chooser_widget_new_for_manager (EggRecentManager *manager)
{
  g_return_val_if_fail (EGG_IS_RECENT_MANAGER (manager), NULL);
  
  return g_object_new (EGG_TYPE_RECENT_CHOOSER_WIDGET,
  		       "recent-manager", manager,
  		       NULL);
}
