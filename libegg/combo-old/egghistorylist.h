#ifndef __EGG_HISTORY_LIST_H__
#define __EGG_HISTORY_LIST_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define EGG_TYPE_HISTORY_LIST           (egg_history_list_get_type ())
#define EGG_HISTORY_LIST(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_HISTORY_LIST, EggHistoryList))
#define EGG_IS_HISTORY_LIST(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_HISTORY_LIST))
#define EGG_HISTORY_LIST_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), EGG_TYPE_HISTORY_LIST, EggHistoryListIface))

typedef struct _EggHistoryList       EggHistoryList;      /* Dummy typedef */
typedef struct _EggHistoryListIface  EggHistoryListIface;


struct _EggHistoryListIface
{
  GTypeInterface g_iface;

  /* vtable */
  void (* add_item)             (EggHistoryList *list,
                                 const gchar    *item);
  void (* remove_from_bottom)   (EggHistoryList *list,
                                 gint            count);
};

GType    egg_history_list_get_type           (void);

void     egg_history_list_add_item           (EggHistoryList *list,
                                              const gchar    *item);
void     egg_history_list_remove_from_bottom (EggHistoryList *list,
                                              gint            count);


G_END_DECLS

#endif /* __EGG_HISTORY_LIST_H__ */
