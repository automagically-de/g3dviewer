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

void g3dgl_init(void)
{
	GLenum error;

#if DEBUG > 1
	g_printerr("init OpenGL\n");
#endif

	TRAP_GL_ERROR("gl_init - start");

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

	TRAP_GL_ERROR("gl_init - alpha, blend, depth");

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

	TRAP_GL_ERROR("gl_init - light stuff");

	/* colors and materials */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	TRAP_GL_ERROR("gl_init - color material");

	/* texture stuff */
	glEnable(GL_TEXTURE_2D);

	TRAP_GL_ERROR("gl_init - enable texture");

#ifdef TIMING
	timer = g_timer_new();
#endif
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

static inline void g3dgl_update_material(G3DGLRenderOptions *options,
	G3DMaterial *material)
{
	GLenum facetype;
	GLfloat normspec[4] = { 0.0, 0.0, 0.0, 1.0 };

	g_return_if_fail(material != NULL);

	if(options->glflags & G3D_FLAG_GL_TWOSIDED)
		facetype = GL_FRONT_AND_BACK;
	else
		facetype = GL_FRONT;

	if(options->glflags & G3D_FLAG_GL_COLORS)
		glColor4f(
			material->r,
			material->g,
			material->b,
			material->a);
	else
		glColor4f(0.7, 0.7, 0.7, 1.0);

	return;

	if(options->glflags & G3D_FLAG_GL_SPECULAR)
		glMaterialfv(facetype, GL_SPECULAR, material->specular);
	else
		glMaterialfv(facetype, GL_SPECULAR, normspec);

	if(options->glflags & G3D_FLAG_GL_SHININESS)
		glMaterialf(facetype, GL_SHININESS, material->shininess * 10);
	else
		glMaterialf(facetype, GL_SHININESS, 0.0);
}


static inline void g3dgl_may_end(gint32 ftype)
{
	if(ftype != -1)
		glEnd();
}

static inline void g3dgl_may_begin(gint32 ftype)
{
	if(ftype != -1)
		glBegin(ftype);
}

static inline void g3dgl_draw_face_list(G3DGLRenderOptions *options,
	G3DMaterial **prev_material_p, guint32 *prev_texid_p,
	G3DObject *object, gfloat min_a, gfloat max_a,
	gboolean *init, gboolean is_shadow)
{
	GSList *fitem;
	G3DFace *face;
	G3DVector nx, ny, nz;
	gint32 prev_ftype = -1;
	gint32 index, j, ftype;

	if(*init) {
		*prev_material_p = NULL;
		*prev_texid_p = 0;
		*init = FALSE;
	}

	for(fitem = object->faces; fitem != NULL; fitem = fitem->next) {
		face = fitem->data;
		if(!is_shadow && (*prev_material_p != face->material)) {
			if((face->material->a < min_a) || (face->material->a >= max_a)) {
				return;
			}

			g3dgl_may_end(prev_ftype);
			g3dgl_update_material(options, face->material);
			g3dgl_may_begin(prev_ftype);

			*prev_material_p = face->material;
			*prev_texid_p = 0;
		}

		/* texture stuff */
		if(!is_shadow && (options->glflags & G3D_FLAG_GL_TEXTURES) &&
			(face->flags & G3D_FLAG_FAC_TEXMAP)) {
			/* if texture has changed update to new texture */
			if(face->tex_image) {
				if(face->tex_image->tex_id != *prev_texid_p) {
					*prev_texid_p = face->tex_image->tex_id;

					g3dgl_may_end(prev_ftype);
#if DEBUG > 5
					g_debug("binding texture %d", *prev_texid_p);
#endif
					glBindTexture(GL_TEXTURE_2D, *prev_texid_p);
					g3dgl_may_begin(prev_ftype);
				}
			}
		} /* texture stuff */

		switch(face->vertex_count) {
			case 3: ftype = GL_TRIANGLES; break;
			case 4: ftype = GL_QUADS; break;
			case 2: ftype = GL_LINES; break;
			default: ftype = GL_POLYGON;
#if DEBUG > 0
				g_debug("face vertex count: %d", face->vertex_count);
#endif
				break;
		}
		if(ftype != prev_ftype) {
			g3dgl_may_end(prev_ftype);
			glBegin(ftype);
			prev_ftype = ftype;
		}

		if(!(face->flags & G3D_FLAG_FAC_NORMALS)) {
			face->normals = g_new0(G3DVector, face->vertex_count * 3);

			g3d_face_get_normal(face, object, &nx, &ny, &nz);
			g3d_vector_unify(&nx, &ny, &nz);

			for(j = 0; j < face->vertex_count; j ++) {
				face->normals[j * 3 + 0] = nx;
				face->normals[j * 3 + 1] = ny;
				face->normals[j * 3 + 2] = nz;
			}
			face->flags |= G3D_FLAG_FAC_NORMALS;
		}

		for(j = 0; j < face->vertex_count; j ++) {
			index = face->vertex_indices[j];

			if(!is_shadow && (options->glflags & G3D_FLAG_GL_TEXTURES) &&
				(face->flags & G3D_FLAG_FAC_TEXMAP))
			{
				glTexCoord2f(
					face->tex_vertex_data[j * 2 + 0],
					face->tex_vertex_data[j * 2 + 1]);
			}

			glNormal3f(
				face->normals[j * 3 + 0],
				face->normals[j * 3 + 1],
				face->normals[j * 3 + 2]);

			glVertex3f(
				object->vertex_data[index * 3 + 0],
				object->vertex_data[index * 3 + 1],
				object->vertex_data[index * 3 + 2]);
		}

	} /* face loop */

	g3dgl_may_end(prev_ftype);
}


void g3dgl_draw_objects(G3DGLRenderOptions *options,
	G3DMaterial **prev_material_p, guint32 *prev_texid_p,
	GSList *objects, gfloat min_a, gfloat max_a, gboolean is_shadow)
{
	GSList *olist;
	int i;
	G3DObject *object;
	gboolean dont_render;
	gboolean init = TRUE;

	olist = objects;
	while(olist != NULL)
	{
		object = (G3DObject *)olist->data;
		olist = olist->next;

		dont_render = FALSE;

		/* don't render invisible objects */
		if(object->hide) continue;

		g_return_if_fail(object != NULL);
#if DEBUG > 3
		g_printerr("name: %s {", object->name);
#endif

#if DEBUG > 2
		g_printerr("new object\n");
#endif

		glPushMatrix();

		if(object->transformation)
		{
			glMultMatrixf(object->transformation->matrix);
		}

		g3dgl_draw_face_list(options, prev_material_p, prev_texid_p,
			object, min_a, max_a, &init, is_shadow);

		if(!is_shadow && (options->glflags & G3D_FLAG_GL_POINTS)) {
			glColor4f(0.2, 0.2, 0.2, 1.0);
			glBegin(GL_POINTS);
			for(i = 0; i < object->vertex_count; i ++) {
				glVertex3f(
					object->vertex_data[i * 3 + 0],
					object->vertex_data[i * 3 + 1],
					object->vertex_data[i * 3 + 2]);
			}
			glEnd();
		}

		/* handle sub-objects */
		g3dgl_draw_objects(options, prev_material_p, prev_texid_p,
			object->objects, min_a, max_a, is_shadow);

		glPopMatrix();

	} /* while olist != NULL */
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
