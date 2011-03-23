# This file will be processed with automake-1.7 to create Makefile.in


AM_CPPFLAGS = \
	-DG_LOG_DOMAIN="\"EggIconChooser\"" \
	-I$(top_srcdir)/libegg/util \
	-I$(top_srcdir)/libegg/pixbufthumbnail \
	-I$(top_srcdir)/libegg/thumbnailpreview \
	$(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED)


noinst_LTLIBRARIES = \
	libeggiconchooser.la

libeggiconchooser_la_LIBADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/pixbufthumbnail/libeggpixbufthumbnail.la \
	$(top_builddir)/libegg/thumbnailpreview/libeggthumbnailpreview.la

libeggiconchooser_la_SOURCES =		\
	gtkfilechooserprivate.c		\
	gtkfilechooserembed.c		\
	eggiconchooser.c		\
	eggiconchooserbutton.c		\
	eggiconchooserdefault.c		\
	eggiconchooserdialog.c		\
	eggiconchooserutils.c		\
	eggiconchooserwidget.c		\
	eggiconchoosertypebuiltins.c

noinst_HEADERS =			\
	gtkfilechooserprivate.h		\
	gtkfilechooserembed.h		\
	eggiconchooser.h		\
	eggiconchooserbutton.h		\
	eggiconchooserdefault.h		\
	eggiconchooserdialog.h		\
	eggiconchooserprivate.h		\
	eggiconchooserutils.h		\
	eggiconchooserwidget.h		\
	prop-editor.h


noinst_PROGRAMS = testiconchooser

testiconchooser_SOURCES =	\
	testiconchooser.c	\
	prop-editor.c

testiconchooser_CFLAGS = \
	$(EGG_CFLAGS) $(WARN_CFLAGS) \
	-DG_LOG_DOMAIN="\"TestIconChooser\""

testiconchooser_LDFLAGS = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/iconchooser/libeggiconchooser.la

testiconchooser_DEPENDENCIES = \
	$(top_builddir)/libegg/util/libeggutil.la \
	$(top_builddir)/libegg/pixbufthumbnail/libeggpixbufthumbnail.la \
	$(top_builddir)/libegg/thumbnailpreview/libeggthumbnailpreview.la \
	$(top_builddir)/libegg/iconchooser/libeggiconchooser.la

	
iconchooser_headers = \
	$(srcdir)/eggiconchooser.h		\
	$(srcdir)/eggiconchooserdialog.h	\
	$(srcdir)/eggiconchooserwidget.h

eggiconchoosertypebuiltins.c: $(thumbnailpreview_headers) eggiconchoosertypebuiltins.h
	@GLIB_MKENUMS@ \
			--fhead "#include <glib-object.h>\n" \
			--fhead "#include \"eggiconchoosertypebuiltins.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(iconchooser_headers) > xgen-gtbec \
	&& cp xgen-gtbec $(@F) \
	&& rm -f xgen-gtbec

eggiconchoosertypebuiltins.h: $(iconchooser_headers) Makefile.am
	@GLIB_MKENUMS@ \
			--fhead "#ifndef __EGG_ICON_CHOOSER_TYPE_BUILTINS_H__\n" \
			--fhead "#define __EGG_ICON_CHOOSER_TYPE_BUILTINS_H__ 1\n\n" \
			--fhead "#include \"eggiconchooser.h\"\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __EGG_ICON_CHOOSER_TYPE_BUILTINS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define EGG_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(iconchooser_headers) > xgen-gth \
	&& cp xgen-gth $(@F) \
	&& rm -f xgen-gth

BUILT_SOURCES = \
	eggiconchoosertypebuiltins.c \
	eggiconchoosertypebuiltins.h

CLEANFILES = \
	eggiconchoosertypebuiltins.c \
	eggiconchoosertypebuiltins.h

EXTRA_DIST = \
	README

MAINTAINERCLEANFILES = \
	Makefile.in
