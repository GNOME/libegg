/* GTK - The GIMP Toolkit
 * eggprintjob.c: Print Job
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

#include <cairo-pdf.h>
#include "eggprintjob.h"

#define EGG_IS_PRINT_JOB_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_JOB))
#define EGG_PRINT_JOB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_JOB, EggPrintJobClass))
#define EGG_PRINT_JOB_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_JOB, EggPrintJobClass))

typedef struct _EggPrintJobClass EggPrintJobClass;

struct _EggPrintJob
{
  GObject parent_instance;

  EggPrintSettings *settings;
  cairo_surface_t *surface;
};

struct _EggPrintJobClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (EggPrintJob, egg_print_job, G_TYPE_OBJECT)

static void
egg_print_job_finalize (GObject *object)
{
  EggPrintJob *job = EGG_PRINT_JOB (object);
  
  g_object_unref (job->settings);
  
  G_OBJECT_CLASS (egg_print_job_parent_class)->finalize (object);
}

static void
egg_print_job_class_init (EggPrintJobClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_job_finalize;
}

static void
egg_print_job_init (EggPrintJob *job)
{
}
  
EggPrintJob *
egg_print_job_new (EggPrintSettings *initial_settings)
{
  EggPrintJob *job = g_object_new (EGG_TYPE_PRINT_JOB, NULL);
  
  job->settings = egg_print_settings_copy (initial_settings);

  return job;
}

EggPrintSettings *
egg_print_job_get_settings (EggPrintJob *job)
{
  g_return_val_if_fail (job != NULL, NULL);
  
  return job->settings;
}

cairo_surface_t *
egg_print_job_get_surface (EggPrintJob *job)
{
  g_return_val_if_fail (job != NULL, NULL);

  if (!job->surface)
    {
      gchar *output_filename;

      g_object_get (job->settings,
		    "output-filename", &output_filename,
		    NULL);

      job->surface = cairo_pdf_surface_create (output_filename,
					       8.5 * 72, 11. * 72);
    }

  return job->surface;
}

cairo_t *
egg_print_job_create_cairo (EggPrintJob *job)
{
  g_return_val_if_fail (EGG_IS_PRINT_JOB (job), NULL);

  return cairo_create (egg_print_job_get_surface (job));
}

void
egg_print_job_end (EggPrintJob *job)
{
  g_return_if_fail (EGG_IS_PRINT_JOB (job));

  if (job->surface)
    {
      cairo_surface_finish (job->surface);
      cairo_surface_destroy (job->surface);

      job->surface = NULL;
    }
}
