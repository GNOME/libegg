/* GTK - The GIMP Toolkit
 * eggprintbackend.h: Abstract printer backend interfaces
 * Copyright (C) 2003, Red Hat, Inc.
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

#ifndef __EGG_PRINT_BACKEND_H__
#define __EGG_PRINT_BACKEND_H__

/* This is a "semi-private" header; it is meant only for
 * alternate EggPrintDialog backend modules; no stability guarantees 
 * are made at this point
 */
#ifndef EGG_PRINT_BACKEND_ENABLE_UNSUPPORTED
#error "EggPrintBackend is not supported API for general use"
#endif

#include <glib-object.h>
#include <cairo.h>

#include "eggprintprinter.h"

G_BEGIN_DECLS
typedef struct _EggPrintBackend       EggPrintBackend;
typedef struct _EggPrintBackendIface  EggPrintBackendIface;

#define EGG_PRINT_BACKEND_ERROR (egg_print_backend_error_quark ())

typedef enum
{
  /* TODO: add specific errors */
  EGG_PRINT_BACKEND_ERROR_GENERIC
} EggPrintBackendError;

GQuark     egg_print_backend_error_quark      (void);

/* The base GtkFileSystem interface
 */
#define EGG_TYPE_PRINT_BACKEND             (egg_print_backend_get_type ())
#define EGG_PRINT_BACKEND(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_BACKEND, EggPrintBackend))
#define EGG_IS_PRINT_BACKEND(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_BACKEND))
#define EGG_PRINT_BACKEND_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), EGG_TYPE_PRINT_BACKEND, EggPrintBackendIface))

struct _EggPrintBackendIface
{
  GTypeInterface base_iface;

  /* Methods
   */
  cairo_surface_t * (*printer_create_cairo_surface) (EggPrintBackend *print_backend,
                                                     EggPrintPrinter *printer,
                                                     gdouble height,
                                                     gdouble width);

  EggPrintPrinter * (*find_printer) (EggPrintBackend *print_backend,
                                     const gchar *printer_name);

  /* Signals 
   */
  void (*printer_added)          (EggPrintPrinter *printer);
  void (*printer_removed)        (EggPrintPrinter *printer);
  void (*printer_status_changed) (EggPrintPrinter *printer);
};

GType   egg_print_backend_get_type       (void) G_GNUC_CONST;

cairo_surface_t *egg_print_backend_printer_create_cairo_surface (EggPrintBackend *print_backend,
                                                                 EggPrintPrinter *printer,
                                                                 gdouble width, 
                                                                 gdouble height);

EggPrintPrinter *egg_print_backend_find_printer                 (EggPrintBackend *print_backend,
                                                                 const gchar *printer_name);
							 

G_END_DECLS

#endif /* __EGG_PRINT_BACKEND_H__ */


