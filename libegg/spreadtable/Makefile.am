AM_CPPFLAGS = \
  $(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
  -DEGG_COMPILATION \
  -DGETTEXT_PACKAGE=\"libegg\"

noinst_LTLIBRARIES = libegg-spreadtable.la

libegg_spreadtable_la_LIBADD = $(EGG_LIBS)

BUILT_SOURCES = eggmarshalers.c eggmarshalers.h

EGGSOURCES = \
	eggspreadtable.c \
	eggspreadtablednd.c \
	eggplaceholder.c \
	eggmarshalers.c


EGGHEADERS = \
	eggspreadtable.h \
	eggspreadtablednd.h \
	eggplaceholder.h \
	eggmarshalers.h

libegg_spreadtable_la_SOURCES = $(EGGSOURCES)

noinst_HEADERS = $(EGG_HEADERS)

#Test program

noinst_PROGRAMS = testspreadtable testspreadtablednd

testspreadtable_SOURCES = testspreadtable.c
testspreadtable_LDADD = $(EGG_LIBS) \
  $(top_builddir)/libegg/spreadtable/libegg-spreadtable.la

testspreadtablednd_SOURCES = testspreadtablednd.c
testspreadtablednd_LDADD = $(EGG_LIBS) \
  $(top_builddir)/libegg/spreadtable/libegg-spreadtable.la


#Marshallers rules
eggmarshalers.h: eggmarshalers.list
	$(GLIB_GENMARSHAL) --prefix=_egg_marshal $(srcdir)/eggmarshalers.list --header > eggmarshalers-h.tmp \
	&& mv eggmarshalers-h.tmp eggmarshalers.h \
	|| ( rm -f eggmarshalers-h.tmp && exit 1)
eggmarshalers.c: eggmarshalers.list
	$(GLIB_GENMARSHAL) --prefix=_egg_marshal $(srcdir)/eggmarshalers.list --body --header > eggmarshalers-c.tmp     \
	&& mv eggmarshalers-c.tmp eggmarshalers.c \
	|| ( rm -f eggmarshalers-c.tmp && exit 1 )
