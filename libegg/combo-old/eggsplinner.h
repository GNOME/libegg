#ifndef __EGG_SPLINNER_H__
#define __EGG_SPLINNER_H__

#include "eggcombobox.h"
#include <gtk/gtkrange.h>

G_BEGIN_DECLS

#define EGG_TYPE_SPLINNER              (egg_splinner_get_type ())
#define EGG_SPLINNER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_SPLINNER, EggSplinner))
#define EGG_SPLINNER_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_SPLINNER, EggSplinnerClass))
#define EGG_IS_SPLINNER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_SPLINNER))
#define EGG_IS_SPLINNER_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_SPLINNER))
#define EGG_SPLINNER_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_SPLINNER, EggSplinnerClass))


typedef struct _EggSplinner		EggSplinner;
typedef struct _EggSplinnerClass	EggSplinnerClass;

struct _EggSplinner
{
  EggComboBox parent_instance;

  /*< private >*/
  GtkWidget *range;
  GtkWidget *entry;
  gboolean   popup_in_progress:1;
};

struct _EggSplinnerClass
{
  EggComboBoxClass parent_class;
};


GType		 egg_splinner_get_type		(void);
GtkWidget	*egg_splinner_new			(void);

G_END_DECLS

#endif /* __EGG_SPLINNER_H__ */
