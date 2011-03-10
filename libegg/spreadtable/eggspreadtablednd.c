/* gtkspreadtablednd.c
 * Copyright (C) 2011 Openismus GmbH
 *
 * Authors:
 *      Tristan Van Berkom <tristanvb@openismus.com>
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

#include "config.h"
#include <gtk/gtk.h>
#include <string.h>
#include "eggspreadtablednd.h"

#define DEFAULT_LINES 2
#define P_(msgid) (msgid)

/* GtkWidgetClass */
static void          egg_spread_table_dnd_realize            (GtkWidget         *widget);
static void          egg_spread_table_dnd_drag_data_received (GtkWidget         *widget,
							      GdkDragContext    *drag_context,
							      gint               x,
							      gint               y,
							      GtkSelectionData  *data,
							      guint              info,
							      guint              time);

/* GtkContainerClass */
static void          egg_spread_table_dnd_remove       (GtkContainer      *container,
							GtkWidget         *child);

/* EggSpreadTableClass */
static void          egg_spread_table_dnd_insert_child (EggSpreadTable    *spread_table,
							GtkWidget         *child,
							gint               index);

/* Drag and Drop callbacks & other utilities */
static void          drag_data_get                     (GtkWidget         *widget,
							GdkDragContext    *context,
							GtkSelectionData  *selection,
							guint              info,
							guint              time,
							EggSpreadTableDnd *spread_table);
static GtkWidget    *get_drag_child                    (EggSpreadTableDnd *spread_table,
							const GtkSelectionData *selection);
static gint          get_drop_target_index             (EggSpreadTableDnd *spread_table,
							GtkWidget         *drag_child,
							gint               x,
							gint               y);

struct _EggSpreadTableDndPrivate {
  GtkWidget *drag_child;
};

typedef struct {
  EggSpreadTableDnd *table;
  GtkWidget         *child;
} EggSpreadTableDndDragData;

static GdkAtom              dnd_target_atom_child = GDK_NONE;
static const GtkTargetEntry dnd_targets[] = {
  { "application/x-egg-spread-table-dnd-child", GTK_TARGET_SAME_APP, 0 }
};

G_DEFINE_TYPE (EggSpreadTableDnd, egg_spread_table_dnd, EGG_TYPE_SPREAD_TABLE)

static void
egg_spread_table_dnd_class_init (EggSpreadTableDndClass *class)
{
  GtkWidgetClass      *widget_class    = GTK_WIDGET_CLASS (class);
  GtkContainerClass   *container_class = GTK_CONTAINER_CLASS (class);
  EggSpreadTableClass *spread_class    = EGG_SPREAD_TABLE_CLASS (class);

  widget_class->realize            = egg_spread_table_dnd_realize;
  widget_class->drag_data_received = egg_spread_table_dnd_drag_data_received;

  container_class->remove    = egg_spread_table_dnd_remove;
  spread_class->insert_child = egg_spread_table_dnd_insert_child;

  dnd_target_atom_child = gdk_atom_intern_static_string (dnd_targets[0].target);

  g_type_class_add_private (class, sizeof (EggSpreadTableDndPrivate));
}

static void
egg_spread_table_dnd_init (EggSpreadTableDnd *spread_table)
{
  EggSpreadTableDndPrivate *priv;

  spread_table->priv = priv =
    G_TYPE_INSTANCE_GET_PRIVATE (spread_table, EGG_TYPE_SPREAD_TABLE_DND, EggSpreadTableDndPrivate);

  /* Setup the spread table as a drag target for our target type */
  gtk_drag_dest_set (GTK_WIDGET (spread_table), 
		     GTK_DEST_DEFAULT_ALL, 
		     dnd_targets, G_N_ELEMENTS (dnd_targets), 
		     GDK_ACTION_MOVE);

  gtk_widget_set_has_window (GTK_WIDGET (spread_table), TRUE);
}

/*****************************************************
 *                 GtkWidgetClass                    *
 *****************************************************/
static void
egg_spread_table_dnd_realize (GtkWidget *widget)
{
  GtkAllocation allocation;
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;

  gtk_widget_set_realized (widget, TRUE);

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.event_mask = gtk_widget_get_events (widget)
                         | GDK_VISIBILITY_NOTIFY_MASK | GDK_EXPOSURE_MASK
                         | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                         | GDK_BUTTON_MOTION_MASK;
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

  window = gdk_window_new (gtk_widget_get_parent_window (widget),
                           &attributes, attributes_mask);
  gtk_widget_set_window (widget, window);
  gdk_window_set_user_data (window, widget);

  gtk_style_context_set_background (gtk_widget_get_style_context (widget), window);
}

static void
egg_spread_table_dnd_drag_data_received (GtkWidget        *widget,
					 GdkDragContext   *drag_context,
					 gint              x,
					 gint              y,
					 GtkSelectionData *data,
					 guint             info,
					 guint             time)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);
  GtkWidget         *child;
  gint               index;

  /* Action time ! now reorder the child to the new position */
  child = get_drag_child (spread_table, data);

  if (child)
    {
      /* This will return -1 if the drop was not on a widget */
      index = get_drop_target_index (spread_table, child, x, y);

      g_object_ref (child);
      gtk_container_remove (GTK_CONTAINER (spread_table), child);
      egg_spread_table_insert_child (EGG_SPREAD_TABLE (spread_table), child, index);
      g_object_unref (child);
    }
}

/*****************************************************
 *                GtkContainerClass                  *
 *****************************************************/
static void
egg_spread_table_dnd_remove (GtkContainer *container,
			     GtkWidget    *child)
{
  /* Disconnect dnd */
  g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_data_get), container);
  gtk_drag_source_unset (child);
  
  GTK_CONTAINER_CLASS (egg_spread_table_dnd_parent_class)->remove (container, child);
}


/*****************************************************
 *               EggSpreadTableClass                 *
 *****************************************************/
static void
egg_spread_table_dnd_insert_child (EggSpreadTable *spread_table,
				   GtkWidget      *child,
				   gint            index)
{
  EGG_SPREAD_TABLE_CLASS (egg_spread_table_dnd_parent_class)->insert_child (spread_table, child, index);

  /* Connect dnd */
  gtk_drag_source_set (child, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
		       dnd_targets, G_N_ELEMENTS (dnd_targets), 
		       GDK_ACTION_MOVE);

  g_signal_connect (child, "drag-data-get", 
		    G_CALLBACK (drag_data_get), spread_table);
}

/*****************************************************
 *       Drag'n'Drop signals & other functions       *
 *****************************************************/
static void
drag_data_get (GtkWidget         *widget,
	       GdkDragContext    *context,
	       GtkSelectionData  *selection,
	       guint              info,
	       guint              time,
	       EggSpreadTableDnd *spread_table)
{
  EggSpreadTableDndDragData drag_data = { spread_table, NULL };
  GdkAtom target;

  target = gtk_selection_data_get_target (selection);

  if (target == dnd_target_atom_child)
    {
      drag_data.child = widget;

      gtk_selection_data_set (selection, target, 8,
			      (guchar*) &drag_data, sizeof (drag_data));
    }
}

static GtkWidget *
get_drag_child (EggSpreadTableDnd      *spread_table,
		const GtkSelectionData *selection)
{
  EggSpreadTableDndDragData *data;
  GdkAtom target;

  g_return_val_if_fail (gtk_selection_data_get_format (selection) == 8, NULL);
  g_return_val_if_fail (gtk_selection_data_get_length (selection) == sizeof (EggSpreadTableDndDragData), NULL);
  target = gtk_selection_data_get_target (selection);
  g_return_val_if_fail (target == dnd_target_atom_child, NULL);

  data = (EggSpreadTableDndDragData*) gtk_selection_data_get_data (selection);

  g_return_val_if_fail (data->table == spread_table, NULL);

  return data->child;
}

static gint
get_drop_target_index (EggSpreadTableDnd *spread_table,
		       GtkWidget         *drag_child,
		       gint               x,
		       gint               y)
{
  GtkWidget    *child;
  GList        *children, *l;
  GtkAllocation allocation;
  gboolean      found = FALSE;
  gint          i = 0, drag_child_index = -1;

  children = gtk_container_get_children (GTK_CONTAINER (spread_table));

  for (i = 0, l = children; l; l = l->next, i++)
    {
      child = l->data;

      if (!gtk_widget_get_visible (child))
	continue;

      gtk_widget_get_allocation (child, &allocation);

      if (child == drag_child)
	drag_child_index = i;

      if (x >= allocation.x && x <= allocation.x + allocation.width &&
	  y >= allocation.y && y <= allocation.y + allocation.height)
	{
	  found = TRUE;
	  break;
	}
    }

  g_list_free (children);

  if (found)
    {
      if (drag_child_index >= 0 && drag_child_index < i)
	return i - 1;
      else
	return i;
    }

  return -1;
}


/*****************************************************
 *                       API                         *
 *****************************************************/

/**
 * egg_spread_table_dnd_new:
 * @orientation: The #GtkOrientation for the #EggSpreadTableDnd
 * @lines: The fixed amount of lines to distribute children to.
 *
 * Creates a #EggSpreadTableDnd.
 *
 * Returns: A new #EggSpreadTableDnd container
 */
GtkWidget *
egg_spread_table_dnd_new (GtkOrientation orientation,
			  guint          lines)
{
  return (GtkWidget *)g_object_new (EGG_TYPE_SPREAD_TABLE_DND,
				    "orientation", orientation,
				    "lines", lines,
				    NULL);
}
