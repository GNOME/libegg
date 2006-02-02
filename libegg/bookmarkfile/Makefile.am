NULL =

DO_PROFILE = -DDO_PROFILE=1
DO_DEBUG = -DDO_DEBUG=1

INCLUDES = \
	-DG_LOG_DOMAIN="\"EggBookmarkFile\""	\
	$(NULL)

noinst_LTLIBRARIES = libeggbookmarkfile.la

libeggbookmarkfile_la_CFLAGS = -O2 $(EGG_CFLAGS)
libeggbookmarkfile_la_LIBADD = \
	$(EGG_LIBS) \
	$(NULL)

libeggbookmarkfile_la_SOURCES = \
	eggbookmarkfile.c	\
	$(NULL)

noinst_HEADERS = \
	eggbookmarkfile.h	\
	$(NULL)

noinst_PROGRAMS = \
	testbookmarkfile \
	$(NULL)

testbookmarkfile_SOURCES = testbookmarkfile.c
testbookmarkfile_CFLAGS = $(EGG_CFLAGS)
testbookmarkfile_LDFLAGS = $(EGG_LIBS) $(top_builddir)/libegg/bookmarkfile/libeggbookmarkfile.la
testbookmarkfile_DEPENDENCIES = $(top_builddir)/libegg/bookmarkfile/libeggbookmarkfile.la

EXTRA_DIST = README test-file.xbel test-one.xbel bookmark-spec.xml
