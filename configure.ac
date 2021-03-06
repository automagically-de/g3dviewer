AC_INIT(configure.ac)
AC_CONFIG_MACRO_DIR([m4])

VERSION=0.2.99.6
PACKAGE=g3dviewer

AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
AC_SUBST(VERSION)

LT_INIT
AC_PROG_LIBTOOL

AC_PROG_CC
AM_PROG_CC_C_O

AC_DISABLE_STATIC
AC_IS_WIN32

ALL_LINGUAS="de"
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)

# glib 2.0
AM_PATH_GLIB_2_0(2.6.0,,AC_MSG_ERROR([GLib >= 2.6.0 is required]), gobject)

# gtk+ 2.0
AM_PATH_GTK_2_0(2.4.0,,AC_MSG_ERROR([GTK+ >= 2.4.0 is required]))

# libglade 2.0
PKG_CHECK_MODULES(LIBGLADE, libglade-2.0 >= 2.4.0,,
	AC_MSG_ERROR([libglade >= 2.4.0 is required]))

# gtkglext
AM_PATH_GTKGLEXT_1_0(1.0.0,,AC_MSG_ERROR([GtkGlExt >= 1.0.0 is required]))

# libg3d
PKG_CHECK_MODULES(LIBG3D, libg3d >= 0.1.0,,
	AC_MSG_ERROR([libg3d >= 0.1.0 is required]))

# libm
AC_CHECK_LIB(m, cos,, [AC_MSG_ERROR([requires math library])])
MATH_LIBS=-lm
AC_SUBST(MATH_LIBS)

# OpenGL stuff
AC_PROG_CXX
AX_CHECK_GL
AX_CHECK_GLU
if test "x$no_gl" = "xyes" || test "x$no_glu" = "xyes" ; then
	AC_MSG_ERROR([libGL and libGLU are required])
fi

# OSMesa stuff
AC_CHECK_HEADERS(GL/osmesa.h, have_osmesa_h=yes)
AC_CHECK_LIB(OSMesa, OSMesaCreateContext, have_osmesa=yes)
AM_CONDITIONAL(HAVE_OSMESA,
	[test "$have_osmesa_h" = "yes" -a "$have_osmesa" = "yes" ])
OSMESA_LIBS=-lOSMesa
AC_SUBST(OSMESA_CFLAGS)
AC_SUBST(OSMESA_LIBS)

# cwiid, optional
PKG_CHECK_MODULES(CWIID, cwiid, [AC_DEFINE(HAVE_CWIID,1,[have cwiid lib])])
AC_SUBST(CWIID_CFLAGS)
AC_SUBST(CWIID_LIBS)

# Check some UNIX headers
AC_CHECK_HEADERS(dirent.h sys/stat.h unistd.h,,
	AC_MSG_ERROR([missing header file]),
	[#include <sys/types.h>])

# Look for snprintf (optional)
AC_CHECK_FUNC(snprintf,[AC_DEFINE(HAVE_SNPRINTF,1,[snprintf])],,stdio.h)

# Look for fstat (required)
AC_CHECK_FUNC(fstat, have_fstat=yes, have_fstat=no, [unistd.h sys/types.h])
if test $have_fstat = no; then
	AC_MSG_ERROR([fstat function not found])
fi

# Debug level (verbosity & optimization)
AC_MSG_CHECKING([for debug level])
AC_ARG_ENABLE(debug, 
  [[  --enable-debug[=LVL]    enable debug level LVL or 1 (default is 0)]],
  [if test "$enableval" = "yes" ; then DEBUG=1; else DEBUG=$enableval; fi],
  DEBUG=0)
AC_MSG_RESULT($DEBUG)
AC_SUBST(DEBUG)

AC_ARG_ENABLE(profile,
	[[  --enable-profile        enable gprof profiling]],
	[if test "x$enableval" = "xyes" ; then profile="-pg"
		else profile=""; fi],
	profile="")

if test "$DEBUG" = "0"; then
	DBG_CFLAGS="-O2 $profile"
else
	DBG_CFLAGS="-g $profile -DGLIB_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED"
fi
AC_SUBST(DBG_CFLAGS)

# Data & plugin directories
AC_DEFINE_DIR(DATA_DIR,   datadir/$PACKAGE, [Data directory])

# Create the files
AC_OUTPUT(Makefile
	m4/Makefile
	man/Makefile
	libs/Makefile
	libs/g3dgl/Makefile
	libs/g3dgl-widget/Makefile
	src/Makefile
	thumbnailer/Makefile
	pixmaps/Makefile
	glade/Makefile
	examples/Makefile
	po/Makefile.in
	g3dviewer.desktop
)
	

