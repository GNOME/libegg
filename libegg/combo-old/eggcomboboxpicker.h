#ifndef __EGG_COMBO_BOX_PICKER_H__
#define __EGG_COMBO_BOX_PICKER_H__

#include "eggcombobox.h"
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeview.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EGG_TYPE_COMBO_BOX_PICKER              (egg_combo_box_picker_get_type ())
#define EGG_COMBO_BOX_PICKER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_COMBO_BOX_PICKER, EggComboBoxPicker))
#define EGG_COMBO_BOX_PICKER_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_COMBO_BOX_PICKER, EggComboBoxPickerClass))
#define EGG_IS_COMBO_BOX_PICKER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_COMBO_BOX_PICKER))
#define EGG_IS_COMBO_BOX_PICKER_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_COMBO_BOX_PICKER))
#define EGG_COMBO_BOX_PICKER_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_COMBO_BOX_PICKER, EggComboBoxPickerClass))


typedef struct _EggComboBoxPicker	EggComboBoxPicker;
typedef struct _EggComboBoxPickerClass	EggComboBoxPickerClass;

struct _EggComboBoxPicker
{
  EggComboBox parent_instance;

  /*< private >*/
  GtkListStore *store;

  /* grid */
  gint              col_column;
  gint              row_column;

  gint              wrap_width;

  /* sample */
  GtkWidget         *cell_view_frame;

  /* list */
  GtkWidget         *tree_view;
  GtkTreeViewColumn *column;

  /* menu */
  GtkWidget         *button;

  guint              inserted_id;
  guint              deleted_id;

  /* common */
  GtkWidget         *cell_view;

  gint               width;
  GSList            *cells;

  guint              changed_id;

  gboolean           popup_in_progress:1;
};

struct _EggComboBoxPickerClass
{
  EggComboBoxClass parent_class;
};


/* construction */
GType		 egg_combo_box_picker_get_type		(void);
GtkWidget	*egg_combo_box_picker_new		(gint n_columns,
    							 ...);
GtkWidget	*egg_combo_box_picker_newv		(gint n_columns,
    							 GType *types);
void		 egg_combo_box_picker_set_column_types	(EggComboBoxPicker *picker,
    							 gint n_columns,
							 GType *types);

/* manipulate list of cell renderers */
void             egg_combo_box_picker_pack_start        (EggComboBoxPicker *picker,
							 GtkCellRenderer *rend,
							 gboolean expand);
void             egg_combo_box_picker_pack_end          (EggComboBoxPicker *picker,
                                                         GtkCellRenderer *rend,
							 gboolean expand);
void             egg_combo_box_picker_set_attributes    (EggComboBoxPicker *picker,
                                                         GtkCellRenderer *rend,
							 ...);

/* grids */
void		 egg_combo_box_picker_set_wrap_width	(EggComboBoxPicker *picker,
		                                         gint               width);
void             egg_combo_box_picker_set_span_columns  (EggComboBoxPicker *picker,
                                                         gint               width_column,
                                                         gint               height_column);

/* data storage manipulation */
void		 egg_combo_box_picker_insert		(EggComboBoxPicker *picker,
    							 gint index,
							 ...);
void		 egg_combo_box_picker_append		(EggComboBoxPicker *picker,
							 ...);
void		 egg_combo_box_picker_prepend		(EggComboBoxPicker *picker,
							 ...);
void		 egg_combo_box_picker_remove		(EggComboBoxPicker *picker,
    							 gint index);
gint		 egg_combo_box_picker_get_index		(EggComboBoxPicker *picker,
							 ...);
void		 egg_combo_box_picker_clear		(EggComboBoxPicker *picker);

/* get/set active item */
gint		 egg_combo_box_picker_get_active	(EggComboBoxPicker *picker);
void		 egg_combo_box_picker_set_active	(EggComboBoxPicker *picker,
    							 gint index);

/* convenience -- text */
GtkWidget       *egg_combo_box_picker_new_text          ();
void             egg_combo_box_picker_append_text       (EggComboBoxPicker *picker,
                                                         const gchar       *text);
void             egg_combo_box_picker_insert_text       (EggComboBoxPicker *picker,
                                                         gint               position,
                                                         const gchar       *text);
void             egg_combo_box_picker_prepend_text      (EggComboBoxPicker *picker,
                                                         const gchar       *text);

/* convenience -- pixbuf/text pairs, based on stock items */
GtkWidget       *egg_combo_box_picker_new_pixtext       ();
void             egg_combo_box_picker_append_pixtext    (EggComboBoxPicker *picker,
                                                         GdkPixbuf         *pixbuf,
                                                         const gchar       *stock);
void             egg_combo_box_picker_insert_pixtext    (EggComboBoxPicker *picker,
                                                         gint               position,
                                                         GdkPixbuf         *pixbuf,
                                                         const gchar       *stock);
void             egg_combo_box_picker_prepend_pixtext   (EggComboBoxPicker *picker,
                                                         GdkPixbuf         *pixbuf,
                                                         const gchar       *stock);

#ifdef __cplusplus
}
#endif

#endif /* __EGG_COMBO_BOX_PICKER_H__ */
