/* EggPrinterLpr
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
#ifndef __EGG_PRINTER_LPR_H__
#define __EGG_PRINTER_LPR_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "eggprinter.h"
#include "eggprinteroption.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER_LPR                  (egg_printer_lpr_get_type ())
#define EGG_PRINTER_LPR(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER_LPR, EggPrinterLpr))
#define EGG_PRINTER_LPR_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINTER_LPR, EggPrinterLprClass))
#define EGG_IS_PRINTER_LPR(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER_LPR))
#define EGG_IS_PRINTER_LPR_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINTER_LPR))
#define EGG_PRINTER_LPR_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINTER_LPR, EggPrinterLprClass))

typedef struct _EggPrinterLpr	        EggPrinterLpr;
typedef struct _EggPrinterLprClass     EggPrinterLprClass;
typedef struct _EggPrinterLprPrivate   EggPrinterLprPrivate;

struct _EggPrinterLpr
{
  EggPrinter parent_instance;

  EggPrinterOption *options;
};

struct _EggPrinterLprClass
{
  EggPrinterClass parent_class;

};

GType                    egg_printer_lpr_get_type             (void) G_GNUC_CONST;
EggPrinterLpr          *egg_printer_lpr_new                  (void);

G_END_DECLS

#endif /* __EGG_PRINTER_LPR_H__ */
