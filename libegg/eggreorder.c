#include "eggreorder.h"

/**
 * WARNING: this code uses bad hacks (ie accessing private data structures).
 * Do not things like this in your own code. This code will be integrated
 * in GTK+ 2.2.
 */

#define GTK_LIST_STORE_IS_SORTED(list) (GTK_LIST_STORE (list)->sort_column_id != -2)

void
egg_list_store_reorder (GtkListStore *list_store,
			gint         *new_order)
{
  gint i;
  GSList *current_list;
  GSList *new_list = NULL;
  GtkTreePath *path;

  g_return_if_fail (GTK_IS_LIST_STORE (list_store));
  g_return_if_fail (!GTK_LIST_STORE_IS_SORTED (list_store));
  g_return_if_fail (new_order != NULL);

  current_list = list_store->root;
  i = list_store->length - 1;
  
  /* we start at the end and prepend items */
  for ( ; i >= 0; i--)
    {
      GSList *tmp;

      tmp = g_slist_nth (current_list, new_order[i]);
      new_list = g_slist_prepend (new_list, tmp->data);
    }

  g_slist_free (current_list);

  list_store->root = new_list;
  list_store->tail = g_slist_last (new_list);

  path = gtk_tree_path_new ();
  gtk_tree_model_rows_reordered (GTK_TREE_MODEL (list_store),
				 path, NULL, new_order);
  gtk_tree_path_free (path);
}

void
egg_tree_store_reorder (GtkTreeStore *tree_store,
			GtkTreeIter  *parent_iter,
			gint         *new_order)
{

}
