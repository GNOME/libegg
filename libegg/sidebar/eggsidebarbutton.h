/*
 *  Copyright (C) 2002 Jorn Baayen <jorn@nl.linux.org>
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
 *
 *  $Id$
 */

#ifndef __EGG_SIDEBAR_BUTTON_H__
#define __EGG_SIDEBAR_BUTTON_H__

#include <gtk/gtkradiobutton.h>

G_BEGIN_DECLS

#define EGG_TYPE_SIDEBAR_BUTTON         (egg_sidebar_button_get_type ())
#define EGG_SIDEBAR_BUTTON(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), EGG_TYPE_SIDEBAR_BUTTON, EggSidebarButton))
#define RB_SIDEBAR_BUTTON_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), EGG_TYPE_SIDEBAR_BUTTON, EggSidebarButtonClass))
#define EGG_IS_SIDEBAR_BUTTON(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EGG_TYPE_SIDEBAR_BUTTON))
#define EGG_IS_SIDEBAR_BUTTON_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), EGG_TYPE_SIDEBAR_BUTTON))
#define EGG_SIDEBAR_BUTTON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EGG_TYPE_SIDEBAR_BUTTON, EggSidebarButtonClass))

typedef struct EggSidebarButtonPrivate EggSidebarButtonPrivate;

typedef struct
{
	GtkRadioButton parent;

	char *unique_id;

	GtkWidget *label;
	GtkWidget *image;

	EggSidebarButtonPrivate *priv;
} EggSidebarButton;

typedef struct
{
	GtkRadioButtonClass parent;

	void (*edited)  (EggSidebarButton *button);
	void (*deleted) (EggSidebarButton *button);
} EggSidebarButtonClass;


GType             egg_sidebar_button_get_type (void);

EggSidebarButton *egg_sidebar_button_new      (const char        *unique_id);
void              egg_sidebar_button_set      (EggSidebarButton  *button,
					       const char        *stock_id,
					       const char        *text,
					       gboolean           is_static);
void              egg_sidebar_button_get      (EggSidebarButton  *button,
					       char             **stock_id,
					       char             **text,
					       gboolean          *is_static);


G_END_DECLS

#endif /* __EGG_SIDEBAR_BUTTON_H__ */
