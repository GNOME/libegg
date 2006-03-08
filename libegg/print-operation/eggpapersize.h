/* EGG - The GIMP Toolkit
 * eggpapersize.h: Paper Size
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

#ifndef __EGG_PAPER_SIZE_H__
#define __EGG_PAPER_SIZE_H__

#include <glib-object.h>
#include <eggprintenums.h>

G_BEGIN_DECLS

typedef struct _EggPaperSize EggPaperSize;

#define EGG_TYPE_PAPER_SIZE    (egg_paper_size_get_type ())

/* Common names, from PWG 5101.1-2002 PWG: Standard for Media Standardized Names */
#define EGG_PAPER_NAME_A0 "iso_a0"
#define EGG_PAPER_NAME_A1 "iso_a1"
#define EGG_PAPER_NAME_A2 "iso_a2"
#define EGG_PAPER_NAME_A3 "iso_a3"
#define EGG_PAPER_NAME_A4 "iso_a4"
#define EGG_PAPER_NAME_A5 "iso_a5"
#define EGG_PAPER_NAME_A6 "iso_a6"
#define EGG_PAPER_NAME_A7 "iso_a7"
#define EGG_PAPER_NAME_A8 "iso_a8"
#define EGG_PAPER_NAME_A9 "iso_a9"
#define EGG_PAPER_NAME_B0 "iso_b0"
#define EGG_PAPER_NAME_B1 "iso_b1"
#define EGG_PAPER_NAME_B2 "iso_b2"
#define EGG_PAPER_NAME_B3 "iso_b3"
#define EGG_PAPER_NAME_B4 "iso_b4"
#define EGG_PAPER_NAME_B5 "iso_b5"
#define EGG_PAPER_NAME_B6 "iso_b6"
#define EGG_PAPER_NAME_B7 "iso_b7"
#define EGG_PAPER_NAME_B8 "iso_b8"
#define EGG_PAPER_NAME_B9 "iso_b9"
#define EGG_PAPER_NAME_LETTER "na_letter"
#define EGG_PAPER_NAME_EXECUTIVE "na_executive"
#define EGG_PAPER_NAME_LEGAL "na_legal"

GType egg_paper_size_get_type (void);

EggPaperSize *egg_paper_size_new         (const char *name);
EggPaperSize *egg_paper_size_new_custom  (const char *name,
					  double width, double height, EggUnit unit);
EggPaperSize *egg_paper_size_copy        (EggPaperSize *other);
void          egg_paper_size_free        (EggPaperSize *size);


/* The width is always the shortest side, measure in mm */
G_CONST_RETURN char *   egg_paper_size_get_name         (EggPaperSize *size);
G_CONST_RETURN char *   egg_paper_size_get_display_name (EggPaperSize *size);
double   egg_paper_size_get_width        (EggPaperSize *size, EggUnit unit);
double   egg_paper_size_get_height       (EggPaperSize *size, EggUnit unit);
gboolean egg_paper_size_is_custom        (EggPaperSize *size);

/* Only for custom sizes: */
void egg_paper_size_set_size (EggPaperSize *size, double width, double height, EggUnit unit);

G_END_DECLS

#endif /* __EGG_PAPER_SIZE_H__ */
