#ifndef _G3DGLTESSRENDERER_H
#define _G3DGLTESSRENDERER_H

#include <glib-object.h>
#include <g3d/types.h>

#include "g3dgl.h"
#include "G3DGLRenderer.h"

G_BEGIN_DECLS

#define G3D_GL_TYPE_TESS_RENDERER            (g3d_gl_tess_renderer_get_type())
#define G3D_GL_TESS_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	G3D_GL_TYPE_TESS_RENDERER, G3DGLTessRenderer))
#define G3D_GL_TESS_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
	G3D_GL_TYPE_TESS_RENDERER, G3DGLTessRendererClass))
#define G3D_GL_IS_TESS_RENDERER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	G3D_GL_TYPE_TESS_RENDERER))
#define G3D_GL_IS_TESS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
	G3D_GL_TYPE_TESS_RENDERER))
#define G3D_GL_TESS_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
	G3D_GL_TYPE_TESS_RENDERER, G3DGLTessRendererClass))

typedef struct _G3DGLTessRenderer G3DGLTessRenderer;
typedef struct _G3DGLTessRendererClass G3DGLTessRendererClass;
typedef struct _G3DGLTessRendererPriv G3DGLTessRendererPriv;

struct _G3DGLTessRenderer {
	G3DGLRenderer parent_instance;
	G3DGLTessRendererPriv *priv;
};

struct _G3DGLTessRendererClass {
	G3DGLRendererClass parent_class;
};

GType g3d_gl_tess_renderer_get_type(void) G_GNUC_CONST;

G3DGLRenderer *g3d_gl_tess_renderer_new(G3DGLRenderOptions *options);

G_END_DECLS

#endif /* _G3DGLTESSRENDERER_H */
