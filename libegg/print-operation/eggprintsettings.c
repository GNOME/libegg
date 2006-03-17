/* GTK - The GIMP Toolkit
 * eggprintsettings.c: Print Settings
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

#include "eggprintsettings.h"
#include <string.h>
#include <stdlib.h>
#include <glib/gprintf.h>

#define MM_PER_INCH 25.4
#define POINTS_PER_INCH 72

typedef struct _EggPrintSettingsClass EggPrintSettingsClass;

#define EGG_IS_PRINT_SETTINGS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_SETTINGS))
#define EGG_PRINT_SETTINGS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_SETTINGS, EggPrintSettingsClass))
#define EGG_PRINT_SETTINGS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_SETTINGS, EggPrintSettingsClass))

struct _EggPrintSettings
{
  GObject parent_instance;
  
  GHashTable *hash;
};

struct _EggPrintSettingsClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (EggPrintSettings, egg_print_settings, G_TYPE_OBJECT)

static double
to_mm (double len, EggUnit unit)
{
  switch (unit)
    {
    case EGG_UNIT_MM:
      return len;
    case EGG_UNIT_INCH:
      return len * MM_PER_INCH;
    default:
    case EGG_UNIT_PIXEL:
      g_warning ("Unsupported unit");
      /* Fall through */
    case EGG_UNIT_POINTS:
      return len * (MM_PER_INCH / POINTS_PER_INCH);
      break;
    }
}

static double
from_mm (double len, EggUnit unit)
{
  switch (unit)
    {
    case EGG_UNIT_MM:
      return len;
    case EGG_UNIT_INCH:
      return len / MM_PER_INCH;
    default:
    case EGG_UNIT_PIXEL:
      g_warning ("Unsupported unit");
      /* Fall through */
    case EGG_UNIT_POINTS:
      return len / (MM_PER_INCH / POINTS_PER_INCH);
      break;
    }
}

static void
egg_print_settings_finalize (GObject *object)
{
  EggPrintSettings *settings = EGG_PRINT_SETTINGS (object);

  g_hash_table_destroy (settings->hash);

  G_OBJECT_CLASS (egg_print_settings_parent_class)->finalize (object);
}

static void
egg_print_settings_init (EggPrintSettings *settings)
{
  settings->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
					  g_free, g_free);
}

static void
egg_print_settings_class_init (EggPrintSettingsClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_settings_finalize;
}

EggPrintSettings *
egg_print_settings_new (void)
{
  return g_object_new (EGG_TYPE_PRINT_SETTINGS, NULL);
}

static void
copy_hash_entry  (gpointer  key,
		  gpointer  value,
		  gpointer  user_data)
{
  EggPrintSettings *settings = user_data;

  g_hash_table_insert (settings->hash, 
		       g_strdup (key), 
		       g_strdup (value));
}



EggPrintSettings *
egg_print_settings_copy (EggPrintSettings *other)
{
  EggPrintSettings *settings;

  if (other == NULL)
    return NULL;
  
  g_return_val_if_fail (EGG_IS_PRINT_SETTINGS (other), NULL);

  settings = egg_print_settings_new ();

  g_hash_table_foreach (other->hash,
			copy_hash_entry,
			settings);

  return settings;
}

const char *        
egg_print_settings_get (EggPrintSettings *settings,
			const char *key)
{
  return g_hash_table_lookup (settings->hash, key);
}

void
egg_print_settings_set (EggPrintSettings *settings,
			const char *key,
			const char *value)
{
  if (value == NULL)
    egg_print_settings_unset (settings, key);
  else
    g_hash_table_insert (settings->hash, 
			 g_strdup (key), 
			 g_strdup (value));
}

void
egg_print_settings_unset (EggPrintSettings *settings,
			  const char *key)
{
  g_hash_table_remove (settings->hash, key);
}

gboolean        
egg_print_settings_has_key (EggPrintSettings *settings,
			    const char *key)
{
  return egg_print_settings_get (settings, key) != NULL;
}


gboolean
egg_print_settings_get_bool (EggPrintSettings *settings,
			     const char *key)
{
  const char *val;

  val = egg_print_settings_get (settings, key);
  if (val != NULL && strcmp (val, "true") == 0)
    return TRUE;
  
  return FALSE;
}

static gboolean
egg_print_settings_get_bool_with_default (EggPrintSettings *settings,
					  const char *key,
					  gboolean default_val)
{
  const char *val;

  val = egg_print_settings_get (settings, key);
  if (val != NULL && strcmp (val, "true") == 0)
    return TRUE;

  if (val != NULL && strcmp (val, "false") == 0)
    return FALSE;
  
  return default_val;
}

void
egg_print_settings_set_bool (EggPrintSettings *settings,
			     const char *key,
			     gboolean value)
{
  if (value)
    egg_print_settings_set (settings, key, "true");
  else
    egg_print_settings_set (settings, key, "false");
}

double
egg_print_settings_get_double (EggPrintSettings *settings,
			       const char *key)
{
  const char *val;

  val = egg_print_settings_get (settings, key);
  if (val == NULL)
    return 0.0;

  return g_ascii_strtod (val, NULL);
}

void
egg_print_settings_set_double (EggPrintSettings *settings,
			       const char *key,
			       double value)
{
  char buf[G_ASCII_DTOSTR_BUF_SIZE];
  
  g_ascii_dtostr (buf, G_ASCII_DTOSTR_BUF_SIZE, value);
  egg_print_settings_set (settings, key, buf);
}

double
egg_print_settings_get_length (EggPrintSettings *settings,
			       const char *key,
			       EggUnit unit)
{
  double length = egg_print_settings_get_double (settings, key);
  return from_mm (length, unit);
}

void
egg_print_settings_set_length (EggPrintSettings *settings,
			       const char *key,
			       double length, 
			       EggUnit unit)
{
  egg_print_settings_set_double (settings, key,
				 to_mm (length, unit));
}

int
egg_print_settings_get_int (EggPrintSettings *settings,
			    const char *key)
{
  const char *val;

  val = egg_print_settings_get (settings, key);
  if (val == NULL)
    return 0.0;

  return atoi (val);
}

void
egg_print_settings_set_int (EggPrintSettings *settings,
			    const char *key,
			    int value)
{
  char buf[128];
  g_sprintf(buf, "%d", value);
  egg_print_settings_set (settings, key, buf);
}

void
egg_print_settings_foreach (EggPrintSettings *settings,
			    EggPrintSettingsFunc func,
			    gpointer user_data)
{
  g_hash_table_foreach (settings->hash,
			(GHFunc)func, 
			user_data);
}

const char *       
egg_print_settings_get_printer (EggPrintSettings *settings)
{
  return egg_print_settings_get (settings, EGG_PRINT_SETTINGS_PRINTER);
}

void
egg_print_settings_set_printer (EggPrintSettings *settings,
				const char *printer)
{
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PRINTER, printer);
}

EggPageOrientation
egg_print_settings_get_orientation (EggPrintSettings *settings)
{
  const char *val;

  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_ORIENTATION);

  if (val == NULL && strcmp (val, "portrait") == 0)
    return EGG_PAGE_ORIENTATION_PORTRAIT;

  if (strcmp (val, "landscape") == 0)
    return EGG_PAGE_ORIENTATION_LANDSCAPE;
  
  if (strcmp (val, "reverse_portrait") == 0)
    return EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT;
  
  if (strcmp (val, "reverse_landscape") == 0)
    return EGG_PAGE_ORIENTATION_REVERSE_LANDSCAPE;
  
  return EGG_PAGE_ORIENTATION_PORTRAIT;
}

void
egg_print_settings_set_orientation (EggPrintSettings *settings,
				    EggPageOrientation orientation)
{
  const char *val;

  switch (orientation)
    {
    case EGG_PAGE_ORIENTATION_LANDSCAPE:
      val = "landscape";
      break;
    default:
    case EGG_PAGE_ORIENTATION_PORTRAIT:
      val = "portrait";
      break;
    case EGG_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
      val = "reverse_landscape";
      break;
    case EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT:
      val = "reverse_portrait";
      break;
    }
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_ORIENTATION, val);
}

EggPaperSize *     
egg_print_settings_get_paper_size (EggPrintSettings *settings)
{
  const char *val;
  const char *name;
  double w, h;

  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_PAPER_FORMAT);
  if (val == NULL)
    return NULL;

  if (g_str_has_prefix (val, "custom-")) 
    {
      name = val + strlen ("custom-");
      w = egg_print_settings_get_paper_width (settings, EGG_UNIT_MM);
      h = egg_print_settings_get_paper_height (settings, EGG_UNIT_MM);
      return egg_paper_size_new_custom (name, name, w, h, EGG_UNIT_MM);
    }

  return egg_paper_size_new (val);
}

void
egg_print_settings_set_paper_size (EggPrintSettings *settings,
				   EggPaperSize *paper_size)
{
  char *custom_name;

  if (paper_size == NULL) 
    {
      egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAPER_FORMAT, NULL);
      egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAPER_WIDTH, NULL);
      egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAPER_HEIGHT, NULL);
    }
  else if (egg_paper_size_is_custom (paper_size)) 
    {
      custom_name = g_strdup_printf ("custom-%s", 
				     egg_paper_size_get_name (paper_size));
      egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAPER_FORMAT, custom_name);
      g_free (custom_name);
      egg_print_settings_set_paper_width (settings, 
					  egg_paper_size_get_width (paper_size, 
								    EGG_UNIT_MM),
					  EGG_UNIT_MM);
      egg_print_settings_set_paper_height (settings, 
					   egg_paper_size_get_height (paper_size, 
								      EGG_UNIT_MM),
					   EGG_UNIT_MM);
    } 
  else
    egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAPER_FORMAT, 
			    egg_paper_size_get_name (paper_size));
}

double
egg_print_settings_get_paper_width (EggPrintSettings *settings,
				    EggUnit unit)
{
  return egg_print_settings_get_length (settings, EGG_PRINT_SETTINGS_PAPER_WIDTH, unit);
}

void
egg_print_settings_set_paper_width (EggPrintSettings *settings,
				    double width, 
				    EggUnit unit)
{
  egg_print_settings_set_length (settings, EGG_PRINT_SETTINGS_PAPER_WIDTH, width, unit);
}

double
egg_print_settings_get_paper_height (EggPrintSettings *settings,
				     EggUnit unit)
{
  return egg_print_settings_get_length (settings, 
					EGG_PRINT_SETTINGS_PAPER_HEIGHT,
					unit);
}

void
egg_print_settings_set_paper_height (EggPrintSettings *settings,
				     double height, 
				     EggUnit unit)
{
  egg_print_settings_set_length (settings, 
				 EGG_PRINT_SETTINGS_PAPER_HEIGHT, 
				 height, unit);
}

gboolean
egg_print_settings_get_use_color (EggPrintSettings *settings)
{
  return egg_print_settings_get_bool_with_default (settings, 
						   EGG_PRINT_SETTINGS_USE_COLOR,
						   TRUE);
}

void
egg_print_settings_set_use_color (EggPrintSettings *settings,
				  gboolean use_color)
{
  egg_print_settings_set_bool (settings,
			       EGG_PRINT_SETTINGS_USE_COLOR, 
			       use_color);
}

gboolean
egg_print_settings_get_collate (EggPrintSettings *settings)
{
  return egg_print_settings_get_bool (settings, 
				      EGG_PRINT_SETTINGS_COLLATE);
}

void
egg_print_settings_set_collate (EggPrintSettings *settings,
				gboolean collate)
{
  egg_print_settings_set_bool (settings,
			       EGG_PRINT_SETTINGS_COLLATE, 
			       collate);
}

gboolean
egg_print_settings_get_reverse (EggPrintSettings *settings)
{
  return egg_print_settings_get_bool (settings, 
				      EGG_PRINT_SETTINGS_REVERSE);
}

void
egg_print_settings_set_reverse (EggPrintSettings *settings,
				  gboolean reverse)
{
  egg_print_settings_set_bool (settings,
			       EGG_PRINT_SETTINGS_REVERSE, 
			       reverse);
}

EggPrintDuplex
egg_print_settings_get_duplex (EggPrintSettings *settings)
{
  const char *val;

  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_DUPLEX);

  if (val == NULL || (strcmp (val, "simplex") == 0))
    return EGG_PRINT_DUPLEX_SIMPLEX;

  if (strcmp (val, "horizontal") == 0)
    return EGG_PRINT_DUPLEX_HORIZONTAL;
  
  if (strcmp (val, "vertical") == 0)
    return EGG_PRINT_DUPLEX_HORIZONTAL;
  
  return EGG_PRINT_DUPLEX_SIMPLEX;
}

void
egg_print_settings_set_duplex (EggPrintSettings *settings,
			       EggPrintDuplex      duplex)
{
  const char *str;

  switch (duplex)
    {
    default:
    case EGG_PRINT_DUPLEX_SIMPLEX:
      str = "simplex";
      break;
    case EGG_PRINT_DUPLEX_HORIZONTAL:
      str = "horizontal";
      break;
    case EGG_PRINT_DUPLEX_VERTICAL:
      str = "vertical";
      break;
    }
  
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_DUPLEX, str);
}

EggPrintQuality
egg_print_settings_get_quality (EggPrintSettings *settings)
{
  const char *val;

  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_QUALITY);

  if (val == NULL || (strcmp (val, "normal") == 0))
    return EGG_PRINT_QUALITY_NORMAL;

  if (strcmp (val, "high") == 0)
    return EGG_PRINT_QUALITY_HIGH;
  
  if (strcmp (val, "low") == 0)
    return EGG_PRINT_QUALITY_LOW;
  
  if (strcmp (val, "draft") == 0)
    return EGG_PRINT_QUALITY_DRAFT;
  
  return EGG_PRINT_QUALITY_NORMAL;
}

void
egg_print_settings_set_quality (EggPrintSettings *settings,
				EggPrintQuality     quality)
{
  const char *str;

  switch (quality)
    {
    default:
    case EGG_PRINT_QUALITY_NORMAL:
      str = "normal";
      break;
    case EGG_PRINT_QUALITY_HIGH:
      str = "high";
      break;
    case EGG_PRINT_QUALITY_LOW:
      str = "low";
      break;
    case EGG_PRINT_QUALITY_DRAFT:
      str = "draft";
      break;
    }
  
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_QUALITY, str);
}

EggPageSet
egg_print_settings_get_page_set (EggPrintSettings *settings)
{
  const char *val;

  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_PAGE_SET);

  if (val == NULL || (strcmp (val, "all") == 0))
    return EGG_PAGE_SET_ALL;

  if (strcmp (val, "even") == 0)
    return EGG_PAGE_SET_EVEN;
  
  if (strcmp (val, "odd") == 0)
    return EGG_PAGE_SET_ODD;
  
  return EGG_PAGE_SET_ALL;
}

void
egg_print_settings_set_page_set (EggPrintSettings *settings,
				 EggPageSet          page_set)
{
  const char *str;

  switch (page_set)
    {
    default:
    case EGG_PAGE_SET_ALL:
      str = "all";
      break;
    case EGG_PAGE_SET_EVEN:
      str = "even";
      break;
    case EGG_PAGE_SET_ODD:
      str = "odd";
      break;
    }
  
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAGE_SET, str);
}

int
egg_print_settings_get_num_copies (EggPrintSettings *settings)
{
  return egg_print_settings_get_int (settings, EGG_PRINT_SETTINGS_NUM_COPIES);
}

void
egg_print_settings_set_num_copies (EggPrintSettings *settings,
				   int                 num_copies)
{
  egg_print_settings_set_int (settings, EGG_PRINT_SETTINGS_NUM_COPIES,
			      num_copies);
}

int
egg_print_settings_get_number_up (EggPrintSettings *settings)
{
  return egg_print_settings_get_int (settings, EGG_PRINT_SETTINGS_NUMBER_UP);
}

void
egg_print_settings_set_number_up (EggPrintSettings *settings,
				    int                 number_up)
{
  egg_print_settings_set_int (settings, EGG_PRINT_SETTINGS_NUMBER_UP,
				number_up);
}

int
egg_print_settings_get_resolution (EggPrintSettings *settings)
{
  return egg_print_settings_get_int (settings, EGG_PRINT_SETTINGS_RESOLUTION);
}

void
egg_print_settings_set_resolution (EggPrintSettings *settings,
				   int                 resolution)
{
  egg_print_settings_set_int (settings, EGG_PRINT_SETTINGS_RESOLUTION,
			      resolution);
}

double
egg_print_settings_get_scale (EggPrintSettings *settings)
{
  return egg_print_settings_get_double (settings, EGG_PRINT_SETTINGS_SCALE);
}

void
egg_print_settings_set_scale (EggPrintSettings *settings,
			      double scale)
{
  egg_print_settings_set_double (settings, EGG_PRINT_SETTINGS_SCALE,
				 scale);
}

gboolean
egg_print_settings_get_print_to_file (EggPrintSettings *settings)
{
  return egg_print_settings_get_bool (settings, 
				      EGG_PRINT_SETTINGS_PRINT_TO_FILE);
}

void
egg_print_settings_set_print_to_file (EggPrintSettings *settings,
				      gboolean print_to_file)
{
  egg_print_settings_set_bool (settings,
			       EGG_PRINT_SETTINGS_PRINT_TO_FILE, 
			       print_to_file);
}

EggPrintPages
egg_print_settings_get_print_pages (EggPrintSettings *settings)
{
  const char *val;

  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_PRINT_PAGES);

  if (val == NULL || (strcmp (val, "all") == 0))
    return EGG_PRINT_PAGES_ALL;

  if (strcmp (val, "current") == 0)
    return EGG_PRINT_PAGES_CURRENT;
  
  if (strcmp (val, "ranges") == 0)
    return EGG_PRINT_PAGES_RANGES;
  
  return EGG_PRINT_PAGES_ALL;
}

void
egg_print_settings_set_print_pages (EggPrintSettings *settings,
				    EggPrintPages       print_pages)
{
  const char *str;

  switch (print_pages)
    {
    default:
    case EGG_PRINT_PAGES_ALL:
      str = "all";
      break;
    case EGG_PRINT_PAGES_CURRENT:
      str = "current";
      break;
    case EGG_PRINT_PAGES_RANGES:
      str = "ranges";
      break;
    }
  
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PRINT_PAGES, str);
}
     


EggPageRange *
egg_print_settings_get_page_ranges (EggPrintSettings *settings,
				    int                *num_ranges)
{
  const char *val;
  gchar **range_strs;
  EggPageRange *ranges;
  int i, n;
  
  val = egg_print_settings_get (settings, EGG_PRINT_SETTINGS_PAGE_RANGES);

  if (val == NULL)
    {
      *num_ranges = 0;
      return NULL;
    }
  
  range_strs = g_strsplit (val, ",", 0);

  for (i = 0; range_strs[i] != NULL; i++)
    ;

  n = i;

  ranges = g_new0 (EggPageRange, n);

  for (i = 0; i < n; i++)
    {
      int start, end;
      char *str;

      start = (int)strtol (range_strs[i], &str, 10);
      end = start;

      if (*str == '-')
	{
	  str++;
	  end = (int)strtol (str, NULL, 10);
	  if (end < start)
	    end = start;
	}

      ranges[i].start = start;
      ranges[i].end = end;
    }

  *num_ranges = n;
  return ranges;
}

void
egg_print_settings_set_page_ranges  (EggPrintSettings *settings,
				     EggPageRange       *page_ranges,
				     int                 num_ranges)
{
  GString *s;
  int i;
  
  s = g_string_new ("");

  for (i = 0; i < num_ranges; i++)
    {
      if (page_ranges[i].start == page_ranges[i].end)
	g_string_append_printf (s, "%d", page_ranges[i].start);
      else
	g_string_append_printf (s, "%d-%d",
				page_ranges[i].start,
				page_ranges[i].end);
      if (i < num_ranges - 1)
	g_string_append (s, ",");
    }

  
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_PAGE_RANGES, 
			  s->str);

  g_string_free (s, TRUE);
}

const char *
egg_print_settings_get_default_source (EggPrintSettings *settings)
{
  return egg_print_settings_get (settings, EGG_PRINT_SETTINGS_DEFAULT_SOURCE);
}

void
egg_print_settings_set_default_source (EggPrintSettings *settings,
				       const char *default_source)
{
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_DEFAULT_SOURCE, default_source);
}
     
const char *
egg_print_settings_get_media_type (EggPrintSettings *settings)
{
  return egg_print_settings_get (settings, EGG_PRINT_SETTINGS_MEDIA_TYPE);
}

/* The set of media types is defined in PWG 5101.1-2002 PWG */
void
egg_print_settings_set_media_type (EggPrintSettings *settings,
				   const char *media_type)
{
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_MEDIA_TYPE, media_type);
}


const char *
egg_print_settings_get_dither (EggPrintSettings *settings)
{
  return egg_print_settings_get (settings, EGG_PRINT_SETTINGS_DITHER);
}

void
egg_print_settings_set_dither (EggPrintSettings *settings,
			       const char *dither)
{
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_DITHER, dither);
}
     
const char *
egg_print_settings_get_finishings (EggPrintSettings *settings)
{
  return egg_print_settings_get (settings, EGG_PRINT_SETTINGS_FINISHINGS);
}

void
egg_print_settings_set_finishings (EggPrintSettings *settings,
				   const char *finishings)
{
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_FINISHINGS, finishings);
}
     
const char *
egg_print_settings_get_output_bin (EggPrintSettings *settings)
{
  return egg_print_settings_get (settings, EGG_PRINT_SETTINGS_OUTPUT_BIN);
}

void
egg_print_settings_set_output_bin (EggPrintSettings *settings,
				   const char *output_bin)
{
  egg_print_settings_set (settings, EGG_PRINT_SETTINGS_OUTPUT_BIN, output_bin);
}
   
