/* GTK - The GIMP Toolkit
 * eggrecentchooserwidget.h: embeddable recently used resources chooser widget
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

#ifndef __EGG_RECENT_CHOOSER_WIDGET_H__
#define __EGG_RECENT_CHOOSER_WIDGET_H__

#include "eggrecentchooser.h"
#include <gtk/gtkvbox.h>

G_BEGIN_DECLS

#define EGG_TYPE_RECENT_CHOOSER_WIDGET		  (egg_recent_chooser_widget_get_type ())
#define EGG_RECENT_CHOOSER_WIDGET(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_RECENT_CHOOSER_WIDGET, EggRecentChooserWidget))
#define EGG_IS_RECENT_CHOOSER_WIDGET(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_RECENT_CHOOSER_WIDGET))
#define EGG_RECENT_CHOOSER_WIDGET_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_RECENT_CHOOSER_WIDGET, EggRecentChooserWidgetClass))
#define EGG_IS_RECENT_CHOOSER_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_RECENT_CHOOSER_WIDGET))
#define EGG_RECENT_CHOOSER_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_RECENT_CHOOSER_WIDGET, EggRecentChooserWidgetClass))

typedef struct _EggRecentChooserWidget        EggRecentChooserWidget;
typedef struct _EggRecentChooserWidgetClass   EggRecentChooserWidgetClass;

typedef struct _EggRecentChooserWidgetPrivate EggRecentChooserWidgetPrivate;

struct _EggRecentChooserWidget
{
  /*< private >*/
  GtkVBox parent_instance;
  
  EggRecentChooserWidgetPrivate *priv;
};

struct _EggRecentChooserWidgetClass
{
  GtkVBoxClass parent_class;
};

GType      egg_recent_chooser_widget_get_type        (void) G_GNUC_CONST;
GtkWidget *egg_recent_chooser_widget_new             (void);
GtkWidget *egg_recent_chooser_widget_new_for_manager (EggRecentManager *manager);

G_END_DECLS

#endif /* __EGG_RECENT_CHOOSER_WIDGET_H__ */
