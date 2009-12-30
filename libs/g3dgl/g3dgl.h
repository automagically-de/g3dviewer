#ifndef _G3DGL_H
#define _G3DGL_H

#include <GL/gl.h>
#include <g3d/types.h>

#define G3D_FLAG_GL_SPECULAR        (1L << 0)
#define G3D_FLAG_GL_SHININESS       (1L << 1)
#define G3D_FLAG_GL_TWOSIDED        (1L << 2)
#define G3D_FLAG_GL_TEXTURES        (1L << 3)
#define G3D_FLAG_GL_COLORS          (1L << 4)
#define G3D_FLAG_GL_POINTS          (1L << 5)
#define G3D_FLAG_GL_COORD_AXES      (1L << 6)
#define G3D_FLAG_GL_SHADOW          (1L << 7)
#define G3D_FLAG_GL_ISOMETRIC       (1L << 8)
#define G3D_FLAG_GL_WIREFRAME       (1L << 9)

typedef struct _G3DGLRenderState G3DGLRenderState;

typedef struct {
	/* to be set by caller */
	gfloat zoom;
	guint32 width;
	guint32 height;
	gfloat aspect;
	gfloat bgcolor[4];
	gfloat quat[4];
	G3DMatrix shadow_matrix[16];
	G3DFloat min_y;
	guint32 norm_count;
	gfloat offx;
	gfloat offy;
	gint32 glflags;
	gboolean updated;
	gboolean initialized;
	gboolean focused;
	/* can be read by caller */
	guint32 avg_msec;
	/* FIXME: split off */
	G3DGLRenderState *state;
} G3DGLRenderOptions;

void g3dgl_matrix_to_gl(G3DMatrix *g3dm, GLfloat glm[4][4]);
void g3dgl_init(void);
G3DFloat g3dgl_min_y(GSList *objects);
void g3dgl_draw_plane(G3DGLRenderOptions *options);
void g3dgl_setup_floor_stencil(G3DGLRenderOptions *options);
void g3dgl_setup_shadow_stencil(G3DGLRenderOptions *options,
	gint32 dlist_shadow);
void g3dgl_draw_objects(G3DGLRenderOptions *options,
	G3DMaterial **prev_material_p, guint32 *prev_texid_p,
	GSList *objects, gfloat min_a, gfloat max_a, gboolean is_shadow);

void g3dgl_set_twoside(gboolean twoside);
void g3dgl_set_textures(gboolean textures);
void g3dgl_setup_view(G3DGLRenderOptions *options);

void g3dgl_load_texture(gpointer key, gpointer value, gpointer data);
void g3dgl_draw(G3DGLRenderOptions *options, G3DModel *model);
guint8 *g3dgl_get_pixels(guint32 width, guint32 height);

#endif /* _G3DGL_H */
