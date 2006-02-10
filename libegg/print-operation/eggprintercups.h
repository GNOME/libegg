/* EggPrinterCups
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
#ifndef __EGG_PRINTER_CUPS_H__
#define __EGG_PRINTER_CUPS_H__

#include <glib-object.h>

#include "eggprinter.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER_CUPS                  (egg_printer_cups_get_type ())
#define EGG_PRINTER_CUPS(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER_CUPS, EggPrinterCups))
#define EGG_PRINTER_CUPS_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINTER_CUPS, EggPrinterCupsClass))
#define EGG_IS_PRINTER_CUPS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER_CUPS))
#define EGG_IS_PRINTER_CUPS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINTER_CUPS))
#define EGG_PRINTER_CUPS_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINTER_CUPS, EggPrinterCupsClass))

typedef struct _EggPrinterCups	        EggPrinterCups;
typedef struct _EggPrinterCupsClass     EggPrinterCupsClass;
typedef struct _EggPrinterCupsPrivate   EggPrinterCupsPrivate;

struct _EggPrinterCups
{
  EggPrinter parent_instance;

  EggPrinterCupsPrivate *priv;
};

struct _EggPrinterCupsClass
{
  EggPrinterClass parent_class;

};

GType                    egg_printer_cups_get_type             (void) G_GNUC_CONST;
EggPrinterCups          *egg_printer_cups_new                  (void);

G_END_DECLS

#endif /* __EGG_PRINTER_CUPS_H__ */
