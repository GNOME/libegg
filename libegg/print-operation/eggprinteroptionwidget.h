/* EggPrinterOptionWidget 
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
#ifndef __EGG_PRINTER_OPTION_WIDGET_H__
#define __EGG_PRINTER_OPTION_WIDGET_H__

#include "eggprinteroption.h"
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER_OPTION_WIDGET                  (egg_printer_option_widget_get_type ())
#define EGG_PRINTER_OPTION_WIDGET(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER_OPTION_WIDGET, EggPrinterOptionWidget))
#define EGG_PRINTER_OPTION_WIDGET_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINTER_OPTION_WIDGET, EggPrinterOptionWidgetClass))
#define EGG_IS_PRINTER_OPTION_WIDGET(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER_OPTION_WIDGET))
#define EGG_IS_PRINTER_OPTION_WIDGET_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINTER_OPTION_WIDGET))
#define EGG_PRINTER_OPTION_WIDGET_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINTER_OPTION_WIDGET, EggPrinterOptionWidgetClass))


typedef struct _EggPrinterOptionWidget         EggPrinterOptionWidget;
typedef struct _EggPrinterOptionWidgetClass    EggPrinterOptionWidgetClass;
typedef struct EggPrinterOptionWidgetPrivate   EggPrinterOptionWidgetPrivate;

struct _EggPrinterOptionWidget
{
  GtkHBox parent_instance;

  EggPrinterOptionWidgetPrivate *priv;
};

struct _EggPrinterOptionWidgetClass
{
  GtkHBoxClass parent_class;

  void (*changed) (EggPrinterOptionWidget *widget);
};

GType		 egg_printer_option_widget_get_type   (void) G_GNUC_CONST;

GtkWidget * egg_printer_option_widget_new                (EggPrinterOption       *source);
void        egg_printer_option_widget_set_source         (EggPrinterOptionWidget *setting,
							  EggPrinterOption       *source);
gboolean    egg_printer_option_widget_has_external_label (EggPrinterOptionWidget *setting);
GtkWidget * egg_printer_option_widget_get_external_label (EggPrinterOptionWidget *setting);
const char *egg_printer_option_widget_get_value          (EggPrinterOptionWidget *setting);

G_END_DECLS

#endif /* __EGG_PRINTER_OPTION_WIDGET_H__ */
