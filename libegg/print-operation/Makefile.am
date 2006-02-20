INCLUDES = $(EGG_CFLAGS) $(CUPS_CFLAGS) -DEGG_PRINT_BACKEND_ENABLE_UNSUPPORTED
LIBS = $(EGG_LIBS) $(CUPS_LIBS)

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
	eggprintbackendsetting.c	\
	eggprintbackendsettingset.c	\
	eggprinter.c			\
	eggprintoperation-unix.c	\
	eggprintsettingwidget.c		\
	eggprintunixdialog.c		\
        eggprintbackend.c		\
        eggprintbackendcups.c		\
	eggprintercups.c		\
	eggprintjob.c

endif

noinst_HEADERS = 			\
	eggpagesetup.h			\
	eggpapersize.h			\
	eggprint-win32.h		\
	eggprint.h			\
	eggprintbackend.h		\
	eggprintbackendcups.h		\
	eggprintbackendsetting.h	\
	eggprintbackendsettingset.h	\
	eggprintcontext-private.h 	\
	eggprintcontext.h		\
	eggprintenums.h			\
	eggprinter-private.h		\
	eggprinter.h			\
	eggprintercups.h		\
	eggprintersettings.h		\
	eggprintmarshal.h		\
	eggprintoperation-private.h 	\
	eggprintoperation.h		\
	eggprintsettingwidget.h		\
	eggprintunixdialog.h		\
	eggprintercups-private.h	\
	eggprintjob.h			\
	eggprintjob-private.h

marshal_sources = 		\
	eggprintmarshal.h	\
	eggprintmarshal.c

eggprintmarshal.h: eggprintmarshal.list
	$(GLIB_GENMARSHAL) $< --header --prefix=_egg_marshal > $@
eggprintmarshal.c: eggprintmarshal.list
	$(GLIB_GENMARSHAL) $< --body --prefix=_egg_marshal > $@

noinst_PROGRAMS = testprint printedit

if !PLATFORM_WIN32
noinst_PROGRAMS += testshowdialog
endif 

testprint_SOURCES = testprint.c testprintfileoperation.c testprintfileoperation.h
testprint_LDADD = libeggprint.la

testshowdialog_SOURCES = testshowdialog.c
testshowdialog_LDADD = libeggprint.la

printedit_SOURCES = printedit.c
printedit_LDADD = libeggprint.la

EXTRA_DIST =				\
	eggprint-win32.c		\
	eggprintmarshal.list		\
	eggprintoperation-dummy.c	\
	eggprintoperation-win32.c	\
	paper_names.c
