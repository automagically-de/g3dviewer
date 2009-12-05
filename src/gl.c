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

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <glib.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <g3d/types.h>
#include <g3d/quat.h>
#include <g3d/matrix.h>
#include <g3d/vector.h>
#include <g3d/face.h>

#include "gl.h"

struct _G3DGLRenderState {
	gint32 gl_dlist, gl_dlist_shadow;
	G3DMaterial *prev_material;
	guint32 prev_texid;
};

#include "gl_helpers.c"

#if DEBUG > 0
#define TIMING
#endif

#ifdef TIMING
static GTimer *timer = NULL;
#endif

static void gl_init(void)
{
	GLenum error;

#if DEBUG > 1
	g_printerr("init OpenGL\n");
#endif

	TRAP_GL_ERROR("gl_init - start");

	GLfloat light0_pos[4] = { -50.0, 50.0, 0.0, 0.0 };
	GLfloat light0_col[4] = { 0.6, 0.6, 0.6, 1.0 };
	GLfloat light1_pos[4] = {  50.0, 50.0, 0.0, 0.0 };
	GLfloat light1_col[4] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat ambient_lc[4] = { 0.35, 0.35, 0.35, 1.0 };

	/* transparency and blending */
#if 0
	glAlphaFunc(GL_GREATER, 0.1);
#endif
	glEnable(GL_ALPHA_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	TRAP_GL_ERROR("gl_init - alpha, blend, depth");

#if 0
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
#endif

#if 0
	glDisable(GL_DITHER);
#endif
	glShadeModel(GL_SMOOTH);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_lc);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
#ifdef GL_LIGHT_MODEL_COLOR_CONTROL
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#endif
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_col);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_col);
	glLightfv(GL_LIGHT1, GL_SPECULAR,  light1_col);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	TRAP_GL_ERROR("gl_init - light stuff");

	/* colors and materials */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	TRAP_GL_ERROR("gl_init - color material");

	/* texture stuff */
	glEnable(GL_TEXTURE_2D);

	TRAP_GL_ERROR("gl_init - enable texture");

#ifdef TIMING
	timer = g_timer_new();
#endif
}

void gl_set_twoside(gboolean twoside)
{
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, twoside ? 1 : 0);
	glColorMaterial(
		twoside ? GL_FRONT_AND_BACK : GL_FRONT,
		GL_AMBIENT_AND_DIFFUSE);
}

void gl_set_textures(gboolean textures)
{
	if(textures)
		glEnable(GL_TEXTURE_2D);
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

/* GHFunc */
void gl_load_texture(gpointer key, gpointer value, gpointer data)
{
	G3DImage *image = (G3DImage *)value;
	gint32 env;
	GLenum error;

	TRAP_GL_ERROR("gl_load_texture - start");

#if 0
	/* predefined - update object->_tex_images else... */
	glGenTextures(1, &(image->tex_id));
#endif

#if DEBUG > 0
	g_print("gl: loading texture '%s' (%dx%dx%d) - id %d\n",
		image->name ? image->name : "(null)",
		image->width, image->height, image->depth,
		image->tex_id);
#endif

	glBindTexture(GL_TEXTURE_2D, image->tex_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_NEAREST);

	TRAP_GL_ERROR("gl_load_texture - bind, param");

	switch(image->tex_env)
	{
		case G3D_TEXENV_BLEND: env = GL_BLEND; break;
		case G3D_TEXENV_MODULATE: env = GL_MODULATE; break;
		case G3D_TEXENV_DECAL: env = GL_DECAL; break;
		case G3D_TEXENV_REPLACE: env = GL_REPLACE; break;
		default: env = GL_MODULATE; break;
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, env);
	TRAP_GL_ERROR("gl_load_texture - texenv");

	gluBuild2DMipmaps(
		GL_TEXTURE_2D,
		GL_RGBA,
		image->width,
		image->height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image->pixeldata);
	TRAP_GL_ERROR("gl_load_texture - mipmaps");
}

static inline void gl_draw_osd(G3DGLRenderOptions *options)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, options->width - 1, options->height - 1, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (options->focused) {
		glColor3f(0.0, 0.0, 0.0);
		glLineStipple(1, 0xAAAA); /* dotted line */
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINE_LOOP);
		glVertex3f(1, 1, 0);
		glVertex3f(options->width - 2, 1, 0);
		glVertex3f(options->width - 2, options->height - 2, 0);
		glVertex3f(1, options->height - 2, 0);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}
}

void gl_setup_view(G3DGLRenderOptions *options)
{
	GLfloat m[4][4];
	G3DMatrix *g3dm;
	G3DFloat w, h;

	glClearColor(
		options->bgcolor[0],
		options->bgcolor[1],
		options->bgcolor[2],
		options->bgcolor[3]);
	glClearDepth(1.0);
	glClearIndex(0.3);
	glClear(
		GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT |
		GL_ACCUM_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);

	gl_draw_osd(options);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if(options->glflags & G3D_FLAG_GL_ISOMETRIC) {
		w = 0.5 * options->zoom;
		h = w / options->aspect;
		glOrtho(-w / 2.0, w / 2.0, -h / 2.0, h / 2.0, 1, 100);
	} else {
		gluPerspective(options->zoom, options->aspect, 1, 100);
	}
	/* translation of view */
	glTranslatef(options->offx, options->offy, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0, 0, -30);
	g3dm = g3d_matrix_new();
	g3d_quat_to_matrix(options->quat, g3dm);
	matrix_g3d_to_gl(g3dm, m);

	g3d_matrix_free(g3dm);
	glMultMatrixf(&m[0][0]);
}


void gl_draw_coord_system(G3DGLRenderOptions *options)
{
	if(options->glflags & G3D_FLAG_GL_COORD_AXES) {
		/* x: red */
		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(10.0, 0.0, 0.0);
		glEnd();
		/* y: green */
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 10.0, 0.0);
		glEnd();
		/* z: blue */
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 10.0);
		glEnd();
	}
}

void gl_draw(G3DGLRenderOptions *options, G3DModel *model)
{
	GLenum error;
	gfloat f;
#ifdef TIMING
	gboolean ignore_timing = FALSE;
	gulong msec, add;
	gdouble sec;
#endif
	G3DVector light[3] = { 100.0, 500.0, 20.0 };
	G3DVector plane[3] = { 0.0, -20.0, 0.0 };
	G3DVector normal[3] = { 0.0, -1.0, 0.0 };

	TRAP_GL_ERROR("gl_draw - start");

	if(!options->initialized)
	{
		gl_init();
		options->initialized = TRUE;
#ifdef TIMING
		ignore_timing = TRUE;
#endif
	}

	/* reset texture */
	glBindTexture (GL_TEXTURE_2D, 0);
	TRAP_GL_ERROR("gl_draw - bind texture 0");

	if(model == NULL)
		return;

#ifdef TIMING
	g_timer_start(timer);
#endif

	if(options->updated) {
		options->updated = FALSE;
#ifdef TIMING
		ignore_timing = TRUE;
#endif
#if DEBUG > 2
		g_printerr("[gl] creating new display list\n");
#endif
		options->min_y = gl_min_y(model->objects);

		/* update render state */
		if(options->state) {
			glDeleteLists(options->state->gl_dlist, 1);
			glDeleteLists(options->state->gl_dlist_shadow, 1);
			g_free(options->state);
		}
		options->state = g_new0(G3DGLRenderState, 1);

		/* create and execute display list */
		options->state->gl_dlist = glGenLists(1);
		options->state->gl_dlist_shadow = glGenLists(1);

		glNewList(options->state->gl_dlist, GL_COMPILE);
		/* draw all objects */
		for(f = 1.0; f >= 0.0; f -= 0.2)
			gl_draw_objects(options,
				&(options->state->prev_material),
				&(options->state->prev_texid),
				model->objects, f, f + 0.2, FALSE);
		glEndList();

		if(options->glflags & G3D_FLAG_GL_SHADOW) {
			glNewList(options->state->gl_dlist_shadow, GL_COMPILE);
			gl_draw_objects(options,
				&(options->state->prev_material),
				&(options->state->prev_texid),
				model->objects, 0.0, 1.0, TRUE);
			glEndList();
		}

		TRAP_GL_ERROR("gl_draw - building list");
	}

	g_return_if_fail(options->state != NULL);

	gl_draw_coord_system(options);

	if(options->glflags & G3D_FLAG_GL_SHADOW) {
		plane[1] = options->min_y;

		/* reflection */
		glPushMatrix();
		gl_setup_floor_stencil(options);
		glTranslatef(0.0, (options->min_y * 2), 0.0);
		glScalef(1.0, -1.0, 1.0);
		glCallList(options->state->gl_dlist);
		glPopMatrix();

		/* plane */
		glDisable(GL_LIGHTING);
		glBindTexture (GL_TEXTURE_2D, 0);
		glColor4f(0.5, 0.5, 0.5, 0.7);
		gl_draw_plane(options);
		glEnable(GL_LIGHTING);
		/* shadow */
		glPushMatrix();
		g3d_matrix_shadow(light, plane, normal, options->shadow_matrix);
		glBindTexture (GL_TEXTURE_2D, 0);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glMultMatrixf(options->shadow_matrix);
		gl_setup_shadow_stencil(options, options->state->gl_dlist_shadow);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.0, 0.001, 0.0);
		glColor4f(0.3, 0.3, 0.3, 0.7);
		gl_draw_plane(options);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glPopMatrix();

		glDisable(GL_STENCIL_TEST);
	}

	/* execute display list */
	glCallList(options->state->gl_dlist);

	TRAP_GL_ERROR("gl_draw - call list");

#ifdef TIMING /* get time to draw one frame to compare algorithms */
	g_timer_stop(timer);

	if(!ignore_timing) {
		if(options->avg_msec == 0) {
			sec = g_timer_elapsed(timer, &msec);
			options->avg_msec = (gulong)sec * 1000000 + msec;
		} else {
			sec = g_timer_elapsed(timer, &msec);
			add = (gulong)sec * 1000000 + msec;
			options->avg_msec = (options->avg_msec + add) / 2;
		}
	}
#endif

#if DEBUG > 3
	g_printerr("gl.c: drawn...\n");
#endif
}

