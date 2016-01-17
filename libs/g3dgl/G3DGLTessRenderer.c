#include <GL/glu.h>

#include <g3d/face.h>
#include <g3d/vector.h>

#include "G3DGLTessRenderer.h"

#define TRAP_GL_ERROR(text) \
{ \
	GLenum error; \
	error = glGetError(); \
	if(error != GL_NO_ERROR) \
		g_printerr("[gl] %s: E: %d\n", text, error); \
}

typedef struct {
	guint32 start;
	guint32 count;
	gboolean textured;
	gint32 texid;
} G3DGLTessRendererTexChunk;

struct _G3DGLTessRendererPriv {
	GArray *vertex_array;
	GArray *normal_array;
	GArray *color_array;
	GArray *index_array;
	GArray *texco_array;

	G3DGLTessRendererTexChunk *current_chunk;
	GSList *chunks;

	const gchar *last_stype;
	GLenum last_type;
	guint32 last_count;

	GSList *combined_vertices;
};

typedef struct {
	G3DFloat v[3];
	G3DFloat n[3];
	G3DFloat uv[2];
	G3DVector color[4];
	guint32 i;
	G3DFace *face;
	gboolean textured;
	gint32 texid;
	G3DGLTessRendererPriv *priv;
} G3DGLTessRendererVertex;

static void g3d_gl_tess_renderer_tess_objects(G3DGLTessRenderer *self,
	GSList *objects, GLUtesselator *tess);
static void g3d_gl_tess_renderer_cleanup(G3DGLTessRenderer *self);
static void g3d_gl_tess_begin_data(GLenum type, G3DGLTessRenderer *self);
static void g3d_gl_tess_end_data(G3DGLTessRenderer *self);
static void g3d_gl_tess_vertex(G3DGLTessRendererVertex *v);
static void g3d_gl_tess_combine_data(
	GLdouble coords[3],
	G3DGLTessRendererVertex *vertex_data[4],
	GLfloat w[4],
	G3DGLTessRendererVertex **vertex_out,
	G3DGLTessRenderer *self);
static void g3d_gl_tess_error_data(GLenum errno, G3DGLTessRenderer *self);

/* G3DGLRenderer method implementions */

static gboolean g3d_gl_tess_renderer_prepare(G3DGLRenderer *renderer,
	G3DModel *model)
{
	G3DGLRendererPriv *rpriv;
	G3DGLTessRendererPriv *priv;
	GLUtesselator *tess;

	g_return_val_if_fail(G3D_GL_IS_TESS_RENDERER(renderer), FALSE);

	priv = G3D_GL_TESS_RENDERER(renderer)->priv;
	rpriv = G3D_GL_RENDERER(renderer)->priv;

	g_return_val_if_fail(priv != NULL, FALSE);
	g_return_val_if_fail(rpriv != NULL, FALSE);

	g3d_gl_tess_renderer_cleanup(G3D_GL_TESS_RENDERER(renderer));

	priv->vertex_array = g_array_new(FALSE, FALSE, sizeof(GLfloat));
	priv->normal_array = g_array_new(FALSE, FALSE, sizeof(GLfloat));
	priv->color_array = g_array_new(FALSE, FALSE, sizeof(GLfloat));
	priv->texco_array = g_array_new(FALSE, FALSE, sizeof(GLfloat));
	priv->index_array = g_array_new(FALSE, FALSE, sizeof(GLuint));

	tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN_DATA, g3d_gl_tess_begin_data);
	gluTessCallback(tess, GLU_TESS_END_DATA, g3d_gl_tess_end_data);
	gluTessCallback(tess, GLU_TESS_VERTEX, g3d_gl_tess_vertex);
	gluTessCallback(tess, GLU_TESS_COMBINE_DATA, g3d_gl_tess_combine_data);
	gluTessCallback(tess, GLU_TESS_ERROR_DATA, g3d_gl_tess_error_data);

	g3d_gl_tess_renderer_tess_objects(G3D_GL_TESS_RENDERER(renderer),
		model->objects, tess);

	gluDeleteTess(tess);

	return TRUE;
}

static gboolean g3d_gl_tess_renderer_draw(G3DGLRenderer *renderer)
{
	G3DGLRendererPriv *rpriv;
	G3DGLTessRendererPriv *priv;
	G3DGLTessRendererTexChunk *chunk;
	GSList *citem;
	guint32 n;

	g_return_val_if_fail(G3D_GL_IS_TESS_RENDERER(renderer), FALSE);

	priv = G3D_GL_TESS_RENDERER(renderer)->priv;
	rpriv = renderer->priv;

	glVertexPointer(3, GL_FLOAT, 0,
		&g_array_index(priv->vertex_array, GLfloat, 0));
	glNormalPointer(GL_FLOAT, 0,
		&g_array_index(priv->normal_array, GLfloat, 0));
	glColorPointer(4, GL_FLOAT, 0,
		&g_array_index(priv->color_array, GLfloat, 0));
	glTexCoordPointer(2, GL_FLOAT, 0,
		&g_array_index(priv->texco_array, GLfloat, 0));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	if(rpriv->options->glflags & G3D_FLAG_GL_COLORS)
		glEnableClientState(GL_COLOR_ARRAY);

	if(rpriv->options->glflags & G3D_FLAG_GL_TEXTURES)
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#if DEBUG > 3
	g_debug("drawing %d triangles", priv->index_array->len / 3);
#endif

	for(n = 0, citem = priv->chunks; citem != NULL; n ++, citem = citem->next) {
		chunk = citem->data;
#if DEBUG > 2
		g_debug("chunk #%d: %d - %d (%d)", n,
			chunk->start, chunk->count, chunk->texid);
#endif

		if(chunk->textured &&
			(rpriv->options->glflags & G3D_FLAG_GL_TEXTURES)) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, chunk->texid);
		} else {
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glDrawElements(GL_TRIANGLES,
			chunk->count * 3,
			GL_UNSIGNED_INT,
			&g_array_index(priv->index_array, GLuint, chunk->start * 3));
		TRAP_GL_ERROR("glDrawElement");
	}
	return TRUE;
}

static gboolean g3d_gl_tess_renderer_draw_shadow(G3DGLRenderer *renderer)
{
	G3DGLTessRendererPriv *priv;
	G3DGLTessRendererTexChunk *chunk;
	GSList *citem;
	guint32 n;

	g_return_val_if_fail(G3D_GL_IS_TESS_RENDERER(renderer), FALSE);

	priv = G3D_GL_TESS_RENDERER(renderer)->priv;

	glVertexPointer(3, GL_FLOAT, 0,
		&g_array_index(priv->vertex_array, GLfloat, 0));
	glNormalPointer(GL_FLOAT, 0,
		&g_array_index(priv->normal_array, GLfloat, 0));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	for(n = 0, citem = priv->chunks; citem != NULL; n ++, citem = citem->next) {
		chunk = citem->data;
#if DEBUG > 2
		g_debug("chunk #%d: %d - %d (%d)", n,
			chunk->start, chunk->count, chunk->texid);
#endif
		glDrawElements(GL_TRIANGLES,
			chunk->count * 3,
			GL_UNSIGNED_INT,
			&g_array_index(priv->index_array, GLuint, chunk->start * 3));
	}

	glPopAttrib();

	return TRUE;
}

/* common GType stuff */

static void g3d_gl_tess_renderer_class_init(G3DGLTessRendererClass *klass)
{
	g_type_class_add_private(klass, sizeof(G3DGLTessRendererPriv));

	G3D_GL_RENDERER_CLASS(klass)->prepare = g3d_gl_tess_renderer_prepare;
	G3D_GL_RENDERER_CLASS(klass)->draw = g3d_gl_tess_renderer_draw;
	G3D_GL_RENDERER_CLASS(klass)->draw_shadow =
		g3d_gl_tess_renderer_draw_shadow;
}

static void g3d_gl_tess_renderer_init(G3DGLTessRenderer *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,
		G3D_GL_TYPE_TESS_RENDERER, G3DGLTessRendererPriv);
}

G_DEFINE_TYPE(G3DGLTessRenderer, g3d_gl_tess_renderer, G3D_GL_TYPE_RENDERER)

G3DGLRenderer *g3d_gl_tess_renderer_new(G3DGLRenderOptions *options)
{
	G3DGLTessRenderer *renderer;

	renderer = g_object_new(G3D_GL_TYPE_TESS_RENDERER, NULL);
	g_return_val_if_fail(renderer != NULL, NULL);
	G3D_GL_RENDERER(renderer)->priv->options = options;
	return G3D_GL_RENDERER(renderer);
}

/* G3DGLTessRenderer functions */

static void g3d_gl_tess_renderer_tess_objects(G3DGLTessRenderer *self,
	GSList *objects, GLUtesselator *tess)
{
	GSList *oitem, *fitem;
	G3DObject *object;
	G3DFace *face;
	G3DVector normal[3];
	gint32 i, j;
	guint32 index;
	GLdouble *verts;
	G3DGLTessRendererVertex *tessverts;

	for(oitem = objects; oitem != NULL; oitem = oitem->next) {
		object = oitem->data;

		if(object->hide)
			continue;

#if DEBUG > 1
		g_debug("object: %s", (object->name ? object->name : "unnamed"));
#endif

		for(fitem = object->faces; fitem != NULL; fitem = fitem->next) {
			face = fitem->data;

			verts = g_new0(GLdouble, face->vertex_count * 3);
			tessverts = g_new0(G3DGLTessRendererVertex, face->vertex_count);

			gluTessBeginPolygon(tess, self);
			gluTessBeginContour(tess);

			if(!(face->flags & G3D_FLAG_FAC_NORMALS)) {
				g3d_face_get_normal(face, object,
					normal, normal + 1, normal + 2);
				g3d_vector_unify(normal, normal + 1, normal + 2);
			}

			for(i = 0; i < face->vertex_count; i ++) {
				index = face->vertex_indices[i];
				verts[i * 3 + 0] = object->vertex_data[index * 3 + 0];
				verts[i * 3 + 1] = object->vertex_data[index * 3 + 1];
				verts[i * 3 + 2] = object->vertex_data[index * 3 + 2];

				if (object->vertex_color_data) {
					for (j = 0; j < 4; j ++)
						tessverts[i].color[j] =
							object->vertex_color_data[index * 4 + j];
				}
				else {
					tessverts[i].color[0] = face->material->r;
					tessverts[i].color[1] = face->material->g;
					tessverts[i].color[2] = face->material->b;
					tessverts[i].color[3] = face->material->a;
				}

				tessverts[i].v[0] = verts[i * 3 + 0];
				tessverts[i].v[1] = verts[i * 3 + 1];
				tessverts[i].v[2] = verts[i * 3 + 2];
				tessverts[i].i = i;
				tessverts[i].face = face;
				tessverts[i].textured = (face->flags & G3D_FLAG_FAC_TEXMAP);
				if(tessverts[i].textured)
					tessverts[i].texid = face->tex_image->tex_id;
				tessverts[i].priv = self->priv;

				if(face->flags & G3D_FLAG_FAC_NORMALS) {
					tessverts[i].n[0] = face->normals[i * 3 + 0];
					tessverts[i].n[1] = face->normals[i * 3 + 1];
					tessverts[i].n[2] = face->normals[i * 3 + 2];
				} else {
					tessverts[i].n[0] = normal[0];
					tessverts[i].n[1] = normal[1];
					tessverts[i].n[2] = normal[2];
				}
				if(face->flags & G3D_FLAG_FAC_TEXMAP) {
					tessverts[i].uv[0] = face->tex_vertex_data[i * 2 + 0];
					tessverts[i].uv[1] = face->tex_vertex_data[i * 2 + 1];
				}

				gluTessVertex(tess, &(verts[i * 3]), &(tessverts[i]));
			}
		
			gluTessEndContour(tess);
			gluTessEndPolygon(tess);

			g_free(verts);
			g_free(tessverts);
		}

		g3d_gl_tess_renderer_tess_objects(self, object->objects, tess);
	}
}

static void g3d_gl_tess_renderer_cleanup(G3DGLTessRenderer *self)
{
	G3DGLTessRendererTexChunk *chunk;
	GSList *citem, *next;

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	if(self->priv->vertex_array) {
		g_array_free(self->priv->vertex_array, TRUE);	
		g_array_free(self->priv->normal_array, TRUE);	
		g_array_free(self->priv->index_array, TRUE);	
		g_array_free(self->priv->color_array, TRUE);	
		g_array_free(self->priv->texco_array, TRUE);	
		self->priv->vertex_array = NULL;
		self->priv->normal_array = NULL;
		self->priv->index_array = NULL;
		self->priv->color_array = NULL;
		self->priv->texco_array = NULL;
	}

	citem = self->priv->chunks;
	while(citem != NULL) {
		chunk = citem->data;
		g_free(chunk);
		next = citem->next;
		g_slist_free_1(citem);
		citem = next;
	}
	self->priv->chunks = NULL;
	self->priv->current_chunk = NULL;

	citem = self->priv->combined_vertices;
	while(citem != NULL) {
		chunk = citem->data;
		g_free(chunk);
		next = citem->next;
		g_slist_free_1(citem);
		citem = next;
	}
	self->priv->combined_vertices = NULL;
}

static void g3d_gl_tess_begin_data(GLenum type, G3DGLTessRenderer *self)
{
	self->priv->last_type = type;
	self->priv->last_count = 0;
	switch(type) {
		case GL_TRIANGLES: self->priv->last_stype = "TRIANGLES"; break;
		case GL_TRIANGLE_FAN: self->priv->last_stype = "TRIANGLE FAN"; break;
		case GL_TRIANGLE_STRIP: self->priv->last_stype = "TRIANGLE STRIP"; break;
		default: self->priv->last_stype = "UNKNOWN";
	}
#if DEBUG > 2
	g_debug("BEGIN_DATA: type = %s (%d)", self->priv->last_stype, type);
#endif
}

static void g3d_gl_tess_end_data(G3DGLTessRenderer *self)
{
	gint32 i;
	guint32 baseindex;
	GLuint tri[3];

#if DEBUG > 2
	g_debug("END_DATA (%s, %d vertices)", self->priv->last_stype,
		self->priv->last_count);
#endif
	g_return_if_fail(
		self->priv->last_count <= (self->priv->vertex_array->len / 3));
	
	baseindex = (self->priv->vertex_array->len / 3) - self->priv->last_count;
	switch(self->priv->last_type) {
		case GL_TRIANGLES:
			for(i = 0; (i + 2) < self->priv->last_count; i += 3) {
				tri[0] = baseindex + i + 0;
				tri[1] = baseindex + i + 1;
				tri[2] = baseindex + i + 2;
#if DEBUG > 2
				g_debug("[0] tri = (%d, %d, %d)", tri[0], tri[1], tri[2]);
#endif
				g_array_append_vals(self->priv->index_array, tri, 3);
				self->priv->current_chunk->count ++;
			}
			break;
		case GL_TRIANGLE_STRIP:
			for(i = 0; (i + 2) < self->priv->last_count; i ++) {
				if(i % 2) { /* "even" in OpenGL language (2, 4, 6...) */
					tri[0] = baseindex + i + 1;
					tri[1] = baseindex + i + 0;
					tri[2] = baseindex + i + 2;
				} else { /* "odd" in OpenGL language (1, 3, 5...) */
					tri[0] = baseindex + i + 0;
					tri[1] = baseindex + i + 1;
					tri[2] = baseindex + i + 2;
				}
#if DEBUG > 2
				g_debug("[1] tri = (%d, %d, %d)", tri[0], tri[1], tri[2]);
#endif
				g_array_append_vals(self->priv->index_array, tri, 3);
				self->priv->current_chunk->count ++;
			}
			break;
		case GL_TRIANGLE_FAN:
			for(i = 0; (i + 2) < self->priv->last_count; i ++) {
				tri[0] = baseindex;
				tri[1] = baseindex + i + 1;
				tri[2] = baseindex + i + 2;
#if DEBUG > 2
				g_debug("[2] tri = (%d, %d, %d)", tri[0], tri[1], tri[2]);
#endif
				g_array_append_vals(self->priv->index_array, tri, 3);
				self->priv->current_chunk->count ++;
			}
			break;
		default:
			g_warning("unhandled polygon type: %d", self->priv->last_type);
			return;
	}
}

static void g3d_gl_tess_update_material(G3DGLTessRendererPriv *priv,
	gboolean textured, gint32 texid)
{
	G3DGLTessRendererTexChunk *chunk;

	g_return_if_fail(priv != NULL);

	if((priv->current_chunk == NULL) ||
		(priv->current_chunk->textured != textured) ||
		(priv->current_chunk->texid != texid))
	{
		chunk = g_new0(G3DGLTessRendererTexChunk, 1);
		chunk->textured = textured;
		chunk->texid = texid;
		chunk->start = priv->index_array->len / 3;
		priv->current_chunk = chunk;
		priv->chunks = g_slist_append(priv->chunks, chunk);
	}
}

static void g3d_gl_tess_vertex(G3DGLTessRendererVertex *v)
{
	g3d_gl_tess_update_material(v->priv, v->textured, v->texid);

#if DEBUG > 2
	g_debug("VERTEX_DATA (%0.2f, %0.2f, %0.2f), %p",
		v->v[0], v->v[1], v->v[2], v->face);
#endif
	g_array_append_vals(v->priv->vertex_array, v->v, 3);
	g_array_append_vals(v->priv->normal_array, v->n, 3);
	g_array_append_vals(v->priv->color_array, v->color, 4);
	g_array_append_vals(v->priv->texco_array, v->uv, 2);
	v->priv->last_count ++;
}

static void g3d_gl_tess_combine_data(
	GLdouble coords[3],
	G3DGLTessRendererVertex *vertex_data[4],
	GLfloat w[4],
	G3DGLTessRendererVertex **vertex_out,
	G3DGLTessRenderer *self) {

	G3DGLTessRendererVertex *out = g_new0(G3DGLTessRendererVertex, 1);
	out->priv = self->priv;
	guint32 i, j;

	for (i = 0; i < 3; i ++)
		out->v[i] = coords[i];

	for (j = 0; j < 4; j ++) {
		if (w[j] == 0.0)
			break;
		for (i = 0; i < 4; i ++)
			out->color[i] += vertex_data[j]->color[i] * w[j];
		for (i = 0; i < 3; i ++)
			out->n[i] += vertex_data[j]->n[i] * w[j];
		for (i = 0; i < 2; i ++)
			out->uv[i] += vertex_data[j]->uv[i] * w[j];
	}

	/* track pointer to clean it up later */
	self->priv->combined_vertices = g_slist_prepend(self->priv->combined_vertices, out);

	*vertex_out = out;
}

static void g3d_gl_tess_error_data(GLenum errno, G3DGLTessRenderer *self)
{
	g_warning("ERROR_DATA: %s (%d)", gluErrorString(errno), errno);
}
