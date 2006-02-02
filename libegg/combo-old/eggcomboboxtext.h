#ifndef __EGG_COMBO_BOX_TEXT_H__
#define __EGG_COMBO_BOX_TEXT_H__

#include "eggcombobox.h"
#include "eggentry.h"
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeview.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EGG_TYPE_COMBO_BOX_TEXT              (egg_combo_box_text_get_type ())
#define EGG_COMBO_BOX_TEXT(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_COMBO_BOX_TEXT, EggComboBoxText))
#define EGG_COMBO_BOX_TEXT_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_COMBO_BOX_TEXT, EggComboBoxTextClass))
#define EGG_IS_COMBO_BOX_TEXT(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_COMBO_BOX_TEXT))
#define EGG_IS_COMBO_BOX_TEXT_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_COMBO_BOX_TEXT))
#define EGG_COMBO_BOX_TEXT_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_COMBO_BOX_TEXT, EggComboBoxTextClass))

typedef struct _EggComboBoxText		EggComboBoxText;
typedef struct _EggComboBoxTextClass	EggComboBoxTextClass;

struct _EggComboBoxText
{
  EggComboBox parent_instance;

  /*< private >*/
  GtkWidget         *entry;
  gint               sample_index;

  GtkWidget         *tree_view;

  GtkListStore      *store;

  gboolean           popup_in_progress:1;
};

struct _EggComboBoxTextClass
{
  EggComboBoxClass parent_class;
};



GType		 egg_combo_box_text_get_type		(void);
GtkWidget       *egg_combo_box_text_new_with_entry      (EggEntry        *entry);
GtkWidget	*egg_combo_box_text_new_with_model	(GtkListStore    *model,
		                                         gint             text_column,
							 gint             history_max);

gint		 egg_combo_box_text_get_index		(EggComboBoxText *textcombo,
							 const gchar     *text);
gint		 egg_combo_box_text_get_length		(EggComboBoxText *textcombo);
gint		 egg_combo_box_text_get_sample_index	(EggComboBoxText *textcombo);
gchar           *egg_combo_box_text_get_sample_text     (EggComboBoxText *textcombo);
void		 egg_combo_box_text_set_sample		(EggComboBoxText *textcombo,
							 gint             index);

#ifdef __cplusplus
}
#endif


#endif /* __EGG_COMBO_BOX_TEXT_H__ */
