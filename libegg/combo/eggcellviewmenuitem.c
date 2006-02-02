/* eggcellviewmenuitem.c
 * Copyright (C) 2003  Kristian Rietveld <kris@gtk.org>
 *
 * <FIXME put GTK LGPL header here>
 */
#include "eggcellviewmenuitem.h"
#include "eggcellview.h"

static void egg_cell_view_menu_item_init       (EggCellViewMenuItem      *item);
static void egg_cell_view_menu_item_class_init (EggCellViewMenuItemClass *klass);


GType
egg_cell_view_menu_item_get_type (void)
{
  static GType cell_view_menu_item_type = 0;

  if (!cell_view_menu_item_type)
    {
      static const GTypeInfo cell_view_menu_item_info =
        {
	  sizeof (EggCellViewMenuItemClass),
	  NULL,
	  NULL,
	  (GClassInitFunc) egg_cell_view_menu_item_class_init,
	  NULL,
	  NULL,
	  sizeof (EggCellViewMenuItem),
	  0,
	  (GInstanceInitFunc) egg_cell_view_menu_item_init
	};

      cell_view_menu_item_type =
	g_type_register_static (GTK_TYPE_MENU_ITEM, "EggCellViewMenuItem",
                                &cell_view_menu_item_info, 0);
    }

  return cell_view_menu_item_type;
}

static void
egg_cell_view_menu_item_class_init (EggCellViewMenuItemClass *klass)
{
}

static void
egg_cell_view_menu_item_init (EggCellViewMenuItem *item)
{
}

GtkWidget *
egg_cell_view_menu_item_new (void)
{
  EggCellViewMenuItem *item;

  item = g_object_new (EGG_TYPE_CELL_VIEW_MENU_ITEM, NULL);

  item->cell_view = egg_cell_view_new ();
  gtk_container_add (GTK_CONTAINER (item), item->cell_view);
  gtk_widget_show (item->cell_view);

  return GTK_WIDGET (item);
}

GtkWidget *
egg_cell_view_menu_item_new_with_pixbuf (GdkPixbuf *pixbuf)
{
  EggCellViewMenuItem *item;

  item = g_object_new (EGG_TYPE_CELL_VIEW_MENU_ITEM, NULL);

  item->cell_view = egg_cell_view_new_with_pixbuf (pixbuf);
  gtk_container_add (GTK_CONTAINER (item), item->cell_view);
  gtk_widget_show (item->cell_view);

  return GTK_WIDGET (item);
}

GtkWidget *
egg_cell_view_menu_item_new_with_text (const gchar *text)
{
  EggCellViewMenuItem *item;

  item = g_object_new (EGG_TYPE_CELL_VIEW_MENU_ITEM, NULL);

  item->cell_view = egg_cell_view_new_with_text (text);
  gtk_container_add (GTK_CONTAINER (item), item->cell_view);
  gtk_widget_show (item->cell_view);

  return GTK_WIDGET (item);
}

GtkWidget *
egg_cell_view_menu_item_new_with_markup (const gchar *markup)
{
  EggCellViewMenuItem *item;

  item = g_object_new (EGG_TYPE_CELL_VIEW_MENU_ITEM, NULL);

  item->cell_view = egg_cell_view_new_with_markup (markup);
  gtk_container_add (GTK_CONTAINER (item), item->cell_view);
  gtk_widget_show (item->cell_view);

  return GTK_WIDGET (item);
}

GtkWidget *
egg_cell_view_menu_item_new_from_model (GtkTreeModel *model,
                                        GtkTreePath  *path)
{
  EggCellViewMenuItem *item;

  item = g_object_new (EGG_TYPE_CELL_VIEW_MENU_ITEM, NULL);

  item->cell_view = egg_cell_view_new ();
  gtk_container_add (GTK_CONTAINER (item), item->cell_view);

  egg_cell_view_set_model (EGG_CELL_VIEW (item->cell_view), model);
  egg_cell_view_set_displayed_row (EGG_CELL_VIEW (item->cell_view), path);

  gtk_widget_show (item->cell_view);

  return GTK_WIDGET (item);
}
