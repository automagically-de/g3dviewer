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

#include <stdio.h>
#include <stdlib.h>

#include <GL/osmesa.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <g3d/g3d.h>
#include <g3d/quat.h>

#include "gl.h"
#include "screenshot.h"
#include "texture.h"

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message, gpointer user_data)
{
}

int main(int argc, char *argv[])
{
	OSMesaContext omctxt;
	G3DContext *context;
	G3DModel *model;
	G3DGLRenderOptions *options;
	G3DQuat q1[4], q2[4];
	G3DVector a1[3] = { 0.0, 1.0, 0.0 }, a2[3] = {1.0, 0.0, 1.0};
	guint32 width = 128;
	guint32 height = 128;
	gint retval = EXIT_FAILURE;
	guint8 *imgbuf;

	gtk_init(&argc, &argv);
	g_log_set_handler("LibG3D", G_LOG_LEVEL_DEBUG, log_handler, NULL);

	if(argc < 3) {
		g_print("usage: %s <input file: model> <output file: image> "
			"[<width in px>]\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	if(argc > 3) {
		/* size */
		width = atoi(argv[3]);
		/* height = width / 4 * 3; */
		height = width;
	}

	/* initialize render options */
	options = g_new0(G3DGLRenderOptions, 1);
	options->updated = TRUE;
	options->initialized = FALSE;
	options->glflags =
		G3D_FLAG_GL_SHININESS | G3D_FLAG_GL_ALLTWOSIDE |
		G3D_FLAG_GL_TEXTURES | G3D_FLAG_GL_COLORS;
	options->zoom = 40;
	options->aspect = (gfloat)width / (gfloat)height;
	options->bgcolor[0] = options->bgcolor[1] = options->bgcolor[2] = 0.5;
	options->bgcolor[3] = 0.0;

	g3d_quat_trackball(options->quat, 0.0, 0.0, 0.0, 0.0, 0.8);
	g3d_quat_rotate(q1, a1, - 45.0 * G_PI / 180.0);
	g3d_quat_rotate(q2, a2, - 45.0 * G_PI / 180.0);
	g3d_quat_add(options->quat, q1, q2);

	/* initialize OSMesa */
	omctxt = OSMesaCreateContext(OSMESA_RGBA, NULL);
	if(omctxt == 0) {
		g_printerr("failed to create OSMesa context");
		return EXIT_FAILURE;
	}

	imgbuf = g_new0(guint8, width * height * 4);
	if(!OSMesaMakeCurrent(omctxt, imgbuf, GL_UNSIGNED_BYTE, width, height)) {
		g_printerr("OSMesaMakeCurrent failed");
		return EXIT_FAILURE;
	}

	context = g3d_context_new();
	model = g3d_model_load(context, argv[1]);

	if(model) {
		texture_load_all_textures(model);
		gl_draw(options, model);
		glFinish();
		if(screenshot_save_from_pixels(imgbuf, argv[2], width, height))
			retval = EXIT_SUCCESS;
		g3d_model_free(model);
	}

	g3d_context_free(context);
	OSMesaDestroyContext(omctxt);
	g_free(imgbuf);
	g_free(options);
	return retval;
}
