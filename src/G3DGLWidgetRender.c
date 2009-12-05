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

#include <GL/gl.h>
#include <GL/glu.h>

#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/quat.h>
#include <g3d/face.h>

#include "G3DGLWidget.h"
#include "G3DGLWidgetPriv.h"

#include "gl_helpers.c"

gboolean g3d_gl_widget_render_init(G3DGLWidget *self)
{
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

	/* colors and materials */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	/* texture stuff */
	glEnable(GL_TEXTURE_2D);

	return TRUE;
}

void g3d_gl_widget_render_decorations(G3DGLWidget *self)
{
	guint32 width, height;

	width = GTK_WIDGET(self)->allocation.width;
	height = GTK_WIDGET(self)->allocation.height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width - 1, height - 1, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (self->priv->focused) {
		glColor3f(0.0, 0.0, 0.0);
		glLineStipple(1, 0xAAAA); /* dotted line */
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINE_LOOP);
		glVertex3f(1, 1, 0);
		glVertex3f(width - 2, 1, 0);
		glVertex3f(width - 2, height - 2, 0);
		glVertex3f(1, height - 2, 0);
		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}
}

gboolean g3d_gl_widget_render_setup_view(G3DGLWidget *self)
{
	G3DFloat w, h;
	GLfloat m[4][4];
	G3DMatrix *g3dm;
	G3DGLRenderOptions *options = self->priv->gloptions;

	glClearColor(
		self->priv->bgcolor[0],
		self->priv->bgcolor[1],
		self->priv->bgcolor[2],
		self->priv->bgcolor[3]);
	glClearDepth(1.0);
	glClearIndex(0.3);
	glClear(
		GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT |
		GL_ACCUM_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);

	g3d_gl_widget_render_decorations(self);

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

	return TRUE;
}

static void g3d_gl_widget_render_shadow_plane(G3DGLWidget *self)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	G3DVector plane[3] = { 0.0, -20.0, 0.0 };
	G3DVector light[3] = { 100.0, 500.0, 20.0 };
	G3DVector normal[3] = { 0.0, -1.0, 0.0 };
	G3DMatrix shadow_matrix[16];

	if(options->glflags & G3D_FLAG_GL_SHADOW) {
		plane[1] = self->priv->model_min_y;

		/* reflection */
		glPushMatrix();
		gl_setup_floor_stencil(options);
		glTranslatef(0.0, (self->priv->model_min_y * 2), 0.0);
		glScalef(1.0, -1.0, 1.0);
		glCallList(self->priv->dlists[G3DGLW_DLIST_SHADOW]);
		glPopMatrix();

		/* plane */
		glDisable(GL_LIGHTING);
		glBindTexture (GL_TEXTURE_2D, 0);
		glColor4f(0.5, 0.5, 0.5, 0.7);
		gl_draw_plane(options);
		glEnable(GL_LIGHTING);
		/* shadow */
		glPushMatrix();
		g3d_matrix_shadow(light, plane, normal, shadow_matrix);
		glBindTexture (GL_TEXTURE_2D, 0);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glMultMatrixf(shadow_matrix);
		gl_setup_shadow_stencil(options,
			self->priv->dlists[G3DGLW_DLIST_SHADOW]);
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
}

void g3d_gl_widget_render(G3DGLWidget *self)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	G3DModel *model = self->priv->model;
	G3DFloat f;
	gint32 i;

	glBindTexture(GL_TEXTURE_2D, 0);

	if(!model || !options)
		return;

	if(options->updated) {
		if(self->priv->dlists_valid) {
			for(i = 0; i < G3DGLW_N_DLISTS; i ++)
				glDeleteLists(self->priv->dlists[i], 1);
			self->priv->dlists_valid = FALSE;
		}

		/* create display lists */
		for(i = 0; i < G3DGLW_N_DLISTS; i ++)
			self->priv->dlists[i] = glGenLists(1);

		/* for position of ground plane */
		self->priv->model_min_y = gl_min_y(model->objects);
		
		/* fill lists */
		glNewList(self->priv->dlists[G3DGLW_DLIST_MODEL], GL_COMPILE);
		for(f = 1.0; f >= 0.0; f -= 0.2)
			gl_draw_objects(options,
				&(self->priv->prev_material),
				&(self->priv->prev_texid),
				model->objects, f, f + 0.2, FALSE);
		glEndList();

		glNewList(self->priv->dlists[G3DGLW_DLIST_SHADOW], GL_COMPILE);
		gl_draw_objects(options,
			&(self->priv->prev_material),
			&(self->priv->prev_texid),
			model->objects, 0.0, 1.0, TRUE);
		glEndList();

		/* list are valid now */
		self->priv->dlists_valid = TRUE;
		options->updated = TRUE;
	}

	/* shadow */
	g3d_gl_widget_render_shadow_plane(self);

	/* draw model */
	glCallList(self->priv->dlists[G3DGLW_DLIST_MODEL]);
}

