/* EGG - The GIMP Toolkit
 * eggprintcontext.h: Print Context
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

#ifndef __EGG_PRINT_CONTEXT_H__
#define __EGG_PRINT_CONTEXT_H__

#include <glib-object.h>
#include <pango/pangocairo.h>
#include <eggprintenums.h>
#include <eggpagesetup.h>

G_BEGIN_DECLS

typedef struct _EggPrintContext EggPrintContext;

#define EGG_TYPE_PRINT_CONTEXT    (egg_print_context_get_type ())
#define EGG_PRINT_CONTEXT(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_CONTEXT, EggPrintContext))
#define EGG_IS_PRINT_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_CONTEXT))

GType          egg_print_context_get_type (void);


/* Rendering */
cairo_t *     egg_print_context_get_cairo      (EggPrintContext *context);

EggPageSetup *egg_print_context_get_page_setup (EggPrintContext *context);
double        egg_print_context_get_width      (EggPrintContext *context);
double        egg_print_context_get_height     (EggPrintContext *context);
double        egg_print_context_get_dpi_x      (EggPrintContext *context);
double        egg_print_context_get_dpi_y      (EggPrintContext *context);

/* Fonts */
PangoFontMap *egg_print_context_get_fontmap    (EggPrintContext *context);
PangoContext *egg_print_context_create_context (EggPrintContext *context);
PangoLayout * egg_print_context_create_layout  (EggPrintContext *context);



G_END_DECLS

#endif /* __EGG_PRINT_CONTEXT_H__ */
