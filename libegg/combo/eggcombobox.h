/* eggcombobox.h
 * Copyright (C) 2002, 2003  Kristian Rietveld <kris@gtk.org>
 *
 * <FIXME put GTK LGPL header here>
 */
#ifndef __EGG_COMBO_BOX_H__
#define __EGG_COMBO_BOX_H__

#include <gtk/gtkbin.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtktreeview.h>

G_BEGIN_DECLS

#define EGG_TYPE_COMBO_BOX             (egg_combo_box_get_type ())
#define EGG_COMBO_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_COMBO_BOX, EggComboBox))
#define EGG_COMBO_BOX_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_COMBO_BOX, EggComboBoxClass))
#define EGG_IS_COMBO_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_COMBO_BOX))
#define EGG_IS_COMBO_BOX_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_COMBO_BOX))
#define EGG_COMBO_BOX_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), EGG_TYPE_COMBO_BOX, EggComboBoxClass))


typedef struct _EggComboBox        EggComboBox;
typedef struct _EggComboBoxClass   EggComboBoxClass;

struct _EggComboBox
{
  GtkBin parent_instance;

  /*< private >*/
  GtkTreeModel *model;

  gint col_column;
  gint row_column;

  gint wrap_width;

  gint active_item;

  GtkWidget *tree_view;
  GtkTreeViewColumn *column;

  GtkWidget *cell_view;
  GtkWidget *measurer;

  GtkWidget *hbox;
  GtkWidget *cell_view_frame;

  GtkWidget *button;
  GtkWidget *arrow;
  GtkWidget *separator;

  GtkWidget *popup_widget;
  GtkWidget *popup_window;
  GtkWidget *popup_frame;

  guint inserted_id;
  guint deleted_id;

  gint width;
  GSList *cells;

  guint changed_id;

  gboolean popup_in_progress:1;
};

struct _EggComboBoxClass
{
  GtkBinClass parent_class;

  /* signals */
  void     (* changed)          (EggComboBox *combo_box);
};


/* construction */
GType         egg_combo_box_get_type         (void);
GtkWidget    *egg_combo_box_new              (GtkTreeModel    *model);

/* manipulate list of cell renderers */
void          egg_combo_box_pack_start       (EggComboBox     *combo_box,
                                              GtkCellRenderer *cell,
                                              gboolean         expand);
void          egg_combo_box_pack_end         (EggComboBox     *combo_box,
                                              GtkCellRenderer *cell,
                                              gboolean         expand);
void          egg_combo_box_set_attributes   (EggComboBox     *combo_box,
                                              GtkCellRenderer *cell,
                                              ...);
void          egg_combo_box_clear            (EggComboBox     *combo_box);

/* grids */
void          egg_combo_box_set_wrap_width         (EggComboBox *combo_box,
                                                    gint         width);
void          egg_combo_box_set_row_span_column    (EggComboBox *combo_box,
                                                    gint         row_span);
void          egg_combo_box_set_column_span_column (EggComboBox *combo_box,
                                                    gint         column_span);

/* get/set active item */
gint          egg_combo_box_get_active       (EggComboBox     *combo_box);
void          egg_combo_box_set_active       (EggComboBox     *combo_box,
                                              gint             index);

/* convenience -- text */
GtkWidget    *egg_combo_box_new_text         ();
void          egg_combo_box_append_text      (EggComboBox     *combo_box,
                                              const gchar     *text);
void          egg_combo_box_insert_text      (EggComboBox     *combo_box,
                                              gint             position,
                                              const gchar     *text);
void          egg_combo_box_prepend_text     (EggComboBox     *combo_box,
                                              const gchar     *text);

G_END_DECLS

#endif /* __EGG_COMBO_BOX_H__ */
