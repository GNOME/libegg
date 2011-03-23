AM_CPPFLAGS = \
  $(EGG_30_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
  -DEGG_COMPILATION \
  -DGETTEXT_PACKAGE=\"libegg\"

noinst_LTLIBRARIES = libegg-spreadtable.la

libegg_spreadtable_la_LIBADD = $(EGG_30_LIBS)

EGGSOURCES = \
	eggspreadtable.c

EGGHEADERS = \
	eggspreadtable.h

libegg_spreadtable_la_SOURCES = $(EGGSOURCES)

noinst_HEADERS = $(EGG_HEADERS)

#Test program

noinst_PROGRAMS = testspreadtable

testspreadtable_SOURCES = testspreadtable.c

testspreadtable_LDADD = $(EGG_30_LIBS) \
  $(top_builddir)/libegg/spreadtable/libegg-spreadtable.la
