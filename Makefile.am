ACLOCAL_AMFLAGS = -I m4
SUBDIRS = m4 man libs src thumbnailer pixmaps glade examples po

appldir = ${datadir}/applications
appl_DATA = g3dviewer.desktop

deb:
	fakeroot dpkg-buildpackage

EXTRA_DIST = \
	CREDITS README.thumbnailer \
	g3dviewer.desktop.in
