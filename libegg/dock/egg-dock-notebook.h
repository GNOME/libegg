/*
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

#ifndef __EGG_DOCK_NOTEBOOK_H__
#define __EGG_DOCK_NOTEBOOK_H__

#include <dock/egg-dock-item.h>

G_BEGIN_DECLS

/* standard macros */
#define EGG_TYPE_DOCK_NOTEBOOK            (egg_dock_notebook_get_type ())
#define EGG_DOCK_NOTEBOOK(obj)            (GTK_CHECK_CAST ((obj), EGG_TYPE_DOCK_NOTEBOOK, EggDockNotebook))
#define EGG_DOCK_NOTEBOOK_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EGG_TYPE_DOCK_NOTEBOOK, EggDockNotebookClass))
#define EGG_IS_DOCK_NOTEBOOK(obj)         (GTK_CHECK_TYPE ((obj), EGG_TYPE_DOCK_NOTEBOOK))
#define EGG_IS_DOCK_NOTEBOOK_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EGG_TYPE_DOCK_NOTEBOOK))
#define EGG_DOCK_NOTEBOOK_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_DOCK_NOTEBOOK, EggDockNotebookClass))

/* data types & structures */
typedef struct _EggDockNotebook        EggDockNotebook;
typedef struct _EggDockNotebookClass   EggDockNotebookClass;
typedef struct _EggDockNotebookPrivate EggDockNotebookPrivate;

struct _EggDockNotebook {
    EggDockItem  item;
	EggDockNotebookPrivate *_priv;
};

struct _EggDockNotebookClass {
    EggDockItemClass  parent_class;
	gpointer unused[2];
};


/* public interface */
 
GtkWidget     *egg_dock_notebook_new               (void);

GType          egg_dock_notebook_get_type          (void);

G_END_DECLS

#endif
