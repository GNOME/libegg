/* eggdruidpagestandard.h
 * Copyright (C) 1999  Red Hat, Inc.
 * Copyright (C) 2002  Anders Carlsson <andersca@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __EGG_DRUID_PAGE_STANDARD_H__
#define __EGG_DRUID_PAGE_STANDARD_H__

#include "eggdruidpage.h"

G_BEGIN_DECLS

#define EGG_TYPE_DRUID_PAGE_STANDARD             (egg_druid_page_standard_get_type ())
#define EGG_DRUID_PAGE_STANDARD(obj)	         (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_DRUID_PAGE_STANDARD, EggDruidPageStandard))
#define EGG_DRUID_PAGE_STANDARD_CLASS(klass)	 (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_DRUID_PAGE_STANDARD, EggDruidPageStandardClass))
#define EGG_IS_DRUID_PAGE_STANDARD(obj)	         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_DRUID_PAGE_STANDARD))
#define EGG_IS_DRUID_PAGE_STANDARD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_DRUID_PAGE_STANDARD))
#define EGG_DRUID_PAGE_STANDARD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_DRUID_PAGE_STANDARD, EggDruidPageStandardClass))

typedef struct _EggDruidPageStandard      EggDruidPageStandard;
typedef struct _EggDruidPageStandardClass EggDruidPageStandardClass;

struct _EggDruidPageStandard
{
  EggDruidPage parent;

  gchar *title;
};

struct _EggDruidPageStandardClass
{
  EggDruidPageClass parent_class;
};

GType egg_druid_page_standard_get_type (void);

EggDruidPage *egg_druid_page_standard_new (void);
void egg_druid_page_standard_set_title (EggDruidPageStandard *page, const gchar *title);

#endif /* __EGG_DRUID_PAGE_STANDARD_H__ */
