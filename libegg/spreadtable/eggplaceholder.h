 
#ifndef __EGG_PLACEHOLDER_H__
#define __EGG_PLACEHOLDER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EGG_TYPE_PLACEHOLDER            (egg_placeholder_get_type ())
#define EGG_PLACEHOLDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_PLACEHOLDER, EggPlaceholder))
#define EGG_PLACEHOLDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_PLACEHOLDER, EggPlaceholderClass))
#define EGG_IS_PLACEHOLDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_PLACEHOLDER))
#define EGG_IS_PLACEHOLDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_PLACEHOLDER))
#define EGG_PLACEHOLDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_PLACEHOLDER, EggPlaceholderClass))

typedef struct _EggPlaceholder        EggPlaceholder;
typedef struct _EggPlaceholderClass   EggPlaceholderClass;
typedef struct _EggPlaceholderPrivate EggPlaceholderPrivate;


typedef enum {
  EGG_PLACEHOLDER_ANIM_NONE,
  EGG_PLACEHOLDER_ANIM_IN,
  EGG_PLACEHOLDER_ANIM_OUT
} EggPlaceholderAnimDirection;


struct _EggPlaceholder
{
  GtkDrawingArea drawing_area;
	
  EggPlaceholderPrivate *priv;
};

struct _EggPlaceholderClass
{
  GtkDrawingAreaClass parent_class;
};

GType         egg_placeholder_get_type        (void) G_GNUC_CONST;

GtkWidget    *egg_placeholder_new             (gint            width,
					       gint            height);
void          egg_placeholder_animate_in      (EggPlaceholder *placeholder,
					       GtkOrientation  orientation);
void          egg_placeholder_animate_out     (EggPlaceholder *placeholder,
					       GtkOrientation  orientation);

EggPlaceholderAnimDirection egg_placeholder_get_animating (EggPlaceholder *placeholder);

G_END_DECLS

#endif /* __EGG_PLACEHOLDER_H__ */
