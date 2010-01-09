lib_LTLIBRARIES = libg3dgl-widget.la
noinst_PROGRAMS = test_g3dw

libg3dgl_widget_la_SOURCES = \
	G3DGLWidget.c G3DGLWidget.h \
	G3DGLWidgetPriv.h \
	G3DGLWidgetProperties.c G3DGLWidgetProperties.h \
	G3DGLWidgetRender.c G3DGLWidgetRender.h

libg3dgl_widget_la_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-I$(top_srcdir)/libs/g3dgl \
	-Wall -ansi \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ @GTKGLEXT_CFLAGS@ \
	@GL_CFLAGS@ @GLU_CFLAGS@ \
	@LIBG3D_CFLAGS@ \
	-DDEBUG=@DEBUG@ \
	-DPACKAGE=\"@PACKAGE@\"

libg3dgl_widget_la_LDFLAGS = \
	$(extra_ldflags)

libg3dgl_widget_la_LIBADD = \
	@GTK_LIBS@ \
	@GTKGLEXT_LIBS@ @LIBG3D_LIBS@ $(extra_libs) \
	../g3dgl/libg3dgl.la

libg3dgl_widget_la_DEPENDENCIES = ../g3dgl/libg3dgl.la

test_g3dw_SOURCES = \
	test_g3dw.c
test_g3dw_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-Wall -ansi \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ @GTKGLEXT_CFLAGS@ \
	@GL_CFLAGS@ @GLU_CFLAGS@ \
	@LIBG3D_CFLAGS@ \
	-DDEBUG=@DEBUG@ \
	-DPACKAGE=\"@PACKAGE@\"

test_g3dw_LDFLAGS = \
	$(extra_ldflags)
test_g3dw_LDADD = \
	@GTK_LIBS@ \
	@GTKGLEXT_LIBS@ @LIBG3D_LIBS@ $(extra_libs) \
	libg3dgl-widget.la ../g3dgl/libg3dgl.la

test_g3dw_DEPENDENCIES = \
	../g3dgl/libg3dgl.la \
	libg3dgl-widget.la

