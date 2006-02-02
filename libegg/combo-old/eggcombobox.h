#ifndef __EGG_COMBO_BOX_H__
#define __EGG_COMBO_BOX_H__

#include <gtk/gtkwidget.h>
#include <gtk/gtkhbox.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EGG_TYPE_COMBO_BOX              (egg_combo_box_get_type ())
#define EGG_COMBO_BOX(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_COMBO_BOX, EggComboBox))
#define EGG_COMBO_BOX_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_COMBO_BOX, EggComboBoxClass))
#define EGG_IS_COMBO_BOX(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_COMBO_BOX))
#define EGG_IS_COMBO_BOX_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_COMBO_BOX))
#define EGG_COMBO_BOX_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_COMBO_BOX, EggComboBoxClass))

typedef struct _EggComboBox		EggComboBox;
typedef struct _EggComboBoxClass	EggComboBoxClass;

struct _EggComboBox
{
  GtkHBox parent_instance;

  gboolean popped_up:1;

  /*< private >*/
  GtkWidget *sample;
  GtkWidget *popup;

  GtkWidget *popup_window;
  GtkWidget *button;
  GtkWidget *frame;
};

struct _EggComboBoxClass
{
  GtkHBoxClass parent_class;

  void     (* changed)     (EggComboBox *combobox);
  void     (* popped_up)   (EggComboBox *combobox);
  void     (* popped_down) (EggComboBox *combobox);

  /* Key Binding signals */
  gboolean (* popup)       (EggComboBox *combobox);
};

GType      egg_combo_box_get_type (void);
GtkWidget *egg_combo_box_new      (void);

void       egg_combo_box_set_sample_widget (EggComboBox *combobox,
					    GtkWidget   *sample);
GtkWidget *egg_combo_box_get_sample_widget (EggComboBox *combobox);
void       egg_combo_box_set_popup_widget  (EggComboBox *combobox,
					    GtkWidget   *popup);
GtkWidget *egg_combo_box_get_popup_widget  (EggComboBox *combobox);

void       egg_combo_box_popup             (EggComboBox *combobox);
void       egg_combo_box_popdown           (EggComboBox *combobox);



#ifdef __cplusplus
}
#endif

#endif /* __EGG_COMBO_BOX_H__ */
