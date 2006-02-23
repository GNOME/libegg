/* EGG - The GIMP Toolkit
 * eggprintersettings.h: Printer Settings
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

#ifndef __EGG_PRINTER_SETTINGS_H__
#define __EGG_PRINTER_SETTINGS_H__

#include <glib-object.h>
#include "eggpapersize.h"

G_BEGIN_DECLS

typedef struct _EggPrinterSettings EggPrinterSettings;

#define EGG_TYPE_PRINTER_SETTINGS    (egg_printer_settings_get_type ())
#define EGG_PRINTER_SETTINGS(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINTER_SETTINGS, EggPrinterSettings))
#define EGG_IS_PRINTER_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINTER_SETTINGS))

typedef void  (*EggPrinterSettingsFunc)  (const char *key,
					  const char *value,
					  gpointer  user_data);

typedef struct {
  int start;
  int end;
} EggPageRange;

GType               egg_printer_settings_get_type (void);
EggPrinterSettings *egg_printer_settings_new      (void);
EggPrinterSettings *egg_printer_settings_copy     (EggPrinterSettings *other);

gboolean            egg_printer_settings_has_key  (EggPrinterSettings *settings,
						    const char *key);
const char *        egg_printer_settings_get       (EggPrinterSettings *settings,
						    const char *key);
void                egg_printer_settings_set       (EggPrinterSettings *settings,
						    const char *key,
						    const char *value);
void                egg_printer_settings_foreach   (EggPrinterSettings *settings,
						    EggPrinterSettingsFunc func,
						    gpointer user_data);
gboolean            egg_printer_settings_get_bool  (EggPrinterSettings *settings,
						    const char *key);
void                egg_printer_settings_set_bool  (EggPrinterSettings *settings,
						    const char *key,
						    gboolean value);
double              egg_printer_settings_get_double(EggPrinterSettings *settings,
						    const char *key);
void                egg_printer_settings_set_double(EggPrinterSettings *settings,
						    const char *key,
						    double value);
double              egg_printer_settings_get_length(EggPrinterSettings *settings,
						    const char *key,
						    EggUnit unit);
void                egg_printer_settings_set_length(EggPrinterSettings *settings,
						    const char *key,
						    double value,
						    EggUnit unit);
int                 egg_printer_settings_get_int   (EggPrinterSettings *settings,
						    const char *key);
void                egg_printer_settings_set_int    (EggPrinterSettings *settings,
						     const char *key,
						     int value);

#define EGG_PRINTER_SETTINGS_PRINTER "printer"
#define EGG_PRINTER_SETTINGS_ORIENTATION "orientation"
#define EGG_PRINTER_SETTINGS_PAPER_FORMAT "paper-format"
#define EGG_PRINTER_SETTINGS_PAPER_WIDTH "paper-width"
#define EGG_PRINTER_SETTINGS_PAPER_HEIGHT "paper-height"
#define EGG_PRINTER_SETTINGS_NUM_COPIES "num-copies"
#define EGG_PRINTER_SETTINGS_DEFAULT_SOURCE "default-source"
#define EGG_PRINTER_SETTINGS_QUALITY "quality"
#define EGG_PRINTER_SETTINGS_RESOLUTION "resolution"
#define EGG_PRINTER_SETTINGS_USE_COLOR "use-color"
#define EGG_PRINTER_SETTINGS_DUPLEX "duplex"
#define EGG_PRINTER_SETTINGS_COLLATE "collate"
#define EGG_PRINTER_SETTINGS_REVERSE "reverse"
#define EGG_PRINTER_SETTINGS_MEDIA_TYPE "media-type"
#define EGG_PRINTER_SETTINGS_DITHER "dither"
#define EGG_PRINTER_SETTINGS_SCALE "scale"
#define EGG_PRINTER_SETTINGS_PRINT_PAGES "print-pages"
#define EGG_PRINTER_SETTINGS_PAGE_RANGES "page-ranges"
#define EGG_PRINTER_SETTINGS_PAGE_SET "page-set"
#define EGG_PRINTER_SETTINGS_PRINT_TO_FILE "print-to-file"
#define EGG_PRINTER_SETTINGS_FINISHINGS "finishings"
#define EGG_PRINTER_SETTINGS_NUMBER_UP "number-up"
#define EGG_PRINTER_SETTINGS_OUTPUT_BIN "output-bin"

#define EGG_PRINTER_SETTINGS_WIN32_DRIVER_VERSION "win32-driver-version"
#define EGG_PRINTER_SETTINGS_WIN32_DRIVER_EXTRA "win32-driver-extra"

/* Helpers: */

const char *       egg_printer_settings_get_printer      (EggPrinterSettings *settings);
void               egg_printer_settings_set_printer      (EggPrinterSettings *settings,
							  const char *printer);
EggPageOrientation egg_printer_settings_get_orientation  (EggPrinterSettings *settings);
void               egg_printer_settings_set_orientation  (EggPrinterSettings *settings,
							  EggPageOrientation orientation);
EggPaperSize *     egg_printer_settings_get_paper_size   (EggPrinterSettings *settings);
void               egg_printer_settings_set_paper_size   (EggPrinterSettings *settings,
							  EggPaperSize *paper_size);
double             egg_printer_settings_get_paper_width  (EggPrinterSettings *settings,
							  EggUnit unit);
void               egg_printer_settings_set_paper_width  (EggPrinterSettings *settings,
							  double width, 
							  EggUnit unit);
double             egg_printer_settings_get_paper_height (EggPrinterSettings *settings,
							  EggUnit unit);
void               egg_printer_settings_set_paper_height (EggPrinterSettings *settings,
							  double width, 
							  EggUnit unit);
gboolean           egg_printer_settings_get_use_color    (EggPrinterSettings *settings);
void               egg_printer_settings_set_use_color    (EggPrinterSettings *settings,
							  gboolean use_color);
gboolean           egg_printer_settings_get_collate      (EggPrinterSettings *settings);
void               egg_printer_settings_set_collate      (EggPrinterSettings *settings,
							  gboolean            collate);
gboolean           egg_printer_settings_get_reverse      (EggPrinterSettings *settings);
void               egg_printer_settings_set_reverse      (EggPrinterSettings *settings,
							  gboolean            reverse);
EggPrintDuplex     egg_printer_settings_get_duplex       (EggPrinterSettings *settings);
void               egg_printer_settings_set_duplex       (EggPrinterSettings *settings,
							  EggPrintDuplex      duplex);
EggPrintQuality    egg_printer_settings_get_quality      (EggPrinterSettings *settings);
void               egg_printer_settings_set_quality      (EggPrinterSettings *settings,
							  EggPrintQuality     quality);
int                egg_printer_settings_get_num_copies   (EggPrinterSettings *settings);
void               egg_printer_settings_set_num_copies   (EggPrinterSettings *settings,
							  int                 num_copies);
int                egg_printer_settings_get_number_up    (EggPrinterSettings *settings);
void               egg_printer_settings_set_number_up    (EggPrinterSettings *settings,
							  int                 number_up);
int                egg_printer_settings_get_resolution    (EggPrinterSettings *settings);
void               egg_printer_settings_set_resolution    (EggPrinterSettings *settings,
							   int                 resolution);
double             egg_printer_settings_get_scale        (EggPrinterSettings *settings);
void               egg_printer_settings_set_scale        (EggPrinterSettings *settings,
							  double scale);
gboolean           egg_printer_settings_get_print_to_file(EggPrinterSettings *settings);
void               egg_printer_settings_set_print_to_file(EggPrinterSettings *settings,
							  gboolean            print_to_file);
EggPrintPages      egg_printer_settings_get_print_pages  (EggPrinterSettings *settings);
void               egg_printer_settings_set_print_pages  (EggPrinterSettings *settings,
							  EggPrintPages       pages);
EggPageRange *     egg_printer_settings_get_page_ranges  (EggPrinterSettings *settings,
							  int                *num_ranges);
void               egg_printer_settings_set_page_ranges  (EggPrinterSettings *settings,
							  EggPageRange       *page_ranges,
							  int                 num_ranges);
EggPageSet         egg_printer_settings_get_page_set     (EggPrinterSettings *settings);
void               egg_printer_settings_set_page_set     (EggPrinterSettings *settings,
							  EggPageSet          page_set);
const char *       egg_printer_settings_get_default_source(EggPrinterSettings *settings);
void               egg_printer_settings_set_default_source(EggPrinterSettings *settings,
							   const char *default_source);
const char *       egg_printer_settings_get_media_type   (EggPrinterSettings *settings);
void               egg_printer_settings_set_media_type   (EggPrinterSettings *settings,
							  const char *media_type);
const char *       egg_printer_settings_get_dither       (EggPrinterSettings *settings);
void               egg_printer_settings_set_dither       (EggPrinterSettings *settings,
							  const char *dither);
const char *       egg_printer_settings_get_finishings   (EggPrinterSettings *settings);
void               egg_printer_settings_set_finishings   (EggPrinterSettings *settings,
							  const char *finishings);
const char *       egg_printer_settings_get_output_bin   (EggPrinterSettings *settings);
void               egg_printer_settings_set_output_bin   (EggPrinterSettings *settings,
							  const char *output_bin);

G_END_DECLS

#endif /* __EGG_PRINTER_SETTINGS_H__ */
