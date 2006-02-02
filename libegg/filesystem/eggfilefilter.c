/* eggfilefilter.c
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

#include "eggfilefilter.h"

enum
{
  CHANGED,
  LAST_SIGNAL
};

static void egg_file_filter_class_init (EggFileFilterClass *klass);
static void egg_file_filter_init       (EggFileFilter      *filter);

static void egg_file_filter_finalize   (GObject            *object);

static GObjectClass *parent_class = NULL;
static guint filter_signals[LAST_SIGNAL] = { 0 };

GType
egg_file_filter_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
	{	  
	  sizeof (EggFileFilterClass),
	  NULL,		/* base_init */
	  NULL,		/* base_finalize */
	  (GClassInitFunc) egg_file_filter_class_init,
	  NULL,		/* class_finalize */
	  NULL,		/* class_data */
	  sizeof (EggFileFilter),
	  0,            /* n_preallocs */
	  (GInstanceInitFunc) egg_file_filter_init
	};
      
      object_type = g_type_register_static (G_TYPE_OBJECT, "EggFileFilter", &object_info, 0);
    }
  
  return object_type;
}

static void
egg_file_filter_class_init (EggFileFilterClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *)klass;
  parent_class = g_type_class_peek_parent (klass);
  
  gobject_class->finalize = egg_file_filter_finalize;

  filter_signals[CHANGED] =
    g_signal_new ("changed",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (EggFileFilterClass, changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
}

static void
egg_file_filter_init (EggFileFilter *filter)
{
}

static void
egg_file_filter_finalize (GObject *object)
{
  EggFileFilter *filter;

  filter = EGG_FILE_FILTER (object);

  g_free (filter->description);

  if (filter->destroy)
    (* filter->destroy) (filter->data);

  (* parent_class->finalize) (object);
}

EggFileFilter *
egg_file_filter_new (const gchar       *description,
		     EggFileFilterFunc  filter_func,
		     gpointer           data,
		     GDestroyNotify     destroy)
{
  EggFileFilter *filter;

  g_return_val_if_fail (description != NULL, NULL);
  
  filter = g_object_new (EGG_TYPE_FILE_FILTER, NULL);
  filter->description = g_strdup (description);
  filter->filter_func = filter_func;
  filter->data = data;
  filter->destroy = destroy;
  
  return filter;
}

gboolean
egg_file_filter_should_show (EggFileFilter     *filter,
			     EggFileSystemItem *item)
{
  g_return_val_if_fail (EGG_IS_FILE_FILTER (filter), FALSE);
  g_return_val_if_fail (item != NULL, FALSE);

  if (!filter->filter_func)
    return TRUE;
  
  return (* filter->filter_func) (filter, item, filter->data);
}

void
egg_file_filter_changed (EggFileFilter *filter)
{
  g_signal_emit (filter, filter_signals[CHANGED], 0);
}

