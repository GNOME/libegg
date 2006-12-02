# This file will be processed with automake-1.7 to create Makefile.in

INCLUDES = \
	-DG_LOG_DOMAIN=\""EggSequence"\" \
	$(EGG_CFLAGS)

noinst_LTLIBRARIES = \
	libeggsequence.la

libeggsequence_la_LIBADD = \
	$(EGG_LIBS)

libeggsequence_la_SOURCES =	\
	eggsequence.c

noinst_PROGRAMS = \
	testsequence \
	benchsortedsequence

testsequence_CFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DG_LOG_DOMAIN=\""Egg"\" \
	-DGTK_FILE_SYSTEM_ENABLE_UNSUPPORTED \
	$(EGG_CFLAGS)

testsequence_LDADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/sequence/libeggsequence.la

testsequence_SOURCES = \
	testsequence.c

benchsortedsequence_LDADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/sequence/libeggsequence.la

benchsortedsequence_SOURCES = \
	benchsortedsequence.c

EXTRA_DIST = \
	README \
	eggsequence.h

MAINTAINERCLEANFILES = \
	Makefile.in
