/* EggPrintJob 
 * Copyright (C) 2006 Red Hat,Inc. 
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
#ifndef __EGG_PRINT_JOB_H__
#define __EGG_PRINT_JOB_H__

#include <glib-object.h>
#include <cairo.h>

#include "eggprintsettings.h"

G_BEGIN_DECLS

#define EGG_TYPE_PRINT_JOB                  (egg_print_job_get_type ())
#define EGG_PRINT_JOB(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_JOB, EggPrintJob))
#define EGG_PRINT_JOB_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_JOB, EggPrintJobClass))
#define EGG_IS_PRINT_JOB(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_JOB))
#define EGG_IS_PRINT_JOB_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_JOB))
#define EGG_PRINT_JOB_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_JOB, EggPrintJobClass))


typedef struct _EggPrintJob	     EggPrintJob;
typedef struct _EggPrintJobClass     EggPrintJobClass;
typedef struct _EggPrintJobPrivate   EggPrintJobPrivate;

typedef void (*EggPrintJobCompleteFunc) (EggPrintJob *print_job,
                                         void *user_data, 
                                         GError **error);

struct _EggPrinter;

struct _EggPrintJob
{
  GObject parent_instance;

  EggPrintJobPrivate *priv;
};

struct _EggPrintJobClass
{
  GObjectClass parent_class;


  /* Padding for future expansion */
  void (*_egg_reserved1) (void);
  void (*_egg_reserved2) (void);
  void (*_egg_reserved3) (void);
  void (*_egg_reserved4) (void);
  void (*_egg_reserved5) (void);
  void (*_egg_reserved6) (void);
  void (*_egg_reserved7) (void);
};

GType                    egg_print_job_get_type     (void) G_GNUC_CONST;
EggPrintJob             *egg_print_job_new          (const gchar              *title,
						     EggPrintSettings         *settings,
						     struct _EggPrinter       *printer,
						     gdouble                   width,
						     gdouble                   height);
EggPrintSettings      *  egg_print_job_get_settings (EggPrintJob              *print_job);
struct _EggPrinter      *egg_print_job_get_printer  (EggPrintJob              *print_job);
cairo_surface_t         *egg_print_job_get_surface  (EggPrintJob              *print_job);
gboolean                 egg_print_job_send         (EggPrintJob              *print_job,
						     EggPrintJobCompleteFunc   callback,
						     gpointer                  user_data,
						     GError                  **error);
gboolean                 egg_print_job_prep         (EggPrintJob              *job,
						     GError                  **error);


G_END_DECLS

#endif /* __EGG_PRINT_JOB_H__ */
