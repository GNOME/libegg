INCLUDES = \
  $(EGG_30_CFLAGS) $(EGG_WARNING_FLAGS) \
  -I$(top_srcdir)/libegg/util \
  -DEGG_COMPILATION


noinst_LTLIBRARIES = libeggfileformatchooser.la

libeggfileformatchooser_la_LIBADD = $(EGG_30_LIBS)

libeggfileformatchooser_la_SOURCES = \
  eggfileformatchooser.c

noinst_HEADERS = \
  eggfileformatchooser.h

noinst_PROGRAMS = testfileformatchooser

testfileformatchooser_SOURCES = testfileformatchooser.c
testfileformatchooser_LDFLAGS = libeggfileformatchooser.la $(EGG_30_LIBS)
testfileformatchooser_DEPENDENCIES = libeggfileformatchooser.la 

