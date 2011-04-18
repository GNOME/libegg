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
#include "eggplaceholder.h"
#include "eggmarshalers.h"

#define DEFAULT_LINES 2
#define P_(msgid) (msgid)

/* GtkWidgetClass */
static void          egg_spread_table_dnd_realize            (GtkWidget         *widget);
static gboolean      egg_spread_table_dnd_motion             (GtkWidget         *widget,
							      GdkEventMotion    *event);
static gboolean      egg_spread_table_dnd_leave              (GtkWidget         *widget,
							      GdkEventCrossing  *event);
static gboolean      egg_spread_table_dnd_button_press       (GtkWidget         *widget,
							      GdkEventButton    *event);
static gboolean      egg_spread_table_dnd_button_release     (GtkWidget         *widget,
							      GdkEventButton    *event);
static void          egg_spread_table_dnd_size_allocate      (GtkWidget         *widget,
							      GtkAllocation     *allocation);

/* GtkWidgetClass drag-source */
static void          egg_spread_table_dnd_drag_data_get      (GtkWidget         *widget,
							      GdkDragContext    *context,
							      GtkSelectionData  *selection_data,
							      guint              info,
							      guint              time_);

/* GtkWidgetClass drag-dest */
static void          egg_spread_table_dnd_drag_leave         (GtkWidget         *widget,
							      GdkDragContext    *context,
							      guint              time_);
static gboolean      egg_spread_table_dnd_drag_motion        (GtkWidget	        *widget,
							      GdkDragContext    *context,
							      gint               x,
							      gint               y,
							      guint              time_);
static gboolean      egg_spread_table_dnd_drag_drop          (GtkWidget         *widget,
							      GdkDragContext    *context,
							      gint               x,
							      gint               y,
							      guint              time_);
static void          egg_spread_table_dnd_drag_data_received (GtkWidget         *widget,
							      GdkDragContext    *drag_context,
							      gint               x,
							      gint               y,
							      GtkSelectionData  *data,
							      guint              info,
							      guint              time);

/* GtkContainerClass */
static void          egg_spread_table_dnd_remove             (GtkContainer      *container,
							      GtkWidget         *child);

/* EggSpreadTableClass */
static void          egg_spread_table_dnd_insert_child       (EggSpreadTable    *spread_table,
							      GtkWidget         *child,
							      gint               index);
static gint          egg_spread_table_dnd_build_segments     (EggSpreadTable    *table,
							      gint               for_size,
							      gint             **segments);

/* EggSpreadTableDndClass */
static gboolean      egg_spread_table_dnd_drop_possible(EggSpreadTableDnd *table,
							GtkWidget         *widget);

/* Drag and Drop callbacks & other utilities */
static void          drag_begin                        (GtkWidget         *widget,
							GdkDragContext    *context,
							EggSpreadTableDnd *spread_table);
static void          drag_end                          (GtkWidget         *widget,
							GdkDragContext    *context,
							EggSpreadTableDnd *spread_table);
static void          drag_data_get                     (GtkWidget         *widget,
							GdkDragContext    *context,
							GtkSelectionData  *selection,
							guint              info,
							guint              time,
							EggSpreadTableDnd *spread_table);
static gboolean      drag_failed                       (GtkWidget         *widget,
							GdkDragContext    *drag_context,
							GtkDragResult      result,
							EggSpreadTableDnd *spread_table);

static GtkWidget    *get_child_at_position             (EggSpreadTableDnd *spread_table,
							gint               x,
							gint               y);
static gint          get_index_at_position             (EggSpreadTableDnd *spread_table,
							gint               x,
							gint               y,
							gint              *line_ret);
static gboolean      boolean_handled_accumulator       (GSignalInvocationHint *ihint,
							GValue                *return_accu,
							const GValue          *handler_return,
							gpointer               dummy);

static gboolean      drop_possible                     (EggSpreadTableDnd *spread_table,
							GtkWidget         *widget);
static void          adjust_line_segment               (EggSpreadTableDnd *table,
							gint               segment,
							gint               offset);
static void          lock_table                        (EggSpreadTableDnd *spread_table);
static void          unlock_table                      (EggSpreadTableDnd *spread_table);
static void          animate_out_drop_target           (EggSpreadTableDnd *table,
							gboolean           end);

typedef struct {
  EggSpreadTableDnd *table;
  GtkWidget         *child;
} EggSpreadTableDndDragData;

struct _EggSpreadTableDndPrivate {

  /* State of drop target while drag-motion is happening over this spread table */
  GtkWidget *drop_target;  /* Remember which child widget is the active placeholder */

  /* After successfully calling gtk_drag_get_data(), the drag data ends up in this struct */
  EggSpreadTableDndDragData drag_data;

  GtkWidget *drag_child;   /* If the drag started on a widget with no window, then the spread table
			    * keeps a hold on which child is being dragged */

  guint      dragging : 1; /* Whether the drag'n'drop operation is currently active over this table */

  gint       disappearing; /* Count of placeholders that are currently disappearing */

  /* These states are used to trigger a drag operation on a child widget with no window */
  gint       pressed_button;
  gint       press_start_x;
  gint       press_start_y;

  /* Caching and locking the child configuration */
  gint      *locked_config;

};


enum {
  SIGNAL_DROP_POSSIBLE,
  LAST_SIGNAL
};

static guint                dnd_table_signals [LAST_SIGNAL] = { 0 };
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

  widget_class->realize              = egg_spread_table_dnd_realize;
  widget_class->button_press_event   = egg_spread_table_dnd_button_press;
  widget_class->button_release_event = egg_spread_table_dnd_button_release;
  widget_class->motion_notify_event  = egg_spread_table_dnd_motion;
  widget_class->leave_notify_event   = egg_spread_table_dnd_leave;

  widget_class->size_allocate        = egg_spread_table_dnd_size_allocate;

  /* Drag source */
  widget_class->drag_data_get      = egg_spread_table_dnd_drag_data_get;

  /* Drag dest */
  widget_class->drag_leave         = egg_spread_table_dnd_drag_leave;
  widget_class->drag_motion        = egg_spread_table_dnd_drag_motion;
  widget_class->drag_drop          = egg_spread_table_dnd_drag_drop;
  widget_class->drag_data_received = egg_spread_table_dnd_drag_data_received;

  container_class->remove    = egg_spread_table_dnd_remove;

  spread_class->insert_child            = egg_spread_table_dnd_insert_child;
  spread_class->build_segments_for_size = egg_spread_table_dnd_build_segments;

  class->widget_drop_possible = egg_spread_table_dnd_drop_possible;

  /**
   * EggSpreadTableDnd::widget-drop-possible
   *
   * Emitted to check if a widget can be dropped into this spread table.
   *
   * The first connected signal to return TRUE decides that the widget
   * can be dropped.
   *
   * By default EggSpreadTableDnd accepts drops from the same table
   * (the only way to dissallow drops from the same spread table is
   * to implement a subclass which overrides the
   * EggSpreadTableDndClass.widget_drop_possible() virtual method).
   */
  dnd_table_signals[SIGNAL_DROP_POSSIBLE] =
        g_signal_new ("widget-drop-possible",
		      G_TYPE_FROM_CLASS (class),
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (EggSpreadTableDndClass, widget_drop_possible),
		      boolean_handled_accumulator, NULL,
		      _egg_marshal_BOOLEAN__OBJECT,
		      G_TYPE_BOOLEAN, 1, GTK_TYPE_WIDGET);


  dnd_target_atom_child = gdk_atom_intern_static_string (dnd_targets[0].target);

  g_type_class_add_private (class, sizeof (EggSpreadTableDndPrivate));
}

static void
egg_spread_table_dnd_init (EggSpreadTableDnd *spread_table)
{
  EggSpreadTableDndPrivate *priv;

  spread_table->priv = priv =
    G_TYPE_INSTANCE_GET_PRIVATE (spread_table, EGG_TYPE_SPREAD_TABLE_DND, EggSpreadTableDndPrivate);

  priv->pressed_button = -1;

  /* Setup the spread table as a drag target for our target type */
  gtk_drag_dest_set (GTK_WIDGET (spread_table),
		     0,
		     dnd_targets, G_N_ELEMENTS (dnd_targets),
		     GDK_ACTION_MOVE);

  /* Setup the spread table as a drag source for our target type also
   * (to handle no-window widget children) */
  gtk_drag_source_set (GTK_WIDGET (spread_table),
		       0, dnd_targets, G_N_ELEMENTS (dnd_targets),
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
                         | GDK_POINTER_MOTION_MASK | GDK_LEAVE_NOTIFY_MASK;
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

  window = gdk_window_new (gtk_widget_get_parent_window (widget),
                           &attributes, attributes_mask);
  gtk_widget_set_window (widget, window);
  gdk_window_set_user_data (window, widget);

  gtk_style_context_set_background (gtk_widget_get_style_context (widget), window);
}

static gboolean
egg_spread_table_dnd_motion (GtkWidget         *widget,
			     GdkEventMotion    *event)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  if (spread_table->priv->pressed_button >= 0 &&
      gtk_drag_check_threshold (widget,
				spread_table->priv->press_start_x,
				spread_table->priv->press_start_y,
				event->x, event->y))
    {
      spread_table->priv->drag_child =
	get_child_at_position (spread_table,
			       spread_table->priv->press_start_x,
			       spread_table->priv->press_start_y);

      if (spread_table->priv->drag_child)
	{
	  gtk_drag_begin (spread_table->priv->drag_child,
			  gtk_drag_source_get_target_list (widget),
			  GDK_ACTION_MOVE,
			  spread_table->priv->pressed_button,
			  (GdkEvent*)event);
	  return TRUE;
	}
    }
  return FALSE;
}

static gboolean
egg_spread_table_dnd_leave (GtkWidget        *widget,
			    G_GNUC_UNUSED GdkEventCrossing *event)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  spread_table->priv->pressed_button = -1;

  return TRUE;
}

static gboolean
egg_spread_table_dnd_button_press (GtkWidget         *widget,
				   GdkEventButton    *event)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);
  gboolean           handled = FALSE;

  if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
    {
      /* Save press to possibly begin a drag */
      if (get_child_at_position (spread_table, event->x, event->y) &&
	  spread_table->priv->pressed_button < 0)
	{
	  spread_table->priv->pressed_button = event->button;
	  spread_table->priv->press_start_x  = event->x;
	  spread_table->priv->press_start_y  = event->y;

	  handled = TRUE;
	}
    }

  return handled;
}

static gboolean
egg_spread_table_dnd_button_release (GtkWidget      *widget,
				     GdkEventButton *event)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  if (spread_table->priv->pressed_button == (gint)event->button)
    spread_table->priv->pressed_button = -1;

  return TRUE;
}

static void
get_widget_size (GtkWidget      *widget,
		 GtkOrientation  orientation,
		 gint            for_size,
		 gint           *min_size,
		 gint           *nat_size)
{
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      if (for_size < 0)
	gtk_widget_get_preferred_width (widget, min_size, nat_size);
      else
	gtk_widget_get_preferred_width_for_height (widget, for_size, min_size, nat_size);
    }
  else
    {
      if (for_size < 0)
	gtk_widget_get_preferred_height (widget, min_size, nat_size);
      else
	gtk_widget_get_preferred_height_for_width (widget, for_size, min_size, nat_size);
    }
}

static void
allocate_child (EggSpreadTableDnd *table,
                GtkWidget         *child,
                gint               item_offset,
                gint               line_offset,
                gint               item_size,
                gint               line_size)
{
  GtkAllocation           widget_allocation;
  GtkAllocation           child_allocation;
  GtkOrientation          orientation;

  gtk_widget_get_allocation (GTK_WIDGET (table), &widget_allocation);
  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (table));

  if (gtk_widget_get_has_window (GTK_WIDGET (table)))
    {
      widget_allocation.x = 0;
      widget_allocation.y = 0;
    }

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      child_allocation.x      = widget_allocation.x + item_offset;
      child_allocation.y      = widget_allocation.y + line_offset;
      child_allocation.width  = item_size;
      child_allocation.height = line_size;
    }
  else /* GTK_ORIENTATION_VERTICAL */
    {
      child_allocation.x      = widget_allocation.x + line_offset;
      child_allocation.y      = widget_allocation.y + item_offset;
      child_allocation.width  = line_size;
      child_allocation.height = item_size;
    }

  gtk_widget_size_allocate (child, &child_allocation);
}

static void
get_spread_table_dimentions (EggSpreadTableDnd *spread_table,
			     gint               for_size,
			     gint              *line_spacing,
			     gint              *item_spacing,
			     gint              *full_size,
			     gint              *line_width)
{
  EggSpreadTable *table = EGG_SPREAD_TABLE (spread_table);
  gint local_full_size, local_item_spacing, local_spacing;
  gint lines = egg_spread_table_get_lines (table);
  GtkAllocation allocation;

  gtk_widget_get_allocation (GTK_WIDGET (table), &allocation);

  if (gtk_orientable_get_orientation (GTK_ORIENTABLE (table)) == GTK_ORIENTATION_VERTICAL)
    {
      local_full_size    = for_size < 0 ? allocation.width : for_size;
      local_spacing      = egg_spread_table_get_horizontal_spacing (table);
      local_item_spacing = egg_spread_table_get_vertical_spacing (table);
    }
  else
    {
      local_full_size    = for_size < 0 ? allocation.height : for_size;
      local_spacing      = egg_spread_table_get_vertical_spacing (table);
      local_item_spacing = egg_spread_table_get_horizontal_spacing (table);
    }

  if (full_size)
    *full_size = local_full_size;
  if (line_spacing)
    *line_spacing = local_spacing;
  if (item_spacing)
    *item_spacing = local_item_spacing;

  if (line_width)
    *line_width = (local_full_size - (local_spacing * (lines -1))) / lines;
}

static void
egg_spread_table_dnd_size_allocate (GtkWidget         *widget,
				    GtkAllocation     *allocation)
{
  EggSpreadTableDnd        *table = EGG_SPREAD_TABLE_DND (widget);
  GList                    *list, *children;
  gint                     *segments = NULL;
  gint                      full_thickness;
  gint                      i, j;
  gint                      line_offset, item_offset;
  gint                      line_thickness;
  gint                      line_spacing;
  gint                      item_spacing;
  gint                      placeholder_cnt = 0;
  gint                      lines;
  GtkWidgetClass           *parent_parent_class;
  GtkOrientation            orientation;

  /* Skip the EggSpreadTableClass allocator, chain up to it's parent to resize
   * the GdkWindow properly */
  parent_parent_class = g_type_class_peek_parent (egg_spread_table_dnd_parent_class);
  parent_parent_class->size_allocate (widget, allocation);

  get_spread_table_dimentions (table, -1, &line_spacing, &item_spacing, &full_thickness, &line_thickness);
  lines       = egg_spread_table_get_lines (EGG_SPREAD_TABLE (table));
  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (table));

  egg_spread_table_build_segments_for_size (EGG_SPREAD_TABLE (table), full_thickness, &segments);

  children = gtk_container_get_children (GTK_CONTAINER (table));

  for (list = children, line_offset = 0, i = 0; i < lines;
       line_offset += line_thickness + line_spacing, i++)
    {
      /* Count the placeholders on each line */
      placeholder_cnt = 0;

      for (j = 0, item_offset = 0; list && j < segments[i]; list = list->next, j++)
	{
	  GtkWidget *child = list->data;
	  gint       child_size;

	  if (!gtk_widget_get_visible (child))
	    continue;

	  get_widget_size (child, orientation, line_thickness, NULL, &child_size);

	  /* Stop allocating children on this line after 2 placeholders
	   * this avoids annoying flicker when moving a widget inside a single column */
	  if (placeholder_cnt >= 2)
	    continue;

	  if (placeholder_cnt < 2 && EGG_IS_PLACEHOLDER (child))
	      placeholder_cnt++;

	  allocate_child (table, child, item_offset, line_offset, child_size, line_thickness);

	  item_offset += child_size + item_spacing;
	}
    }

  g_list_free (children);
  g_free (segments);
}


/*****************************************************
 *            GtkWidgetClass drag source             *
 *****************************************************/

static void
egg_spread_table_dnd_drag_data_get (GtkWidget         *widget,
				    G_GNUC_UNUSED GdkDragContext    *context,
				    GtkSelectionData  *selection,
				    G_GNUC_UNUSED guint              info,
				    G_GNUC_UNUSED guint              time_)
{
  EggSpreadTableDnd        *spread_table = EGG_SPREAD_TABLE_DND (widget);
  EggSpreadTableDndDragData drag_data    = { spread_table, NULL };
  GdkAtom target;

  target = gtk_selection_data_get_target (selection);

  if (spread_table->priv->drag_child &&
      target == dnd_target_atom_child)
    {
      drag_data.child = spread_table->priv->drag_child;

      gtk_selection_data_set (selection, target, 8,
			      (guchar*) &drag_data, sizeof (drag_data));
    }
}

/*****************************************************
 *            GtkWidgetClass drag dest               *
 *****************************************************/
static gint
get_child_line (EggSpreadTableDnd *table,
		GtkWidget         *child)
{
  gint size;

  if (gtk_orientable_get_orientation (GTK_ORIENTABLE (table)) == GTK_ORIENTATION_VERTICAL)
    size = gtk_widget_get_allocated_width (GTK_WIDGET (table));
  else
    size = gtk_widget_get_allocated_height (GTK_WIDGET (table));

  return egg_spread_table_get_child_line (EGG_SPREAD_TABLE (table), child, size);
}

static void
placeholder_animated_out (GtkWidget         *placeholder,
			  EggSpreadTableDnd *spread_table)
{
  gint line = -1;
  gboolean last_target = FALSE;

  if (spread_table->priv->drop_target == placeholder)
    {
      spread_table->priv->drop_target = NULL;
      last_target = TRUE;
    }

  if (spread_table->priv->dragging)
    line = get_child_line (spread_table, placeholder);

  gtk_container_remove (GTK_CONTAINER (spread_table), placeholder);

  /* Adjust line segment here manually since table may be locked */
  if (spread_table->priv->dragging)
    adjust_line_segment (spread_table, line, -1);
  else if (last_target)
    /* Unlock the table after the drag is finished */
    unlock_table (spread_table);

  spread_table->priv->disappearing--;
}

static void
egg_spread_table_dnd_drag_leave (GtkWidget         *widget,
				 GdkDragContext    *context,
				 guint              time_)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  gtk_drag_get_data (widget, context, dnd_target_atom_child, time_);

  /* Animate-out drop target for drop-zone spread table */
  animate_out_drop_target (spread_table, TRUE);

  /* Animate-out drop target for drag-source spread table
   * (sometimes when drag'n'drop happens very fast no drag-leave gets
   * emitted on the source spread table so we take care of it here)
   */
  if (spread_table->priv->drag_data.table != spread_table)
    animate_out_drop_target (spread_table->priv->drag_data.table, TRUE);
}

static void
get_placeholder_size (EggSpreadTableDnd *spread_table,
		      gint              *width,
		      gint              *height)
{
  GtkOrientation orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (spread_table));
  gint           line_width;

  /* Calculate the size of the required placeholder based on the dimentions of the drag widget */
  get_spread_table_dimentions (spread_table, -1, NULL, NULL, NULL, &line_width);

  if (orientation == GTK_ORIENTATION_VERTICAL)
    {
      gint min_width;

      gtk_widget_get_preferred_width (spread_table->priv->drag_data.child, &min_width, NULL);

      *width = MAX (line_width, min_width);

      gtk_widget_get_preferred_height_for_width (spread_table->priv->drag_data.child,
						 *width, height, NULL);
    }
  else
    {
      gint min_height;

      gtk_widget_get_preferred_width (spread_table->priv->drag_data.child, &min_height, NULL);

      *height = MAX (line_width, min_height);

      gtk_widget_get_preferred_width_for_height (spread_table->priv->drag_data.child,
						 *height, width, NULL);
    }
}

static gboolean
egg_spread_table_dnd_drag_motion (GtkWidget         *widget,
				  GdkDragContext    *context,
				  gint               x,
				  gint               y,
				  guint              time_)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);
  gint               index, line, drop_index;

  /* Could be comming from another spread table, lock it now incase
   * its not already locked. */

  gtk_drag_get_data (widget, context, dnd_target_atom_child, time_);

  if (!drop_possible (spread_table, spread_table->priv->drag_data.child))
    return FALSE;

  lock_table (spread_table);
  spread_table->priv->dragging = TRUE;

  /* Dont do anything until the currently drop target placeholder finishes animating in */
  if ((spread_table->priv->drop_target &&
       egg_placeholder_get_animating
       (EGG_PLACEHOLDER (spread_table->priv->drop_target)) != EGG_PLACEHOLDER_ANIM_NONE) ||
      spread_table->priv->disappearing)
    return TRUE;

  if (spread_table->priv->drop_target)
    gtk_container_child_get (GTK_CONTAINER (spread_table),
			     spread_table->priv->drop_target,
			     "position", &drop_index,
			     NULL);
  else
    drop_index = -1;

  index = get_index_at_position (spread_table, x, y, &line);

  if (index != drop_index)
    {

      animate_out_drop_target (spread_table, FALSE);

      if (index >= 0)
	{
	  gint width, height;

	  /* Import the drag data, get the drag widget and query it's size for this spread table */
	  get_placeholder_size (spread_table, &width, &height);

	  spread_table->priv->drop_target = egg_placeholder_new (width, height);
	  egg_spread_table_insert_child (EGG_SPREAD_TABLE (spread_table),
					 spread_table->priv->drop_target, index);
	  adjust_line_segment (spread_table, line, 1);

	  egg_placeholder_animate_in (EGG_PLACEHOLDER (spread_table->priv->drop_target),
				      gtk_orientable_get_orientation (GTK_ORIENTABLE (spread_table)));
	}
    }

  return TRUE;
}

static gboolean
egg_spread_table_dnd_drag_drop (GtkWidget         *widget,
				GdkDragContext    *context,
				G_GNUC_UNUSED gint               x,
				G_GNUC_UNUSED gint               y,
				guint              time_)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  gtk_drag_get_data (widget, context, dnd_target_atom_child, time_);

  if (spread_table->priv->drop_target &&
      spread_table->priv->drag_data.child)
    {
      gint drop_index;

      /* Carry the widget over */
      g_object_ref (spread_table->priv->drag_data.child);

      gtk_container_remove (GTK_CONTAINER (spread_table->priv->drag_data.table),
			    spread_table->priv->drag_data.child);

      /* Get the appropriate target index */
      gtk_container_child_get (GTK_CONTAINER (spread_table),
			       spread_table->priv->drop_target,
			       "position", &drop_index,
			       NULL);

      /* Insert drag child at the index */
      egg_spread_table_insert_child (EGG_SPREAD_TABLE (spread_table),
				     spread_table->priv->drag_data.child,
				     drop_index);
      g_object_unref (spread_table->priv->drag_data.child);

      /* Ensure visibility */
      gtk_widget_show (spread_table->priv->drag_data.child);

      /* Hide the drop target placeholder in the target spread table,
       * it will be removed and the spread table unlocked after animating out
       * (the placeholder started animating out at "drag-leave" time).
       */
      gtk_widget_hide (spread_table->priv->drop_target);

      gtk_drag_finish (context, TRUE, TRUE, time_);
      return TRUE;
    }
  else
    {
      gtk_drag_finish (context, FALSE, TRUE, time_);
      return FALSE;
    }

}

static void
egg_spread_table_dnd_drag_data_received (GtkWidget        *widget,
					 GdkDragContext   *context,
					 G_GNUC_UNUSED gint              x,
					 G_GNUC_UNUSED gint              y,
					 GtkSelectionData *data,
					 G_GNUC_UNUSED guint             info,
					 guint             time_)
{
  EggSpreadTableDnd         *spread_table = EGG_SPREAD_TABLE_DND (widget);
  EggSpreadTableDndDragData *drag_data;

  memset (&spread_table->priv->drag_data, 0x0, sizeof (EggSpreadTableDndDragData));

  g_return_if_fail (gtk_selection_data_get_format (data) == 8);
  g_return_if_fail (gtk_selection_data_get_length (data) == sizeof (EggSpreadTableDndDragData));
  g_return_if_fail (gtk_selection_data_get_target (data) == dnd_target_atom_child);

  drag_data = (EggSpreadTableDndDragData*) gtk_selection_data_get_data (data);
  memcpy (&spread_table->priv->drag_data, drag_data, sizeof (EggSpreadTableDndDragData));

  gdk_drag_status (context, GDK_ACTION_MOVE, time_);
}

/*****************************************************
 *                GtkContainerClass                  *
 *****************************************************/
static void
egg_spread_table_dnd_remove (GtkContainer *container,
			     GtkWidget    *child)
{
  /* Disconnect dnd */
  if (!EGG_IS_PLACEHOLDER (child))
    {
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_data_get), container);
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_failed), container);
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_begin), container);
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_end), container);
      gtk_drag_source_unset (child);
    }

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
  if (!EGG_IS_PLACEHOLDER (child))
    {
      gtk_drag_source_set (child, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
			   dnd_targets, G_N_ELEMENTS (dnd_targets),
			   GDK_ACTION_MOVE);

      g_signal_connect (child, "drag-data-get",
			G_CALLBACK (drag_data_get), spread_table);
      g_signal_connect (child, "drag-failed",
			G_CALLBACK (drag_failed), spread_table);
      g_signal_connect (child, "drag-begin",
			G_CALLBACK (drag_begin), spread_table);
      g_signal_connect (child, "drag-end",
			G_CALLBACK (drag_end), spread_table);
    }
}

static gint
egg_spread_table_dnd_build_segments (EggSpreadTable *table,
				     gint            for_size,
				     gint          **segments)
{
  EggSpreadTableDnd        *dnd_table = EGG_SPREAD_TABLE_DND (table);
  EggSpreadTableDndPrivate *priv = dnd_table->priv;
  GList                    *list, *children;
  gint                      i, j, lines;
  gint                      largest_line = 0, line_size = 0;
  gint                      line_thickness;
  gint                      spacing;
  GtkOrientation            orientation;
  gboolean                  first_widget = TRUE;

  if (!priv->locked_config)
    return EGG_SPREAD_TABLE_CLASS
      (egg_spread_table_dnd_parent_class)->build_segments_for_size (table, for_size, segments);

  get_spread_table_dimentions (dnd_table, for_size, NULL, &spacing, NULL, &line_thickness);

  children    = gtk_container_get_children (GTK_CONTAINER (table));
  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (table));
  lines       = egg_spread_table_get_lines (table);

  for (list = children, i = 0; i < lines; i++)
    {
      for (j = 0; list && j < priv->locked_config[i]; list = list->next, j++)
	{
	  GtkWidget *child = list->data;
	  gint       child_size;

	  if (!gtk_widget_get_visible (child))
	    continue;

	  get_widget_size (child, orientation, line_thickness, NULL, &child_size);

	  line_size += child_size;
	  if (!first_widget)
	    line_size += spacing;
	  else
	    first_widget = FALSE;
	}

      largest_line = MAX (largest_line, line_size);
      line_size = 0;
      first_widget = TRUE;
    }

  if (segments)
    *segments = g_memdup (priv->locked_config, lines * sizeof (gint));

  g_list_free (children);

  return largest_line;
}


/*****************************************************
 *              EggSpreadTableDndClass               *
 *****************************************************/
static gboolean
egg_spread_table_dnd_drop_possible (EggSpreadTableDnd *table,
				    GtkWidget         *widget)
{
  return (GTK_WIDGET (table) == gtk_widget_get_parent (widget));
}

/*****************************************************
 *       Drag'n'Drop signals & other functions       *
 *****************************************************/

static void
drag_begin (GtkWidget         *widget,
	    G_GNUC_UNUSED GdkDragContext    *context,
	    EggSpreadTableDnd *spread_table)
{
  GtkAllocation   allocation;
  gint            drop_index;

  /* Mark the spread table for an active drag */
  lock_table (spread_table);
  spread_table->priv->dragging = TRUE;

  /* Just assign a drag child, this is only important for
   * child widgets that dont have a GdkWindow though really */
  spread_table->priv->drag_child = widget;

  /* Save the drag origin in case of failed drags and insert a placeholder as the first
   * default drop target */
  gtk_container_child_get (GTK_CONTAINER (spread_table),
			   spread_table->priv->drag_child,
			   "position", &drop_index,
			   NULL);

  /* Create a placeholder of the correct dimentions and insert it at the drag origin */
  gtk_widget_get_allocation (widget, &allocation);
  spread_table->priv->drop_target = egg_placeholder_new (allocation.width, allocation.height);

  egg_spread_table_insert_child (EGG_SPREAD_TABLE (spread_table),
				 spread_table->priv->drop_target,
				 drop_index);

  /* Add one index for the new placeholder */
  adjust_line_segment (spread_table,
		       get_child_line (spread_table, spread_table->priv->drop_target), 1);

  /* Hide the drag child (we cant remove it because it needs a GdkWindow in the mean time) */
  gtk_widget_hide (spread_table->priv->drag_child);
}

static void
drag_end (G_GNUC_UNUSED GtkWidget         *widget,
	  G_GNUC_UNUSED GdkDragContext    *context,
	  EggSpreadTableDnd *spread_table)
{
  /* Sometimes when drag'n'drop happens very fast, drag-leave,
   * drag-failed and drag-drop dont happen, so we cancel it out here
   */
  animate_out_drop_target (spread_table, TRUE);
}

static void
drag_data_get (GtkWidget         *widget,
	       G_GNUC_UNUSED GdkDragContext    *context,
	       GtkSelectionData  *selection,
	       G_GNUC_UNUSED guint              info,
	       G_GNUC_UNUSED guint              time,
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

static gboolean
drag_failed (GtkWidget         *widget,
	     G_GNUC_UNUSED GdkDragContext    *drag_context,
	     G_GNUC_UNUSED GtkDragResult      result,
	     G_GNUC_UNUSED EggSpreadTableDnd *spread_table)
{
  gtk_widget_show (widget);

  return FALSE;
}

static gint
get_index_at_position (EggSpreadTableDnd *spread_table,
		       gint               x,
		       gint               y,
		       gint              *line_ret)
{
  EggSpreadTable *table;
  GtkWidget      *widget, *child;
  GList          *children, *l;
  GtkAllocation   allocation;
  gint            placeholder_cnt = 0;
  GtkOrientation  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (spread_table));
  gint           *segments, lines, line = -1, i, full_size, spacing, position, line_width, first_child;
  gint            index = -1;

  widget = GTK_WIDGET (spread_table);
  table  = EGG_SPREAD_TABLE (spread_table);

  /* First find the "line" in question */
  lines    = egg_spread_table_get_lines (table);
  segments = egg_spread_table_get_segments (table);

  get_spread_table_dimentions (spread_table, -1, &spacing, NULL, &full_size, &line_width);

  if (orientation == GTK_ORIENTATION_VERTICAL)
    position = x;
  else
    position = y;

  for (i = 0; i < lines; i++)
    {
      gint start, end;

      start = line_width * i + (spacing / 2) * i;
      end   = start + line_width + (spacing / 2) * i;

      if (i == lines - 1)
	end = full_size;

      if (position >= start && position <= end)
	{
	  line = i;
	  break;
	}
    }
  g_assert (line >= 0);

  /* Get the first child on this line */
  for (i = 0, first_child = 0; i < line; i++)
    first_child += segments[i];

  children = gtk_container_get_children (GTK_CONTAINER (spread_table));

  for (l = g_list_nth (children, first_child), i = 0;
       l != NULL && index < 0 && i < segments[line];
       l = l->next, i++)
    {
      child = l->data;

      if (!gtk_widget_get_visible (child))
	continue;

      gtk_widget_get_allocation (child, &allocation);

      if (child == spread_table->priv->drop_target)
	{
	  if (orientation == GTK_ORIENTATION_VERTICAL)
	    {
	      if (y < allocation.y + allocation.height)
		index = first_child + i;
	    }
	  else
	    {
	      if (x < allocation.x + allocation.width)
		index = first_child + i;
	    }

	  placeholder_cnt++;
	}
      else
	{
	  if (orientation == GTK_ORIENTATION_VERTICAL)
	    {
	      if (y < allocation.y + allocation.height / 2)
		index = first_child + i + placeholder_cnt;
	    }
	  else
	    {
	      if (x < allocation.x + allocation.width / 2)
		index = first_child + i + placeholder_cnt;
	    }
	}
    }

  if (index < 0)
    index = first_child + segments[line];

  g_list_free (children);

  g_free (segments);

  if (line_ret)
    *line_ret = line;

  g_assert (index >= 0);

  return index;
}

static GtkWidget *
get_child_at_position (EggSpreadTableDnd *spread_table,
		       gint               x,
		       gint               y)
{
  GtkWidget    *child, *ret_child = NULL;
  GList        *children, *l;
  GtkAllocation allocation;

  children = gtk_container_get_children (GTK_CONTAINER (spread_table));

  for (l = children; ret_child == NULL && l != NULL; l = l->next)
    {
      child = l->data;

      if (!gtk_widget_get_visible (child))
	continue;

      gtk_widget_get_allocation (child, &allocation);

      if (x >= allocation.x && x <= allocation.x + allocation.width &&
	  y >= allocation.y && y <= allocation.y + allocation.height)
	{
	  ret_child = child;
	}
    }

  g_list_free (children);

  return ret_child;
}

static gboolean
drop_possible (EggSpreadTableDnd *spread_table,
	       GtkWidget         *widget)
{
  gboolean possible = FALSE;

  g_signal_emit (spread_table, dnd_table_signals[SIGNAL_DROP_POSSIBLE], 0, widget, &possible);

  return possible;
}

/* Copy of _gtk_boolean_handled_accumulator */
static gboolean
boolean_handled_accumulator (G_GNUC_UNUSED GSignalInvocationHint *ihint,
			     GValue                *return_accu,
			     const GValue          *handler_return,
			     G_GNUC_UNUSED gpointer               dummy)
{
  gboolean continue_emission;
  gboolean signal_handled;

  signal_handled = g_value_get_boolean (handler_return);
  g_value_set_boolean (return_accu, signal_handled);
  continue_emission = !signal_handled;

  return continue_emission;
}

static void
adjust_line_segment (EggSpreadTableDnd *table,
		     gint               segment,
		     gint               offset)
{
  if (table->priv->locked_config)
    table->priv->locked_config[segment] += offset;
}

static void
animate_out_drop_target (EggSpreadTableDnd *table,
			 gboolean           end)
{
  if (table->priv->drop_target &&
      egg_placeholder_get_animating
      (EGG_PLACEHOLDER (table->priv->drop_target)) != EGG_PLACEHOLDER_ANIM_OUT)
    {
      egg_placeholder_animate_out (EGG_PLACEHOLDER (table->priv->drop_target),
				   gtk_orientable_get_orientation (GTK_ORIENTABLE (table)));

      g_signal_connect (table->priv->drop_target, "animation-done",
			G_CALLBACK (placeholder_animated_out), table);

      table->priv->disappearing++;
    }

  if (end)
    table->priv->dragging = FALSE;
}

static void
lock_table (EggSpreadTableDnd *table)

{
  if (table->priv->locked_config == NULL)
    table->priv->locked_config = egg_spread_table_get_segments (EGG_SPREAD_TABLE (table));
}

static void
unlock_table (EggSpreadTableDnd *table)
{

  g_free (table->priv->locked_config);
  table->priv->locked_config = NULL;

  gtk_widget_queue_resize (GTK_WIDGET (table));
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
