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
 *      Jan Arne Petersen
 */

#include "eggtoolitemgroup.h"
#include "eggtoolpaletteprivate.h"

#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

#define P_(msgid)               (msgid)

#define ANIMATION_TIMEOUT        50
#define ANIMATION_DURATION      (ANIMATION_TIMEOUT * 4)
#define DEFAULT_EXPANDER_SIZE    16
#define DEFAULT_HEADER_SPACING   2

#define DEFAULT_NAME             NULL
#define DEFAULT_COLLAPSED        FALSE
#define DEFAULT_ELLIPSIZE        PANGO_ELLIPSIZE_NONE

enum
{
  PROP_NONE,
  PROP_NAME,
  PROP_COLLAPSED,
  PROP_ELLIPSIZE,
};

enum
{
  CHILD_PROP_NONE,
  CHILD_PROP_HOMOGENEOUS,
  CHILD_PROP_EXPAND,
  CHILD_PROP_FILL,
  CHILD_PROP_NEW_ROW,
  CHILD_PROP_POSITION,
};

typedef struct _EggToolItemGroupChild EggToolItemGroupChild;

struct _EggToolItemGroupPrivate
{
  GtkWidget         *header;

  GList             *children;

  gint64             animation_start;
  GSource           *animation_timeout;
  GtkExpanderStyle   expander_style;
  gint               expander_size;
  gint               header_spacing;
  PangoEllipsizeMode ellipsize;

  guint              collapsed : 1;

};

struct _EggToolItemGroupChild
{
  GtkToolItem *item;

  guint        homogeneous : 1;
  guint        expand : 1;
  guint        fill : 1;
  guint        new_row : 1;
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

#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090

static PangoEllipsizeMode
egg_tool_item_group_get_ellipsize_mode (GtkToolShell *shell)
{
  return EGG_TOOL_ITEM_GROUP (shell)->priv->ellipsize;
}

static gfloat
egg_tool_item_group_get_text_alignment (GtkToolShell *shell)
{
  if (GTK_TOOLBAR_TEXT == egg_tool_item_group_get_style (shell) ||
      GTK_TOOLBAR_BOTH_HORIZ == egg_tool_item_group_get_style (shell))
    return 0.0;

  return 0.5;
}

static GtkOrientation
egg_tool_item_group_get_text_orientation (GtkToolShell *shell)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (shell));

  if (EGG_IS_TOOL_PALETTE (parent))
    {
      GtkOrientation orientation = egg_tool_palette_get_orientation (EGG_TOOL_PALETTE (parent));
      if (GTK_ORIENTATION_HORIZONTAL == orientation &&
          (GTK_TOOLBAR_TEXT == egg_tool_item_group_get_style (shell)/* ||
           GTK_TOOLBAR_BOTH_HORIZ == egg_tool_item_group_get_style (shell)*/))
        return GTK_ORIENTATION_VERTICAL;
    }

  return GTK_ORIENTATION_HORIZONTAL;
}

static GtkSizeGroup *
egg_tool_item_group_get_text_size_group (GtkToolShell *shell)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (shell));

  if (EGG_IS_TOOL_PALETTE (parent))
    return _egg_tool_palette_get_size_group (EGG_TOOL_PALETTE (parent));

  return NULL;
}

#endif

static void
egg_tool_item_group_tool_shell_init (GtkToolShellIface *iface)
{
  iface->get_icon_size = egg_tool_item_group_get_icon_size;
  iface->get_orientation = egg_tool_item_group_get_orientation;
  iface->get_style = egg_tool_item_group_get_style;
#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
  iface->get_text_alignment = egg_tool_item_group_get_text_alignment;
  iface->get_text_orientation = egg_tool_item_group_get_text_orientation;
  iface->get_text_size_group = egg_tool_item_group_get_text_size_group;
  iface->get_ellipsize_mode = egg_tool_item_group_get_ellipsize_mode;
#endif
}

#endif /* GTK_TYPE_TOOL_SHELL */

static gboolean
egg_tool_item_group_header_expose_event_cb (GtkWidget      *widget,
                                            GdkEventExpose *event,
                                            gpointer        data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (data);
  GtkExpanderStyle expander_style;
  GtkOrientation orientation;
  gint x, y;
  GtkTextDirection direction;

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));
  expander_style = group->priv->expander_style;
  direction = gtk_widget_get_direction (widget);

  if (GTK_ORIENTATION_VERTICAL == orientation)
    {
      if (GTK_TEXT_DIR_RTL == direction)
        x = widget->allocation.x + widget->allocation.width - group->priv->expander_size / 2;
      else
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
  GtkWidget *parent = gtk_widget_get_parent (data);

  if (group->priv->collapsed ||
      !EGG_IS_TOOL_PALETTE (parent) ||
      !egg_tool_palette_get_exclusive (EGG_TOOL_PALETTE (parent), data))
    egg_tool_item_group_set_collapsed (group, !group->priv->collapsed);
}

static void
egg_tool_item_group_header_adjust_style (EggToolItemGroup *group)
{
  GtkWidget *alignment = egg_tool_item_group_get_alignment (group);
  GtkWidget *label = gtk_bin_get_child (GTK_BIN (alignment));
  GtkWidget *widget = GTK_WIDGET (group);
  gint dx = 0, dy = 0;
  GtkTextDirection direction = gtk_widget_get_direction (widget);

  gtk_widget_style_get (widget,
                        "header-spacing", &group->priv->header_spacing,
                        "expander-size", &group->priv->expander_size,
                        NULL);

  switch (gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group)))
    {
      case GTK_ORIENTATION_HORIZONTAL:
        dy = group->priv->header_spacing + group->priv->expander_size;
        gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_NONE);
        if (GTK_TEXT_DIR_RTL == direction)
          gtk_label_set_angle (GTK_LABEL (label), -90);
        else
          gtk_label_set_angle (GTK_LABEL (label), 90);
        break;

      case GTK_ORIENTATION_VERTICAL:
        dx = group->priv->header_spacing + group->priv->expander_size;
        gtk_label_set_ellipsize (GTK_LABEL (label), group->priv->ellipsize);
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

  group->priv->children = NULL;
  group->priv->header_spacing = DEFAULT_HEADER_SPACING;
  group->priv->expander_size = DEFAULT_EXPANDER_SIZE;
  group->priv->expander_style = GTK_EXPANDER_EXPANDED;

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

      case PROP_COLLAPSED:
        egg_tool_item_group_set_collapsed (group, g_value_get_boolean (value));
        break;

      case PROP_ELLIPSIZE:
        egg_tool_item_group_set_ellipsize (group, g_value_get_enum (value));
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

      case PROP_COLLAPSED:
        g_value_set_boolean (value, egg_tool_item_group_get_collapsed (group));
        break;

      case PROP_ELLIPSIZE:
        g_value_set_enum (value, egg_tool_item_group_get_ellipsize (group));
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

  if (group->priv->children)
    {
      g_list_free (group->priv->children);
      group->priv->children = NULL;
    }

  G_OBJECT_CLASS (egg_tool_item_group_parent_class)->finalize (object);
}

static void
egg_tool_item_group_get_item_size (EggToolItemGroup *group,
                                   GtkRequisition   *item_size,
                                   gboolean          homogeneous_only,
                                   guint            *requested_rows)
{
  GtkWidget *parent = gtk_widget_get_parent (GTK_WIDGET (group));

  if (EGG_IS_TOOL_PALETTE (parent))
    _egg_tool_palette_get_item_size (EGG_TOOL_PALETTE (parent), item_size, homogeneous_only, requested_rows);
  else
    _egg_tool_item_group_item_size_request (group, item_size, homogeneous_only, requested_rows);
}

static void
egg_tool_item_group_size_request (GtkWidget      *widget,
                                  GtkRequisition *requisition)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);
  GtkOrientation orientation;
  GtkRequisition item_size;
  guint requested_rows;

  if (group->priv->children && egg_tool_item_group_get_name (group))
    {
      gtk_widget_size_request (group->priv->header, requisition);
      gtk_widget_show (group->priv->header);
    }
  else
    {
      requisition->width = requisition->height = 0;
      gtk_widget_hide (group->priv->header);
    }

  egg_tool_item_group_get_item_size (group, &item_size, FALSE, &requested_rows);

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));

  if (GTK_ORIENTATION_VERTICAL == orientation)
    requisition->width = MAX (requisition->width, item_size.width);
  else
    requisition->height = MAX (requisition->height, item_size.height * requested_rows);

  requisition->width += border_width * 2;
  requisition->height += border_width * 2;
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
egg_tool_item_group_real_size_query (GtkWidget      *widget,
                                     GtkAllocation  *allocation,
                                     GtkRequisition *inquery)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);

  GtkRequisition item_size;
  GtkAllocation item_area;

  GtkOrientation orientation;
  GtkToolbarStyle style;

  guint min_rows;

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));
  style = gtk_tool_shell_get_style (GTK_TOOL_SHELL (group));

  /* figure out the size of homogeneous items */
  egg_tool_item_group_get_item_size (group, &item_size, TRUE, &min_rows);

  if (GTK_ORIENTATION_VERTICAL == orientation)
    item_size.width = MIN (item_size.width, allocation->width);
  else
    item_size.height = MIN (item_size.height, allocation->height);

  item_area.width = 0;
  item_area.height = 0;

  /* figure out the required columns (n_columns) and rows (n_rows) to place all items */
  if (!group->priv->collapsed || group->priv->animation_timeout)
    {
      guint n_columns, n_rows;
      GList *it;

      if (GTK_ORIENTATION_VERTICAL == orientation)
        {
          gboolean new_row;
          gint row = -1;
          guint col = 0;

          item_area.width = allocation->width - 2 * border_width;
          n_columns = MAX (item_area.width / item_size.width, 1);

          /* calculate required rows for n_columns columns */
          for (it = group->priv->children; it != NULL; it = it->next)
            {
              EggToolItemGroupChild *child = it->data;

              if (!egg_tool_item_group_is_item_visible (child->item, orientation))
                continue;

              if (new_row || child->new_row)
                {
                  new_row = FALSE;
                  row++;
                  col = 0;
                }

              if (child->expand)
                new_row = TRUE;

              if (child->homogeneous)
                {
                  col++;
                  if (col >= n_columns)
                    new_row = TRUE;
                }
              else
                {
                  GtkRequisition req = {0};
                  guint width;

                  gtk_widget_size_request (GTK_WIDGET (child->item), &req);

                  width = (guint) ceil (1.0 * req.width / item_size.width);
                  col += width;
                  if (col > n_columns)
                    row++;
                  col = width;
                  if (col >= n_columns)
                    new_row = TRUE;
                }
            }
          n_rows = row + 2;
        }
      else
        {
          guint *row_min_width;
          gint row = -1;
          gboolean new_row = TRUE;
          guint col = 0, min_col, max_col = 0, all_items = 0;
          guint i;

          item_area.height = allocation->height - 2 * border_width;
          n_rows = MAX (item_area.height / item_size.height, min_rows);

          row_min_width = g_new0 (guint, n_rows);

          /* calculate minimal and maximal required cols and minimal required rows */
          for (it = group->priv->children; it != NULL; it = it->next)
            {
              EggToolItemGroupChild *child = it->data;

              if (!egg_tool_item_group_is_item_visible (child->item, orientation))
                continue;

              if (new_row || child->new_row)
                {
                  new_row = FALSE;
                  row++;
                  col = 0;
                  row_min_width[row] = 1;
                }

              if (child->expand)
                new_row = TRUE;

              if (child->homogeneous)
                {
                  col++;
                  all_items++;
                }
              else
                {
                  GtkRequisition req = {0};
                  guint width;

                  /* in horizontal text mode non homogneneous items are not supported */
                  if (GTK_TOOLBAR_TEXT == style)
                    continue;

                  gtk_widget_size_request (GTK_WIDGET (child->item), &req);

                  width = (guint) ceil (1.0 * req.width / item_size.width);
                  col += width;
                  all_items += width;

                  row_min_width[row] = MAX (row_min_width[row], width);
                }

              max_col = MAX (max_col, col);
            }

          /* calculate minimal required cols */
          min_col = (guint) ceil (1.0 * all_items / n_rows);
          for (i = 0; i <= row; i++)
            {
              min_col = MAX (min_col, row_min_width[i]);
            }

          /* simple linear search for minimal required columns if maximal row count is n_rows */
          for (n_columns = min_col; n_columns < max_col; n_columns ++)
            {
              new_row = TRUE;
              row = -1;
              /* calculate required rows for n_columns columns */
              for (it = group->priv->children; it != NULL; it = it->next)
                {
                  EggToolItemGroupChild *child = it->data;

                  if (!egg_tool_item_group_is_item_visible (child->item, orientation))
                    continue;

                  if (new_row || child->new_row)
                    {
                      new_row = FALSE;
                      row++;
                      col = 0;
                    }

                  if (child->expand)
                    new_row = TRUE;

                  if (child->homogeneous)
                    {
                      col++;
                      if (col >= n_columns)
                        new_row = TRUE;
                    }
                  else
                    {
                      GtkRequisition req = {0};
                      guint width;

                      /* in horizontal text mode non homogneneous items are not supported */
                      if (GTK_TOOLBAR_TEXT == style)
                        continue;

                      gtk_widget_size_request (GTK_WIDGET (child->item), &req);

                      width = (guint) ceil (1.0 * req.width / item_size.width);
                      col += width;
                      if (col > n_columns)
                        row++;
                      col = width;
                      if (col >= n_columns)
                        new_row = TRUE;
                    }
                }

              if (row < n_rows)
                break;
            }
        }

      item_area.width = item_size.width * n_columns;
      item_area.height = item_size.height * n_rows;
    }

  inquery->width = 0;
  inquery->height = 0;

  /* figure out header widget size */
  if (GTK_WIDGET_VISIBLE (group->priv->header))
    {
      GtkRequisition child_requisition;

      gtk_widget_size_request (group->priv->header, &child_requisition);

      if (GTK_ORIENTATION_VERTICAL == orientation)
        inquery->height += child_requisition.height;
      else
        inquery->width += child_requisition.width;
    }

  /* report effective widget size */
  inquery->width += item_area.width + 2 * border_width;
  inquery->height += item_area.height + 2 * border_width;
}

static void
egg_tool_item_group_real_size_allocate (GtkWidget      *widget,
                                        GtkAllocation  *allocation)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (widget);
  GtkRequisition child_requisition;
  GtkAllocation child_allocation;

  GtkRequisition item_size;
  GtkAllocation item_area;

  GtkOrientation orientation;
  GtkToolbarStyle style;

  GList *it;

  guint n_columns, n_rows = 1, min_rows;

  GtkTextDirection direction = gtk_widget_get_direction (widget);

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));
  style = gtk_tool_shell_get_style (GTK_TOOL_SHELL (group));

  /* chain up */
  GTK_WIDGET_CLASS (egg_tool_item_group_parent_class)->size_allocate (widget, allocation);

  child_allocation.x = border_width;
  child_allocation.y = border_width;

  /* place the header widget */
  if (GTK_WIDGET_VISIBLE (group->priv->header))
    {
      gtk_widget_size_request (group->priv->header, &child_requisition);

      if (GTK_ORIENTATION_VERTICAL == orientation)
        {
          child_allocation.width = allocation->width;
          child_allocation.height = child_requisition.height;
        }
      else
        {
          child_allocation.width = child_requisition.width;
          child_allocation.height = allocation->height;

	  if (GTK_TEXT_DIR_RTL == direction)
            child_allocation.x = allocation->width - border_width - child_allocation.width;
        }

      gtk_widget_size_allocate (group->priv->header, &child_allocation);

      if (GTK_ORIENTATION_VERTICAL == orientation)
        child_allocation.y += child_allocation.height;
      else if (GTK_TEXT_DIR_RTL != direction)
        child_allocation.x += child_allocation.width;
      else
        child_allocation.x = border_width;
    }
  else
    child_requisition.width = child_requisition.height = 0;

  /* figure out the size of homogeneous items */
  egg_tool_item_group_get_item_size (group, &item_size, TRUE, &min_rows);

  /* figure out the available columns and size of item_area */
  if (GTK_ORIENTATION_VERTICAL == orientation)
    {
      item_size.width = MIN (item_size.width, allocation->width);

      item_area.width = allocation->width - 2 * border_width;
      item_area.height = allocation->height - 2 * border_width - child_requisition.height;

      n_columns = MAX (item_area.width / item_size.width, 1);

      item_size.width = item_area.width / n_columns;
    }
  else
    {
      item_size.height = MIN (item_size.height, allocation->height);

      item_area.width = allocation->width - 2 * border_width - child_requisition.width;
      item_area.height = allocation->height - 2 * border_width;

      n_columns = MAX (item_area.width / item_size.width, 1);
      n_rows = MAX (item_area.height / item_size.height, min_rows);

      item_size.height = item_area.height / n_rows;
    }

  item_area.x = child_allocation.x;
  item_area.y = child_allocation.y;

  /* when expanded or in transition, place the tool items in a grid like layout */
  if (!group->priv->collapsed || group->priv->animation_timeout)
    {
      guint col = 0, row = 0;

      for (it = group->priv->children; it != NULL; it = it->next)
        {
          EggToolItemGroupChild *child = it->data;

          if (!egg_tool_item_group_is_item_visible (child->item, orientation))
            {
              gtk_widget_set_child_visible (GTK_WIDGET (child->item), FALSE);

              continue;
            }

          /* for non homogeneous widgets request the required size */
          child_requisition.width = 0;
	  if (!child->homogeneous)
            {
              /* in horizontal text mode non homogneneous items are not supported */
              if (GTK_ORIENTATION_HORIZONTAL == orientation && GTK_TOOLBAR_TEXT == style)
                {
                  gtk_widget_set_child_visible (GTK_WIDGET (child->item), FALSE);
                  continue;
                }

              gtk_widget_size_request (GTK_WIDGET (child->item), &child_requisition);
              child_requisition.width = MIN (child_requisition.width, item_area.width);
            }

          /* select next row if at end of row */
          if (col > 0 && (child->new_row || (col * item_size.width) + MAX (child_requisition.width, item_size.width) > item_area.width))
            {
              row++;
              col = 0;
              child_allocation.y += child_allocation.height;
            }

          /* calculate the position and size of the item */
          if (!child->homogeneous)
            {
              guint col_width;
              guint width;

              if (child->expand)
                col_width = n_columns - col;
              else
                col_width = (guint) ceil (1.0 * child_requisition.width / item_size.width);

              width = col_width * item_size.width;

              if (child->fill)
                {
                  child_allocation.x = item_area.x + 
			  (((GTK_TEXT_DIR_RTL == direction) ? (n_columns - col - col_width) : col) * item_size.width);
                  child_allocation.width = width;
                }
              else
                {
                  child_allocation.x = item_area.x + 
			  (((GTK_TEXT_DIR_RTL == direction) ? (n_columns - col - col_width) : col) * item_size.width) + 
			  (width - child_requisition.width) / 2;
                  child_allocation.width = child_requisition.width;
                }

              col += col_width;
            }
          else
            {
              child_allocation.x = item_area.x + 
		      (((GTK_TEXT_DIR_RTL == direction) ? (n_columns - col - 1) : col) * item_size.width);
              child_allocation.width = item_size.width;

              col++;
            }

          child_allocation.height = item_size.height;

          gtk_widget_size_allocate (GTK_WIDGET (child->item), &child_allocation);
          gtk_widget_set_child_visible (GTK_WIDGET (child->item), TRUE);
        }

      child_allocation.y += item_size.height;
    }

  /* or just hide all items, when collapsed */

  else
    {
      for (it = group->priv->children; it != NULL; it = it->next)
        {
          EggToolItemGroupChild *child = it->data;

          gtk_widget_set_child_visible (GTK_WIDGET (child->item), FALSE);
        }
    }
}

static void
egg_tool_item_group_size_allocate (GtkWidget     *widget,
                                   GtkAllocation *allocation)
{
  egg_tool_item_group_real_size_allocate (widget, allocation);

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
  GList *it;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (container));
  group = EGG_TOOL_ITEM_GROUP (container);

  for (it = group->priv->children; it != NULL; it = it->next)
    {
      EggToolItemGroupChild *child_info = it->data;

      if ((GtkWidget *)child_info->item == child)
        {
          g_object_unref (child);
          gtk_widget_unparent (child);

          g_free (child_info);
          group->priv->children = g_list_delete_link (group->priv->children, it);

          gtk_widget_queue_resize (GTK_WIDGET (container));
          break;
        }
    }
}

static void
egg_tool_item_group_forall (GtkContainer *container,
                            gboolean      internals,
                            GtkCallback   callback,
                            gpointer      callback_data)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (container);
  GList *children;

  if (internals && group->priv->header)
    callback (group->priv->header, callback_data);

  children = group->priv->children;
  while (children)
    {
      EggToolItemGroupChild *child = children->data;
      children = children->next; /* store pointer before call to callback
				    because the child pointer is invalid if the
				    child->item is removed from the item group 
				    in callback */

      callback (GTK_WIDGET (child->item), callback_data);
    }
}

static GType
egg_tool_item_group_child_type (GtkContainer *container G_GNUC_UNUSED)
{
  return GTK_TYPE_TOOL_ITEM;
}

static EggToolItemGroupChild *
egg_tool_item_group_get_child (EggToolItemGroup  *group,
                               GtkToolItem       *item,
                               gint              *position,
                               GList            **link)
{
  guint i;
  GList *it;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);
  g_return_val_if_fail (GTK_IS_TOOL_ITEM (item), NULL);

  for (it = group->priv->children, i = 0; it != NULL; it = it->next, ++i)
    {
      EggToolItemGroupChild *child = it->data;

      if (child->item == item)
        {
          if (position)
            *position = i;

          if (link)
            *link = it;

          return child;
        }
    }

  return NULL;
}

static void
egg_tool_item_group_get_item_packing (EggToolItemGroup *group,
                                      GtkToolItem      *item,
                                      gboolean         *homogeneous,
                                      gboolean         *expand,
                                      gboolean         *fill,
                                      gboolean         *new_row)
{
  EggToolItemGroupChild *child;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));

  child = egg_tool_item_group_get_child (group, item, NULL, NULL);
  if (!child)
    return;

  if (expand)
    *expand = child->expand;

  if (homogeneous)
    *homogeneous = child->homogeneous;

  if (fill)
    *fill = child->fill;

  if (new_row)
    *new_row = child->new_row;
}

static void
egg_tool_item_group_set_item_packing (EggToolItemGroup *group,
                                      GtkToolItem      *item,
                                      gboolean          homogeneous,
                                      gboolean          expand,
                                      gboolean          fill,
                                      gboolean          new_row)
{
  EggToolItemGroupChild *child;
  gboolean changed = FALSE;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));

  child = egg_tool_item_group_get_child (group, item, NULL, NULL);
  if (!child)
    return;

  gtk_widget_freeze_child_notify (GTK_WIDGET (item));

  if (child->homogeneous != homogeneous)
    {
      child->homogeneous = homogeneous;
      changed = TRUE;
      gtk_widget_child_notify (GTK_WIDGET (item), "homogeneous");
    }
  if (child->expand != expand)
    {
      child->expand = expand;
      changed = TRUE;
      gtk_widget_child_notify (GTK_WIDGET (item), "expand");
    }
  if (child->fill != fill)
    {
      child->fill = fill;
      changed = TRUE;
      gtk_widget_child_notify (GTK_WIDGET (item), "fill");
    }
  if (child->new_row != new_row)
    {
      child->new_row = new_row;
      changed = TRUE;
      gtk_widget_child_notify (GTK_WIDGET (item), "new-row");
    }

  gtk_widget_thaw_child_notify (GTK_WIDGET (item));

  if (changed && GTK_WIDGET_VISIBLE (group) && GTK_WIDGET_VISIBLE (item))
    gtk_widget_queue_resize (GTK_WIDGET (group));
}

static void
egg_tool_item_group_set_child_property (GtkContainer *container,
                                        GtkWidget    *child,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (container);
  GtkToolItem *item = GTK_TOOL_ITEM (child);
  gboolean homogeneous, expand, fill, new_row;

  if (prop_id != CHILD_PROP_POSITION)
    egg_tool_item_group_get_item_packing (group, item,
                                          &homogeneous,
                                          &expand,
                                          &fill,
                                          &new_row);

  switch (prop_id)
    {
      case CHILD_PROP_HOMOGENEOUS:
        egg_tool_item_group_set_item_packing (group, item,
                                              g_value_get_boolean (value),
                                              expand,
                                              fill,
                                              new_row);
        break;

      case CHILD_PROP_EXPAND:
        egg_tool_item_group_set_item_packing (group, item,
                                              homogeneous,
                                              g_value_get_boolean (value),
                                              fill,
                                              new_row);
        break;

      case CHILD_PROP_FILL:
        egg_tool_item_group_set_item_packing (group, item,
                                              homogeneous,
                                              expand,
                                              g_value_get_boolean (value),
                                              new_row);
        break;

      case CHILD_PROP_NEW_ROW:
        egg_tool_item_group_set_item_packing (group, item,
                                              homogeneous,
                                              expand,
                                              fill,
                                              g_value_get_boolean (value));
        break;

      case CHILD_PROP_POSITION:
        egg_tool_item_group_set_item_position (group, item, g_value_get_int (value));
        break;

      default:
        GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
        break;
    }
}

static void
egg_tool_item_group_get_child_property (GtkContainer *container,
                                        GtkWidget    *child,
                                        guint         prop_id,
                                        GValue       *value,
                                        GParamSpec   *pspec)
{
  EggToolItemGroup *group = EGG_TOOL_ITEM_GROUP (container);
  GtkToolItem *item = GTK_TOOL_ITEM (child);
  gboolean homogeneous, expand, fill, new_row;

  if (prop_id != CHILD_PROP_POSITION)
    egg_tool_item_group_get_item_packing (group, item,
                                          &homogeneous,
                                          &expand,
                                          &fill,
                                          &new_row);

  switch (prop_id)
    {
      case CHILD_PROP_HOMOGENEOUS:
        g_value_set_boolean (value, homogeneous);
        break;

       case CHILD_PROP_EXPAND:
        g_value_set_boolean (value, expand);
        break;

       case CHILD_PROP_FILL:
        g_value_set_boolean (value, fill);
        break;

       case CHILD_PROP_NEW_ROW:
        g_value_set_boolean (value, new_row);
        break;

     case CHILD_PROP_POSITION:
        g_value_set_int (value, egg_tool_item_group_get_item_position (group, item));
        break;

      default:
        GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
        break;
    }
}

static void
egg_tool_item_group_class_init (EggToolItemGroupClass *cls)
{
  GObjectClass       *oclass = G_OBJECT_CLASS (cls);
  GtkWidgetClass     *wclass = GTK_WIDGET_CLASS (cls);
  GtkContainerClass  *cclass = GTK_CONTAINER_CLASS (cls);

  oclass->set_property       = egg_tool_item_group_set_property;
  oclass->get_property       = egg_tool_item_group_get_property;
  oclass->finalize           = egg_tool_item_group_finalize;

  wclass->size_request       = egg_tool_item_group_size_request;
  wclass->size_allocate      = egg_tool_item_group_size_allocate;
  wclass->realize            = egg_tool_item_group_realize;
  wclass->style_set          = egg_tool_item_group_style_set;

  cclass->add                = egg_tool_item_group_add;
  cclass->remove             = egg_tool_item_group_remove;
  cclass->forall             = egg_tool_item_group_forall;
  cclass->child_type         = egg_tool_item_group_child_type;
  cclass->set_child_property = egg_tool_item_group_set_child_property;
  cclass->get_child_property = egg_tool_item_group_get_child_property;

  g_object_class_install_property (oclass, PROP_NAME,
                                   g_param_spec_string ("name",
                                                        P_("Name"),
                                                        P_("The name of this item group"),
                                                        DEFAULT_NAME,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (oclass, PROP_COLLAPSED,
                                   g_param_spec_boolean ("collapsed",
                                                         P_("Collapsed"),
                                                         P_("Wether the group has been collapsed and items are hidden"),
                                                         DEFAULT_COLLAPSED,
                                                         G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                         G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  g_object_class_install_property (oclass, PROP_ELLIPSIZE,
                                   g_param_spec_enum ("ellipsize",
                                                      P_("ellipsize"),
                                                      P_("Ellipsize for item group headers"),
                                                      PANGO_TYPE_ELLIPSIZE_MODE, DEFAULT_ELLIPSIZE,
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

  gtk_container_class_install_child_property (cclass, CHILD_PROP_HOMOGENEOUS,
                                              g_param_spec_boolean ("homogeneous",
                                                                    P_("Homogeneous"),
                                                                    P_("Whether the item should be the same size as other homogeneous items"),
                                                                    TRUE,
                                                                    G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                    G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_container_class_install_child_property (cclass, CHILD_PROP_EXPAND,
                                              g_param_spec_boolean ("expand",
                                                                    P_("Expand"),
                                                                    P_("Whether the item should receive extra space when the toolbar grows"),
                                                                    FALSE,
                                                                    G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                    G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_container_class_install_child_property (cclass, CHILD_PROP_FILL,
                                              g_param_spec_boolean ("fill",
                                                                    P_("Fill"),
                                                                    P_("Whether the item should fill the avaiable space"),
                                                                    TRUE,
                                                                    G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                    G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_container_class_install_child_property (cclass, CHILD_PROP_NEW_ROW,
                                              g_param_spec_boolean ("new-row",
                                                                    P_("New Row"),
                                                                    P_("Whether the item should start a new row"),
                                                                    FALSE,
                                                                    G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                    G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));

  gtk_container_class_install_child_property (cclass, CHILD_PROP_POSITION,
                                              g_param_spec_int ("position",
                                                                P_("Position"),
                                                                P_("Position of the item within this group"),
                                                                0,
                                                                G_MAXINT,
                                                                0,
                                                                G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
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

      if (name && group->priv->children)
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

  if (group->priv->collapsed)
    {
      if (group->priv->expander_style == GTK_EXPANDER_EXPANDED)
        group->priv->expander_style = GTK_EXPANDER_SEMI_COLLAPSED;
      else
        group->priv->expander_style = GTK_EXPANDER_COLLAPSED;
    }
  else
    {
      if (group->priv->expander_style == GTK_EXPANDER_COLLAPSED)
        group->priv->expander_style = GTK_EXPANDER_SEMI_EXPANDED;
      else
        group->priv->expander_style = GTK_EXPANDER_EXPANDED;
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
egg_tool_item_group_set_collapsed (EggToolItemGroup *group,
                                   gboolean          collapsed)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  if (collapsed != group->priv->collapsed)
    {
      GTimeVal now;
      GtkWidget *parent;

      g_get_current_time (&now);

      if (group->priv->animation_timeout)
        g_source_destroy (group->priv->animation_timeout);

      group->priv->collapsed = collapsed;
      group->priv->animation_start = (now.tv_sec * G_USEC_PER_SEC + now.tv_usec);
      group->priv->animation_timeout = g_timeout_source_new (ANIMATION_TIMEOUT);

      parent = gtk_widget_get_parent (GTK_WIDGET (group));
      if (EGG_IS_TOOL_PALETTE (parent) && !collapsed)
        _egg_tool_palette_set_expanding_child (EGG_TOOL_PALETTE (parent), GTK_WIDGET (group));

      g_source_set_callback (group->priv->animation_timeout,
                             egg_tool_item_group_animation_cb,
                             group, NULL);

      g_source_attach (group->priv->animation_timeout, NULL);
      g_object_notify (G_OBJECT (group), "collapsed");
    }
}

void
egg_tool_item_group_set_ellipsize (EggToolItemGroup   *group,
                                   PangoEllipsizeMode  ellipsize)
{
  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));

  if (ellipsize != group->priv->ellipsize)
    {
      group->priv->ellipsize = ellipsize;
      egg_tool_item_group_header_adjust_style (group);
      g_object_notify (G_OBJECT (group), "ellipsize");
#ifdef HAVE_EXTENDED_TOOL_SHELL_SUPPORT_BUG_535090
      _egg_tool_item_group_palette_reconfigured (group);
#endif
    }
}

G_CONST_RETURN gchar*
egg_tool_item_group_get_name (EggToolItemGroup *group)
{
  GtkWidget *label;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), DEFAULT_NAME);

  label = egg_tool_item_group_get_label (group);
  return gtk_label_get_text (GTK_LABEL (label));
}

gboolean
egg_tool_item_group_get_collapsed (EggToolItemGroup *group)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), DEFAULT_COLLAPSED);
  return group->priv->collapsed;
}

PangoEllipsizeMode
egg_tool_item_group_get_ellipsize (EggToolItemGroup *group)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), DEFAULT_ELLIPSIZE);
  return group->priv->ellipsize;
}

void
egg_tool_item_group_insert (EggToolItemGroup *group,
                            GtkToolItem      *item,
                            gint              position)
{
  GtkWidget *parent;
  EggToolItemGroupChild *child;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));
  g_return_if_fail (position >= -1);

  parent = gtk_widget_get_parent (GTK_WIDGET (group));

  child = g_new (EggToolItemGroupChild, 1);
  child->item = g_object_ref_sink (item);
  child->homogeneous = TRUE;
  child->expand = FALSE;
  child->fill = TRUE;
  child->new_row = FALSE;

  group->priv->children = g_list_insert (group->priv->children, child, position);

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
  GList *link;
  EggToolItemGroupChild *child;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));

  g_return_if_fail (position >= -1);

  child = egg_tool_item_group_get_child (group, item, &old_position, &link);

  g_return_if_fail (child != NULL);

  if (position == old_position)
    return;

  group->priv->children = g_list_delete_link (group->priv->children, link);
  group->priv->children = g_list_insert (group->priv->children, child, position);

  gtk_widget_child_notify (GTK_WIDGET (item), "position");
  if (GTK_WIDGET_VISIBLE (group) && GTK_WIDGET_VISIBLE (item))
    gtk_widget_queue_resize (GTK_WIDGET (group));
}

gint
egg_tool_item_group_get_item_position (EggToolItemGroup *group,
                                       GtkToolItem      *item)
{
  gint position;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), -1);
  g_return_val_if_fail (GTK_IS_TOOL_ITEM (item), -1);

  if (egg_tool_item_group_get_child (group, item, &position, NULL))
    return position;

  return -1;
}

guint
egg_tool_item_group_get_n_items (EggToolItemGroup *group)
{
  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), 0);

  return g_list_length (group->priv->children);
}

GtkToolItem*
egg_tool_item_group_get_nth_item (EggToolItemGroup *group,
                                  guint             index)
{
  EggToolItemGroupChild *child;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);

  child = g_list_nth_data (group->priv->children, index);

  return child != NULL ? child->item : NULL;
}

GtkToolItem*
egg_tool_item_group_get_drop_item (EggToolItemGroup *group,
                                   gint              x,
                                   gint              y)
{
  GtkAllocation *allocation;
  GtkOrientation orientation;
  GList *it;

  g_return_val_if_fail (EGG_IS_TOOL_ITEM_GROUP (group), NULL);

  allocation = &GTK_WIDGET (group)->allocation;
  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));

  g_return_val_if_fail (x >= 0 && x < allocation->width, NULL);
  g_return_val_if_fail (y >= 0 && y < allocation->height, NULL);

  for (it = group->priv->children; it != NULL; it = it->next)
    {
      EggToolItemGroupChild *child = it->data;
      GtkToolItem *item = child->item;
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
                                        GtkRequisition   *item_size,
                                        gboolean          homogeneous_only,
                                        guint            *requested_rows)
{
  GtkRequisition child_requisition;
  GList *it;
  guint rows = 0;
  gboolean new_row = TRUE;
  GtkOrientation orientation;

  g_return_if_fail (EGG_IS_TOOL_ITEM_GROUP (group));
  g_return_if_fail (NULL != item_size);

  orientation = gtk_tool_shell_get_orientation (GTK_TOOL_SHELL (group));
  item_size->width = item_size->height = 0;

  for (it = group->priv->children; it != NULL; it = it->next)
    {
      EggToolItemGroupChild *child = it->data;

      if (!egg_tool_item_group_is_item_visible (child->item, orientation))
        continue;

      if (child->new_row || new_row)
        {
          rows++;
          new_row = FALSE;
        }

      if (!child->homogeneous && child->expand)
          new_row = TRUE;

      gtk_widget_size_request (GTK_WIDGET (child->item), &child_requisition);

      if (!homogeneous_only || child->homogeneous)
        item_size->width = MAX (item_size->width, child_requisition.width);
      item_size->height = MAX (item_size->height, child_requisition.height);
    }

  if (requested_rows)
    *requested_rows = rows;
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

gint
_egg_tool_item_group_get_size_for_limit (EggToolItemGroup *group,
                                         gint              limit,
                                         gboolean          vertical,
                                         gboolean          animation)
{
  GtkRequisition requisition;

  gtk_widget_size_request (GTK_WIDGET (group), &requisition);

  if (!group->priv->collapsed || group->priv->animation_timeout)
    {
      GtkAllocation allocation = { 0, 0, requisition.width, requisition.height };
      GtkRequisition inquery;

      if (vertical)
        allocation.width = limit;
      else
        allocation.height = limit;

      egg_tool_item_group_real_size_query (GTK_WIDGET (group),
                                           &allocation, &inquery);

      if (vertical)
        inquery.height -= requisition.height;
      else
        inquery.width -= requisition.width;

      if (group->priv->animation_timeout && animation)
        {
          gint64 timestamp = egg_tool_item_group_get_animation_timestamp (group);

          timestamp = MIN (timestamp, ANIMATION_DURATION);

          if (group->priv->collapsed)
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
  return _egg_tool_item_group_get_size_for_limit (group, width, TRUE, TRUE);
}

gint
_egg_tool_item_group_get_width_for_height (EggToolItemGroup *group,
                                           gint              height)
{
  return _egg_tool_item_group_get_size_for_limit (group, height, FALSE, TRUE);
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
