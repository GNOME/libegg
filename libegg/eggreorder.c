#include "eggreorder.h"

/**
 * WARNING: this code uses bad hacks (ie accessing private data structures).
 * Do not things like this in your own code. This code will be integrated
 * in GTK+ 2.2.
 */

#define GTK_LIST_STORE_IS_SORTED(list) (GTK_LIST_STORE (list)->sort_column_id != -2)
#define G_SLIST(list) ((GSList *)list)

typedef struct
{
  gint order;
  gpointer el_ptr;
} EggReorderStruct;


static gint
egg_reorder_func (gconstpointer a,
		  gconstpointer b,
		  gpointer data)
{
  EggReorderStruct *a_reorder;
  EggReorderStruct *b_reorder;
  gboolean *invalid_order = (int *) data;

  a_reorder = (EggReorderStruct *) a;
  b_reorder = (EggReorderStruct *) b;

  if (a_reorder->order < b_reorder->order)
    return -1;
  if (a_reorder->order > b_reorder->order)
    return 1;

  /* There's a duplicate setting */
  *invalid_order = TRUE;
  return 0;
}

void
egg_list_store_reorder (GtkListStore *list_store,
			gint         *new_order)
{
  gint i;
  GSList *current_list;
  GtkTreePath *path;
  EggReorderStruct *sort_array;
  gboolean invalid_order = FALSE;

  g_return_if_fail (GTK_IS_LIST_STORE (list_store));
  g_return_if_fail (!GTK_LIST_STORE_IS_SORTED (list_store));
  g_return_if_fail (new_order != NULL);

  sort_array = g_new (EggReorderStruct, list_store->length);

  current_list = list_store->root;

  for (i = 0; i < list_store->length; i++)
    {
      sort_array[i].order = new_order[i];
      sort_array[i].el_ptr = current_list;

      current_list = current_list->next;
    }

  g_qsort_with_data (sort_array,
		     list_store->length,
		     sizeof (EggReorderStruct),
		     egg_reorder_func,
		     NULL);
  if (invalid_order)
    {
      g_warning ("Duplicate warning passed to egg_list_store_reorder");
      g_free (sort_array);
      return;
    }
  

  for (i = 0; i < list_store->length - 1; i++)
    {
      G_SLIST (sort_array[i].el_ptr)->next = G_SLIST (sort_array[i+1].el_ptr);
    }

  list_store->root = G_SLIST (sort_array[0].el_ptr);
  list_store->tail = G_SLIST (sort_array[list_store->length-1].el_ptr);
  G_SLIST (list_store->tail)->next = NULL;

  /* emit signal */
  path = gtk_tree_path_new ();
  gtk_tree_model_rows_reordered (GTK_TREE_MODEL (list_store),
				 path, NULL, new_order);
  gtk_tree_path_free (path);
  g_free (sort_array);
}

void
egg_tree_store_reorder (GtkTreeStore *tree_store,
			GtkTreeIter  *parent_iter,
			gint         *new_order)
{

}
