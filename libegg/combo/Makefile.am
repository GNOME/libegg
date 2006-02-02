INCLUDES = \
  $(EGG_CFLAGS) \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED \
  -I$(top_builddir)/libegg/util \
  -I$(top_builddir)/libegg/treeviewutils

noinst_LTLIBRARIES = libegg-combo.la

libegg_combo_la_LIBADD = \
	$(EGG_LIBS)					\
	$(top_builddir)/libegg/util/libeggutil.la	\
	$(top_builddir)/libegg/treeviewutils/libegg-treeview.la

libegg_combo_la_SOURCES = \
	eggcellview.c		\
	eggcellviewmenuitem.c	\
	eggcombobox.c		\
	eggcomboboxentry.c

noinst_PROGRAMS = test-combo

test_combo_SOURCES = test.c
test_combo_LDFLAGS = libegg-combo.la $(EGG_LIBS)
test_combo_DEPENDENCIES = libegg-combo.la
