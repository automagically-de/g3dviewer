#include <g3d/face.h>
#include <g3d/vector.h>

#include "G3DGLSimpleRenderer.h"

enum {
	G3DGL_DLIST_MODEL,
	G3DGL_DLIST_SHADOW,
	G3DGL_N_DLISTS
};

struct _G3DGLSimpleRendererPriv {
	gboolean dlists_valid;
	int dlists[G3DGL_N_DLISTS];

	G3DMaterial *prev_material;
	guint32 prev_texid;
};

static void g3d_gl_draw_objects(G3DGLRenderOptions *options,
	G3DMaterial **prev_material_p, guint32 *prev_texid_p,
	GSList *objects, gfloat min_a, gfloat max_a, gboolean is_shadow);
static inline void g3d_gl_update_material(G3DGLRenderOptions *options,
	G3DMaterial *material);

/* G3DGLRenderer method implementions */

static gboolean g3d_gl_simple_renderer_prepare(G3DGLRenderer *renderer,
	G3DModel *model)
{
	G3DGLRendererPriv *rpriv;
	G3DGLSimpleRendererPriv *priv;
	gint32 i;
	gfloat f;

	g_return_val_if_fail(G3D_GL_IS_SIMPLE_RENDERER(renderer), FALSE);

	priv = G3D_GL_SIMPLE_RENDERER(renderer)->priv;
	rpriv = G3D_GL_RENDERER(renderer)->priv;

	g_return_val_if_fail(priv != NULL, FALSE);
	g_return_val_if_fail(rpriv != NULL, FALSE);

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
			g3d_gl_draw_objects(rpriv->options,
				&priv->prev_material,
				&priv->prev_texid,
				model->objects, f, f + 0.2, FALSE);
		glEndList();

		priv->prev_material = NULL;
		priv->prev_texid = 0;

		glNewList(priv->dlists[G3DGL_DLIST_SHADOW], GL_COMPILE);
		g3d_gl_draw_objects(rpriv->options,
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
	g_return_val_if_fail(renderer != NULL, NULL);
	G3D_GL_RENDERER(renderer)->priv->options = options;
	return G3D_GL_RENDERER(renderer);
}

/* static stuff */

static inline void g3d_gl_update_material(G3DGLRenderOptions *options,
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

static inline void g3d_gl_may_end(gint32 ftype)
{
	if(ftype != -1)
		glEnd();
}

static inline void g3d_gl_may_begin(gint32 ftype)
{
	if(ftype != -1)
		glBegin(ftype);
}

static inline void g3d_gl_draw_face_list(G3DGLRenderOptions *options,
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

			g3d_gl_may_end(prev_ftype);
			g3d_gl_update_material(options, face->material);
			g3d_gl_may_begin(prev_ftype);

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

					g3d_gl_may_end(prev_ftype);
#if DEBUG > 5
					g_debug("binding texture %d", *prev_texid_p);
#endif
					glBindTexture(GL_TEXTURE_2D, *prev_texid_p);
					g3d_gl_may_begin(prev_ftype);
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
			g3d_gl_may_end(prev_ftype);
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

	g3d_gl_may_end(prev_ftype);
}

static void g3d_gl_draw_objects(G3DGLRenderOptions *options,
	G3DMaterial **prev_material_p, guint32 *prev_texid_p,
	GSList *objects, gfloat min_a, gfloat max_a, gboolean is_shadow)
{
	GSList *olist;
	int i;
	G3DObject *object;
	gboolean dont_render;
	gboolean init = TRUE;

	olist = objects;
	while(olist != NULL) {
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
			glMultMatrixf(object->transformation->matrix);

		g3d_gl_draw_face_list(options, prev_material_p, prev_texid_p,
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
		g3d_gl_draw_objects(options, prev_material_p, prev_texid_p,
			object->objects, min_a, max_a, is_shadow);

		glPopMatrix();

	} /* while olist != NULL */
}


