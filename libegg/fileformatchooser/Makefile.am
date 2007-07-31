INCLUDES = \
  $(EGG_CFLAGS) \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED


noinst_LTLIBRARIES = libeggfileformatchooser.la

libeggfileformatchooser_la_LIBADD = $(EGG_LIBS)

libeggfileformatchooser_la_SOURCES = \
  eggfileformatchooser.c

noinst_HEADERS = \
  eggfileformatchooser.h

noinst_PROGRAMS = testfileformatchooser

testfileformatchooser_SOURCES = testfileformatchooser.c
testfileformatchooser_LDFLAGS = libeggfileformatchooser.la $(EGG_LIBS)
testfileformatchooser_DEPENDENCIES = libeggfileformatchooser.la 

