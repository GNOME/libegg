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
#include "eggprintsettings.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER                  (egg_printer_get_type ())
#define EGG_PRINTER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER, EggPrinter))
#define EGG_PRINTER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINTER, EggPrinterClass))
#define EGG_IS_PRINTER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER))
#define EGG_IS_PRINTER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINTER))
#define EGG_PRINTER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINTER, EggPrinterClass))


typedef struct _EggPrinter          EggPrinter;
typedef struct _EggPrinterClass     EggPrinterClass;
typedef struct _EggPrinterPrivate   EggPrinterPrivate;

struct _EggPrintBackend;
struct _EggPrintJob;

struct _EggPrinter
{
  GObject parent_instance;

  EggPrinterPrivate *priv;
};

struct _EggPrinterClass
{
  GObjectClass parent_class;

  void (*details_acquired) (EggPrinter *printer, gboolean success);
  
  /* Padding for future expansion */
  void (*_egg_reserved1) (void);
  void (*_egg_reserved2) (void);
  void (*_egg_reserved3) (void);
  void (*_egg_reserved4) (void);
  void (*_egg_reserved5) (void);
  void (*_egg_reserved6) (void);
  void (*_egg_reserved7) (void);
};

GType                    egg_printer_get_type             (void) G_GNUC_CONST;
EggPrinter              *egg_printer_new                  (void);

struct _EggPrintBackend *egg_printer_get_backend          (EggPrinter *printer);

const gchar             *egg_printer_get_name             (EggPrinter *printer);
const gchar             *egg_printer_get_state_message    (EggPrinter *printer);
const gchar             *egg_printer_get_location         (EggPrinter *printer);
const gchar             *egg_printer_get_icon_name        (EggPrinter *printer);
gint                     egg_printer_get_job_count        (EggPrinter *printer);

struct _EggPrintJob     *egg_printer_prep_job             (EggPrinter *printer,
							   EggPrintSettings *settings,
		                                           const gchar *title,
                                                           double width, 
                                                           double height,
	                                                   GError **error);

G_END_DECLS

#endif /* __EGG_PRINTER_H__ */
