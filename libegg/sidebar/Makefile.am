
INCLUDES = \
  $(EGG_CFLAGS) \
  `pkg-config --cflags libxml-2.0`	\
  -DEGG_COMPILATION \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED


noinst_LTLIBRARIES = libegg-sidebar.la

libegg_sidebar_la_LIBADD = $(EGG_LIBS)
libegg_sidebar_la_SOURCES = \
	eggsidebar.h		\
	eggsidebar.c		\
	eggsidebarbutton.h	\
	eggsidebarbutton.c

noinst_PROGRAMS = test-sidebar

test_sidebar_SOURCES = test-sidebar.c
test_sidebar_LDFLAGS = libegg-sidebar.la $(EGG_LIBS) `pkg-config --libs libxml-2.0`
test_sidebar_DEPENDENCIES = libegg-sidebar.la

