INCLUDES =                               \
	-DG_LOG_DOMAIN=\""EggSMClient"\" \
	-DHAVE_XSMP                      \
	$(EGG_LIBGNOMEUI_CFLAGS)

noinst_LTLIBRARIES =                     \
	libeggsmclient.la                \
	libeggsmclient-gnome.la

libeggsmclient_la_LIBADD =               \
	$(EGG_LIBS)                      \
	-lSM -lICE

libeggsmclient_la_SOURCES =              \
	eggdesktopfile.c                 \
	eggdesktopfile.h                 \
	eggsmclient.c                    \
	eggsmclient.h                    \
	eggsmclient-private.h            \
	eggsmclient-xsmp.c               \
	eggsmclient-xsmp.h

libeggsmclient_gnome_la_LIBADD =         \
	$(EGG_LIBGNOMEUI_LIBS)           \
	-lSM -lICE

libeggsmclient_gnome_la_SOURCES =        \
	eggdesktopfile.c                 \
	eggdesktopfile.h                 \
	eggsmclient.c                    \
	eggsmclient.h                    \
	eggsmclient-libgnomeui.c         \
	eggsmclient-libgnomeui.h         \
	eggsmclient-private.h            \
	eggsmclient-xsmp.c               \
	eggsmclient-xsmp.h

noinst_PROGRAMS =                        \
	egg-session-end

egg_session_end_SOURCES =                \
	egg-session-end.c

egg_session_end_LDADD =                  \
	libeggsmclient.la

EXTRA_DIST =                             \
	README                           \
	gedit.diff                       \
	eggsmclient-osx.c                \
	eggsmclient-win32.c
