#include "eggcellview.h"
#include <gtk/gtksignal.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>
#include <gobject/gmarshal.h>

#define _


typedef struct _EggCellViewCellInfo EggCellViewCellInfo;
struct _EggCellViewCellInfo
{
  GtkCellRenderer *cell;

  gint requested_width;
  gint real_width;
  guint expand : 1;
  guint pack : 1;

  GSList *attributes;
};


static void	egg_cell_view_class_init		(EggCellViewClass *klass);
static void	egg_cell_view_get_property		(GObject           *object,
							 guint             param_id,
							 GValue           *value,
							 GParamSpec       *pspec);
static void     egg_cell_view_set_property              (GObject          *object,
							 guint             param_id,
							 const GValue     *value,
							 GParamSpec       *pspec);
static void	egg_cell_view_init			(EggCellView      *cellview);
static void	egg_cell_view_finalize			(GObject          *object);
static void	egg_cell_view_style_set			(GtkWidget        *widget,
							 GtkStyle         *previous_style);
static void	egg_cell_view_size_request		(GtkWidget        *widget,
							 GtkRequisition   *requisition);
static void	egg_cell_view_size_allocate		(GtkWidget        *widget,
							 GtkAllocation    *allocation);
static gboolean	egg_cell_view_expose			(GtkWidget        *widget,
							 GdkEventExpose   *event);
static void     egg_cell_view_set_attributesv           (EggCellView      *cellview,
							 GtkCellRenderer  *renderer,
							 va_list           args);
static void	egg_cell_view_set_valuesv		(EggCellView      *cellview,
							 GtkCellRenderer  *renderer,
							 va_list           args);
static void     egg_cell_view_set_cell_data             (EggCellView      *cellview);

enum {
  PROP_0,
  PROP_BACKGROUND,
  PROP_BACKGROUND_GDK,
  PROP_BACKGROUND_SET
};

static GtkObjectClass *parent_class = NULL;


GType
egg_cell_view_get_type (void)
{
  static GType cell_view_type = 0;

  if (!cell_view_type)
    {
      static const GTypeInfo cell_view_info =
        {
	  sizeof (EggCellViewClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) egg_cell_view_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (EggCellView),
	  0,
	  (GInstanceInitFunc) egg_cell_view_init
        };

      cell_view_type = g_type_register_static (GTK_TYPE_WIDGET, "EggCellView",
                                               &cell_view_info, 0);
    }

  return cell_view_type;
}

static void
egg_cell_view_class_init (EggCellViewClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->get_property = egg_cell_view_get_property;
  gobject_class->set_property = egg_cell_view_set_property;
  gobject_class->finalize = egg_cell_view_finalize;

  widget_class->expose_event = egg_cell_view_expose;
  widget_class->size_allocate = egg_cell_view_size_allocate;
  widget_class->size_request = egg_cell_view_size_request;
  widget_class->style_set = egg_cell_view_style_set;

  /* properties */
  g_object_class_install_property (gobject_class,
				   PROP_BACKGROUND,
				   g_param_spec_string ("background",
							_("Background color name"),
							_("Background color as a string"),
							NULL,
							G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class,
				   PROP_BACKGROUND_GDK,
				   g_param_spec_boxed ("background_gdk",
						      _("Background color"),
						      _("Background color as a GdkColor"),
						      GDK_TYPE_COLOR,
						      G_PARAM_READABLE | G_PARAM_WRITABLE));

#define ADD_SET_PROP(propname, propval, nick, blurb) g_object_class_install_property (gobject_class, propval, g_param_spec_boolean (propname, nick, blurb, FALSE, G_PARAM_READABLE | G_PARAM_WRITABLE))

  ADD_SET_PROP ("background_set", PROP_BACKGROUND_SET,
                _("Background set"),
                _("Whether this tag affects the background color"));
}

static void
egg_cell_view_get_property (GObject    *object,
			    guint       param_id,
			    GValue     *value,
			    GParamSpec *pspec)
{
  EggCellView *view = EGG_CELL_VIEW (object);

  switch (param_id)
    {
      case PROP_BACKGROUND_GDK:
	{
	  GdkColor color;

	  color = view->background;

	  g_value_set_boxed (value, &color);
	}
	break;
      case PROP_BACKGROUND_SET:
	g_value_set_boolean (value, view->background_set);
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
	break;
    }
}

static void
egg_cell_view_set_property (GObject      *object,
			    guint         param_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
  EggCellView *view = EGG_CELL_VIEW (object);

  switch (param_id)
    {
      case PROP_BACKGROUND:
	{
	  GdkColor color;

	  if (!g_value_get_string (value))
	    egg_cell_view_set_background_color (view, NULL);
	  else if (gdk_color_parse (g_value_get_string (value), &color))
	    egg_cell_view_set_background_color (view, &color);
	  else
	    g_warning ("Don't know color `%s'", g_value_get_string (value));

	  g_object_notify (object, "background_gdk");
	}
	break;
      case PROP_BACKGROUND_GDK:
	egg_cell_view_set_background_color (view, g_value_get_boxed (value));
	break;
      case PROP_BACKGROUND_SET:
	view->background_set = g_value_get_boolean (value);
	break;
      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
	break;
    }
}

static void
egg_cell_view_init (EggCellView *cellview)
{
  GTK_WIDGET_SET_FLAGS (cellview, GTK_NO_WINDOW);
}

static void
egg_cell_view_map (GtkWidget *widget)
{
  EggCellView *cellview;

  g_return_if_fail (EGG_IS_CELL_VIEW (widget));

  cellview = EGG_CELL_VIEW (widget);

  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

  gdk_window_show (widget->window);

  if (GTK_WIDGET_CLASS (parent_class)->map)
    (* GTK_WIDGET_CLASS (parent_class)->map) (widget);
}

static void
egg_cell_view_realize (GtkWidget *widget)
{
  EggCellView *cellview;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (EGG_IS_CELL_VIEW (widget));

  cellview = EGG_CELL_VIEW (widget);

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask =
    GDK_EXPOSURE_MASK |
    GDK_BUTTON_PRESS_MASK |
    GDK_BUTTON_RELEASE_MASK |
    gtk_widget_get_events (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void
egg_cell_view_style_set (GtkWidget *widget,
                         GtkStyle  *previous_style)
{
  if (previous_style && GTK_WIDGET_REALIZED (widget))
    gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
}

static void
egg_cell_view_finalize (GObject *object)
{
  EggCellView *cellview = EGG_CELL_VIEW (object);

  if (cellview->cell_list)
    {
      g_list_foreach (cellview->cell_list, (GFunc)g_free, NULL);
      g_list_free (cellview->cell_list);
    }
  cellview->cell_list = NULL;

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
egg_cell_view_size_request (GtkWidget      *widget,
                            GtkRequisition *requisition)
{
  GList *i;
  gboolean first_cell = TRUE;
  EggCellView *cellview;

  cellview = EGG_CELL_VIEW (widget);

  widget->requisition.width = 0;
  widget->requisition.height = 0;

  if (cellview->displayed_row)
    egg_cell_view_set_cell_data (cellview);

  for (i = cellview->cell_list; i; i = i->next)
    {
      gint width, height;
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (!info->cell->visible)
	continue;

      if (!first_cell)
	widget->requisition.width += cellview->spacing;

      gtk_cell_renderer_get_size (info->cell, widget, NULL, NULL, NULL,
                                  &width, &height);

      info->requested_width = width;
      widget->requisition.width += width;
      widget->requisition.height = MAX (widget->requisition.height, height);

      first_cell = FALSE;
    }
}

static void
egg_cell_view_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
  GList *i;
  gint expand_cell_count = 0;
  gint full_requested_width = 0;
  gint extra_space;
  EggCellView *cellview;

  widget->allocation = *allocation;

  cellview = EGG_CELL_VIEW (widget);

  /* checking how much extra space we have */
  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (!info->cell->visible)
	continue;

      if (info->expand)
	expand_cell_count++;

      full_requested_width += info->requested_width;
    }

  extra_space = widget->allocation.width - full_requested_width;
  if (extra_space < 0)
    extra_space = 0;
  else if (extra_space > 0 && expand_cell_count > 0)
    extra_space /= expand_cell_count;

  /* iterate list for PACK_START cells */
  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (info->pack == GTK_PACK_END)
	continue;

      if (!info->cell->visible)
	continue;

      info->real_width = info->requested_width + (info->expand?extra_space:0);
    }

  /* iterate list for PACK_END cells */
  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (info->pack == GTK_PACK_START)
	continue;

      if (!info->cell->visible)
	continue;

      info->real_width = info->requested_width + (info->expand?extra_space:0);
    }
}

static gboolean
egg_cell_view_expose (GtkWidget      *widget,
                      GdkEventExpose *event)
{
  GList *i;
  EggCellView *cellview;
  GdkRectangle area;

  cellview = EGG_CELL_VIEW (widget);

  if (! GTK_WIDGET_DRAWABLE (widget))
    return FALSE;

  /* "blank" background */
  if (cellview->background_set)
    {
      GdkGC *gc;

      gc = gdk_gc_new (GTK_WIDGET (cellview)->window);
      gdk_gc_set_rgb_fg_color (gc, &cellview->background);

      gdk_draw_rectangle (GTK_WIDGET (cellview)->window,
			  gc,
			  TRUE,

			  /*0, 0,*/
			  widget->allocation.x,
			  widget->allocation.y,

			  widget->allocation.width,
			  widget->allocation.height);

      g_object_unref (G_OBJECT (gc));
    }

  /* set cell data (if applicable) */
  if (cellview->displayed_row)
    egg_cell_view_set_cell_data (cellview);

  /* render cells */
  area = widget->allocation;

  /* we draw on our very own window, initialize x and y to zero */
  area.x = widget->allocation.x;
  area.y = widget->allocation.y;

  /* PACK_START */
  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (info->pack == GTK_PACK_END)
	continue;

      if (!info->cell->visible)
	continue;

      area.width = info->real_width;

      gtk_cell_renderer_render (info->cell,
                                event->window,
				widget,
				/* FIXME! */
				&area, &area, &event->area, 0);

      area.x += info->real_width;
    }

  /* PACK_END */
  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (info->pack == GTK_PACK_START)
	continue;

      if (!info->cell->visible)
	continue;

      area.width = info->real_width;

      gtk_cell_renderer_render (info->cell,
                                widget->window,
				widget,
				/* FIXME ! */
				&area, &area, &event->area, 0);
      area.x += info->real_width;
    }

  return FALSE;
}

GtkWidget *
egg_cell_view_new ()
{
  EggCellView *cellview;

  cellview = EGG_CELL_VIEW (g_object_new (egg_cell_view_get_type (), NULL));

  return GTK_WIDGET (cellview);
}

GtkWidget *
egg_cell_view_new_with_text (const gchar *text)
{
  EggCellView *cellview;
  GtkCellRenderer *renderer;
  GValue value = {0, };

  cellview = EGG_CELL_VIEW (egg_cell_view_new ());

  renderer = gtk_cell_renderer_text_new ();
  egg_cell_view_pack_start (cellview, renderer, TRUE);

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, text);
  egg_cell_view_set_values (cellview, renderer, "text", &value, NULL);
  g_value_unset (&value);

  return GTK_WIDGET (cellview);
}

GtkWidget *
egg_cell_view_new_with_markup (const gchar *markup)
{
  EggCellView *cellview;
  GtkCellRenderer *renderer;
  GValue value = {0, };

  cellview = EGG_CELL_VIEW (egg_cell_view_new ());

  renderer = gtk_cell_renderer_text_new ();
  egg_cell_view_pack_start (cellview, renderer, TRUE);

  g_value_init (&value, G_TYPE_STRING);
  g_value_set_string (&value, markup);
  egg_cell_view_set_values (cellview, renderer, "markup", &value, NULL);
  g_value_unset (&value);

  return GTK_WIDGET (cellview);
}

GtkWidget *
egg_cell_view_new_with_pixbuf (GdkPixbuf *pixbuf)
{
  EggCellView *cellview;
  GtkCellRenderer *renderer;
  GValue value = {0, };

  cellview = EGG_CELL_VIEW (egg_cell_view_new ());

  renderer = gtk_cell_renderer_pixbuf_new ();
  egg_cell_view_pack_start (cellview, renderer, TRUE);

  g_value_init (&value, GDK_TYPE_PIXBUF);
  g_value_set_object (&value, pixbuf);
  egg_cell_view_set_values (cellview, renderer, "pixbuf", &value, NULL);
  g_value_unset (&value);

  return GTK_WIDGET (cellview);
}

#if 0
static void
egg_cell_view_clear (EggCellView *cellview)
{
  GList *i;

  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      egg_cell_view_clear_attributes (cellview, info->cell);
      g_object_unref (G_OBJECT (info->cell));
    }

  g_list_foreach (cellview->cell_list, (GFunc)g_free, NULL);
  g_list_free (cellview->cell_list);
  cellview->cell_list = NULL;
}
#endif

static EggCellViewCellInfo *
egg_cell_view_get_cell_info (EggCellView     *cellview,
			     GtkCellRenderer *renderer)
{
  GList *i;

  for (i = cellview->cell_list; i; i = i->next)
    {
      EggCellViewCellInfo *info = (EggCellViewCellInfo *)i->data;

      if (info->cell == renderer)
	return info;
    }

  return NULL;
}

void
egg_cell_view_pack_start (EggCellView     *cellview,
			  GtkCellRenderer *renderer,
                          gboolean         expand)
{
  EggCellViewCellInfo *info;

  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_CELL_RENDERER (renderer));
  g_return_if_fail (!egg_cell_view_get_cell_info (cellview, renderer));

  g_object_ref (G_OBJECT (renderer));
  gtk_object_sink (GTK_OBJECT (renderer));

  info = g_new0 (EggCellViewCellInfo, 1);
  info->cell = renderer;
  info->expand = expand ? TRUE : FALSE;
  info->pack = GTK_PACK_START;

  cellview->cell_list = g_list_append (cellview->cell_list, info);
}

void
egg_cell_view_pack_end (EggCellView     *cellview,
			GtkCellRenderer *renderer,
                        gboolean         expand)
{
  EggCellViewCellInfo *info;

  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_CELL_RENDERER (renderer));
  g_return_if_fail (!egg_cell_view_get_cell_info (cellview, renderer));

  g_object_ref (G_OBJECT (renderer));
  gtk_object_sink (GTK_OBJECT (renderer));

  info = g_new0 (EggCellViewCellInfo, 1);
  info->cell = renderer;
  info->expand = expand ? TRUE : FALSE;
  info->pack = GTK_PACK_END;

  cellview->cell_list = g_list_append (cellview->cell_list, info);
}

void
egg_cell_view_add_attribute (EggCellView     *cellview,
			     GtkCellRenderer *renderer,
			     const gchar     *attribute,
			     gint             column)
{
  EggCellViewCellInfo *info;

  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  info = egg_cell_view_get_cell_info (cellview, renderer);
  g_return_if_fail (info != NULL);

  info->attributes = g_slist_prepend (info->attributes,
				      GINT_TO_POINTER (column));
  info->attributes = g_slist_prepend (info->attributes,
				      g_strdup (attribute));
}

void
egg_cell_view_clear_attributes (EggCellView     *cellview,
                                GtkCellRenderer *renderer)
{
  EggCellViewCellInfo *info;
  GSList *list;

  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_CELL_RENDERER (renderer));

  info = egg_cell_view_get_cell_info (cellview, renderer);

  list = info->attributes;

  while (list && list->next)
    {
      g_free (list->data);
      list = list->next->next;
    }
  g_slist_free (list);

  info->attributes = NULL;
}

static void
egg_cell_view_set_attributesv (EggCellView     *cellview,
			       GtkCellRenderer *renderer,
			       va_list          args)
{
  gchar *attribute;
  gint column;

  attribute = va_arg (args, gchar *);

  egg_cell_view_clear_attributes (cellview, renderer);

  while (attribute)
    {
      column = va_arg (args, gint);
      egg_cell_view_add_attribute (cellview, renderer, attribute, column);
      attribute = va_arg (args, gchar *);
    }
}

void
egg_cell_view_set_attributes (EggCellView     *cellview,
			      GtkCellRenderer *renderer,
			      ...)
{
  va_list args;

  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_CELL_RENDERER (renderer));
  g_return_if_fail (egg_cell_view_get_cell_info (cellview, renderer));

  va_start (args, renderer);
  egg_cell_view_set_attributesv (cellview, renderer, args);
  va_end (args);
}

void
egg_cell_view_set_value (EggCellView     *cellview,
			 GtkCellRenderer *renderer,
			 gchar           *property,
			 GValue          *value)
{
  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_CELL_RENDERER (renderer));

  g_object_set_property (G_OBJECT (renderer), property, value);

  /* force redraw */
  gtk_widget_queue_draw (GTK_WIDGET (cellview));
}

static void
egg_cell_view_set_valuesv (EggCellView     *cellview,
			   GtkCellRenderer *renderer,
			   va_list          args)
{
  gchar *attribute;
  GValue *value;

  attribute = va_arg (args, gchar *);

  while (attribute)
    {
      value = va_arg (args, GValue *);
      egg_cell_view_set_value (cellview, renderer, attribute, value);
      attribute = va_arg (args, gchar *);
    }
}

void
egg_cell_view_set_values (EggCellView     *cellview,
                          GtkCellRenderer *renderer,
			  ...)
{
  va_list args;

  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_CELL_RENDERER (renderer));
  g_return_if_fail (egg_cell_view_get_cell_info (cellview, renderer));

  va_start (args, renderer);
  egg_cell_view_set_valuesv (cellview, renderer, args);
  va_end (args);
}

void
egg_cell_view_set_model (EggCellView  *cellview,
                         GtkTreeModel *model)
{
  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));

  if (cellview->model)
    {
      if (cellview->displayed_row)
	gtk_tree_row_reference_free (cellview->displayed_row);
      cellview->displayed_row = NULL;

      g_object_unref (G_OBJECT (cellview->model));
      cellview->model = NULL;
    }

  cellview->model = model;

  if (cellview->model)
    g_object_ref (G_OBJECT (cellview->model));
}

static void
egg_cell_view_set_cell_data (EggCellView *cellview)
{
  GList *i;
  GtkTreeIter iter;

  g_return_if_fail (cellview->displayed_row != NULL);

  gtk_tree_model_get_iter (cellview->model, &iter,
			   gtk_tree_row_reference_get_path (cellview->displayed_row));

  for (i = cellview->cell_list; i; i = i->next)
    {
      GSList *j;
      EggCellViewCellInfo *info = i->data;

      for (j = info->attributes; j && j->next; j = j->next->next)
        {
	  gchar *property = j->data;
	  gint column = GPOINTER_TO_INT (j->next->data);
	  GValue value = {0, };

	  gtk_tree_model_get_value (cellview->model, &iter,
				    column, &value);
	  g_object_set_property (G_OBJECT (info->cell),
				 property, &value);
	  g_value_unset (&value);
	}
    }
}

void
egg_cell_view_set_displayed_row (EggCellView *cellview,
				 GtkTreePath *path)
{
  g_return_if_fail (EGG_IS_CELL_VIEW (cellview));
  g_return_if_fail (GTK_IS_TREE_MODEL (cellview->model));
  g_return_if_fail (path != NULL);

  if (cellview->displayed_row)
    gtk_tree_row_reference_free (cellview->displayed_row);

  cellview->displayed_row = gtk_tree_row_reference_new (cellview->model,
						        path);

  /* force redraw */
  gtk_widget_queue_draw (GTK_WIDGET (cellview));
}

GtkTreePath *
egg_cell_view_get_displayed_row (EggCellView *cellview)
{
  g_return_val_if_fail (EGG_IS_CELL_VIEW (cellview), NULL);

  if (!cellview->displayed_row)
    return NULL;

  return gtk_tree_row_reference_get_path (cellview->displayed_row);
}

void
egg_cell_view_set_background_color (EggCellView *view,
				    GdkColor    *color)
{
  g_return_if_fail (EGG_IS_CELL_VIEW (view));

  if (color)
    {
      if (!view->background_set)
        {
	  view->background_set = TRUE;
	  g_object_notify (G_OBJECT (view), "background_set");
	}

      view->background = *color;
    }
  else
    {
      if (view->background_set)
        {
	  view->background_set = FALSE;
	  g_object_notify (G_OBJECT (view), "background_set");
	}
    }
}
