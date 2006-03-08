/* GTK - The GIMP Toolkit
 * eggpapersize.c: Paper Size
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

#include "eggpapersize.h"
#include <string.h>
#include <stdlib.h>

#define MM_PER_INCH 25.4
#define POINTS_PER_INCH 72

struct _EggPaperSize
{
  char *name;
  char *display_name;
  double width, height; /* Stored in mm */
  gboolean is_custom;
};

typedef struct {
  const char *name;
  const char *size;
  const char *display_name;
} PageInfo;


#include "paper_names.c"


GType
egg_paper_size_get_type (void)
{
  static GType our_type = 0;
  
  if (our_type == 0)
    our_type = g_boxed_type_register_static ("EggPaperSize",
					     (GBoxedCopyFunc)egg_paper_size_copy,
					     (GBoxedFreeFunc)egg_paper_size_free);
  return our_type;
}

static int
page_info_compare (const void *_a, const void *_b)
{
  const PageInfo *a = _a;
  const PageInfo *b = _b;

  return strcmp (a->name, b->name);
}
  
static double
to_mm (double len, EggUnit unit)
{
  switch (unit) {
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
  switch (unit) {
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

static gboolean
parse_media_size (const char *size,
		  double *width_mm, double *height_mm)
{
  const char *p;
  char *e;
  double short_dim, long_dim;

  p = size;
  
  short_dim = g_ascii_strtod (p, &e);

  if (p == e || *e != 'x')
    return FALSE;

  p = e + 1; /* Skip x */

  long_dim = g_ascii_strtod (p, &e);

  if (p == e)
    return FALSE;

  p = e;

  if (strcmp (p, "in") == 0) {
    short_dim = short_dim * MM_PER_INCH;
    long_dim = long_dim * MM_PER_INCH;
  } else if (strcmp (p, "mm") != 0) {
    return FALSE;
  }

  if (width_mm)
    *width_mm = short_dim;
  if (height_mm)
    *height_mm = long_dim;
  
  return TRUE;  
}


static gboolean
parse_full_media_size_name (const char *full_name,
			    char **name,
			    double *width_mm, double *height_mm)
{
  const char *p;
  const char *end_of_name;
  
  /* From the spec:
   media-size-self-describing-name =
        ( class-in "_" size-name "_" short-dim "x" long-dim "in" ) |
        ( class-mm "_" size-name "_" short-dim "x" long-dim "mm" )
   class-in = "custom" | "na" | "asme" | "roc" | "oe"
   class-mm = "custom" | "iso" | "jis" | "jpn" | "prc" | "om"
   size-name = ( lowalpha | digit ) *( lowalpha | digit | "-" )
   short-dim = dim
   long-dim = dim
   dim = integer-part [fraction-part] | "0" fraction-part
   integer-part = non-zero-digit *digit
   fraction-part = "." *digit non-zero-digit
   lowalpha = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
              "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
              "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
   non-zero-digit = "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
   digit    = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
 */

  p = strchr (full_name, '_');
  if (p == NULL)
    return FALSE;

  p++; /* Skip _ */
  
  p = strchr (p, '_');
  if (p == NULL)
    return FALSE;

  end_of_name = p;

  p++; /* Skip _ */

  if (!parse_media_size (p, width_mm, height_mm))
    return FALSE;

  if (name)
    *name = g_strndup (full_name, end_of_name - full_name);
  
  return TRUE;  
}

EggPaperSize *
egg_paper_size_new (const char *name)
{
  EggPaperSize *size;
  char *short_name;
  double width, height;
  PageInfo *info;

  size = g_new0 (EggPaperSize, 1);

  if (parse_full_media_size_name (name, &short_name, &width, &height)) {
    size->width = width;
    size->height = height;
    size->name = short_name;
    size->display_name = g_strdup (short_name);
  } else {
    PageInfo key;
    key.name = name;
    info = bsearch (&key, standard_names, G_N_ELEMENTS (standard_names),
		    sizeof (PageInfo), page_info_compare);

    if (info != NULL) {
      parse_media_size (info->size, &size->width, &size->height);
      size->name = g_strdup (info->name);
      size->display_name = g_strdup (info->display_name);
    } else {
      g_warning ("Unknown paper size %s\n", name);
      size->name = g_strdup (name);
      size->display_name = g_strdup (name);
      /* Default to A4 size */
      size->width = 210;
      size->height = 297;
    }
  }

  return size;
  
}

EggPaperSize *
egg_paper_size_new_custom (const char *name,
			   double width, double height, EggUnit unit)
{
  EggPaperSize *size;
  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (unit != EGG_UNIT_PIXEL, NULL);

  size = g_new0 (EggPaperSize, 1);
  size->name = g_strdup (name);
  
  /* Width is always the shorter one */
  if (width > height) {
    double t = width;
    width = height;
    height = t;
  }

  size->width = to_mm (width, unit);
  size->height = to_mm (height, unit);
  
  return size;
}

EggPaperSize *
egg_paper_size_copy (EggPaperSize *other)
{
  EggPaperSize *size;

  size = g_new0 (EggPaperSize, 1);

  size->name = g_strdup (other->name);
  size->display_name = g_strdup (other->display_name);
  size->width = other->width;
  size->height = other->height;
  size->is_custom = other->is_custom;

  return size;
}

void
egg_paper_size_free (EggPaperSize *size)
{
  g_free (size->name);
  g_free (size->display_name);
  g_free (size);
}
 
G_CONST_RETURN char *
egg_paper_size_get_name (EggPaperSize *size)
{
  return size->name;
}

G_CONST_RETURN char *
egg_paper_size_get_display_name (EggPaperSize *size)
{
  return size->display_name;
}

double
egg_paper_size_get_width (EggPaperSize *size, EggUnit unit)
{
  return from_mm (size->width, unit);
}
double
egg_paper_size_get_height (EggPaperSize *size, EggUnit unit)
{
  return from_mm (size->height, unit);
}

gboolean
egg_paper_size_is_custom (EggPaperSize *size)
{
  return size->is_custom;
}

/* Only for custom sizes: */
void
egg_paper_size_set_size (EggPaperSize *size, double width, double height, EggUnit unit)
{
  g_return_if_fail (size != NULL);
  g_return_if_fail (size->is_custom);

  /* Width is always the shorter one */
  if (width > height) {
    double t = width;
    width = height;
    height = t;
  }

  size->width = to_mm (width, unit);
  size->height = to_mm (height, unit);
}
