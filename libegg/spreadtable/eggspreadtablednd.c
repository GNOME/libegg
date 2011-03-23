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
static gboolean      egg_spread_table_dnd_motion             (GtkWidget         *widget,
							      GdkEventMotion    *event);
static gboolean      egg_spread_table_dnd_button_press       (GtkWidget         *widget,
							      GdkEventButton    *event);
static gboolean      egg_spread_table_dnd_button_release     (GtkWidget         *widget,
							      GdkEventButton    *event);

/* GtkWidgetClass drag-source */
static void          egg_spread_table_dnd_drag_begin         (GtkWidget         *widget,
							      GdkDragContext    *context);
static void          egg_spread_table_dnd_drag_end	     (GtkWidget         *widget,
							      GdkDragContext    *context);
static void          egg_spread_table_dnd_drag_data_get      (GtkWidget         *widget,
							      GdkDragContext    *context,
							      GtkSelectionData  *selection_data,
							      guint              info,
							      guint              time_);
static void          egg_spread_table_dnd_drag_data_delete   (GtkWidget         *widget,
							      GdkDragContext    *context);


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
static gboolean      egg_spread_table_dnd_drag_failed        (GtkWidget         *widget,
							      GdkDragContext    *context,
							      GtkDragResult      result);

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
static GtkWidget    *get_child_at_position             (EggSpreadTableDnd *spread_table,
							gint               x,
							gint               y,
							gboolean          *before);
static void          cancel_drag                       (EggSpreadTableDnd *spread_table);


static gboolean      drag_highlight_draw               (GtkWidget         *widget,
							cairo_t           *cr,
							gpointer           data);
static void          highlight_drag                    (GtkWidget         *widget, 
							gboolean           before);
static void          unhighlight_drag                  (GtkWidget         *widget);


struct _EggSpreadTableDndPrivate {
  GtkWidget *drop_target;  /* Remember which child widget has the drop highlight */

  GtkWidget *drag_child;   /* The widget that is currently being dragged, if the widget has
			    * no window and the SpreadTable is the active drag source */
  gint pressed_button;
  gint press_start_x;
  gint press_start_y;
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

  widget_class->realize              = egg_spread_table_dnd_realize;
  widget_class->button_press_event   = egg_spread_table_dnd_button_press;
  widget_class->button_release_event = egg_spread_table_dnd_button_release;
  widget_class->motion_notify_event  = egg_spread_table_dnd_motion;

  /* Drag source */
  widget_class->drag_begin         = egg_spread_table_dnd_drag_begin;
  widget_class->drag_end           = egg_spread_table_dnd_drag_end;
  widget_class->drag_data_get      = egg_spread_table_dnd_drag_data_get;
  widget_class->drag_data_delete   = egg_spread_table_dnd_drag_data_delete;

  /* Drag dest */
  widget_class->drag_leave         = egg_spread_table_dnd_drag_leave;
  widget_class->drag_motion        = egg_spread_table_dnd_drag_motion;
  widget_class->drag_drop          = egg_spread_table_dnd_drag_drop;
  widget_class->drag_data_received = egg_spread_table_dnd_drag_data_received;
  widget_class->drag_failed        = egg_spread_table_dnd_drag_failed;

  container_class->remove = egg_spread_table_dnd_remove;
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

  priv->pressed_button = -1;

  /* Setup the spread table as a drag target for our target type */
  gtk_drag_dest_set (GTK_WIDGET (spread_table),
		     GTK_DEST_DEFAULT_ALL,
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
 *                  GObectClass                      *
 *****************************************************/

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
                         | GDK_POINTER_MOTION_MASK;
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
			       spread_table->priv->press_start_y, NULL);

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
egg_spread_table_dnd_button_press (GtkWidget         *widget,
				   GdkEventButton    *event)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);
  gboolean           handled = FALSE;

  if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
    {
      /* Save press to possibly begin a drag */
      if (get_child_at_position (spread_table, event->x, event->y, NULL) && 
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
  
  if (spread_table->priv->pressed_button == event->button)
    spread_table->priv->pressed_button = -1;

  return TRUE;
}

/*****************************************************
 *            GtkWidgetClass drag source             *
 *****************************************************/
static void
egg_spread_table_dnd_drag_begin (GtkWidget         *widget,
				 GdkDragContext    *context)
{

}

static void
egg_spread_table_dnd_drag_end (GtkWidget         *widget,
			       GdkDragContext    *context)
{

}

static void
egg_spread_table_dnd_drag_data_get (GtkWidget         *widget,
				    GdkDragContext    *context,
				    GtkSelectionData  *selection,
				    guint              info,
				    guint              time_)
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

static void
egg_spread_table_dnd_drag_data_delete (GtkWidget         *widget,
				       GdkDragContext    *context)
{

}

/*****************************************************
 *            GtkWidgetClass drag dest               *
 *****************************************************/
static void
egg_spread_table_dnd_drag_leave (GtkWidget         *widget,
				 GdkDragContext    *context,
				 guint              time_)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  /* Cancel the drag here also, so that highlights go away when
   * leaving the target spread table */
  cancel_drag (spread_table);
}

static gboolean
egg_spread_table_dnd_drag_motion (GtkWidget         *widget,
				  GdkDragContext    *context,
				  gint               x,
				  gint               y,
				  guint              time_)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);
  GtkWidget         *drop_target;
  gboolean           before;

  drop_target = get_child_at_position (spread_table, x, y, &before);

  if (spread_table->priv->drop_target)
    unhighlight_drag (spread_table->priv->drop_target);

  spread_table->priv->drop_target = drop_target;

  if (spread_table->priv->drop_target)
    highlight_drag (spread_table->priv->drop_target, before);

  return FALSE;
}

static gboolean
egg_spread_table_dnd_drag_drop (GtkWidget         *widget,
				GdkDragContext    *context,
				gint               x,
				gint               y,
				guint              time_)
{
  EggSpreadTableDnd *spread_table = EGG_SPREAD_TABLE_DND (widget);

  /* Cancel the drag here too (otherwise the highlight can remain) */
  cancel_drag (spread_table);

  return FALSE;
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
  GtkWidget         *drag_child;

  /* Action time ! now reorder the child to the new position */
  drag_child = get_drag_child (spread_table, data);

  if (drag_child)
    {
      GtkWidget  *drop_widget;
      gint        index = -1, orig_index;
      GList      *children;
      gboolean    before;

      drop_widget = get_child_at_position (spread_table, x, y, &before);

      if (drop_widget)
	{
	  children   = gtk_container_get_children (GTK_CONTAINER (spread_table));
	  index      = g_list_index (children, drop_widget);
	  orig_index = g_list_index (children, drag_child);
	  g_list_free (children);

	  if (index >= 0 && orig_index >= 0 && orig_index < index)
	    index--;

	  if (!before)
	    index++;
	}

      /* It might have been inside another spread table in the same application */
      g_object_ref (drag_child);
      gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (drag_child)), drag_child);
      egg_spread_table_insert_child (EGG_SPREAD_TABLE (spread_table), drag_child, index);
      g_object_unref (drag_child);

    }

  cancel_drag (spread_table);

}

static gboolean
egg_spread_table_dnd_drag_failed (GtkWidget         *widget,
				  GdkDragContext    *context,
				  GtkDragResult      result)
{

  return FALSE;
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

  /* Cancel the drag on the original drag data (incase this is a table-to-table dnd) */
  cancel_drag (data->table);

  return data->child;
}

static void
cancel_drag (EggSpreadTableDnd *spread_table)
{
  if (spread_table->priv->drop_target)
    {
      unhighlight_drag (spread_table->priv->drop_target);
      spread_table->priv->drop_target = NULL;
    }
  spread_table->priv->pressed_button = -1;
}

static GtkWidget *
get_child_at_position (EggSpreadTableDnd *spread_table,
		       gint               x,
		       gint               y,
		       gboolean          *before)
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

	  if (before)
	    {
	      if (y < allocation.y + allocation.height / 2)
		*before = TRUE;
	      else
		*before = FALSE;
	    }
	}
    }

  g_list_free (children);

  return ret_child;
}


static gboolean
drag_highlight_draw (GtkWidget *widget,
		     cairo_t   *cr,
		     gpointer   data)
{
  int width = gtk_widget_get_allocated_width (widget);
  int height = gtk_widget_get_allocated_height (widget);
  int y = 0;
  gboolean before = GPOINTER_TO_INT (data);
  GtkStyleContext *context;

  if (before)
    height = 2;
  else
    {
      y = height - 2;
      height = 2;
    }

  context = gtk_widget_get_style_context (widget);

  gtk_style_context_save (context);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_DND);

  gtk_render_frame (context, cr, 0, y, width, height);

  gtk_style_context_restore (context);

  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0); /* black */
  cairo_set_line_width (cr, 1.0);
  cairo_rectangle (cr,
                   0.5, y + 0.5,
                   width - 1, height - 1);
  cairo_stroke (cr);

  return FALSE;
}

static void 
highlight_drag (GtkWidget  *widget, 
		gboolean    before)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  g_signal_connect_after (widget, "draw",
			  G_CALLBACK (drag_highlight_draw),
			  GINT_TO_POINTER (before));

  gtk_widget_queue_draw (widget);
}

static void 
unhighlight_drag (GtkWidget *widget)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));

  g_signal_handlers_disconnect_by_func (widget,
					drag_highlight_draw,
					GINT_TO_POINTER (TRUE));
  g_signal_handlers_disconnect_by_func (widget,
					drag_highlight_draw,
					GINT_TO_POINTER (FALSE));
  
  gtk_widget_queue_draw (widget);
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
