
#include "eggplaceholder.h"

/* GObjectClass */
static void      egg_placeholder_finalize             (GObject    *object);

/* GtkWidgetClass */
static void      egg_placeholder_get_preferred_width  (GtkWidget  *widget,
						       gint       *min_width,
						       gint       *nat_width);
static void      egg_placeholder_get_preferred_height (GtkWidget  *widget,
						       gint       *min_height,
						       gint       *nat_height);

#define ANIMATION_STEP 0.12F  /* How much percentage of the size to animate per iteration */
#define ANIMATION_FREQ 20    /* At what frequency in millisecs to animate */

enum {
  SIGNAL_ANIMATION_DONE,
  NUM_SIGNALS
};

static guint placeholder_signals[NUM_SIGNALS] = { 0, };

struct _EggPlaceholderPrivate
{
  gint           width;
  gint           height;

  GtkOrientation animation_orientation; /* Whether we are animating in/out horizontally or vertically */
  gdouble        animation_percent;     /* A multiplier between 0 and 1 representing the current progress */
  guint          animation_id;          /* The GSource id for the animation timeout */
  EggPlaceholderAnimDirection animation_direction;  /* Whether the placeholder is animating in or out */
};

G_DEFINE_TYPE (EggPlaceholder, egg_placeholder, GTK_TYPE_DRAWING_AREA)

static void egg_placeholder_class_init (EggPlaceholderClass * klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = egg_placeholder_finalize;

  widget_class->get_preferred_width  = egg_placeholder_get_preferred_width;
  widget_class->get_preferred_height = egg_placeholder_get_preferred_height;

  placeholder_signals[SIGNAL_ANIMATION_DONE] = 
    g_signal_new ("animation-done",
		  G_OBJECT_CLASS_TYPE (klass),
		  G_SIGNAL_RUN_FIRST,
		  0, NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  g_type_class_add_private (klass, sizeof (EggPlaceholderPrivate));
}

static void
egg_placeholder_init (EggPlaceholder * placeholder)
{
  placeholder->priv = 
    G_TYPE_INSTANCE_GET_PRIVATE (placeholder,
				 EGG_TYPE_PLACEHOLDER,
				 EggPlaceholderPrivate);

  placeholder->priv->animation_percent = 1.0F;

  /* Default visible, avoid cluttering code in eggspreadtablednd.c */
  gtk_widget_show (GTK_WIDGET (placeholder));
}

static void
egg_placeholder_finalize (GObject * object)
{
  EggPlaceholder *placeholder = EGG_PLACEHOLDER (object);

  if (placeholder->priv->animation_id > 0)
    {
      g_source_remove (placeholder->priv->animation_id);
      placeholder->priv->animation_id = 0;
    }

  G_OBJECT_CLASS (egg_placeholder_parent_class)->finalize (object);
}

static void
egg_placeholder_get_preferred_width (GtkWidget  *widget,
				     gint       *min_width,
				     gint       *nat_width)
{
  EggPlaceholder *placeholder = EGG_PLACEHOLDER (widget);
  gint            width;

  if (placeholder->priv->animation_orientation == GTK_ORIENTATION_HORIZONTAL)
    width = placeholder->priv->width * placeholder->priv->animation_percent;
  else
    width = placeholder->priv->width;

  *min_width = *nat_width = width;
}

static void
egg_placeholder_get_preferred_height (GtkWidget  *widget,
				      gint       *min_height,
				      gint       *nat_height)
{
  EggPlaceholder *placeholder = EGG_PLACEHOLDER (widget);
  gint            height;

  if (placeholder->priv->animation_orientation == GTK_ORIENTATION_VERTICAL)
    height = placeholder->priv->height * placeholder->priv->animation_percent;
  else
    height = placeholder->priv->height;

  *min_height = *nat_height = height;
}

static gboolean 
placeholder_animate (EggPlaceholder *placeholder)
{
  if (placeholder->priv->animation_direction == EGG_PLACEHOLDER_ANIM_IN)
    placeholder->priv->animation_percent += ANIMATION_STEP;
  else if (placeholder->priv->animation_direction == EGG_PLACEHOLDER_ANIM_OUT)
    placeholder->priv->animation_percent -= ANIMATION_STEP;
  else
    g_error ("Placeholder animation called while not animating");

  placeholder->priv->animation_percent =
    CLAMP (placeholder->priv->animation_percent, 0.0, 1.0);

  gtk_widget_queue_resize (GTK_WIDGET (placeholder));

  if (placeholder->priv->animation_percent >= 1.0 ||
      placeholder->priv->animation_percent <= 0.0)
    {
      placeholder->priv->animation_id = 0;
      placeholder->priv->animation_direction = EGG_PLACEHOLDER_ANIM_NONE;

      g_signal_emit (placeholder, placeholder_signals[SIGNAL_ANIMATION_DONE], 0);

      return FALSE;
    }

  return TRUE;
}

void
egg_placeholder_animate_in (EggPlaceholder *placeholder,
			    GtkOrientation  orientation)
{
  g_return_if_fail (EGG_IS_PLACEHOLDER (placeholder));

  placeholder->priv->animation_orientation = orientation;
  placeholder->priv->animation_direction   = EGG_PLACEHOLDER_ANIM_IN;

  if (placeholder->priv->animation_id == 0)
    {
      placeholder->priv->animation_percent = 0.0F;
      placeholder->priv->animation_id = 
	gdk_threads_add_timeout (ANIMATION_FREQ, 
				 (GSourceFunc)placeholder_animate, 
				 placeholder);
    }

  gtk_widget_queue_resize (GTK_WIDGET (placeholder));
}

void
egg_placeholder_animate_out (EggPlaceholder *placeholder,
			     GtkOrientation  orientation)
{
  g_return_if_fail (EGG_IS_PLACEHOLDER (placeholder));

  placeholder->priv->animation_orientation = orientation;
  placeholder->priv->animation_direction   = EGG_PLACEHOLDER_ANIM_OUT;

  if (placeholder->priv->animation_id == 0)
    {
      placeholder->priv->animation_percent = 1.0F;
      placeholder->priv->animation_id = 
	gdk_threads_add_timeout (ANIMATION_FREQ, 
				 (GSourceFunc)placeholder_animate, 
				 placeholder);
    }

  gtk_widget_queue_resize (GTK_WIDGET (placeholder));
}


EggPlaceholderAnimDirection 
egg_placeholder_get_animating (EggPlaceholder *placeholder)
{
  g_return_val_if_fail (EGG_IS_PLACEHOLDER (placeholder),
			EGG_PLACEHOLDER_ANIM_NONE);

  return placeholder->priv->animation_direction;
}


GtkWidget *
egg_placeholder_new (gint width, 
		     gint height)
{
  EggPlaceholder *placeholder = g_object_new (EGG_TYPE_PLACEHOLDER, NULL);

  placeholder->priv->width  = width;
  placeholder->priv->height = height;

  return GTK_WIDGET (placeholder);
}
