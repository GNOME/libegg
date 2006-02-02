/* EGG - The GIMP Toolkit
 * eggprintcontextprivate.h: Print Context
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

#ifndef __EGG_PRINT_CONTEXT_PRIVATE_H__
#define __EGG_PRINT_CONTEXT_PRIVATE_H__

#include "eggprintcontext.h"
#include "eggprintoperation.h"

G_BEGIN_DECLS

EggPrintContext *_egg_print_context_new (EggPrintOperation *op);

void _egg_print_context_set_page_setup (EggPrintContext *context,
					EggPageSetup *page_setup);
void _egg_print_context_translate_into_margin (EggPrintContext *context);



G_END_DECLS

#endif /* __EGG_PRINT_CONTEXT_PRIVATE_H__ */
