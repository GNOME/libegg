/* EggPrintSettingWidget 
 * Copyright (C) 2006 John (J5) Palmieri <johnp@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __EGG_PRINT_SETTING_WIDGET_H__
#define __EGG_PRINT_SETTING_WIDGET_H__

#include "eggprintbackendsetting.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_SETTING_WIDGET                  (egg_print_setting_widget_get_type ())
#define EGG_PRINT_SETTING_WIDGET(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_SETTING_WIDGET, EggPrintSettingWidget))
#define EGG_PRINT_SETTING_WIDGET_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_SETTING_WIDGET, EggPrintSettingWidgetClass))
#define EGG_IS_PRINT_SETTING_WIDGET(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_SETTING_WIDGET))
#define EGG_IS_PRINT_SETTING_WIDGET_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_SETTING_WIDGET))
#define EGG_PRINT_SETTING_WIDGET_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_SETTING_WIDGET, EggPrintSettingWidgetClass))


typedef struct _EggPrintSettingWidget         EggPrintSettingWidget;
typedef struct _EggPrintSettingWidgetClass    EggPrintSettingWidgetClass;
typedef struct EggPrintSettingWidgetPrivate   EggPrintSettingWidgetPrivate;

struct _EggPrintSettingWidget
{
  GtkHBox parent_instance;

  EggPrintSettingWidgetPrivate *priv;
};

struct _EggPrintSettingWidgetClass
{
  GtkHBoxClass parent_class;
};

GType		 egg_print_setting_widget_get_type   (void) G_GNUC_CONST;

GtkWidget *egg_print_setting_widget_new                (EggPrintBackendSetting *source);
void       egg_print_setting_widget_set_source         (EggPrintSettingWidget  *setting,
							EggPrintBackendSetting *source);
gboolean   egg_print_setting_widget_has_external_label (EggPrintSettingWidget  *setting);
GtkWidget *egg_print_setting_widget_get_external_label (EggPrintSettingWidget  *setting);


G_END_DECLS

#endif /* __EGG_PRINT_SETTING_WIDGET_H__ */
