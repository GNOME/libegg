#ifndef __EGG_HISTORY_ENTRY_H__
#define __EGG_HISTORY_ENTRY_H__

#include <gtk/gtkentry.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeviewcolumn.h>
#include "egghistorylist.h"

G_BEGIN_DECLS

#define EGG_TYPE_HISTORY_ENTRY			(egg_history_entry_get_type ())
#define EGG_HISTORY_ENTRY(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), EGG_TYPE_HISTORY_ENTRY, EggHistoryEntry))
#define EGG_HISTORY_ENTRY_CLASS(vtable)		(G_TYPE_CHECK_CLASS_CAST ((vtable), EGG_TYPE_HISTORY_ENTRY, EggHistoryEntryClass))
#define EGG_IS_HISTORY_ENTRY(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EGG_TYPE_HISTORY_ENTRY))
#define EGG_IS_HISTORY_ENTRY_CLASS(vtable)	(G_TYPE_CHECK_CLASS_TYPE ((vtable), EGG_TYPE_HISTORY_ENTRY))
#define EGG_HISTORY_ENTRY_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_HISTORY_ENTRY, EggHistoryEntryClass))

typedef struct _EggHistoryEntry		EggHistoryEntry;
typedef struct _EggHistoryEntryClass	EggHistoryEntryClass;

struct _EggHistoryEntry
{
  GtkEntry parent_instance;

  /*< private >*/
  gint text_column;
  gint list_column;
  gint entry_column;

  gint sample_index;

  gint history_max;

  GtkTreeModel *model;
  GtkWidget *tree_view;
  GtkWidget *popup_window;
  GtkWidget *popup_frame;

  gboolean popped_up : 1;
  gboolean popup_in_progress : 1;
};

struct _EggHistoryEntryClass
{
  GtkEntryClass parent_class;

  /* signals */
  void     (* active_changed)     (EggHistoryEntry *entry);
  void     (* popped_up)          (EggHistoryEntry *entry);
  void     (* popped_down)        (EggHistoryEntry *entry);

  /* key binding signals */
  gboolean (* popup)              (EggHistoryEntry *entry);
};

typedef gboolean (* EggCompletionFunc) (const gchar *key,
                                        const gchar *item,
                                        GtkTreeIter *iter,
                                        gpointer     user_data);


GType              egg_history_entry_get_type                    (void);
GtkWidget         *egg_history_entry_new                         (GtkTreeModel      *model,
                                                                  gint               text_column);

void               egg_history_entry_set_show_completion_popdown (EggHistoryEntry   *entry,
                                                                  gboolean           show);
gboolean           egg_history_entry_get_show_completion_popdown (EggHistoryEntry   *entry);

void               egg_history_entry_set_show_history_popdown    (EggHistoryEntry   *entry,
                                                                  gboolean           show);
gboolean           egg_history_entry_get_show_history_popdown    (EggHistoryEntry   *entry);

void               egg_history_entry_enable_completion           (EggHistoryEntry   *entry,
                                                                  GtkListStore      *model,
                                                                  gint               list_column,
                                                                  gint               entry_column,
                                                                  EggCompletionFunc  func,
                                                                  gpointer           func_data,
                                                                  GDestroyNotify     func_destroy);
gboolean           egg_history_entry_get_completion_enabled      (EggHistoryEntry   *entry);

GtkTreeViewColumn *egg_history_entry_completion_get_column       (EggHistoryEntry   *entry);
void               egg_history_entry_completion_get_model        (EggHistoryEntry   *entry,
                                                                  GtkTreeModel     **model,
                                                                  gint              *list_model);

void               egg_history_entry_set_max                     (EggHistoryEntry   *entry,
                                                                  gint               max);
gint               egg_history_entry_get_max                     (EggHistoryEntry   *entry);

G_END_DECLS

#endif /* __EGG_HISTORY_ENTRY_H__ */
