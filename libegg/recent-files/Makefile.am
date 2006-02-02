INCLUDES = \
	$(EGG_RECENT_CFLAGS)		\
	$(EGG_EXEC_CFLAGS)		\
	$(EGG_VFS_MODULE_CFLAGS)	\
	-DEGG_COMPILATION

noinst_LTLIBRARIES = libeggrecent.la

vfsdir = $(libdir)/gnome-vfs-2.0/modules
vfs_LTLIBRARIES = librecent-files.la

vfsconfdir = $(sysconfdir)/gnome-vfs-2.0/modules
vfsconf_DATA = egg-recent-files-vfs.conf

libeggrecent_la_LIBADD = $(EGG_RECENT_LIBS)

libeggrecent_la_SOURCES =		\
	egg-recent.h			\
	egg-recent-item.h		\
	egg-recent-item.c		\
	egg-recent-model.c		\
	egg-recent-model.h		\
	egg-recent-view.c		\
	egg-recent-view.h		\
	egg-recent-view-gtk.c		\
	egg-recent-view-gtk.h		\
	egg-recent-view-bonobo.c	\
	egg-recent-view-bonobo.h	\
	egg-recent-view-uimanager.c	\
	egg-recent-view-uimanager.h	\
	egg-recent-util.c		\
	egg-recent-util.h

noinst_PROGRAMS = bonobo_view_test gtk_view_test uimanager_view_test populate_recent

bonobo_view_test_SOURCES = bonobo-view-test.c
bonobo_view_test_LDFLAGS = libeggrecent.la $(EGG_RECENT_LIBS) $(EGG_EXEC_LIBS)
bonobo_view_test_DEPENDENCIES = libeggrecent.la

gtk_view_test_SOURCES = gtk-view-test.c
gtk_view_test_LDFLAGS = libeggrecent.la $(EGG_RECENT_LIBS) $(EGG_EXEC_LIBS)
gtk_view_test_DEPENDENCIES = libeggrecent.la

uimanager_view_test_SOURCES = uimanager-view-test.c
uimanager_view_test_LDFLAGS = libeggrecent.la $(EGG_RECENT_LIBS) $(EGG_EXEC_LIBS)
uimanager_view_test_DEPENDENCIES = libeggrecent.la

populate_recent_SOURCES = populate-recent.c
populate_recent_LDFLAGS = libeggrecent.la $(EGG_RECENT_LIBS) $(EGG_EXEC_LIBS)
populate_recent_DEPENDENCIES = libeggrecent.la

librecent_files_la_SOURCES = egg-recent-files-module.c
librecent_files_la_LDFLAGS = -export-dynamic -avoid-version -module libeggrecent.la $(EGG_RECENT_LIBS) $(EGG_EXEC_LIBS)
librecent_files_la_LIBADD = $(EGG_VFS_MODULE_LIBS)
librecent_files_la_DEPENDENCIES = libeggrecent.la
