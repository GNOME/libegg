/* GTK - The GIMP Toolkit
 * eggprintcontext.c: Print Context
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

#include "eggprintcontext-private.h"
#include "eggprintoperation-private.h"

typedef struct _EggPrintContextClass EggPrintContextClass;

#define EGG_IS_PRINT_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PRINT_CONTEXT))
#define EGG_PRINT_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PRINT_CONTEXT, EggPrintContextClass))
#define EGG_PRINT_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PRINT_CONTEXT, EggPrintContextClass))

#define MM_PER_INCH 25.4
#define POINTS_PER_INCH 72

struct _EggPrintContext
{
  GObject parent_instance;

  EggPrintOperation *op;
  cairo_t *cr;
  EggPageSetup *page_setup;
  PangoFontMap *fontmap;

  double pixels_per_unit_x, pixels_per_unit_y;
  
};

struct _EggPrintContextClass
{
  GObjectClass parent_class;
};

G_DEFINE_TYPE (EggPrintContext, egg_print_context, G_TYPE_OBJECT)

static void
egg_print_context_finalize (GObject *object)
{
  EggPrintContext *context = EGG_PRINT_CONTEXT (object);

  g_object_unref (context->fontmap);
  if (context->page_setup)
    g_object_unref (context->page_setup);

  cairo_destroy (context->cr);

  
  G_OBJECT_CLASS (egg_print_context_parent_class)->finalize (object);
}

static void
egg_print_context_init (EggPrintContext *context)
{
}

static void
egg_print_context_class_init (EggPrintContextClass *class)
{
  GObjectClass *gobject_class = (GObjectClass *)class;

  gobject_class->finalize = egg_print_context_finalize;
}


EggPrintContext *
_egg_print_context_new (EggPrintOperation *op)
{
  EggPrintContext *context;
  
  context = g_object_new (EGG_TYPE_PRINT_CONTEXT, NULL);

  context->op = op;
  context->cr = cairo_create (op->priv->surface);

  switch (op->priv->unit) {
  default:
  case EGG_UNIT_PIXEL:
    /* Do nothing, this is the cairo default unit */
    context->pixels_per_unit_x = 1.0;
    context->pixels_per_unit_y = 1.0;
    break;
  case EGG_UNIT_POINTS:
    context->pixels_per_unit_x = op->priv->dpi_x / POINTS_PER_INCH;
    context->pixels_per_unit_y = op->priv->dpi_y / POINTS_PER_INCH;
    break;
  case EGG_UNIT_INCH:
    context->pixels_per_unit_x = op->priv->dpi_x;
    context->pixels_per_unit_y = op->priv->dpi_y;
    break;
  case EGG_UNIT_MM:
    context->pixels_per_unit_x = op->priv->dpi_x / MM_PER_INCH;
    context->pixels_per_unit_y = op->priv->dpi_y / MM_PER_INCH;
    break;
  }
  cairo_scale (context->cr,
	       context->pixels_per_unit_x,
	       context->pixels_per_unit_y);
    
  context->fontmap = pango_cairo_font_map_new ();
  /* We use the unit-scaled resolution, as we still want fonts given in points to work */
  pango_cairo_font_map_set_resolution (PANGO_CAIRO_FONT_MAP (context->fontmap),
				       op->priv->dpi_y / context->pixels_per_unit_y);
  

  /* TODO: Should we rotate the space if we're on landscape? */
  
  return context;
}

void
_egg_print_context_translate_into_margin (EggPrintContext *context)
{
  double left, top;

  g_return_if_fail (EGG_IS_PRINT_CONTEXT (context));

  /* We do it this way to also handle EGG_UNIT_PIXELS */
  
  left = egg_page_setup_get_left_margin (context->page_setup, EGG_UNIT_INCH);
  top = egg_page_setup_get_top_margin (context->page_setup, EGG_UNIT_INCH);

  cairo_translate (context->cr,
		   left * context->op->priv->dpi_x / context->pixels_per_unit_x,
		   top * context->op->priv->dpi_y / context->pixels_per_unit_y);
}

void
_egg_print_context_set_page_setup (EggPrintContext *context,
				   EggPageSetup *page_setup)
{
  g_return_if_fail (EGG_IS_PRINT_CONTEXT (context));
  g_return_if_fail (page_setup == NULL ||
		    EGG_IS_PAGE_SETUP (page_setup));
  
  g_object_ref (page_setup);

  if (context->page_setup != NULL)
    g_object_unref (context->page_setup);

  context->page_setup = page_setup;
}

cairo_t *
egg_print_context_get_cairo (EggPrintContext *context)
{
  return context->cr;
}

EggPageSetup *
egg_print_context_get_page_setup (EggPrintContext *context)
{
  return context->page_setup;
}

double
egg_print_context_get_width (EggPrintContext *context)
{
  double width;
  if (context->op->priv->use_full_page)
    width = egg_page_setup_get_paper_width (context->page_setup, EGG_UNIT_INCH);
  else
    width = egg_page_setup_get_page_width (context->page_setup, EGG_UNIT_INCH);

  /* Really dpi_x? What about landscape? what does dpi_x mean in that case? */
  return width * context->op->priv->dpi_x / context->pixels_per_unit_x;
}

double
egg_print_context_get_height (EggPrintContext *context)
{
  double height;
  if (context->op->priv->use_full_page)
    height = egg_page_setup_get_paper_height (context->page_setup, EGG_UNIT_INCH);
  else
    height = egg_page_setup_get_page_height (context->page_setup, EGG_UNIT_INCH);

  /* Really dpi_x? What about landscape? what does dpi_x mean in that case? */
  return height * context->op->priv->dpi_y / context->pixels_per_unit_y;
}

double
egg_print_context_get_dpi_x (EggPrintContext *context)
{
  return context->op->priv->dpi_x;
}

double
egg_print_context_get_dpi_y (EggPrintContext *context)
{
  return context->op->priv->dpi_y;
}


/* Fonts */
PangoFontMap *
egg_print_context_get_fontmap (EggPrintContext *context)
{
  return context->fontmap;
}

PangoContext *
egg_print_context_create_context (EggPrintContext *context)
{
  PangoContext *pango_context;
  
  pango_context = pango_cairo_font_map_create_context (PANGO_CAIRO_FONT_MAP (context->fontmap));
  
  return pango_context;
}

PangoLayout *
egg_print_context_create_layout (EggPrintContext *context)
{
  PangoContext *pango_context;
  PangoLayout *layout;

  g_return_val_if_fail (context != NULL, NULL);

  pango_context = egg_print_context_create_context (context);
  layout = pango_layout_new (pango_context);

  pango_cairo_update_context (context->cr, pango_context);
  g_object_unref (pango_context);

  return layout;
}

