INCLUDES = $(EGG_CFLAGS) $(EGGMODULES_CFLAGS) -DEGG_PRINT_BACKEND_ENABLE_UNSUPPORTED

LDADDS = $(top_builddir)/libeggprint.la \
	$(EGG_LIBS) 			\
	$(EGGMODULES_LIBS)

lib_LTLIBRARIES = libeggprintbackendlpr.la

libeggprintbackendlpr_la_SOURCES =	\
	eggprintbackendlpr.c		\
	eggprinterlpr.c

noinst_HEADERS =			\
	eggprintbackendlpr.h		\
	eggprinterlpr.h

libeggprintbackendlpr_la_LDFLAGS = -avoid-version -module
libeggprintbackendlpr_la_LIBADD = $(LDADDS)
