
#ifndef __EGG_FILE_SYSTEM_UTIL__
#define __EGG_FILE_SYSTEM_UTIL__

#include <gtk/gtk.h>
#include <libgnomeui/gnome-icon-theme.h>

G_BEGIN_DECLS

GdkPixbuf * egg_file_system_util_get_icon (GnomeIconTheme *theme,
				      const gchar *uri,
				      const gchar *mime_type);

G_END_DECLS

#endif /* __EGG_FILE_SYSTEM_UTIL__ */
