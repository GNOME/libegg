AM_CPPFLAGS = \
	-I$(top_srcdir)/libegg/util \
	-I$(top_srcdir)/libegg/pixbufthumbnail \
	-DG_LOG_DOMAIN=\""EggThumbnailPreview"\" \
	$(EGG_30_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = \
	libeggthumbnailpreview.la

libeggthumbnailpreview_la_LIBADD = \
	$(EGG_30_LIBS) \
	$(top_builddir)/libegg/pixbufthumbnail/libeggpixbufthumbnail.la

libeggthumbnailpreview_la_SOURCES = \
	eggthumbnailpreview.c \
	eggthumbnailpreviewtypebuiltins.c

	
thumbnailpreview_headers = \
	$(srcdir)/eggthumbnailpreview.h

eggthumbnailpreviewtypebuiltins.c: $(thumbnailpreview_headers) eggthumbnailpreviewtypebuiltins.h
	@GLIB_MKENUMS@ \
			--fhead "#include <glib-object.h>\n" \
			--fhead "#include \"eggthumbnailpreviewtypebuiltins.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(thumbnailpreview_headers) > xgen-gtbec \
	&& cp xgen-gtbec $(@F) \
	&& rm -f xgen-gtbec

eggthumbnailpreviewtypebuiltins.h: $(thumbnailpreview_headers) Makefile.am
	@GLIB_MKENUMS@ \
			--fhead "#ifndef __EGG_THUMBNAIL_PREVIEW_TYPE_BUILTINS_H__\n" \
			--fhead "#define __EGG_THUMBNAIL_PREVIEW_TYPE_BUILTINS_H__ 1\n\n" \
			--fhead "#include \"eggthumbnailpreview.h\"\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __EGG_THUMBNAIL_PREVIEW_TYPE_BUILTINS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define EGG_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(thumbnailpreview_headers) > xgen-gth \
	&& cp xgen-gth $(@F) \
	&& rm -f xgen-gth

EXTRA_DIST = \
	README \
	eggthumbnailpreview.h

BUILT_SOURCES = \
	eggthumbnailpreviewtypebuiltins.c \
	eggthumbnailpreviewtypebuiltins.h

CLEANFILES = \
	eggthumbnailpreviewtypebuiltins.c \
	eggthumbnailpreviewtypebuiltins.h

MAINTAINERCLEANFILES = \
	Makefile.in
