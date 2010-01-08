#include <GL/gl.h>
#include <GL/glu.h>

#include <g3d/matrix.h>
#include <g3d/quat.h>

#include "G3DGLRenderer.h"

#include "g3dgl.h"

static gboolean g3d_gl_renderer_clear_impl(G3DGLRenderer *self);
static gboolean g3d_gl_renderer_setup_view_impl(G3DGLRenderer *self);

static void g3d_gl_renderer_class_init(G3DGLRendererClass *klass)
{
	g_type_class_add_private(klass, sizeof(G3DGLRendererPriv));

	klass->clear = g3d_gl_renderer_clear_impl;
	klass->setup_view = g3d_gl_renderer_setup_view_impl;
	klass->prepare = NULL;
	klass->draw = NULL;
	klass->draw_shadow = NULL;
}

static void g3d_gl_renderer_init(G3DGLRenderer *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
		G3D_GL_TYPE_RENDERER, G3DGLRendererPriv);
}

G_DEFINE_TYPE(G3DGLRenderer, g3d_gl_renderer, G_TYPE_OBJECT)

static gboolean g3d_gl_renderer_clear_impl(G3DGLRenderer *self)
{
	G3DGLRenderOptions *options = self->priv->options;

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

	return TRUE;
}

static gboolean g3d_gl_renderer_setup_view_impl(G3DGLRenderer *self)
{
	G3DFloat w, h;
	GLfloat m[4][4];
	G3DMatrix *g3dm;
	G3DGLRenderOptions *options = self->priv->options;

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
	g3d_gl_matrix_to_gl(g3dm, m);

	g3d_matrix_free(g3dm);
	glMultMatrixf(&m[0][0]);

	return TRUE;
}

gboolean g3d_gl_renderer_clear(G3DGLRenderer *self)
{
	g_return_val_if_fail(G3D_GL_IS_RENDERER(self), FALSE);
	g_return_val_if_fail(G3D_GL_RENDERER_GET_CLASS(self)->draw, FALSE);
	return G3D_GL_RENDERER_GET_CLASS(self)->clear(self);
}

gboolean g3d_gl_renderer_setup_view(G3DGLRenderer *self)
{
	g_return_val_if_fail(G3D_GL_IS_RENDERER(self), FALSE);
	g_return_val_if_fail(G3D_GL_RENDERER_GET_CLASS(self)->draw, FALSE);
	return G3D_GL_RENDERER_GET_CLASS(self)->setup_view(self);
}

gboolean g3d_gl_renderer_prepare(G3DGLRenderer *self, G3DModel *model)
{
	g_return_val_if_fail(G3D_GL_IS_RENDERER(self), FALSE);
	g_return_val_if_fail(G3D_GL_RENDERER_GET_CLASS(self)->prepare, FALSE);
	return G3D_GL_RENDERER_GET_CLASS(self)->prepare(self, model);
}

gboolean g3d_gl_renderer_draw(G3DGLRenderer *self)
{
	g_return_val_if_fail(G3D_GL_IS_RENDERER(self), FALSE);
	g_return_val_if_fail(G3D_GL_RENDERER_GET_CLASS(self)->draw, FALSE);
	return G3D_GL_RENDERER_GET_CLASS(self)->draw(self);
}

gboolean g3d_gl_renderer_draw_shadow(G3DGLRenderer *self)
{
	g_return_val_if_fail(G3D_GL_IS_RENDERER(self), FALSE);
	g_return_val_if_fail(G3D_GL_RENDERER_GET_CLASS(self)->draw_shadow, FALSE);
	return G3D_GL_RENDERER_GET_CLASS(self)->draw_shadow(self);
}

