/* EGG - The GIMP Toolkit
 * testprint.c: Print example
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

#include <math.h>
#include <pango/pangocairo.h>
#include <gtk/gtk.h>
#include "eggprint.h"
#include "testprintfileoperation.h"

static void
request_page_setup (EggPrintOperation *operation,
		    EggPrintContext *context,
		    int page_nr,
		    EggPageSetup *setup)
{
  if (page_nr == 1)
    egg_page_setup_set_orientation (setup, EGG_PAGE_ORIENTATION_LANDSCAPE);
}

static void
draw_page (EggPrintOperation *operation,
	   EggPrintContext *context,
	   int page_nr)
{
  cairo_t *cr;
  PangoLayout *layout;
  PangoFontDescription *desc;
  
  cr = egg_print_context_get_cairo (context);

  /* Draw a red rectangle, as wide as the paper inside the margins */
  cairo_set_source_rgb (cr, 1.0, 0, 0);
  cairo_rectangle (cr, 0, 0, egg_print_context_get_width (context), 50);
  
  cairo_fill (cr);

  /* Draw some lines */
  cairo_move_to (cr, 20, 10);
  cairo_line_to (cr, 40, 20);
  cairo_arc (cr, 60, 60, 20, 0, M_PI);
  cairo_line_to (cr, 80, 20);
  
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_set_line_width (cr, 5);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
  
  cairo_stroke (cr);

  /* Draw some text */
  
  layout = egg_print_context_create_layout (context);
  pango_layout_set_text (layout, "Hello World! Printing is easy", -1);
  desc = pango_font_description_from_string ("sans 28");
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  cairo_move_to (cr, 30, 20);
  pango_cairo_layout_path (cr, layout);

  /* Font Outline */
  cairo_set_source_rgb (cr, 0.93, 1.0, 0.47);
  cairo_set_line_width (cr, 0.5);
  cairo_stroke_preserve (cr);

  /* Font Fill */
  cairo_set_source_rgb (cr, 0, 0.0, 1.0);
  cairo_fill (cr);
  
  g_object_unref (layout);
}

static void
print_setting (const char *key,
	       const char *value,
	       gpointer  user_data)
{
  g_print ("%s = %s\n", key, value);
}

static void
print_settings (EggPrinterSettings *settings)
{
  g_print ("settings for %p:\n", settings);
  egg_printer_settings_foreach (settings, print_setting, NULL);
}

int
main (int argc, char **argv)
{
  EggPrintOperation *print;
  TestPrintFileOperation *print_file;
  EggPrinterSettings *settings;
  gboolean res;

  gtk_init (&argc, &argv);

  print = egg_print_operation_new ();
  egg_print_operation_set_nr_of_pages (print, 2);
  egg_print_operation_set_unit (print, EGG_UNIT_MM);
  //egg_print_operation_set_pdf_target (print, "test.pdf");
  
  g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), NULL);
  g_signal_connect (print, "request_page_setup", G_CALLBACK (request_page_setup), NULL);
  
  res = egg_print_operation_run (print, NULL);
  
  settings = egg_print_operation_get_printer_settings (print);
  print_settings (settings);

  print_file = test_print_file_operation_new ("testprint.c");
  
  egg_print_operation_set_printer_settings (EGG_PRINT_OPERATION (print_file), settings);
  
  g_object_unref (settings);
  test_print_file_operation_set_font_size (print_file, 12.0);
  //egg_print_operation_set_pdf_target (EGG_PRINT_OPERATION (print_file), "test2.pdf");
  res = egg_print_operation_run (EGG_PRINT_OPERATION (print_file), NULL);

  settings = egg_print_operation_get_printer_settings (print);
  print_settings (settings);
  
  return 0;
}
