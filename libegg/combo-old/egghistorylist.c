#include "egghistorylist.h"


GType
egg_history_list_get_type (void)
{
  static GType history_list_type = 0;

  if (!history_list_type)
    {
      static const GTypeInfo history_list_info =
      {
        sizeof (EggHistoryListIface),
        NULL, /* base_init */
        NULL, /* base_finalize */
        NULL,
        NULL, /* class_finalize */
        NULL, /* class_data */
        0, 
        0,
        NULL
      };

      history_list_type = 
        g_type_register_static (G_TYPE_INTERFACE, "EggHistoryList",
                                &history_list_info, 0);

      g_type_interface_add_prerequisite (history_list_type, G_TYPE_OBJECT);
    }

  return history_list_type;
}


void
egg_history_list_add_item (EggHistoryList *list,
                           const gchar    *item)
{
  g_return_if_fail (EGG_IS_HISTORY_LIST (list));
  g_return_if_fail (item != NULL);

  return (* EGG_HISTORY_LIST_GET_IFACE (list)->add_item) (list, item);
}

void
egg_history_list_remove_from_bottom (EggHistoryList *list,
                                     gint            count)
{
  g_return_if_fail (EGG_IS_HISTORY_LIST (list));
  g_return_if_fail (count >= 0);

  return (* EGG_HISTORY_LIST_GET_IFACE (list)->remove_from_bottom) (list, count);
}
