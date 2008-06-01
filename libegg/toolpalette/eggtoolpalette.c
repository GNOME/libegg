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

#include "eggtoolpalette.h"
#include "eggtoolpaletteprivate.h"
#include "eggtoolitemgroup.h"
#include "eggmarshalers.h"

#include <gtk/gtk.h>
#include <string.h>

#define DEFAULT_ICON_SIZE       GTK_ICON_SIZE_SMALL_TOOLBAR
#define DEFAULT_ORIENTATION     GTK_ORIENTATION_VERTICAL
#define DEFAULT_TOOLBAR_STYLE   GTK_TOOLBAR_ICONS

#define DEFAULT_CHILD_EXCLUSIVE FALSE
#define DEFAULT_CHILD_EXPAND    FALSE

#define P_(msgid) (msgid)

typedef struct _EggToolItemGroupInfo   EggToolItemGroupInfo;
typedef struct _EggToolPaletteDragData EggToolPaletteDragData;

enum
{
  PROP_NONE,
  PROP_ICON_SIZE,
  PROP_ORIENTATION,
  PROP_TOOLBAR_STYLE,
};

enum
{
  CHILD_PROP_NONE,
  CHILD_PROP_EXCLUSIVE,
  CHILD_PROP_EXPAND,
};

struct _EggToolItemGroupInfo
{
  EggToolItemGroup *widget;

  guint             notify_collapsed;
  guint             exclusive : 1;
  guint             expand : 1;
};

struct _EggToolPalettePrivate
{
  EggToolItemGroupInfo *groups;
  gsize                 groups_size;
  gsize                 groups_length;

  GtkAdjustment        *hadjustment;
  GtkAdjustment        *vadjustment;

  GtkRequisition        item_size;
  GtkIconSize           icon_size;
  GtkOrientation        orientation;
  GtkToolbarStyle       style;

  GtkWidget            *expanding_child;

#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
  GtkSizeGroup         *text_size_group;
#endif

  guint                 sparse_groups : 1;
  guint                 drag_source : 1;
};

struct _EggToolPaletteDragData
{
  EggToolPalette *palette;
  GtkWidget      *item;
};

static GdkAtom dnd_target_atom_item = GDK_NONE;
static GdkAtom dnd_target_atom_group = GDK_NONE;

static const GtkTargetEntry dnd_targets[] =
{
  { "application/x-egg-tool-palette-item", GTK_TARGET_SAME_APP, 0 },
  { "application/x-egg-tool-palette-group", GTK_TARGET_SAME_APP, 0 },
};

G_DEFINE_TYPE (EggToolPalette,
               egg_tool_palette,
               GTK_TYPE_CONTAINER);

static void
egg_tool_palette_init (EggToolPalette *palette)
{
  palette->priv = G_TYPE_INSTANCE_GET_PRIVATE (palette,
                                               EGG_TYPE_TOOL_PALETTE,
                                               EggToolPalettePrivate);

  palette->priv->groups_size = 4;
  palette->priv->groups_length = 0;
  palette->priv->groups = g_new0 (EggToolItemGroupInfo, palette->priv->groups_size);

  palette->priv->icon_size = DEFAULT_ICON_SIZE;
  palette->priv->orientation = DEFAULT_ORIENTATION;
  palette->priv->style = DEFAULT_TOOLBAR_STYLE;

#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
  palette->priv->text_size_group = gtk_size_group_new (GTK_SIZE_GROUP_BOTH);
#endif
}

static void
egg_tool_palette_reconfigured (EggToolPalette *palette)
{
  guint i;

  for (i = 0; i < palette->priv->groups_length; ++i)
    {
      if (palette->priv->groups[i].widget)
        _egg_tool_item_group_palette_reconfigured (palette->priv->groups[i].widget);
    }

  gtk_widget_queue_resize_no_redraw (GTK_WIDGET (palette));
}

static void
egg_tool_palette_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (object);

  switch (prop_id)
    {
      case PROP_ICON_SIZE:
        if ((guint) g_value_get_enum (value) != palette->priv->icon_size)
          {
            palette->priv->icon_size = g_value_get_enum (value);
            egg_tool_palette_reconfigured (palette);
          }
        break;

      case PROP_ORIENTATION:
        if ((guint) g_value_get_enum (value) != palette->priv->orientation)
          {
            palette->priv->orientation = g_value_get_enum (value);
            egg_tool_palette_reconfigured (palette);
          }
        break;

      case PROP_TOOLBAR_STYLE:
        if ((guint) g_value_get_enum (value) != palette->priv->style)
          {
            palette->priv->style = g_value_get_enum (value);
            egg_tool_palette_reconfigured (palette);
          }
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_tool_palette_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (object);

  switch (prop_id)
    {
      case PROP_ICON_SIZE:
        g_value_set_enum (value, egg_tool_palette_get_icon_size (palette));
        break;

      case PROP_ORIENTATION:
        g_value_set_enum (value, egg_tool_palette_get_orientation (palette));
        break;

      case PROP_TOOLBAR_STYLE:
        g_value_set_enum (value, egg_tool_palette_get_style (palette));
        break;

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
egg_tool_palette_dispose (GObject *object)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (object);
  guint i;

  if (palette->priv->hadjustment)
    {
      g_object_unref (palette->priv->hadjustment);
      palette->priv->hadjustment = NULL;
    }

  if (palette->priv->vadjustment)
    {
      g_object_unref (palette->priv->vadjustment);
      palette->priv->vadjustment = NULL;
    }

  for (i = 0; i < palette->priv->groups_size; ++i)
    {
      EggToolItemGroupInfo *group = &palette->priv->groups[i];

      if (group->notify_collapsed)
        {
          g_signal_handler_disconnect (group->widget, group->notify_collapsed);
          group->notify_collapsed = 0;
        }
    }

#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
  if (palette->priv->text_size_group)
    {
      g_object_unref (palette->priv->text_size_group);
      palette->priv->text_size_group = NULL;
    }
#endif

  G_OBJECT_CLASS (egg_tool_palette_parent_class)->dispose (object);
}

static void
egg_tool_palette_finalize (GObject *object)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (object);

  if (palette->priv->groups)
    {
      palette->priv->groups_length = 0;
      g_free (palette->priv->groups);
      palette->priv->groups = NULL;
    }

  G_OBJECT_CLASS (egg_tool_palette_parent_class)->finalize (object);
}

static void
egg_tool_palette_size_request (GtkWidget      *widget,
                               GtkRequisition *requisition)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolPalette *palette = EGG_TOOL_PALETTE (widget);
  GtkRequisition child_requisition;
  guint i;

  requisition->width = 0;
  requisition->height = 0;

  palette->priv->item_size.width = 0;
  palette->priv->item_size.height = 0;

  for (i = 0; i < palette->priv->groups_length; ++i)
    {
      EggToolItemGroupInfo *group = &palette->priv->groups[i];

      if (!group->widget)
        continue;

      gtk_widget_size_request (GTK_WIDGET (group->widget), &child_requisition);

      if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
        {
          requisition->width = MAX (requisition->width, child_requisition.width);
          requisition->height += child_requisition.height;
        }
      else
        {
          requisition->width += child_requisition.width;
          requisition->height = MAX (requisition->height, child_requisition.height);
        }

      _egg_tool_item_group_item_size_request (group->widget, &child_requisition);

      palette->priv->item_size.width = MAX (palette->priv->item_size.width,
                                            child_requisition.width);
      palette->priv->item_size.height = MAX (palette->priv->item_size.height,
                                             child_requisition.height);
    }

  requisition->width += border_width * 2;
  requisition->height += border_width * 2;
}

static void
egg_tool_palette_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolPalette *palette = EGG_TOOL_PALETTE (widget);
  GtkAdjustment *adjustment = NULL;
  GtkAllocation child_allocation;

  gint n_expand_groups = 0;
  gint remaining_space = 0;
  gint expand_space = 0;

  gint page_start, page_size = 0;
  gint offset = 0;
  guint i;

  gint min_offset = -1, max_offset = -1;

  gint x;

  gint *group_sizes = g_newa(gint, palette->priv->groups_length);

  GtkTextDirection direction = gtk_widget_get_direction (widget);

  GTK_WIDGET_CLASS (egg_tool_palette_parent_class)->size_allocate (widget, allocation);

  if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
    {
      adjustment = palette->priv->vadjustment;
      page_size = allocation->height;
    }
  else
    {
      adjustment = palette->priv->hadjustment;
      page_size = allocation->width;
    }

  if (adjustment)
    offset = gtk_adjustment_get_value (adjustment);
  if (GTK_ORIENTATION_HORIZONTAL == palette->priv->orientation &&
      GTK_TEXT_DIR_RTL == direction)
    offset = -offset;

  if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
    child_allocation.width = allocation->width - border_width * 2;
  else
    child_allocation.height = allocation->height - border_width * 2;

  if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
    remaining_space = allocation->height;
  else
    remaining_space = allocation->width;

  for (i = 0; i < palette->priv->groups_length; ++i)
    {
      EggToolItemGroupInfo *group = &palette->priv->groups[i];
      gint size;

      if (!group->widget)
        continue;

      widget = GTK_WIDGET (group->widget);

      if (egg_tool_item_group_get_n_items (group->widget))
        {
          if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
            size = _egg_tool_item_group_get_height_for_width (group->widget, child_allocation.width);
          else
            size = _egg_tool_item_group_get_width_for_height (group->widget, child_allocation.height);

          if (group->expand && !egg_tool_item_group_get_collapsed (group->widget))
            n_expand_groups += 1;
        }
      else
        size = 0;

      remaining_space -= size;
      group_sizes[i] = size;

      if (widget == palette->priv->expanding_child)
        {
          gint j, real_size;
          gint limit = GTK_ORIENTATION_VERTICAL == palette->priv->orientation ? child_allocation.width : child_allocation.height;

          min_offset = 0;
          for (j = 0; j < i; ++j)
            {
              min_offset += group_sizes[j];
            }
          max_offset = min_offset + group_sizes[i];

          real_size = _egg_tool_item_group_get_size_for_limit (EGG_TOOL_ITEM_GROUP (widget),
                                                               limit,
                                                               GTK_ORIENTATION_VERTICAL == palette->priv->orientation,
                                                               FALSE);
          if (size == real_size)
            palette->priv->expanding_child = NULL;
        }
    }

  if (n_expand_groups > 0)
    {
      remaining_space = MAX (0, remaining_space);
      expand_space = remaining_space / n_expand_groups;
    }

  if (max_offset != -1)
    {
      gint limit = GTK_ORIENTATION_VERTICAL == palette->priv->orientation ? allocation->height : allocation->width;

      offset = MIN(MAX (offset, max_offset - limit), min_offset);
    }

  if (remaining_space > 0)
    offset = 0;

  x = border_width;
  child_allocation.y = border_width;

  if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
    child_allocation.y -= offset;
  else
    x -= offset;

  for (i = 0; i < palette->priv->groups_length; ++i)
    {
      EggToolItemGroupInfo *group = &palette->priv->groups[i];
      GtkWidget *widget;

      if (!group->widget)
        continue;

      widget = GTK_WIDGET (group->widget);

      if (egg_tool_item_group_get_n_items (group->widget))
        {
          gint size = group_sizes[i];

          if (group->expand && !egg_tool_item_group_get_collapsed (group->widget))
            {
              size += MIN (expand_space, remaining_space);
              remaining_space -= expand_space;
            }

          if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
            child_allocation.height = size;
          else
            child_allocation.width = size;

          if (GTK_ORIENTATION_HORIZONTAL == palette->priv->orientation &&
              GTK_TEXT_DIR_RTL == direction)
            child_allocation.x = allocation->width - x - child_allocation.width;
          else
            child_allocation.x = x;

          gtk_widget_size_allocate (widget, &child_allocation);
          gtk_widget_show (widget);

          if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
            child_allocation.y += child_allocation.height;
          else
            x += child_allocation.width;
        }
      else
        gtk_widget_hide (widget);
    }

  if (GTK_ORIENTATION_VERTICAL == palette->priv->orientation)
    {
      child_allocation.y += border_width;
      child_allocation.y += offset;

      page_start = child_allocation.y;
    }
  else
    {
      x += border_width;
      x += offset;

      page_start = x;
    }

  if (adjustment)
    {
      gdouble value;

      adjustment->page_increment = page_size * 0.9;
      adjustment->step_increment = page_size * 0.1;
      adjustment->page_size = page_size;
      if (GTK_ORIENTATION_HORIZONTAL == palette->priv->orientation &&
          GTK_TEXT_DIR_RTL == direction)
        {
          adjustment->lower = page_size - MAX (0, page_start);
          adjustment->upper = page_size;

          offset = -offset;

          value = MAX(offset, adjustment->lower);
          gtk_adjustment_clamp_page (adjustment, offset, value + page_size);
        }
      else
        {
          adjustment->lower = 0;
          adjustment->upper = MAX (0, page_start);

          value = MIN (offset, adjustment->upper - adjustment->page_size);
          gtk_adjustment_clamp_page (adjustment, value, offset + page_size);
        }

      gtk_adjustment_changed (adjustment);
    }
}

static gboolean
egg_tool_palette_expose_event (GtkWidget      *widget,
                               GdkEventExpose *event)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (widget);
  GdkDisplay *display;
  cairo_t *cr;
  guint i;

  display = gdk_drawable_get_display (widget->window);

  if (!gdk_display_supports_composite (display))
    return FALSE;

  cr = gdk_cairo_create (widget->window);
  gdk_cairo_region (cr, event->region);
  cairo_clip (cr);

  cairo_push_group (cr);

  for (i = 0; i < palette->priv->groups_length; ++i)
    if (palette->priv->groups[i].widget)
      _egg_tool_item_group_paint (palette->priv->groups[i].widget, cr);

  cairo_pop_group_to_source (cr);

  cairo_paint (cr);
  cairo_destroy (cr);

  return FALSE;
}

static void
egg_tool_palette_realize (GtkWidget *widget)
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

static void
egg_tool_palette_adjustment_value_changed (GtkAdjustment *adjustment G_GNUC_UNUSED,
                                           gpointer       data)
{
  GtkWidget *widget = GTK_WIDGET (data);
  egg_tool_palette_size_allocate (widget, &widget->allocation);
}

static void
egg_tool_palette_set_scroll_adjustments (GtkWidget     *widget,
                                         GtkAdjustment *hadjustment,
                                         GtkAdjustment *vadjustment)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (widget);

  if (palette->priv->hadjustment)
    g_object_unref (palette->priv->hadjustment);
  if (palette->priv->vadjustment)
    g_object_unref (palette->priv->vadjustment);

  if (hadjustment)
    g_object_ref_sink (hadjustment);
  if (vadjustment)
    g_object_ref_sink (vadjustment);

  palette->priv->hadjustment = hadjustment;
  palette->priv->vadjustment = vadjustment;

  if (palette->priv->hadjustment)
    g_signal_connect (palette->priv->hadjustment, "value-changed",
                      G_CALLBACK (egg_tool_palette_adjustment_value_changed),
                      palette);
  if (palette->priv->vadjustment)
    g_signal_connect (palette->priv->vadjustment, "value-changed",
                      G_CALLBACK (egg_tool_palette_adjustment_value_changed),
                      palette);
}

static void
egg_tool_palette_repack (EggToolPalette *palette)
{
  guint si, di;

  for (si = di = 0; di < palette->priv->groups_length; ++si)
    {
      if (palette->priv->groups[si].widget)
        {
          palette->priv->groups[di] = palette->priv->groups[si];
          ++di;
        }
      else
        --palette->priv->groups_length;
    }

  palette->priv->sparse_groups = FALSE;
}

static void
egg_tool_palette_add (GtkContainer *container,
                      GtkWidget    *child)
{
  EggToolPalette *palette;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (container));
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (child));

  palette = EGG_TOOL_PALETTE (container);

  if (palette->priv->groups_length == palette->priv->groups_size)
    egg_tool_palette_repack (palette);

  if (palette->priv->groups_length == palette->priv->groups_size)
    {
      gsize old_size = palette->priv->groups_size;
      gsize new_size = old_size * 2;

      palette->priv->groups = g_renew (EggToolItemGroupInfo,
                                       palette->priv->groups,
                                       new_size);

      memset (palette->priv->groups + old_size, 0,
              sizeof (EggToolItemGroupInfo) * old_size);

      palette->priv->groups_size = new_size;
    }

  palette->priv->groups[palette->priv->groups_length].widget = g_object_ref_sink (child);
  palette->priv->groups_length += 1;

  gtk_widget_set_parent (child, GTK_WIDGET (palette));
}

static void
egg_tool_palette_remove (GtkContainer *container,
                         GtkWidget    *child)
{
  EggToolPalette *palette;
  guint i;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (container));
  palette = EGG_TOOL_PALETTE (container);

  for (i = 0; i < palette->priv->groups_length; ++i)
    if ((GtkWidget*) palette->priv->groups[i].widget == child)
      {
        g_object_unref (child);
        gtk_widget_unparent (child);

        memset (&palette->priv->groups[i], 0, sizeof (EggToolItemGroupInfo));
        palette->priv->sparse_groups = TRUE;
      }
}

static void
egg_tool_palette_forall (GtkContainer *container,
                         gboolean      internals G_GNUC_UNUSED,
                         GtkCallback   callback,
                         gpointer      callback_data)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (container);
  guint i;

  if (palette->priv->groups)
    {
      for (i = 0; i < palette->priv->groups_length; ++i)
        if (palette->priv->groups[i].widget)
          callback (GTK_WIDGET (palette->priv->groups[i].widget),
                    callback_data);
    }
}

static GType
egg_tool_palette_child_type (GtkContainer *container G_GNUC_UNUSED)
{
  return EGG_TYPE_TOOL_ITEM_GROUP;
}

static void
egg_tool_palette_set_child_property (GtkContainer *container,
                                     GtkWidget    *child,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (container);

  switch (prop_id)
    {
      case CHILD_PROP_EXCLUSIVE:
        egg_tool_palette_set_exclusive (palette, child, g_value_get_boolean (value));
        break;

      case CHILD_PROP_EXPAND:
        egg_tool_palette_set_expand (palette, child, g_value_get_boolean (value));
        break;

      default:
        GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
        break;
    }
}

static void
egg_tool_palette_get_child_property (GtkContainer *container,
                                     GtkWidget    *child,
                                     guint         prop_id,
                                     GValue       *value,
                                     GParamSpec   *pspec)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (container);

  switch (prop_id)
    {
      case CHILD_PROP_EXCLUSIVE:
        g_value_set_boolean (value, egg_tool_palette_get_exclusive (palette, child));
        break;

      case CHILD_PROP_EXPAND:
        g_value_set_boolean (value, egg_tool_palette_get_expand (palette, child));
        break;

      default:
        GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
        break;
    }
}

static void
egg_tool_palette_class_init (EggToolPaletteClass *cls)
{
  GObjectClass      *oclass   = G_OBJECT_CLASS (cls);
  GtkWidgetClass    *wclass   = GTK_WIDGET_CLASS (cls);
  GtkContainerClass *cclass   = GTK_CONTAINER_CLASS (cls);

  oclass->set_property        = egg_tool_palette_set_property;
  oclass->get_property        = egg_tool_palette_get_property;
  oclass->dispose             = egg_tool_palette_dispose;
  oclass->finalize            = egg_tool_palette_finalize;

  wclass->size_request        = egg_tool_palette_size_request;
  wclass->size_allocate       = egg_tool_palette_size_allocate;
  wclass->expose_event        = egg_tool_palette_expose_event;
  wclass->realize             = egg_tool_palette_realize;

  cclass->add                 = egg_tool_palette_add;
  cclass->remove              = egg_tool_palette_remove;
  cclass->forall              = egg_tool_palette_forall;
  cclass->child_type          = egg_tool_palette_child_type;
  cclass->set_child_property  = egg_tool_palette_set_child_property;
  cclass->get_child_property  = egg_tool_palette_get_child_property;

  cls->set_scroll_adjustments = egg_tool_palette_set_scroll_adjustments;

  wclass->set_scroll_adjustments_signal =
    g_signal_new ("set-scroll-adjustments",
                  G_TYPE_FROM_CLASS (oclass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (EggToolPaletteClass, set_scroll_adjustments),
                  NULL, NULL,
                  _egg_marshal_VOID__OBJECT_OBJECT,
                  G_TYPE_NONE, 2,
                  GTK_TYPE_ADJUSTMENT,
                  GTK_TYPE_ADJUSTMENT);

  g_object_class_install_property (oclass, PROP_ICON_SIZE,
                                   g_param_spec_enum ("icon-size",
                                                      P_("Icon Size"),
                                                      P_("The size of palette icons"),
                                                      GTK_TYPE_ICON_SIZE,
                                                      DEFAULT_ICON_SIZE,
                                                      G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (oclass, PROP_ORIENTATION,
                                   g_param_spec_enum ("orientation",
                                                      P_("Orientation"),
                                                      P_("Orientation of the tool palette"),
                                                      GTK_TYPE_ORIENTATION,
                                                      DEFAULT_ORIENTATION,
                                                      G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (oclass, PROP_TOOLBAR_STYLE,
                                   g_param_spec_enum ("toolbar-style",
                                                      P_("Toolbar Style"),
                                                      P_("Style of items in the tool palette"),
                                                      GTK_TYPE_TOOLBAR_STYLE,
                                                      DEFAULT_TOOLBAR_STYLE,
                                                      G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_container_class_install_child_property (cclass, CHILD_PROP_EXCLUSIVE,
                                              g_param_spec_boolean ("exclusive",
                                                                    P_("Exclusive"),
                                                                    P_("Whether the item group should be the only expanded at a given time"),
                                                                    DEFAULT_CHILD_EXCLUSIVE,
                                                                    G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                    G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_container_class_install_child_property (cclass, CHILD_PROP_EXPAND,
                                              g_param_spec_boolean ("expand",
                                                                    P_("Expand"),
                                                                    P_("Whether the item group should receive extra space when the palette grows"),
                                                                    DEFAULT_CHILD_EXPAND,
                                                                    G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                    G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_type_class_add_private (cls, sizeof (EggToolPalettePrivate));

  dnd_target_atom_item = gdk_atom_intern_static_string (dnd_targets[0].target);
  dnd_target_atom_group = gdk_atom_intern_static_string (dnd_targets[1].target);
}

GtkWidget*
egg_tool_palette_new (void)
{
  return g_object_new (EGG_TYPE_TOOL_PALETTE, NULL);
}

void
egg_tool_palette_set_icon_size (EggToolPalette *palette,
                                GtkIconSize     icon_size)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));

  if (icon_size != palette->priv->icon_size)
    g_object_set (palette, "icon-size", icon_size, NULL);
}

void
egg_tool_palette_set_orientation (EggToolPalette *palette,
                                  GtkOrientation  orientation)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));

  if (orientation != palette->priv->orientation)
    g_object_set (palette, "orientation", orientation, NULL);
}

void
egg_tool_palette_set_style (EggToolPalette  *palette,
                            GtkToolbarStyle  style)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));

  if (style != palette->priv->style)
    g_object_set (palette, "style", style, NULL);
}

GtkIconSize
egg_tool_palette_get_icon_size (EggToolPalette *palette)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), DEFAULT_ICON_SIZE);
  return palette->priv->icon_size;
}

GtkOrientation
egg_tool_palette_get_orientation (EggToolPalette *palette)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), DEFAULT_ORIENTATION);
  return palette->priv->orientation;
}

GtkToolbarStyle
egg_tool_palette_get_style (EggToolPalette *palette)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), DEFAULT_TOOLBAR_STYLE);
  return palette->priv->style;
}

void
egg_tool_palette_set_group_position (EggToolPalette *palette,
                                     GtkWidget      *group,
                                     gint            position)
{
  EggToolItemGroupInfo group_info;
  gint old_position;
  gpointer src, dst;
  gsize len;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  egg_tool_palette_repack (palette);

  g_return_if_fail (position >= -1);

  if (-1 == position)
    position = palette->priv->groups_length - 1;

  g_return_if_fail ((guint) position < palette->priv->groups_length);

  if (EGG_TOOL_ITEM_GROUP (group) == palette->priv->groups[position].widget)
    return;

  old_position = egg_tool_palette_get_group_position (palette, group);
  g_return_if_fail (old_position >= 0);

  group_info = palette->priv->groups[old_position];

  if (position < old_position)
    {
      dst = palette->priv->groups + position + 1;
      src = palette->priv->groups + position;
      len = old_position - position;
    }
  else
    {
      dst = palette->priv->groups + old_position;
      src = palette->priv->groups + old_position + 1;
      len = position - old_position;
    }

  memmove (dst, src, len * sizeof (*palette->priv->groups));
  palette->priv->groups[position] = group_info;

  gtk_widget_queue_resize (GTK_WIDGET (palette));
}

static void
egg_tool_palette_group_notify_collapsed (EggToolItemGroup *group,
                                         GParamSpec       *pspec G_GNUC_UNUSED,
                                         gpointer          data)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (data);
  guint i;

  if (egg_tool_item_group_get_collapsed (group))
    return;

  for (i = 0; i < palette->priv->groups_size; ++i)
    {
      EggToolItemGroup *current_group = palette->priv->groups[i].widget;

      if (current_group && current_group != group)
        egg_tool_item_group_set_collapsed (palette->priv->groups[i].widget, TRUE);
    }
}

void
egg_tool_palette_set_exclusive (EggToolPalette *palette,
                                GtkWidget      *group,
                                gboolean        exclusive)
{
  EggToolItemGroupInfo *group_info;
  gint position;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  position = egg_tool_palette_get_group_position (palette, group);
  g_return_if_fail (position >= 0);

  group_info = &palette->priv->groups[position];

  if (exclusive == group_info->exclusive)
    return;

  group_info->exclusive = exclusive;

  if (group_info->exclusive != (0 != group_info->notify_collapsed))
    {
      if (group_info->exclusive)
        {
          group_info->notify_collapsed =
            g_signal_connect (group, "notify::collapsed",
                              G_CALLBACK (egg_tool_palette_group_notify_collapsed),
                              palette);
        }
      else
        {
          g_signal_handler_disconnect (group, group_info->notify_collapsed);
          group_info->notify_collapsed = 0;
        }
    }

  egg_tool_palette_group_notify_collapsed (group_info->widget, NULL, palette);
  gtk_widget_child_notify (group, "exclusive");
}

void
egg_tool_palette_set_expand (EggToolPalette *palette,
                             GtkWidget      *group,
                             gboolean        expand G_GNUC_UNUSED)
{
  EggToolItemGroupInfo *group_info;
  gint position;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  position = egg_tool_palette_get_group_position (palette, group);
  g_return_if_fail (position >= 0);

  group_info = &palette->priv->groups[position];

  if (expand != group_info->expand)
    {
      group_info->expand = expand;
      gtk_widget_queue_resize (GTK_WIDGET (palette));
      gtk_widget_child_notify (group, "expand");
    }
}

gint
egg_tool_palette_get_group_position (EggToolPalette *palette,
                                     GtkWidget      *group)
{
  guint i;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), -1);
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), -1);

  for (i = 0; i < palette->priv->groups_length; ++i)
    if ((gpointer) group == palette->priv->groups[i].widget)
      return i;

  return -1;
}

gboolean
egg_tool_palette_get_exclusive (EggToolPalette *palette G_GNUC_UNUSED,
                                GtkWidget      *group G_GNUC_UNUSED)
{
  gint position;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), DEFAULT_CHILD_EXCLUSIVE);
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), DEFAULT_CHILD_EXCLUSIVE);

  position = egg_tool_palette_get_group_position (palette, group);
  g_return_val_if_fail (position >= 0, DEFAULT_CHILD_EXCLUSIVE);

  return palette->priv->groups[position].exclusive;
}

gboolean
egg_tool_palette_get_expand (EggToolPalette *palette,
                             GtkWidget      *group)
{
  gint position;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), DEFAULT_CHILD_EXPAND);
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), DEFAULT_CHILD_EXPAND);

  position = egg_tool_palette_get_group_position (palette, group);
  g_return_val_if_fail (position >= 0, DEFAULT_CHILD_EXPAND);

  return palette->priv->groups[position].expand;
}

GtkToolItem*
egg_tool_palette_get_drop_item (EggToolPalette *palette,
                                gint            x,
                                gint            y)
{
  GtkWidget *group = egg_tool_palette_get_drop_group (palette, x, y);

  if (group)
    return egg_tool_item_group_get_drop_item (EGG_TOOL_ITEM_GROUP (group),
                                              x - group->allocation.x,
                                              y - group->allocation.y);

  return NULL;
}

GtkWidget*
egg_tool_palette_get_drop_group (EggToolPalette *palette,
                                 gint            x,
                                 gint            y)
{
  GtkAllocation *allocation;
  guint i;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), NULL);

  allocation = &GTK_WIDGET (palette)->allocation;

  g_return_val_if_fail (x >= 0 && x < allocation->width, NULL);
  g_return_val_if_fail (y >= 0 && y < allocation->height, NULL);

  for (i = 0; i < palette->priv->groups_length; ++i)
    {
      EggToolItemGroupInfo *group = &palette->priv->groups[i];
      GtkWidget *widget;
      gint x0, y0;

      if (!group->widget)
        continue;

      widget = GTK_WIDGET (group->widget);

      x0 = x - widget->allocation.x;
      y0 = y - widget->allocation.y;

      if (x0 >= 0 && x0 < widget->allocation.width &&
          y0 >= 0 && y0 < widget->allocation.height)
        return widget;
    }

  return NULL;
}

GtkWidget*
egg_tool_palette_get_drag_item (EggToolPalette         *palette,
                                const GtkSelectionData *selection)
{
  EggToolPaletteDragData *data;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), NULL);
  g_return_val_if_fail (NULL != selection, NULL);

  g_return_val_if_fail (selection->format == 8, NULL);
  g_return_val_if_fail (selection->length == sizeof (EggToolPaletteDragData), NULL);
  g_return_val_if_fail (selection->target == dnd_target_atom_item ||
                        selection->target == dnd_target_atom_group,
                        NULL);

  data = (EggToolPaletteDragData*) selection->data;

  g_return_val_if_fail (data->palette == palette, NULL);

  if (dnd_target_atom_item == selection->target)
    g_return_val_if_fail (GTK_IS_TOOL_ITEM (data->item), NULL);
  else if (dnd_target_atom_group == selection->target)
    g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (data->item), NULL);

  return data->item;
}

void
egg_tool_palette_set_drag_source (EggToolPalette *palette)
{
  guint i;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));

  if (palette->priv->drag_source)
    return;

  palette->priv->drag_source = TRUE;

  for (i = 0; i < palette->priv->groups_length; ++i)
    {
      if (palette->priv->groups[i].widget)
        gtk_container_forall (GTK_CONTAINER (palette->priv->groups[i].widget),
                              _egg_tool_palette_child_set_drag_source,
                              palette);
    }
}

void
egg_tool_palette_add_drag_dest (EggToolPalette            *palette,
                                GtkWidget                 *widget,
                                GtkDestDefaults            flags,
                                EggToolPaletteDragTargets  targets,
                                GdkDragAction              actions)
{
  GtkTargetEntry entries[G_N_ELEMENTS (dnd_targets)];
  gint n_entries = 0;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  egg_tool_palette_set_drag_source (palette);

  if (targets & EGG_TOOL_PALETTE_DRAG_ITEMS)
    entries[n_entries++] = dnd_targets[0];
  if (targets & EGG_TOOL_PALETTE_DRAG_GROUPS)
    entries[n_entries++] = dnd_targets[1];

  gtk_drag_dest_set (widget, flags, entries, n_entries, actions);
}

void
_egg_tool_palette_get_item_size (EggToolPalette *palette,
                                 GtkRequisition *item_size)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));
  g_return_if_fail (NULL != item_size);

  *item_size = palette->priv->item_size;
}

static GtkWidget*
egg_tool_palette_find_anchestor (GtkWidget *widget,
                                 GType      type)
{
  while (widget)
    {
      if (G_TYPE_CHECK_INSTANCE_TYPE (widget, type))
        return widget;

      widget = gtk_widget_get_parent (widget);
    }

  return NULL;
}

static void
egg_tool_palette_item_drag_data_get (GtkWidget        *widget,
                                     GdkDragContext   *context G_GNUC_UNUSED,
                                     GtkSelectionData *selection,
                                     guint             info G_GNUC_UNUSED,
                                     guint             time G_GNUC_UNUSED,
                                     gpointer          data)
{
  EggToolPaletteDragData drag_data = { EGG_TOOL_PALETTE (data), NULL };

  if (selection->target == dnd_target_atom_item)
    drag_data.item = egg_tool_palette_find_anchestor (widget, GTK_TYPE_TOOL_ITEM);

  if (drag_data.item)
    gtk_selection_data_set (selection, selection->target, 8,
                            (guchar*) &drag_data, sizeof (drag_data));
}

static void
egg_tool_palette_child_drag_data_get (GtkWidget        *widget,
                                      GdkDragContext   *context G_GNUC_UNUSED,
                                      GtkSelectionData *selection,
                                      guint             info G_GNUC_UNUSED,
                                      guint             time G_GNUC_UNUSED,
                                      gpointer          data)
{
  EggToolPaletteDragData drag_data = { EGG_TOOL_PALETTE (data), NULL };

  if (selection->target == dnd_target_atom_group)
    drag_data.item = egg_tool_palette_find_anchestor (widget, EGG_TYPE_TOOL_ITEM_GROUP);

  if (drag_data.item)
    gtk_selection_data_set (selection, selection->target, 8,
                            (guchar*) &drag_data, sizeof (drag_data));
}

void
_egg_tool_palette_child_set_drag_source (GtkWidget *child,
                                         gpointer   data)
{
  EggToolPalette *palette = EGG_TOOL_PALETTE (data);

  /* Check drag_source,
   * to work properly when called from egg_tool_item_group_insert().
   */
  if (!palette->priv->drag_source)
    return;

  if (GTK_IS_TOOL_ITEM (child))
    {
      /* Connect to child instead of the item itself,
       * to work arround bug 510377.
       */
      child = gtk_bin_get_child (GTK_BIN (child));

      if (!child)
        return;

      gtk_drag_source_set (child, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
                           &dnd_targets[0], 1, GDK_ACTION_COPY | GDK_ACTION_MOVE);

      g_signal_connect (child, "drag-data-get",
                        G_CALLBACK (egg_tool_palette_item_drag_data_get),
                        palette);
    }
  else if (GTK_IS_BUTTON (child))
    {
      gtk_drag_source_set (child, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
                           &dnd_targets[1], 1, GDK_ACTION_COPY | GDK_ACTION_MOVE);

      g_signal_connect (child, "drag-data-get",
                        G_CALLBACK (egg_tool_palette_child_drag_data_get),
                        palette);
    }
}

G_CONST_RETURN GtkTargetEntry*
egg_tool_palette_get_drag_target_item (void)
{
  return &dnd_targets[0];
}

G_CONST_RETURN GtkTargetEntry*
egg_tool_palette_get_drag_target_group (void)
{
  return &dnd_targets[1];
}

void
_egg_tool_palette_set_expanding_child (EggToolPalette   *palette,
                                       GtkWidget        *widget)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (palette));

  palette->priv->expanding_child = widget;
}

#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
GtkSizeGroup *
_egg_tool_palette_get_size_group (EggToolPalette *palette)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (palette), NULL);

  return palette->priv->text_size_group;
}
#endif
