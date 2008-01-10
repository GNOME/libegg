/* EggToolPalette -- A tool palette with categories and DnD support
 * Copyright (C) 2008  Openismus GmbH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authors:
 *      Mathias Hasselmann
 */

#include "eggtoolitemgroup.h"
#include "eggtoolpaletteprivate.h"

#include <glib/gi18n.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>

#include <string.h>

#define DEFAULT_EXPANDER_SIZE   16
#define DEFAULT_HEADER_SPACING  2

#define P_(msgid) _(msgid)

enum
{
  PROP_NONE,
  PROP_NAME,
  PROP_EXPANED,
};

struct _EggToolItemGroupPrivate
{
  GtkWidget        *header;
  GArray           *items;

  guint             animation_timeout;
  GtkExpanderStyle  expander_style;
  gint              expander_size;
  gint              header_spacing;

  guint             expanded : 1;
};

G_DEFINE_TYPE (EggToolItemGroup,
               egg_tool_item_group,
               GTK_TYPE_CONTAINER);

static gboolean
egg_tool_item_group_header_expose_event_cb (GtkWidget      *widget,
                                            GdkEventExpose *event,
                                            gpointer        data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);

  gint x = widget->allocation.x + group->priv->expander_size / 2;
  gint y = widget->allocation.y + widget->allocation.height / 2;

  gtk_paint_expander (widget->style, widget->window,
                      group->priv->header->state,
                      &event->area, GTK_WIDGET (group),
                      "tool-palette-header", x, y,
                      group->priv->expander_style);

  return FALSE;
}

static void
egg_tool_item_group_header_size_request_cb (GtkWidget      *widget G_GNUC_UNUSED,
                                            GtkRequisition *requisition,
                                            gpointer        data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);
  requisition->height = MAX (requisition->height, group->priv->expander_size);
}

static void
egg_tool_item_group_header_clicked_cb (GtkButton *button G_GNUC_UNUSED,
                                       gpointer   data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);
  egg_tool_item_group_set_expanded (group, !group->priv->expanded);
}

static void
egg_tool_item_group_init (EggToolItemGroup *group)
{
  GtkWidget *alignment;
  GtkWidget *label;

  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (group), FALSE);

  group->priv = G_TYPE_INSTANCE_GET_PRIVATE (group,
                                            EGG_TYPE_TOOL_ITEM_GROUP,
                                            EggToolItemGroupPrivate);

  group->priv->items = g_array_new (TRUE, TRUE, sizeof (GtkToolItem*));
  group->priv->header_spacing = DEFAULT_HEADER_SPACING;
  group->priv->expander_size = DEFAULT_EXPANDER_SIZE;
  group->priv->expander_style = GTK_EXPANDER_EXPANDED;
  group->priv->expanded = TRUE;

  label = gtk_label_new (NULL);
  alignment = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);

  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0,
                             group->priv->header_spacing +
                             group->priv->expander_size, 0);

  gtk_container_add (GTK_CONTAINER (alignment), label);
  gtk_widget_show_all (alignment);

  gtk_widget_push_composite_child ();
  group->priv->header = gtk_button_new ();
  gtk_widget_set_composite_name (group->priv->header, "header");
  gtk_widget_pop_composite_child ();

  g_object_ref_sink (group->priv->header);
  gtk_button_set_focus_on_click (GTK_BUTTON (group->priv->header), FALSE);
  gtk_container_add (GTK_CONTAINER (group->priv->header), alignment);
  gtk_widget_set_parent (group->priv->header, GTK_WIDGET (group));

  g_signal_connect_after (alignment, "expose-event",
                          G_CALLBACK (egg_tool_item_group_header_expose_event_cb),
                          group);
  g_signal_connect_after (alignment, "size-request",
                          G_CALLBACK (egg_tool_item_group_header_size_request_cb),
                          group);

  g_signal_connect (group->priv->header, "clicked",
                    G_CALLBACK (egg_tool_item_group_header_clicked_cb),
                    group);
}

static void
egg_tool_item_group_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  EggToolItemGroup *self = EGG_TOOL_ITEM_GROUP (object);

  switch (prop_id)
    {
      case PROP_NAME:
        egg_tool_item_group_set_name (self, g_value_get_string (value));
        break;

      case PROP_EXPANED:
        egg_tool_item_group_set_expanded (self, g_value_get_boolean (value));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_tool_item_group_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  EggToolItemGroup *self = EGG_TOOL_ITEM_GROUP (object);

  switch (prop_id)
    {
      case PROP_NAME:
        g_value_set_string (value, egg_tool_item_group_get_name (self));
        break;

      case PROP_EXPANED:
        g_value_set_boolean (value, egg_tool_item_group_get_expanded (self));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_tool_item_group_finalize (GObject *object)
{
  EggToolItemGroup *self = EGG_TOOL_ITEM_GROUP (object);

  if (self->priv->items)
    {
      g_array_free (self->priv->items, TRUE);
      self->priv->items = NULL;
    }

  G_OBJECT_CLASS (egg_tool_item_group_parent_class)->finalize (object);
}

static void
egg_tool_item_group_size_request (GtkWidget      *widget,
                                  GtkRequisition *requisition)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *self = EGG_TOOL_ITEM_GROUP (widget);

  if (self->priv->items->len && egg_tool_item_group_get_name (self))
    {
      gtk_widget_size_request (self->priv->header, requisition);
      gtk_widget_show (self->priv->header);
    }
  else
    {
      requisition->width = requisition->height = 0;
      gtk_widget_hide (self->priv->header);
    }

  requisition->width += border_width;
  requisition->height += border_width;
}

static void
egg_tool_item_group_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);
  GtkRequisition child_requistion;
  GtkAllocation child_allocation;
  GtkRequisition item_size;
  GtkWidget *parent;
  guint i;

  GTK_WIDGET_CLASS (egg_tool_item_group_parent_class)->size_allocate (widget, allocation);

  parent = gtk_widget_get_parent (widget);

  if (EGG_IS_TOOL_PALETTE (parent))
    _egg_tool_palette_get_item_size (EGG_TOOL_PALETTE (parent), &item_size);
  else
    _egg_tool_item_group_item_size_request (group, &item_size);

  child_allocation.x = border_width;
  child_allocation.y = border_width;

  if (GTK_WIDGET_VISIBLE (group->priv->header))
    {
      gtk_widget_size_request (group->priv->header, &child_requistion);

      child_allocation.width = allocation->width;
      child_allocation.height = child_requistion.height;

      gtk_widget_size_allocate (group->priv->header, &child_allocation);

      child_allocation.y += child_allocation.height;
    }

    if (group->priv->expanded)
      {
        for (i = 0; i < group->priv->items->len; ++i)
          {
            GtkToolItem *item = egg_tool_item_group_get_nth_item (group, i);

            child_allocation.width = item_size.width;
            child_allocation.height = item_size.height;

            if (child_allocation.x + child_allocation.width > allocation->width)
              {
                child_allocation.y += child_allocation.height;
                child_allocation.x = border_width;
              }

            gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);
            gtk_widget_show (GTK_WIDGET (item));

            child_allocation.x += child_allocation.width;
          }

        child_allocation.y += item_size.height;
        child_allocation.x = border_width;
      }
    else
      {
        for (i = 0; i < group->priv->items->len; ++i)
          {
            GtkToolItem *item = egg_tool_item_group_get_nth_item (group, i);
            gtk_widget_hide (GTK_WIDGET (item));
          }
      }

  if (GTK_WIDGET_MAPPED (widget))
    gdk_window_invalidate_rect (widget->window, NULL, FALSE);
}

static void
egg_tool_item_group_realize (GtkWidget *widget)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  GdkWindowAttr attributes;

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x + border_width;
  attributes.y = widget->allocation.y + border_width;
  attributes.width = widget->allocation.width - border_width * 2;
  attributes.height = widget->allocation.height - border_width * 2;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK | GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                        | GDK_BUTTON_MOTION_MASK;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                   &attributes, attributes_mask);

  gdk_window_set_user_data (widget->window, widget);
  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  gtk_container_forall (GTK_CONTAINER (widget),
                        (GtkCallback) gtk_widget_set_parent_window,
                        widget->window);

  gtk_widget_queue_resize_no_redraw (widget);
}

static GtkWidget*
egg_tool_item_group_get_alignment (EggToolItemGroup *group)
{
  return gtk_bin_get_child (GTK_BIN (group->priv->header));
}

static GtkWidget*
egg_tool_item_group_get_label (EggToolItemGroup *group)
{
  GtkWidget *alignment = egg_tool_item_group_get_alignment (group);
  return gtk_bin_get_child (GTK_BIN (alignment));
}

static void
egg_tool_item_group_style_set (GtkWidget *widget,
                               GtkStyle  *previous_style)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);
  GtkWidget *alignment = NULL;

  gtk_widget_style_get (widget,
                        "header-spacing", &group->priv->header_spacing,
                        "expander-size", &group->priv->expander_size,
                        NULL);

  alignment = egg_tool_item_group_get_alignment (group);

  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment),
                             0, 0,
                             group->priv->header_spacing +
                             group->priv->expander_size, 0);

  GTK_WIDGET_CLASS (egg_tool_item_group_parent_class)->style_set (widget, previous_style);
}

static void
egg_tool_item_group_add (GtkContainer *container,
                         GtkWidget    *widget)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (container));
  g_return_if_fail (GTK_IS_TOOL_ITEM (widget));

  egg_tool_item_group_insert (EGG_TOOL_ITEM_GROUP (container),
                              GTK_TOOL_ITEM (widget), -1);
}

static void
egg_tool_item_group_remove (GtkContainer *container,
                            GtkWidget    *widget G_GNUC_UNUSED)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (container));
  g_return_if_reached ();
}

static void
egg_tool_item_group_forall (GtkContainer *container,
                            gboolean      internals,
                            GtkCallback   callback,
                            gpointer      callback_data)
{
  EggToolItemGroup *self = EGG_TOOL_ITEM_GROUP (container);
  guint i;

  if (internals && self->priv->header)
    callback (self->priv->header, callback_data);

  if (NULL != self->priv->items)
    for (i = 0; i < self->priv->items->len; ++i)
      {
        GtkToolItem *item = egg_tool_item_group_get_nth_item (self, i);
        callback (GTK_WIDGET (item), callback_data);
      }
}

static GType
egg_tool_item_group_child_type (GtkContainer *container G_GNUC_UNUSED)
{
  return GTK_TYPE_TOOL_ITEM;
}

static void
egg_tool_item_group_class_init (EggToolItemGroupClass *cls)
{
  GObjectClass *oclass = G_OBJECT_CLASS (cls);
  GtkWidgetClass *wclass = GTK_WIDGET_CLASS (cls);
  GtkContainerClass *cclass = GTK_CONTAINER_CLASS (cls);

  oclass->set_property  = egg_tool_item_group_set_property;
  oclass->get_property  = egg_tool_item_group_get_property;
  oclass->finalize      = egg_tool_item_group_finalize;

  wclass->size_request  = egg_tool_item_group_size_request;
  wclass->size_allocate = egg_tool_item_group_size_allocate;
  wclass->realize       = egg_tool_item_group_realize;
  wclass->style_set     = egg_tool_item_group_style_set;

  cclass->add           = egg_tool_item_group_add;
  cclass->remove        = egg_tool_item_group_remove;
  cclass->forall        = egg_tool_item_group_forall;
  cclass->child_type    = egg_tool_item_group_child_type;

  g_object_class_install_property (oclass, PROP_NAME,
                                   g_param_spec_string ("name",
                                                        P_("Name"),
                                                        P_("The name of this item group"),
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (oclass, PROP_EXPANED,
                                   g_param_spec_boolean ("expanded",
                                                         P_("Expanded"),
                                                         P_("Wether the group has been expanded and items are shown"),
                                                         TRUE,
                                                         G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                         G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_widget_class_install_style_property (wclass,
                                           g_param_spec_int ("expander-size",
                                                             P_("Expander Size"),
                                                             P_("Size of the expander arrow"),
                                                             0,
                                                             G_MAXINT,
                                                             DEFAULT_EXPANDER_SIZE,
                                                             G_PARAM_READABLE | G_PARAM_STATIC_NAME |
                                                             G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_widget_class_install_style_property (wclass,
                                           g_param_spec_int ("header-spacing",
                                                             P_("Header Spacing"),
                                                             P_("Spacing between expander arrow and caption"),
                                                             0,
                                                             G_MAXINT,
                                                             DEFAULT_HEADER_SPACING,
                                                             G_PARAM_READABLE | G_PARAM_STATIC_NAME |
                                                             G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_type_class_add_private (cls, sizeof (EggToolItemGroupPrivate));
}

GtkWidget*
egg_tool_item_group_new (const gchar *name)
{
  return g_object_new (EGG_TYPE_TOOL_ITEM_GROUP, "name", name, NULL);
}

void
egg_tool_item_group_set_name (EggToolItemGroup *group,
                              const gchar      *name)
{
  const gchar *current_name;
  GtkWidget *label;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  current_name = egg_tool_item_group_get_name (group);

  if (current_name != name && (!current_name || !name || strcmp (current_name, name)))
    {
      label = egg_tool_item_group_get_label (group);
      gtk_label_set_text (GTK_LABEL (label), name);

      if (name && group->priv->items->len)
        gtk_widget_show (group->priv->header);
      else
        gtk_widget_hide (group->priv->header);

      g_object_notify (G_OBJECT (group), "name");
    }
}

static gboolean
egg_tool_item_group_animation_cb (gpointer data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);
  gboolean finish = TRUE;
  GdkRectangle area;

  if (GTK_WIDGET_REALIZED (group->priv->header))
    {
      GtkWidget *alignment = egg_tool_item_group_get_alignment (group);

      area.x = alignment->allocation.x;
      area.y = alignment->allocation.y + (alignment->allocation.height - group->priv->expander_size) / 2;
      area.height = group->priv->expander_size;
      area.width = group->priv->expander_size;

      gdk_window_invalidate_rect (group->priv->header->window, &area, TRUE);
    }

  if (group->priv->expanded)
    {
      if (group->priv->expander_style == GTK_EXPANDER_COLLAPSED)
        {
          group->priv->expander_style = GTK_EXPANDER_SEMI_EXPANDED;
          finish = FALSE;
        }
      else
        group->priv->expander_style = GTK_EXPANDER_EXPANDED;
    }
  else
    {
      if (group->priv->expander_style == GTK_EXPANDER_EXPANDED)
        {
          group->priv->expander_style = GTK_EXPANDER_SEMI_COLLAPSED;
          finish = FALSE;
        }
      else
        group->priv->expander_style = GTK_EXPANDER_COLLAPSED;
    }

  if (finish)
    {
      group->priv->animation_timeout = 0;
      gtk_widget_queue_resize_no_redraw (GTK_WIDGET (group));
    }

  return !finish;
}

void
egg_tool_item_group_set_expanded (EggToolItemGroup *group,
                                  gboolean          expanded)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  if (expanded != group->priv->expanded)
    {
      if (group->priv->animation_timeout)
        g_source_remove (group->priv->animation_timeout);

      group->priv->expanded = expanded;
      group->priv->animation_timeout = g_timeout_add (50, egg_tool_item_group_animation_cb, group);

      g_object_notify (G_OBJECT (group), "expanded");
    }
}

G_CONST_RETURN gchar*
egg_tool_item_group_get_name (EggToolItemGroup *group)
{
  GtkWidget *label;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);

  label = egg_tool_item_group_get_label (group);
  return gtk_label_get_text (GTK_LABEL (label));
}

gboolean
egg_tool_item_group_get_expanded (EggToolItemGroup *group)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), FALSE);
  return group->priv->expanded;
}

void
egg_tool_item_group_insert (EggToolItemGroup *group,
                            GtkToolItem      *item,
                            gint              position)
{
  GtkWidget *parent;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));
  g_return_if_fail (position >= -1);

  parent = gtk_widget_get_parent (GTK_WIDGET (group));

  if (-1 == position)
    position = egg_tool_item_group_get_n_items (group);

  g_return_if_fail (position <= egg_tool_item_group_get_n_items (group));
  g_array_insert_val (group->priv->items, position, item);
  g_object_ref_sink (item);

  if (EGG_IS_TOOL_PALETTE (parent))
    _egg_tool_palette_item_set_drag_source (GTK_WIDGET (item), parent);

  gtk_widget_set_parent (GTK_WIDGET (item), GTK_WIDGET (group));
}

void
egg_tool_item_group_set_item_position (EggToolItemGroup *group,
                                       GtkToolItem      *item,
                                       guint             position)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));

  g_return_if_fail (position < (guint) egg_tool_item_group_get_n_items (group));
  g_return_if_reached ();
}

gint
egg_tool_item_group_get_item_position (EggToolItemGroup *group,
                                       GtkToolItem      *item)
{
  guint i;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), -1);
  g_return_val_if_fail (GTK_IS_TOOL_ITEM (item), -1);

  for (i = 0; i < group->priv->items->len; ++i)
    if (item == egg_tool_item_group_get_nth_item (group, i))
      return i;

  return -1;
}

gint
egg_tool_item_group_get_n_items (EggToolItemGroup *group)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), 0);
  return group->priv->items->len;
}

GtkToolItem*
egg_tool_item_group_get_nth_item (EggToolItemGroup *group,
                                  gint              index)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);
  g_return_val_if_fail (index < egg_tool_item_group_get_n_items (group), NULL);
  g_return_val_if_fail (index >= 0, NULL);

  return g_array_index (group->priv->items, GtkToolItem*, index);
}

GtkToolItem*
egg_tool_item_group_get_drop_item (EggToolItemGroup *group,
                                   gint              x,
                                   gint              y)
{
  GtkAllocation *allocation;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);

  allocation = &GTK_WIDGET (group)->allocation;

  g_return_val_if_fail (x >= 0 && x < allocation->width, NULL);
  g_return_val_if_fail (y >= 0 && y < allocation->width, NULL);

  g_return_val_if_reached (NULL);
}

void
_egg_tool_item_group_item_size_request (EggToolItemGroup *group,
                                        GtkRequisition   *item_size)
{
  GtkRequisition child_requisition;
  guint i;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (NULL != item_size);

  item_size->width = item_size->height = 0;

  for (i = 0; i < group->priv->items->len; ++i)
    {
      GtkToolItem *item = egg_tool_item_group_get_nth_item (group, i);

      gtk_widget_size_request (GTK_WIDGET (item), &child_requisition);

      item_size->width = MAX (item_size->width, child_requisition.width);
      item_size->height = MAX (item_size->height, child_requisition.height);
    }
}
