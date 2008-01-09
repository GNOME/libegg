if HAVE_GTK210
else
if HAVE_GTK21
tray_SUBDIRS = tray
endif
endif

if HAVE_GTK24
else
menu_SUBDIRS = menu
endif

if HAVE_GTK25
thumbnail_SUBDIRS = \
  md5 \
  pixbufthumbnail \
  thumbnailpreview
endif

if HAVE_GTK27
iconchooser_SUBDIRS = \
  iconchooser
endif

if HAVE_GTK_TARGET_X11
background_monitor_SUBDIRS = \
  background-monitor
endif

SUBDIRS = util column-chooser treeviewutils toolbar $(menu_SUBDIRS) iconlist \
  sidebar druid filesystem $(background_monitor_SUBDIRS) recent-files \
  $(tray_SUBDIRS) dock datetime desktopentries $(thumbnail_SUBDIRS) \
  $(iconchooser_SUBDIRS) bookmarkfile recentchooser regex sequence smclient \
  fileformatchooser toolpalette

INCLUDES = \
  $(EGG_CFLAGS)  \
  -DEGG_COMPILATION  \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED

##noinst_LTLIBRARIES = libegg.la
