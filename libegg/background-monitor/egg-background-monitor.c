/*
 * (c) Copyright 2002 Ian McKellar <yakk@yakk.net>
 */

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#include "egg-background-monitor.h"

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

static GdkFilterReturn egg_background_monitor_xevent_filter (GdkXEvent 	*xevent,
				  			 GdkEvent	*event,
				  			 gpointer	 data);

struct _EggBackgroundMonitorClass
{
	GObjectClass	parent_class;
	void (* changed) (EggBackgroundMonitor *monitor);
};

struct _EggBackgroundMonitor {
	GObject		 parent_instance;

	XID		 xwindow;
	GdkWindow	*gdkwindow;

	Atom		 xatom;
	GdkAtom		 gdkatom;

	GdkPixmap	*gdkpixmap;
	GdkPixbuf	*gdkpixbuf;

	int		 width;
	int		 height;
};

static void
egg_background_monitor_finalize (GObject *object)
{
	EggBackgroundMonitor *monitor;

	monitor = EGG_BACKGROUND_MONITOR (object);

	g_object_unref (G_OBJECT (monitor->gdkpixmap));
	g_object_unref (G_OBJECT (monitor->gdkpixbuf));

	gdk_window_remove_filter(monitor->gdkwindow,
			egg_background_monitor_xevent_filter, monitor);
}

static void
egg_background_monitor_class_init (EggBackgroundMonitorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	signals[CHANGED] =
		g_signal_new ("changed",
				G_OBJECT_CLASS_TYPE (object_class),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (EggBackgroundMonitorClass,
					changed),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0);

	object_class->finalize = egg_background_monitor_finalize;
}

static void
egg_background_monitor_init (EggBackgroundMonitor *monitor)
{
	monitor->gdkwindow = gdk_get_default_root_window ();
	monitor->xwindow = GDK_DRAWABLE_XID (monitor->gdkwindow);

	monitor->gdkatom = gdk_atom_intern ("_XROOTPMAP_ID", TRUE);
	monitor->xatom = gdk_x11_atom_to_xatom (monitor->gdkatom);

	monitor->gdkpixmap = NULL;
	monitor->gdkpixbuf = NULL;

	gdk_window_add_filter(monitor->gdkwindow,
			egg_background_monitor_xevent_filter, monitor);

	gdk_window_set_events(monitor->gdkwindow,
			gdk_window_get_events(monitor->gdkwindow) |
			GDK_PROPERTY_CHANGE_MASK);
}

GType
egg_background_monitor_get_type (void)
{
	static GType object_type = 0;
	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (EggBackgroundMonitorClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) egg_background_monitor_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (EggBackgroundMonitor),
			0,              /* n_preallocs */
			(GInstanceInitFunc) egg_background_monitor_init,
		};
		object_type = g_type_register_static (G_TYPE_OBJECT,
				"EggBackgroundMonitor", &object_info, 0);
	}
	return object_type;
}

EggBackgroundMonitor *
egg_background_monitor_new () {
	EggBackgroundMonitor *p;

	p = (EggBackgroundMonitor *)g_object_new (egg_background_monitor_get_type (),
			NULL);
	return p;
}



static GdkFilterReturn
egg_background_monitor_xevent_filter (GdkXEvent 	*xevent,
				  GdkEvent	*event,
				  gpointer	 data)
{
	XEvent 			*xev;
	EggBackgroundMonitor	*monitor;

	xev = (XEvent *)xevent;
	monitor = EGG_BACKGROUND_MONITOR (data);

	if (xev->type == PropertyNotify) {
		if (xev->xproperty.atom == monitor->xatom &&
				xev->xproperty.window == monitor->xwindow) {
			monitor->gdkpixmap = NULL;
			monitor->gdkpixbuf = NULL;
			g_signal_emit (G_OBJECT (monitor), signals[CHANGED], 0);
		}
	}
	return GDK_FILTER_CONTINUE;
}

static void
egg_background_monitor_setup_pixmap (EggBackgroundMonitor *monitor)
{
	union {
		Pixmap	*pixmap;
		guchar  *data;
	} prop_data;
	GdkAtom	 prop_type;

	prop_data.data = NULL;

	gdk_property_get(monitor->gdkwindow, monitor->gdkatom, 0, 0, 10,
			FALSE, &prop_type, NULL, NULL, &prop_data.data);

	if ((prop_type == GDK_TARGET_PIXMAP)
			&& (prop_data.pixmap != NULL)
			&& (prop_data.pixmap[0] != 0)) {
		monitor->gdkpixmap = gdk_pixmap_foreign_new(prop_data.pixmap[0]);

		if (monitor->gdkpixmap == NULL) {
			g_print ("couldn't get background pixmap\n");
		}
	}

}

static void
egg_background_monitor_setup_pixbuf (EggBackgroundMonitor *monitor)
{
	GdkColormap *colormap = NULL;
	int rwidth, rheight, pwidth, pheight;

	if (monitor->gdkpixmap == NULL) {
		egg_background_monitor_setup_pixmap (monitor);
	}
	if (monitor->gdkpixmap == NULL) {
		return;
	}

	gdk_pixmap_get_size (monitor->gdkpixmap,
			&pwidth, &pheight);

  rwidth = gdk_window_get_width(monitor->gdkwindow);
  rheight = gdk_window_get_height(monitor->gdkwindow);

	monitor->width = MIN(pwidth, rwidth);
	monitor->height = MIN(pheight, rheight);

	colormap = gdk_drawable_get_colormap(monitor->gdkwindow);

	monitor->gdkpixbuf = gdk_pixbuf_get_from_drawable(NULL,
			monitor->gdkpixmap, colormap, 0, 0, 0, 0,
			monitor->width, monitor->height);

}

GdkPixbuf *
egg_background_monitor_get_region (EggBackgroundMonitor *monitor, int x, int y,
		int width, int height)
{
	int subwidth, subheight, subx, suby;
	GdkPixbuf *pixbuf, *tmpbuf;

	if (monitor->gdkpixbuf == NULL) {
		egg_background_monitor_setup_pixbuf (monitor);
	}
	if (monitor->gdkpixbuf == NULL) {
		return NULL;
	}

	subwidth = MIN (width, monitor->width-x);
	subheight = MIN (height, monitor->height-y);
	subx = MAX (x, 0);
	suby = MAX (y, 0);

	if ( (subwidth <= 0) || (subheight <= 0) ||
			(monitor->width-x < 0) || (monitor->height-y < 0) ) {
		/* region is completely offscreen */
		return gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width,
				height); /* who cares */
	}

	pixbuf = gdk_pixbuf_new_subpixbuf (monitor->gdkpixbuf, subx, suby,
			subwidth, subheight);

	/* FIXME: don't handle regions off the top or left edge */

	if ( (subwidth < width) || (subheight < height) ) {
		tmpbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
				width, height);
		gdk_pixbuf_composite (pixbuf, tmpbuf, 0, 0, subwidth, subheight,
				0.0, 0.0, 1.0, 1.0, GDK_INTERP_NEAREST, 255);
		g_object_unref (pixbuf);
		pixbuf = tmpbuf;
	}

	return pixbuf;
}

GdkPixbuf *
egg_background_monitor_get_widget_background (EggBackgroundMonitor *monitor,
		GtkWidget *widget)
{
	int x, y;

	/* FIXME: do we need deskrelative_origin? screw E IMHO */
	gdk_window_get_origin (widget->window, &x, &y);

	return egg_background_monitor_get_region (monitor, x+widget->allocation.x,
			y+widget->allocation.y,
			widget->allocation.width,
			widget->allocation.height);
}
