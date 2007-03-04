if PLATFORM_WIN32
platform_defines =
platform_ltlibraries =
platform_libs =
platform_sources = eggsmclient-win32.c
platform_app_ldflags = -mwindows
platform_programs =
else
platform_defines = -DEGG_SM_CLIENT_BACKEND_XSMP
platform_ltlibraries =                   \
	libegglauncher.la                \
	libeggsmclient-gnome.la
platform_libs = libegglauncher.la -lSM -lICE
platform_sources = eggsmclient-xsmp.c
platform_app_ldflags =
platform_programs = egg-launch
endif

INCLUDES =                               \
	-DG_LOG_DOMAIN=\""EggSMClient"\" \
	$(platform_defines)              \
	$(EGG_SMCLIENT_CFLAGS)           \
	$(EGG_LIBGNOMEUI_CFLAGS)         \
	$(EGG_LAUNCHER_CFLAGS)

noinst_LTLIBRARIES =                     \
	libeggsmclient.la                \
	$(platform_ltlibraries)

libeggsmclient_la_LIBADD =               \
	$(EGG_SMCLIENT_LIBS)             \
	$(platform_libs)

libeggsmclient_la_SOURCES =              \
	eggsmclient.c                    \
	eggsmclient.h                    \
	eggsmclient-private.h            \
	$(platform_sources)

libeggsmclient_gnome_la_LIBADD =         \
	$(EGG_LIBGNOMEUI_LIBS)           \
	$(libeggsmclient_la_LIBADD)

libeggsmclient_gnome_la_SOURCES =        \
	eggsmclient.c                    \
	eggsmclient.h                    \
	eggsmclient-libgnomeui.c         \
	eggsmclient-libgnomeui.h         \
	eggsmclient-private.h            \
	$(platform_sources)

libegglauncher_la_LIBADD =               \
	$(EGG_LAUNCHER_LIBS)

libegglauncher_la_SOURCES =              \
	egglauncher.c                    \
	egglauncher.h

noinst_PROGRAMS =                        \
	egg-session-end                  \
	logout-test                      \
	$(platform_programs)

egg_session_end_SOURCES =                \
	egg-session-end.c

egg_session_end_LDADD =                  \
	libeggsmclient.la

logout_test_SOURCES =                    \
	logout-test.c

logout_test_LDADD =                      \
	libeggsmclient.la

logout_test_LDFLAGS =                    \
	$(platform_app_ldflags)

egg_launch_SOURCES =                     \
	egg-launch.c

egg_launch_LDADD =                       \
	libegglauncher.la

EXTRA_DIST =                             \
	README                           \
	gedit.diff                       \
	eggsmclient-osx.c                \
	eggsmclient-win32.c              \
	eggsmclient-xsmp.c
