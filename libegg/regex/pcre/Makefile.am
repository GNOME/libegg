## Process this file with automake to produce Makefile.in

INCLUDES = \
  $(EGG_CFLAGS) \
  -I$(top_srcdir)/libegg/util \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED

noinst_LTLIBRARIES = libpcre.la

libpcre_la_SOURCES = 	\
	get.c \
	internal.h \
	pcre-config.h \
	maketables.c \
	chartables.c \
	pcre.c \
	pcre.h \
	printint.h \
	study.c
