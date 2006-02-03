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

#ifndef __EGG_PRINT_OPERATION_PRIVATE_H__
#define __EGG_PRINT_OPERATION_PRIVATE_H__

#include "eggprintoperation.h"

G_BEGIN_DECLS

struct _EggPrintOperationPrivate
{
  EggPageSetup *default_page_setup;
  EggPrinterSettings *printer_settings;
  char *job_name;
  int nr_of_pages;
  int current_page;
  gboolean use_full_page;
  EggUnit unit;
  gboolean show_dialog;
  char *pdf_target;

  /* Data for the print job: */
  cairo_surface_t *surface;
  double dpi_x, dpi_y;

  int manual_num_copies;
  gboolean manual_collation;
 
  void *platform_data;

  void (*start_page) (EggPrintOperation *operation,
		      EggPrintContext *print_context,
		      EggPageSetup *page_setup);
  void (*end_page) (EggPrintOperation *operation,
		    EggPrintContext *print_context);
  void (*end_run) (EggPrintOperation *operation);
};

EggPrintOperationResult egg_print_operation_platfrom_backend_run_dialog (EggPrintOperation *operation,
									 GtkWindow *parent,
									 gboolean *do_print,
									 GError **error);

G_END_DECLS

#endif /* __EGG_PRINT_OPERATION_PRIVATE_H__ */
