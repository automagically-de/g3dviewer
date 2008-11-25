bin_PROGRAMS = g3dviewer

datadir = @datadir@
localedir = $(datadir)/locale

g3dviewer_SOURCES = \
	main.c main.h \
	gl.c gl.h \
	glarea.h glarea.c \
	trackball.c trackball.h \
	gui_glade.c gui_glade.h \
	gui_infowin.c gui_infowin.h \
	gui_log.c gui_log.h \
	screenshot.c screenshot.h \
	gui_callbacks.c \
	texture.c texture.h \
	model.h model.c

g3dviewer_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-Wall -ansi \
	-DLOCALEDIR=\"$(localedir)\" \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ @GTKGLEXT_CFLAGS@ \
	@LIBGLADE_CFLAGS@ \
	@X_CFLAGS@ \
	@LIBG3D_CFLAGS@ \
	-DDEBUG=@DEBUG@

if OS_WIN32
extra_ldflags = -Wl,--export-all-symbols
extra_libs = @GLIB_LIBS@
else
extra_ldflags =
extra_libs = 
endif

g3dviewer_LDFLAGS = \
	$(extra_ldflags)
g3dviewer_LDADD = \
	@GTK_LIBS@ @LIBGLADE_LIBS@ \
	@GTKGLEXT_LIBS@ @LIBG3D_LIBS@ $(extra_libs)

