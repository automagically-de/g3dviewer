#ifndef _G3DGLRENDERER_H
#define _G3DGLRENDERER_H

#include <glib-object.h>
#include <g3d/types.h>

#include "g3dgl.h"

G_BEGIN_DECLS

#define G3D_GL_TYPE_RENDERER            (g3d_gl_renderer_get_type())
#define G3D_GL_RENDERER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	G3D_GL_TYPE_RENDERER, G3DGLRenderer))
#define G3D_GL_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
	G3D_GL_TYPE_RENDERER, G3DGLRendererClass))
#define G3D_GL_IS_RENDERER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	G3D_GL_TYPE_RENDERER))
#define G3D_GL_IS_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
	G3D_GL_TYPE_RENDERER))
#define G3D_GL_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
	G3D_GL_TYPE_RENDERER, G3DGLRendererClass))

typedef struct _G3DGLRenderer G3DGLRenderer;
typedef struct _G3DGLRendererClass G3DGLRendererClass;
typedef struct _G3DGLRendererPriv G3DGLRendererPriv;

struct _G3DGLRenderer {
	GObject parent_instance;
	G3DGLRendererPriv *priv;
};

struct _G3DGLRendererClass {
	GObjectClass parent_class;

	gboolean (* clear)(G3DGLRenderer *);
	gboolean (* setup_view)(G3DGLRenderer *);
	gboolean (* prepare)(G3DGLRenderer *, G3DModel *);
	gboolean (* draw)(G3DGLRenderer *);
	gboolean (* draw_shadow)(G3DGLRenderer *);
};

struct _G3DGLRendererPriv {
	G3DGLRenderOptions *options;
};

GType g3d_gl_renderer_get_type(void) G_GNUC_CONST;

/* GtkRenderer *g3d_gl_renderer_new(void); */

gboolean g3d_gl_renderer_clear(G3DGLRenderer *r);
gboolean g3d_gl_renderer_setup_view(G3DGLRenderer *r);
gboolean g3d_gl_renderer_prepare(G3DGLRenderer *r, G3DModel *model);
gboolean g3d_gl_renderer_draw(G3DGLRenderer *r);
gboolean g3d_gl_renderer_draw_shadow(G3DGLRenderer *r);

G_END_DECLS

#endif /* _G3DGLRENDERER_H */
