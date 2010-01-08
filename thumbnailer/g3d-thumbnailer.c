/* $Id$ */

/*
	G3DViewer - 3D object viewer

	Copyright (C) 2005 - 2009  Markus Dahms <mad@automagically.de>

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
#include <string.h>
#include <locale.h>

#include <GL/osmesa.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <g3d/g3d.h>
#include <g3d/quat.h>

#include <G3DGLSimpleRenderer.h>

static gboolean screenshot_save_from_pixels(guint8 *pixels,
	const gchar *filename,
	guint32 width, guint32 height);

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message, gpointer user_data)
{
}

int main(int argc, char *argv[])
{
	OSMesaContext omctxt;
	G3DContext *context;
	G3DModel *model;
	G3DGLRenderer *renderer;
	G3DGLRenderOptions *options;
	G3DQuat q1[4], q2[4];
	G3DVector a1[3] = { 0.0, 1.0, 0.0 }, a2[3] = {1.0, 0.0, 1.0};
	guint32 width = 128;
	guint32 height = 128;
	gint retval = EXIT_FAILURE;
	guint8 *imgbuf;
	GError *error = NULL;
	GOptionContext *opt_ctxt;
	gdouble opt_angle_x = 45.0;
	gdouble opt_angle_y = 45.0;
	gint32 opt_height = 0;
	gchar *opt_bgcolor = NULL;
	guint32 bgcolor;
	GOptionEntry opt_entries[] = {
		{ "angle-x", 'x', 0, G_OPTION_ARG_DOUBLE, &opt_angle_x,
			_("x rotation of view angle"), NULL },
		{ "angle-y", 'y', 0, G_OPTION_ARG_DOUBLE, &opt_angle_y,
			_("y rotation of view angle"), NULL },
		{ "bgcolor", 'c', 0, G_OPTION_ARG_STRING, &opt_bgcolor,
			_("background color in #RRGGBBAA notation"), NULL },
		{ "height", 'h', 0, G_OPTION_ARG_INT, &opt_height,
			_("height of image in px (default: width)"), NULL },
		{ NULL }
	};

	/* localization stuff */
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	g_type_init();

	opt_ctxt = g_option_context_new(
		_("<input model> <output image> [<width in px>]"));
	g_option_context_add_main_entries(opt_ctxt, opt_entries, PACKAGE);
	g_option_context_parse(opt_ctxt, &argc, &argv, &error);
	if(error) {
		g_warning("option parser: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	if(argc < 3) {
		g_print(_("usage: %s <input file: model> <output file: image> "
			"[<width in px>]\n"),
			argv[0]);
		return EXIT_FAILURE;
	}
	g_option_context_free(opt_ctxt);

	g_log_set_handler("LibG3D", G_LOG_LEVEL_DEBUG, log_handler, NULL);

	if(argc > 3) {
		/* size */
		width = atoi(argv[3]);
		/* height = width / 4 * 3; */
		if(opt_height > 0)
			height = opt_height;
		else
			height = width;
	}

	/* initialize render options */
	options = g_new0(G3DGLRenderOptions, 1);
	options->updated = TRUE;
	options->initialized = FALSE;
	options->glflags =
		/* G3D_FLAG_GL_SHININESS | G3D_FLAG_GL_TWOSIDED | */
		G3D_FLAG_GL_TEXTURES | G3D_FLAG_GL_COLORS;
	options->zoom = 40;
	options->aspect = (gfloat)width / (gfloat)height;
	options->bgcolor[0] = options->bgcolor[1] = options->bgcolor[2] = 0.5;
	options->bgcolor[3] = 0.0;

	if(opt_bgcolor && (opt_bgcolor[0] == '#') && (strlen(opt_bgcolor) == 9)) {
		bgcolor = strtoul(opt_bgcolor + 1, NULL, 16);
		options->bgcolor[0] = ((bgcolor >> 24) & 0xFF) / 255.0;
		options->bgcolor[1] = ((bgcolor >> 16) & 0xFF) / 255.0;
		options->bgcolor[2] = ((bgcolor >> 8) & 0xFF) / 255.0;
		options->bgcolor[3] = (bgcolor & 0xFF) / 255.0;
	}

	g3d_quat_trackball(options->quat, 0.0, 0.0, 0.0, 0.0, 0.8);
	g3d_quat_rotate(q1, a1, - opt_angle_y * G_PI / 180.0);
	g3d_quat_rotate(q2, a2, - opt_angle_x * G_PI / 180.0);
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
	model = g3d_model_load_full(context, argv[1],
		G3D_MODEL_SCALE | G3D_MODEL_CENTER);

	if(model) {
		if(model->tex_images)
			g_hash_table_foreach(model->tex_images, g3dgl_load_texture, NULL);

		renderer = g3d_gl_simple_renderer_new(options);
		g3d_gl_renderer_prepare(renderer, model);
		g3d_gl_renderer_clear(renderer);
		g3d_gl_renderer_setup_view(renderer);
		g3d_gl_renderer_draw(renderer);

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

static gboolean screenshot_save_from_pixels(guint8 *pixels,
	const gchar *filename,
	guint32 width, guint32 height)
{
	GdkPixbuf *pixbuf, *flipped;

	g_return_val_if_fail(pixels != NULL, FALSE);

	pixbuf = gdk_pixbuf_new_from_data(pixels,
		GDK_COLORSPACE_RGB, TRUE,
		8, width, height, width * 4,
		NULL, NULL);

	if(pixbuf == NULL)
		return FALSE;

	flipped = gdk_pixbuf_flip(pixbuf, FALSE);
	if(flipped == NULL)
		return FALSE;
	gdk_pixbuf_save(flipped, filename, "png", NULL, NULL);

	gdk_pixbuf_unref(flipped);
	gdk_pixbuf_unref(pixbuf);

	return TRUE;
}

