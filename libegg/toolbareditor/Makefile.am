AM_CPPFLAGS = \
  -DCURSOR_DIR=\".\" \
  $(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
  $(EGG_XML_CFLAGS) \
  -DEGG_COMPILATION \
  -I$(top_builddir)/libegg/util \
  -DGETTEXT_PACKAGE=\"libegg\"

noinst_LTLIBRARIES = libegg-toolbareditor.la

libegg_toolbareditor_la_LIBADD = $(EGG_LIBS) $(EGG_XML_LIBS) \
	$(top_builddir)/libegg/util/libeggutil.la

EGGSOURCES = \
	egg-editable-toolbar.c \
	egg-toolbars-model.c \
	egg-toolbar-editor.c

EGGHEADERS = \
	egg-editable-toolbar.h \
	egg-toolbars-model.h \
	egg-toolbar-editor.h

libegg_toolbareditor_la_SOURCES = $(EGGSOURCES)

BUILT_SOURCES = \
	eggmarshalers.c \
	eggmarshalers.h \
	eggtypebuiltins.c \
	eggtypebuiltins.h

nodist_libegg_toolbareditor_la_SOURCES = \
	$(BUILT_SOURCES)

noinst_HEADERS = $(EGG_HEADERS)

#Test program

noinst_PROGRAMS = test-toolbar-editor

test_toolbar_editor_SOURCES = test-toolbar-editor.c

test_toolbar_editor_LDADD = $(EGG_LIBS) \
  $(top_builddir)/libegg/toolbareditor/libegg-toolbareditor.la

# Buidl sources 

stamp_files = \
	stamp-eggmarshalers.c \
	stamp-eggmarshalers.h \
	stamp-eggtypebuiltins.c \
	stamp-eggtypebuiltins.h

eggmarshalers.h: stamp-eggmarshalers.h
	@true
stamp-eggmarshalers.h: eggmarshalers.list
	$(GLIB_GENMARSHAL) --prefix=_egg_marshal $(srcdir)/eggmarshalers.list --header > eggmarshalers.h \
	&& echo timestamp > $(@F)

eggmarshalers.c: stamp-eggmarshalers.c
	@true
stamp-eggmarshalers.c: eggmarshalers.list
	$(GLIB_GENMARSHAL) --prefix=_egg_marshal $(srcdir)/eggmarshalers.list --header --body > eggmarshalers.c \
	&& echo timestamp > $(@F)

eggtypebuiltins.c: stamp-eggtypebuiltins.c
	@true
stamp-eggtypebuiltins.c: $(EGGHEADERS)
	( cd $(srcdir) && $(GLIB_MKENUMS) \
		--fhead "#include \"eggtypebuiltins.h\"\n\n" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--fprod "\n#include \"@filename@\"" \
		--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
		--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "  { 0, NULL, NULL }\n};\n\n" \
		--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
		--vtail "  static GType type = 0;\n\n" \
		--vtail "  if (G_UNLIKELY (type == 0))\n" \
		--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
		--vtail "  return type;\n}\n\n" \
		$(^F) ) > xgen-$(@F) \
	&& ( cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%) ) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

eggtypebuiltins.h: stamp-eggtypebuiltins.h
	@true
stamp-eggtypebuiltins.h: $(EGGHEADERS)
	( cd $(srcdir) && $(GLIB_MKENUMS) \
		--fhead "#ifndef __EGGTYPEBUILTINS_H__\n" \
		--fhead "#define __EGGTYPEBUILTINS_H__ 1\n\n" \
		--fhead "#include <glib-object.h>\n\n" \
		--fhead "G_BEGIN_DECLS\n\n" \
		--ftail "G_END_DECLS\n\n" \
		--ftail "#endif /* __EGGTYPEBUILTINS_H__ */\n" \
		--fprod "\n/* --- @filename@ --- */" \
		--eprod "#define EGG_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
		--eprod "GType @enum_name@_get_type (void);\n" \
		$(^F) ) > xgen-$(@F) \
	&& ( cmp -s xgen-$(@F) $(@F:stamp-%=%) || cp xgen-$(@F) $(@F:stamp-%=%) ) \
	&& rm -f xgen-$(@F) \
	&& echo timestamp > $(@F)

EXTRA_DIST = \
	eggmarshalers.list

# End of built sources

