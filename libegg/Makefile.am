if HAVE_GTK_TARGET_X11
background_monitor_SUBDIRS = \
  background-monitor
endif

SUBDIRS = util column-chooser treeviewutils \
  $(background_monitor_SUBDIRS) \
  datetime \
  fileformatchooser \
  pixbufthumbnail \
  spreadtable \
  thumbnailpreview \
  smclient \
  wrapbox

# Don't compile broken code
# SUBDIRS += iconchooser

AM_CPPFLAGS = \
  $(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
  -DEGG_COMPILATION

##noinst_LTLIBRARIES = libegg.la
