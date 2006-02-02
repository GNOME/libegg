
INCLUDES = \
  $(EGG_CFLAGS) \
  $(EGG_VFS_FS_CFLAGS) \
  -I$(top_srcdir)/libegg/util \
  -DEGG_COMPILATION \
  -DGTK_DISABLE_DEPRECATED \
  -DGDK_DISABLE_DEPRECATED \
  -DG_DISABLE_DEPRECATED


noinst_LTLIBRARIES = libeggfilesystem.la

libeggfilesystem_la_LIBADD = \
	$(EGG_LIBS) $(EGG_VFS_FS_LIBS)\
	$(top_builddir)/libegg/util/libeggutil.la

libeggfilesystem_la_SOURCES =	\
	eggfilefilter.c		\
	eggfilesystem.c		\
	eggfilesystem-unix.c	\
	eggfilesystem-vfs.c	\
	eggfilesystemutil.c	

noinst_HEADERS =		\
	eggfilefilter.h		\
	eggfilesystem.h		\
	eggfilesystem-unix.h	\
	eggfilesystem-vfs.h	\
	eggfilesystemutil.h	

noinst_PROGRAMS = testfilesystem

testfilesystem_SOURCES = testfilesystem.c
testfilesystem_LDFLAGS = libeggfilesystem.la $(EGG_LIBS) $(EGG_VFS_FS_LIBS)
testfilesystem_DEPENDENCIES = libeggfilesystem.la 
