/* $Id$ */

/*
	G3DViewer - 3D object viewer

	Copyright (C) 2005, 2006  Markus Dahms <mad@automagically.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _MAIN_H
#define _MAIN_H

#include <GL/gl.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <glade/glade-xml.h>
#include <g3d/g3d.h>

#define G3D_FLAG_GL_SPECULAR        (1L << 0)
#define G3D_FLAG_GL_SHININESS       (1L << 1)
#define G3D_FLAG_GL_ALLTWOSIDE      (1L << 2)
#define G3D_FLAG_GL_TEXTURES        (1L << 3)

typedef struct {
	G3DContext *g3dcontext;
	G3DModel *model;
	gchar *filename;

	/* GL stuff */
	GLfloat quat[4];
	GLfloat zoom;
	GLfloat aspect;
	GLfloat bgcolor[4];
	guint32 glflags;

	/* mouse */
	struct {
		gint32 beginx, beginy;
	} mouse;

	/* model information */
	struct {
		GtkTreeStore *treestore;
	} info;

	/* interface */
	struct {
		GladeXML *xml;

		GtkWidget *window;
		GtkWidget *infowin;

		GtkWidget *glarea;
		GdkPixbuf **icons;
	} interface;

} G3DViewer;

#endif /* _MAIN_H */
