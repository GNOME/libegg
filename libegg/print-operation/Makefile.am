INCLUDES = $(EGG_CFLAGS)
LIBS = $(EGG_LIBS)

noinst_LTLIBRARIES = libeggprint.la

libeggprint_la_SOURCES = 		\
	eggpagesetup.c			\
	eggpapersize.c			\
	eggprintcontext.c		\
	eggprintersettings.c		\
	eggprintmarshal.c		\
	eggprintoperation.c

if PLATFORM_WIN32

LIBS += -lcomdlg32

libeggprint_la_SOURCES +=		\
	eggprint-win32.c		\
	eggprintoperation-win32.c

else

libeggprint_la_SOURCES +=		\
	eggprintoperation-dummy.c

endif

noinst_HEADERS = 			\
	eggpagesetup.h			\
	eggpapersize.h			\
	eggprint-win32.h		\
	eggprint.h			\
	eggprintcontext-private.h 	\
	eggprintcontext.h		\
	eggprintenums.h			\
	eggprintersettings.h		\
	eggprintmarshal.h		\
	eggprintoperation-private.h 	\
	eggprintoperation.h

marshal_sources = 		\
	eggprintmarshal.h	\
	eggprintmarshal.c

eggprintmarshal.h: eggprintmarshal.list
	$(GLIB_GENMARSHAL) $< --header --prefix=_egg_marshal > $@
eggprintmarshal.c: eggprintmarshal.list
	$(GLIB_GENMARSHAL) $< --body --prefix=_egg_marshal > $@

noinst_PROGRAMS = testprint

testprint_SOURCES = testprint.c testprintfileoperation.c testprintfileoperation.h
testprint_LDADD = libeggprint.la

EXTRA_DIST =				\
	eggprint-win32.c		\
	eggprintmarshal.list		\
	eggprintoperation-dummy.c	\
	eggprintoperation-win32.c	\
	paper_names.c
