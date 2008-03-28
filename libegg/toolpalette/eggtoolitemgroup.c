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

#include <gtk/gtk.h>
#include <string.h>

#define ANIMATION_TIMEOUT        50
#define ANIMATION_DURATION      (ANIMATION_TIMEOUT * 4)
#define DEFAULT_EXPANDER_SIZE    16
#define DEFAULT_HEADER_SPACING   2

#define P_(msgid) (msgid)

enum
{
  PROP_NONE,
  PROP_NAME,
  PROP_EXPANED,
};

struct _EggToolItemGroupPrivate
{
  GtkWidget        *header;

  GtkToolItem     **items;
  gsize             items_size;
  gsize             items_length;

  gint64            animation_start;
  GSource          *animation_timeout;
  GtkExpanderStyle  expander_style;
  gint              expander_size;
  gint              header_spacing;

  guint             sparse_items : 1;
  guint             expanded : 1;
};

#ifdef GTK_TYPE_TOOL_SHELL

static void egg_tool_item_group_tool_shell_init (GtkToolShellIface *iface);

G_DEFINE_TYPE_WITH_CODE (EggToolItemGroup, egg_tool_item_group, GTK_TYPE_CONTAINER,
G_IMPLEMENT_INTERFACE (GTK_TYPE_TOOL_SHELL, egg_tool_item_group_tool_shell_init));

#else /* GTK_TYPE_TOOL_SHELL */

#define GTK_TOOL_SHELL(obj)             EGG_TOOL_ITEM_GROUP((obj))

#define GtkToolShell                    EggToolItemGroup

#define gtk_tool_shell_get_orientation  egg_tool_item_group_get_orientation
#define gtk_tool_shell_get_style        egg_tool_item_group_get_style

G_DEFINE_TYPE (EggToolItemGroup, egg_tool_item_group, GTK_TYPE_CONTAINER);

#endif /* GTK_TYPE_TOOL_SHELL */

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

static GtkOrientation
egg_tool_item_group_get_orientation (GtkToolShell *shell)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (shell));

  if (EGG_IS_TOOL_PALETTE (parent))
    return egg_tool_palette_get_orientation (EGG_TOOL_PALETTE (parent));

  return GTK_ORIENTATION_VERTICAL;
}

static GtkToolbarStyle
egg_tool_item_group_get_style (GtkToolShell *shell)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (shell));

  if (EGG_IS_TOOL_PALETTE (parent))
    return egg_tool_palette_get_style (EGG_TOOL_PALETTE (parent));

  return GTK_TOOLBAR_ICONS;
}

#ifdef GTK_TYPE_TOOL_SHELL

static GtkIconSize
egg_tool_item_group_get_icon_size (GtkToolShell *shell)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (shell));

  if (EGG_IS_TOOL_PALETTE (parent))
    return egg_tool_palette_get_icon_size (EGG_TOOL_PALETTE (parent));

  return GTK_ICON_SIZE_SMALL_TOOLBAR;
}

static void
egg_tool_item_group_tool_shell_init (GtkToolShellIface *iface)
{
  iface->get_icon_size = egg_tool_item_group_get_icon_size;
  iface->get_orientation = egg_tool_item_group_get_orientation;
  iface->get_style = egg_tool_item_group_get_style;
}

#endif /* GTK_TYPE_TOOL_SHELL */

static void
egg_tool_item_group_repack (EggToolItemGroup *group)
{
  guint si, di;

  if (group->priv->sparse_items)
    for (si = di = 0; di < group->priv->items_length; ++si)
      {
        if (group->priv->items[si])
          {
            group->priv->items[di] = group->priv->items[si];
            ++di;
          }
        else
          --group->priv->items_length;
      }

  group->priv->sparse_items = FALSE;
}

static gboolean
egg_tool_item_group_header_expose_event_cb (GtkWidget      *widget,
                                            GdkEventExpose *event,
                                            gpointer        data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);
  GtkExpanderStyle expander_style;
  GtkOrientation orientation;
  gint x, y;

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));
  expander_style = group->priv->expander_style;

  if (GTK_ORIENTATION_VERTICAL == orientation)
    {
      x = widget->allocation.x + group->priv->expander_size / 2;
      y = widget->allocation.y + widget->allocation.height / 2;
    }
  else
    {
      x = widget->allocation.x + widget->allocation.width / 2;
      y = widget->allocation.y + group->priv->expander_size / 2;

      /* Unfortunatly gtk_paint_expander() doesn't support rotated drawing
       * modes. Luckily the following shady arithmetics produce the desired
       * result. */
      expander_style = GTK_EXPANDER_EXPANDED - expander_style; /* XXX */
    }

  gtk_paint_expander (widget->style, widget->window,
                      group->priv->header->state,
                      &event->area, GTK_WIDGET (group),
                      "tool-palette-header", x, y,
                      expander_style);

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
egg_tool_item_group_header_adjust_style (EggToolItemGroup *group)
{
  GtkWidget *alignment = egg_tool_item_group_get_alignment (group);
  GtkWidget *label = gtk_bin_get_child (GTK_BIN (alignment));
  GtkWidget *widget = GTK_WIDGET (group);
  gint dx = 0, dy = 0;

  gtk_widget_style_get (widget,
                        "header-spacing", &group->priv->header_spacing,
                        "expander-size", &group->priv->expander_size,
                        NULL);

  switch (gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group)))
    {
      case GTK_ORIENTATION_HORIZONTAL:
        dy = group->priv->header_spacing + group->priv->expander_size;
        gtk_label_set_angle (GTK_LABEL (label), 90);
        break;

      case GTK_ORIENTATION_VERTICAL:
        dx = group->priv->header_spacing + group->priv->expander_size;
        gtk_label_set_angle (GTK_LABEL (label), 0);
        break;
    }

  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), dy, 0, dx, 0);
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

  group->priv->items_length = 0;
  group->priv->items_size = 4;
  group->priv->items = g_new (GtkToolItem*, group->priv->items_size);
  group->priv->header_spacing = DEFAULT_HEADER_SPACING;
  group->priv->expander_size = DEFAULT_EXPANDER_SIZE;
  group->priv->expander_style = GTK_EXPANDER_EXPANDED;
  group->priv->expanded = TRUE;

  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
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

  egg_tool_item_group_header_adjust_style (group);

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
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (object);

  switch (prop_id)
    {
      case PROP_NAME:
        egg_tool_item_group_set_name (group, g_value_get_string (value));
        break;

      case PROP_EXPANED:
        egg_tool_item_group_set_expanded (group, g_value_get_boolean (value));
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
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (object);

  switch (prop_id)
    {
      case PROP_NAME:
        g_value_set_string (value, egg_tool_item_group_get_name (group));
        break;

      case PROP_EXPANED:
        g_value_set_boolean (value, egg_tool_item_group_get_expanded (group));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_tool_item_group_finalize (GObject *object)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (object);

  if (group->priv->items)
    {
      g_free (group->priv->items);
      group->priv->items = NULL;
    }

  G_OBJECT_CLASS (egg_tool_item_group_parent_class)->finalize (object);
}

static void
egg_tool_item_group_get_item_size (EggToolItemGroup *group,
                                   GtkRequisition   *item_size)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (group));

  if (EGG_IS_TOOL_PALETTE (parent))
    _egg_tool_palette_get_item_size (EGG_TOOL_PALETTE (parent), item_size);
  else
    _egg_tool_item_group_item_size_request (group, item_size);
}

static void
egg_tool_item_group_size_request (GtkWidget      *widget,
                                  GtkRequisition *requisition)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);
  GtkOrientation orientation;
  GtkRequisition item_size;

  egg_tool_item_group_repack (group);

  if (group->priv->items_length && egg_tool_item_group_get_name (group))
    {
      gtk_widget_size_request (group->priv->header, requisition);
      gtk_widget_show (group->priv->header);
    }
  else
    {
      requisition->width = requisition->height = 0;
      gtk_widget_hide (group->priv->header);
    }

  egg_tool_item_group_get_item_size (group, &item_size);

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));

  if (GTK_ORIENTATION_VERTICAL == orientation)
    requisition->width = MAX (requisition->width, item_size.width);
  else
    requisition->height = MAX (requisition->height, item_size.height);

  requisition->width += border_width;
  requisition->height += border_width;
}

static gboolean
egg_tool_item_group_is_item_visible (GtkToolItem    *item,
                                     GtkOrientation  orientation)
{
  return
    (GTK_WIDGET_VISIBLE (item)) &&
    (GTK_ORIENTATION_VERTICAL == orientation ?
     gtk_tool_item_get_visible_vertical (item) :
     gtk_tool_item_get_visible_horizontal (item));
}

static void
egg_tool_item_group_real_size_allocate (GtkWidget      *widget,
                                        GtkAllocation  *allocation,
                                        GtkRequisition *inquery)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);
  GtkRequisition child_requistion;
  GtkAllocation child_allocation;

  GtkRequisition item_size;
  GtkAllocation item_area;

  GtkOrientation orientation;
  GtkToolbarStyle style;

  guint n_visible_items, i;

  GTK_WIDGET_CLASS (egg_tool_item_group_parent_class)->size_allocate (widget, allocation);

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));
  style = gtk_tool_shell_get_style (GTK_TOOL_SHELL (group));

  /* figure out item size */

  egg_tool_item_group_get_item_size (group, &item_size);

  if (GTK_ORIENTATION_VERTICAL == orientation)
    item_size.width = MIN (item_size.width, allocation->width);
  else
    item_size.height = MIN (item_size.height, allocation->height);

  n_visible_items = 0;

  for (i = 0; i < group->priv->items_length; ++i)
    {
      GtkToolItem *item = group->priv->items[i];

      if (item && egg_tool_item_group_is_item_visible (item, orientation))
        n_visible_items += 1;
    }

  if (GTK_TOOLBAR_ICONS == style)
    {
      guint n_columns, n_rows;

      if (GTK_ORIENTATION_VERTICAL == orientation)
        {
          n_columns = MAX (allocation->width / item_size.width, 1);
          n_rows = (n_visible_items + n_columns - 1) / n_columns;
        }
      else
        {
          n_rows = MAX (allocation->height / item_size.height, 1);
          n_columns = (n_visible_items + n_rows - 1) / n_rows;
        }

      item_area.width = item_size.width * n_columns;
      item_area.height = item_size.height * n_rows;
    }
  else
    {
      item_area.width = allocation->width;
      item_area.height = allocation->height;
    }

  /* place the header widget */

  child_allocation.x = border_width;
  child_allocation.y = border_width;

  if (GTK_WIDGET_VISIBLE (group->priv->header))
    {
      gtk_widget_size_request (group->priv->header, &child_requistion);

      if (GTK_ORIENTATION_VERTICAL == orientation)
        {
          child_allocation.width = allocation->width;
          child_allocation.height = child_requistion.height;
        }
      else
        {
          child_allocation.width = child_requistion.width;
          child_allocation.height = allocation->height;
        }

      if (!inquery)
        gtk_widget_size_allocate (group->priv->header, &child_allocation);

      if (GTK_ORIENTATION_VERTICAL == orientation)
        child_allocation.y += child_allocation.height;
      else
        child_allocation.x += child_allocation.width;
    }

  item_area.x = child_allocation.x;
  item_area.y = child_allocation.y;

  /* otherwise, when expanded or in transition, place the tool items */

  if (group->priv->expanded || group->priv->animation_timeout)
    {
      for (i = 0; i < group->priv->items_length; ++i)
        {
          GtkToolItem *item = group->priv->items[i];

          if (!item)
            continue;

          if (!egg_tool_item_group_is_item_visible (item, orientation))
            {
              if (!inquery)
                gtk_widget_set_child_visible (GTK_WIDGET (item), FALSE);

              continue;
            }

          if (child_allocation.x + item_size.width > item_area.x + item_area.width)
            {
              child_allocation.y += child_allocation.height;
              child_allocation.x = item_area.x;
            }

          if (style != GTK_TOOLBAR_ICONS || gtk_tool_item_get_expand (item))
            child_allocation.width = item_area.x + item_area.width - child_allocation.x;
          else
            child_allocation.width = item_size.width;

          child_allocation.height = item_size.height;

          if (!inquery)
            {
              gtk_widget_size_allocate (GTK_WIDGET (item), &child_allocation);
              gtk_widget_set_child_visible (GTK_WIDGET (item), TRUE);
            }

          child_allocation.x += child_allocation.width;
        }

      child_allocation.y += item_size.height;
      child_allocation.x = border_width;
    }

  /* or just hide all items, when collapsed */

  else if (!inquery)
    {
      for (i = 0; i < group->priv->items_length; ++i)
        if (group->priv->items[i])
          gtk_widget_set_child_visible (GTK_WIDGET (group->priv->items[i]), FALSE);
    }

  /* report effective widget size */

  if (inquery)
    {
      inquery->width = item_area.x + item_area.width + border_width;
      inquery->height = child_allocation.y + border_width;
    }
}

static void
egg_tool_item_group_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
  egg_tool_item_group_real_size_allocate (widget, allocation, NULL);

  if (GTK_WIDGET_MAPPED (widget))
    gdk_window_invalidate_rect (widget->window, NULL, FALSE);
}

static void
egg_tool_item_group_realize (GtkWidget *widget)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  GdkWindowAttr attributes;
  GdkDisplay *display;

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

  display = gdk_drawable_get_display (widget->window);

  if (gdk_display_supports_composite (display))
    gdk_window_set_composited (widget->window, TRUE);

  gdk_window_set_user_data (widget->window, widget);
  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  gtk_container_forall (GTK_CONTAINER (widget),
                        (GtkCallback) gtk_widget_set_parent_window,
                        widget->window);

  gtk_widget_queue_resize_no_redraw (widget);
}

static void
egg_tool_item_group_style_set (GtkWidget *widget,
                               GtkStyle  *previous_style)
{
  egg_tool_item_group_header_adjust_style (EGG_TOOL_ITEM_GROUP (widget));
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
                            GtkWidget    *child)
{
  EggToolItemGroup *group;
  guint i;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (container));
  group = EGG_TOOL_ITEM_GROUP (container);

  for (i = 0; i < group->priv->items_length; ++i)
    if ((GtkWidget*) group->priv->items[i] == child)
      {
        g_object_unref (child);
        gtk_widget_unparent (child);
        group->priv->items[i] = NULL;
        group->priv->sparse_items = TRUE;
      }
}

static void
egg_tool_item_group_forall (GtkContainer *container,
                            gboolean      internals,
                            GtkCallback   callback,
                            gpointer      callback_data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (container);
  guint i;

  if (internals && group->priv->header)
    callback (group->priv->header, callback_data);

  if (NULL != group->priv->items)
    for (i = 0; i < group->priv->items_length; ++i)
      {
        GtkToolItem *item = group->priv->items[i];

        if (item)
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
      egg_tool_item_group_repack (group);

      if (name && group->priv->items_length)
        gtk_widget_show (group->priv->header);
      else
        gtk_widget_hide (group->priv->header);

      g_object_notify (G_OBJECT (group), "name");
    }
}

static gint64
egg_tool_item_group_get_animation_timestamp (EggToolItemGroup *group)
{
  GTimeVal now;
  g_source_get_current_time (group->priv->animation_timeout, &now);
  return (now.tv_sec * G_USEC_PER_SEC + now.tv_usec - group->priv->animation_start) / 1000;
}

static gboolean
egg_tool_item_group_animation_cb (gpointer data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);
  gint64 timestamp = egg_tool_item_group_get_animation_timestamp (group);

  /* enque this early to reduce number of expose events */
  gtk_widget_queue_resize_no_redraw (GTK_WIDGET (group));

  if (GTK_WIDGET_REALIZED (group->priv->header))
    {
      GtkWidget *alignment = egg_tool_item_group_get_alignment (group);
      GdkRectangle area;

      area.x = alignment->allocation.x;
      area.y = alignment->allocation.y + (alignment->allocation.height - group->priv->expander_size) / 2;
      area.height = group->priv->expander_size;
      area.width = group->priv->expander_size;

      gdk_window_invalidate_rect (group->priv->header->window, &area, TRUE);
    }

  if (GTK_WIDGET_REALIZED (group))
    {
      GtkWidget *widget = GTK_WIDGET (group);
      GtkWidget *parent = gtk_widget_get_parent (widget);

      gint width = widget->allocation.width;
      gint height = widget->allocation.height;
      gint x, y;

      gtk_widget_translate_coordinates (widget, parent, 0, 0, &x, &y);

      if (GTK_WIDGET_VISIBLE (group->priv->header))
        {
          height -= group->priv->header->allocation.height;
          y += group->priv->header->allocation.height;
        }

      gtk_widget_queue_draw_area (parent, x, y, width, height);
    }

  if (group->priv->expanded)
    {
      if (group->priv->expander_style == GTK_EXPANDER_COLLAPSED)
        group->priv->expander_style = GTK_EXPANDER_SEMI_EXPANDED;
      else
        group->priv->expander_style = GTK_EXPANDER_EXPANDED;
    }
  else
    {
      if (group->priv->expander_style == GTK_EXPANDER_EXPANDED)
        group->priv->expander_style = GTK_EXPANDER_SEMI_COLLAPSED;
      else
        group->priv->expander_style = GTK_EXPANDER_COLLAPSED;
    }

  if (timestamp >= ANIMATION_DURATION)
    group->priv->animation_timeout = NULL;

  /* Ensure that all composited windows and child windows are repainted, before
   * the parent widget gets its expose-event. This is needed to avoid heavy
   * rendering artifacts. GTK+ should take care about this issue by itself I
   * guess, but currently it doesn't. Also I don't understand the parameters
   * of this issue well enough yet, to file a bug report.
   */
  gdk_window_process_updates (GTK_WIDGET (group)->window, TRUE);

  return (group->priv->animation_timeout != NULL);
}

void
egg_tool_item_group_set_expanded (EggToolItemGroup *group,
                                  gboolean          expanded)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  if (expanded != group->priv->expanded)
    {
      GTimeVal now;

      g_get_current_time (&now);

      if (group->priv->animation_timeout)
        g_source_destroy (group->priv->animation_timeout);

      group->priv->expanded = expanded;
      group->priv->animation_start = (now.tv_sec * G_USEC_PER_SEC + now.tv_usec);
      group->priv->animation_timeout = g_timeout_source_new (ANIMATION_TIMEOUT);

      g_source_set_callback (group->priv->animation_timeout,
                             egg_tool_item_group_animation_cb,
                             group, NULL);

      g_source_attach (group->priv->animation_timeout, NULL);
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

  g_return_if_fail ((guint) position <= egg_tool_item_group_get_n_items (group));

  if (group->priv->items_length == group->priv->items_size)
    {
      group->priv->items_size *= 2;
      group->priv->items = g_renew (GtkToolItem*,
                                    group->priv->items,
                                    group->priv->items_size);
    }

  memmove (group->priv->items + position + 1, group->priv->items + position,
           sizeof (GtkToolItem*) * (group->priv->items_length - position));

  group->priv->items[position] = g_object_ref_sink (item);
  group->priv->items_length += 1;

  if (EGG_IS_TOOL_PALETTE (parent))
    _egg_tool_palette_child_set_drag_source (GTK_WIDGET (item), parent);

  gtk_widget_set_parent (GTK_WIDGET (item), GTK_WIDGET (group));
}

void
egg_tool_item_group_set_item_position (EggToolItemGroup *group,
                                       GtkToolItem      *item,
                                       gint              position)
{
  gint old_position;
  gpointer src, dst;
  gsize len;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));

  egg_tool_item_group_repack (group);

  g_return_if_fail (position >= -1);

  if (-1 == position)
    position = group->priv->items_length - 1;

  g_return_if_fail ((guint) position < group->priv->items_length);

  if (item == group->priv->items[position])
    return;

  old_position = egg_tool_item_group_get_item_position (group, item);
  g_return_if_fail (old_position >= 0);

  if (position < old_position)
    {
      dst = group->priv->items + position + 1;
      src = group->priv->items + position;
      len = old_position - position;
    }
  else
    {
      dst = group->priv->items + old_position;
      src = group->priv->items + old_position + 1;
      len = position - old_position;
    }

  memmove (dst, src, len * sizeof (*group->priv->items));
  group->priv->items[position] = item;

  gtk_widget_queue_resize (GTK_WIDGET (group));
}

gint
egg_tool_item_group_get_item_position (EggToolItemGroup *group,
                                       GtkToolItem      *item)
{
  guint i;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), -1);
  g_return_val_if_fail (GTK_IS_TOOL_ITEM (item), -1);

  egg_tool_item_group_repack (group);

  for (i = 0; i < group->priv->items_length; ++i)
    if (item == group->priv->items[i])
      return i;

  return -1;
}

guint
egg_tool_item_group_get_n_items (EggToolItemGroup *group)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), 0);
  egg_tool_item_group_repack (group);
  return group->priv->items_length;
}

GtkToolItem*
egg_tool_item_group_get_nth_item (EggToolItemGroup *group,
                                  guint             index)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);

  egg_tool_item_group_repack (group);

  g_return_val_if_fail (index < group->priv->items_length, NULL);

  return group->priv->items[index];
}

GtkToolItem*
egg_tool_item_group_get_drop_item (EggToolItemGroup *group,
                                   gint              x,
                                   gint              y)
{
  GtkAllocation *allocation;
  GtkOrientation orientation;
  guint i;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);

  allocation = &GTK_WIDGET (group)->allocation;
  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));

  g_return_val_if_fail (x >= 0 && x < allocation->width, NULL);
  g_return_val_if_fail (y >= 0 && y < allocation->height, NULL);

  for (i = 0; i < group->priv->items_length; ++i)
    {
      GtkToolItem *item = group->priv->items[i];
      gint x0, y0;

      if (!item || !egg_tool_item_group_is_item_visible (item, orientation))
        continue;

      allocation = &GTK_WIDGET (item)->allocation;

      x0 = x - allocation->x;
      y0 = y - allocation->y;

      if (x0 >= 0 && x0 < allocation->width &&
          y0 >= 0 && y0 < allocation->height)
        return item;
    }

  return NULL;
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

  for (i = 0; i < group->priv->items_length; ++i)
    {
      GtkToolItem *item = group->priv->items[i];

      if (!item)
        continue;

      gtk_widget_size_request (GTK_WIDGET (item), &child_requisition);

      item_size->width = MAX (item_size->width, child_requisition.width);
      item_size->height = MAX (item_size->height, child_requisition.height);
    }
}

void
_egg_tool_item_group_paint (EggToolItemGroup *group,
                            cairo_t          *cr)
{
  GtkWidget *widget = GTK_WIDGET (group);

  gdk_cairo_set_source_pixmap (cr, widget->window,
                               widget->allocation.x,
                               widget->allocation.y);

  if (group->priv->animation_timeout)
    {
      GtkOrientation orientation = egg_tool_item_group_get_orientation (GTK_TOOL_SHELL (group));
      cairo_pattern_t *mask;
      gdouble v0, v1;

      if (GTK_ORIENTATION_VERTICAL == orientation)
        v1 = widget->allocation.height;
      else
        v1 = widget->allocation.width;

      v0 = v1 - 256;

      if (!GTK_WIDGET_VISIBLE (group->priv->header))
        v0 = MAX (v0, 0);
      else if (GTK_ORIENTATION_VERTICAL == orientation)
        v0 = MAX (v0, group->priv->header->allocation.height);
      else
        v0 = MAX (v0, group->priv->header->allocation.width);

      v1 = MIN (v0 + 256, v1);

      if (GTK_ORIENTATION_VERTICAL == orientation)
        {
          v0 += widget->allocation.y;
          v1 += widget->allocation.y;

          mask = cairo_pattern_create_linear (0.0, v0, 0.0, v1);
        }
      else
        {
          v0 += widget->allocation.x;
          v1 += widget->allocation.x;

          mask = cairo_pattern_create_linear (v0, 0.0, v1, 0.0);
        }

      cairo_pattern_add_color_stop_rgba (mask, 0.00, 0.0, 0.0, 0.0, 1.00);
      cairo_pattern_add_color_stop_rgba (mask, 0.25, 0.0, 0.0, 0.0, 0.25);
      cairo_pattern_add_color_stop_rgba (mask, 0.50, 0.0, 0.0, 0.0, 0.10);
      cairo_pattern_add_color_stop_rgba (mask, 0.75, 0.0, 0.0, 0.0, 0.01);
      cairo_pattern_add_color_stop_rgba (mask, 1.00, 0.0, 0.0, 0.0, 0.00);

      cairo_mask (cr, mask);
      cairo_pattern_destroy (mask);
    }
  else
    cairo_paint (cr);
}

static gint
egg_tool_item_group_get_size_for_limit (EggToolItemGroup *group,
                                        gint              limit,
                                        gboolean          vertical)
{
  GtkRequisition requisition;

  egg_tool_item_group_repack (group);
  gtk_widget_size_request (GTK_WIDGET (group), &requisition);

  if (group->priv->expanded || group->priv->animation_timeout)
    {
      GtkAllocation allocation = { 0, 0, requisition.width, requisition.height };
      GtkRequisition inquery;

      if (vertical)
        allocation.width = limit;
      else
        allocation.height = limit;

      egg_tool_item_group_real_size_allocate (GTK_WIDGET (group),
                                              &allocation, &inquery);

      if (vertical)
        inquery.height -= requisition.height;
      else
        inquery.width -= requisition.width;

      if (group->priv->animation_timeout)
        {
          gint64 timestamp = egg_tool_item_group_get_animation_timestamp (group);

          timestamp = MIN (timestamp, ANIMATION_DURATION);

          if (!group->priv->expanded)
            timestamp = ANIMATION_DURATION - timestamp;

          if (vertical)
            {
              inquery.height *= timestamp;
              inquery.height /= ANIMATION_DURATION;
            }
          else
            {
              inquery.width *= timestamp;
              inquery.width /= ANIMATION_DURATION;
            }
        }

      if (vertical)
        requisition.height += inquery.height;
      else
        requisition.width += inquery.width;
    }

  return (vertical ? requisition.height : requisition.width);
}

gint
_egg_tool_item_group_get_height_for_width (EggToolItemGroup *group,
                                           gint              width)
{
  return egg_tool_item_group_get_size_for_limit (group, width, TRUE);
}

gint
_egg_tool_item_group_get_width_for_height (EggToolItemGroup *group,
                                           gint              height)
{
  return egg_tool_item_group_get_size_for_limit (group, height, FALSE);
}

#ifdef GTK_TYPE_TOOL_SHELL

static void
egg_tool_palette_reconfigured_foreach_item (GtkWidget *child,
                                            gpointer   data G_GNUC_UNUSED)
{
  if (GTK_IS_TOOL_ITEM (child))
    gtk_tool_item_toolbar_reconfigured (GTK_TOOL_ITEM (child));
}

#endif /* GTK_TYPE_TOOL_SHELL */

void
_egg_tool_item_group_palette_reconfigured (EggToolItemGroup *group)
{
#ifdef GTK_TYPE_TOOL_SHELL

  gtk_container_foreach (GTK_CONTAINER (group),
                         egg_tool_palette_reconfigured_foreach_item,
                         NULL);

#endif /* GTK_TYPE_TOOL_SHELL */

  egg_tool_item_group_header_adjust_style (group);
}

