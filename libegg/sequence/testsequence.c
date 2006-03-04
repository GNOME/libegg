#include <glib.h>
#include "eggsequence.h"
#include <stdlib.h>

enum
{
    NEW, FREE, GET_LENGTH, FOREACH, FOREACH_RANGE, SORT, SORT_ITER,
    
/* Getting iters */
    GET_BEGIN_ITER, GET_END_ITER, GET_ITER_AT_POS, APPEND, PREPEND,
    INSERT_BEFORE, MOVE, INSERT_SORTED, INSERT_SORTED_ITER, SORT_CHANGED,
    SORT_CHANGED_ITER, REMOVE, REMOVE_RANGE, MOVE_RANGE, SEARCH, SEARCH_ITER,
    
/* dereferencing */
    GET, SET,
    
/* operations on EggSequenceIter * */
    ITER_IS_BEGIN, ITER_IS_END, ITER_NEXT, ITER_PREV, ITER_GET_POSITION,
    ITER_MOVE, ITER_GET_SEQUENCE,
    
/* search */
    ITER_COMPARE, RANGE_GET_MIDPOINT,
    N_OPS
} Op;

typedef struct SequenceInfo
{
    GQueue *		queue;
    EggSequence *	sequence;
    int			n_items;
} SequenceInfo;

static void
check_integrity (SequenceInfo *info)
{
    GList *list;
    EggSequenceIter *iter;

    egg_sequence_self_test (info->sequence);
    
    if (egg_sequence_get_length (info->sequence) != info->n_items)
	g_print ("%d %d\n",
		 egg_sequence_get_length (info->sequence), info->n_items);
    g_assert (info->n_items == g_queue_get_length (info->queue));
    g_assert (egg_sequence_get_length (info->sequence) == info->n_items);

    iter = egg_sequence_get_begin_iter (info->sequence);
    list = info->queue->head;
    while (iter != egg_sequence_get_end_iter (info->sequence))
    {
	g_assert (list->data == iter);
	
	iter = egg_sequence_iter_next (iter);
	list = list->next;
    }

    g_assert (info->n_items == g_queue_get_length (info->queue));
    g_assert (egg_sequence_get_length (info->sequence) == info->n_items);
}

typedef struct
{
    SequenceInfo *seq;
    int		  number;
} Item;

static gpointer
new_item (SequenceInfo *seq)
{
    Item *item = g_new (Item, 1);
    seq->n_items++;
    item->seq = seq;
    item->number = g_random_int ();
    return item;
}

static void
free_item (gpointer data)
{
    Item *item = data;
    item->seq->n_items--;
    g_free (item);
}

static void
seq_foreach (gpointer data,
	     gpointer user_data)
{
    Item *item = data;
    GList **link = user_data;
    EggSequenceIter *iter;

    g_assert (*link != NULL);

    iter = (*link)->data;

    g_assert (egg_sequence_get (iter) == item);

    item->number = g_random_int();
    
    *link = (*link)->next;
}

static gint
compare_items (gconstpointer a,
	       gconstpointer b,
	       gpointer	     data)
{
    const Item *item_a = a;
    const Item *item_b = b;

    if (item_a->number < item_b->number)
	return -1;
    else if (item_a->number == item_b->number)
	return 0;
    else
	return 1;
}

static void
check_sorted (SequenceInfo *info)
{
    GList *list;
    int last;

    check_integrity (info);

    last = G_MININT;
    for (list = info->queue->head; list != NULL; list = list->next)
    {
	EggSequenceIter *iter = list->data;
	Item *item = egg_sequence_get (iter);
	
	g_assert (item->number >= last);

	last = item->number;
    }
}

static gint
compare_iters (gconstpointer a,
	       gconstpointer b,
	       gpointer      data)
{
    EggSequenceIter *iter_a = (EggSequenceIter *)a;
    EggSequenceIter *iter_b = (EggSequenceIter *)b;
    Item *item_a = egg_sequence_get (iter_a);
    Item *item_b = egg_sequence_get (iter_b);

    return compare_items (item_a, item_b, data);
}

/* A version of g_queue_link_index() that treats NULL as just
 * beyond the queue
 */
static int
queue_link_index (SequenceInfo *seq, GList *link)
{
    if (link)
	return g_queue_link_index (seq->queue, link);
    else
	return g_queue_get_length (seq->queue);
}

static void
get_random_range (SequenceInfo *seq,
		  EggSequenceIter **begin_iter,
		  EggSequenceIter **end_iter,
		  GList **begin_link,
		  GList **end_link)
{
    int length = g_queue_get_length (seq->queue);
    int b = g_random_int_range (0, length + 1);
    int e = g_random_int_range (b, length + 1);

    g_assert (length == egg_sequence_get_length (seq->sequence));

    if (begin_iter)
	*begin_iter = egg_sequence_get_iter_at_pos (seq->sequence, b);
    if (end_iter)
	*end_iter = egg_sequence_get_iter_at_pos (seq->sequence, e);
    if (begin_link)
	*begin_link = g_queue_peek_nth_link (seq->queue, b);
    if (end_link)
	*end_link = g_queue_peek_nth_link (seq->queue, e);
    if (begin_iter && begin_link)
    {
	g_assert (
	    queue_link_index (seq, *begin_link) ==
	    egg_sequence_iter_get_position (*begin_iter));
    }
    if (end_iter && end_link)
    {
	g_assert (
	    queue_link_index (seq, *end_link) ==
	    egg_sequence_iter_get_position (*end_iter));
    }
}

static gint
get_random_position (SequenceInfo *seq)
{
    int length = g_queue_get_length (seq->queue);

    g_assert (length == egg_sequence_get_length (seq->sequence));
    
    return g_random_int_range (-2, length + 5);
}

static EggSequenceIter *
get_random_iter (SequenceInfo  *seq,
		 GList        **link)
{
    EggSequenceIter *iter;
    int pos = get_random_position (seq);
    if (link)
	*link = g_queue_peek_nth_link (seq->queue, pos);
    iter = egg_sequence_get_iter_at_pos (seq->sequence, pos);
    if (link)
	g_assert (queue_link_index (seq, *link) == egg_sequence_iter_get_position (iter));
    return iter;
}

static void single_tests (void);

static guint32
get_seed (int argc, char **argv)
{
    if (argc > 1)
    {
	char *endptr;
	
	return strtol (argv[1], &endptr, 0);
    }
    else
    {
	return g_random_int();
    }
}

static void
dump_info (SequenceInfo *seq)
{
#if 0
    EggSequenceIter *iter;
    GList *list;
    
    iter = egg_sequence_get_begin_iter (seq->sequence);
    list = seq->queue->head;
    
    while (iter != egg_sequence_get_end_iter (seq->sequence))
    {
	Item *item = egg_sequence_get (iter);
	g_print ("%p  %p    %d\n", list->data, iter, item->number);
	
	iter = egg_sequence_iter_next (iter);
	list = list->next;
    }
#endif
}

/* A version of g_queue_insert_before() that appends if link is NULL */
static void
queue_insert_before (SequenceInfo *seq, GList *link, gpointer data)
{
    if (link)
	g_queue_insert_before (seq->queue, link, data);
    else
	g_queue_push_tail (seq->queue, data);
}

int
main (int argc,
      char **argv)
{
    int k;
    guint32 seed = get_seed (argc, argv);

    single_tests();

    g_print ("seed: %u\n", seed);

    g_random_set_seed (seed);
    
#define N_ITERATIONS 50000
#define N_SEQUENCES 8
    
    SequenceInfo sequences[N_SEQUENCES];

    for (k = 0; k < N_SEQUENCES; ++k)
    {
	sequences[k].queue = g_queue_new ();
	sequences[k].sequence = egg_sequence_new (free_item);
	sequences[k].n_items = 0;
    }
    
#define RANDOM_SEQUENCE() &(sequences[g_random_int_range (0, N_SEQUENCES)])
    
    for (k = 0; k < N_ITERATIONS; ++k)
    {
	int i;
	SequenceInfo *seq = RANDOM_SEQUENCE();
	int op = g_random_int_range (0, N_OPS);

#if 0
	g_print ("%d\n", op);
#endif
	
	switch (op)
	{
	case NEW:
	case FREE:
	{
	    g_queue_free (seq->queue);
	    egg_sequence_free (seq->sequence);

	    g_assert (seq->n_items == 0);
	    
	    seq->queue = g_queue_new ();
	    seq->sequence = egg_sequence_new (free_item);
	    
	    check_integrity (seq);
	}
	break;
	case GET_LENGTH:
	{
	    int slen = egg_sequence_get_length (seq->sequence);
	    int qlen = g_queue_get_length (seq->queue);
 
	    g_assert (slen == qlen);
	}
	break;
	case FOREACH:
	{
	    GList *link = seq->queue->head;
	    egg_sequence_foreach (seq->sequence, seq_foreach, &link);
	    g_assert (link == NULL);
	}
	break;
	case FOREACH_RANGE:
	{
	    EggSequenceIter *begin_iter, *end_iter;
	    GList *begin_link, *end_link;

	    get_random_range (seq, &begin_iter, &end_iter, &begin_link, &end_link);

	    check_integrity (seq);

	    egg_sequence_foreach_range (begin_iter, end_iter, seq_foreach, &begin_link);

	    g_assert (begin_link == end_link);
	}
	break;
	case SORT:
	{
	    dump_info (seq);
 
	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);
	    
	    dump_info (seq);
	}
	break;
	case SORT_ITER:
	{
	    egg_sequence_sort_iter (seq->sequence,
				    (EggSequenceIterCompareFunc)compare_iters, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);
	    check_sorted (seq);
	}
	break;
	
/* Getting iters */
	case GET_END_ITER:
	case GET_BEGIN_ITER:
	{
	    EggSequenceIter *begin_iter;
	    EggSequenceIter *end_iter;
	    EggSequenceIter *penultimate_iter;

	    begin_iter = egg_sequence_get_begin_iter (seq->sequence);
	    check_integrity (seq);

	    end_iter = egg_sequence_get_end_iter (seq->sequence);
	    check_integrity (seq);

	    penultimate_iter = egg_sequence_iter_prev (end_iter);
	    check_integrity (seq);

	    if (egg_sequence_get_length (seq->sequence) > 0)
	    {
		g_assert (seq->queue->head);
		g_assert (seq->queue->head->data == begin_iter);
		g_assert (seq->queue->tail);
		g_assert (seq->queue->tail->data == penultimate_iter);
	    }
	    else
	    {
		g_assert (penultimate_iter == end_iter);
		g_assert (begin_iter == end_iter);
		g_assert (penultimate_iter == begin_iter);
		g_assert (seq->queue->head == NULL);
		g_assert (seq->queue->tail == NULL);
	    }
	}
	break;
	case GET_ITER_AT_POS:
	{
	    int i;

	    g_assert (g_queue_get_length (seq->queue) == egg_sequence_get_length (seq->sequence));
	    
	    for (i = 0; i < 10; ++i)
	    {
		int pos = get_random_position (seq);
		EggSequenceIter *iter = egg_sequence_get_iter_at_pos (seq->sequence, pos);
		GList *link = g_queue_peek_nth_link (seq->queue, pos);
		check_integrity (seq);
		if (pos >= egg_sequence_get_length (seq->sequence) || pos < 0)
		{
		    g_assert (iter == egg_sequence_get_end_iter (seq->sequence));
		    g_assert (link == NULL);
		}
		else
		{
		    g_assert (link);
		    g_assert (link->data == iter);
		}
	    }
	}
	break;
	case APPEND:
	{
	    for (i = 0; i < 10; ++i)
	    {
		EggSequenceIter *iter = egg_sequence_append (seq->sequence, new_item (seq));
		g_queue_push_tail (seq->queue, iter);
	    }
	}
	break;
	case PREPEND:
	{
	    for (i = 0; i < 10; ++i)
	    {
		EggSequenceIter *iter = egg_sequence_prepend (seq->sequence, new_item (seq));
		g_queue_push_head (seq->queue, iter);
	    }
	}
	break;
	case INSERT_BEFORE:
	{
	    for (i = 0; i < 10; ++i)
	    {
		GList *link;
		EggSequenceIter *iter = get_random_iter (seq, &link);
		EggSequenceIter *new_iter;
		check_integrity (seq);
		
		new_iter = egg_sequence_insert_before (iter, new_item (seq));

		queue_insert_before (seq, link, new_iter);
	    }
	}
	break;
 	case MOVE:
	{
	    GList *link1, *link2;
	    EggSequenceIter *iter1 = get_random_iter (seq, &link1);
	    EggSequenceIter *iter2 = get_random_iter (seq, &link2);

	    if (!egg_sequence_iter_is_end (iter1))
	    {
		egg_sequence_move (iter1, iter2);

		if (!link2)
		    g_assert (egg_sequence_iter_is_end (iter2));

		queue_insert_before (seq, link2, link1->data);
		
		g_queue_delete_link (seq->queue, link1);
	    }

	    check_integrity (seq);
	    
	    iter1 = get_random_iter (seq, NULL);

	    /* Moving an iter to itself should have no effect */
	    if (!egg_sequence_iter_is_end (iter1))
		egg_sequence_move (iter1, iter1);
	}
	break;
	case INSERT_SORTED:
	{
	    int i;
	    dump_info (seq);
 
	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);

	    for (i = 0; i < 15; ++i)
	    {
		EggSequenceIter *iter =
		    egg_sequence_insert_sorted (
			seq->sequence, new_item(seq), compare_items, NULL);

		g_queue_insert_sorted (seq->queue, iter, compare_iters, NULL);
	    }

	    check_sorted (seq);
	    
	    dump_info (seq);
	}
	break;
	case INSERT_SORTED_ITER:
	{
	    int i;
	    dump_info (seq);
 
	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);

	    for (i = 0; i < 15; ++i)
	    {
		EggSequenceIter *iter =
		    egg_sequence_insert_sorted_iter (
			seq->sequence, new_item (seq),
			(EggSequenceIterCompareFunc)compare_iters, NULL);

		g_queue_insert_sorted (seq->queue, iter, compare_iters, NULL);
	    }

	    check_sorted (seq);
	    
	    dump_info (seq);
	}
	break;
	case SORT_CHANGED:
	{
	    int i;

	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);

	    for (i = 0; i < 15; ++i)
	    {
		GList *link;
		EggSequenceIter *iter = get_random_iter (seq, &link);

		if (!egg_sequence_iter_is_end (iter))
		{
		    egg_sequence_set (iter, new_item (seq));
		    egg_sequence_sort_changed (iter, compare_items, NULL);

		    g_queue_delete_link (seq->queue, link);
		    g_queue_insert_sorted (seq->queue, iter, compare_iters, NULL);
		}

		check_sorted (seq);
	    }
	}
	break;
	case SORT_CHANGED_ITER:
	{
	    int i;

	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);

	    for (i = 0; i < 15; ++i)
	    {
		GList *link;
		EggSequenceIter *iter = get_random_iter (seq, &link);

		if (!egg_sequence_iter_is_end (iter))
		{
		    egg_sequence_set (iter, new_item (seq));
		    egg_sequence_sort_changed_iter (
			iter, (EggSequenceIterCompareFunc)compare_iters, NULL);

		    g_queue_delete_link (seq->queue, link);
		    g_queue_insert_sorted (seq->queue, iter, compare_iters, NULL);
		}

		check_sorted (seq);
	    }
	}
	break;
	case REMOVE:
	{
	    int i;

	    for (i = 0; i < 15; ++i)
	    {
		GList *link;
		EggSequenceIter *iter = get_random_iter (seq, &link);
		
		if (!egg_sequence_iter_is_end (iter))
		{
		    egg_sequence_remove (iter);
		    g_queue_delete_link (seq->queue, link);
		}
	    }
	}
	break;
	case REMOVE_RANGE:
	{
	    EggSequenceIter *begin_iter, *end_iter;
	    GList *begin_link, *end_link;
	    GList *list;

	    get_random_range (seq, &begin_iter, &end_iter, &begin_link, &end_link);

	    egg_sequence_remove_range (begin_iter, end_iter);

	    list = begin_link;
	    while (list != end_link)
	    {
		GList *next = list->next;

		g_queue_delete_link (seq->queue, list);

		list = next;
	    }
	}
	break;
	case MOVE_RANGE:
	{
	    SequenceInfo *src = RANDOM_SEQUENCE();
	    SequenceInfo *dst = RANDOM_SEQUENCE();

	    EggSequenceIter *begin_iter, *end_iter;
	    GList *begin_link, *end_link;

	    EggSequenceIter *dst_iter;
	    GList *dst_link;

	    GList *list;

	    g_assert (src->queue);
	    g_assert (dst->queue);
	    
	    get_random_range (src, &begin_iter, &end_iter, &begin_link, &end_link);
	    dst_iter = get_random_iter (dst, &dst_link);
	    
	    egg_sequence_move_range (dst_iter, begin_iter, end_iter);

	    if (dst_link == begin_link || (src == dst && dst_link == end_link))
	    {
		check_integrity (src);
		check_integrity (dst);
		break;
	    }
	    
	    if (queue_link_index (src, begin_link) >=
		queue_link_index (src, end_link))
	    {
		break;
	    }

	    if (src == dst &&
		queue_link_index (src, dst_link) >= queue_link_index (src, begin_link) &&
		queue_link_index (src, dst_link) <= queue_link_index (src, end_link))
	    {
		break;
	    }

	    list = begin_link;
	    while (list != end_link)
	    {
		GList *next = list->next;
		Item *item = egg_sequence_get (list->data);

		g_assert (dst->queue);
		queue_insert_before (dst, dst_link, list->data);
		g_queue_delete_link (src->queue, list);

		g_assert (item->seq == src);
		
		src->n_items--;
		dst->n_items++;
		item->seq = dst;

		list = next;
	    }
	}
	break;
	case SEARCH:
	{
	    Item *item;
	    EggSequenceIter *search_iter;
	    EggSequenceIter *insert_iter;
	    
	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);

	    item = new_item (seq);
	    search_iter = egg_sequence_search (seq->sequence, item, compare_items, NULL);

	    insert_iter = egg_sequence_insert_sorted (seq->sequence, item, compare_items, NULL);

	    g_assert (search_iter == egg_sequence_iter_next (insert_iter));

	    g_queue_insert_sorted (seq->queue, insert_iter, compare_iters, NULL);
	}
	break;
	case SEARCH_ITER:
	{
	    Item *item;
	    EggSequenceIter *search_iter;
	    EggSequenceIter *insert_iter;
	    
	    egg_sequence_sort (seq->sequence, compare_items, NULL);
	    g_queue_sort (seq->queue, compare_iters, NULL);

	    check_sorted (seq);

	    item = new_item (seq);
	    search_iter = egg_sequence_search_iter (
		seq->sequence, item, (EggSequenceIterCompareFunc)compare_iters, NULL);

	    insert_iter = egg_sequence_insert_sorted (seq->sequence, item, compare_items, NULL);

	    g_assert (search_iter == egg_sequence_iter_next (insert_iter));

	    g_queue_insert_sorted (seq->queue, insert_iter, compare_iters, NULL);
	}
	break;
	
/* dereferencing */
	case GET:
	case SET:
	{
	    EggSequenceIter *iter;
	    GList *link;
	    
	    iter = get_random_iter (seq, &link);

	    if (!egg_sequence_iter_is_end (iter))
	    {
		Item *item;
		int i;

		check_integrity (seq);

		/* Test basic functionality */
		item = new_item (seq);
		egg_sequence_set (iter, item);
		g_assert (egg_sequence_get (iter) == item);

		/* Make sure that existing items are freed */
		for (i = 0; i < 15; ++i)
		    egg_sequence_set (iter, new_item (seq));
		
		check_integrity (seq);

		egg_sequence_set (iter, new_item (seq));
	    }
	}
	break;
	
/* operations on EggSequenceIter * */
	case ITER_IS_BEGIN:
	{
	    EggSequenceIter *iter;

	    iter = egg_sequence_get_iter_at_pos (seq->sequence, 0);

	    g_assert (egg_sequence_iter_is_begin (iter));

	    check_integrity (seq);

	    if (egg_sequence_get_length (seq->sequence) > 0)
	    {
		g_assert (!egg_sequence_iter_is_begin (
			      egg_sequence_get_end_iter (seq->sequence)));
	    }
	    else
	    {
		g_assert (egg_sequence_iter_is_begin (
			      egg_sequence_get_end_iter (seq->sequence)));
	    }

	    g_assert (egg_sequence_iter_is_begin (egg_sequence_get_begin_iter (seq->sequence)));
	}
	break;
	case ITER_IS_END:
	{
	    EggSequenceIter *iter;
	    int len = egg_sequence_get_length (seq->sequence);

	    iter = egg_sequence_get_iter_at_pos (seq->sequence, len);

	    g_assert (egg_sequence_iter_is_end (iter));

	    if (len > 0)
	    {
		g_assert (!egg_sequence_iter_is_end (
			      egg_sequence_get_begin_iter (seq->sequence)));
	    }
	    else
	    {
		g_assert (egg_sequence_iter_is_end (
			      egg_sequence_get_begin_iter (seq->sequence)));
	    }

	    g_assert (egg_sequence_iter_is_end (egg_sequence_get_end_iter (seq->sequence)));
	}
	break;
	case ITER_NEXT:
	{
	    EggSequenceIter *iter1, *iter2, *iter3, *end;

	    iter1 = egg_sequence_append (seq->sequence, new_item (seq));
	    iter2 = egg_sequence_append (seq->sequence, new_item (seq));
	    iter3 = egg_sequence_append (seq->sequence, new_item (seq));

	    end = egg_sequence_get_end_iter (seq->sequence);
	    
	    g_assert (egg_sequence_iter_next (iter1) == iter2);
	    g_assert (egg_sequence_iter_next (iter2) == iter3);
	    g_assert (egg_sequence_iter_next (iter3) == end);
	    g_assert (egg_sequence_iter_next (end) == end);

	    g_queue_push_tail (seq->queue, iter1);
	    g_queue_push_tail (seq->queue, iter2);
	    g_queue_push_tail (seq->queue, iter3);
	}
	break;
	case ITER_PREV:
	{
	    EggSequenceIter *iter1, *iter2, *iter3, *begin;

	    iter1 = egg_sequence_prepend (seq->sequence, new_item (seq));
	    iter2 = egg_sequence_prepend (seq->sequence, new_item (seq));
	    iter3 = egg_sequence_prepend (seq->sequence, new_item (seq));

	    begin = egg_sequence_get_begin_iter (seq->sequence);
	    
	    g_assert (egg_sequence_iter_prev (iter1) == iter2);
	    g_assert (egg_sequence_iter_prev (iter2) == iter3);
	    g_assert (iter3 == begin);
	    g_assert (egg_sequence_iter_prev (iter3) == begin);
	    g_assert (egg_sequence_iter_prev (begin) == begin);

	    g_queue_push_head (seq->queue, iter1);
	    g_queue_push_head (seq->queue, iter2);
	    g_queue_push_head (seq->queue, iter3);
	}
	break;
	case ITER_GET_POSITION:
	{
	    GList *link;
	    EggSequenceIter *iter = get_random_iter (seq, &link);

	    g_assert (egg_sequence_iter_get_position (iter) ==
		      queue_link_index (seq, link));
	}
	break;
	case ITER_MOVE:
	{
	    int len = egg_sequence_get_length (seq->sequence);
	    EggSequenceIter *iter;
	    int pos;

	    iter = get_random_iter (seq, NULL);
	    pos = egg_sequence_iter_get_position (iter);
	    iter = egg_sequence_iter_move (iter, len - pos);
	    g_assert (egg_sequence_iter_is_end (iter));


	    iter = get_random_iter (seq, NULL);
	    pos = egg_sequence_iter_get_position (iter);
	    while (pos < len)
	    {
		g_assert (!egg_sequence_iter_is_end (iter));
		pos++;
		iter = egg_sequence_iter_move (iter, 1);
	    }
	    g_assert (egg_sequence_iter_is_end (iter));
	}
	break;
	case ITER_GET_SEQUENCE:
	{
	    EggSequenceIter *iter = get_random_iter (seq, NULL);

	    g_assert (egg_sequence_iter_get_sequence (iter) == seq->sequence);
	}
	break;
	
/* search */
	case ITER_COMPARE:
	{
	    GList *link1, *link2;
	    EggSequenceIter *iter1 = get_random_iter (seq, &link1);
	    EggSequenceIter *iter2 = get_random_iter (seq, &link2);

	    int cmp = egg_sequence_iter_compare (iter1, iter2);
	    int pos1 = queue_link_index (seq, link1);
	    int pos2 = queue_link_index (seq, link2);

	    if (cmp == 0)
	    {
		g_assert (pos1 == pos2);
	    }
	    else if (cmp < 0)
	    {
		g_assert (pos1 < pos2);
	    }
	    else
	    {
		g_assert (pos1 > pos2);
	    }
	}
	break;
	case RANGE_GET_MIDPOINT:
	{
	    EggSequenceIter *iter1 = get_random_iter (seq, NULL);
	    EggSequenceIter *iter2 = get_random_iter (seq, NULL);
	    EggSequenceIter *iter3;
	    int cmp;

	    cmp = egg_sequence_iter_compare (iter1, iter2);
	    
	    if (cmp > 0)
	    {
		EggSequenceIter *tmp;

		tmp = iter1;
		iter1 = iter2;
		iter2 = tmp;
	    }
	    
	    iter3 = egg_sequence_range_get_midpoint (iter1, iter2);

	    if (cmp == 0)
	    {
		g_assert (iter3 == iter1);
		g_assert (iter3 == iter2);
	    }

	    g_assert (egg_sequence_iter_get_position (iter3) >= 
		      egg_sequence_iter_get_position (iter1));
	    g_assert (egg_sequence_iter_get_position (iter2) >= 
		      egg_sequence_iter_get_position (iter3));
	}
	break;

	}
	
	check_integrity (seq);
    }
    
    return 0;
}


/* Single, stand-alone tests */

static void
test_out_of_range_jump (void)
{
    EggSequence *seq = egg_sequence_new (NULL);
    EggSequenceIter *iter = egg_sequence_get_begin_iter (seq);

    egg_sequence_iter_move (iter, 5);

    g_assert (egg_sequence_iter_is_begin (iter));
    g_assert (egg_sequence_iter_is_end (iter));
}

static void
single_tests (void)
{
    test_out_of_range_jump ();
}

/* seeds known to have failed at some point:

801678400
1477639090
3369132895
1192944867
770458294
1099575817
590523467
3583571454

*/
