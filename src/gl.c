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
#include <g3d/matrix.h>

#include "gl.h"
#include "trackball.h"

struct _G3DGLRenderState {
	gint32 gl_dlist;
	G3DMaterial *prev_material;
	guint32 prev_texid;
};

#if DEBUG > 0
#define TIMING
#endif

#ifdef TIMING
static GTimer *timer = NULL;
#endif

#define TRAP_GL_ERROR(text) \
	error = glGetError(); \
	if(error != GL_NO_ERROR) \
		g_printerr("[gl] %s: E: %d\n", text, error);

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

#if 0
	glEnable(GL_CULL_FACE);
#endif

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
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
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

	glTexImage2D(
		GL_TEXTURE_2D /* target */,
		0 /* level */,
		GL_RGBA /* internalFormat */,
		image->width /* width */,
		image->height /* height */,
		0 /* border */,
		GL_RGBA /* format */,
		GL_UNSIGNED_BYTE /* type */,
		image->pixeldata /* pixels */);
	TRAP_GL_ERROR("gl_load_texture - teximage");
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

static inline void gl_update_material(G3DGLRenderOptions *options,
	G3DMaterial *material)
{
	GLenum facetype;
	GLfloat normspec[4] = { 0.0, 0.0, 0.0, 1.0 };

	g_return_if_fail(material != NULL);

	if(options->glflags & G3D_FLAG_GL_ALLTWOSIDE)
		facetype = GL_FRONT_AND_BACK;
	else
		facetype = GL_FRONT;

	if(options->glflags & G3D_FLAG_GL_COLORS)
		glColor4f(
			material->r,
			material->g,
			material->b,
			material->a);
	else
		glColor4f(0.7, 0.7, 0.7, 1.0);

	return;

	if(options->glflags & G3D_FLAG_GL_SPECULAR)
		glMaterialfv(facetype, GL_SPECULAR, material->specular);
	else
		glMaterialfv(facetype, GL_SPECULAR, normspec);

	if(options->glflags & G3D_FLAG_GL_SHININESS)
		glMaterialf(facetype, GL_SHININESS, material->shininess * 10);
	else
		glMaterialf(facetype, GL_SHININESS, 0.0);
}

static inline void gl_draw_face(G3DGLRenderOptions *options,
	G3DObject *object, gint32 i, gfloat min_a, gfloat max_a,
	gboolean *dont_render, gboolean *init)
{
	gint32 j;

	if(*init)
	{
		options->state->prev_material = NULL;
		options->state->prev_texid = 0;
		*init = FALSE;
	}

	/* material check */
	if(options->state->prev_material != object->_materials[i])
	{
		if((object->_materials[i]->a < min_a) ||
			(object->_materials[i]->a >= max_a))
		{
			*dont_render = TRUE;
			return;
		}

		*dont_render = FALSE;

		glEnd();
		gl_update_material(options, object->_materials[i]);
		glBegin(GL_TRIANGLES);
		options->state->prev_material = object->_materials[i];

		options->state->prev_texid = 0;
	}

	if(*dont_render) return;

	/* texture stuff */
	if((options->glflags & G3D_FLAG_GL_TEXTURES) &&
		(object->_flags[i] & G3D_FLAG_FAC_TEXMAP))
	{
		/* if texture has changed update to new texture */
		if(object->_tex_images[i] != options->state->prev_texid)
		{
			options->state->prev_texid = object->_tex_images[i];
			glEnd();
			glBindTexture(GL_TEXTURE_2D, options->state->prev_texid);
			glBegin(GL_TRIANGLES);
#if DEBUG > 5
			g_print("gl: binding to texture id %d\n", prev_texid);
#endif
		}
	}


	/* draw triangles */
	for(j = 0; j < 3; j ++)
	{
		if((options->glflags & G3D_FLAG_GL_TEXTURES) &&
			(object->_flags[i] & G3D_FLAG_FAC_TEXMAP))
		{
			glTexCoord2f(
				object->_tex_coords[(i * 3 + j) * 2 + 0],
				object->_tex_coords[(i * 3 + j) * 2 + 1]);
#if DEBUG > 5
			g_print("gl: setting texture coords: %f, %f\n",
				object->_tex_coords[(i * 3 + j) * 2 + 0],
				object->_tex_coords[(i * 3 + j) * 2 + 1]);
#endif
		}

		glNormal3f(
			object->_normals[(i*3+j)*3+0],
			object->_normals[(i*3+j)*3+1],
			object->_normals[(i*3+j)*3+2]);
		glVertex3f(
			object->vertex_data[object->_indices[i*3+j]*3+0],
			object->vertex_data[object->_indices[i*3+j]*3+1],
			object->vertex_data[object->_indices[i*3+j]*3+2]);

	} /* 1 .. 3 */
}

static inline void gl_draw_objects(G3DGLRenderOptions *options,
	GSList *objects, gfloat min_a, gfloat max_a)
{
	GSList *olist;
	int i;
	G3DObject *object;
	gboolean dont_render;
	gboolean init = TRUE;

	olist = objects;
	while(olist != NULL)
	{
		object = (G3DObject *)olist->data;
		olist = olist->next;

		dont_render = FALSE;

		/* don't render invisible objects */
		if(object->hide) continue;

		g_return_if_fail(object != NULL);
#if DEBUG > 3
		g_printerr("name: %s {", object->name);
#endif

#if DEBUG > 2
		g_printerr("new object\n");
#endif

		glPushMatrix();

		if(object->transformation)
		{
			glMultMatrixf(object->transformation->matrix);
		}

		glBegin(GL_TRIANGLES);

		for(i = 0; i < object->_num_faces; i ++)
		{
			gl_draw_face(options, object, i, min_a, max_a,
				&dont_render, &init);
		} /* all faces */

		glEnd();

		/* handle sub-objects */
		gl_draw_objects(options, object->objects, min_a, max_a);

		glPopMatrix();

	} /* while olist != NULL */
}

static inline void gl_setup_view(G3DGLRenderOptions *options)
{
	GLfloat m[4][4];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(options->zoom, options->aspect, 1, 100);
	/* translation of view */
	glTranslatef(options->offx, options->offy, 0.0);

	glMatrixMode(GL_MODELVIEW);

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

	glLoadIdentity();
	glTranslatef(0, 0, -30);
	build_rotmatrix(m, options->quat);
	glMultMatrixf(&m[0][0]);
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

	TRAP_GL_ERROR("gl_draw - start");

	if(!options->initialized)
	{
		gl_init();
		options->initialized = TRUE;
#ifdef TIMING
		ignore_timing = TRUE;
#endif
	}

	/* prepare viewport */
	gl_setup_view(options);

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

		/* update render state */
		if(options->state) {
			glDeleteLists(options->state->gl_dlist, 1);
			g_free(options->state);
		}
		options->state = g_new0(G3DGLRenderState, 1);

		/* create and execute display list */
		options->state->gl_dlist = glGenLists(1);

		glNewList(options->state->gl_dlist, GL_COMPILE);
		/* draw all objects */
		for(f = 1.0; f >= 0.0; f -= 0.2)
			gl_draw_objects(options, model->objects, f, f + 0.2);
		glEndList();

		TRAP_GL_ERROR("gl_draw - building list");
	}

	g_return_if_fail(options->state != NULL);

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

