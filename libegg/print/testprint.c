/* EGG - The GIMP Toolkit
 * eggprintsettings.h: Print Settings
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

#include "eggprint.h"
#include <gtk/gtk.h>

int
main (int argc, char **argv)
{
  EggPrintDialog *dialog;
  EggPrintSettings *settings;
  EggPrintJob *job;
  cairo_t *cr;

  gtk_init (&argc, &argv);

  settings = egg_print_settings_new ();

  g_object_set (settings,
		"output-filename", "test.pdf",
		NULL);

  dialog = egg_print_dialog_new (settings);

  egg_print_dialog_run (dialog);

  job = egg_print_job_new (settings);

  cr = egg_print_job_create_cairo (job);

  cairo_rectangle (cr, 100, 100, 200, 200);
  cairo_fill (cr);

  cairo_show_page (cr);

  cairo_destroy (cr);

  egg_print_job_end (job);
  
  return 0;
}
