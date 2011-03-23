
AM_CPPFLAGS = \
  $(EGG_CFLAGS) $(WARN_CFLAGS) $(DISABLE_DEPRECATED) \
  -DEGG_COMPILATION


noinst_LTLIBRARIES = libegg-background-monitor.la

libegg_background_monitor_la_LIBADD = $(EGG_LIBS)
libegg_background_monitor_la_SOURCES = \
  egg-background-monitor.c

# fixme: this should be installed
noinst_HEADERS = \
  egg-background-monitor.h

noinst_PROGRAMS = test-ebm

test_ebm_SOURCES = test-ebm.c
test_ebm_LDFLAGS = libegg-background-monitor.la $(EGG_LIBS)
test_ebm_DEPENDENCIES = libegg-background-monitor.la
