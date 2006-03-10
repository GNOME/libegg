/* GTK - The GIMP Toolkit
 * eggprintbackend.h: Abstract printer backend interfaces
 * Copyright (C) 2006, Red Hat, Inc.
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
#include <gtk/gtk.h>
#include <cairo.h>

#include "eggprinter-private.h"
#include "eggprintsettings.h"
#include "eggprinteroption.h"
#include "eggprintjob.h"

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

#define EGG_TYPE_PRINT_BACKEND             (egg_print_backend_get_type ())
#define EGG_PRINT_BACKEND(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_BACKEND, EggPrintBackend))
#define EGG_IS_PRINT_BACKEND(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_BACKEND))
#define EGG_PRINT_BACKEND_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), EGG_TYPE_PRINT_BACKEND, EggPrintBackendIface))

struct _EggPrintBackendIface
{
  GTypeInterface base_iface;

  /* Global backend methods: */

  EggPrinter * (*find_printer) (EggPrintBackend *print_backend,
                                const gchar *printer_name);
  void         (*print_stream) (EggPrintBackend *print_backend,
                                EggPrintJob *job,
				const gchar *title,
				gint data_fd,
				EggPrintJobCompleteFunc callback,
				gpointer user_data);

  /* Printer methods: */
  cairo_surface_t *     (*printer_create_cairo_surface)      (EggPrinter *printer,
							      gdouble height,
							      gdouble width,
							      gint cache_fd);
  EggPrinterOptionSet * (*printer_get_options)               (EggPrinter *printer);
  gboolean              (*printer_mark_conflicts)            (EggPrinter *printer,
							      EggPrinterOptionSet *options);
  void                  (*printer_get_settings_from_options) (EggPrinter *printer,
							      EggPrinterOptionSet *options,
							      EggPrintSettings *settings);
  void                  (*printer_prepare_for_print)         (EggPrinter *printer,
							      EggPrintSettings *settings);
  GList  *              (*printer_get_paper_sizes)           (EggPrinter *printer);

  /* Signals 
   */
  void (*printer_added)          (EggPrinter *printer);
  void (*printer_removed)        (EggPrinter *printer);
  void (*printer_status_changed) (EggPrinter *printer);
};

GType   egg_print_backend_get_type       (void) G_GNUC_CONST;

EggPrinter *egg_print_backend_find_printer                      (EggPrintBackend *print_backend,
                                                                 const gchar *printer_name);
							 
void egg_print_backend_print_stream                             (EggPrintBackend *print_backend,
                                                                 EggPrintJob *job,
				                                 const gchar *title,
				                                 gint data_fd,
				                                 EggPrintJobCompleteFunc callback,
				                                 gpointer user_data);

G_END_DECLS

#endif /* __EGG_PRINT_BACKEND_H__ */
