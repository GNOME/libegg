
INCLUDES = \
  $(EGG_CFLAGS) \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED


noinst_LTLIBRARIES = libegg-druid.la

libegg_druid_la_LIBADD = $(EGG_LIBS)
libegg_druid_la_SOURCES = \
	eggdruid.c \
	eggdruid.h \
	eggdruiddialog.c \
	eggdruiddialog.h \
	eggdruidpage.c \
	eggdruidpage.h \
	eggdruidpagestandard.c \
	eggdruidpagestandard.h

noinst_PROGRAMS = test-druid

test_druid_SOURCES = test-druid.c
test_druid_LDFLAGS = libegg-druid.la $(EGG_LIBS)
test_druid_DEPENDENCIES = libegg-druid.la

