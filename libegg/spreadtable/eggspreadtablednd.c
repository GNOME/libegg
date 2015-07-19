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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h"
#include <gtk/gtk.h>
#include <string.h>
#include "eggspreadtablednd.h"
#include "eggplaceholder.h"
#include "eggmarshalers.h"

#define DEFAULT_LINES 2
#define P_(msgid) (msgid)

/* GObjectClass */
static void          egg_spread_table_dnd_get_property      (GObject            *object,
							     guint               prop_id,
							     GValue             *value,
							     GParamSpec         *pspec);
static void          egg_spread_table_dnd_set_property      (GObject            *object,
							     guint               prop_id,
							     const GValue       *value,
							     GParamSpec         *pspec);

/* GtkWidgetClass */
static void          egg_spread_table_dnd_realize            (GtkWidget         *widget);
static gboolean      egg_spread_table_dnd_draw               (GtkWidget *widget,
                    cairo_t *cr);
static void          egg_spread_table_dnd_size_allocate      (GtkWidget         *widget,
							      GtkAllocation     *allocation);

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
static void          egg_spread_table_dnd_insert_child_impl  (EggSpreadTable    *spread_table,
							      GtkWidget         *child,
							      gint               index);
static gint          egg_spread_table_dnd_build_segments     (EggSpreadTable    *table,
							      gint               for_size,
							      gint             **segments);

/* EggSpreadTableDndClass */
static gboolean      egg_spread_table_dnd_drop_possible(EggSpreadTableDnd *table,
							GtkWidget         *widget,
							gboolean          *possible);

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

  GtkWidget *drag_child;       /* If the drag started on a widget with no window, then the spread table
				* keeps a hold on which child is being dragged */

  guint      dragging     : 1; /* Whether a drag'n'drop operation is currently active over this table */

  guint      drop_enabled : 1; /* Whether dropping is allowed on this table */
  guint      drag_enabled : 2; /* The EggDragEnableMode (can cause the event-boxes to
				* place thier event window above of below all children) */

  gint       disappearing;     /* Count of placeholders that are currently disappearing */
  gint      *locked_config;    /* Caching and locking the child configuration */
};


enum {
  PROP_0,
  PROP_DRAG_ENABLED,
  PROP_DROP_ENABLED
};

enum {
  SIGNAL_DROP_POSSIBLE,
  LAST_SIGNAL
};

static guint                dnd_table_signals [LAST_SIGNAL] = { 0 };
static GQuark		    dnd_table_child_quark     = 0;
static GQuark		    dnd_table_connected_quark = 0;
static GdkAtom              dnd_target_atom_child = GDK_NONE;
static const GtkTargetEntry dnd_targets[] = {
  { "application/x-egg-spread-table-dnd-child", GTK_TARGET_SAME_APP, 0 }
};


G_DEFINE_TYPE (EggSpreadTableDnd, egg_spread_table_dnd, EGG_TYPE_SPREAD_TABLE)

static void
egg_spread_table_dnd_class_init (EggSpreadTableDndClass *class)
{
  GObjectClass        *gobject_class   = G_OBJECT_CLASS (class);
  GtkWidgetClass      *widget_class    = GTK_WIDGET_CLASS (class);
  GtkContainerClass   *container_class = GTK_CONTAINER_CLASS (class);
  EggSpreadTableClass *spread_class    = EGG_SPREAD_TABLE_CLASS (class);

  gobject_class->get_property           = egg_spread_table_dnd_get_property;
  gobject_class->set_property           = egg_spread_table_dnd_set_property;

  widget_class->realize                 = egg_spread_table_dnd_realize;
  widget_class->size_allocate           = egg_spread_table_dnd_size_allocate;
  widget_class->draw                    = egg_spread_table_dnd_draw;
  widget_class->drag_leave              = egg_spread_table_dnd_drag_leave;
  widget_class->drag_motion             = egg_spread_table_dnd_drag_motion;
  widget_class->drag_drop               = egg_spread_table_dnd_drag_drop;
  widget_class->drag_data_received      = egg_spread_table_dnd_drag_data_received;

  container_class->remove               = egg_spread_table_dnd_remove;

  spread_class->insert_child            = egg_spread_table_dnd_insert_child_impl;
  spread_class->build_segments_for_size = egg_spread_table_dnd_build_segments;

  class->widget_drop_possible           = egg_spread_table_dnd_drop_possible;

  /**
   * EggSpreadTableDnd:drag-enabled:
   *
   * Specifies the #EggDragEnableMode controlling whether child
   * widgets can be dragged from this table.
   *
   * 
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DRAG_ENABLED,
                                   g_param_spec_int ("drag-enabled",
						     P_("Drag Enabled"),
						     P_("The #EggDragEnableMode controlling whether "
							"children can be dragged"),
						     EGG_DRAG_DISABLED,
						     EGG_DRAG_FULL,
						     EGG_DRAG_ENABLED,
						     G_PARAM_READABLE | G_PARAM_WRITABLE));

  /**
   * EggSpreadTableDnd:drop-enabled:
   *
   * Whether this spread table accepts drops.
   *
   * If this is set to %FALSE then the EggSpreadTableDnd::widget-drop-possible
   * signal will never be emitted.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_DROP_ENABLED,
                                   g_param_spec_boolean ("drop-enabled",
							 P_("Drop Enabled"),
							 P_("Whether this spread table accepts drops"),
							 TRUE,
							 G_PARAM_READABLE | G_PARAM_WRITABLE));


  /**
   * EggSpreadTableDnd::widget-drop-possible:
   * @eggspreadtablednd: An #EggSpreadTableDnd
   * @widget: The currently dragging widget to check
   * @drop_possible: (out): Location to store whether the @eggspreadtablednd should accept @widget
   *
   * Emitted to check if @widget can be dropped into @eggspreadtablednd.
   *
   * The first connected signal to return TRUE decides whether @widget
   * can be dropped into @eggspreadtablednd.
   *
   * When handling this signal and returning %TRUE, you must set the value of @drop_possible:
   * <programlisting><![CDATA[
   * static gboolean
   * my_spread_table_accepts_your_widget_cb (EggSpreadTableDnd *table,
   *                                         GtkWidget         *child,
   *                                         gboolean          *drop_possible,
   *                                         gpointer           user_data)
   * {
   *   if (i_would_rather_the_default_behaviour_for (child))
   *     return FALSE;
   *
   *   if (child_widget_asked_kindly (child))
   *     *drop_possible = TRUE;
   *   else
   *     *drop_possible = FALSE;
   *
   *   return TRUE;
   * }
   * ]]></programlisting>
   *
   * By default EggSpreadTableDnd accepts drops from only the same table
   *
   * To disable dropping completely in a spread table, use egg_spread_table_set_drop_enabled().
   */
  dnd_table_signals[SIGNAL_DROP_POSSIBLE] =
        g_signal_new ("widget-drop-possible",
		      G_TYPE_FROM_CLASS (class),
		      G_SIGNAL_RUN_LAST,
		      G_STRUCT_OFFSET (EggSpreadTableDndClass, widget_drop_possible),
		      boolean_handled_accumulator, NULL,
		      _egg_marshal_BOOLEAN__OBJECT_POINTER,
		      G_TYPE_BOOLEAN, 2, GTK_TYPE_WIDGET, G_TYPE_POINTER);


  dnd_target_atom_child     = gdk_atom_intern_static_string (dnd_targets[0].target);
  dnd_table_child_quark     = g_quark_from_static_string ("egg-spread-table-dnd-child");
  dnd_table_connected_quark = g_quark_from_static_string ("egg-spread-table-dnd-connected");

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
		     0,
		     dnd_targets, G_N_ELEMENTS (dnd_targets),
		     GDK_ACTION_MOVE);

  /* Setup the spread table as a drag source for our target type also
   * (to handle no-window widget children) */
  gtk_drag_source_set (GTK_WIDGET (spread_table),
		       0, dnd_targets, G_N_ELEMENTS (dnd_targets),
		       GDK_ACTION_MOVE);

  priv->drag_enabled = EGG_DRAG_ENABLED;
  priv->drop_enabled = TRUE;

  gtk_widget_set_has_window (GTK_WIDGET (spread_table), TRUE);
}

/*****************************************************
 *                  GObectClass                      *
 *****************************************************/
static void
egg_spread_table_dnd_get_property (GObject      *object,
				   guint         prop_id,
				   GValue       *value,
				   GParamSpec   *pspec)
{
  EggSpreadTableDnd *table = EGG_SPREAD_TABLE_DND (object);

  switch (prop_id)
    {
    case PROP_DRAG_ENABLED:
      g_value_set_int (value, table->priv->drag_enabled);
      break;
    case PROP_DROP_ENABLED:
      g_value_set_boolean (value, table->priv->drop_enabled);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
egg_spread_table_dnd_set_property (GObject      *object,
				   guint         prop_id,
				   const GValue *value,
				   GParamSpec   *pspec)
{
  EggSpreadTableDnd *table = EGG_SPREAD_TABLE_DND (object);

  switch (prop_id)
    {
    case PROP_DRAG_ENABLED:
      egg_spread_table_dnd_set_drag_enabled (table, g_value_get_int (value));
      break;
    case PROP_DROP_ENABLED:
      egg_spread_table_dnd_set_drop_enabled (table, g_value_get_int (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
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
}

static gboolean
egg_spread_table_dnd_draw (GtkWidget *widget,
  cairo_t *cr)
{
  GtkStyleContext *context;
  GtkAllocation allocation;

  context = gtk_widget_get_style_context (widget);
  gtk_widget_get_allocation (widget, &allocation);

  gtk_render_background (context, cr, 0, 0, allocation.width, allocation.height);
  gtk_render_frame (context, cr, 0, 0, allocation.width, allocation.height);

  return GTK_WIDGET_CLASS (egg_spread_table_dnd_parent_class)->draw (widget, cr);
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
get_spread_table_dimensions (EggSpreadTableDnd *spread_table,
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
    {
      if (lines == 0)
        *line_width = 0;
      else 
        *line_width = (local_full_size - (local_spacing * (lines -1))) / lines;
    }
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

  get_spread_table_dimensions (table, -1, &line_spacing, &item_spacing, &full_thickness, &line_thickness);
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

  /* Calculate the size of the required placeholder based on the dimensions of the drag widget */
  get_spread_table_dimensions (spread_table, -1, NULL, NULL, NULL, &line_width);

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
	  g_object_set_qdata (G_OBJECT (spread_table->priv->drop_target),
			      dnd_table_child_quark, GINT_TO_POINTER (TRUE));

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
egg_spread_table_connect_child (gpointer spread_table,
				gpointer child)
{
  if (!EGG_IS_PLACEHOLDER (child) &&
      GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (child), dnd_table_connected_quark)) == FALSE)
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

      g_object_set_qdata (G_OBJECT (child),
			  dnd_table_connected_quark, GINT_TO_POINTER (TRUE));
    }
}

static void
egg_spread_table_disconnect_child (gpointer spread_table,
				   gpointer child)
{
  /* Disconnect dnd */
  if (!EGG_IS_PLACEHOLDER (child) &&
      GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (child), dnd_table_connected_quark)))
    {
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_data_get), spread_table);
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_failed), spread_table);
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_begin), spread_table);
      g_signal_handlers_disconnect_by_func (child, G_CALLBACK (drag_end), spread_table);
      gtk_drag_source_unset (child);

      g_object_set_qdata (G_OBJECT (child),
			  dnd_table_connected_quark, GINT_TO_POINTER (FALSE));
    }
}

static void
egg_spread_table_dnd_remove (GtkContainer *container,
			     GtkWidget    *child)
{
  if (GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (child), dnd_table_child_quark)) == FALSE)
    {
      g_message ("Refusing to remove child widget from EggSpreadTableDnd directly, "
		 "use egg_spread_table_dnd_remove_child() instead.");
      return;
    }

  /* Disconnect dnd */
  egg_spread_table_disconnect_child (container, child);

  GTK_CONTAINER_CLASS (egg_spread_table_dnd_parent_class)->remove (container, child);
}


/*****************************************************
 *               EggSpreadTableClass                 *
 *****************************************************/
static void
egg_spread_table_dnd_insert_child_impl (EggSpreadTable *spread_table,
					GtkWidget      *child,
					gint            index)
{
  EggSpreadTableDnd *table = EGG_SPREAD_TABLE_DND (spread_table);

  if (GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (child), dnd_table_child_quark)) == FALSE)
    {
      g_message ("Refusing to add child widget to an EggSpreadTableDnd directly, "
		 "use egg_spread_table_dnd_insert_child() instead.");
      return;
    }

  EGG_SPREAD_TABLE_CLASS (egg_spread_table_dnd_parent_class)->insert_child (spread_table, child, index);

  /* Connect dnd after really parenting the child */
  if (table->priv->drag_enabled != EGG_DRAG_DISABLED)
    egg_spread_table_connect_child (spread_table, child);
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

  get_spread_table_dimensions (dnd_table, for_size, NULL, &spacing, NULL, &line_thickness);

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
				    GtkWidget         *widget,
				    gboolean          *possible)
{
  *possible = (GTK_WIDGET (table) == gtk_widget_get_parent (widget));

  return TRUE;
}

/*****************************************************
 *       Drag'n'Drop signals & other functions       *
 *****************************************************/
static void
set_drag_icon (GtkWidget      *widget,
	       GdkDragContext *context)
{
  GtkAllocation    allocation;
  cairo_surface_t *surface;
  cairo_t         *cr;
  GtkStyleContext *style;
  GdkPixbuf       *pixbuf;
  gint             hot_x, hot_y;
  GdkModifierType  modifier_mask;

  /* XXX Force allocate here ? need to absolutely have an allocated widget
   * for this to work (gtk_widget_draw() needs that). */

  gtk_widget_get_allocation (widget, &allocation);

  gdk_window_get_device_position(
    gtk_widget_get_window (widget),
    gtk_get_current_event_device (),
    &hot_x, &hot_y, &modifier_mask);

  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);
  cr      = cairo_create (surface);

  /* Synthetically render a background */
  style = gtk_widget_get_style_context (widget);
  gtk_style_context_save (style);
  gtk_style_context_add_class (style, GTK_STYLE_CLASS_BACKGROUND);
  gtk_render_background (style, cr, 0, 0, allocation.width, allocation.height);
  gtk_style_context_restore (style);

  /* Draw the actual widget, this might or might not draw the background */
  gtk_widget_draw (widget, cr);

  /* Make a pixbuf and use that (just to take advantage of the 'hot_x'/'hot_y' parameters) */
  pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, allocation.width, allocation.height);
  gtk_drag_set_icon_pixbuf (context, pixbuf, hot_x, hot_y);

  g_object_unref (pixbuf);
  cairo_destroy (cr);
  cairo_surface_destroy (surface);
}

static void
drag_begin (GtkWidget         *widget,
	    GdkDragContext    *context,
	    EggSpreadTableDnd *spread_table)
{
  GtkAllocation   allocation;
  gint            drop_index;

  /* Set the icon for the drag */
  set_drag_icon (widget, context);

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

  /* Create a placeholder of the correct dimensions and insert it at the drag origin */
  gtk_widget_get_allocation (widget, &allocation);
  spread_table->priv->drop_target = egg_placeholder_new (allocation.width, allocation.height);
  g_object_set_qdata (G_OBJECT (spread_table->priv->drop_target),
		      dnd_table_child_quark, GINT_TO_POINTER (TRUE));


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
  GtkWidget      *child;
  GList          *children, *l;
  GtkAllocation   allocation;
  gint            placeholder_cnt = 0;
  GtkOrientation  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (spread_table));
  gint           *segments, lines, line = -1, i, full_size, spacing, position, line_width, first_child;
  gint            index = -1;

  table  = EGG_SPREAD_TABLE (spread_table);

  /* First find the "line" in question */
  lines    = egg_spread_table_get_lines (table);
  segments = egg_spread_table_get_segments (table);

  get_spread_table_dimensions (spread_table, -1, &spacing, NULL, &full_size, &line_width);

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

static gboolean
drop_possible (EggSpreadTableDnd *spread_table,
	       GtkWidget         *widget)
{
  gboolean possible = FALSE;
  gboolean handled = FALSE;

  if (spread_table->priv->drop_enabled)
    g_signal_emit (spread_table, dnd_table_signals[SIGNAL_DROP_POSSIBLE], 0, widget, &possible, &handled);

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

/**
 * egg_spread_table_dnd_insert_child:
 * @table: An #EggSpreadTableDnd
 * @child: The child widget to insert.
 *
 * Adds a child widget to an #EggSpreadTableDnd.
 *
 * <note><para>Regular #GtkContainer apis and #EggSpreadTable
 * apis are inappropriate for adding children as those
 * are reserved for internal use by the #EggSpreadTableDnd.</para></note>
 */
void
egg_spread_table_dnd_insert_child (EggSpreadTableDnd *table,
				   GtkWidget         *child,
				   gint               index)
{
  GtkWidget *event_box;

  g_return_if_fail (EGG_IS_SPREAD_TABLE_DND (table));
  g_return_if_fail (GTK_IS_WIDGET (child));

  event_box = gtk_event_box_new ();
  gtk_event_box_set_above_child (GTK_EVENT_BOX (event_box), table->priv->drag_enabled == EGG_DRAG_FULL);
  g_object_set_qdata (G_OBJECT (event_box), dnd_table_child_quark, GINT_TO_POINTER (TRUE));
  gtk_widget_show (event_box);

  gtk_container_add (GTK_CONTAINER (event_box), child);
  egg_spread_table_insert_child (EGG_SPREAD_TABLE (table), event_box, index);
}

/**
 * egg_spread_table_dnd_remove_child:
 * @table: An #EggSpreadTableDnd
 * @child: The child widget to insert.
 *
 * Adds a child widget to an #EggSpreadTableDnd.
 *
 * <note><para>Regular #GtkContainer apis and #EggSpreadTable
 * apis are inappropriate for removing children as those
 * are reserved for internal use by the #EggSpreadTableDnd.</para></note>
 */
void
egg_spread_table_dnd_remove_child (EggSpreadTableDnd *table,
				   GtkWidget         *child)
{
  GtkWidget *event_box;

  g_return_if_fail (EGG_IS_SPREAD_TABLE_DND (table));
  g_return_if_fail (GTK_IS_WIDGET (child));

  event_box = gtk_widget_get_parent (child);
  if (!event_box)
    {
      g_message ("Bad hierarchy encountered in %s. The child had no parent.", G_STRFUNC);
      return;
    }

  if (GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (event_box), dnd_table_child_quark)) == FALSE)
    {
      g_message ("Bad hierarchy encountered in %s.", G_STRFUNC);
      return;
    }

  /* unparent the user's child and remove the intermediate spread-table owned event-box. */
  gtk_container_remove (GTK_CONTAINER (event_box), child);
  gtk_container_remove (GTK_CONTAINER (table), event_box);
}

static void
reconfigure_children (GtkEventBox       *child,
		      EggSpreadTableDnd *table)
{
  /* Besides the internally owned event boxes, only EggPlaceholders can exist 
   * as direct children of the EggSpreadTableDnd */
  if (GTK_IS_EVENT_BOX (child))
    {
      gtk_event_box_set_above_child (child, table->priv->drag_enabled == EGG_DRAG_FULL);

      if (table->priv->drag_enabled == EGG_DRAG_DISABLED)
	egg_spread_table_disconnect_child (table, child);
      else
	egg_spread_table_connect_child (table, child);
    }
}

/**
 * egg_spread_table_dnd_set_drag_enabled:
 * @table: An #EggSpreadTableDnd
 * @drag_enabled: The #EggDragEnableMode to set
 *
 * Sets the #EggDragEnableMode determining whther
 * dragging of children is enabled.
 */
void
egg_spread_table_dnd_set_drag_enabled (EggSpreadTableDnd *table,
				       EggDragEnableMode  drag_enabled)
{
  g_return_if_fail (EGG_IS_SPREAD_TABLE_DND (table));

  if (table->priv->drag_enabled != drag_enabled)
    {
      table->priv->drag_enabled = drag_enabled;

      gtk_container_foreach (GTK_CONTAINER (table),
			     (GtkCallback)reconfigure_children, table);

      g_object_notify (G_OBJECT (table), "drag-enabled");
    }
}

/**
 * egg_spread_table_dnd_get_drag_enabled:
 * @table: An #EggSpreadTableDnd
 *
 * Gets the #EggDragEnableMode determining whther
 * dragging of children is enabled.
 *
 * Returns: The #EggDragEnableMode for @table
 */
EggDragEnableMode
egg_spread_table_dnd_get_drag_enabled (EggSpreadTableDnd *table)
{
  g_return_val_if_fail (EGG_IS_SPREAD_TABLE_DND (table), 0);

  return table->priv->drag_enabled;
}

/**
 * egg_spread_table_dnd_set_drop_enabled:
 * @table: An #EggSpreadTableDnd
 * @drop_enabled: Whether to enable or disable dropping in @table
 *
 * Enables/disables dropping of children into @table.
 */
void
egg_spread_table_dnd_set_drop_enabled (EggSpreadTableDnd *table,
				       gboolean           drop_enabled)
{
  g_return_if_fail (EGG_IS_SPREAD_TABLE_DND (table));

  if (table->priv->drop_enabled != drop_enabled)
    {
      table->priv->drop_enabled = drop_enabled;

      g_object_notify (G_OBJECT (table), "drop-enabled");
    }
}

/**
 * egg_spread_table_dnd_get_drop_enabled:
 * @table: An #EggSpreadTableDnd
 *
 * Gets whether dropping of children into @table is enabled.
 *
 * Returns: Whether dropping of children into @table is enabled.
 */
gboolean
egg_spread_table_dnd_get_drop_enabled (EggSpreadTableDnd *table)
{
  g_return_val_if_fail (EGG_IS_SPREAD_TABLE_DND (table), FALSE);

  return table->priv->drop_enabled;
}

