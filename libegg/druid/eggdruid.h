/* eggdruid.h
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

#ifndef __EGG_DRUID_H__
#define __EGG_DRUID_H__

#include <gtk/gtkcontainer.h>
#include "eggdruidpage.h"

G_BEGIN_DECLS

#define EGG_TYPE_DRUID             (egg_druid_get_type ())
#define EGG_DRUID(obj)	           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_DRUID, EggDruid))
#define EGG_DRUID_CLASS(klass)	   (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_DRUID, EggDruidClass))
#define EGG_IS_DRUID(obj)	   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_DRUID))
#define EGG_IS_DRUID_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), EGG_TYPE_DRUID))
#define EGG_DRUID_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_DRUID, EggDruidClass))

typedef struct _EggDruid      EggDruid;
typedef struct _EggDruidClass EggDruidClass;

struct _EggDruid
{
  GtkContainer parent;

  GtkWidget *header;
  GtkWidget *header_box;
  GtkWidget *header_image;
  GtkWidget *header_title;
  GtkWidget *header_description;
  
  GtkWidget *sidebar;
  GtkWidget *sidebar_image;
  
  GList *children;
  EggDruidPage *current;
};

struct _EggDruidClass
{
  GtkContainerClass parent_class;
};

GType      egg_druid_get_type          (void);
GtkWidget *egg_druid_new               (void);

void egg_druid_set_sidebar_image           (EggDruid  *druid,
					    GdkPixbuf *image);
void egg_druid_set_sidebar_image_alignment (EggDruid  *druid,
					    gfloat     alignment);
void egg_druid_set_sidebar_color           (EggDruid  *druid,
					    GdkColor  *color);
void egg_druid_set_header_image            (EggDruid  *druid,
					    GdkPixbuf *image);
void egg_druid_set_header_color            (EggDruid  *druid,
					    GdkColor  *color);
void egg_druid_set_header_text_color       (EggDruid  *druid,
					    GdkColor  *color);

void egg_druid_insert_page_after (EggDruid     *druid,
				  EggDruidPage *sibling,
				  EggDruidPage *page);
void egg_druid_append_page       (EggDruid     *druid,
				  EggDruidPage *page);
void egg_druid_set_current_page  (EggDruid     *druid,
				  EggDruidPage *page);



     
G_END_DECLS

#endif /* __EGG_DRUID_H__ */
