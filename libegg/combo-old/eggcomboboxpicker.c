#include "eggcomboboxpicker.h"
#include "eggcellview.h"
#include "eggcellviewmenuitem.h"

#include <gtk/gtktreeselection.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkvseparator.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include <gdk/gdkkeysyms.h>

#include <string.h>
#include <gobject/gvaluecollector.h>

#include <stdarg.h>

/* WELCOME, to THE house of evil code */


typedef struct _PickerCellInfo PickerCellInfo;
struct _PickerCellInfo
{
  GtkCellRenderer *cell;
  GSList *attributes;

  guint expand : 1;
  guint pack : 1;
};


/* common */
static void     egg_combo_box_picker_class_init           (EggComboBoxPickerClass *klass);
static void     egg_combo_box_picker_init                 (EggComboBoxPicker      *picker);

static void     egg_combo_box_picker_screen_changed       (GtkWidget *widget,
                                                           GdkScreen *previous_screen,
						           gpointer   data);

/* list */
static void     egg_combo_box_picker_list_setup           (EggComboBoxPicker      *picker);
static void     egg_combo_box_picker_list_destroy         (EggComboBoxPicker      *picker);
static gboolean egg_combo_box_picker_list_button_released (GtkWidget              *widget,
							   GdkEventButton         *event,
							   gpointer                data);
static gboolean egg_combo_box_picker_list_key_press       (GtkWidget              *widget,
							   GdkEventKey            *event,
							   gpointer                data);
static void     egg_combo_box_picker_list_popped_up       (GtkWidget              *combo,
							   gpointer                data);
static gboolean egg_combo_box_picker_list_button_pressed  (GtkWidget              *widget,
							   GdkEventButton         *event,
							   gpointer                data);

static gint     egg_combo_box_picker_value_compare        (GValue                 *a_value,
						           GValue                 *b_value,
						           GType                   type);

static gint     egg_combo_box_picker_list_calc_requested_width (EggComboBoxPicker      *picker,
							        GtkTreeIter            *iter);
static void     egg_combo_box_picker_list_row_changed     (GtkTreeModel           *model,
							   GtkTreePath            *path,
							   GtkTreeIter            *iter,
							   gpointer                data);

/* menu */
static void     egg_combo_box_picker_menu_setup           (EggComboBoxPicker      *picker,
                                                           gboolean                add_childs);
static void     egg_combo_box_picker_menu_destroy         (EggComboBoxPicker      *picker);


static void     egg_combo_box_picker_relayout_item        (EggComboBoxPicker      *picker,
                                                           gint                    index);
static void     egg_combo_box_picker_relayout             (EggComboBoxPicker      *picker);

static void     egg_combo_box_picker_menu_item_activate   (GtkWidget              *item,
                                                           gpointer                user_data);

static void     egg_combo_box_picker_menu_row_inserted    (GtkTreeModel           *model,
                                                           GtkTreePath            *path,
					                   GtkTreeIter            *iter,
					                   gpointer                user_data);

static void     egg_combo_box_picker_menu_row_deleted     (GtkTreeModel           *model,
                                                           GtkTreePath            *path,
						           gpointer                user_data);

static void     egg_combo_box_picker_menu_row_changed     (GtkTreeModel           *model,
                                                           GtkTreePath            *path,
						           GtkTreeIter            *iter,
						           gpointer                data);


GType
egg_combo_box_picker_get_type (void)
{
  static GType combo_box_picker_type = 0;

  if (!combo_box_picker_type)
    {
      static const GTypeInfo combo_box_picker_info =
        {
	  sizeof (EggComboBoxPickerClass),
	  NULL, /* base_init */
	  NULL, /* base_finalize */
	  (GClassInitFunc) egg_combo_box_picker_class_init,
	  NULL, /* class_finalize */
	  NULL, /* class_data */
	  sizeof (EggComboBoxPicker),
	  0,
	  (GInstanceInitFunc) egg_combo_box_picker_init
	};

      combo_box_picker_type = g_type_register_static (EGG_TYPE_COMBO_BOX,
	  					      "EggComboBoxPicker",
						      &combo_box_picker_info,
						      0);
    }

  return combo_box_picker_type;
}

/* common */
static void
egg_combo_box_picker_class_init (EggComboBoxPickerClass *klass)
{
}

static void
egg_combo_box_picker_init (EggComboBoxPicker *picker)
{
  g_signal_connect (picker, "screen_changed",
                    G_CALLBACK (egg_combo_box_picker_screen_changed), NULL);

  picker->cell_view = egg_cell_view_new ();

  picker->wrap_width = 0;
}

/* FIXME: The next two functions are not finished yet. And _switch won't be
 * in the final API
 */
void
egg_combo_box_picker_switch (EggComboBoxPicker *picker)
{
  if (!picker->tree_view)
    {
      egg_combo_box_picker_menu_destroy (picker);
      egg_combo_box_picker_list_setup (picker);
    }
  else
    {
      egg_combo_box_picker_list_destroy (picker);
      egg_combo_box_picker_menu_setup (picker, TRUE);
    }
}

static void
egg_combo_box_picker_screen_changed (GtkWidget *widget,
                                     GdkScreen *previous_screen,
				     gpointer   data)
{
  if (EGG_COMBO_BOX_PICKER (widget)->wrap_width)
    return;

  /* YES this is not evil code */
  egg_combo_box_picker_list_setup (EGG_COMBO_BOX_PICKER (widget));
//  egg_combo_box_picker_menu_setup (EGG_COMBO_BOX_PICKER (widget), TRUE);
}

static PickerCellInfo *
egg_combo_box_picker_get_cell_info (EggComboBoxPicker *picker,
                                    GtkCellRenderer   *cell)
{
  GSList *i;

  for (i = picker->cells; i; i = i->next)
    {
      PickerCellInfo *info = (PickerCellInfo *)i->data;

      if (info->cell == cell)
	return info;
    }

  return NULL;
}


/*
 * menu style
 */

static void
cell_view_sync_cells (EggComboBoxPicker *picker,
                      EggCellView       *cellview)
{
  GSList *k;

  for (k = picker->cells; k; k = k->next)
    {
      GSList *j;
      PickerCellInfo *info = (PickerCellInfo *)k->data;

      if (info->pack == GTK_PACK_START)
        egg_cell_view_pack_start (cellview,
		                  info->cell, info->expand);
      else if (info->pack == GTK_PACK_END)
	egg_cell_view_pack_end (cellview,
		                info->cell, info->expand);

      for (j = info->attributes; j; j = j->next->next)
        {
          egg_cell_view_add_attribute (cellview,
	                               info->cell,
				       j->data,
				       GPOINTER_TO_INT (j->next->data));
        }
    }
}

static void
egg_combo_box_picker_menu_setup (EggComboBoxPicker *picker,
                                 gboolean           add_childs)
{
  gint i, items;
  GtkWidget *box;
  GtkWidget *tmp;

  /* FIXME: we use a button and other widgets now, but it might be nicer
   * to write our own expose func and stuff
   */

  box = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start (GTK_BOX (box), picker->cell_view,
                      TRUE, TRUE, 0);

  tmp = gtk_vseparator_new ();
  gtk_box_pack_start (GTK_BOX (box), tmp, FALSE, FALSE, 2);

  gtk_box_set_child_packing (GTK_BOX (picker),
                             EGG_COMBO_BOX (picker)->button,
			     TRUE, TRUE, 0, GTK_PACK_START);

  tmp = GTK_BIN (EGG_COMBO_BOX (picker)->button)->child;
  gtk_widget_reparent (tmp, box);
  gtk_box_set_child_packing (GTK_BOX (box), tmp,
                             FALSE, FALSE, 0, GTK_PACK_START);

  gtk_container_add (GTK_CONTAINER (EGG_COMBO_BOX (picker)->button), box);

  gtk_widget_show_all (box);

  picker->inserted_id =
    g_signal_connect (picker->store, "row_inserted",
                      G_CALLBACK (egg_combo_box_picker_menu_row_inserted),
		      picker);
  picker->deleted_id =
    g_signal_connect (picker->store, "row_deleted",
                      G_CALLBACK (egg_combo_box_picker_menu_row_deleted),
		      picker);
  picker->changed_id =
    g_signal_connect (picker->store, "row_changed",
	              G_CALLBACK (egg_combo_box_picker_menu_row_changed),
		      picker);

  /* create our funky menu */
  box = gtk_menu_new ();
  egg_combo_box_set_popup_widget (EGG_COMBO_BOX (picker), box);

  /* add items */
  if (!add_childs)
    return;

  items = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (picker->store), NULL);

  for (i = 0; i < items; i++)
    {
      GtkTreePath *path;

      path = gtk_tree_path_new_from_indices (i, -1);
      tmp = egg_cell_view_menu_item_new_from_model (GTK_TREE_MODEL (picker->store),
	                                            path);
      g_signal_connect (tmp, "activate",
                        G_CALLBACK (egg_combo_box_picker_menu_item_activate),
		        picker);

      cell_view_sync_cells (picker, EGG_CELL_VIEW (GTK_BIN (tmp)->child));

      gtk_menu_shell_append (GTK_MENU_SHELL (box), tmp);
      gtk_widget_show (tmp);

      gtk_tree_path_free (path);
    }
}

static void
egg_combo_box_picker_menu_destroy (EggComboBoxPicker *picker)
{
  GList *childs;
  GtkWidget *box;
  GtkWidget *button;
  GtkWidget *arrow;

  /* hackity hack */
  button = EGG_COMBO_BOX (picker)->button;
  box = GTK_BIN (button)->child;
  childs = gtk_container_get_children (GTK_CONTAINER (box));
  arrow = GTK_WIDGET (childs->next->next->data);

  g_object_ref (G_OBJECT (picker->cell_view));
  g_object_ref (G_OBJECT (arrow));
  gtk_container_remove (GTK_CONTAINER (button), box);

  /* box is destroyed now */
  gtk_container_add (GTK_CONTAINER (button), arrow);

  gtk_box_set_child_packing (GTK_BOX (picker), button,
                             FALSE, FALSE, 0, GTK_PACK_START);

  gtk_widget_show_all (button);

  g_signal_handler_disconnect (picker->store, picker->inserted_id);
  g_signal_handler_disconnect (picker->store, picker->deleted_id);

  picker->inserted_id = picker->deleted_id = -1;

  /* changing the popup window will unref the menu and the childs */
}

/*
 * grid
 */

static void
egg_combo_box_picker_relayout_item (EggComboBoxPicker *picker,
                                    gint               index)
{
  gint current_col = 0, current_row = 0;
  gint rows, cols;
  GList *list;
  GtkWidget *item;
  GtkWidget *menu;
  GtkTreeIter iter;

  menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (picker));
  if (!GTK_IS_MENU_SHELL (menu))
    return;

  list = gtk_container_get_children (GTK_CONTAINER (menu));
  item = g_list_nth_data (list, index);

  gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (picker->store),
                                 &iter, NULL, index);
  gtk_tree_model_get (GTK_TREE_MODEL (picker->store), &iter,
                      picker->col_column, &cols,
                      picker->row_column, &rows,
                      -1);

  /* look for a good spot */
  while (1)
    {
      if (current_col + cols > picker->wrap_width)
        {
          current_col = 0;
          current_row++;
        }

      if (!gtk_menu_occupied (GTK_MENU (menu),
                              current_col, current_col + cols,
                              current_row, current_row + rows))
        break;

      current_col++;
    }

  /* set attach props */
  gtk_menu_set_attach (GTK_MENU (menu), item,
                       current_col, current_col + cols,
                       current_row, current_row + rows);
}

static void
egg_combo_box_picker_relayout (EggComboBoxPicker *picker)
{
  gint i, items;
  GList *list, *j;
  GtkWidget *menu;

  /* ensure we are in menu style */
  if (picker->tree_view)
    egg_combo_box_picker_list_destroy (picker);

  menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (picker));

  if (!GTK_IS_MENU_SHELL (menu))
    {
      egg_combo_box_picker_menu_setup (picker, FALSE);
      menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (picker));
    }

  /* get rid of all children */
  g_return_if_fail (GTK_IS_MENU_SHELL (menu));

  list = gtk_container_get_children (GTK_CONTAINER (menu));

  for (j = g_list_last (list); j; j = j->prev)
    gtk_container_remove (GTK_CONTAINER (menu), j->data);

  g_list_free (j);

  /* and relayout */
  items = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (picker->store), NULL);

  for (i = 0; i < items; i++)
    {
      gint cols, rows;
      GtkWidget *tmp;
      GtkTreeIter iter;
      GtkTreePath *path;

      path = gtk_tree_path_new_from_indices (i, -1);
      tmp = egg_cell_view_menu_item_new_from_model (GTK_TREE_MODEL (picker->store), path);

      g_signal_connect (tmp, "activate",
                        G_CALLBACK (egg_combo_box_picker_menu_item_activate),
                        picker);

      cell_view_sync_cells (picker, EGG_CELL_VIEW (GTK_BIN (tmp)->child));

      gtk_tree_model_get_iter (GTK_TREE_MODEL (picker->store), &iter, path);
      gtk_tree_model_get (GTK_TREE_MODEL (picker->store), &iter,
                          picker->col_column, &cols,
                          picker->row_column, &rows,
                          -1);

      gtk_menu_shell_insert (GTK_MENU_SHELL (menu), tmp, i);

      egg_combo_box_picker_relayout_item (picker, i);
      gtk_widget_show (tmp);

      gtk_tree_path_free (path);
    }
}

/* callbacks */
static void
egg_combo_box_picker_menu_item_activate (GtkWidget *item,
                                         gpointer   user_data)
{
  gint index;
  GtkWidget *menu;
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (user_data);

  menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (picker));
  g_return_if_fail (GTK_IS_MENU (menu));

  index = g_list_index (GTK_MENU_SHELL (menu)->children, item);

  egg_combo_box_picker_set_active (picker, index);
}

static void
egg_combo_box_picker_menu_row_inserted (GtkTreeModel *model,
                                        GtkTreePath  *path,
					GtkTreeIter  *iter,
					gpointer      user_data)
{
  GtkWidget *menu;
  GtkWidget *item;
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (user_data);

  menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (user_data));
  g_return_if_fail (GTK_IS_MENU (menu));

  item = egg_cell_view_menu_item_new_from_model (model, path);
  g_signal_connect (item, "activate",
                    G_CALLBACK (egg_combo_box_picker_menu_item_activate),
		    picker);

  cell_view_sync_cells (picker, EGG_CELL_VIEW (GTK_BIN (item)->child));

  gtk_menu_shell_insert (GTK_MENU_SHELL (menu), item,
                         gtk_tree_path_get_indices (path)[0]);
  gtk_widget_show (item);
}

static void
egg_combo_box_picker_menu_row_deleted (GtkTreeModel *model,
                                       GtkTreePath  *path,
				       gpointer      user_data)
{
  gint index, items;
  GtkWidget *menu;
  GtkWidget *item;
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (user_data);

  index = gtk_tree_path_get_indices (path)[0];
  items = gtk_tree_model_iter_n_children (model, NULL);

  if (egg_combo_box_picker_get_active (picker) == index)
    egg_combo_box_picker_set_active (picker, index + 1 % items);

  menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (user_data));
  g_return_if_fail (GTK_IS_MENU (menu));

  item = g_list_nth_data (GTK_MENU_SHELL (menu)->children, index);
  g_return_if_fail (GTK_IS_MENU_ITEM (item));

  gtk_container_remove (GTK_CONTAINER (menu), item);
}

static int
egg_combo_box_picker_menu_calc_requested_width (EggComboBoxPicker *picker,
                                                gint               index)
{
  GtkWidget *menu;
  GtkWidget *cellview;
  GtkRequisition req;

  menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (picker));
  cellview = g_list_nth_data (GTK_MENU_SHELL (menu)->children, index);
  cellview = GTK_BIN (cellview)->child;

  gtk_widget_ensure_style (GTK_WIDGET (picker));
  gtk_widget_ensure_style (cellview);

  gtk_widget_size_request (cellview, &req);

  return req.width;
}

static void
egg_combo_box_picker_menu_row_changed (GtkTreeModel *model,
                                       GtkTreePath  *path,
				       GtkTreeIter  *iter,
				       gpointer      user_data)
{
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (user_data);
  gint width;
  gint index = gtk_tree_path_get_indices (path)[0];

  if (picker->wrap_width)
    egg_combo_box_picker_relayout_item (picker, index);

  width = egg_combo_box_picker_menu_calc_requested_width (picker, index);

  if (width > picker->width)
    {
      gtk_widget_set_size_request (picker->cell_view, width, -1);
      gtk_widget_queue_resize (picker->cell_view);
      picker->width = width;
    }
}

/*
 * list style
 */

static void
egg_combo_box_picker_list_setup (EggComboBoxPicker *picker)
{
  GSList *i;
  GtkWidget *eventbox;
  GtkTreePath *path;
  GtkTreeSelection *sel;

  g_signal_connect (picker, "popped_up",
		    G_CALLBACK (egg_combo_box_picker_list_popped_up), NULL);

  g_signal_connect (EGG_COMBO_BOX (picker)->button, "button_press_event",
	            G_CALLBACK (egg_combo_box_picker_list_button_pressed), picker);

  picker->cell_view_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (picker->cell_view_frame),
			     GTK_SHADOW_IN);

  g_object_set (G_OBJECT (picker->cell_view),
		"background", "white",
		"background_set", TRUE,
		NULL);

  eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (eventbox), picker->cell_view);

  g_signal_connect (eventbox, "button_press_event",
		    G_CALLBACK (egg_combo_box_picker_list_button_pressed),
		    picker);

  gtk_container_add (GTK_CONTAINER (picker->cell_view_frame),
		     eventbox);
  gtk_widget_show_all (picker->cell_view_frame);

  picker->tree_view = gtk_tree_view_new ();
  sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (picker->tree_view));
  gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (picker->tree_view),
				     FALSE);

  g_signal_connect (picker->tree_view, "button_press_event",
		    G_CALLBACK (egg_combo_box_picker_list_button_pressed),
		    picker);
  g_signal_connect (picker->tree_view, "button_release_event",
                    G_CALLBACK (egg_combo_box_picker_list_button_released),
		    picker);
  g_signal_connect (picker->tree_view, "key_press_event",
		    G_CALLBACK (egg_combo_box_picker_list_key_press),
		    picker);

  picker->column = gtk_tree_view_column_new ();
  gtk_tree_view_append_column (GTK_TREE_VIEW (picker->tree_view),
			       picker->column);

  /* set the models */
  gtk_tree_view_set_model (GTK_TREE_VIEW (picker->tree_view),
			   GTK_TREE_MODEL (picker->store));

  picker->changed_id =
    g_signal_connect (GTK_TREE_MODEL (picker->store), "row_changed",
		      G_CALLBACK (egg_combo_box_picker_list_row_changed),
		      picker);

  /* sync up */
  for (i = picker->cells; i; i = i->next)
    {
      GSList *j;
      PickerCellInfo *info = (PickerCellInfo *)i->data;

      if (info->pack == GTK_PACK_START)
	gtk_tree_view_column_pack_start (picker->column,
	                                 info->cell, info->expand);
      else if (info->pack == GTK_PACK_END)
	gtk_tree_view_column_pack_end (picker->column,
                                       info->cell, info->expand);

      for (j = info->attributes; j; j = j->next->next)
        {
	  gtk_tree_view_column_add_attribute (picker->column,
	                                      info->cell,
					      j->data,
					      GPOINTER_TO_INT (j->next->data));
	}
    }

  path = egg_cell_view_get_displayed_row (EGG_CELL_VIEW (picker->cell_view));
  if (path)
    {
      gtk_tree_view_set_cursor (GTK_TREE_VIEW (picker->tree_view), path,
                                NULL, FALSE);
      gtk_tree_path_free (path);
    }

  /* set sample/popup widgets */
  egg_combo_box_set_sample_widget (EGG_COMBO_BOX (picker),
      				   picker->cell_view_frame);
  egg_combo_box_set_popup_widget (EGG_COMBO_BOX (picker),
      				  picker->tree_view);


  gtk_widget_show (picker->tree_view);
}

static void
egg_combo_box_picker_list_destroy (EggComboBoxPicker *picker)
{
  GtkWidget *tmp;

  tmp = picker->cell_view->parent->parent;

  g_object_ref (G_OBJECT (picker->cell_view));
  gtk_widget_destroy (tmp);

  /* dum de dum */
  EGG_COMBO_BOX (picker)->sample = NULL;

  g_object_set (G_OBJECT (picker->cell_view),
                "background_set", FALSE,
		NULL);

  /* disconnect signals */
  g_signal_handler_disconnect (picker->store, picker->changed_id);
  picker->changed_id = -1;

  g_signal_handlers_disconnect_matched (picker->tree_view, G_SIGNAL_MATCH_DATA,
                                        0, 0, NULL, NULL, picker);
  g_signal_handlers_disconnect_matched (picker, G_SIGNAL_MATCH_FUNC,
                                        0, 0, NULL,
				        egg_combo_box_picker_list_popped_up, NULL);
  g_signal_handlers_disconnect_matched (EGG_COMBO_BOX (picker)->button,
                                        G_SIGNAL_MATCH_DATA,
                                        0, 0, NULL,
				        egg_combo_box_picker_list_button_pressed, NULL);

  /* _set_popup_widget will unref */
  picker->tree_view = NULL;
}

/* callbacks */
static void
egg_combo_box_picker_list_remove_grabs (EggComboBoxPicker *picker)
{
  EggComboBox *box = EGG_COMBO_BOX (picker);

  if (GTK_WIDGET_HAS_GRAB (picker->tree_view))
    gtk_grab_remove (picker->tree_view);

  if (GTK_WIDGET_HAS_GRAB (box->popup_window))
    {
      gtk_grab_remove (box->popup_window);
      gdk_pointer_ungrab (GDK_CURRENT_TIME);
    }
}

static void
egg_combo_box_picker_list_popped_up (GtkWidget *combobox,
				     gpointer   data)
{
  EggComboBox *box = EGG_COMBO_BOX (combobox);
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (combobox);

  if (!GTK_WIDGET_HAS_FOCUS (picker->tree_view))
    {
      gdk_keyboard_grab (box->popup_window->window, FALSE, GDK_CURRENT_TIME);
      gtk_widget_grab_focus (picker->tree_view);
    }
}

static gboolean
egg_combo_box_picker_list_button_pressed (GtkWidget      *widget,
				          GdkEventButton *event,
				          gpointer        data)
{
  EggComboBox *box = EGG_COMBO_BOX (data);
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (data);

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (ewidget == picker->tree_view)
    return TRUE;

  if ((ewidget != box->button && ewidget != picker->cell_view->parent) ||
      gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (box->button)))
    return FALSE;

  egg_combo_box_popup (box);

  gtk_grab_add (box->popup_window);
  gdk_pointer_grab (box->popup_window->window, TRUE,
		    GDK_BUTTON_PRESS_MASK |
		    GDK_BUTTON_RELEASE_MASK |
		    GDK_POINTER_MOTION_MASK,
		    NULL, NULL, GDK_CURRENT_TIME);

  /* FIXME: drag selection on treeview */
  gtk_grab_add (picker->tree_view);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (box->button), TRUE);

  picker->popup_in_progress = TRUE;

  return TRUE;
}

static gboolean
egg_combo_box_picker_list_button_released (GtkWidget      *widget,
				           GdkEventButton *event,
				           gpointer        data)
{
  gboolean ret;
  GtkTreePath *path = NULL;

  EggComboBox *box = EGG_COMBO_BOX (data);
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (data);

  gboolean popup_in_progress = FALSE;

  GtkWidget *ewidget = gtk_get_event_widget ((GdkEvent *)event);

  if (picker->popup_in_progress)
    {
      popup_in_progress = TRUE;
      picker->popup_in_progress = FALSE;
    }

  if (ewidget != picker->tree_view)
    {
      if ((ewidget == box->button || ewidget == picker->cell_view->parent) &&
	  !popup_in_progress &&
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (box->button)))
        {
	  egg_combo_box_picker_list_remove_grabs (picker);
	  egg_combo_box_popdown (box);
	  return TRUE;
	}

      /* released outside treeview */
      if (ewidget != box->button && ewidget != picker->cell_view->parent)
        {
	  egg_combo_box_picker_list_remove_grabs (picker);
	  egg_combo_box_popdown (box);

	  return TRUE;
	}

      return FALSE;
    }

  /* drop grabs */
  egg_combo_box_picker_list_remove_grabs (picker);

  /* select something cool */
  ret = gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
				       event->x, event->y,
				       &path,
				       NULL, NULL, NULL);

  if (!ret)
    return TRUE; /* clicked outside window? */

  egg_combo_box_picker_set_active (picker, gtk_tree_path_get_indices (path)[0]);
  egg_combo_box_popdown (box);

  gtk_tree_path_free (path);

  return TRUE;
}

static gboolean
egg_combo_box_picker_list_key_press (GtkWidget   *widget,
				     GdkEventKey *event,
				     gpointer     data)
{
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (data);

  if ((event->keyval == GDK_Return || event->keyval == GDK_KP_Enter ||
       event->keyval == GDK_space || event->keyval == GDK_KP_Space) ||
      event->keyval == GDK_Escape)
    {
      if (event->keyval != GDK_Escape)
        {
	  gboolean ret;
	  GtkTreeIter iter;
	  GtkTreeModel *model = NULL;
	  GtkTreeSelection *sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (picker->tree_view));

	  ret = gtk_tree_selection_get_selected (sel, &model, &iter);
	  if (ret)
	    {
	      GtkTreePath *path;

	      path = gtk_tree_model_get_path (model, &iter);
	      if (path)
	        {
		  egg_combo_box_picker_set_active (picker, gtk_tree_path_get_indices (path)[0]);
	          gtk_tree_path_free (path);
		}
	    }
	}
      else
	/* reset active item -- this is incredibly lame and ugly */
	egg_combo_box_picker_set_active (picker,
	    egg_combo_box_picker_get_active (picker));

      egg_combo_box_picker_list_remove_grabs (picker);
      egg_combo_box_popdown (EGG_COMBO_BOX (picker));

      return TRUE;
    }

  return FALSE;
}

static gint
egg_combo_box_picker_list_calc_requested_width (EggComboBoxPicker *picker,
					        GtkTreeIter       *iter)
{
  gint width;
  GtkTreeViewColumn *col;
  GtkTreeModel *model;

  col = gtk_tree_view_get_column (GTK_TREE_VIEW (picker->tree_view), 0);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (picker->tree_view));

  /* this seems to be needed so we get the correct font_desc in the style */
  gtk_widget_ensure_style (GTK_WIDGET (picker));
  gtk_widget_ensure_style (picker->tree_view);

  gtk_tree_view_column_cell_set_cell_data (col, model, iter,
					   FALSE, FALSE);
  gtk_tree_view_column_cell_get_size (col, NULL, NULL, NULL, &width, NULL);

  return width;
}

static void
egg_combo_box_picker_list_row_changed (GtkTreeModel *model,
				       GtkTreePath  *path,
				       GtkTreeIter  *iter,
				       gpointer      data)
{
  EggComboBoxPicker *picker = EGG_COMBO_BOX_PICKER (data);
  gint width;

  width = egg_combo_box_picker_list_calc_requested_width (picker, iter);

  if (width > picker->width)
    {
      gtk_widget_set_size_request (picker->cell_view, width, -1);
      gtk_widget_queue_resize (picker->cell_view);
      picker->width = width;
    }
}

/*
 * public API
 */

GtkWidget *
egg_combo_box_picker_new (gint n_columns,
    			  ...)
{
  GtkWidget *ret;
  va_list args;
  GType *types;
  gint i;

  va_start (args, n_columns);

  types = g_new0 (GType, n_columns);

  for (i = 0; i < n_columns; i++)
    types[i] = va_arg (args, GType);

  va_end (args);

  ret = egg_combo_box_picker_newv (n_columns, types);

  g_free (types);

  return ret;
}

GtkWidget *
egg_combo_box_picker_newv (gint   n_columns,
			   GType *types)
{
  EggComboBoxPicker *comboboxpicker;

  comboboxpicker = EGG_COMBO_BOX_PICKER (g_object_new (egg_combo_box_picker_get_type (), NULL));

  comboboxpicker->store = gtk_list_store_newv (n_columns, types);
  g_object_ref (G_OBJECT (comboboxpicker->store));
  egg_cell_view_set_model (EGG_CELL_VIEW (comboboxpicker->cell_view),
			   GTK_TREE_MODEL (comboboxpicker->store));

  /* work around treeview bug ... */
  {
    GtkTreeIter iter;
    gtk_list_store_append (comboboxpicker->store, &iter);
    gtk_tree_row_reference_new (GTK_TREE_MODEL (comboboxpicker->store), gtk_tree_path_new_from_string ("0"));
    gtk_list_store_remove (comboboxpicker->store, &iter);
  }

  return GTK_WIDGET (comboboxpicker);
}

void
egg_combo_box_picker_set_column_types (EggComboBoxPicker *picker,
				       gint               n_columns,
				       GType             *types)
{
  /* FIXME */
}

void
egg_combo_box_picker_pack_start (EggComboBoxPicker *picker,
                                 GtkCellRenderer   *rend,
				 gboolean           expand)
{
  PickerCellInfo *info;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_CELL_RENDERER (rend));

  info = g_new0 (PickerCellInfo, 1);
  info->cell = rend;
  info->expand = expand;
  info->pack = GTK_PACK_START;

  picker->cells = g_slist_append (picker->cells, info);

  egg_cell_view_pack_start (EGG_CELL_VIEW (picker->cell_view),
                            rend, expand);

  if (picker->column)
    gtk_tree_view_column_pack_start (picker->column, rend, expand);
}

void
egg_combo_box_picker_pack_end (EggComboBoxPicker *picker,
                               GtkCellRenderer   *rend,
                               gboolean           expand)
{
  PickerCellInfo *info;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_CELL_RENDERER (rend));

  info = g_new0 (PickerCellInfo, 1);
  info->cell = rend;
  info->expand = expand;
  info->pack = GTK_PACK_END;

  egg_cell_view_pack_end (EGG_CELL_VIEW (picker->cell_view),
			  rend, expand);

  if (picker->column)
    gtk_tree_view_column_pack_end (picker->column, rend, expand);
}

void
egg_combo_box_picker_set_attributes (EggComboBoxPicker *picker,
                                     GtkCellRenderer   *rend,
				     ...)
{
  va_list args;
  gchar *attribute;
  gint column;
  PickerCellInfo *info;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_CELL_RENDERER (rend));

  info = egg_combo_box_picker_get_cell_info (picker, rend);

  va_start (args, rend);

  attribute = va_arg (args, gchar *);

  while (attribute)
    {
      column = va_arg (args, gint);

      info->attributes = g_slist_prepend (info->attributes,
	                                  GINT_TO_POINTER (column));
      info->attributes = g_slist_prepend (info->attributes,
	                                  g_strdup (attribute));


      egg_cell_view_add_attribute (EGG_CELL_VIEW (picker->cell_view), rend,
				   attribute, column);

      if (picker->column)
        gtk_tree_view_column_add_attribute (picker->column, rend,
					    attribute, column);

      attribute = va_arg (args, gchar *);
    }

  va_end (args);
}

void
egg_combo_box_picker_insert (EggComboBoxPicker *picker,
			     gint               index,
			     ...)
{
  GtkTreeIter iter;
  va_list arg;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_TREE_MODEL (picker->store));
  g_return_if_fail (index >= 0);

  gtk_list_store_insert (picker->store, &iter, index);

  va_start (arg, index);
  gtk_list_store_set_valist (picker->store, &iter, arg);
  va_end (arg);
}

void
egg_combo_box_picker_append (EggComboBoxPicker *picker,
			     ...)
{
  GtkTreeIter iter;
  va_list arg;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_TREE_MODEL (picker->store));

  gtk_list_store_append (picker->store, &iter);

  va_start (arg, picker);
  gtk_list_store_set_valist (picker->store, &iter, arg);
  va_end (arg);
}

void
egg_combo_box_picker_prepend (EggComboBoxPicker *picker,
			      ...)
{
  GtkTreeIter iter;
  va_list arg;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_TREE_MODEL (picker->store));

  gtk_list_store_prepend (picker->store, &iter);

  va_start (arg, picker);
  gtk_list_store_set_valist (picker->store, &iter, arg);
  va_end (arg);
}

void
egg_combo_box_picker_remove (EggComboBoxPicker *picker,
			     gint               index)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (index >= 0);
  g_return_if_fail (index < picker->store->length);

  if (!gtk_tree_model_iter_nth_child (GTK_TREE_MODEL (picker->store),
				      &iter, NULL, index))
    return;

  gtk_list_store_remove (picker->store, &iter);

  /* select new item */
  if (index == picker->store->length)
    egg_combo_box_picker_set_active (picker, index - 1);
  else if (picker->store->length != 0)
    egg_combo_box_picker_set_active (picker, index);
}

static gint
egg_combo_box_picker_value_compare (GValue *a_value,
				    GValue *b_value,
				    GType   type)
{
  gint retval = 0;
  const gchar *stra, *strb;

  switch (G_TYPE_FUNDAMENTAL (type))
    {
    case G_TYPE_BOOLEAN:
      if (g_value_get_int (a_value) < g_value_get_int (b_value))
        retval = -1;
      else if (g_value_get_int (a_value) == g_value_get_int (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_CHAR:
      if (g_value_get_char (a_value) < g_value_get_char (b_value))
        retval = -1;
      else if (g_value_get_char (a_value) == g_value_get_char (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_UCHAR:
      if (g_value_get_uchar (a_value) < g_value_get_uchar (b_value))
        retval = -1;
      else if (g_value_get_uchar (a_value) == g_value_get_uchar (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_INT:
      if (g_value_get_int (a_value) < g_value_get_int (b_value))
        retval = -1;
      else if (g_value_get_int (a_value) == g_value_get_int (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_UINT:
      if (g_value_get_uint (a_value) < g_value_get_uint (b_value))
        retval = -1;
      else if (g_value_get_uint (a_value) == g_value_get_uint (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_LONG:
      if (g_value_get_long (a_value) < g_value_get_long (b_value))
        retval = -1;
      else if (g_value_get_long (a_value) == g_value_get_long (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_ULONG:
      if (g_value_get_ulong (a_value) < g_value_get_ulong (b_value))
        retval = -1;
      else if (g_value_get_ulong (a_value) == g_value_get_ulong (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_INT64:
      if (g_value_get_int64 (a_value) < g_value_get_int64 (b_value))
        retval = -1;
      else if (g_value_get_int64 (a_value) == g_value_get_int64 (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_UINT64:
      if (g_value_get_uint64 (a_value) < g_value_get_uint64 (b_value))
        retval = -1;
      else if (g_value_get_uint64 (a_value) == g_value_get_uint64 (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_ENUM:
      /* this is somewhat bogus. */
      if (g_value_get_enum (a_value) < g_value_get_enum (b_value))
        retval = -1;
      else if (g_value_get_enum (a_value) == g_value_get_enum (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_FLAGS:
      /* this is even more bogus. */
      if (g_value_get_flags (a_value) < g_value_get_flags (b_value))
        retval = -1;
      else if (g_value_get_flags (a_value) == g_value_get_flags (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_FLOAT:
      if (g_value_get_float (a_value) < g_value_get_float (b_value))
        retval = -1;
      else if (g_value_get_float (a_value) == g_value_get_float (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_DOUBLE:
      if (g_value_get_double (a_value) < g_value_get_double (b_value))
        retval = -1;
      else if (g_value_get_double (a_value) == g_value_get_double (b_value))
        retval = 0;
      else
        retval = 1;
      break;
    case G_TYPE_STRING:
      stra = g_value_get_string (a_value);
      strb = g_value_get_string (b_value);
      if (stra == NULL) stra = "";
      if (strb == NULL) strb = "";
      retval = g_utf8_collate (stra, strb);
      break;
    case G_TYPE_POINTER:
    case G_TYPE_BOXED:
    case G_TYPE_OBJECT:
    default:
      g_warning ("Attempting to compare on invalid type %s\n", g_type_name (type));      retval = FALSE;
      break;
    }

  return retval;
}

gint
egg_combo_box_picker_get_index (EggComboBoxPicker *picker,
				...)
{
  gint column;
  gint index = -1;
  gint i;
  va_list args;
  GtkTreeIter iter;
  GArray *values = NULL;

  /* FIXME: return -1 like the combo text? */
  g_return_val_if_fail (EGG_IS_COMBO_BOX_PICKER (picker), 0);
  g_return_val_if_fail (GTK_IS_TREE_MODEL (picker->store), 0);

  /* put all values in an array */
  values = g_array_sized_new (FALSE, FALSE, sizeof (GValue),
			      picker->store->n_columns);
  for (i = 0; i < picker->store->n_columns; i++)
    {
      GValue value = {0,};
      g_array_insert_val (values, i, value);
    }

  va_start (args, picker);
  column = va_arg (args, gint);

  while (column != -1)
    {
      GValue value = {0,};
      gchar *error = NULL;

      if (column >= picker->store->n_columns)
        {
	  g_warning ("%s: Invalid column number %d passed to egg_combo_box_picker_get_index (remember to end your list of columns with a -1)", G_STRLOC, column);
	  break;
	}

      g_value_init (&value, picker->store->column_headers[column]);
      G_VALUE_COLLECT (&value, args, 0, &error);
      if (error)
        {
	  g_warning ("%s: %s", G_STRLOC, error);
	  g_free (error);

	  break;
	}

      g_array_index (values, GValue, column) = value;

      column = va_arg (args, gint);
    }

  va_end (args);

  /* now iterate over the list and compare stuff
   * yes this is like slow as in incredibly slow
   */

  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (picker->store), &iter);

  for (i = 0; i < picker->store->length; i++)
    {
      gint j;
      GValue value = {0,};

      for (j = 0; j < picker->store->n_columns; j++)
        {
	  gint ret;

	  /* hack */
	  if (! g_array_index (values, GValue, j).g_type)
	    continue;

	  gtk_tree_model_get_value (GTK_TREE_MODEL (picker->store), &iter,
				    j, &value);
	  ret = egg_combo_box_picker_value_compare (&value, &g_array_index (values, GValue, j), picker->store->column_headers[j]);
	  g_value_unset (&value);

	  if (ret != 0)
	    break;
	}

      if (j == picker->store->n_columns)
        {
	  /* yay -- we got a match here */
	  index = i;
	  break;
	}

      gtk_tree_model_iter_next (GTK_TREE_MODEL (picker->store), &iter);
    }

  for (i = 0; i < picker->store->n_columns; i++)
    if (g_array_index (values, GValue, i).g_type)
      g_value_unset (&g_array_index (values, GValue, i));

  /* done */
  g_array_free (values, TRUE);

  return index;
}

void
egg_combo_box_picker_clear (EggComboBoxPicker *picker)
{
  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (GTK_IS_TREE_MODEL (picker->store));

  gtk_list_store_clear (picker->store);

  /* FIXME: reset cell view? */
}

gint
egg_combo_box_picker_get_active (EggComboBoxPicker *picker)
{
  GtkTreePath *path;
  gint active;

  g_return_val_if_fail (EGG_IS_COMBO_BOX_PICKER (picker), 0);
  g_return_val_if_fail (EGG_IS_CELL_VIEW (picker->cell_view), 0);

  path = egg_cell_view_get_displayed_row (EGG_CELL_VIEW (picker->cell_view));
  if (!path)
    return 0;

  active = gtk_tree_path_get_indices (path)[0];
  gtk_tree_path_free (path);

  return active;
}

void
egg_combo_box_picker_set_active (EggComboBoxPicker *picker,
				 gint               index)
{
  GtkTreePath *path;
  GtkTreePath *current_path;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (index >= 0);
  g_return_if_fail (EGG_IS_CELL_VIEW (picker->cell_view));
  g_return_if_fail (GTK_IS_TREE_MODEL (picker->store));
  g_return_if_fail (gtk_tree_model_iter_n_children (GTK_TREE_MODEL (picker->store), NULL));

  path = gtk_tree_path_new ();
  gtk_tree_path_append_index (path, index);

  current_path =
	  egg_cell_view_get_displayed_row (EGG_CELL_VIEW (picker->cell_view));

  if (current_path && !gtk_tree_path_compare (path, current_path))
    {
      gtk_tree_path_free (path);
      gtk_tree_path_free (current_path);

      return;
    }

  if (picker->tree_view)
    gtk_tree_view_set_cursor (GTK_TREE_VIEW (picker->tree_view), path,
			      NULL, FALSE);
  else
    {
      GtkWidget *menu = egg_combo_box_get_popup_widget (EGG_COMBO_BOX (picker));

      if (GTK_IS_MENU (menu))
	gtk_menu_set_active (GTK_MENU (menu), index);
    }

  egg_cell_view_set_displayed_row (EGG_CELL_VIEW (picker->cell_view), path);

  g_signal_emit_by_name (picker, "changed", NULL, NULL);

  gtk_tree_path_free (path);

  if (current_path)
    gtk_tree_path_free (current_path);
}

void
egg_combo_box_picker_set_wrap_width (EggComboBoxPicker *picker,
                                     gint               width)
{
  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (width > 0);

  picker->wrap_width = width;

  egg_combo_box_picker_relayout (picker);
}

void
egg_combo_box_picker_set_span_columns (EggComboBoxPicker *picker,
                                       gint               col_column,
                                       gint               row_column)
{
  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (col_column >= 0 && col_column < picker->store->n_columns);
  g_return_if_fail (row_column >= 0 && row_column < picker->store->n_columns);

  picker->col_column = col_column;
  picker->row_column = row_column;

  egg_combo_box_picker_relayout (picker);
}

GtkWidget *
egg_combo_box_picker_new_text ()
{
  GtkWidget *combo;
  GtkCellRenderer *cell;

  combo = egg_combo_box_picker_new (1, G_TYPE_STRING);

  cell = gtk_cell_renderer_text_new ();
  egg_combo_box_picker_pack_start (EGG_COMBO_BOX_PICKER (combo),
                                   cell, TRUE);
  egg_combo_box_picker_set_attributes (EGG_COMBO_BOX_PICKER (combo),
                                       cell,
                                       "text", 0,
                                       NULL);

  return combo;
}

void
egg_combo_box_picker_append_text (EggComboBoxPicker *picker,
                                  const gchar       *text)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (text != NULL);

  gtk_list_store_append (picker->store, &iter);
  gtk_list_store_set (picker->store, &iter, 0, text, -1);
}

void
egg_combo_box_picker_insert_text (EggComboBoxPicker *picker,
                                  gint               position,
                                  const gchar       *text)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (position >= 0);
  g_return_if_fail (text != NULL);

  gtk_list_store_insert (picker->store, &iter, position);
  gtk_list_store_set (picker->store, &iter, 0, text, -1);
}

void
egg_combo_box_picker_prepend_text (EggComboBoxPicker *picker,
                                   const gchar       *text)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (text != NULL);

  gtk_list_store_prepend (picker->store, &iter);
  gtk_list_store_set (picker->store, &iter, 0, text, -1);
}

GtkWidget *
egg_combo_box_picker_new_pixtext ()
{
  GtkWidget *combo;
  GtkCellRenderer *pixbuf;
  GtkCellRenderer *text;

  combo = egg_combo_box_picker_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

  pixbuf = gtk_cell_renderer_pixbuf_new ();
  egg_combo_box_picker_pack_start (EGG_COMBO_BOX_PICKER (combo),
                                   pixbuf, FALSE);
  egg_combo_box_picker_set_attributes (EGG_COMBO_BOX_PICKER (combo),
                                       pixbuf,
                                       "pixbuf", 0,
                                       NULL);

  text = gtk_cell_renderer_text_new ();
  egg_combo_box_picker_pack_start (EGG_COMBO_BOX_PICKER (combo),
                                   text, TRUE);
  egg_combo_box_picker_set_attributes (EGG_COMBO_BOX_PICKER (combo),
                                       text,
                                       "text", 1,
                                       NULL);

  return combo;
}

void
egg_combo_box_picker_append_pixtext (EggComboBoxPicker *picker,
                                     GdkPixbuf         *pixbuf,
                                     const gchar       *text)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (pixbuf != NULL);
  g_return_if_fail (text != NULL);

  gtk_list_store_append (picker->store, &iter);
  gtk_list_store_set (picker->store, &iter,
                      0, pixbuf,
                      1, text,
                      -1);
}

void
egg_combo_box_picker_insert_pixtext (EggComboBoxPicker *picker,
                                     gint               position,
                                     GdkPixbuf         *pixbuf,
                                     const gchar       *text)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (position >= 0);
  g_return_if_fail (pixbuf != NULL);
  g_return_if_fail (text != NULL);

  gtk_list_store_insert (picker->store, &iter, position);
  gtk_list_store_set (picker->store, &iter,
                      0, pixbuf,
                      1, text,
                      -1);
}

void
egg_combo_box_picker_prepend_pixtext (EggComboBoxPicker *picker,
                                      GdkPixbuf         *pixbuf,
                                      const gchar       *text)
{
  GtkTreeIter iter;

  g_return_if_fail (EGG_IS_COMBO_BOX_PICKER (picker));
  g_return_if_fail (pixbuf != NULL);
  g_return_if_fail (text != NULL);

  gtk_list_store_prepend (picker->store, &iter);
  gtk_list_store_set (picker->store, &iter,
                      0, pixbuf,
                      1, text,
                      -1);
}
