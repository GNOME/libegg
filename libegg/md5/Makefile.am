# This file will be processed with automake-1.7 to create Makefile.in

INCLUDES = \
	-DG_LOG_DOMAIN=\""EggMd5"\" \
	$(EGG_CFLAGS)

noinst_LTLIBRARIES = \
	libeggmd5.la

libeggmd5_la_LIBADD = \
	$(EGG_LIBS)

libeggmd5_la_SOURCES =	\
	eggmd5.c

noinst_PROGRAMS = \
	testeggmd5

testeggmd5_CFLAGS = \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DG_LOG_DOMAIN=\""Egg"\" \
	-DGTK_FILE_SYSTEM_ENABLE_UNSUPPORTED \
	$(EGG_CFLAGS)

testeggmd5_LDADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/md5/libeggmd5.la

testeggmd5_SOURCES = \
	testeggmd5.c

EXTRA_DIST = \
	README \
	eggmd5.h

MAINTAINERCLEANFILES = \
	Makefile.in
