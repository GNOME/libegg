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

#ifndef __EGG_PRINT_OPERATION_H__
#define __EGG_PRINT_OPERATION_H__

#include <glib-object.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <eggprintenums.h>
#include <eggpagesetup.h>
#include <eggprintersettings.h>
#include <eggprintcontext.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_OPERATION    (egg_print_operation_get_type ())
#define EGG_PRINT_OPERATION(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_OPERATION, EggPrintOperation))
#define EGG_IS_PRINT_OPERATION(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_OPERATION))

typedef struct _EggPrintOperationClass   EggPrintOperationClass;
typedef struct _EggPrintOperationPrivate EggPrintOperationPrivate;
typedef struct _EggPrintOperation        EggPrintOperation;

struct _EggPrintOperation
{
  GObject parent_instance;
  
  EggPrintOperationPrivate *priv;
};

struct _EggPrintOperationClass
{
  GObjectClass parent_class;
  
  void (*begin_print) (EggPrintOperation *operation, EggPrintContext *context);
  void (*request_page_setup) (EggPrintOperation *operation,
			      EggPrintContext *context,
			      int page_nr,
			      EggPageSetup *setup);
  void (*draw_page) (EggPrintOperation *operation,
		     EggPrintContext *context,
		     int page_nr);
  void (*end_print) (EggPrintOperation *operation,
		     EggPrintContext *context);
};

GType                   egg_print_operation_get_type               (void);
EggPrintOperation *     egg_print_operation_new                    (void);
void                    egg_print_operation_set_default_page_setup (EggPrintOperation  *op,
								    EggPageSetup       *default_page_setup);
EggPageSetup *          egg_print_operation_get_default_page_setup (EggPrintOperation  *op);
void                    egg_print_operation_set_printer_settings   (EggPrintOperation  *op,
								    EggPrinterSettings *printer_settings);
EggPrinterSettings *    egg_print_operation_get_printer_settings   (EggPrintOperation  *op);
void                    egg_print_operation_set_job_name           (EggPrintOperation  *op,
								    const char         *job_name);
void                    egg_print_operation_set_nr_of_pages        (EggPrintOperation  *op,
								    int                 n_pages);
void                    egg_print_operation_set_current_page       (EggPrintOperation  *op,
								    int                 current_page);
void                    egg_print_operation_set_use_full_page      (EggPrintOperation  *op,
								    gboolean            full_page);
void                    egg_print_operation_set_unit               (EggPrintOperation  *op,
								    EggUnit             unit);
void                    egg_print_operation_set_show_dialog        (EggPrintOperation  *op,
								    gboolean            show_dialog);
void                    egg_print_operation_set_pdf_target         (EggPrintOperation  *op,
								    const char         *filename);
gboolean                egg_print_operation_run                    (EggPrintOperation  *op,
								    GtkWindow          *parent);

G_END_DECLS

#endif /* __EGG_PRINT_OPERATION_H__ */
