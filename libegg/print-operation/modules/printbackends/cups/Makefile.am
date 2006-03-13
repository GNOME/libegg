INCLUDES = $(EGG_CFLAGS) $(CUPS_CFLAGS) $(EGGMODULES_CFLAGS) -DEGG_PRINT_BACKEND_ENABLE_UNSUPPORTED

LDADDS = $(top_builddir)/libeggprint.la \
	$(EGG_LIBS) 			\
	$(EGGMODULES_LIBS)		\
	$(CUPS_LIBS)


lib_LTLIBRARIES = libeggprintbackendcups.la

libeggprintbackendcups_la_SOURCES =	\
	eggprintbackendcups.c		\
	eggprintercups.c		\
	eggcupsutils.c

noinst_HEADERS =			\
	eggprintbackendcups.h		\
	eggprintercups.h		\
	eggcupsutils.h

libeggprintbackendcups_la_LDFLAGS = -avoid-version -module
libeggprintbackendcups_la_LIBADD = $(LDADDS)
