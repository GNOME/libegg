INCLUDES =                               \
	-DG_LOG_DOMAIN=\""EggSMClient"\" \
	-DEGG_SM_CLIENT_BACKEND_XSMP     \
	$(EGG_LIBGNOMEUI_CFLAGS)         \
	$(EGG_LAUNCHER_CFLAGS)

noinst_LTLIBRARIES =                     \
	libeggsmclient.la                \
	libeggsmclient-gnome.la          \
	libegglauncher.la

libeggsmclient_la_LIBADD =               \
	$(EGG_LIBS)                      \
	libegglauncher.la                \
	-lSM -lICE

libeggsmclient_la_SOURCES =              \
	eggsmclient.c                    \
	eggsmclient.h                    \
	eggsmclient-private.h            \
	eggsmclient-xsmp.c               \
	eggsmclient-xsmp.h

libeggsmclient_gnome_la_LIBADD =         \
	$(EGG_LIBGNOMEUI_LIBS)           \
	libegglauncher.la                \
	-lSM -lICE

libeggsmclient_gnome_la_SOURCES =        \
	eggsmclient.c                    \
	eggsmclient.h                    \
	eggsmclient-libgnomeui.c         \
	eggsmclient-libgnomeui.h         \
	eggsmclient-private.h            \
	eggsmclient-xsmp.c               \
	eggsmclient-xsmp.h

libegglauncher_la_LIBADD =               \
	$(EGG_LAUNCHER_LIBS)

libegglauncher_la_SOURCES =              \
	egglauncher.c                    \
	egglauncher.h

noinst_PROGRAMS =                        \
	egg-session-end                  \
	logout-test                      \
	egg-launch

egg_session_end_SOURCES =                \
	egg-session-end.c

egg_session_end_LDADD =                  \
	libeggsmclient.la

logout_test_SOURCES =                    \
	logout-test.c

logout_test_LDADD =                      \
	libeggsmclient.la

egg_launch_SOURCES =                     \
	egg-launch.c

egg_launch_LDADD =                       \
	libegglauncher.la

EXTRA_DIST =                             \
	README                           \
	gedit.diff                       \
	eggsmclient-osx.c                \
	eggsmclient-win32.c
