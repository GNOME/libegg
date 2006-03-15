/* GTK - The GIMP Toolkit
 * eggprintbackendpdf.h: Default implementation of EggPrintBackend 
 * for printing to a PDF 
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

#ifndef __EGG_PRINT_BACKEND_PDF_H__
#define __EGG_PRINT_BACKEND_PDF_H__

#include <glib-object.h>
#include "eggprintbackend.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_BACKEND_PDF            (egg_print_backend_pdf_get_type ())
#define EGG_PRINT_BACKEND_PDF(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_BACKEND_PDF, EggPrintBackendPdf))
#define EGG_IS_PRINT_BACKEND_PDF(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_BACKEND_PDF))

typedef struct _EggPrintBackendPdf      EggPrintBackendPdf;

EggPrintBackend *egg_print_backend_pdf_new      (void);
GType          egg_print_backend_pdf_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __EGG_PRINT_BACKEND_PDF_H__ */


