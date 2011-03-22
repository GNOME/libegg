INCLUDES = \
	$(EGG_CFLAGS) $(EGG_WARNING_FLAGS) \
	$(EGG_DOCK_CFLAGS) \
	$(EGG_XML_CFLAGS) \
	-DG_LOG_DOMAIN=\"Egg\" \
	-DEGG_GLADEDIR=\".\" \
	-DGETTEXT_PACKAGE=NULL \
	-I$(top_srcdir)/libegg

lib_LTLIBRARIES = libegg-dock.la

BUILT_SOURCES = \
	libeggdocktypebuiltins.c \
	libeggdocktypebuiltins.h

CLEANFILES = \
	libeggdocktypebuiltins.c \
	libeggdocktypebuiltins.h

libeggdock_headers = \
	egg-dock-object.h \
	egg-dock-master.h \
	egg-dock.h \
	egg-dock-item.h \
	egg-dock-item-grip.h \
	egg-dock-notebook.h \
	egg-dock-paned.h \
	egg-dock-tablabel.h \
	egg-dock-layout.h \
	egg-dock-placeholder.h

libegg_dock_la_SOURCES = \
	$(libeggdock_headers) \
	egg-dock-object.c \
	egg-dock-master.c \
	egg-dock.c \
	egg-dock-item.c \
	egg-dock-item-grip.c \
	egg-dock-notebook.c \
	egg-dock-paned.c \
	egg-dock-tablabel.c \
	egg-dock-layout.c \
	egg-dock-placeholder.c \
	libeggdocktypebuiltins.h \
	libeggdocktypebuiltins.c

libegg_dock_la_LDFLAGS = $(NO_UNDEFINED)

libegg_dock_la_LIBADD = \
	$(EGG_LIBS) \
	$(EGG_DOCK_LIBS) \
	$(EGG_XML_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la

noinst_PROGRAMS=test-dock

test_dock_sources=test-dock.c
test_dock_LDADD=$(srcdir)/libegg-dock.la $(EGG_LIBS)

# enumerations code generation

libeggdocktypebuiltins.c: $(libeggdock_headers)
	@GLIB_MKENUMS@ \
			--fhead "#include <glib-object.h>\n" \
			--fhead "#include \"libeggdocktypebuiltins.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(libeggdock_headers) > xgen-gtbec \
	&& cp xgen-gtbec $(@F) \
	&& rm -f xgen-gtbec

libeggdocktypebuiltins.h: $(libeggdock_headers)
	@GLIB_MKENUMS@ \
			--fhead "#ifndef __LIBEGGDOCKTYPEBUILTINS_H__\n" \
			--fhead "#define __LIBEGGDOCKTYPEBUILTINS_H__ 1\n\n" \
			--fhead "#include <dock/egg-dock.h>\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __LIBEGGDOCKTYPEBUILTINS_H__ */\n" \
			--fprod "\n/* --- @filename@ --- */" \
			--eprod "#define EGG_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(libeggdock_headers) > xgen-gth \
	&& cp xgen-gth $(@F) \
	&& rm -f xgen-gth

