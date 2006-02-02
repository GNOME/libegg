/* 
 * (c) Copyright 2002 Ian McKellar <yakk@yakk.net>
 */

#ifndef EGG_BACKGROUND_MONITOR_H
#define EGG_BACKGROUND_MONITOR_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#define EGG_TYPE_BACKGROUND_MONITOR              (egg_background_monitor_get_type ())
#define EGG_BACKGROUND_MONITOR(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), EGG_TYPE_BACKGROUND_MONITOR, EggBackgroundMonitor))
#define EGG_BACKGROUND_MONITOR_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), EGG_TYPE_BACKGROUND_MONITOR, EggBackgroundMonitorClass))
#define EGG_IS_BACKGROUND_MONITOR(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), EGG_TYPE_BACKGROUND_MONITOR))
#define EGG_IS_BACKGROUND_MONITOR_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), EGG_TYPE_BACKGROUND_MONITOR))
#define EGG_BACKGROUND_MONITOR_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), EGG_TYPE_BACKGROUND_MONITOR, EggBackgroundMonitorClass))

typedef struct _EggBackgroundMonitorClass EggBackgroundMonitorClass;
typedef struct _EggBackgroundMonitor EggBackgroundMonitor;

GType			egg_background_monitor_get_type 			(void);
EggBackgroundMonitor*	egg_background_monitor_new			();
GdkPixbuf * 		egg_background_monitor_get_region (EggBackgroundMonitor *monitor, int x, int y, int width, int height);
GdkPixbuf * 		egg_background_monitor_get_widget_background (EggBackgroundMonitor *monitor, GtkWidget *widget);
#endif /* EGG_BACKGROUND_MONITOR_H */
