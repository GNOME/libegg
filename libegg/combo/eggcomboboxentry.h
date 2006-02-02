/* eggcomboboxentry.h
 * Copyright (C) 2002, 2003  Kristian Rietveld <kris@gtk.org>
 *
 * <FIXME put GTK LGPL header here>
 */
#ifndef __EGG_COMBO_BOX_ENTRY_H__
#define __EGG_COMBO_BOX_ENTRY_H__

#include "eggcombobox.h"

#include <gtk/gtktreemodel.h>

G_BEGIN_DECLS

#define EGG_TYPE_COMBO_BOX_ENTRY             (egg_combo_box_entry_get_type ())
#define EGG_COMBO_BOX_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_COMBO_BOX_ENTRY, EggComboBoxEntry))
#define EGG_COMBO_BOX_ENTRY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_COMBO_BOX_ENTRY, EggComboBoxEntryClass))
#define EGG_IS_COMBO_BOX_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_COMBO_BOX_ENTRY))
#define EGG_IS_COMBO_BOX_ENTRY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_COMBO_BOX_ENTRY))
#define EGG_COMBO_BOX_ENTRY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), EGG_TYPE_COMBO_BOX_ENTRY, EggComboBoxEntryClass))

typedef struct _EggComboBoxEntry             EggComboBoxEntry;
typedef struct _EggComboBoxEntryClass        EggComboBoxEntryClass;

struct _EggComboBoxEntry
{
  EggComboBox parent_instance;

  /*< private >*/
  GtkWidget *entry;

  gint text_column;
};

struct _EggComboBoxEntryClass
{
  EggComboBoxClass parent_class;
};


GType         egg_combo_box_entry_get_type  (void);
GtkWidget    *egg_combo_box_entry_new       (GtkTreeModel      *model,
                                             gint               text_column);


G_END_DECLS

#endif /* __EGG_COMBO_BOX_ENTRY_H__ */
