lib_LTLIBRARIES = libg3dgl.la
noinst_PROGRAMS = test_tess

libg3dgl_la_SOURCES = \
	G3DGLRenderer.c G3DGLRenderer.h \
	G3DGLSimpleRenderer.c G3DGLSimpleRenderer.h \
	G3DGLTessRenderer.c G3DGLTessRenderer.h \
	g3dgl.c g3dgl.h

libg3dgl_la_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-Wall -ansi \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@ \
	@LIBG3D_CFLAGS@ \
	@GL_CFLAGS@ @GLU_CFLAGS@ \
	-DDEBUG=@DEBUG@ \
	-DPACKAGE=\"@PACKAGE@\"

libg3dgl_la_LDFLAGS = \
	$(extra_ldflags)

libg3dgl_la_LIBADD = \
	@LIBG3D_LIBS@ @GL_LIBS@ @GLU_LIBS@ $(extra_libs)

test_tess_SOURCES = \
	test_tess.c
test_tess_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-Wall -ansi \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@ \
	@LIBG3D_CFLAGS@ \
	-DDEBUG=@DEBUG@ \
	-DPACKAGE=\"@PACKAGE@\"
test_tess_LDFLAGS = \
	$(extra_ldflags)
test_tess_LDADD = \
	@LIBG3D_LIBS@ \
	libg3dgl.la \
	-lGL -lGLU \
	$(extra_libs)
test_tess_DEPENDENCIES = \
	libg3dgl.la

