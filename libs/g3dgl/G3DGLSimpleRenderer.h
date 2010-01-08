#ifndef _G3DGLSIMPLERENDERER_H
#define _G3DGLSIMPLERENDERER_H

#include <glib-object.h>
#include <g3d/types.h>

#include "g3dgl.h"
#include "G3DGLRenderer.h"

G_BEGIN_DECLS

#define G3D_GL_TYPE_SIMPLE_RENDERER            (g3d_gl_simple_renderer_get_type())
#define G3D_GL_SIMPLE_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	G3D_GL_TYPE_SIMPLE_RENDERER, G3DGLSimpleRenderer))
#define G3D_GL_SIMPLE_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
	G3D_GL_TYPE_SIMPLE_RENDERER, G3DGLSimpleRendererClass))
#define G3D_GL_IS_SIMPLE_RENDERER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	G3D_GL_TYPE_SIMPLE_RENDERER))
#define G3D_GL_IS_SIMPLE_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
	G3D_GL_TYPE_SIMPLE_RENDERER))
#define G3D_GL_SIMPLE_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
	G3D_GL_TYPE_SIMPLE_RENDERER, G3DGLSimpleRendererClass))

typedef struct _G3DGLSimpleRenderer G3DGLSimpleRenderer;
typedef struct _G3DGLSimpleRendererClass G3DGLSimpleRendererClass;
typedef struct _G3DGLSimpleRendererPriv G3DGLSimpleRendererPriv;

struct _G3DGLSimpleRenderer {
	G3DGLRenderer parent_instance;
	G3DGLSimpleRendererPriv *priv;
};

struct _G3DGLSimpleRendererClass {
	G3DGLRendererClass parent_class;
};

GType g3d_gl_simple_renderer_get_type(void) G_GNUC_CONST;

G3DGLRenderer *g3d_gl_simple_renderer_new(G3DGLRenderOptions *options);

G_END_DECLS

#endif /* _G3DGLSIMPLERENDERER_H */
