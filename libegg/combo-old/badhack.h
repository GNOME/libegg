/* hahaha bad hack
 *
 * The entry includes some internal headers. So I pasted the internal bits
 * it needed here. It seems to compile.
 */


/* gtkmarshalers.h */
#include <glib-object.h>


extern void _gtk_marshal_VOID__ENUM_INT_BOOLEAN (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

extern void _gtk_marshal_VOID__ENUM_INT (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data);

#define _gtk_marshal_VOID__OBJECT       g_cclosure_marshal_VOID__OBJECT

#define _gtk_marshal_VOID__VOID g_cclosure_marshal_VOID__VOID

#define _gtk_marshal_VOID__STRING       g_cclosure_marshal_VOID__STRING


/* gtktextutil.h */
#include <gtk/gtkwidget.h>
#include <gtk/gtkmenushell.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* This is a private uninstalled header shared between GtkTextView and GtkEntry */
typedef void (* GtkTextUtilCharChosenFunc) (const char *text,
                                            gpointer    data);

void _gtk_text_util_append_special_char_menuitems (GtkMenuShell              *menushell,
                                                   GtkTextUtilCharChosenFunc  func,
                                                   gpointer                   data);


#ifdef __cplusplus
}
#endif /* __cplusplus */
