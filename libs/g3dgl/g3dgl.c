#include <GL/gl.h>
#include <GL/glu.h>

#include <g3d/types.h>
#include <g3d/face.h>
#include <g3d/vector.h>
#include <g3d/matrix.h>
#include <g3d/quat.h>

#include "g3dgl.h"

#define TRAP_GL_ERROR(text) \
	error = glGetError(); \
	if(error != GL_NO_ERROR) \
		g_printerr("[gl] %s: E: %d\n", text, error);

#if DEBUG > 0
#define TIMING
#endif

#ifdef TIMING
static GTimer *timer = NULL;
#endif

void g3dgl_matrix_to_gl(G3DMatrix *g3dm, GLfloat glm[4][4])
{
	guint32 i, j;

	for(i = 0; i < 4; i ++)
		for(j = 0; j < 4; j ++)
			glm[i][j] = g3dm[i * 4 + j];
}

void g3d_gl_init(void) {
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
}

void g3dgl_set_twoside(gboolean twoside)
{
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, twoside ? 1 : 0);
	glColorMaterial(
		twoside ? GL_FRONT_AND_BACK : GL_FRONT,
		GL_AMBIENT_AND_DIFFUSE);
}

void g3dgl_set_textures(gboolean textures)
{
	if(textures)
		glEnable(GL_TEXTURE_2D);
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}

G3DFloat g3dgl_min_y(GSList *objects)
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
		tmp_y = g3dgl_min_y(object->objects);
		if(tmp_y < min_y)
			min_y = tmp_y;
	}
	return min_y;
}

void g3dgl_draw_plane(G3DGLRenderOptions *options)
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

void g3dgl_setup_floor_stencil(G3DGLRenderOptions *options)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	g3dgl_draw_plane(options);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

/* GHFunc */
void g3dgl_load_texture(gpointer key, gpointer value, gpointer data)
{
	G3DImage *image = (G3DImage *)value;
	gint32 env;
	GLenum error;

	TRAP_GL_ERROR("gl_load_texture - start");

#if 0
	/* predefined - update object->_tex_images else... */
	glGenTextures(1, &(image->tex_id));
#endif

#if DEBUG > 0
	g_print("gl: loading texture '%s' (%dx%dx%d) - id %d\n",
		image->name ? image->name : "(null)",
		image->width, image->height, image->depth,
		image->tex_id);
#endif

	glBindTexture(GL_TEXTURE_2D, image->tex_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_NEAREST);

	TRAP_GL_ERROR("gl_load_texture - bind, param");

	switch(image->tex_env)
	{
		case G3D_TEXENV_BLEND: env = GL_BLEND; break;
		case G3D_TEXENV_MODULATE: env = GL_MODULATE; break;
		case G3D_TEXENV_DECAL: env = GL_DECAL; break;
		case G3D_TEXENV_REPLACE: env = GL_REPLACE; break;
		default: env = GL_MODULATE; break;
	}
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, env);
	TRAP_GL_ERROR("gl_load_texture - texenv");

	gluBuild2DMipmaps(
		GL_TEXTURE_2D,
		GL_RGBA,
		image->width,
		image->height,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image->pixeldata);
	TRAP_GL_ERROR("gl_load_texture - mipmaps");
}

void g3dgl_draw_coord_system(G3DGLRenderOptions *options)
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

guint8 *g3dgl_get_pixels(guint32 width, guint32 height)
{
	guint8 *pixels;

	pixels = g_new(guint8, width * height * 4);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	return pixels;
}
