/* EggPrintJob
 * Copyright (C) 2006 John (J5) Palmieri  <johnp@redhat.com>
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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "eggintl.h"
#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>

#include "eggprintjob.h"
#include "eggprinter.h"
#include "eggprintbackend.h"

struct _EggPrintJobPrivate
{
  gchar *title;

  gint cache_fd;
  cairo_surface_t *surface;

  EggPrintSettings *settings;
  EggPageSetup *page_setup;
  EggPrintBackend *backend;  
  EggPrinter *printer;

  gint printer_set : 1;
  gint page_setup_set : 1;
  gint settings_set  : 1;
  gint prepped     : 1;
};


#define EGG_PRINT_JOB_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), EGG_TYPE_PRINT_JOB, EggPrintJobPrivate))

static void egg_print_job_finalize     (GObject      *object);
static void egg_print_job_set_property (GObject      *object,
					guint         prop_id,
					const GValue *value,
					GParamSpec   *pspec);
static void egg_print_job_get_property (GObject      *object,
					guint         prop_id,
					GValue       *value,
					GParamSpec   *pspec);

enum {
  PROP_0,
  EGG_PRINT_JOB_PROP_TITLE,
  EGG_PRINT_JOB_PROP_PRINTER,
  EGG_PRINT_JOB_PROP_PAGE_SETUP,
  EGG_PRINT_JOB_PROP_SETTINGS
};

G_DEFINE_TYPE (EggPrintJob, egg_print_job, G_TYPE_OBJECT);

static void
egg_print_job_class_init (EggPrintJobClass *class)
{
  GObjectClass *object_class;
  object_class = (GObjectClass *) class;

  object_class->finalize = egg_print_job_finalize;
  object_class->set_property = egg_print_job_set_property;
  object_class->get_property = egg_print_job_get_property;

  g_type_class_add_private (class, sizeof (EggPrintJobPrivate));

  g_object_class_install_property (G_OBJECT_CLASS (class),
                                   EGG_PRINT_JOB_PROP_TITLE,
                                   g_param_spec_string ("title",
						        P_("Title"),
						        P_("Title of the print job"),
						        NULL,
							GTK_PARAM_WRITABLE |
						        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (G_OBJECT_CLASS (class),
                                   EGG_PRINT_JOB_PROP_PRINTER,
                                   g_param_spec_object ("printer",
						        P_("Printer"),
						        P_("Printer to print the job to"),
						        EGG_TYPE_PRINTER,
							GTK_PARAM_READWRITE |
						        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (G_OBJECT_CLASS (class),
                                   EGG_PRINT_JOB_PROP_SETTINGS,
                                   g_param_spec_object ("settings",
						        P_("Settings"),
						        P_("Printer settings"),
						        EGG_TYPE_PRINT_SETTINGS,
							GTK_PARAM_READWRITE |
						        G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (G_OBJECT_CLASS (class),
                                   EGG_PRINT_JOB_PROP_PAGE_SETUP,
                                   g_param_spec_object ("page-setup",
						        P_("Page Setup"),
						        P_("Page Setup"),
						        EGG_TYPE_PAGE_SETUP,
							GTK_PARAM_READWRITE |
						        G_PARAM_CONSTRUCT_ONLY));
}

static void
egg_print_job_init (EggPrintJob *print_job)
{
  print_job->priv = EGG_PRINT_JOB_GET_PRIVATE (print_job); 
  print_job->priv->cache_fd = 0;

  print_job->priv->title = NULL;
  print_job->priv->surface = NULL;
  print_job->priv->backend = NULL;
  print_job->priv->printer = NULL;

  print_job->priv->printer_set = FALSE;
  print_job->priv->settings_set = FALSE;
  print_job->priv->page_setup_set = FALSE;
}

static void
egg_print_job_finalize (GObject *object)
{
  g_return_if_fail (object != NULL);

  EggPrintJob *print_job = EGG_PRINT_JOB (object);

  if (print_job->priv->cache_fd != 0)
    close (print_job->priv->cache_fd);
  
  if (print_job->priv->backend)
    g_object_unref (G_OBJECT (print_job->priv->backend));

  if (print_job->priv->printer)
    g_object_unref (G_OBJECT (print_job->priv->printer));

  if (print_job->priv->surface)
    cairo_surface_destroy (print_job->priv->surface);

  if (print_job->priv->settings)
    g_object_unref (print_job->priv->settings);
  
  if (print_job->priv->page_setup)
    g_object_unref (print_job->priv->page_setup);
  
  if (G_OBJECT_CLASS (egg_print_job_parent_class)->finalize)
    G_OBJECT_CLASS (egg_print_job_parent_class)->finalize (object);
}

/**
 * egg_print_job_new:
 *
 * Creates a new #EggPrintJob.
 *
 * Return value: a new #EggPrintJob
 *
 * Since: 2.8
 **/
EggPrintJob *
egg_print_job_new (const gchar *title,
		   EggPrintSettings *settings,
		   EggPageSetup *page_setup,
                   EggPrinter *printer)
{
  GObject *result;
  
  result = g_object_new (EGG_TYPE_PRINT_JOB,
                         "title", title,
			 "printer", printer,
			 "settings", settings,
			 "page-setup", page_setup,
			 NULL);

  return (EggPrintJob *) result;
}

EggPrintSettings *
egg_print_job_get_settings (EggPrintJob *print_job)
{
  g_assert (EGG_IS_PRINT_SETTINGS (print_job->priv->settings));
  return print_job->priv->settings;
}


EggPrinter *
egg_print_job_get_printer (EggPrintJob *print_job)
{
  g_return_val_if_fail (EGG_IS_PRINT_JOB (print_job), NULL);
  
  return print_job->priv->printer;
}


cairo_surface_t *
egg_print_job_get_surface (EggPrintJob *print_job)
{
  g_return_val_if_fail (EGG_IS_PRINT_JOB (print_job), NULL);

  return print_job->priv->surface;
}

gboolean 
egg_print_job_prep (EggPrintJob *job, 
                    GError **error)
{
  char *filename;
  double width, height;
  
  /* TODO: populate GError */
  if (!(job->priv->printer_set &&
	job->priv->settings_set &&
	job->priv->page_setup_set))
    return FALSE;

  job->priv->prepped = TRUE;
  job->priv->cache_fd = g_file_open_tmp ("eggprint_XXXXXX", 
                                         &filename, 
					 error);
  fchmod (job->priv->cache_fd, S_IRUSR | S_IWUSR);
  unlink (filename);

  if (error != NULL && *error != NULL)
    return FALSE;

  width = egg_page_setup_get_paper_width (job->priv->page_setup, EGG_UNIT_POINTS);
  height = egg_page_setup_get_paper_height (job->priv->page_setup, EGG_UNIT_POINTS);

  job->priv->surface = _egg_printer_create_cairo_surface (job->priv->printer,
							  width, height,
							  job->priv->cache_fd);

  _egg_printer_prepare_for_print (job->priv->printer,
				  job->priv->settings,
				  job->priv->page_setup);
  
  return TRUE;
}

static void
egg_print_job_set_property (GObject      *object,
	                    guint         prop_id,
	                    const GValue *value,
                            GParamSpec   *pspec)

{
  EggPrintJob *impl = EGG_PRINT_JOB (object);

  switch (prop_id)
    {
    case EGG_PRINT_JOB_PROP_TITLE:
      impl->priv->title = g_value_dup_string (value);
      break;
    
    case EGG_PRINT_JOB_PROP_PRINTER:
      impl->priv->printer = EGG_PRINTER (g_value_dup_object (value));
      impl->priv->printer_set = TRUE;
      impl->priv->backend = g_object_ref (egg_printer_get_backend (impl->priv->printer));
      break;

    case EGG_PRINT_JOB_PROP_PAGE_SETUP:
      impl->priv->page_setup = EGG_PAGE_SETUP (g_value_dup_object (value));
      impl->priv->page_setup_set = TRUE;
      break;
      
    case EGG_PRINT_JOB_PROP_SETTINGS:
      impl->priv->settings = EGG_PRINT_SETTINGS (g_value_dup_object (value));
      impl->priv->settings_set = TRUE;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_print_job_get_property (GObject    *object,
			    guint       prop_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  EggPrintJob *impl = EGG_PRINT_JOB (object);

  switch (prop_id)
    {
    case EGG_PRINT_JOB_PROP_PRINTER:
      g_value_set_object (value, impl->priv->printer);
      break;
    case EGG_PRINT_JOB_PROP_SETTINGS:
      g_value_set_object (value, impl->priv->settings);
      break;
    case EGG_PRINT_JOB_PROP_PAGE_SETUP:
      g_value_set_object (value, impl->priv->page_setup);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

gboolean
egg_print_job_send (EggPrintJob *print_job,
                    EggPrintJobCompleteFunc callback,
                    gpointer user_data,
		    GError **error)
{
  /* TODO: set gerror */
  if (!print_job->priv->prepped)
    return FALSE;

  lseek (print_job->priv->cache_fd, 0, SEEK_SET);
  egg_print_backend_print_stream (print_job->priv->backend,
                                  print_job,
                                  print_job->priv->title,
				  print_job->priv->cache_fd,
                                  callback,
                                  user_data);

  return TRUE;
}

