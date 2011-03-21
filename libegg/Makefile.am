if HAVE_GTK30
spreadtable_SUBDIRS = \
  spreadtable
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

if HAVE_GTK210
gtk210_SUBDIRS = \
  fileformatchooser
endif

if HAVE_GTK215
gtk215_SUBDIRS =
endif

if HAVE_GTK_TARGET_X11
background_monitor_SUBDIRS = \
  background-monitor
endif

SUBDIRS = util column-chooser treeviewutils \
  $(background_monitor_SUBDIRS) \
  dock datetime $(thumbnail_SUBDIRS) \
  smclient $(spreadtable_SUBDIRS) \
  $(gtk210_SUBDIRS) \
  $(gtk215_SUBDIRS) \
  $(gtk30_SUBDIRS)

# Don't compile broken code
#$(iconchooser_SUBDIRS)

INCLUDES = \
  $(EGG_CFLAGS)  \
  -DEGG_COMPILATION  \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED

##noinst_LTLIBRARIES = libegg.la
