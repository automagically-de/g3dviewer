#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <g3d/g3d.h>

#include "gl.h"
#include "trackball.h"
#include "screenshot.h"

static gboolean setup_gl()
{
	Display *display;
	XVisualInfo *visinfo;
	Pixmap pixmap;
	GLXPixmap glxpixmap;
	GLXContext glxctx;
	char *dpyname;
	int attrlist_dbl[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None};
	int attrlist_sng[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None};

	dpyname = getenv("DISPLAY");

	display = XOpenDisplay(dpyname);
	if(display == NULL)
	{
		g_printerr("ERROR: could not open display '%s'\n",
			dpyname ? dpyname : "(null)");
		return FALSE;
	}

	visinfo = glXChooseVisual(display, DefaultScreen(display), attrlist_sng);
	if(visinfo == NULL)
	{
		/* try with double buffering */
		visinfo = glXChooseVisual(display, DefaultScreen(display),
			attrlist_dbl);
		if(visinfo == NULL)
		{
			g_printerr("ERROR: could not get a supported visual\n");
			return FALSE;
		}
	}

	glxctx = glXCreateContext(display, visinfo, 0, GL_FALSE);
	if(glxctx == NULL)
	{
		g_printerr("ERROR: could not create GLX context\n");
		return FALSE;
	}

	pixmap = XCreatePixmap(display, RootWindow(display, visinfo->screen),
		240, 160, visinfo->depth);
	if(pixmap <= 0)
	{
		g_printerr("ERROR: could not create pixmap\n");
		return FALSE;
	}

	glxpixmap = glXCreateGLXPixmap(display, visinfo, pixmap);
	if(glxpixmap <= 0)
	{
		g_printerr("ERROR: could not create GLX pixmap\n");
		return FALSE;
	}

	glXMakeCurrent(display, glxpixmap, glxctx);

	return TRUE;
}

int main(int argc, char *argv[])
{
	G3DContext *context;
	G3DModel *model;
	gfloat bgcolor[4] = { 1.0, 1.0, 1.0, 0.0 };
	gfloat quat[4] = { 0.0, 0.0, 0.0, 0.0 };

	gtk_init(&argc, &argv);

	setup_gl();
	trackball(quat, 0.5, 0.5, 0.0, 0.0);

	if(argc < 3)
	{
		g_print("usage: %s <input file: model> <output file: image>\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	context = g3d_context_new();
	model = g3d_model_load(context, argv[1]);

	if(model)
	{
		gl_draw(
			G3D_FLAG_GL_SHININESS | G3D_FLAG_GL_ALLTWOSIDE |
			G3D_FLAG_GL_TEXTURES,
			45 /* zoom */,
			240.0 / 160.0,
			bgcolor,
			quat,
			model);

		glXWaitGL();

		return screenshot_save(argv[2], 240, 160);
	}

	return EXIT_FAILURE;
}
