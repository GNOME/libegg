/* EggFileFormatChooser
 * Copyright (C) 2007 Mathias Hasselmann
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
#include <gtk/gtk.h>
#include <stdlib.h>

#include "eggfileformatchooser.h"

#define ICON_NAME_ODS   "gnome-mime-application-vnd.oasis.opendocument.spreadsheet"
#define ICON_NAME_EXCEL "gnome-mime-application-vnd.ms-excel"
#define ICON_NAME_HTML  "gnome-mime-text-html"

static void
setup_file_formats (EggFileFormatChooser *chooser)
{
  guint dummy, ods, excel, html;

  dummy = egg_file_format_chooser_add_format (
    chooser, 0, "Dummy Format (shall be removed)",
    NULL, NULL);

  ods = egg_file_format_chooser_add_format (
    chooser, 0, "OpenDocument Spreadsheet",
    ICON_NAME_ODS, "ods", NULL);

  excel = egg_file_format_chooser_add_format (
    chooser, 0, "Excel Spreadsheet",
    ICON_NAME_EXCEL, NULL);

  egg_file_format_chooser_add_format (
      chooser, excel, "Excel 2003 Spreadsheet (OpenXML)",
      ICON_NAME_EXCEL, "xml", NULL);
  egg_file_format_chooser_add_format (
      chooser, excel, "Excel 97 Spreadsheet",
      ICON_NAME_EXCEL, "xls", NULL);

  html = egg_file_format_chooser_add_format (
    chooser, 0, "HTML Document",
    ICON_NAME_HTML, NULL);

  egg_file_format_chooser_add_format (
      chooser, html, "Full HTML Document",
      ICON_NAME_EXCEL, "html", "htm", NULL);
  egg_file_format_chooser_add_format (
      chooser, html, "HTML Document Fragment",
      ICON_NAME_EXCEL, "html", "htm", NULL);

  egg_file_format_chooser_remove_format (chooser, dummy);

  g_assert (2 == ods);
  g_assert (3 == excel);
  g_assert (6 == html);
}

static void
verify_format_guessing (EggFileFormatChooser *chooser)
{
  const struct {
    const gchar *filename;
    guint format;
  } tests[] = {
    { "test.ods",  2 },
    { "test.xml",  4 },
    { "test.xls",  5 },
    { "test.htm",  7 },
    { "test.html", 7 },
    { "test.txt",  0 },
    { "test",      0 },
    { NULL,        0 }
  };

  int i;

  for(i = 0; tests[i].filename; ++i)
    {
      guint format;

      format = egg_file_format_chooser_get_format (chooser, tests[i].filename);

      g_print ("%s: format=%d, expected=%d (%s)\n", 
               tests[i].filename, format, tests[i].format,
               format == tests[i].format ? "SUCCESS" : "FAILED");
    }
}

static void
verify_append_extension (EggFileFormatChooser *chooser)
{
  const struct {
    guint format;
    const gchar *filename;
    const gchar *expected;
  } tests[] = {
    { 2, "test", "test.ods" },
    { 2, "test.ods", "test.ods" },
    { 2, "test.xml", "test.xml.ods" },
    { 3, "test", "test.xml" },
    { 4, "test", "test.xml" },
    { 5, "test", "test.xls" },
    { 6, "test", "test.html" },
    { 7, "test", "test.html" },
    { 8, "test", "test.html" },
    { 0, NULL }
  };

  int i;

  for(i = 0; tests[i].format; ++i)
    {
      gchar *filename;
      gchar *result;

      filename = egg_file_format_chooser_append_extension (
        chooser, tests[i].filename, tests[i].format);

      result = 
        (NULL == filename && NULL == tests[i].expected) ||
        (NULL != filename && NULL != tests[i].expected &&
         g_str_equal (filename, tests[i].expected)) ?
         "SUCCESS" : "FAILED";

      g_print ("format: %d, filename: %s, expected: %s => %s (%s)\n",
               i, tests[i].filename, tests[i].expected,
               filename, result);

      g_free (filename);
    }
}

static void
selection_changed (EggFileFormatChooser *chooser)
{
  guint format = egg_file_format_chooser_get_format (chooser, NULL);
  g_print ("selected format: %d\n", format);
}

static guint
run_file_chooser (EggFileFormatChooser *format_chooser,
                  const gchar          *default_filename)
{
  GtkWidget *file_chooser;
  gchar *filename = NULL;
  guint format = 0;

  file_chooser = gtk_file_chooser_dialog_new (
    "Testing EggFileFormatChooser", NULL,
    GTK_FILE_CHOOSER_ACTION_SAVE,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
    NULL);

  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (file_chooser),
                                     GTK_WIDGET (format_chooser));
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (file_chooser),
                                     default_filename);

  if (GTK_RESPONSE_ACCEPT == gtk_dialog_run (GTK_DIALOG (file_chooser)))
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));

  if (filename)
    {
      format = egg_file_format_chooser_get_format (format_chooser, filename);
      g_print ("%s: format=%d\n", filename, format);
      g_free (filename);
    }

  gtk_widget_destroy (file_chooser);
  return format;
}

int
main (int argc, char *argv[]) 
{
  GtkWidget *format_chooser = NULL;
  const gchar *filename = "test.ods";
  guint format = 0;

  gtk_init (&argc, &argv);

  if (argc > 1)
    format = atoi (argv[1]);
  if (argc > 2)
    filename = argv[2];

  format_chooser = egg_file_format_chooser_new ();
  setup_file_formats (EGG_FILE_FORMAT_CHOOSER (format_chooser));
  verify_format_guessing (EGG_FILE_FORMAT_CHOOSER (format_chooser));
  verify_append_extension (EGG_FILE_FORMAT_CHOOSER (format_chooser));

  g_signal_connect (format_chooser, "selection-changed",
                    G_CALLBACK (selection_changed), NULL);

  egg_file_format_chooser_set_format (EGG_FILE_FORMAT_CHOOSER (format_chooser),
                                      format);

  run_file_chooser (EGG_FILE_FORMAT_CHOOSER (format_chooser), filename);

  format_chooser = g_object_ref (egg_file_format_chooser_new ());
  egg_file_format_chooser_add_pixbuf_formats (EGG_FILE_FORMAT_CHOOSER (format_chooser),
                                              0, NULL);

  format = run_file_chooser (EGG_FILE_FORMAT_CHOOSER (format_chooser), "test.png");

  if (format > 0)
    {
      const gchar *pixbuf_format = 
        egg_file_format_chooser_get_format_data (EGG_FILE_FORMAT_CHOOSER (format_chooser), 
                                                 format);
      g_print ("choosen pixbuf format is %s\n", pixbuf_format);
    }

  g_object_unref (format_chooser);

  return 0;
}

/* vim: set sw=2 sta et: */
