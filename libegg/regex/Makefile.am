SUBDIRS = pcre

INCLUDES = \
  $(EGG_CFLAGS) \
  -I$(top_srcdir)/libegg/util \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED


noinst_LTLIBRARIES = libeggregex.la

libeggregex_la_LIBADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/regex/pcre/libpcre.la

libeggregex_la_SOURCES = \
  eggregex.c

noinst_HEADERS = \
  eggregex.h

noinst_PROGRAMS = testregex

testregex_SOURCES = testregex.c
testregex_LDFLAGS = libeggregex.la $(EGG_LIBS)
testregex_DEPENDENCIES = libeggregex.la 

