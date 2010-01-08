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

#include "G3DGLRenderer.h"

static G3DFloat g3d_gl_widget_min_y(GSList *objects);
static void g3d_gl_widget_draw_plane(G3DGLRenderOptions *options);
static void g3d_gl_widget_setup_floor_stencil(G3DGLRenderOptions *options);
static void g3d_gl_widget_draw_coord_system(G3DGLRenderOptions *options);

gboolean g3d_gl_widget_render_init(G3DGLWidget *self)
{
	g3d_gl_init();
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
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glColor3f(0.0, 0.0, 0.0);
		glLineStipple(1, 0xAAAA); /* dotted line */
		glEnable(GL_LINE_STIPPLE);
		glBegin(GL_LINE_LOOP);
		glVertex3f(1, 1, 0);
		glVertex3f(width - 2, 1, 0);
		glVertex3f(width - 2, height - 2, 0);
		glVertex3f(1, height - 2, 0);
		glEnd();
		glPopAttrib();
	}
}

gboolean g3d_gl_widget_render_setup_view(G3DGLWidget *self)
{
	g3d_gl_renderer_clear(self->priv->renderer);
	g3d_gl_widget_render_decorations(self);
	g3d_gl_renderer_setup_view(self->priv->renderer);
	return TRUE;
}

static void g3d_gl_widget_render_shadow_plane(G3DGLWidget *self)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	G3DVector plane[3] = { 0.0, 0.0, 0.0 };
	G3DVector light[3] = { 100.0, 500.0, 20.0 };
	G3DVector normal[3] = { 0.0, -1.0, 0.0 };
	G3DMatrix shadow_matrix[16];

	if(options->glflags & G3D_FLAG_GL_SHADOW) {
		glPushMatrix();
		glTranslatef(0.0, self->priv->model_min_y, 0.0);

		/* reflection */
		glPushMatrix();
		g3d_gl_widget_setup_floor_stencil(options);
		glTranslatef(0.0, self->priv->model_min_y, 0.0);
		glScalef(1.0, -1.0, 1.0);
		g3d_gl_renderer_draw(self->priv->renderer);
		glPopMatrix();

		/* plane */
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4f(0.5, 0.5, 0.5, 0.7);
		g3d_gl_widget_draw_plane(options);
		glPopAttrib();

		/* shadow */
		glPushMatrix();
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		g3d_matrix_shadow(light, plane, normal, shadow_matrix);
		glBindTexture (GL_TEXTURE_2D, 0);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glMultMatrixf(shadow_matrix);
		g3d_gl_renderer_draw_shadow(self->priv->renderer);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(0.0, 0.001, 0.0);
		glColor4f(0.3, 0.3, 0.3, 0.7);
		g3d_gl_widget_draw_plane(options);
		glPopMatrix();
		glPopAttrib();

		glPopMatrix();

		glDisable(GL_STENCIL_TEST);
	}
}

static void g3d_gl_widget_render_load_texture(gpointer key, gpointer value,
	gpointer data)
{
	G3DImage *image = (G3DImage *)value;
	gint32 env;

#if DEBUG > 2
	g_debug("loading texture %s (id %d)", image->name, image->tex_id);
#endif

	glBindTexture(GL_TEXTURE_2D, image->tex_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_NEAREST);

	switch(image->tex_env) {
		case G3D_TEXENV_BLEND: env = GL_BLEND; break;
		case G3D_TEXENV_MODULATE: env = GL_MODULATE; break;
		case G3D_TEXENV_DECAL: env = GL_DECAL; break;
		case G3D_TEXENV_REPLACE: env = GL_REPLACE; break;
		default: env = GL_MODULATE; break;
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, env);

	gluBuild2DMipmaps(
		GL_TEXTURE_2D,
		GL_RGBA,
		image->width,
		image->height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image->pixeldata);
}

void g3d_gl_widget_render(G3DGLWidget *self)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	G3DModel *model = self->priv->model;

	glBindTexture(GL_TEXTURE_2D, 0);

	if(!model || !options)
		return;

	if(self->priv->texture_hash_updated) {
		if(self->priv->texture_hash)
			g_hash_table_foreach(self->priv->texture_hash,
				g3d_gl_widget_render_load_texture, NULL);
		self->priv->texture_hash_updated = FALSE;
	}

	if(options->updated) {
		/* set options */
		if(options->glflags & G3D_FLAG_GL_WIREFRAME) {
			glPolygonMode(GL_FRONT, GL_LINE);
			glPolygonMode(GL_BACK, GL_LINE);
		} else {
			glPolygonMode(GL_FRONT, GL_FILL);
			glPolygonMode(GL_BACK, GL_FILL);
		}

		/* for position of ground plane */
		self->priv->model_min_y = g3d_gl_widget_min_y(model->objects);
	
		g3d_gl_renderer_prepare(self->priv->renderer, model);
		options->updated = TRUE;
	}

	/* shadow */
	g3d_gl_widget_render_shadow_plane(self);

	/* draw model */
	g3d_gl_renderer_draw(self->priv->renderer);

	g3d_gl_widget_draw_coord_system(options);
}

static G3DFloat g3d_gl_widget_min_y(GSList *objects)
{
	G3DFloat min_y = 10.0, tmp_y;
	GSList *oitem;
	G3DObject *object;
	gint32 i;

	for(oitem = objects; oitem != NULL; oitem = oitem->next) {
		object = oitem->data;
		for(i = 0; i < object->vertex_count; i ++)
			if(object->vertex_data[i * 3 + 1] < min_y)
				min_y = object->vertex_data[i * 3 + 1];
		tmp_y = g3d_gl_widget_min_y(object->objects);
		if(tmp_y < min_y)
			min_y = tmp_y;
	}
	return min_y;
}

static void g3d_gl_widget_draw_plane(G3DGLRenderOptions *options)
{
	glBegin(GL_QUADS);
	glNormal3f(0.0, -1.0, 0.0);
#define PLANE_MAX 12
	glVertex3f(-PLANE_MAX, options->min_y - 0.001,  PLANE_MAX);
	glVertex3f( PLANE_MAX, options->min_y - 0.001,  PLANE_MAX);
	glVertex3f( PLANE_MAX, options->min_y - 0.001, -PLANE_MAX);
	glVertex3f(-PLANE_MAX, options->min_y - 0.001, -PLANE_MAX);
#undef PLANE_MAX
	glEnd();
}

static void g3d_gl_widget_setup_floor_stencil(G3DGLRenderOptions *options)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	g3d_gl_widget_draw_plane(options);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

static void g3d_gl_widget_draw_coord_system(G3DGLRenderOptions *options)
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


