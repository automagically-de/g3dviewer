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
} G3DGLRenderOptions;

void g3d_gl_init(void);
void g3d_gl_matrix_to_gl(G3DMatrix *g3dm, GLfloat glm[4][4]);
void g3d_gl_set_twoside(gboolean twoside);
void g3d_gl_set_textures(gboolean textures);
void g3d_gl_load_texture(gpointer key, gpointer value, gpointer data);
guint8 *g3di_gl_get_pixels(guint32 width, guint32 height);

#endif /* _G3DGL_H */
