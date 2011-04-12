AM_CPPFLAGS = \
  $(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
  -DEGG_COMPILATION \
  -I$(top_builddir)/libegg/util

noinst_LTLIBRARIES = libegg-treeview.la

libegg_treeview_la_LIBADD = $(EGG_LIBS)
libegg_treeview_la_SOURCES = \
  eggaccelerators.c	\
  eggcellrendererkeys.c \
  eggcellrendererpopup.c \
  egg-cell-renderer-text.c \
  eggtreemodelunion.c \
  eggtreemultidnd.c \
  eggtreeviewstate.c

noinst_HEADERS = \
  eggaccelerators.h	\
  eggcellrendererkeys.h \
  eggcellrendererpopup.h \
  egg-cell-renderer-text.h \
  eggtreemultidnd.h \
  eggtreeviewstate.h


noinst_PROGRAMS = test-tree-utils test-multi-drag test-union

test_tree_utils_SOURCES = test-tree-utils.c
test_tree_utils_LDADD = libegg-treeview.la $(EGG_LIBS)
test_tree_utils_DEPENDENCIES = libegg-treeview.la

test_multi_drag_SOURCES = test-multi-drag.c
test_multi_drag_LDADD = libegg-treeview.la $(EGG_LIBS)
test_multi_drag_DEPENDENCIES = libegg-treeview.la

test_union_SOURCES = test-union.c
test_union_LDADD = libegg-treeview.la $(EGG_LIBS)
test_union_DEPENDENCIES = libegg-treeview.la
