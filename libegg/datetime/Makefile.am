AM_CPPFLAGS = \
	$(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
	-DGETTEXT_PACKAGE=NULL

noinst_LTLIBRARIES = libeggdatetime.la

libeggdatetime_la_LIBADD = $(EGG_LIBS)

libeggdatetime_la_SOURCES = egg-datetime.c

noinst_HEADERS = egg-datetime.h

noinst_PROGRAMS = test-datetime
test_datetime_SOURCES = test-datetime.c
test_datetime_LDADD = $(EGG_LIBS) libeggdatetime.la
