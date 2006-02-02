/**
 * eggtreemodelunion.c: Union treemodel implementation
 *
 * Copyright (C) 2003, Kristian Rietveld
 */


/* Notes/ToDo:
 *  - there is a nasty bug in here which I can't easily repro. Need to
 *    investigate
 *  - need to implement_set_child
 *  - fix bugs
 */

#include "eggtreemodelunion.h"

#include <glib/gprintf.h>
#include <string.h> /* for memcpy */


typedef struct _ModelMap ModelMap;

struct _ModelMap
{
  GtkTreeModel *model;
  gint nodes;
  gint offset;

  gint *column_mapping;
};

#define MODEL_MAP(map) ((ModelMap *)map)

/* iter format:
 * iter->stamp = umodel->stamp
 * iter->user_data = ModelMap
 * iter->user_data2 = offset relative to child_model
 * iter->user_data3 = index in the child hash.
 *
 * Iters will be invalidated on insertion/deletion of rows.
 */


/* object construction/deconstruction */
static void          egg_tree_model_union_init                      (EggTreeModelUnion *umodel);
static void          egg_tree_model_union_class_init                (EggTreeModelUnionClass *u_class);
static void          egg_tree_model_union_model_init                (GtkTreeModelIface *iface);

static void          egg_tree_model_union_finalize                  (GObject *object);

/* helpers */
static void          egg_tree_model_union_set_n_columns             (EggTreeModelUnion *umodel,
                                                                     gint               n_columns);
static void          egg_tree_model_union_set_column_type           (EggTreeModelUnion *umodel,
                                                                     gint               column,
						                     GType              type);
static void          model_map_free                                 (ModelMap          *map);
static GtkTreePath  *get_path_from_hash                             (EggTreeModelUnion *umodel,
                                                                     gchar             *str);
static gchar        *path_to_string_without_first                   (GtkTreePath       *path);
static void          egg_tree_model_union_update_stamp              (EggTreeModelUnion *umodel);
static ModelMap     *egg_tree_model_union_get_map_from_offset       (EggTreeModelUnion *umodel,
                                                                     gint               offset);
static ModelMap     *egg_tree_model_union_get_map_from_iter          (GtkTreeIter      *iter);
static void          egg_tree_model_union_convert_iter_to_child_iter(EggTreeModelUnion *umodel,
                                                                     GtkTreeIter       *child_iter,
							             GtkTreeIter       *union_iter);
static gint         egg_tree_model_union_convert_to_child_column    (EggTreeModelUnion *umodel,
                                                                     GtkTreeIter       *iter,
							             gint               column);
static gboolean     egg_tree_model_union_column_check               (EggTreeModelUnion *umodel,
                                                                     GtkTreeModel      *model,
						                     gint              *column_mapping);
static void         egg_tree_model_union_emit_inserted              (EggTreeModelUnion *umodel,
                                                                     gint               start,
						                     gint               length);
static void         egg_tree_model_union_emit_deleted               (EggTreeModelUnion *umodel,
                                                                     gint               start,
						                     gint               length);

/* TreeModel signals */
static void         egg_tree_model_union_row_changed           (GtkTreeModel *c_model,
                                                                GtkTreePath  *c_path,
					                        GtkTreeIter  *c_iter,
					                        gpointer       data);
static void         egg_tree_model_union_row_inserted          (GtkTreeModel *c_model,
                                                                GtkTreePath  *c_path,
					                        GtkTreeIter  *c_iter,
					                        gpointer      data);
static void         egg_tree_model_union_row_deleted           (GtkTreeModel *c_model,
                                                                GtkTreePath  *c_path,
					                        gpointer      data);
static void         egg_tree_model_union_row_has_child_toggled (GtkTreeModel *c_model,
                                                                GtkTreePath  *c_path,
                                                                GtkTreeIter  *c_iter,
							        gpointer      data);
static void         egg_tree_model_union_rows_reordered        (GtkTreeModel *c_model,
                                                                GtkTreePath  *c_path,
						                GtkTreeIter  *c_iter,
						                gint         *new_order,
						                gpointer      data);
/* TreeModelIface impl */
static guint        egg_tree_model_union_get_flags       (GtkTreeModel *model);
static gint         egg_tree_model_union_get_n_columns   (GtkTreeModel *model);
static GType        egg_tree_model_union_get_column_type (GtkTreeModel *model,
                                                          gint          index);
static gboolean     egg_tree_model_union_get_iter        (GtkTreeModel *model,
                                                          GtkTreeIter  *iter,
					                  GtkTreePath  *path);
static GtkTreePath *egg_tree_model_union_get_path        (GtkTreeModel *model,
                                                          GtkTreeIter  *iter);
static void         egg_tree_model_union_get_value       (GtkTreeModel *model,
                                                          GtkTreeIter  *iter,
					                  gint          column,
					                  GValue       *value);
static gboolean     egg_tree_model_union_iter_next       (GtkTreeModel *model,
                                                          GtkTreeIter  *iter);
static gboolean     egg_tree_model_union_iter_children   (GtkTreeModel *model,
                                                          GtkTreeIter  *iter,
						          GtkTreeIter  *parent);
static gboolean     egg_tree_model_union_iter_has_child  (GtkTreeModel *model,
                                                          GtkTreeIter  *iter);
static gint         egg_tree_model_union_iter_n_children (GtkTreeModel *model,
                                                          GtkTreeIter  *iter);
static gboolean     egg_tree_model_union_iter_nth_child  (GtkTreeModel *model,
                                                          GtkTreeIter  *iter,
						          GtkTreeIter  *parent,
						          gint          n);
static gboolean     egg_tree_model_union_iter_parent     (GtkTreeModel *model,
                                                          GtkTreeIter  *iter,
						          GtkTreeIter  *child);
static void         egg_tree_model_union_ref_node        (GtkTreeModel *model,
                                                          GtkTreeIter  *iter);
static void         egg_tree_model_union_unref_node      (GtkTreeModel *model,
                                                          GtkTreeIter  *iter);


static GObjectClass *parent_class = NULL;

GType
egg_tree_model_union_get_type (void)
{
  static GType tree_model_union_type = 0;

  if (!tree_model_union_type)
    {
      static const GTypeInfo tree_model_union_info =
        {
	  sizeof (EggTreeModelUnionClass),
	  NULL,
	  NULL,
	  (GClassInitFunc) egg_tree_model_union_class_init,
	  NULL,
	  NULL,
	  sizeof (EggTreeModelUnion),
	  0,
	  (GInstanceInitFunc) egg_tree_model_union_init
	};

      static const GInterfaceInfo tree_model_info =
        {
	  (GInterfaceInitFunc) egg_tree_model_union_model_init,
	  NULL,
	  NULL
	};

      tree_model_union_type = g_type_register_static (G_TYPE_OBJECT,
	                                              "EggTreeModelUnion",
						      &tree_model_union_info, 0);

      g_type_add_interface_static (tree_model_union_type,
	                           GTK_TYPE_TREE_MODEL,
				   &tree_model_info);
    }

  return tree_model_union_type;
}

static void
egg_tree_model_union_init (EggTreeModelUnion *umodel)
{
  umodel->root = NULL;
  umodel->childs = NULL;

  umodel->child_paths = g_hash_table_new_full (g_str_hash, g_str_equal,
                                               g_free, (GDestroyNotify)gtk_tree_path_free);

  umodel->n_columns = 0;
  umodel->column_headers = NULL;

  umodel->stamp = g_random_int ();
}

static void
egg_tree_model_union_class_init (EggTreeModelUnionClass *u_class)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (u_class);
  parent_class = g_type_class_peek_parent (u_class);

  object_class->finalize = egg_tree_model_union_finalize;
}

static void
egg_tree_model_union_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags = egg_tree_model_union_get_flags;
  iface->get_n_columns = egg_tree_model_union_get_n_columns;
  iface->get_column_type = egg_tree_model_union_get_column_type;
  iface->get_iter = egg_tree_model_union_get_iter;
  iface->get_path = egg_tree_model_union_get_path;
  iface->get_value = egg_tree_model_union_get_value;
  iface->iter_next = egg_tree_model_union_iter_next;
  iface->iter_children = egg_tree_model_union_iter_children;
  iface->iter_has_child = egg_tree_model_union_iter_has_child;
  iface->iter_n_children = egg_tree_model_union_iter_n_children;
  iface->iter_nth_child = egg_tree_model_union_iter_nth_child;
  iface->iter_parent = egg_tree_model_union_iter_parent;
  iface->ref_node = egg_tree_model_union_ref_node;
  iface->unref_node = egg_tree_model_union_unref_node;
}


static void
egg_tree_model_union_finalize (GObject *object)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *) object;

  egg_tree_model_union_clear (umodel);

  if (umodel->column_headers)
    g_free (umodel->column_headers);

  if (umodel->child_paths)
    g_hash_table_destroy (umodel->child_paths);

  /* chain */
  parent_class->finalize (object);
}

/* Helpers */
static void
egg_tree_model_union_set_n_columns (EggTreeModelUnion *umodel,
                                    gint               n_columns)
{
  GType *new_columns;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  if (umodel->n_columns == n_columns)
    return;

  new_columns = g_new0 (GType, n_columns);
  if (umodel->column_headers)
    {
      /* copy the old header orders over */
      if (n_columns >= umodel->n_columns)
	memcpy (new_columns, umodel->column_headers, umodel->n_columns * sizeof (gchar *));
      else
	memcpy (new_columns, umodel->column_headers, n_columns * sizeof (GType));

      g_free (umodel->column_headers);
    }

  umodel->column_headers = new_columns;
  umodel->n_columns = n_columns;
}

static void
egg_tree_model_union_set_column_type (EggTreeModelUnion *umodel,
                                      gint               column,
				      GType              type)
{
  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));
  g_return_if_fail (column >= 0 && column < umodel->n_columns);

  umodel->column_headers[column] = type;
}

static void
model_map_free (ModelMap *map)
{
  if (map->column_mapping)
    g_free (map->column_mapping);

  g_free (map);
}

static GtkTreePath *
get_path_from_hash (EggTreeModelUnion *umodel,
                    gchar *str)
{
  gpointer ret;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (umodel), NULL);

  ret = g_hash_table_lookup (umodel->child_paths, str);

  if (ret)
    return ret;

  ret = gtk_tree_path_new_from_string (str);
  g_hash_table_insert (umodel->child_paths, g_strdup (str), ret);

  return ret;
}

static gchar *
path_to_string_without_first (GtkTreePath *path)
{
  gint i;
  gchar *str, *ptr;

  gint depth = gtk_tree_path_get_depth (path);
  gint *indices = gtk_tree_path_get_indices (path);

  /* NOTE: we skip the first index here */
  str = ptr = (gchar *)g_new0 (gchar, depth * 8);
  g_sprintf (str, "%d", indices[1]);
  while (*ptr != '\0')
    ptr++;

  for (i = 2; i < depth; i++)
    {
      g_sprintf (ptr, ":%d", indices[i]);
      while (*ptr != '\0')
        ptr++;
    }

  return str;
}

static void
egg_tree_model_union_update_stamp (EggTreeModelUnion *umodel)
{
  do
    {
      umodel->stamp++;
    }
  while (!umodel->stamp);
}

static ModelMap *
egg_tree_model_union_get_map_from_offset (EggTreeModelUnion *umodel,
                                          gint               offset)
{
  GList *i;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (umodel), NULL);

  if (offset < 0 || offset >= umodel->length)
    return NULL;

  for (i = umodel->root; i; i = i->next)
    {
      ModelMap *map = MODEL_MAP (i->data);

      if (map->offset <= offset
	  && offset < map->offset + map->nodes)
	return map;
    }

  return NULL;
}

static ModelMap *
egg_tree_model_union_get_map_from_iter (GtkTreeIter *iter)
{
  return MODEL_MAP (iter->user_data);
}

static ModelMap *
egg_tree_model_union_get_map_from_model (EggTreeModelUnion *umodel,
                                         GtkTreeModel      *model)
{
  GList *i;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (umodel), NULL);

  for (i = umodel->root; i; i = i->next)
    if (MODEL_MAP (i->data)->model == model)
      return MODEL_MAP (i->data);

  return NULL;
}

static void
egg_tree_model_union_convert_iter_to_union_iter (EggTreeModelUnion *umodel,
                                                 GtkTreeModel      *model,
                                                 GtkTreeIter       *child_iter,
						 GtkTreeIter       *union_iter)
{
  ModelMap *map;
  GtkTreePath *path;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));

  map = egg_tree_model_union_get_map_from_model (umodel, model);

  if (!map)
    {
      union_iter->stamp = 0;
      return;
    }

  path = gtk_tree_model_get_path (model, child_iter);
  if (!path)
    {
      union_iter->stamp = 0;
      return;
    }

  union_iter->stamp = umodel->stamp;
  union_iter->user_data = map;
  union_iter->user_data2 = GINT_TO_POINTER (gtk_tree_path_get_indices (path)[0]);

  if (gtk_tree_path_get_depth (path) > 1)
    {
      gchar *str = path_to_string_without_first (path);

      union_iter->user_data3 = get_path_from_hash (umodel, str);
      g_free (str);
    }
  else
    union_iter->user_data3 = NULL;

  gtk_tree_path_free (path);
}

static void
egg_tree_model_union_convert_iter_to_child_iter (EggTreeModelUnion *umodel,
                                                 GtkTreeIter       *child_iter,
						 GtkTreeIter       *union_iter)
{
  ModelMap *map;
  GtkTreePath *path;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));
  g_return_if_fail (umodel->stamp == union_iter->stamp);

  map = egg_tree_model_union_get_map_from_iter (union_iter);

  if (union_iter->user_data3)
    path = gtk_tree_path_copy (union_iter->user_data3);
  else
    path = gtk_tree_path_new ();

  gtk_tree_path_prepend_index (path, GPOINTER_TO_INT (union_iter->user_data2));
  gtk_tree_model_get_iter (map->model, child_iter, path);

  gtk_tree_path_free (path);
}

static gint
egg_tree_model_union_convert_to_child_column (EggTreeModelUnion *umodel,
                                              GtkTreeIter       *iter,
					      gint               column)
{
  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (umodel), 0);
  g_return_val_if_fail (umodel->stamp == iter->stamp, 0);
  g_return_val_if_fail (column >= 0 && column < umodel->n_columns, 0);

  if (!MODEL_MAP (iter->user_data)->column_mapping)
    return column;

  return (MODEL_MAP (iter->user_data))->column_mapping[column];
}

static gboolean
egg_tree_model_union_column_check (EggTreeModelUnion *umodel,
                                   GtkTreeModel      *model,
				   gint              *column_mapping)
{
  gint i;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (umodel), FALSE);
  g_return_val_if_fail (GTK_IS_TREE_MODEL (model), FALSE);

  for (i = 0; i < umodel->n_columns; i++)
    {
      GType type;

      if (column_mapping)
	type = gtk_tree_model_get_column_type (model, column_mapping[i]);
      else
	type = gtk_tree_model_get_column_type (model, i);

      if (type != umodel->column_headers[i])
	return FALSE;
    }

  return TRUE;
}

static void
egg_tree_model_union_emit_inserted (EggTreeModelUnion *umodel,
                                    gint               start,
				    gint               length)
{
  gint i;
  GtkTreeIter iter;
  GtkTreePath *path;

  if (!length)
    return;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  path = gtk_tree_path_new_from_indices (start, -1);
  egg_tree_model_union_get_iter (GTK_TREE_MODEL (umodel), &iter, path);

  /* FIXME: do we need to emit inserted for child nodes? */

  for (i = 0; i < length; i++)
    {
      gtk_tree_model_row_inserted (GTK_TREE_MODEL (umodel), path, &iter);
      gtk_tree_path_next (path);
      egg_tree_model_union_iter_next (GTK_TREE_MODEL (umodel), &iter);
    }

  gtk_tree_path_free (path);
}

static void
egg_tree_model_union_emit_deleted (EggTreeModelUnion *umodel,
                                   gint               start,
				   gint               length)
{
  gint i;
  GtkTreePath *path;

  if (!length)
    return;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  path = gtk_tree_path_new_from_indices (start, -1);

  /* FIXME: do we need to emit deleted for child nodes? */

  for (i = 0; i < length; i++)
    {
      gtk_tree_model_row_deleted (GTK_TREE_MODEL (umodel), path);
      gtk_tree_path_next (path);
    }

  gtk_tree_path_free (path);
}

/* TreeModel signals */
static void
egg_tree_model_union_row_changed (GtkTreeModel *c_model,
                                  GtkTreePath  *c_path,
				  GtkTreeIter  *c_iter,
				  gpointer      data)
{
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (data);
  GtkTreeIter uiter;
  GtkTreePath *path;

  g_return_if_fail (GTK_IS_TREE_MODEL (c_model));
  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (data));

  /* translate the iter to a union iter and pass the event on */
  egg_tree_model_union_convert_iter_to_union_iter (umodel, c_model,
                                                   c_iter, &uiter);

  path = egg_tree_model_union_get_path (GTK_TREE_MODEL (data), &uiter);
  gtk_tree_model_row_changed (GTK_TREE_MODEL (data), path, &uiter);
  gtk_tree_path_free (path);
}

static void
egg_tree_model_union_row_inserted (GtkTreeModel *c_model,
                                   GtkTreePath  *c_path,
				   GtkTreeIter  *c_iter,
				   gpointer      data)
{
  gint position;
  GList *i;
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (data);

  g_return_if_fail (GTK_IS_TREE_MODEL (c_model));
  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (data));

  if (gtk_tree_path_get_depth (c_path) > 1)
    {
      GtkTreeIter uiter;
      GtkTreePath *path;

      /* translate and propagate */
      egg_tree_model_union_convert_iter_to_union_iter (umodel,
	                                               c_model,
						       c_iter,
						       &uiter);
      path = gtk_tree_model_get_path (GTK_TREE_MODEL (data), &uiter);
      gtk_tree_model_row_inserted (GTK_TREE_MODEL (data), path, &uiter);
      gtk_tree_path_free (path);

      return;
    }

  for (i = umodel->root; i; i = i->next)
    if (MODEL_MAP (i->data)->model == c_model)
      break;

  if (!i)
    return;

  MODEL_MAP (i->data)->nodes++;
  position = MODEL_MAP (i->data)->offset;

  for (i = i->next; i; i = i->next)
    MODEL_MAP (i->data)->offset++;

  position += gtk_tree_path_get_indices (c_path)[0];

  umodel->length++;

  egg_tree_model_union_update_stamp (umodel);
  egg_tree_model_union_emit_inserted (umodel, position, 1);
}

static void
egg_tree_model_union_row_deleted (GtkTreeModel *c_model,
                                  GtkTreePath  *c_path,
				  gpointer      data)
{
  GList *i;
  gint position;
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (data);

  g_return_if_fail (GTK_IS_TREE_MODEL (c_model));
  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (data));

  for (i = umodel->root; i; i = i->next)
    if (MODEL_MAP (i->data)->model == c_model)
      break;

  if (!i)
    return;

  if (gtk_tree_path_get_depth (c_path) > 1)
    {
      GtkTreePath *path = gtk_tree_path_copy (c_path);

      gtk_tree_path_get_indices (path)[0] += MODEL_MAP (i->data)->offset;
      gtk_tree_model_row_deleted (GTK_TREE_MODEL (data), path);
      gtk_tree_path_free (path);

      return;
    }

  MODEL_MAP (i->data)->nodes--;
  position = MODEL_MAP (i->data)->offset;

  for (i = i->next; i; i = i->next)
    MODEL_MAP (i->data)->offset--;

  position += gtk_tree_path_get_indices (c_path)[0];

  umodel->length--;

  egg_tree_model_union_update_stamp (umodel);
  egg_tree_model_union_emit_deleted (umodel, position, 1);
}

static void
egg_tree_model_union_row_has_child_toggled (GtkTreeModel *c_model,
                                            GtkTreePath  *c_path,
					    GtkTreeIter  *c_iter,
					    gpointer      data)
{
  GtkTreeIter uiter;
  GtkTreePath *path;
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (data);

  g_return_if_fail (GTK_IS_TREE_MODEL (c_model));
  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (data));

  /* translate the iter to a union iter and pass the event on */
  egg_tree_model_union_convert_iter_to_union_iter (umodel, c_model,
                                                   c_iter, &uiter);

  path = gtk_tree_model_get_path (GTK_TREE_MODEL (data), &uiter);
  gtk_tree_model_row_has_child_toggled (GTK_TREE_MODEL (data), path, &uiter);
  gtk_tree_path_free (path);
}

typedef struct
{
  GtkTreePath *root;
  gint level;
  gint *order;
} ReorderInfo;

static void
reorder_func (GtkTreePath *path,
              ReorderInfo *ri)
{
  gtk_tree_path_get_indices (path)[0] = gtk_tree_path_get_indices (ri->root)[0];

  if (!gtk_tree_path_is_descendant (path, ri->root))
    return;

  gtk_tree_path_get_indices (path)[ri->level] = ri->order[gtk_tree_path_get_indices (path)[ri->level]];
}

static void
egg_tree_model_union_rows_reordered (GtkTreeModel *c_model,
                                     GtkTreePath  *c_path,
				     GtkTreeIter  *c_iter,
				     gint         *new_order,
				     gpointer      data)
{
  gint i;
  gint *order;
  ModelMap *map;
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (data);

  g_return_if_fail (GTK_IS_TREE_MODEL (c_model));
  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (data));

  map = egg_tree_model_union_get_map_from_model (umodel, c_model);
  if (!map)
    return;

  if (gtk_tree_path_get_depth (c_path) > 0)
    {
      GtkTreePath *path;
      GtkTreeIter uiter;
      ReorderInfo ri;

      ri.root = c_path;
      ri.level = gtk_tree_path_get_depth (c_path);
      ri.order = new_order;

      g_hash_table_foreach (umodel->child_paths, (GHFunc)reorder_func, &ri);

      /* convert the iter and emit the signal */
      egg_tree_model_union_convert_iter_to_union_iter (umodel, c_model,
                                                       c_iter, &uiter);

      path = gtk_tree_model_get_path (GTK_TREE_MODEL (data), &uiter);
      gtk_tree_model_rows_reordered (GTK_TREE_MODEL (data), path,
	                             &uiter, new_order);
      gtk_tree_path_free (path);

      return;
    }

  order = g_new (gint, umodel->length);
  for (i = 0; i < umodel->length; i++)
    if (i < map->offset || i >= map->offset + map->nodes)
      order[i] = i;
    else
      order[i] = new_order[i] + map->offset;

  gtk_tree_model_rows_reordered (GTK_TREE_MODEL (data), NULL, NULL, order);
  g_free (order);
}

/* TreeModelIface implementation */
static guint
egg_tree_model_union_get_flags (GtkTreeModel *model)
{
  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), 0);

  return 0;
}

static gint
egg_tree_model_union_get_n_columns (GtkTreeModel *model)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), 0);

  return umodel->n_columns;
}

static GType
egg_tree_model_union_get_column_type (GtkTreeModel *model,
                                      gint          index)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), G_TYPE_INVALID);
  g_return_val_if_fail (index >= 0 && index < umodel->n_columns, G_TYPE_INVALID);

  return umodel->column_headers[index];
}

static gboolean
egg_tree_model_union_get_iter (GtkTreeModel *model,
                               GtkTreeIter  *iter,
			       GtkTreePath  *path)
{
  ModelMap *map;
  GtkTreeIter c_iter;
  GtkTreePath *c_path;
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), FALSE);
  g_return_val_if_fail (iter, FALSE);

  /* we don't want a warning on the screen if this fails */
  if (!umodel->root)
    {
      iter->stamp = 0;
      return FALSE;
    }

  map = egg_tree_model_union_get_map_from_offset (umodel, gtk_tree_path_get_indices (path)[0]);

  if (!map)
    {
      iter->stamp = 0;
      return FALSE;
    }

  /* check if the iter actually exists in the child model */
  c_path = gtk_tree_path_copy (path);
  gtk_tree_path_get_indices (c_path)[0] -= map->offset;

  if (!gtk_tree_model_get_iter (map->model, &c_iter, c_path))
    {
      gtk_tree_path_free (c_path);
      return FALSE;
    }

  gtk_tree_path_free (c_path);

  iter->stamp = umodel->stamp;
  iter->user_data = map;
  iter->user_data2 = GINT_TO_POINTER (gtk_tree_path_get_indices (path)[0] - map->offset);

  if (gtk_tree_path_get_depth (path) > 1)
    {
      gchar *str = path_to_string_without_first (path);

      iter->user_data3 = get_path_from_hash (umodel, str);
      g_free (str);
    }
  else
    iter->user_data3 = NULL;

  return TRUE;
}

static GtkTreePath *
egg_tree_model_union_get_path (GtkTreeModel *model,
                               GtkTreeIter  *iter)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;
  GtkTreePath *path;
  ModelMap *map;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), NULL);
  g_return_val_if_fail (umodel->stamp == iter->stamp, NULL);
  g_return_val_if_fail (umodel->root, NULL);

  map = egg_tree_model_union_get_map_from_iter (iter);

  if (iter->user_data3)
    {
      path = gtk_tree_path_copy (iter->user_data3);
    }
  else
    path = gtk_tree_path_new ();

  gtk_tree_path_prepend_index (path, map->offset + GPOINTER_TO_INT (iter->user_data2));

  return path;
}

static void
egg_tree_model_union_get_value (GtkTreeModel *model,
                                GtkTreeIter  *iter,
				gint          column,
				GValue       *value)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;
  GtkTreeIter child_iter;
  gint col;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (model));
  g_return_if_fail (umodel->stamp == iter->stamp);
  g_return_if_fail (umodel->root);

  egg_tree_model_union_convert_iter_to_child_iter (umodel, &child_iter, iter);
  col = egg_tree_model_union_convert_to_child_column (umodel, iter, column);
  gtk_tree_model_get_value (MODEL_MAP (iter->user_data)->model,
                            &child_iter, col, value);
}

static gboolean
egg_tree_model_union_iter_next (GtkTreeModel *model,
                                GtkTreeIter  *iter)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;
  GtkTreePath *path;
  gboolean ret;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), FALSE);
  g_return_val_if_fail (umodel->stamp == iter->stamp, FALSE);
  g_return_val_if_fail (umodel->root, FALSE);

  /* let's do it the lazy and easy way */
  path = egg_tree_model_union_get_path (model, iter);
  gtk_tree_path_next (path);

  ret = egg_tree_model_union_get_iter (model, iter, path);
  gtk_tree_path_free (path);

  return ret;
}

static gboolean
egg_tree_model_union_iter_children (GtkTreeModel *model,
                                    GtkTreeIter  *iter,
				    GtkTreeIter  *parent)
{
  ModelMap *map;
  GtkTreeIter child_iter;
  GtkTreeIter child_iter2;
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (model);

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), FALSE);

  if (!parent)
    {
      gtk_tree_model_get_iter_first (model, iter);
      return TRUE;
    }

  g_return_val_if_fail (umodel->stamp == parent->stamp, FALSE);

  map = egg_tree_model_union_get_map_from_iter (parent);

  egg_tree_model_union_convert_iter_to_child_iter (umodel, &child_iter,
                                                   parent);

  if (!gtk_tree_model_iter_children (map->model, &child_iter2, &child_iter))
    {
      iter->stamp = 0;
      return FALSE;
    }

  egg_tree_model_union_convert_iter_to_union_iter (umodel, map->model,
                                                   &child_iter2, iter);

  return TRUE;
}

static gboolean
egg_tree_model_union_iter_has_child (GtkTreeModel *model,
                                     GtkTreeIter  *iter)
{
  ModelMap *map;
  GtkTreeIter child_iter;
  EggTreeModelUnion *umodel = EGG_TREE_MODEL_UNION (model);

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), FALSE);

  if (!iter)
    return TRUE;

  g_return_val_if_fail (umodel->stamp == iter->stamp, FALSE);

  map = egg_tree_model_union_get_map_from_iter (iter);

  egg_tree_model_union_convert_iter_to_child_iter (umodel, &child_iter,
                                                   iter);

  return gtk_tree_model_iter_has_child (map->model, &child_iter);
}

static gint
egg_tree_model_union_iter_n_children (GtkTreeModel *model,
                                      GtkTreeIter  *iter)
{
  ModelMap *map;
  GtkTreeIter child_iter;
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), 0);

  if (!iter)
    return umodel->length;

  g_return_val_if_fail (umodel->stamp == iter->stamp, FALSE);

  map = egg_tree_model_union_get_map_from_iter (iter);

  egg_tree_model_union_convert_iter_to_child_iter (umodel, &child_iter,
                                                   iter);

  return gtk_tree_model_iter_n_children (map->model, &child_iter);
}

static gboolean
egg_tree_model_union_iter_nth_child (GtkTreeModel *model,
                                     GtkTreeIter  *iter,
				     GtkTreeIter  *parent,
				     gint          n)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;
  GtkTreePath *path;
  gboolean ret;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), FALSE);

  if (!parent)
    {
      path = gtk_tree_path_new_from_indices (n, -1);
      ret = egg_tree_model_union_get_iter (model, iter, path);
      gtk_tree_path_free (path);

      return TRUE;
    }

  g_return_val_if_fail (umodel->stamp == parent->stamp, FALSE);

  path = gtk_tree_model_get_path (model, parent);
  gtk_tree_path_append_index (path, n);

  ret = gtk_tree_model_get_iter (model, iter, path);

  gtk_tree_path_free (path);

  return ret;
}

static gboolean
egg_tree_model_union_iter_parent (GtkTreeModel *model,
                                  GtkTreeIter  *iter,
				  GtkTreeIter  *child)
{
  EggTreeModelUnion *umodel = (EggTreeModelUnion *)model;
  GtkTreePath *path;
  gboolean ret;

  g_return_val_if_fail (EGG_IS_TREE_MODEL_UNION (model), FALSE);

  if (!child)
    {
      iter->stamp = 0;
      return FALSE;
    }

  g_return_val_if_fail (umodel->stamp == child->stamp, FALSE);

  path = gtk_tree_model_get_path (model, child);

  if (gtk_tree_path_get_depth (path) <= 1)
    {
      gtk_tree_path_free (path);

      iter->stamp = 0;
      return FALSE;
    }

  gtk_tree_path_up (path);

  ret = gtk_tree_model_get_iter (model, iter, path);
  gtk_tree_path_free (path);

  return ret;
}

static void
egg_tree_model_union_ref_node (GtkTreeModel *model,
                               GtkTreeIter  *iter)
{
}

static void
egg_tree_model_union_unref_node (GtkTreeModel *model,
                                 GtkTreeIter  *iter)
{
}

/* public API */
GtkTreeModel *
egg_tree_model_union_new (gint n_columns,
		          ...)
{
  EggTreeModelUnion *retval;
  va_list args;
  gint i;

  g_return_val_if_fail (n_columns > 0, NULL);

  retval = g_object_new (EGG_TYPE_TREE_MODEL_UNION, NULL);
  egg_tree_model_union_set_n_columns (retval, n_columns);

  va_start (args, n_columns);

  for (i = 0; i < n_columns; i++)
    {
      GType type = va_arg (args, GType);
      egg_tree_model_union_set_column_type (retval, i, type);
    }

  va_end (args);

  return GTK_TREE_MODEL (retval);
}

GtkTreeModel *
egg_tree_model_union_newv (gint   n_columns,
		           GType *types)
{
  EggTreeModelUnion *retval;
  gint i;

  g_return_val_if_fail (n_columns > 0, NULL);

  retval = g_object_new (EGG_TYPE_TREE_MODEL_UNION, NULL);
  egg_tree_model_union_set_n_columns (retval, n_columns);

  for (i = 0; i < n_columns; i++)
    egg_tree_model_union_set_column_type (retval, i, types[i]);

  return GTK_TREE_MODEL (retval);
}

void
egg_tree_model_union_set_column_types (EggTreeModelUnion *umodel,
                                       gint               n_types,
				       GType             *types)
{
  /* FIXME */
}


void
egg_tree_model_union_append (EggTreeModelUnion *umodel,
                             GtkTreeModel      *model)
{
  egg_tree_model_union_insert (umodel, model, -1);
}

void
egg_tree_model_union_prepend (EggTreeModelUnion *umodel,
                              GtkTreeModel      *model)
{
  egg_tree_model_union_insert (umodel, model, 0);
}

void
egg_tree_model_union_insert (EggTreeModelUnion *umodel,
                             GtkTreeModel      *model,
			     gint               position)
{
  egg_tree_model_union_insert_with_mappingv (umodel, model, position, NULL);
}

void
egg_tree_model_union_append_with_mapping (EggTreeModelUnion *umodel,
                                          GtkTreeModel      *model,
					  ...)
{
  gint *column_mapping;
  va_list args;
  gint i;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  column_mapping = g_new0 (gint, umodel->n_columns);

  va_start (args, model);
  for (i = 0; i < umodel->n_columns; i++)
    column_mapping[i] = va_arg (args, gint);
  va_end (args);

  egg_tree_model_union_insert_with_mappingv (umodel, model, -1, column_mapping);

  g_free (column_mapping);

}

void
egg_tree_model_union_prepend_with_mapping (EggTreeModelUnion *umodel,
                                           GtkTreeModel      *model,
					   ...)
{
  gint *column_mapping;
  va_list args;
  gint i;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  column_mapping = g_new0 (gint, umodel->n_columns);

  va_start (args, model);
  for (i = 0; i < umodel->n_columns; i++)
    column_mapping[i] = va_arg (args, gint);
  va_end (args);

  egg_tree_model_union_insert_with_mappingv (umodel, model, 0, column_mapping);

  g_free (column_mapping);
}

void
egg_tree_model_union_insert_with_mapping (EggTreeModelUnion *umodel,
                                          GtkTreeModel      *model,
					  gint               position,
					  ...)
{
  gint *column_mapping;
  va_list args;
  gint i;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  column_mapping = g_new0 (gint, umodel->n_columns);

  va_start (args, position);
  for (i = 0; i < umodel->n_columns; i++)
    column_mapping[i] = va_arg (args, gint);
  va_end (args);

  egg_tree_model_union_insert_with_mappingv (umodel, model, position, column_mapping);

  g_free (column_mapping);
}

void
egg_tree_model_union_insert_with_mappingv (EggTreeModelUnion *umodel,
                                           GtkTreeModel      *model,
					   gint               position,
					   gint              *column_mapping)
{
  ModelMap *map;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));

  /* leaks ... */
  if (column_mapping)
    g_return_if_fail (egg_tree_model_union_column_check (umodel, model, column_mapping));
  else
    g_return_if_fail (egg_tree_model_union_column_check (umodel, model, NULL));

  g_object_ref (G_OBJECT (model));

  map = g_new0 (ModelMap, 1);
  map->model = model;
  map->nodes = gtk_tree_model_iter_n_children (model, NULL);
  if (column_mapping)
    {
      map->column_mapping = g_new0 (gint, umodel->n_columns);
      memcpy (map->column_mapping, column_mapping, sizeof (gint) * umodel->n_columns);
    }

  umodel->length += map->nodes;

  g_signal_connect (model, "row_inserted",
                    G_CALLBACK (egg_tree_model_union_row_inserted), umodel);
  g_signal_connect (model, "row_changed",
                    G_CALLBACK (egg_tree_model_union_row_changed), umodel);
  g_signal_connect (model, "row_deleted",
                    G_CALLBACK (egg_tree_model_union_row_deleted), umodel);
  g_signal_connect (model, "row_has_child_toggled",
                    G_CALLBACK (egg_tree_model_union_row_has_child_toggled),
		    umodel);
  g_signal_connect (model, "rows_reordered",
                    G_CALLBACK (egg_tree_model_union_rows_reordered), umodel);

  if (position == 0)
    {
      GList *j;

      umodel->root = g_list_prepend (umodel->root, map);
      map->offset = 0;

      /* fix up offsets */
      for (j = umodel->root->next; j; j = j->next)
	MODEL_MAP (j->data)->offset += map->nodes;

      /* update stamp */
      egg_tree_model_union_update_stamp (umodel);

      /* emit signals */
      egg_tree_model_union_emit_inserted (umodel, 0, map->nodes);
    }
  else if (position == -1)
    {
      GList *j;

      map->offset = 0;

      for (j = umodel->root; j; j = j->next)
	map->offset += MODEL_MAP (j->data)->nodes;

      umodel->root = g_list_append (umodel->root, map);

      /* update stamp */
      egg_tree_model_union_update_stamp (umodel);

      /* emit signals */
      egg_tree_model_union_emit_inserted (umodel, map->offset, map->nodes);
    }
  else
    {
      GList *j;

      umodel->root = g_list_insert (umodel->root, map, position);

      map->offset = 0;

      for (j = umodel->root; j->data != map; j = j->next)
	map->offset += MODEL_MAP (j->data)->nodes;

      for (j = j->next; j; j = j->next)
	MODEL_MAP (j->data)->offset += map->nodes;

      /* update stamp */
      egg_tree_model_union_update_stamp (umodel);

      /* emit signals */
      egg_tree_model_union_emit_inserted (umodel, map->offset, map->nodes);
    }
}

void
egg_tree_model_union_clear (EggTreeModelUnion *umodel)
{
  gint length;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));

  g_list_foreach (umodel->root, (GFunc)model_map_free, NULL);
  g_list_free (umodel->root);
  umodel->root = NULL;

  length = umodel->length;
  umodel->length = 0;

  egg_tree_model_union_update_stamp (umodel);

  egg_tree_model_union_emit_deleted (umodel, 0, length);
}

void
egg_tree_model_union_remove (EggTreeModelUnion *umodel,
                             GtkTreeModel      *model)
{
  GList *i;
  GList *next;
  ModelMap *map;

  g_return_if_fail (EGG_IS_TREE_MODEL_UNION (umodel));
  g_return_if_fail (GTK_IS_TREE_MODEL (model));
  g_return_if_fail (umodel->root);

  for (i = umodel->root; i; i = i->next)
    if (MODEL_MAP (i->data)->model == model)
      break;

  g_return_if_fail (i);

  next = i->next;

  map = MODEL_MAP (i->data);

  umodel->root = g_list_remove_link (umodel->root, i);
  umodel->length -= map->nodes;

  for (i = next; i; i = i->next)
    MODEL_MAP (i->data)->offset -= map->nodes;

  g_signal_handlers_disconnect_by_func (map->model,
                                        egg_tree_model_union_row_inserted,
					umodel);
  g_signal_handlers_disconnect_by_func (map->model,
                                        egg_tree_model_union_row_deleted,
					umodel);
  g_signal_handlers_disconnect_by_func (map->model,
                                        egg_tree_model_union_row_changed,
					umodel);
  g_signal_handlers_disconnect_by_func (map->model,
                                        egg_tree_model_union_row_has_child_toggled,
					umodel);
  g_signal_handlers_disconnect_by_func (map->model,
                                        egg_tree_model_union_rows_reordered,
					umodel);

  g_object_unref (G_OBJECT (map->model));

  egg_tree_model_union_update_stamp (umodel);

  egg_tree_model_union_emit_deleted (umodel, map->offset, map->nodes);

  model_map_free (map);
}
