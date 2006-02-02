#ifndef __EGG_CELL_VIEW_H__
#define __EGG_CELL_VIEW_H__

#include <gtk/gtkwidget.h>
#include <gtk/gtkcellrenderer.h>
#include <gtk/gtktreemodel.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define EGG_TYPE_CELL_VIEW		(egg_cell_view_get_type ())
#define EGG_CELL_VIEW(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_CELL_VIEW, EggCellView))
#define EGG_CELL_VIEW_CLASS(vtable)	(G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_CELL_VIEW, EggCellViewClass))
#define EGG_IS_CELL_VIEW(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_CELL_VIEW))
#define EGG_IS_CELL_VIEW_CLASS(vtable)	(G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_CELL_VIEW))
#define EGG_CELL_VIEW_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_CELL_VIEW, EggCellViewClass))

typedef struct _EggCellView		EggCellView;
typedef struct _EggCellViewClass	EggCellViewClass;

struct _EggCellView
{
  GtkWidget parent_instance;

  /*< private >*/
  GtkTreeModel *model;
  GtkTreeRowReference *displayed_row;
  GList *cell_list;
  gint spacing;

  GdkColor background;
  gboolean background_set;
};

struct _EggCellViewClass
{
  GtkWidgetClass parent_class;
};

GType		 egg_cell_view_get_type			(void);
GtkWidget	*egg_cell_view_new			(void);
GtkWidget	*egg_cell_view_new_with_text		(const gchar     *text);
GtkWidget	*egg_cell_view_new_with_markup		(const gchar     *markup);
GtkWidget	*egg_cell_view_new_with_pixbuf		(GdkPixbuf       *pixbuf);


void		 egg_cell_view_pack_start		(EggCellView     *cellview,
							 GtkCellRenderer *renderer,
							 gboolean         expand);
void		 egg_cell_view_pack_end			(EggCellView     *cellview,
							 GtkCellRenderer *renderer,
							 gboolean         expand);

void             egg_cell_view_add_attribute            (EggCellView     *cellview,
							 GtkCellRenderer *renderer,
							 const gchar     *attribute,
							 gint             column);
void             egg_cell_view_clear_attributes         (EggCellView     *cellview,
							 GtkCellRenderer *attribute);
void             egg_cell_view_set_attributes           (EggCellView     *cellview,
							 GtkCellRenderer *renderer,
							 ...);
void             egg_cell_view_set_value                (EggCellView     *cellview,
							 GtkCellRenderer *renderer,
							 gchar           *property,
							 GValue          *value);
void		 egg_cell_view_set_values		(EggCellView     *cellview,
							 GtkCellRenderer *renderer,
							 ...);

void             egg_cell_view_set_model                (EggCellView     *cellview,
							 GtkTreeModel    *model);
void             egg_cell_view_set_displayed_row        (EggCellView     *cellview,
							 GtkTreePath     *path);
GtkTreePath     *egg_cell_view_get_displayed_row        (EggCellView     *cellview);

void             egg_cell_view_set_background_color     (EggCellView     *cellview,
							 GdkColor        *color);

#ifdef __cplusplus
}
#endif

#endif /* __EGG_CELL_VIEW_H__ */
