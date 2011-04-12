AC_INIT(libegg/util/eggmarshalers.list)

AC_CONFIG_MACRO_DIR(m4)

AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE(libegg, 0.0.1)

AM_MAINTAINER_MODE

AC_PROG_CC
AC_ISC_POSIX
AC_HEADER_STDC
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

AC_MSG_CHECKING([platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform="win32"
    NO_UNDEFINED=-no-undefined
    ;;
  *-apple-darwin*)
    platform="os x"
    ;;
  *)
    platform="unix"
    ;;
esac
AC_MSG_RESULT([$platform])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform" = "win32")
AM_CONDITIONAL(PLATFORM_OSX, test "$platform" = "os x")
AC_SUBST(NO_UNDEFINED)

changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac

  if test "x$enable_ansi" = "xyes"; then
    case " $CFLAGS " in
    *[\ \	]-ansi[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -ansi" ;;
    esac

    case " $CFLAGS " in
    *[\ \	]-pedantic[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -pedantic" ;;
    esac
  fi
fi
changequote([,])dnl

## Joke from Anders, but he didn't have the bad taste to commit to CVS ;-)
AC_MSG_CHECKING([for chicken])
AC_MSG_RESULT([must have egg first])

AC_PATH_XTRA

# Check return type of signal
AC_TYPE_SIGNAL

AC_CHECK_FUNCS(memmove strerror timegm)

## here we get the flags we'll actually use
PKG_CHECK_MODULES(EGG, gtk+-3.0 >= 3.0.0)
AC_SUBST(EGG_LIBS)
AC_SUBST(EGG_CFLAGS)

PKG_CHECK_MODULES(EGG_XML, libxml-2.0)
AC_SUBST(EGG_XML_LIBS)
AC_SUBST(EGG_XML_CFLAGS)

## TODO: Use GDBus from glib instead?
PKG_CHECK_EXISTS(dbus-glib-1, have_dbus=yes, have_dbus=no)
AS_IF([test "x$have_dbus" = "xyes"], [DBUS_DEPS="dbus-glib-1"])
AM_CONDITIONAL(HAVE_DBUS, test "$have_dbus" = "yes")

PKG_CHECK_MODULES(EGG_SMCLIENT, gtk+-3.0 $DBUS_DEPS)
AC_SUBST(EGG_SMCLIENT_LIBS)
AC_SUBST(EGG_SMCLIENT_CFLAGS)

## check if we have the X11 gtk+ backend
AM_CONDITIONAL(HAVE_GTK_TARGET_X11, test `$PKG_CONFIG --variable=target gtk+-3.0` = x11)

GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)

GLIB_MKENUMS=`$PKG_CONFIG --variable=glib_mkenums glib-2.0`
AC_SUBST(GLIB_MKENUMS)

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AC_ARG_WITH(html-dir, [  --with-html-dir=PATH path to installed docs ])
if test "x$with_html_dir" = "x" ; then
  HTML_DIR='${datadir}/gtk-doc/html'
else
  HTML_DIR=$with_html_dir
fi
AC_SUBST(HTML_DIR)
AM_CONDITIONAL(ENABLE_GTK_DOC, true)

GNOME_COMPILE_WARNINGS
GNOME_MAINTAINER_MODE_DEFINES

AC_OUTPUT([
Makefile
libegg/Makefile
libegg/util/Makefile
libegg/background-monitor/Makefile
libegg/treeviewutils/Makefile
libegg/column-chooser/Makefile
libegg/toolbareditor/Makefile
libegg/datetime/Makefile
libegg/pixbufthumbnail/Makefile
libegg/thumbnailpreview/Makefile
libegg/iconchooser/Makefile
libegg/smclient/Makefile
libegg/spreadtable/Makefile
libegg/wrapbox/Makefile
libegg/fileformatchooser/Makefile
doc/Makefile
])

echo "

libegg is not a regular library, it's a collection of various pieces of code
that are or were aspiring to become included in one of the platform libraries.
Whenever a part of libegg is accepted into a platform library, it's removed
from libegg and a README file is left explaining where it moved.

Note #1: It's not advised to build libegg by using "make" from the top
directory. Instead you should run "make" inside the specific subdirectory that
holds the feature you need to build and or modify. Each subdirectory typically
contains a small test program as well.

Note #2: Some GNOME applications have a source copy of libegg within their own
source trees. If you fix a bug in such a copy of libegg, please make sure you
also commit that bugfix in the upstream libegg module.

"
