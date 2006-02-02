/**
 * eggtreemodelunion.h: Union treemodel.
 *
 * Copyright (C) 2003, Kristian Rietveld.
 */

#ifndef __EGG_TREE_MODEL_UNION_H__
#define __EGG_TREE_MODEL_UNION_H__

#include <gtk/gtktreemodel.h>

G_BEGIN_DECLS

#define EGG_TYPE_TREE_MODEL_UNION              (egg_tree_model_union_get_type ())
#define EGG_TREE_MODEL_UNION(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_TREE_MODEL_UNION, EggTreeModelUnion))
#define EGG_TREE_MODEL_UNION_CLASS(vtable)     (G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_TREE_MODEL_UNION, EggTreeModelUnionClass))
#define EGG_IS_TREE_MODEL_UNION(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_TREE_MODEL_UNION))
#define EGG_IS_TREE_MODEL_UNION_CLASS(vtable)  (G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_TREE_MODEL_UNION))
#define EGG_TREE_MODEL_UNION_GET_CLASS(inst)   (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_TREE_MODEL_UNION, EggTreeModelUnionClass))

typedef struct _EggTreeModelUnion		EggTreeModelUnion;
typedef struct _EggTreeModelUnionClass		EggTreeModelUnionClass;

struct _EggTreeModelUnion
{
  GObject parent_instance;

  /*< private >*/
  GList *root;
  GHashTable *childs;

  GHashTable *child_paths;

  gint length;

  gint n_columns;
  GType *column_headers;

  gint stamp;
};

struct _EggTreeModelUnionClass
{
  GObjectClass parent_class;
};


GType         egg_tree_model_union_get_type             (void);
GtkTreeModel *egg_tree_model_union_new                  (gint               n_columns,
							 ...);
GtkTreeModel *egg_tree_model_union_newv                 (gint               n_columns,
		                                         GType             *types);
void          egg_tree_model_union_set_column_types     (EggTreeModelUnion *umodel,
		                                         gint               n_types,
							 GType             *types);

void          egg_tree_model_union_append               (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model);
void          egg_tree_model_union_prepend              (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model);
void          egg_tree_model_union_insert               (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model,
				                         gint              position);

void          egg_tree_model_union_append_with_mapping  (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model,
					                 ...);
void          egg_tree_model_union_prepend_with_mapping (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model,
					                 ...);
void          egg_tree_model_union_insert_with_mapping  (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model,
					                 gint               position,
					                 ...);
void          egg_tree_model_union_insert_with_mappingv (EggTreeModelUnion *umodel,
		                                        GtkTreeModel       *model,
							gint                position,
							gint               *column_mappings);

void          egg_tree_model_union_set_child            (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model,
                                                         GtkTreeIter       *iter);

void          egg_tree_model_union_clear                (EggTreeModelUnion *umodel);
void          egg_tree_model_union_remove               (EggTreeModelUnion *umodel,
                                                         GtkTreeModel      *model);

G_END_DECLS

#endif /* __EGG_TREE_MODEL_UNION_H__ */
