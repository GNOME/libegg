INCLUDES = \
  $(EGG_30_CFLAGS) \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED \
  -DGETTEXT_PACKAGE=\"libegg\"

noinst_LTLIBRARIES = libegg-spreadtable.la

libegg_spreadtable_la_LIBADD = $(EGG_30_LIBS)

EGGSOURCES = \
	eggspreadtable.c \
	eggspreadtablednd.c

EGGHEADERS = \
	eggspreadtable.h \
	eggspreadtablednd.h

libegg_spreadtable_la_SOURCES = $(EGGSOURCES)

noinst_HEADERS = $(EGG_HEADERS)

#Test program

noinst_PROGRAMS = testspreadtable

testspreadtable_SOURCES = testspreadtable.c

testspreadtable_LDADD = $(EGG_30_LIBS) \
  $(top_builddir)/libegg/spreadtable/libegg-spreadtable.la
