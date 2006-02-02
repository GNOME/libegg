/* eggdruid.c
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

#include "eggdruid.h"
#include <gtk/gtkalignment.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>

#define _(x) (x)

static void egg_druid_class_init (EggDruidClass *klass);
static void egg_druid_init       (EggDruid      *druid);

static void egg_druid_size_request  (GtkWidget      *widget,
				     GtkRequisition *requisition);
static void egg_druid_size_allocate (GtkWidget      *widget,
				     GtkAllocation  *allocation);
static void egg_druid_map           (GtkWidget      *widget);

static void egg_druid_forall (GtkContainer *container,
			      gboolean      include_internals,
			      GtkCallback   callback,
			      gpointer      callback_data);

GType
egg_druid_get_type (void)
{
  static GType our_type = 0;

  if (!our_type)
    {
      static const GTypeInfo our_type_info =
      {
	sizeof (EggDruidClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) egg_druid_class_init,
	(GClassFinalizeFunc) NULL,
	NULL,
	sizeof (EggDruid),
	0, /* n_preallocs */
	(GInstanceInitFunc) egg_druid_init
      };

      our_type = g_type_register_static (GTK_TYPE_CONTAINER, "EggDruid",
					&our_type_info, 0);
    }

  return our_type;
}

static void
egg_druid_class_init (EggDruidClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  
  widget_class = (GtkWidgetClass *)klass;
  container_class = (GtkContainerClass *)klass;
  
  widget_class->size_request = egg_druid_size_request;
  widget_class->size_allocate = egg_druid_size_allocate;
  widget_class->map = egg_druid_map;

  container_class->forall = egg_druid_forall;

#define _DRUID_HEADER_PADDING 4
  gtk_widget_class_install_style_property (widget_class,
					   g_param_spec_int ("header_padding",
							     _("Header Padding"),
							     _("Number of pixels around the header."),
							     0,
							     G_MAXINT,
							     _DRUID_HEADER_PADDING,
							     G_PARAM_READABLE));
}

static void
egg_druid_init (EggDruid *druid)
{
  GtkWidget *vbox, *align;
  
  GTK_WIDGET_SET_FLAGS (druid, GTK_NO_WINDOW);

  druid->sidebar = gtk_event_box_new ();
  gtk_widget_set_parent (druid->sidebar, GTK_WIDGET (druid));
  druid->sidebar_image = gtk_image_new ();
  gtk_container_add (GTK_CONTAINER (druid->sidebar), druid->sidebar_image);
  gtk_widget_show_all (druid->sidebar);

  druid->header = gtk_event_box_new ();
  gtk_widget_set_parent (druid->header, GTK_WIDGET (druid));
  druid->header_box = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (druid->header), druid->header_box);
  druid->header_image = gtk_image_new ();
  gtk_box_pack_end (GTK_BOX (druid->header_box), druid->header_image, FALSE, FALSE, 0);

  vbox = gtk_vbox_new (FALSE, 0);
  align = gtk_alignment_new (0.0, 0.5, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (align), vbox);
  gtk_box_pack_start (GTK_BOX (druid->header_box), align, TRUE, TRUE, 0);

  druid->header_title = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (druid->header_title), 0.0, 0.5);
  gtk_label_set_markup (GTK_LABEL (druid->header_title), "<b>Test</b>");
  gtk_box_pack_start (GTK_BOX (vbox), druid->header_title, FALSE, FALSE, 0);

  druid->header_description = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (druid->header_description), 0.0, 0.5);
  gtk_label_set_markup (GTK_LABEL (druid->header_description), "A nice little description");
  gtk_box_pack_start (GTK_BOX (vbox), druid->header_description, FALSE, FALSE, 0);
  
  gtk_widget_show_all (druid->header);
}

static void
egg_druid_size_request (GtkWidget      *widget,
			GtkRequisition *requisition)
{
  EggDruid *druid;
  GtkRequisition child_requisition;
  gint temp_width, temp_height;
  gint header_padding;
  GList *list;
  
  druid = EGG_DRUID (widget);

  gtk_widget_style_get (widget, "header_padding", &header_padding, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (druid->header_box), header_padding);
  
  temp_width = 0;
  temp_height = 0;

  list = druid->children;
  while (list)
    {
      EggDruidPage *child = list->data;

      if (GTK_WIDGET_VISIBLE (child))
	{
	  gtk_widget_size_request (GTK_WIDGET (child), &child_requisition);
	  temp_width = MAX (temp_width, child_requisition.width);
	  temp_height = MAX (temp_height, child_requisition.height);
	  if (GTK_WIDGET_MAPPED (child) && child != druid->current)
	    gtk_widget_unmap (GTK_WIDGET(child));
	}
      
      list = list->next;
    }
  
  gtk_widget_size_request (druid->sidebar, &child_requisition);
  temp_height = MAX (temp_height, child_requisition.height);
  temp_width += child_requisition.width;

  gtk_widget_size_request (druid->header, &child_requisition);
  temp_height = MAX (temp_height, child_requisition.height);
  temp_width += child_requisition.width;
  
  temp_width += GTK_CONTAINER (druid)->border_width * 2;
  temp_height += GTK_CONTAINER (druid)->border_width * 2;
  
  requisition->width = temp_width;
  requisition->height = temp_height;
}

static void
egg_druid_size_allocate (GtkWidget      *widget,
			 GtkAllocation  *allocation)
{
  EggDruid *druid;
  GtkAllocation child_allocation;
  GList *list;
  
  druid = EGG_DRUID (widget);
  widget->allocation = *allocation;
  
  child_allocation.x = GTK_CONTAINER (widget)->border_width;
  child_allocation.y = GTK_CONTAINER (widget)->border_width;
  child_allocation.width = druid->sidebar->requisition.width;
  child_allocation.height = allocation->height - 2 * GTK_CONTAINER (widget)->border_width;

  gtk_widget_size_allocate (druid->sidebar, &child_allocation);

  child_allocation.x = GTK_CONTAINER (widget)->border_width + druid->sidebar->allocation.width;
  child_allocation.y = GTK_CONTAINER (widget)->border_width;
  child_allocation.height = druid->header->requisition.height;
  child_allocation.width = allocation->width - GTK_CONTAINER (widget)->border_width * 2 -
    druid->sidebar->allocation.width;
  gtk_widget_size_allocate (druid->header, &child_allocation);

  child_allocation.x = GTK_CONTAINER (widget)->border_width + druid->sidebar->allocation.width;
  child_allocation.y = GTK_CONTAINER (widget)->border_width + druid->header->allocation.height;
  child_allocation.width = allocation->width - GTK_CONTAINER (widget)->border_width * 2 -
    druid->sidebar->allocation.width;
  child_allocation.height = allocation->height - GTK_CONTAINER (widget)->border_width * 2 -
    druid->header->allocation.height;
  
  list = druid->children;
  while (list)
    {
      EggDruidPage *page = list->data;

      if (GTK_WIDGET_VISIBLE (page))
	gtk_widget_size_allocate (GTK_WIDGET (page), &child_allocation);
      
      list = list->next;
    }
}

static void
egg_druid_map (GtkWidget *widget)
{
  EggDruid *druid;
  
  druid = EGG_DRUID (widget);

  GTK_WIDGET_SET_FLAGS (druid, GTK_MAPPED);

  gtk_widget_map (druid->sidebar);
  gtk_widget_map (druid->header);

  if (druid->current &&
      GTK_WIDGET_VISIBLE (druid->current) &&
      !GTK_WIDGET_MAPPED (druid->current))
    gtk_widget_map (GTK_WIDGET (druid->current));
}

static void
egg_druid_forall (GtkContainer *container,
		  gboolean      include_internals,
		  GtkCallback   callback,
		  gpointer      callback_data)
{
  EggDruid *druid;
  GList *list;
  
  druid = EGG_DRUID (container);

  if (include_internals)
    {
      (* callback) (druid->sidebar, callback_data);
      (* callback) (druid->header, callback_data);  
    }
  
  list = druid->children;
  while (list)
    {
      EggDruidPage *page = list->data;
      list = list->next;

      (* callback) (GTK_WIDGET (page), callback_data);
    }
}


GtkWidget *
egg_druid_new (void)
{
  EggDruid *druid;

  druid = g_object_new (EGG_TYPE_DRUID, NULL);

  return GTK_WIDGET (druid);
}

void
egg_druid_set_sidebar_image (EggDruid  *druid,
			     GdkPixbuf *image)
{
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (GDK_IS_PIXBUF (image));

  gtk_image_set_from_pixbuf (GTK_IMAGE (druid->sidebar_image), image);
}

void
egg_druid_set_sidebar_image_alignment (EggDruid *druid,
				       gfloat alignment)
{
  g_return_if_fail (EGG_IS_DRUID (druid));
  
  alignment = CLAMP (alignment, 0.0, 1.0);

  gtk_misc_set_alignment (GTK_MISC (druid->sidebar_image), 0.5, alignment);
}

void
egg_druid_set_sidebar_color (EggDruid  *druid,
			     GdkColor  *color)
{
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (color != NULL);

  gtk_widget_modify_bg (druid->sidebar, GTK_STATE_NORMAL, color);
}


void
egg_druid_set_header_image (EggDruid  *druid,
			    GdkPixbuf *image)
{
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (GDK_IS_PIXBUF (image));

  gtk_image_set_from_pixbuf (GTK_IMAGE (druid->header_image), image);
}


void
egg_druid_set_header_color (EggDruid  *druid,
			    GdkColor  *color)
{
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (color != NULL);

  gtk_widget_modify_bg (druid->header, GTK_STATE_NORMAL, color);
}

void
egg_druid_set_header_text_color (EggDruid  *druid,
				 GdkColor  *color)
{
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (color != NULL);

  gtk_widget_modify_fg (druid->header_title, GTK_STATE_NORMAL, color);
  gtk_widget_modify_fg (druid->header_description, GTK_STATE_NORMAL, color);  
}

void
egg_druid_insert_page_after (EggDruid     *druid,
			     EggDruidPage *sibling,
			     EggDruidPage *page)
{
  GList *list;
  
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (EGG_IS_DRUID_PAGE (page));
  
  list = g_list_find (druid->children, sibling);

  if (!list)
    druid->children = g_list_prepend (druid->children, page);
  else
    g_assert_not_reached ();

  gtk_widget_set_parent (GTK_WIDGET (page), GTK_WIDGET (druid));
  
  if (GTK_WIDGET_REALIZED (GTK_WIDGET (druid)))
    gtk_widget_realize (GTK_WIDGET (page));

  if (druid->children->next == NULL)
    egg_druid_set_current_page (druid, page);
  
}
		       

void
egg_druid_append_page (EggDruid     *druid,
		       EggDruidPage *page)
{
  GList *list;
  
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (EGG_IS_DRUID_PAGE (page));

  list = g_list_last (druid->children);
  
  if (list) 
    egg_druid_insert_page_after (druid, EGG_DRUID_PAGE (list->data), page);
  else
    egg_druid_insert_page_after (druid, NULL, page);
}


void
egg_druid_set_current_page (EggDruid     *druid,
			    EggDruidPage *page)
{
  GtkWidget *old_page;
  gchar *page_title, *tmp_str, *tmp_str2;
  
  g_return_if_fail (EGG_IS_DRUID (druid));
  g_return_if_fail (EGG_IS_DRUID_PAGE (page));

  if (druid->current == page)
    return;

  g_return_if_fail (g_list_find (druid->children, page) != NULL);

  if (druid->current &&
      GTK_WIDGET_VISIBLE (druid->current) &&
      GTK_WIDGET_MAPPED (druid->current))
    old_page = GTK_WIDGET (druid->current);
  else
    old_page = NULL;
  
  druid->current = page;

  /* FIXME: Call prepare */

  /* Get the page title from the page */
  g_object_get (G_OBJECT (druid->current),
		"title", &tmp_str,
		NULL);
  if (tmp_str)
    {
      tmp_str2 = g_markup_escape_text (tmp_str, -1);
      g_free (tmp_str);
      page_title = g_strconcat ("<b>", tmp_str2, "</b>", NULL);
      g_free (tmp_str2);

      gtk_label_set_markup (GTK_LABEL (druid->header_title), page_title);      
    }
  else
    gtk_label_set_text (GTK_LABEL (druid->header_title), "");      
  
  if (GTK_WIDGET_VISIBLE (druid->current) &&
      GTK_WIDGET_MAPPED (druid))
    gtk_widget_map (GTK_WIDGET (druid->current));

  if (old_page && GTK_WIDGET_MAPPED (old_page))
    gtk_widget_unmap (old_page);
  
}


