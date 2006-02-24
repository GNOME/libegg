/* EGG - The GIMP Toolkit
 * eggprintoperation.h: Print Operation
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

#ifndef __EGG_PRINTER_PRIVATE_H__
#define __EGG_PRINTER_PRIVATE_H__

#include <glib.h>
#include "eggprinter.h"
#include "eggprintersettings.h"
#include "eggprintbackendsettingset.h"

G_BEGIN_DECLS
struct _EggPrinterPrivate
{
  gchar *name;
  gchar *location;
  gchar *description;
  gchar *icon_name;

  guint is_active: 1;
  guint is_new: 1;

  gchar *state_message;  
  gint job_count;

  struct _EggPrintBackend *backend;
};


EggPrintBackendSettingSet *_egg_printer_get_backend_settings (EggPrinter                *printer);
gboolean                   _egg_printer_mark_conflicts       (EggPrinter                *printer,
							      EggPrintBackendSettingSet *settings);
void                       _egg_printer_add_backend_settings (EggPrinter                *printer,
							      EggPrintBackendSettingSet *backend_settings,
							      EggPrinterSettings        *settings);
void                       _egg_printer_prepare_for_print    (EggPrinter                *printer,
							      EggPrinterSettings        *settings);
cairo_surface_t *          _egg_printer_create_cairo_surface (EggPrinter                *printer,
							      gdouble                    width,
							      gdouble                    height,
							      gint                       cache_fd);

void                       _egg_printer_emit_settings_retrieved (EggPrinter *printer);
G_END_DECLS
#endif /* __EGG_PRINT_OPERATION_PRIVATE_H__ */
