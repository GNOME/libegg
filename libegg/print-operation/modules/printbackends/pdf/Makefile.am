INCLUDES = $(EGG_CFLAGS) $(EGGMODULES_CFLAGS) -DEGG_PRINT_BACKEND_ENABLE_UNSUPPORTED

LDADDS = $(top_builddir)/libeggprint.la \
	$(EGG_LIBS) 			\
	$(EGGMODULES_LIBS)


lib_LTLIBRARIES = libeggprintbackendpdf.la

libeggprintbackendpdf_la_SOURCES =	\
	eggprintbackendpdf.c		\
	eggprinterpdf.c

noinst_HEADERS =			\
	eggprintbackendpdf.h		\
	eggprinterpdf.h

libeggprintbackendpdf_la_LDFLAGS = -avoid-version -module
libeggprintbackendpdf_la_LIBADD = $(LDADDS)
