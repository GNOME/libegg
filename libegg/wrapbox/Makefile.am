AM_CPPFLAGS = \
	$(EGG_30_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED)

noinst_LTLIBRARIES = libegg-wrapbox.la

libegg_wrapbox_la_LIBADD = $(EGG_30_LIBS)

libegg_wrapbox_la_SOURCES = \
	eggwrapbox.h \
	eggwrapbox.c

noinst_PROGRAMS = testwrapbox

testwrapbox_SOURCES = testwrapbox.c

testwrapbox_LDADD = $(EGG_30_LIBS) \
	libegg-wrapbox.la
