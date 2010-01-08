#include "G3DGLSimpleRenderer.h"

enum {
	G3DGL_DLIST_MODEL,
	G3DGL_DLIST_SHADOW,
	G3DGL_N_DLISTS
};

struct _G3DGLSimpleRendererPriv {
	G3DGLRenderOptions *options;
	gboolean dlists_valid;
	int dlists[G3DGL_N_DLISTS];

	G3DMaterial *prev_material;
	guint32 prev_texid;
};

/* G3DGLRenderer method implementions */

static gboolean g3d_gl_simple_renderer_prepare(G3DGLRenderer *renderer,
	G3DModel *model)
{
	G3DGLSimpleRendererPriv *priv;
	gint32 i;
	gfloat f;

	g_return_val_if_fail(G3D_GL_IS_SIMPLE_RENDERER(renderer), FALSE);

	priv = G3D_GL_SIMPLE_RENDERER(renderer)->priv;

	priv->prev_material = NULL;
	priv->prev_texid = 0;

	if(priv->dlists_valid) {
		for(i = 0; i < G3DGL_N_DLISTS; i ++)
			glDeleteLists(priv->dlists[i], 1);
		priv->dlists_valid = FALSE;
	}

	/* create display lists */
	for(i = 0; i < G3DGL_N_DLISTS; i ++)
		priv->dlists[i] = glGenLists(1);

		/* fill lists */
		glNewList(priv->dlists[G3DGL_DLIST_MODEL], GL_COMPILE);
		for(f = 1.0; f >= 0.0; f -= 0.2)
			g3dgl_draw_objects(priv->options,
				&priv->prev_material,
				&priv->prev_texid,
				model->objects, f, f + 0.2, FALSE);
		glEndList();

		priv->prev_material = NULL;
		priv->prev_texid = 0;

		glNewList(priv->dlists[G3DGL_DLIST_SHADOW], GL_COMPILE);
		g3dgl_draw_objects(priv->options,
			&priv->prev_material,
			&priv->prev_texid,
			model->objects, 0.0, 1.0, TRUE);
		glEndList();

		/* list are valid now */
		priv->dlists_valid = TRUE;

	return TRUE;
}

static gboolean g3d_gl_simple_renderer_draw(G3DGLRenderer *renderer)
{
	G3DGLSimpleRendererPriv *priv;

	g_return_val_if_fail(G3D_GL_IS_SIMPLE_RENDERER(renderer), FALSE);

	priv = G3D_GL_SIMPLE_RENDERER(renderer)->priv;
	g_return_val_if_fail(priv->dlists_valid, FALSE);

	glCallList(priv->dlists[G3DGL_DLIST_MODEL]);
	return TRUE;
}

static gboolean g3d_gl_simple_renderer_draw_shadow(G3DGLRenderer *renderer)
{
	G3DGLSimpleRendererPriv *priv;

	g_return_val_if_fail(G3D_GL_IS_SIMPLE_RENDERER(renderer), FALSE);

	priv = G3D_GL_SIMPLE_RENDERER(renderer)->priv;
	g_return_val_if_fail(priv->dlists_valid, FALSE);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	glCallList(priv->dlists[G3DGL_DLIST_SHADOW]);

	glPopAttrib();

	return TRUE;
}

/* common GType stuff */

static void g3d_gl_simple_renderer_class_init(G3DGLSimpleRendererClass *klass)
{
	g_type_class_add_private(klass, sizeof(G3DGLSimpleRendererPriv));

	G3D_GL_RENDERER_CLASS(klass)->prepare = g3d_gl_simple_renderer_prepare;
	G3D_GL_RENDERER_CLASS(klass)->draw = g3d_gl_simple_renderer_draw;
	G3D_GL_RENDERER_CLASS(klass)->draw_shadow =
		g3d_gl_simple_renderer_draw_shadow;
}

static void g3d_gl_simple_renderer_init(G3DGLSimpleRenderer *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
		G3D_GL_TYPE_SIMPLE_RENDERER, G3DGLSimpleRendererPriv);
}

G_DEFINE_TYPE(G3DGLSimpleRenderer, g3d_gl_simple_renderer, G3D_GL_TYPE_RENDERER)

/* G3DGLSimpleRenderer functions */

G3DGLRenderer *g3d_gl_simple_renderer_new(G3DGLRenderOptions *options)
{
	G3DGLSimpleRenderer *renderer;

	renderer = g_object_new(G3D_GL_TYPE_SIMPLE_RENDERER, NULL);
	renderer->priv->options = options;
	return G3D_GL_RENDERER(renderer);
}

