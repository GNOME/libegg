INCLUDES=	\
	$(EGG_CFLAGS) 			\
	-DEGG_COMPILATION		\
	-DGTK_DISABLE_DEPRECATED	\
	-DGDK_DISABLE_DEPRECATED 	\
	-DG_DISABLE_DEPRECATED

noinst_LTLIBRARIES = libegg.la

libegg_la_SOURCES = \
	eggcellrendererkeys.c		\
	eggcellrendererpixbuf.c		\
	eggtreeviewstate.c

noinst_PROGRAMS=test-egg

test_egg_SOURCES = test-egg.c

test_egg_LDADD = 	\
	$(EGG_LIBS)			\
	$(top_builddir)/libegg/libegg.la
