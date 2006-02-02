
INCLUDES = \
  -DCURSOR_DIR=\".\" \
  $(EGG_CFLAGS) \
  $(EGG_XML_CFLAGS) \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED \
  -I$(top_builddir)/libegg/util

noinst_LTLIBRARIES = libegg-toolbareditor.la

libegg_toolbareditor_la_LIBADD = $(EGG_LIBS) $(EGG_XML_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la

libegg_toolbareditor_la_SOURCES = \
  egg-toolbars-model.c		\
  egg-editable-toolbar.c	\
  egg-toolbar-editor.c

noinst_HEADERS = \
  egg-toolbars-model.h		\
  egg-editable-toolbar.h	\
  egg-toolbar-editor.h

noinst_PROGRAMS = test-toolbar-editor

test_toolbar_editor_SOURCES = test-toolbar-editor.c

test_toolbar_editor_LDADD = $(EGG_LIBS) \
  $(top_builddir)/libegg/toolbareditor/libegg-toolbareditor.la
