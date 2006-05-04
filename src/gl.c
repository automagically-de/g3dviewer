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
#include <math.h>

#include <glib.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <g3d/types.h>

#include "main.h"
#include "gl.h"
#include "trackball.h"

#if DEBUG > 1
#define TIMING
#endif

static int _initialized = 0;

#ifdef TIMING
static GTimer *timer = NULL;
static gulong avg_msec = 0;
#endif

GLuint evil;

void gl_init(G3DViewer *viewer)
{
#if DEBUG > 1
	g_printerr("init OpenGL\n");
#endif

	GLfloat light0_pos[4] = { -50.0, 50.0, 0.0, 0.0 };
	GLfloat light0_col[4] = { 0.6, 0.6, 0.6, 1.0 };
	GLfloat light1_pos[4] = {  50.0, 50.0, 0.0, 0.0 };
	GLfloat light1_col[4] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat ambient_lc[4] = { 0.25, 0.25, 0.25, 1.0 };

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
	glEnable(GL_DEPTH_TEST);

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
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_col);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_col);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	/* colors and materials */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	/* texture stuff */
	glEnable(GL_TEXTURE_2D);

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

#if 0
	/* predefined - update object->_tex_images else... */
	glGenTextures(1, &(image->gl_texid));
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
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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
	gluBuild2DMipmaps(
		GL_TEXTURE_2D,
		GL_RGBA,
		image->width,
		image->height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image->pixeldata);
}

void gl_update_material(G3DViewer *viewer, G3DMaterial *material)
{
	GLenum facetype;
	GLfloat normspec[4] = { 0.0, 0.0, 0.0, 1.0 };

	g_return_if_fail(material != NULL);

	if(viewer->glflags & G3D_FLAG_GL_ALLTWOSIDE)
		facetype = GL_FRONT_AND_BACK;
	else
		facetype = GL_FRONT;

	glColor4f(
		material->r,
		material->g,
		material->b,
		material->a);

	if(viewer->glflags & G3D_FLAG_GL_SPECULAR)
		glMaterialfv(facetype, GL_SPECULAR, material->specular);
	else
		glMaterialfv(facetype, GL_SPECULAR, normspec);

	if(viewer->glflags & G3D_FLAG_GL_SHININESS)
		glMaterialf(facetype, GL_SHININESS, material->shininess);
	else
		glMaterialf(facetype, GL_SHININESS, 0.0);
}

static void gl_draw_objects(G3DViewer *viewer, GSList *objects)
{
	GSList *olist;
	int i, j;
	G3DMaterial *prev_material = NULL;
	G3DObject *object;
	guint32 prev_texid = 0;

	olist = objects;
	while(olist != NULL)
	{
		object = (G3DObject *)olist->data;
		olist = olist->next;

		/* don't render invisible objects */
		if(object->hide) continue;

		g_return_if_fail(object != NULL);
#if DEBUG > 3
		g_printerr("name: %s {", object->name);
#endif

#if DEBUG > 2
		g_printerr("new object\n");
#endif

		glBegin(GL_TRIANGLES);

		for(i = 0; i < object->_num_faces; i ++)
		{
			if(prev_material != object->_materials[i])
			{
				gl_update_material(viewer, object->_materials[i]);
				prev_material = object->_materials[i];
			}

			if((viewer->glflags & G3D_FLAG_GL_TEXTURES) &&
				(object->_flags[i] & G3D_FLAG_FAC_TEXMAP))
			{
				/* if texture has changed update to new texture */
				if(object->_tex_images[i] != prev_texid)
				{
					prev_texid = object->_tex_images[i];
					glEnd();
					glBindTexture (GL_TEXTURE_2D, prev_texid);
					glBegin(GL_TRIANGLES);
#if DEBUG > 5
					g_print("gl: binding to texture id %d\n", prev_texid);
#endif
				}
			}

			/* draw triangles */
			for(j = 0; j < 3; j ++)
			{
				if((viewer->glflags & G3D_FLAG_GL_TEXTURES) &&
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
		/* all faces */

		glEnd();

		/* handle sub-objects */
		gl_draw_objects(viewer, object->objects);

	} /* while olist != NULL */
}

void gl_draw(G3DViewer *viewer)
{
	GLfloat m[4][4];

	if(! _initialized)
	{
		gl_init(viewer);
		_initialized = 1;
	}

	/* draw scene... */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(viewer->zoom, viewer->aspect, 1, 100);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(
		viewer->bgcolor[0],
		viewer->bgcolor[1],
		viewer->bgcolor[2],
		0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -30);
	build_rotmatrix(m, viewer->quat);
	glMultMatrixf(&m[0][0]);

	/* reset texture */
	glBindTexture (GL_TEXTURE_2D, 0);

	if(viewer->model == NULL)
		return;

#ifdef TIMING
	g_timer_start(timer);
#endif

	/* draw all objects */
	gl_draw_objects(viewer, viewer->model->objects);

#ifdef TIMING /* get time to draw one frame to compare algorithms */
	g_timer_stop(timer);

	if(avg_msec == 0)
	{
		gulong msec;
		gdouble sec;

		sec = g_timer_elapsed(timer, &msec);
		avg_msec = (gulong)sec * 1000000 + msec;
	}
	else
	{
		gulong msec, add;
		gdouble sec;

		sec = g_timer_elapsed(timer, &msec);
		add = (gulong)sec * 1000000 + msec;
		avg_msec = (avg_msec + add) / 2;
	}

	g_printerr("average time to render frame: %lu µs\n", avg_msec);
#endif

#if DEBUG > 3
	g_printerr("gl.c: drawn...\n");
#endif
}

