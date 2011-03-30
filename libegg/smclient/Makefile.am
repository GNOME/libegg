if PLATFORM_WIN32
platform_sources = eggsmclient-win32.c
platform_logout_test_ldflags = -mwindows
else
if PLATFORM_OSX
platform_defines = -xobjective-c
platform_ldflags = -framework Carbon
platform_session_end_ldflags = -framework Carbon
platform_logout_test_ldflags = -framework Carbon
platform_sources = eggsmclient-osx.c
else
platform_defines = -DEGG_SM_CLIENT_BACKEND_XSMP
platform_ltlibraries =                   \
	libeggdesktopfile.la             \
	libeggsmclient-gnome.la
platform_libs = libeggdesktopfile.la -lSM -lICE
platform_sources = eggsmclient-xsmp.c
platform_programs = egg-launch
if HAVE_DBUS
platform_defines += -DEGG_SM_CLIENT_BACKEND_DBUS
platform_sources += eggsmclient-dbus.c
endif
endif
endif

AM_CPPFLAGS =                            \
	-DG_LOG_DOMAIN=\""EggSMClient"\" \
	$(platform_defines)              \
	$(EGG_SMCLIENT_CFLAGS)           \
	$(WARN_CFLAGS)                   \
	$(DISABLE_DEPRECATED)

noinst_LTLIBRARIES =                     \
	libeggsmclient.la                \
	$(platform_ltlibraries)

libeggsmclient_la_LIBADD =               \
	$(EGG_SMCLIENT_LIBS)             \
	$(EGG_SMCLIENT_DBUS_LIBS)        \
	$(platform_libs)

libeggsmclient_la_LDFLAGS =              \
	$(platform_ldflags)

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

libeggdesktopfile_la_LIBADD =            \
	$(EGG_LIBS)

libeggdesktopfile_la_SOURCES =           \
	eggdesktopfile.c                 \
	eggdesktopfile.h

noinst_PROGRAMS =                        \
	egg-session-end                  \
	logout-test                      \
	$(platform_programs)

egg_session_end_SOURCES =                \
	egg-session-end.c

egg_session_end_LDADD =                  \
	libeggsmclient.la

egg_session_end_LDFLAGS =                \
	$(platform_session_end_ldflags)

logout_test_SOURCES =                    \
	logout-test.c

logout_test_LDADD =                      \
	libeggsmclient.la

logout_test_LDFLAGS =                    \
	$(platform_logout_test_ldflags)

egg_launch_SOURCES =                     \
	egg-launch.c

egg_launch_LDADD =                       \
	libeggdesktopfile.la

EXTRA_DIST =                             \
	README                           \
	gedit.diff                       \
	eggsmclient-dbus.c               \
	eggsmclient-osx.c                \
	eggsmclient-win32.c              \
	eggsmclient-xsmp.c
