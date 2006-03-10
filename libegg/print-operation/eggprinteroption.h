/* GTK - The GIMP Toolkit
 * eggprinteroption.h: printer option
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

#ifndef __EGG_PRINTER_OPTION_H__
#define __EGG_PRINTER_OPTION_H__

/* This is a "semi-private" header; it is meant only for
 * alternate EggPrintDialog backend modules; no stability guarantees 
 * are made at this point
 */
#ifndef EGG_PRINT_BACKEND_ENABLE_UNSUPPORTED
#error "EggPrintBackend is not supported API for general use"
#endif

#include <glib-object.h>

G_BEGIN_DECLS

#define EGG_TYPE_PRINTER_OPTION             (egg_printer_option_get_type ())
#define EGG_PRINTER_OPTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER_OPTION, EggPrinterOption))
#define EGG_IS_PRINTER_OPTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER_OPTION))

typedef struct _EggPrinterOption       EggPrinterOption;
typedef struct _EggPrinterOptionClass  EggPrinterOptionClass;

#define EGG_PRINTER_OPTION_GROUP_IMAGE_QUALITY "ImageQuality"
#define EGG_PRINTER_OPTION_GROUP_FINISHING "Finishing"

typedef enum {
  EGG_PRINTER_OPTION_TYPE_BOOLEAN,
  EGG_PRINTER_OPTION_TYPE_PICKONE,
  EGG_PRINTER_OPTION_TYPE_STRING
} EggPrinterOptionType;

struct _EggPrinterOption
{
  GObject parent_instance;

  char *name;
  char *display_text;
  EggPrinterOptionType type;

  char *value;
  
  int num_choices;
  char **choices;
  char **choices_display;
  
  gboolean has_conflict;
  char *group;
};

struct _EggPrinterOptionClass
{
  GObjectClass parent_class;

  void (*changed) (EggPrinterOption *option);

  /* Padding for future expansion */
  void (*_egg_reserved1) (void);
  void (*_egg_reserved2) (void);
  void (*_egg_reserved3) (void);
  void (*_egg_reserved4) (void);
  void (*_egg_reserved5) (void);
  void (*_egg_reserved6) (void);
  void (*_egg_reserved7) (void);
};

GType   egg_printer_option_get_type       (void) G_GNUC_CONST;

EggPrinterOption *egg_printer_option_new                (const char           *name,
							 const char           *display_text,
							 EggPrinterOptionType  type);
void              egg_printer_option_set                (EggPrinterOption     *setting,
							 const char           *value);
void              egg_printer_option_set_has_conflict   (EggPrinterOption     *setting,
							 gboolean              has_conflict);
void              egg_printer_option_clear_has_conflict (EggPrinterOption     *setting);
void              egg_printer_option_set_boolean        (EggPrinterOption     *setting,
							 gboolean              value);
void              egg_printer_option_allocate_choices   (EggPrinterOption     *setting,
							 int                   num);
void              egg_printer_option_choices_from_array (EggPrinterOption     *setting,
							 int                   num_choices,
							 char                 *choices[],
							 char                 *choices_display[]);


G_END_DECLS

#endif /* __EGG_PRINTER_OPTION_H__ */


