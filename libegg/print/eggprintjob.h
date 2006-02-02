/* GTK - The GIMP Toolkit
 * eggprintjob.h: Print Job
 * Copyright (C) 2005, Red Hat, Inc.
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

#ifndef __EGG_PRINT_JOB_H__
#define __EGG_PRINT_JOB_H__

#include <eggprintsettings.h>
#include <cairo.h>

G_BEGIN_DECLS

typedef struct _EggPrintJob EggPrintJob;

#define EGG_TYPE_PRINT_JOB    (egg_print_job_get_type ())
#define EGG_PRINT_JOB(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_JOB, EggPrintJob))
#define EGG_IS_PRINT_JOB(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_JOB))

GType egg_print_job_get_type (void);

EggPrintJob *egg_print_job_new (EggPrintSettings *initial_settings);

EggPrintSettings *egg_print_job_get_settings (EggPrintJob *job);
cairo_surface_t * egg_print_job_get_surface  (EggPrintJob *job);

/* cairo_create (egg_print_job_get_surface (job)) */
cairo_t *        egg_print_job_create_cairo (EggPrintJob *job);

/* Send the job to the printer */
void             egg_print_job_end          (EggPrintJob *job);

G_END_DECLS

#endif /* __EGG_PRINT_JOB_H__ */
