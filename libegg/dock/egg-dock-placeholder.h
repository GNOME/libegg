/*
 * egg-dock-placeholder.h - Placeholders for docking items
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2003 Biswapesh Chattopadhyay <biswapesh_chatterjee@tcscal.co.in>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 */

#ifndef __EGG_DOCK_PLACEHOLDER_H__
#define __EGG_DOCK_PLACEHOLDER_H__

#include <dock/egg-dock-object.h>

G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK_PLACEHOLDER             (egg_dock_placeholder_get_type ())
#define EGG_DOCK_PLACEHOLDER(obj)             (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_PLACEHOLDER, EggDockPlaceholder))
#define EGG_DOCK_PLACEHOLDER_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_PLACEHOLDER, EggDockPlaceholderClass))
#define EGG_IS_DOCK_PLACEHOLDER(obj)          (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_PLACEHOLDER))
#define EGG_IS_DOCK_PLACEHOLDER_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_PLACEHOLDER))
#define EGG_DOCK_PLACEHOLDER_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_PLACEHOLDER, EggDockPlaceholderClass))

/* data types & structures */
typedef struct _EggDockPlaceholder        EggDockPlaceholder;
typedef struct _EggDockPlaceholderClass   EggDockPlaceholderClass;
typedef struct _EggDockPlaceholderPrivate EggDockPlaceholderPrivate;

struct _EggDockPlaceholder {
    EggDockObject              object;

    EggDockPlaceholderPrivate *_priv;
};

struct _EggDockPlaceholderClass {
    EggDockObjectClass parent_class;
	gpointer unused[2];
};

/* public interface */

GType       egg_dock_placeholder_get_type (void);

GtkWidget  *egg_dock_placeholder_new      (gchar              *name,
                                           EggDockObject      *object,
                                           EggDockPlacement    position,
                                           gboolean            sticky);

void        egg_dock_placeholder_attach   (EggDockPlaceholder *ph,
                                           EggDockObject      *object);


G_END_DECLS

#endif /* __EGG_DOCK_PLACEHOLDER_H__ */
