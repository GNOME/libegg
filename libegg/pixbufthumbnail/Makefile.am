# This file will be processed with automake-1.7 to create Makefile.in

INCLUDES = \
	$(EGG_CFLAGS)

noinst_LTLIBRARIES = \
	libeggpixbufthumbnail.la

libeggpixbufthumbnail_la_CFLAGS = \
	-DG_LOG_DOMAIN=\""EggPixbufThumbnail"\"

libeggpixbufthumbnail_la_LIBADD = \
	$(EGG_LIBS)

libeggpixbufthumbnail_la_SOURCES = \
	egg-pixbuf-thumbnail.c \
	egg-pixbuf-thumbnail-type-builtins.c


noinst_PROGRAMS = \
	testeggpixbufthumbnail

testeggpixbufthumbnail_CFLAGS = \
	-DGTK_FILE_SYSTEM_ENABLE_UNSUPPORTED \
	-DG_LOG_DOMAIN=\""TestEggPixbufThumbnail"\"

testeggpixbufthumbnail_LDADD = \
	$(EGG_LIBS) \
	$(top_builddir)/libegg/pixbufthumbnail/libeggpixbufthumbnail.la

testeggpixbufthumbnail_SOURCES = \
	testeggpixbufthumbnail.c
	
pixbufthumbnail_headers = \
	$(srcdir)/egg-pixbuf-thumbnail.h

egg-pixbuf-thumbnail-type-builtins.c: $(pixbufthumbnail_headers) egg-pixbuf-thumbnail-type-builtins.h
	@GLIB_MKENUMS@ \
			--fhead "#include <glib-object.h>\n" \
			--fhead "#include \"egg-pixbuf-thumbnail-type-builtins.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(pixbufthumbnail_headers) > xgen-gtbec \
	&& cp xgen-gtbec $(@F) \
	&& rm -f xgen-gtbec

egg-pixbuf-thumbnail-type-builtins.h: $(pixbufthumbnail_headers) Makefile.am
	@GLIB_MKENUMS@ \
			--fhead "#ifndef __EGG_PIXBUF_THUMBNAIL_TYPE_BUILTINS_H__\n" \
			--fhead "#define __EGG_PIXBUF_THUMBNAIL_TYPE_BUILTINS_H__ 1\n\n" \
			--fhead "#include \"egg-pixbuf-thumbnail.h\"\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __EGG_PIXBUF_THUMBNAIL_TYPE_BUILTINS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define EGG_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(pixbufthumbnail_headers) > xgen-gth \
	&& cp xgen-gth $(@F) \
	&& rm -f xgen-gth

EXTRA_DIST = \
	README \
	egg-pixbuf-thumbnail.h

BUILT_SOURCES = \
	egg-pixbuf-thumbnail-type-builtins.c \
	egg-pixbuf-thumbnail-type-builtins.h

CLEANFILES = \
	egg-pixbuf-thumbnail-type-builtins.c \
	egg-pixbuf-thumbnail-type-builtins.h

MAINTAINERCLEANFILES = \
	Makefile.in
