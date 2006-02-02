NULL =

SUBDIRS = xdgmime

INCLUDES = \
	-DG_LOG_DOMAIN="\"EggRecentChooser\""	\
	-I$(top_srcdir)/libegg/util		\
	-I$(top_srcdir)/libegg/bookmarkfile	\
	$(NULL)

noinst_LTLIBRARIES = libeggrecentchooser.la

libeggrecentchooser_la_CFLAGS = $(EGG_CFLAGS)
libeggrecentchooser_la_LIBADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/bookmarkfile/libeggbookmarkfile.la \
	$(top_builddir)/libegg/recentchooser/xdgmime/libxdgmime.la \
	$(NULL)

libeggrecentchooser_la_SOURCES = \
	fnmatch.c			\
	eggrecenttypebuiltins.c		\
	eggrecentmanager.c		\
	eggrecentfilter.c		\
	eggrecentchooser.c		\
	eggrecentchooserutils.c		\
	eggrecentchoosermenu.c		\
	eggrecentchooserdefault.c	\
	eggrecentchooserwidget.c	\
	eggrecentchooserdialog.c	\
	$(NULL)

noinst_HEADERS = \
	eggrecentmanager.h		\
	eggrecentfilter.h		\
	eggrecentchooser.h		\
	eggrecentchooserutils.h		\
	eggrecentchoosermenu.h		\
	eggrecentchooserdefault.h	\
	eggrecentchooserwidget.h	\
	eggrecentchooserdialog.h	\
	eggrecentchooserprivate.h	\
	eggrecenttypebuiltins.h		\
	eggrecent.h			\
	$(NULL)

noinst_PROGRAMS = \
	populate-recent \
	test-recent-menu \
	test-recent-widget \
	$(NULL)

populate_recent_SOURCES = populate-recent.c
populate_recent_CFLAGS = $(EGG_CFLAGS)
populate_recent_LDFLAGS = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/recentchooser/libeggrecentchooser.la \
	$(NULL)
populate_recent_DEPENDENCIES = \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/bookmarkfile/libeggbookmarkfile.la \
	$(top_builddir)/libegg/recentchooser/libeggrecentchooser.la \
	$(NULL)

test_recent_menu_SOURCES = test-recent-menu.c
test_recent_menu_CFLAGS = $(EGG_EXEC_CFLAGS)
test_recent_menu_LDFLAGS = \
	$(EGG_EXEC_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/recentchooser/libeggrecentchooser.la \
	$(NULL)
test_recent_menu_DEPENDENCIES = \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/bookmarkfile/libeggbookmarkfile.la \
	$(top_builddir)/libegg/recentchooser/libeggrecentchooser.la \
	$(NULL)

test_recent_widget_SOURCES = test-recent-widget.c
test_recent_widget_CFLAGS = $(EGG_EXEC_CFLAGS)
test_recent_widget_LDFLAGS = \
	$(EGG_EXEC_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/recentchooser/libeggrecentchooser.la \
	$(NULL)
test_recent_widget_DEPENDENCIES = \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/bookmarkfile/libeggbookmarkfile.la \
	$(top_builddir)/libegg/recentchooser/libeggrecentchooser.la \
	$(NULL)

recent_headers = \
	$(srcdir)/eggrecentmanager.h \
	$(srcdir)/eggrecentfilter.h \
	$(srcdir)/eggrecentchooser.h \
	$(srcdir)/eggrecentchooserprivate.h \
	$(srcdir)/eggrecentchoosermenu.h \
	$(srcdir)/eggrecentchooserdefault.h \
	$(srcdir)/eggrecentchooserwidget.h \
	$(srcdir)/eggrecentchooserdialog.h \
	$(NULL)

eggrecenttypebuiltins.c: $(recent_headers) eggrecenttypebuiltins.h
	@GLIB_MKENUMS@ \
		--fhead "#include <glib-object.h>\n" \
		--fhead "#include \"eggrecenttypebuiltins.h\"\n\n" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
		--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "  { 0, NULL, NULL }\n};\n\n" \
		--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
		--vtail "  static GType type = 0;\n\n" \
		--vtail "  if (!type)\n" \
		--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
		--vtail "  return type;\n}\n\n" \
	    $(recent_headers) > xgen-gtbec \
	&& cp xgen-gtbec $(@F) \
	&& rm -f xgen-gtbec

eggrecenttypebuiltins.h: $(recent_headers) Makefile.am
	@GLIB_MKENUMS@ \
		--fhead "#ifndef __EGG_RECENT_CHOOSER_TYPE_BUILTINS_H__\n" \
		--fhead "#define __EGG_RECENT_CHOOSER_TYPE_BUILTINS_H__ 1\n\n" \
		--fhead "#include \"eggrecent.h\"\n\n" \
		--fhead "G_BEGIN_DECLS\n\n" \
		--ftail "G_END_DECLS\n\n" \
		--ftail "#endif /* __EGG_RECENT_CHOOSER_TYPE_BUILTINS_H__ */\n" \
		--fprod "\n/* --- @filename@ --- */" \
		--eprod "#define EGG_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
		--eprod "GType @enum_name@_get_type (void);\n" \
	    $(recent_headers) > xgen-gth \
	&& cp xgen-gth $(@F) \
	&& rm -f xgen-gth

BUILT_SOURCES = \
	eggrecenttypebuiltins.h \
	eggrecenttypebuiltins.c \
	$(NULL)

CLEAN_FILES = \
	eggrecenttypebuiltins.h \
	eggrecenttypebuiltins.c \
	$(NULL)

EXTRA_DIST = README TODO recently-used.xbel test-recent-menu.glade
