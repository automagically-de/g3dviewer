SUBDIRS = m4 man src thumbnailer pixmaps glade examples po

appldir = ${datadir}/applications
appl_DATA = g3dviewer.desktop

deb:
	fakeroot dpkg-buildpackage

EXTRA_DIST = \
	CREDITS COPYING.trackball README.thumbnailer \
	g3dviewer.desktop.in
