if HAVE_GTK210
else
if HAVE_GTK21
tray_SUBDIRS = tray
endif
endif

if HAVE_GTK25
thumbnail_SUBDIRS = \
  pixbufthumbnail \
  thumbnailpreview
endif

if HAVE_GTK27
iconchooser_SUBDIRS = \
  iconchooser
endif

if HAVE_GTK215
gtk215_SUBDIRS = \
  toolpalette
endif

if HAVE_GTK_TARGET_X11
background_monitor_SUBDIRS = \
  background-monitor
endif

SUBDIRS = util column-chooser treeviewutils \
  sidebar $(background_monitor_SUBDIRS) \
  $(tray_SUBDIRS) dock datetime $(thumbnail_SUBDIRS) \
  $(iconchooser_SUBDIRS) smclient \
  fileformatchooser $(gtk215_SUBDIRS)

INCLUDES = \
  $(EGG_CFLAGS)  \
  -DEGG_COMPILATION  \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED

##noinst_LTLIBRARIES = libegg.la
