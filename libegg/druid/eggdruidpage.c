/* eggdruidpage.c
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

#include "eggdruidpage.h"

#define _(x) (x)

enum {
  PROP_0,
  PROP_TITLE,
};

static void egg_druid_page_class_init   (EggDruidPageClass *klass);
static void egg_druid_page_init         (EggDruidPage      *druid);
static void egg_druid_page_get_property (GObject           *object,
					 guint              prop_id,
					 GValue            *value,
					 GParamSpec        *pspec);



static void egg_druid_page_size_request  (GtkWidget      *widget,
					  GtkRequisition *requisition);
static void egg_druid_page_size_allocate (GtkWidget      *widget,
					  GtkAllocation  *allocation);


GType
egg_druid_page_get_type (void)
{
  static GType our_type = 0;

  if (!our_type)
    {
      static const GTypeInfo our_type_info =
      {
	sizeof (EggDruidPageClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) egg_druid_page_class_init,
	(GClassFinalizeFunc) NULL,
	NULL,
	sizeof (EggDruidPage),
	0, /* n_preallocs */
	(GInstanceInitFunc) egg_druid_page_init
      };

      our_type = g_type_register_static (GTK_TYPE_BIN, "EggDruidPage",
					&our_type_info, 0);
    }

  return our_type;
}

static void
egg_druid_page_class_init (EggDruidPageClass *klass)
{
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class = (GObjectClass *)klass;
  widget_class = (GtkWidgetClass *)klass;

  gobject_class->get_property = egg_druid_page_get_property;
  
  widget_class->size_request = egg_druid_page_size_request;
  widget_class->size_allocate = egg_druid_page_size_allocate;

  g_object_class_install_property (gobject_class,
				   PROP_TITLE,
				   g_param_spec_string ("title",
							_("Title"),
							_("Title of the page"),
							NULL,
							G_PARAM_READABLE));
							
}

static void
egg_druid_page_init (EggDruidPage *druid)
{
}

static void
egg_druid_page_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
  switch (prop_id)
    {
    case PROP_TITLE:
      g_warning ("The EggDruidPage::%s property must be installed by `%s'",
		 pspec->name, G_OBJECT_TYPE_NAME (object));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
egg_druid_page_size_request (GtkWidget      *widget,
			     GtkRequisition *requisition)
{
  GtkBin *bin;

  bin = GTK_BIN (widget);

  requisition->width = GTK_CONTAINER (widget)->border_width * 2;
  requisition->height = GTK_CONTAINER (widget)->border_width * 2;

  if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
      GtkRequisition child_requisition;
      
      gtk_widget_size_request (bin->child, &child_requisition);
      
      requisition->width += child_requisition.width;
      requisition->height += child_requisition.height;
  }
}

static void
egg_druid_page_size_allocate (GtkWidget      *widget,
			      GtkAllocation  *allocation)
{
  GtkBin *bin;
  GtkAllocation child_allocation;
  
  bin = GTK_BIN (widget);
  widget->allocation = *allocation;

  if (bin->child) 
    {
      child_allocation.x = allocation->x + GTK_CONTAINER (widget)->border_width;
      child_allocation.y = allocation->y + GTK_CONTAINER (widget)->border_width;
      child_allocation.width = MAX (allocation->width - GTK_CONTAINER (widget)->border_width * 2, 0);
      child_allocation.height = MAX (allocation->height - GTK_CONTAINER (widget)->border_width * 2, 0);
      gtk_widget_size_allocate (bin->child, &child_allocation);
    }
}

