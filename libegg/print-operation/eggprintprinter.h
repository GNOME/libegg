/* EggPrintPrinter 
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
#ifndef __EGG_PRINT_PRINTER_H__
#define __EGG_PRINT_PRINTER_H__

#include <glib-object.h>
#include <cairo.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_PRINTER                  (egg_print_printer_get_type ())
#define EGG_PRINT_PRINTER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_PRINTER, EggPrintPrinter))
#define EGG_PRINT_PRINTER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_PRINTER, EggPrintPrinterClass))
#define EGG_IS_PRINT_PRINTER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_PRINTER))
#define EGG_IS_PRINT_PRINTER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_PRINTER))
#define EGG_PRINT_PRINTER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_PRINTER, EggPrintPrinterClass))


typedef struct _EggPrintPrinter	        EggPrintPrinter;
typedef struct _EggPrintPrinterClass    EggPrintPrinterClass;
typedef struct _EggPrintPrinterPrivate   EggPrintPrinterPrivate;

struct _EggPrintPrinter
{
  GObject parent_instance;

  gchar *name;
  gchar *location;
  gchar *description;

  guint is_active: 1;
  guint is_new: 1;

  gchar *state_message;  
  gint job_count;

  EggPrintPrinterPrivate *priv;
};

struct _EggPrintPrinterClass
{
  GObjectClass parent_class;

};

GType		 egg_print_printer_get_type	    (void) G_GNUC_CONST;
EggPrintPrinter *egg_print_printer_new              (void);

void             egg_print_printer_set_backend_data (EggPrintPrinter *printer,
                                                     void *data,
                                                     GFreeFunc destroy_notify);
						     
cairo_t *egg_print_printer_create_cairo_surface (EggPrintPrinter *printer,
                                                 guint height, 
                                                 guint width);

G_END_DECLS

#endif /* __EGG_PRINT_PRINTER_H__ */
