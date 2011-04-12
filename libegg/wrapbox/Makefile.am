AM_CPPFLAGS = \
	$(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libegg-wrapbox.la

libegg_wrapbox_la_LIBADD = $(EGG_LIBS)

libegg_wrapbox_la_SOURCES = \
	$(enum_data) \
	$(eggwrapbox_header) \
	eggwrapbox.c

eggwrapbox_header = eggwrapbox.h

enum_data = \
	eggwrapbox-enums.c \
	eggwrapbox-enums.h

eggwrapbox-enums.c: $(eggwrapbox_header) eggwrapbox-enums.h
	$(AM_V_GEN)$(GLIB_MKENUMS) \
		--fhead "#include \"eggwrapbox-enums.h\"\n\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--fprod "#include \"@filename@\"\n" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n" \
		--vhead "  static GType type = 0;\n\n" \
		--vhead "  if (!type)\n  {\n" \
		--vhead "    static const G@Type@Value _@enum_name@_values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n\n" \
		--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n  }\n\n" \
		--vtail "  return type;\n}\n\n" \
		$(eggwrapbox_header) > $@

eggwrapbox-enums.h: $(eggwrapbox_header)
	$(AM_V_GEN)$(GLIB_MKENUMS) \
		--fhead "#ifndef EGG_WRAPBOX_ENUMS_H_\n" \
		--fhead "#define EGG_WRAPBOX_ENUMS_H_\n\n" \
		--fhead "#include <glib-object.h>\n\n" \
		--fhead "G_BEGIN_DECLS\n\n" \
		--fprod "/* enumerations from @filename@ */\n" \
		--vhead "GType @enum_name@_get_type (void) G_GNUC_CONST;\n" \
		--vhead "#define EGG_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n" \
		--ftail "#endif /* EGG_WRAPBOX_ENUMS_H_ */" \
		$(eggwrapbox_header) > $@


noinst_PROGRAMS = testwrapbox

testwrapbox_SOURCES = testwrapbox.c

testwrapbox_LDADD = $(EGG_LIBS) \
	libegg-wrapbox.la

CLEANFILES = $(enum_data)
