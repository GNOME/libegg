/* eggdruidpage.c
 * Copyright (C) 2002  Red Hat, Inc.
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

#include "eggdruidpagestandard.h"

#define _(x) (x)

enum {
  PROP_0,
  PROP_TITLE,
};

static void egg_druid_page_standard_class_init (EggDruidPageStandardClass *klass);
static void egg_druid_page_standard_init       (EggDruidPageStandard      *druid);

static void egg_druid_page_standard_set_property (GObject      *object,
						  guint         prop_id,
						  const GValue *value,
						  GParamSpec   *pspec);
static void egg_druid_page_standard_get_property (GObject      *object,
						  guint         prop_id,
						  GValue       *value,
						  GParamSpec   *pspec);



GType
egg_druid_page_standard_get_type (void)
{
  static GType our_type = 0;

  if (!our_type)
    {
      static const GTypeInfo our_type_info =
      {
	sizeof (EggDruidPageStandardClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) egg_druid_page_standard_class_init,
	(GClassFinalizeFunc) NULL,
	NULL,
	sizeof (EggDruidPageStandard),
	0, /* n_preallocs */
	(GInstanceInitFunc) egg_druid_page_standard_init
      };

      our_type = g_type_register_static (EGG_TYPE_DRUID_PAGE, "EggDruidPageStandard",
					&our_type_info, 0);
    }

  return our_type;
}

static void
egg_druid_page_standard_class_init (EggDruidPageStandardClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *)klass;

  gobject_class->get_property = egg_druid_page_standard_get_property;
  gobject_class->set_property = egg_druid_page_standard_set_property;
  
  g_object_class_install_property (gobject_class,
				   PROP_TITLE,
				   g_param_spec_string ("title",
							_("Title"),
							_("Title of the page"),
							NULL,
							G_PARAM_READWRITE));
  
}

static void
egg_druid_page_standard_set_property (GObject      *object,
				      guint         prop_id,
				      const GValue *value,
				      GParamSpec   *pspec)
{
  EggDruidPageStandard *page;

  page = EGG_DRUID_PAGE_STANDARD (object);
  
  switch (prop_id)
    {
    case PROP_TITLE:
      egg_druid_page_standard_set_title (page, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_druid_page_standard_get_property (GObject      *object,
				      guint         prop_id,
				      GValue       *value,
				      GParamSpec   *pspec)
{
  EggDruidPageStandard *page;
  
  page = EGG_DRUID_PAGE_STANDARD (object);
  
  switch (prop_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, page->title);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_druid_page_standard_init (EggDruidPageStandard *page)
{
}

EggDruidPage *
egg_druid_page_standard_new (void)
{
  return g_object_new (EGG_TYPE_DRUID_PAGE_STANDARD, NULL);
}

void
egg_druid_page_standard_set_title (EggDruidPageStandard *page, const gchar *title)
{
  g_return_if_fail (EGG_IS_DRUID_PAGE_STANDARD (page));
  
  g_free (page->title);
  page->title = g_strdup (title);

  g_object_notify (G_OBJECT (page), "title");
}

