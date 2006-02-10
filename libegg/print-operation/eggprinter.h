/* EggPrinter 
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
#ifndef __EGG_PRINTER_H__
#define __EGG_PRINTER_H__

#include <glib-object.h>
#include <cairo.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER                  (egg_printer_get_type ())
#define EGG_PRINTER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER, EggPrinter))
#define EGG_PRINTER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINTER, EggPrinterClass))
#define EGG_IS_PRINTER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER))
#define EGG_IS_PRINTER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINTER))
#define EGG_PRINTER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINTER, EggPrinterClass))


typedef struct _EggPrinter	        EggPrinter;
typedef struct _EggPrinterClass    EggPrinterClass;
typedef struct _EggPrinterPrivate   EggPrinterPrivate;

typedef void (*EggPrinterSendCompleteFunc) (EggPrinter *printer,
                                           void *user_data, 
                                           GError **error);

struct _EggPrintBackend;

struct _EggPrinter
{
  GObject parent_instance;

  EggPrinterPrivate *priv;
};

struct _EggPrinterClass
{
  GObjectClass parent_class;

};

GType                    egg_printer_get_type             (void) G_GNUC_CONST;
EggPrinter              *egg_printer_new                  (void);

void                     egg_printer_set_backend_data     (EggPrinter *printer,
                                                                 void *data,
                                                                 GFreeFunc destroy_notify);
						     
struct _EggPrintBackend *egg_printer_get_backend          (EggPrinter *printer);

const gchar             *egg_printer_get_name             (EggPrinter *printer);
const gchar             *egg_printer_get_state_message    (EggPrinter *printer);
const gchar             *egg_printer_get_location         (EggPrinter *printer);
gint                     egg_printer_get_job_count        (EggPrinter *printer);

cairo_surface_t         *egg_printer_create_cairo_surface (EggPrinter *printer,
                                                           double width,
                                                           double height,
                                                           gint cache_fd);

void                     egg_printer_print_stream         (EggPrinter *printer,
                                                           const gchar *title,
							   gint data_fd, 
							   EggPrinterSendCompleteFunc callback,
							   gpointer user_data);

G_END_DECLS

#endif /* __EGG_PRINTER_H__ */
