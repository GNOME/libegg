#ifndef __EGG_CELL_VIEW_MENU_ITEM_H__
#define __EGG_CELL_VIEW_MENU_ITEM_H__

#include <gtk/gtkmenuitem.h>
#include <gtk/gtktreemodel.h>

G_BEGIN_DECLS

#define EGG_TYPE_CELL_VIEW_MENU_ITEM              (egg_cell_view_menu_item_get_type ())
#define EGG_CELL_VIEW_MENU_ITEM(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_CELL_VIEW_MENU_ITEM, EggCellViewMenuItem))
#define EGG_CELL_VIEW_MENU_ITEM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_CELL_VIEW_MENU_ITEM, EggCellViewMenuItemClass))
#define EGG_IS_CELL_VIEW_MENU_ITEM(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_CELL_VIEW_MENU_ITEM))
#define EGG_IS_CELL_VIEW_MENU_ITEM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_CELL_VIEW_MENU_ITEM))
#define EGG_CELL_VIEW_MENU_ITEM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_CELL_VIEW_MENU_ITEM, EggCellViewMenuItemClass))


typedef struct _EggCellViewMenuItem      EggCellViewMenuItem;
typedef struct _EggCellViewMenuItemClass EggCellViewMenuItemClass;

struct _EggCellViewMenuItem
{
  GtkMenuItem parent_instance;

  /*< private >*/
  GtkWidget *cell_view;
};

struct _EggCellViewMenuItemClass
{
  GtkMenuItemClass parent_class;
};


GType	   egg_cell_view_menu_item_get_type        (void) G_GNUC_CONST;
GtkWidget *egg_cell_view_menu_item_new             (void);

GtkWidget *egg_cell_view_menu_item_new_with_pixbuf (GdkPixbuf   *pixbuf);
GtkWidget *egg_cell_view_menu_item_new_with_text   (const gchar *text);
GtkWidget *egg_cell_view_menu_item_new_with_markup (const gchar *markup);

GtkWidget *egg_cell_view_menu_item_new_from_model (GtkTreeModel *model,
                                                   GtkTreePath  *path);


G_END_DECLS

#endif /* __EGG_CELL_VIEW_MENU_ITEM_H__ */
