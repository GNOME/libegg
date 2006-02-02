# $Id$

INCLUDES = \
	$(EGG_CFLAGS)			\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_DISABLE_DEPRECATED	\
	-DG_DISABLE_DEPRECATED		\
	-DGETTEXT_PACKAGE=NULL

noinst_LTLIBRARIES = libeggdatetime.la

libeggdatetime_la_LIBADD = $(EGG_LIBS)

libeggdatetime_la_SOURCES = egg-datetime.c

noinst_HEADERS = egg-datetime.h

noinst_PROGRAMS = test-datetime
test_datetime_SOURCES = test-datetime.c
test_datetime_LDADD = $(EGG_LIBS) $(top_builddir)/libegg/datetime/libeggdatetime.la
