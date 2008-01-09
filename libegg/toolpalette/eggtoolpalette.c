#include "eggtoolpalette.h"
#include "eggmarshalers.h"

#include <gtk/gtk.h>
#include <string.h>

#define DEFAULT_HEADER_SPACING  2
#define DEFAULT_EXPANDER_SIZE   16

typedef struct _EggToolPaletteCategory EggToolPaletteCategory;
typedef struct _EggToolPaletteCallbackData EggToolPaletteCallbackData;

enum
{
  PROP_NONE
  /* TODO: fill in properties */
};

struct _EggToolPaletteCategory
{
  gchar            *id;
  GArray           *items;

  GtkWidget        *header_button;
  GtkWidget        *header_label;
  GtkExpanderStyle  expander_style;
  guint             animation_timeout;

  GdkWindow        *window;

  guint             expanded : 1;
};

struct _EggToolPalettePrivate
{
  EggToolPaletteCategory *current_category;
  GArray                 *categories;

  GtkAdjustment          *hadjustment;
  GtkAdjustment          *vadjustment;

  gint                    header_spacing;
  gint                    expander_size;

  guint                   is_drag_source : 1;
};

struct _EggToolPaletteCallbackData
{
  EggToolPaletteCallback callback;
  gpointer               user_data;
};

static GtkTargetEntry dnd_targets[] =
{
  { "application/x-egg-tool-palette-item", GTK_TARGET_SAME_APP, 0 }
};

G_DEFINE_TYPE (EggToolPalette, egg_tool_palette, GTK_TYPE_CONTAINER);

static EggToolPaletteCategory*
egg_tool_palette_get_category (EggToolPalette *self,
                               guint           index)
{
  g_return_val_if_fail (index < self->priv->categories->len, NULL);

  return g_array_index (self->priv->categories,
                        EggToolPaletteCategory*,
                        index);
}

static EggToolPaletteCategory*
egg_tool_palette_find_category (EggToolPalette *self,
                                const gchar    *id)
{
  guint i;

  if (NULL == id)
    id = "";

  if (self->priv->current_category &&
      strcmp (id, self->priv->current_category->id))
    self->priv->current_category = NULL;

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);

      if (g_str_equal (id, category->id))
        {
          self->priv->current_category = category;
          break;
        }
    }

  return self->priv->current_category;
}

static gboolean
egg_tool_palette_header_expose_event (GtkWidget      *widget,
                                      GdkEventExpose *event G_GNUC_UNUSED,
                                      gpointer        data)
{
  EggToolPaletteCategory *category = data;
  EggToolPalette *self;
  GtkWidget *palette;
  gint x, y;

  palette = gtk_widget_get_parent (category->header_button);
  self = EGG_TOOL_PALETTE (palette);

  x = widget->allocation.x + self->priv->expander_size / 2;
  y = widget->allocation.y + widget->allocation.height / 2;

  gtk_paint_expander (widget->style, widget->window,
                      category->header_button->state,
                      &event->area, palette, NULL,
                      x, y, category->expander_style);

  return FALSE;
}

static gboolean
egg_tool_palette_expand_animation (gpointer data)
{
  EggToolPaletteCategory *category = data;
  GtkWidget *palette = gtk_widget_get_parent (category->header_button);
  gboolean finish = TRUE;

  if (GTK_WIDGET_REALIZED (category->header_button))
    {
      GtkWidget *alignment = gtk_bin_get_child (GTK_BIN (category->header_button));
      EggToolPalette *self = EGG_TOOL_PALETTE (palette);
      GdkRectangle area;

      area.x = alignment->allocation.x;
      area.y = alignment->allocation.y + (alignment->allocation.height - self->priv->expander_size) / 2;
      area.height = self->priv->expander_size;
      area.width = self->priv->expander_size;

      gdk_window_invalidate_rect (category->header_button->window, &area, TRUE);
    }

  if (category->expanded)
    {
      if (category->expander_style == GTK_EXPANDER_COLLAPSED)
        {
          category->expander_style = GTK_EXPANDER_SEMI_EXPANDED;
          finish = FALSE;
        }
      else
        category->expander_style = GTK_EXPANDER_EXPANDED;
    }
  else
    {
      if (category->expander_style == GTK_EXPANDER_EXPANDED)
        {
          category->expander_style = GTK_EXPANDER_SEMI_COLLAPSED;
          finish = FALSE;
        }
      else
        category->expander_style = GTK_EXPANDER_COLLAPSED;
    }

  if (finish)
    {
      category->animation_timeout = 0;
      gtk_widget_queue_resize (palette);
    }

  return !finish;
}

static void
egg_tool_palette_category_set_expanded (EggToolPaletteCategory *category,
                                        gboolean                expanded)
{
  if (category->animation_timeout)
    g_source_remove (category->animation_timeout);

  category->expanded = expanded;
  category->animation_timeout = g_timeout_add (50, egg_tool_palette_expand_animation, category);
}

static void
egg_tool_palette_header_clicked (GtkButton *button G_GNUC_UNUSED,
                                 gpointer   data)
{
  EggToolPaletteCategory *category = data;
  egg_tool_palette_category_set_expanded (category, !category->expanded);
}

static EggToolPaletteCategory*
egg_tool_palette_create_category (EggToolPalette *self,
                                  const gchar    *id,
                                  const gchar    *name)
{
  EggToolPaletteCategory *category;
  GtkWidget *alignment;

  g_assert (NULL != id);

  category = g_slice_new0 (EggToolPaletteCategory);
  category->items = g_array_new (TRUE, TRUE, sizeof (GtkToolItem*));
  category->id = g_strdup (id);

  category->expanded = TRUE;
  category->expander_style = GTK_EXPANDER_EXPANDED;

  gtk_widget_push_composite_child ();

  category->header_button = gtk_button_new ();
  category->header_label  = gtk_label_new (name);

  gtk_widget_set_composite_name (category->header_button, "header-button");
  gtk_widget_set_composite_name (category->header_button, "header-label");
  gtk_widget_set_composite_name (category->header_button, "header-arrow");

  gtk_widget_pop_composite_child ();

  alignment = gtk_alignment_new (0.0, 0.5, 0.0, 0.0);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0,
                             self->priv->header_spacing +
                             self->priv->expander_size, 0);
  gtk_widget_set_size_request (alignment, -1, self->priv->expander_size);
  gtk_container_add (GTK_CONTAINER (alignment), category->header_label);
  gtk_widget_show_all (alignment);

  gtk_button_set_focus_on_click (GTK_BUTTON (category->header_button), FALSE);
  gtk_container_add (GTK_CONTAINER (category->header_button), alignment);
  gtk_widget_set_parent (category->header_button, GTK_WIDGET (self));

  g_signal_connect_after (alignment, "expose-event",
                          G_CALLBACK (egg_tool_palette_header_expose_event),
                          category);

  g_signal_connect (category->header_button, "clicked",
                    G_CALLBACK (egg_tool_palette_header_clicked),
                    category);

  if (name)
    gtk_widget_show (category->header_button);

  g_array_append_val (self->priv->categories, category);
  self->priv->current_category = category;
  return self->priv->current_category;
}

static void
egg_tool_palette_category_free (EggToolPaletteCategory *category)
{
  if (category->header_button)
    gtk_widget_unparent (category->header_button);
  if (category->window)
    g_object_unref (category->window);
  if (category->items)
    g_array_free (category->items, TRUE);

  g_slice_free (EggToolPaletteCategory, category);
}

static GtkToolItem*
egg_tool_palette_category_get_child (EggToolPaletteCategory *category,
                                     guint                   index)
{
  g_return_val_if_fail (index < category->items->len, NULL);
  return g_array_index (category->items, GtkToolItem*, index);
}

static void
egg_tool_palette_init (EggToolPalette *self)
{
  gtk_widget_set_redraw_on_allocate (GTK_WIDGET (self), FALSE);
#if 0
  GTK_WIDGET_SET_FLAGS (self, GTK_NO_WINDOW);
#endif

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                                            EGG_TYPE_TOOL_PALETTE,
                                            EggToolPalettePrivate);

  self->priv->categories = g_array_new (TRUE, TRUE, sizeof (EggToolPaletteCategory*));
  egg_tool_palette_create_category (self, "", NULL);
}

#if 0
static void
egg_tool_palette_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  EggToolPalette *self = EGG_TOOL_PALETTE (object);

  switch (prop_id)
    {
      /* TODO: handle properties */

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
  EggToolPalette *self = EGG_TOOL_PALETTE (object);

  switch (prop_id)
    {
      /* TODO: handle properties */

      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}
#endif

static void
egg_tool_palette_dispose (GObject *object)
{
  EggToolPalette *self = EGG_TOOL_PALETTE (object);
  guint i;

  gtk_container_forall (GTK_CONTAINER (self), (GtkCallback) g_object_unref, NULL);

  if (self->priv->hadjustment)
    {
      g_object_unref (self->priv->hadjustment);
      self->priv->hadjustment = NULL;
    }

  if (self->priv->vadjustment)
    {
      g_object_unref (self->priv->vadjustment);
      self->priv->vadjustment = NULL;
    }

  if (self->priv->categories)
    {
      for (i = 0; i < self->priv->categories->len; ++i)
        egg_tool_palette_category_free (egg_tool_palette_get_category (self, i));

      g_array_free (self->priv->categories, TRUE);
      self->priv->categories = NULL;
    }

  G_OBJECT_CLASS (egg_tool_palette_parent_class)->dispose (object);
}

static void
egg_tool_palette_realize (GtkWidget *widget)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  GdkWindowAttr attributes;

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

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

  gtk_container_forall (GTK_CONTAINER (widget),
                        (GtkCallback) gtk_widget_set_parent_window,
                        widget->window);

  gtk_widget_queue_resize_no_redraw (widget);
}

static void
egg_tool_palette_unrealize (GtkWidget *widget)
{
  EggToolPalette *self = EGG_TOOL_PALETTE (widget);
  guint i;

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);

      if (category->animation_timeout)
        {
          g_source_remove (category->animation_timeout);
          category->animation_timeout = 0;
        }

      if (category->window)
        {
          g_object_unref (category->window);
          category->window = NULL;
        }
    }
}

static void
egg_tool_palette_size_request (GtkWidget      *widget,
                               GtkRequisition *requisition)
{
  EggToolPalette *self = EGG_TOOL_PALETTE (widget);
  guint i;

  requisition->width = requisition->height = 0;

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);
      GtkRequisition child_requistion;

      if (GTK_WIDGET_VISIBLE (category->header_button))
        {
          gtk_widget_size_request (category->header_button, &child_requistion);
          requisition->width = MAX (requisition->width, child_requistion.width);
          requisition->height += child_requistion.height;
        }
    }

  requisition->width += GTK_CONTAINER (self)->border_width * 2;
  requisition->height += GTK_CONTAINER (self)->border_width * 2;
}

static void
egg_tool_palette_size_allocate (GtkWidget     *widget,
                                GtkAllocation *allocation)
{
  const gint border_width = GTK_CONTAINER (widget)->border_width;
  EggToolPalette *self = EGG_TOOL_PALETTE (widget);
  GtkRequisition child_requistion;
  GtkAllocation child_allocation;
  gint item_height = 0;
  gint item_width = 0;
  gint offset = 0;
  guint i, j;

  GTK_WIDGET_CLASS (egg_tool_palette_parent_class)->size_allocate (widget, allocation);

  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
                              allocation->x      + border_width,
                              allocation->y      + border_width,
                              allocation->width  - border_width * 2,
                              allocation->height - border_width * 2);
    }

  if (self->priv->vadjustment)
    offset = -gtk_adjustment_get_value (self->priv->vadjustment);

  child_allocation.x = border_width;
  child_allocation.y = border_width + offset;

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);

      for (j = 0; j < category->items->len; ++j)
        {
          GtkToolItem *child = egg_tool_palette_category_get_child (category, j);

          gtk_widget_size_request (GTK_WIDGET (child), &child_requistion);

          item_width = MAX (item_width, child_requistion.width);
          item_height = MAX (item_height, child_requistion.height);
        }
    }

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);

      if (!category->items->len)
        continue;

      if (GTK_WIDGET_VISIBLE (category->header_button))
        {
          gtk_widget_size_request (category->header_button, &child_requistion);

          child_allocation.width  = allocation->width - border_width * 2;
          child_allocation.height = child_requistion.height;

          gtk_widget_size_allocate (category->header_button, &child_allocation);

          child_allocation.y += child_allocation.height;
        }

      if (category->expanded)
        {
          for (j = 0; j < category->items->len; ++j)
            {
              GtkToolItem *child = egg_tool_palette_category_get_child (category, j);

              child_allocation.width = item_width;
              child_allocation.height = item_height;

              if (child_allocation.x + child_allocation.width > allocation->width)
                {
                  child_allocation.y += child_allocation.height;
                  child_allocation.x = border_width;
                }

              gtk_widget_size_allocate (GTK_WIDGET (child), &child_allocation);
              gtk_widget_show (GTK_WIDGET (child));

              child_allocation.x += child_allocation.width;
            }

          child_allocation.y += item_height;
          child_allocation.x = border_width;
        }
      else
        {
          for (j = 0; j < category->items->len; ++j)
            {
              GtkToolItem *child = egg_tool_palette_category_get_child (category, j);
              gtk_widget_hide (GTK_WIDGET (child));
            }
        }
    }

  child_allocation.y += border_width;

  if (self->priv->vadjustment)
    {
      self->priv->vadjustment->page_size = allocation->height;
      self->priv->vadjustment->page_increment = allocation->height * 0.9;
      self->priv->vadjustment->step_increment = allocation->height * 0.1;
      self->priv->vadjustment->upper = MAX (0, child_allocation.y - offset);

      gtk_adjustment_changed (self->priv->vadjustment);
      gtk_adjustment_clamp_page (self->priv->vadjustment, -offset, allocation->height -offset);
    }

  if (GTK_WIDGET_MAPPED (widget))
    gdk_window_invalidate_rect (widget->window, NULL, FALSE);
}

static gboolean
egg_tool_palette_expose_event (GtkWidget      *widget,
                               GdkEventExpose *event)
{
  EggToolPalette *self =  EGG_TOOL_PALETTE (widget);
  guint i;

  if (GTK_WIDGET_CLASS (egg_tool_palette_parent_class)->expose_event (widget, event))
    return TRUE;

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);

      if (category->window)
        {
          /* TODO: draw composited window */
        }
    }

  return FALSE;
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
  EggToolPalette *self = EGG_TOOL_PALETTE (widget);

  if (self->priv->hadjustment)
    g_object_unref (self->priv->hadjustment);
  if (self->priv->vadjustment)
    g_object_unref (self->priv->vadjustment);

  if (hadjustment)
    g_object_ref_sink (hadjustment);
  if (vadjustment)
    g_object_ref_sink (vadjustment);

  self->priv->hadjustment = hadjustment;
  self->priv->vadjustment = vadjustment;

  if (self->priv->hadjustment)
    g_signal_connect (self->priv->hadjustment, "value-changed",
                      G_CALLBACK (egg_tool_palette_adjustment_value_changed),
                      self);
  if (self->priv->vadjustment)
    g_signal_connect (self->priv->vadjustment, "value-changed",
                      G_CALLBACK (egg_tool_palette_adjustment_value_changed),
                      self);
}

static void
egg_tool_palette_style_set (GtkWidget *widget,
                            GtkStyle  *previous_style)
{
  EggToolPalette *self = EGG_TOOL_PALETTE (widget);
  guint i;

  self->priv->header_spacing = DEFAULT_HEADER_SPACING; /* TODO: use real style property */
  self->priv->expander_size = DEFAULT_EXPANDER_SIZE; /* TODO: use real style property */

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);
      GtkWidget *alignment = NULL;

      if (category->header_button)
        {
          alignment = gtk_bin_get_child (GTK_BIN (category->header_button));

          gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0,
                                     self->priv->header_spacing +
                                     self->priv->expander_size, 0);
          gtk_widget_set_size_request (alignment, -1, self->priv->expander_size);
        }
    }

  GTK_WIDGET_CLASS (egg_tool_palette_parent_class)->style_set (widget, previous_style);
}

static void
egg_tool_palette_add (GtkContainer *container G_GNUC_UNUSED,
                      GtkWidget    *child     G_GNUC_UNUSED)
{
  g_return_if_reached ();
}

static void
egg_tool_palette_remove (GtkContainer *container G_GNUC_UNUSED,
                         GtkWidget    *child     G_GNUC_UNUSED)
{
  g_return_if_reached ();
}

static void
egg_tool_palette_forall (GtkContainer *container,
                         gboolean      internals,
                         GtkCallback   callback,
                         gpointer      callback_data)
{
  EggToolPalette *self = EGG_TOOL_PALETTE (container);
  guint i, j;

  if (NULL == self->priv->categories)
    return;

  for (i = 0; i < self->priv->categories->len; ++i)
    {
      EggToolPaletteCategory *category = egg_tool_palette_get_category (self, i);

      if (internals && category->header_button)
        callback (category->header_button, callback_data);

      for (j = 0; j < category->items->len; ++j)
        {
          GtkToolItem *child = egg_tool_palette_category_get_child (category, j);
          callback (GTK_WIDGET (child), callback_data);
        }
    }
}

static GType
egg_tool_palette_child_type (GtkContainer *container G_GNUC_UNUSED)
{
  return GTK_TYPE_TOOL_ITEM;
}

static void
egg_tool_palette_class_init (EggToolPaletteClass *cls)
{
  GObjectClass      *oclass = G_OBJECT_CLASS (cls);
  GtkWidgetClass    *wclass = GTK_WIDGET_CLASS (cls);
  GtkContainerClass *cclass = GTK_CONTAINER_CLASS (cls);

#if 0
  oclass->set_property        = egg_tool_palette_set_property;
  oclass->get_property        = egg_tool_palette_get_property;
#endif
  oclass->dispose             = egg_tool_palette_dispose;

  wclass->realize             = egg_tool_palette_realize;
  wclass->unrealize           = egg_tool_palette_unrealize;
  wclass->size_request        = egg_tool_palette_size_request;
  wclass->size_allocate       = egg_tool_palette_size_allocate;
  wclass->expose_event        = egg_tool_palette_expose_event;
  wclass->style_set           = egg_tool_palette_style_set;

  cclass->add                 = egg_tool_palette_add;
  cclass->remove              = egg_tool_palette_remove;
  cclass->forall              = egg_tool_palette_forall;
  cclass->child_type          = egg_tool_palette_child_type;

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

  g_type_class_add_private (cls, sizeof (EggToolPalettePrivate));
}

/* ===== instance handling ===== */

GtkWidget*
egg_tool_palette_new (void)
{
  return g_object_new (EGG_TYPE_TOOL_PALETTE, NULL);
}

/* ===== category settings ===== */

void
egg_tool_palette_set_category_name (EggToolPalette *self,
                                    const gchar    *category,
                                    const gchar    *name G_GNUC_UNUSED)
{
  EggToolPaletteCategory *category_info;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (NULL != category);

  category_info = egg_tool_palette_find_category (self, category);

  if (category_info)
    {
      gtk_label_set_text (GTK_LABEL (category_info->header_label), name);
      gtk_widget_show (category_info->header_button);
    }
}

void
egg_tool_palette_set_category_position (EggToolPalette *self,
                                        const gchar    *category,
                                        gint            position G_GNUC_UNUSED)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (NULL != category);
  g_return_if_reached ();
}

void
egg_tool_palette_set_category_expanded (EggToolPalette *self,
                                        const gchar    *category,
                                        gboolean        expanded)
{
  EggToolPaletteCategory *category_info;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (NULL != category);

  category_info = egg_tool_palette_find_category (self, category);
  g_return_if_fail (NULL != category_info);

  egg_tool_palette_category_set_expanded (category_info, expanded);
}

G_CONST_RETURN gchar*
egg_tool_palette_get_category_name (EggToolPalette *self,
                                    const gchar    *category)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), NULL);
  g_return_val_if_fail (NULL != category, NULL);
  g_return_val_if_reached (NULL);
}

gint
egg_tool_palette_get_category_position (EggToolPalette *self,
                                        const gchar    *category)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), 0);
  g_return_val_if_fail (NULL != category, 0);
  g_return_val_if_reached (0);
}

gboolean
egg_tool_palette_get_category_expanded (EggToolPalette *self,
                                        const gchar    *category)
{
  EggToolPaletteCategory *category_info;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), FALSE);
  g_return_val_if_fail (NULL != category, FALSE);

  category_info = egg_tool_palette_find_category (self, category);
  g_return_val_if_fail (NULL != category_info, FALSE);

  return category_info->expanded;
}

/* ===== item packing ===== */

static void
egg_tool_palette_item_drag_data_get (GtkWidget        *widget,
                                     GdkDragContext   *context G_GNUC_UNUSED,
                                     GtkSelectionData *selection,
                                     guint             info G_GNUC_UNUSED,
                                     guint             time G_GNUC_UNUSED,
                                     gpointer          data G_GNUC_UNUSED)
{
  const gchar *target_name = gdk_atom_name (selection->target);

  if (g_str_equal (target_name, dnd_targets[0].target))
    gtk_selection_data_set (selection, selection->target, 8,
                            (guchar*) &widget, sizeof (&widget));
}

static void
egg_tool_palette_item_set_drag_source (GtkWidget *widget,
                                       gpointer   data)
{
  gtk_tool_item_set_use_drag_window (GTK_TOOL_ITEM (widget), TRUE);
  gtk_drag_source_set (widget, GDK_BUTTON1_MASK | GDK_BUTTON3_MASK,
                       dnd_targets, G_N_ELEMENTS (dnd_targets),
                       GDK_ACTION_COPY);

  g_signal_connect (widget, "drag-data-get",
                    G_CALLBACK (egg_tool_palette_item_drag_data_get),
                    data);
}

void
egg_tool_palette_insert (EggToolPalette *self,
                         const gchar    *category,
                         GtkToolItem    *item,
                         gint            position)
{
  EggToolPaletteCategory *category_info;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));
  g_return_if_fail (position >= -1);

  category_info = egg_tool_palette_find_category (self, category);

  if (NULL == category_info)
    category_info = egg_tool_palette_create_category (self, category, NULL);
  if (-1 == position)
    position = category_info->items->len;

  g_return_if_fail ((guint) position <= category_info->items->len);
  g_array_insert_val (category_info->items, position, item);

  if (self->priv->is_drag_source)
    egg_tool_palette_item_set_drag_source (GTK_WIDGET (item), self);

  gtk_widget_set_parent (GTK_WIDGET (item), GTK_WIDGET (self));
  g_object_ref (item);
}

gint
egg_tool_palette_get_n_items (EggToolPalette *self,
                              const gchar    *category)
{
  EggToolPaletteCategory *category_info;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), 0);
  category_info = egg_tool_palette_find_category (self, category);

  if (NULL != category_info)
    return category_info->items->len;

  return 0;
}

GtkToolItem*
egg_tool_palette_get_nth_item (EggToolPalette *self,
                               const gchar    *category,
                               gint            index)
{
  EggToolPaletteCategory *category_info;

  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), NULL);
  category_info = egg_tool_palette_find_category (self, category);

  if (NULL != category_info)
    return egg_tool_palette_category_get_child (category_info, index);

  return 0;
}

GtkToolItem*
egg_tool_palette_get_drop_item (EggToolPalette *self,
                                gint            x G_GNUC_UNUSED,
                                gint            y G_GNUC_UNUSED)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), NULL);
  g_return_val_if_reached (NULL);
}

/* ===== item settings ===== */

void
egg_tool_palette_set_item_category (EggToolPalette *self,
                                    GtkToolItem    *item,
                                    const gchar    *category G_GNUC_UNUSED)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));
  g_return_if_reached ();
}

void
egg_tool_palette_set_item_position (EggToolPalette *self,
                                    GtkToolItem    *item,
                                    gint            position G_GNUC_UNUSED)
{
  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (GTK_IS_TOOL_ITEM (item));
  g_return_if_reached ();
}

G_CONST_RETURN gchar*
egg_tool_palette_get_item_category (EggToolPalette *self,
                                    GtkToolItem    *item)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), NULL);
  g_return_val_if_fail (GTK_IS_TOOL_ITEM (item), NULL);
  g_return_val_if_reached (NULL);
}

gint
egg_tool_palette_get_item_position (EggToolPalette *self,
                                    GtkToolItem    *item)
{
  g_return_val_if_fail (EGG_IS_TOOL_PALETTE (self), 0);
  g_return_val_if_fail (GTK_IS_TOOL_ITEM (item), 0);
  g_return_val_if_reached (0);
}

void
egg_tool_palette_set_drag_source (EggToolPalette *self)
{
  if (self->priv->is_drag_source)
    return;

  gtk_container_foreach (GTK_CONTAINER (self),
                         egg_tool_palette_item_set_drag_source,
                         self);

  self->priv->is_drag_source = TRUE;
}

static void
egg_tool_palette_drag_data_received (GtkWidget        *widget G_GNUC_UNUSED,
                                     GdkDragContext   *context G_GNUC_UNUSED,
                                     gint              x G_GNUC_UNUSED,
                                     gint              y G_GNUC_UNUSED,
                                     GtkSelectionData *selection G_GNUC_UNUSED,
                                     guint             info G_GNUC_UNUSED,
                                     guint             time G_GNUC_UNUSED,
                                     gpointer          data)
{
  GtkWidget *item = *(GtkWidget**) selection->data;
  EggToolPaletteCallbackData *callback_data = data;
  EggToolPalette *palette;

  if (GTK_IS_TOOL_ITEM (item))
    {
      palette = EGG_TOOL_PALETTE (gtk_widget_get_parent (item));
      callback_data->callback (palette, GTK_TOOL_ITEM (item), callback_data->user_data);
    }
}

static void
egg_tool_palette_callback_data_free (gpointer  data,
                                     GClosure *closure G_GNUC_UNUSED)
{
  g_slice_free (EggToolPaletteCallbackData, data);
}

void
egg_tool_palette_add_drag_dest (EggToolPalette         *self,
                                GtkWidget              *widget,
                                EggToolPaletteCallback  callback,
                                gpointer                user_data)
{
  EggToolPaletteCallbackData *callback_data;

  g_return_if_fail (EGG_IS_TOOL_PALETTE (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (NULL != callback);

  callback_data = g_slice_new (EggToolPaletteCallbackData);
  callback_data->user_data = user_data;
  callback_data->callback = callback;

  gtk_drag_dest_set (widget, GTK_DEST_DEFAULT_ALL,
                     dnd_targets, G_N_ELEMENTS (dnd_targets),
                     GDK_ACTION_COPY);

  g_signal_connect_data (widget, "drag-data-received",
                         G_CALLBACK (egg_tool_palette_drag_data_received),
                         callback_data, egg_tool_palette_callback_data_free, 0);
}

