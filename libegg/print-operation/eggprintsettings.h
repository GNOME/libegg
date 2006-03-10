/* EGG - The GIMP Toolkit
 * eggprintsettings.h: Print Settings
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

#ifndef __EGG_PRINT_SETTINGS_H__
#define __EGG_PRINT_SETTINGS_H__

#include <glib-object.h>
#include "eggpapersize.h"

G_BEGIN_DECLS

typedef struct _EggPrintSettings EggPrintSettings;

#define EGG_TYPE_PRINT_SETTINGS    (egg_print_settings_get_type ())
#define EGG_PRINT_SETTINGS(obj)    (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PRINT_SETTINGS, EggPrintSettings))
#define EGG_IS_PRINT_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PRINT_SETTINGS))

typedef void  (*EggPrintSettingsFunc)  (const char *key,
					  const char *value,
					  gpointer  user_data);

typedef struct {
  int start;
  int end;
} EggPageRange;

GType             egg_print_settings_get_type (void);
EggPrintSettings *egg_print_settings_new      (void);

EggPrintSettings *egg_print_settings_copy       (EggPrintSettings     *other);
gboolean          egg_print_settings_has_key    (EggPrintSettings     *settings,
						 const char           *key);
const char *      egg_print_settings_get        (EggPrintSettings     *settings,
						 const char           *key);
void              egg_print_settings_set        (EggPrintSettings     *settings,
						 const char           *key,
						 const char           *value);
void              egg_print_settings_foreach    (EggPrintSettings     *settings,
						 EggPrintSettingsFunc  func,
						 gpointer              user_data);
gboolean          egg_print_settings_get_bool   (EggPrintSettings     *settings,
						 const char           *key);
void              egg_print_settings_set_bool   (EggPrintSettings     *settings,
						 const char           *key,
						 gboolean              value);
double            egg_print_settings_get_double (EggPrintSettings     *settings,
						 const char           *key);
void              egg_print_settings_set_double (EggPrintSettings     *settings,
						 const char           *key,
						 double                value);
double            egg_print_settings_get_length (EggPrintSettings     *settings,
						 const char           *key,
						 EggUnit               unit);
void              egg_print_settings_set_length (EggPrintSettings     *settings,
						 const char           *key,
						 double                value,
						 EggUnit               unit);
int               egg_print_settings_get_int    (EggPrintSettings     *settings,
						 const char           *key);
void              egg_print_settings_set_int    (EggPrintSettings     *settings,
						 const char           *key,
						 int                   value);

#define EGG_PRINT_SETTINGS_PRINTER "printer"
#define EGG_PRINT_SETTINGS_ORIENTATION "orientation"
#define EGG_PRINT_SETTINGS_PAPER_FORMAT "paper-format"
#define EGG_PRINT_SETTINGS_PAPER_WIDTH "paper-width"
#define EGG_PRINT_SETTINGS_PAPER_HEIGHT "paper-height"
#define EGG_PRINT_SETTINGS_NUM_COPIES "num-copies"
#define EGG_PRINT_SETTINGS_DEFAULT_SOURCE "default-source"
#define EGG_PRINT_SETTINGS_QUALITY "quality"
#define EGG_PRINT_SETTINGS_RESOLUTION "resolution"
#define EGG_PRINT_SETTINGS_USE_COLOR "use-color"
#define EGG_PRINT_SETTINGS_DUPLEX "duplex"
#define EGG_PRINT_SETTINGS_COLLATE "collate"
#define EGG_PRINT_SETTINGS_REVERSE "reverse"
#define EGG_PRINT_SETTINGS_MEDIA_TYPE "media-type"
#define EGG_PRINT_SETTINGS_DITHER "dither"
#define EGG_PRINT_SETTINGS_SCALE "scale"
#define EGG_PRINT_SETTINGS_PRINT_PAGES "print-pages"
#define EGG_PRINT_SETTINGS_PAGE_RANGES "page-ranges"
#define EGG_PRINT_SETTINGS_PAGE_SET "page-set"
#define EGG_PRINT_SETTINGS_PRINT_TO_FILE "print-to-file"
#define EGG_PRINT_SETTINGS_FINISHINGS "finishings"
#define EGG_PRINT_SETTINGS_NUMBER_UP "number-up"
#define EGG_PRINT_SETTINGS_OUTPUT_BIN "output-bin"

#define EGG_PRINT_SETTINGS_WIN32_DRIVER_VERSION "win32-driver-version"
#define EGG_PRINT_SETTINGS_WIN32_DRIVER_EXTRA "win32-driver-extra"

/* Helpers: */

const char *       egg_print_settings_get_printer        (EggPrintSettings   *settings);
void               egg_print_settings_set_printer        (EggPrintSettings   *settings,
							  const char         *printer);
EggPageOrientation egg_print_settings_get_orientation    (EggPrintSettings   *settings);
void               egg_print_settings_set_orientation    (EggPrintSettings   *settings,
							  EggPageOrientation  orientation);
EggPaperSize *     egg_print_settings_get_paper_size     (EggPrintSettings   *settings);
void               egg_print_settings_set_paper_size     (EggPrintSettings   *settings,
							  EggPaperSize       *paper_size);
double             egg_print_settings_get_paper_width    (EggPrintSettings   *settings,
							  EggUnit             unit);
void               egg_print_settings_set_paper_width    (EggPrintSettings   *settings,
							  double              width,
							  EggUnit             unit);
double             egg_print_settings_get_paper_height   (EggPrintSettings   *settings,
							  EggUnit             unit);
void               egg_print_settings_set_paper_height   (EggPrintSettings   *settings,
							  double              width,
							  EggUnit             unit);
gboolean           egg_print_settings_get_use_color      (EggPrintSettings   *settings);
void               egg_print_settings_set_use_color      (EggPrintSettings   *settings,
							  gboolean            use_color);
gboolean           egg_print_settings_get_collate        (EggPrintSettings   *settings);
void               egg_print_settings_set_collate        (EggPrintSettings   *settings,
							  gboolean            collate);
gboolean           egg_print_settings_get_reverse        (EggPrintSettings   *settings);
void               egg_print_settings_set_reverse        (EggPrintSettings   *settings,
							  gboolean            reverse);
EggPrintDuplex     egg_print_settings_get_duplex         (EggPrintSettings   *settings);
void               egg_print_settings_set_duplex         (EggPrintSettings   *settings,
							  EggPrintDuplex      duplex);
EggPrintQuality    egg_print_settings_get_quality        (EggPrintSettings   *settings);
void               egg_print_settings_set_quality        (EggPrintSettings   *settings,
							  EggPrintQuality     quality);
int                egg_print_settings_get_num_copies     (EggPrintSettings   *settings);
void               egg_print_settings_set_num_copies     (EggPrintSettings   *settings,
							  int                 num_copies);
int                egg_print_settings_get_number_up      (EggPrintSettings   *settings);
void               egg_print_settings_set_number_up      (EggPrintSettings   *settings,
							  int                 number_up);
int                egg_print_settings_get_resolution     (EggPrintSettings   *settings);
void               egg_print_settings_set_resolution     (EggPrintSettings   *settings,
							  int                 resolution);
double             egg_print_settings_get_scale          (EggPrintSettings   *settings);
void               egg_print_settings_set_scale          (EggPrintSettings   *settings,
							  double              scale);
gboolean           egg_print_settings_get_print_to_file  (EggPrintSettings   *settings);
void               egg_print_settings_set_print_to_file  (EggPrintSettings   *settings,
							  gboolean            print_to_file);
EggPrintPages      egg_print_settings_get_print_pages    (EggPrintSettings   *settings);
void               egg_print_settings_set_print_pages    (EggPrintSettings   *settings,
							  EggPrintPages       pages);
EggPageRange *     egg_print_settings_get_page_ranges    (EggPrintSettings   *settings,
							  int                *num_ranges);
void               egg_print_settings_set_page_ranges    (EggPrintSettings   *settings,
							  EggPageRange       *page_ranges,
							  int                 num_ranges);
EggPageSet         egg_print_settings_get_page_set       (EggPrintSettings   *settings);
void               egg_print_settings_set_page_set       (EggPrintSettings   *settings,
							  EggPageSet          page_set);
const char *       egg_print_settings_get_default_source (EggPrintSettings   *settings);
void               egg_print_settings_set_default_source (EggPrintSettings   *settings,
							  const char         *default_source);
const char *       egg_print_settings_get_media_type     (EggPrintSettings   *settings);
void               egg_print_settings_set_media_type     (EggPrintSettings   *settings,
							  const char         *media_type);
const char *       egg_print_settings_get_dither         (EggPrintSettings   *settings);
void               egg_print_settings_set_dither         (EggPrintSettings   *settings,
							  const char         *dither);
const char *       egg_print_settings_get_finishings     (EggPrintSettings   *settings);
void               egg_print_settings_set_finishings     (EggPrintSettings   *settings,
							  const char         *finishings);
const char *       egg_print_settings_get_output_bin     (EggPrintSettings   *settings);
void               egg_print_settings_set_output_bin     (EggPrintSettings   *settings,
							  const char         *output_bin);

G_END_DECLS

#endif /* __EGG_PRINT_SETTINGS_H__ */
