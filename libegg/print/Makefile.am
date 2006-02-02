INCLUDES = $(EGG_CFLAGS)
LIBS = $(EGG_LIBS)

noinst_LTLIBRARIES = libeggprint.la

libeggprint_la_SOURCES = 	\
	eggprintdialog.c	\
	eggprintjob.c		\
	eggprintsettings.c

noinst_HEADERS = 		\
	eggprintdialog.h	\
	eggprintjob.h		\
	eggprintsettings.h

noinst_PROGRAMS = testprint

testprint_SOURCES = testprint.c
testprint_LDADD = libeggprint.la
