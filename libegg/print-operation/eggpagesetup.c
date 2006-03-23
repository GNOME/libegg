/* GTK - The GIMP Toolkit
 * eggpagesetup.c: Page Setup
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

#include "eggpagesetup.h"

#define MM_PER_INCH 25.4
#define POINTS_PER_INCH 72

typedef struct _EggPageSetupClass EggPageSetupClass;

#define EGG_IS_PAGE_SETUP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PAGE_SETUP))
#define EGG_PAGE_SETUP_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PAGE_SETUP, EggPageSetupClass))
#define EGG_PAGE_SETUP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PAGE_SETUP, EggPageSetupClass))

struct _EggPageSetup
{
  GObject parent_instance;

  EggPageOrientation orientation;
  EggPaperSize *paper_size;
  /* These are stored in mm */
  double top_margin, bottom_margin, left_margin, right_margin;
};

struct _EggPageSetupClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (EggPageSetup, egg_page_setup, G_TYPE_OBJECT)

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
egg_page_setup_finalize (GObject *object)
{
  EggPageSetup *setup = EGG_PAGE_SETUP (object);
  
  egg_paper_size_free (setup->paper_size);
  
  G_OBJECT_CLASS (egg_page_setup_parent_class)->finalize (object);
}

static void
egg_page_setup_init (EggPageSetup *setup)
{
  setup->paper_size = egg_paper_size_new (NULL);
  setup->orientation = EGG_PAGE_ORIENTATION_PORTRAIT;
  setup->top_margin = egg_paper_size_get_default_top_margin (setup->paper_size, EGG_UNIT_MM);
  setup->bottom_margin = egg_paper_size_get_default_bottom_margin (setup->paper_size, EGG_UNIT_MM);
  setup->left_margin = egg_paper_size_get_default_left_margin (setup->paper_size, EGG_UNIT_MM);
  setup->right_margin = egg_paper_size_get_default_right_margin (setup->paper_size, EGG_UNIT_MM);
}

static void
egg_page_setup_class_init (EggPageSetupClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_page_setup_finalize;
}
  
EggPageSetup *
egg_page_setup_new (void)
{
  return g_object_new (EGG_TYPE_PAGE_SETUP, NULL);
}

EggPageSetup *
egg_page_setup_copy (EggPageSetup *other)
{
  EggPageSetup *copy;

  copy = egg_page_setup_new ();
  copy->orientation = other->orientation;
  copy->paper_size = egg_paper_size_copy (other->paper_size);
  copy->top_margin = other->top_margin;
  copy->bottom_margin = other->bottom_margin;
  copy->left_margin = other->left_margin;
  copy->right_margin = other->right_margin;

  return copy;
}

EggPageOrientation
egg_page_setup_get_orientation (EggPageSetup *setup)
{
  return setup->orientation;
}

void
egg_page_setup_set_orientation (EggPageSetup *setup,
				EggPageOrientation orientation)
{
  setup->orientation = orientation;
}

EggPaperSize *
egg_page_setup_get_paper_size (EggPageSetup *setup)
{
  return setup->paper_size;
}

void
egg_page_setup_set_paper_size (EggPageSetup *setup,
			       EggPaperSize *size)
{
  setup->paper_size = egg_paper_size_copy (size);
}

void
egg_page_setup_set_paper_size_and_default_margins (EggPageSetup *setup,
						   EggPaperSize *size)
{
  setup->paper_size = egg_paper_size_copy (size);
  setup->top_margin = egg_paper_size_get_default_top_margin (setup->paper_size, EGG_UNIT_MM);
  setup->bottom_margin = egg_paper_size_get_default_bottom_margin (setup->paper_size, EGG_UNIT_MM);
  setup->left_margin = egg_paper_size_get_default_left_margin (setup->paper_size, EGG_UNIT_MM);
  setup->right_margin = egg_paper_size_get_default_right_margin (setup->paper_size, EGG_UNIT_MM);
}

double
egg_page_setup_get_top_margin (EggPageSetup *setup,
			       EggUnit       unit)
{
  return from_mm (setup->top_margin, unit);
}

void
egg_page_setup_set_top_margin (EggPageSetup *setup,
			       double        margin,
			       EggUnit       unit)
{
  setup->top_margin = to_mm (margin, unit);
}

double
egg_page_setup_get_bottom_margin (EggPageSetup *setup,
				  EggUnit       unit)
{
  return from_mm (setup->bottom_margin, unit);
}

void
egg_page_setup_set_bottom_margin (EggPageSetup *setup,
				  double        margin,
				  EggUnit       unit)
{
  setup->bottom_margin = to_mm (margin, unit);
}

double
egg_page_setup_get_left_margin (EggPageSetup    *setup,
				EggUnit          unit)
{
  return from_mm (setup->left_margin, unit);
}

void
egg_page_setup_set_left_margin (EggPageSetup    *setup,
				double           margin,
				EggUnit          unit)
{
  setup->left_margin = to_mm (margin, unit);
}

double
egg_page_setup_get_right_margin (EggPageSetup    *setup,
				 EggUnit          unit)
{
  return from_mm (setup->right_margin, unit);
}

void
egg_page_setup_set_right_margin (EggPageSetup    *setup,
				 double           margin,
				 EggUnit          unit)
{
  setup->right_margin = to_mm (margin, unit);
}

/* These take orientation, but not margins into consideration */
double
egg_page_setup_get_paper_width (EggPageSetup *setup,
				EggUnit       unit)
{
  if (setup->orientation == EGG_PAGE_ORIENTATION_PORTRAIT ||
      setup->orientation == EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT)
    return egg_paper_size_get_width (setup->paper_size, unit);
  else
    return egg_paper_size_get_height (setup->paper_size, unit);
}

double
egg_page_setup_get_paper_height (EggPageSetup  *setup,
				 EggUnit        unit)
{
  if (setup->orientation == EGG_PAGE_ORIENTATION_PORTRAIT ||
      setup->orientation == EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT)
    return egg_paper_size_get_height (setup->paper_size, unit);
  else
    return egg_paper_size_get_width (setup->paper_size, unit);
}

/* These take orientation, and margins into consideration */
double
egg_page_setup_get_page_width (EggPageSetup    *setup,
			       EggUnit          unit)
{
  double width;
  
  width = egg_page_setup_get_paper_width (setup, EGG_UNIT_MM);
  width -= setup->left_margin + setup->right_margin;
  
  return from_mm (width, unit);
}

double
egg_page_setup_get_page_height (EggPageSetup    *setup,
				EggUnit          unit)
{
  double height;
  
  height = egg_page_setup_get_paper_height (setup, EGG_UNIT_MM);
  height -= setup->top_margin + setup->bottom_margin;
  
  return from_mm (height, unit);
}
