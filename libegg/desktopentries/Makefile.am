
INCLUDES = \
  $(EGG_DESKTOP_ENTRIES_CFLAGS) \
  -I$(top_srcdir)/libegg/util \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED


noinst_LTLIBRARIES = libeggdesktopentries.la

libeggdesktopentries_la_LIBADD = \
	$(EGG_DESKTOP_ENTRIES_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la

libeggdesktopentries_la_SOURCES = \
  eggdesktopentries.c \
  eggdirfuncs.c

noinst_HEADERS = \
  eggdesktopentries.h \
  eggdirfuncs.h

noinst_PROGRAMS = testentries

testentries_SOURCES = testentries.c
testentries_LDFLAGS = libeggdesktopentries.la $(EGG_DESKTOP_ENTRIES_LIBS)
testentries_DEPENDENCIES = libeggdesktopentries.la 
