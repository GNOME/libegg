
/* GTK - The GIMP Toolkit
 * eggprinteroptionset.h: printer option set
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

#ifndef __EGG_PRINTER_OPTION_SET_H__
#define __EGG_PRINTER_OPTION_SET_H__

/* This is a "semi-private" header; it is meant only for
 * alternate EggPrintDialog backend modules; no stability guarantees 
 * are made at this point
 */
#ifndef EGG_PRINT_BACKEND_ENABLE_UNSUPPORTED
#error "EggPrintBackend is not supported API for general use"
#endif

#include <glib-object.h>
#include "eggprinteroption.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER_OPTION_SET             (egg_printer_option_set_get_type ())
#define EGG_PRINTER_OPTION_SET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER_OPTION_SET, EggPrinterOptionSet))
#define EGG_IS_PRINTER_OPTION_SET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER_OPTION_SET))

typedef struct _EggPrinterOptionSet       EggPrinterOptionSet;
typedef struct _EggPrinterOptionSetClass  EggPrinterOptionSetClass;

struct _EggPrinterOptionSet
{
  GObject parent_instance;

  /*< private >*/
  GPtrArray *array;
  GHashTable *hash;
};

struct _EggPrinterOptionSetClass
{
  GObjectClass parent_class;

  void (*changed) (EggPrinterOptionSet *option);


  /* Padding for future expansion */
  void (*_egg_reserved1) (void);
  void (*_egg_reserved2) (void);
  void (*_egg_reserved3) (void);
  void (*_egg_reserved4) (void);
  void (*_egg_reserved5) (void);
  void (*_egg_reserved6) (void);
  void (*_egg_reserved7) (void);
};

typedef void (*EggPrinterOptionSetFunc) (EggPrinterOption  *option,
					 gpointer           user_data);


GType   egg_printer_option_set_get_type       (void) G_GNUC_CONST;

EggPrinterOptionSet *egg_printer_option_set_new              (void);
void                 egg_printer_option_set_add              (EggPrinterOptionSet     *set,
							      EggPrinterOption        *option);
void                 egg_printer_option_set_remove           (EggPrinterOptionSet     *set,
							      EggPrinterOption        *option);
EggPrinterOption *   egg_printer_option_set_lookup           (EggPrinterOptionSet     *set,
							      const char              *name);
void                 egg_printer_option_set_foreach          (EggPrinterOptionSet     *set,
							      EggPrinterOptionSetFunc  func,
							      gpointer                 user_data);
void                 egg_printer_option_set_clear_conflicts  (EggPrinterOptionSet     *set);
GList *              egg_printer_option_set_get_groups       (EggPrinterOptionSet     *set);
void                 egg_printer_option_set_foreach_in_group (EggPrinterOptionSet     *set,
							      const char              *group,
							      EggPrinterOptionSetFunc  func,
							      gpointer                 user_data);

G_END_DECLS

#endif /* __EGG_PRINTER_OPTION_SET_H__ */
