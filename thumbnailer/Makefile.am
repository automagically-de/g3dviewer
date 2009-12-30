if HAVE_OSMESA
bin_PROGRAMS = g3d-thumbnailer
endif

localedir = $(datadir)/locale

g3d_thumbnailer_SOURCES = \
	g3d-thumbnailer.c

g3d_thumbnailer_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-I$(top_srcdir)/libs/g3dgl \
	-I$(top_srcdir)/libs/g3dgl-widget \
	-Wall -ansi \
	-DLOCALEDIR=\"$(localedir)\" \
	@X_CFLAGS@ @LIBG3D_CFLAGS@ \
	@GTK_CFLAGS@ @OSMESA_CFLAGS@ \
	-DDEBUG=@DEBUG@ \
	-DPACKAGE=\"@PACKAGE@\"

g3d_thumbnailer_LDFLAGS = \
	@X_LIBS@ @GTK_LIBS@ @LIBG3D_LIBS@ @OSMESA_LIBS@ @GL_LIBS@ \
	../libs/g3dgl/libg3dgl.la

g3d_thumbnailer_DEPENDENCIES = \
	../libs/g3dgl/libg3dgl.la

../libs/g3dgl/libg3dgl.la:
	$(MAKE) -C ../libs/g3dgl

mimelocaldir = ${HOME}/.local/share/mime
mimefiles = g3dviewer.xml

install-data-user: install-mime-user install-schema-user

install-mime-user: ${mimefiles}
	mkdir -p ${mimelocaldir}/packages
	mkdir -p ${mimelocaldir}/applications
	cp ${mimefiles} ${mimelocaldir}/packages
	update-mime-database ${mimelocaldir}

install-schema-user:
	./gen_schema.sh
	gconftool-2 --install-schema-file=g3d-thumbnailer.schema

EXTRA_DIST = \
	g3dviewer.xml \
	gen_schema.sh
