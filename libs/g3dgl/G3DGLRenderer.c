#include "G3DGLRenderer.h"

static void g3d_gl_renderer_class_init(G3DGLRendererClass *klass)
{
	klass->prepare = NULL;
	klass->draw = NULL;
	klass->draw_shadow = NULL;
}

static void g3d_gl_renderer_init(G3DGLRenderer *self)
{
}

G_DEFINE_TYPE(G3DGLRenderer, g3d_gl_renderer, G_TYPE_OBJECT)

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

