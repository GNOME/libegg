/* eggfileselector.h
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
#ifndef __EGG_FILE_FILTER_H__
#define __EGG_FILE_FILTER_H__

#include "eggfilesystem.h"

G_BEGIN_DECLS

#define EGG_TYPE_FILE_FILTER		  (egg_file_filter_get_type ())
#define EGG_FILE_FILTER(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_FILE_FILTER, EggFileFilter))
#define EGG_FILE_FILTER_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_FILE_FILTER, EggFileFilterClass))
#define EGG_IS_FILE_FILTER(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_FILE_FILTER))
#define EGG_IS_FILE_FILTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_FILE_FILTER))
#define EGG_FILE_FILTER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_FILE_FILTER, EggFileFilterClass))

typedef struct _EggFileFilter      EggFileFilter;
typedef struct _EggFileFilterClass EggFileFilterClass;
typedef gboolean (* EggFileFilterFunc) (EggFileFilter *filter, EggFileSystemItem *item, gpointer data);

struct _EggFileFilter
{
  GObject parent;

  gchar *description;
  
  EggFileFilterFunc filter_func;
  gpointer data;
  GDestroyNotify destroy;
};

struct _EggFileFilterClass
{
  GObjectClass parent_class;

  void (* changed) (EggFileFilter *filter);
};

GType          egg_file_filter_get_type    (void);

EggFileFilter *egg_file_filter_new         (const gchar       *description,
					    EggFileFilterFunc  filter_func,
					    gpointer           data,
					    GDestroyNotify     destroy);
gboolean       egg_file_filter_should_show (EggFileFilter     *filter,
					    EggFileSystemItem *item);
void           egg_file_filter_changed     (EggFileFilter     *filter);
					    
G_END_DECLS

#endif /* __EGG_FILE_FILTER_H__ */
