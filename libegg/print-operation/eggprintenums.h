/* EGG - The GIMP Toolkit
 * eggprintcontextprivate.h: Print Context
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

#ifndef __EGG_PRINT_ENUMS_H__
#define __EGG_PRINT_ENUMS_H__

typedef enum {
  EGG_PRINT_PAGES_ALL,
  EGG_PRINT_PAGES_CURRENT,
  EGG_PRINT_PAGES_RANGES
} EggPrintPages;

typedef enum {
  EGG_PAGE_ORIENTATION_PORTRAIT,
  EGG_PAGE_ORIENTATION_LANDSCAPE,
  EGG_PAGE_ORIENTATION_REVERSE_PORTRAIT,
  EGG_PAGE_ORIENTATION_REVERSE_LANDSCAPE
} EggPageOrientation;

typedef enum {
  EGG_PRINT_QUALITY_LOW,
  EGG_PRINT_QUALITY_NORMAL,
  EGG_PRINT_QUALITY_HIGH,
  EGG_PRINT_QUALITY_DRAFT
} EggPrintQuality;

typedef enum {
  EGG_PRINT_DUPLEX_SIMPLEX,
  EGG_PRINT_DUPLEX_HORIZONTAL,
  EGG_PRINT_DUPLEX_VERTICAL,
} EggPrintDuplex;


typedef enum {
  EGG_UNIT_PIXEL,
  EGG_UNIT_POINTS,
  EGG_UNIT_INCH,
  EGG_UNIT_MM
} EggUnit;


#endif /* __EGG_PRINT_ENUMS_H__ */
