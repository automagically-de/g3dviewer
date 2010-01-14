#include <GL/glu.h>

#include <g3d/face.h>
#include <g3d/vector.h>

#include "G3DGLTessRenderer.h"

struct _G3DGLTessRendererPriv {
	GArray *vertex_array;
	GArray *normal_array;
	GArray *index_array;

	const gchar *last_stype;
	GLenum last_type;
	guint32 last_count;
};

typedef struct {
	G3DFloat v[3];
	G3DFloat n[3];
	guint32 i;
	G3DFace *face;
	G3DGLTessRendererPriv *priv;
} G3DGLTessRendererVertex;

static void g3d_gl_tess_renderer_tess_objects(G3DGLTessRenderer *self,
	GSList *objects, GLUtesselator *tess);
static void g3d_gl_tess_begin_data(GLenum type, G3DGLTessRenderer *self);
static void g3d_gl_tess_end_data(G3DGLTessRenderer *self);
static void g3d_gl_tess_vertex_data(G3DGLTessRendererVertex *v, G3DFace *face);
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

	priv->vertex_array = g_array_new(FALSE, FALSE, sizeof(GLfloat));
	priv->normal_array = g_array_new(FALSE, FALSE, sizeof(GLfloat));
	priv->index_array = g_array_new(FALSE, FALSE, sizeof(GLuint));

	tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN_DATA, g3d_gl_tess_begin_data);
	gluTessCallback(tess, GLU_TESS_END_DATA, g3d_gl_tess_end_data);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, g3d_gl_tess_vertex_data);
	gluTessCallback(tess, GLU_TESS_ERROR_DATA, g3d_gl_tess_error_data);

	g3d_gl_tess_renderer_tess_objects(G3D_GL_TESS_RENDERER(renderer),
		model->objects, tess);

	return TRUE;
}

static gboolean g3d_gl_tess_renderer_draw(G3DGLRenderer *renderer)
{
	G3DGLTessRendererPriv *priv;

	g_return_val_if_fail(G3D_GL_IS_TESS_RENDERER(renderer), FALSE);

	priv = G3D_GL_TESS_RENDERER(renderer)->priv;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0,
		&g_array_index(priv->vertex_array, GLfloat, 0));
	glNormalPointer(GL_FLOAT, 0,
		&g_array_index(priv->normal_array, GLfloat, 0));


	glColor4f(0.7, 0.7, 0.7, 1.0);

#if DEBUG > 3
	g_debug("drawing %d triangles", priv->index_array->len / 3);
#endif

	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawElements(GL_TRIANGLES, priv->index_array->len, GL_UNSIGNED_INT,
		&g_array_index(priv->index_array, GLuint, 0));

	return TRUE;
}

static gboolean g3d_gl_tess_renderer_draw_shadow(G3DGLRenderer *renderer)
{
	G3DGLTessRendererPriv *priv;

	g_return_val_if_fail(G3D_GL_IS_TESS_RENDERER(renderer), FALSE);

	priv = G3D_GL_TESS_RENDERER(renderer)->priv;

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);


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
	gint32 i;
	guint32 index;
	GLdouble *verts;
	G3DGLTessRendererVertex *tessverts;

	for(oitem = objects; oitem != NULL; oitem = oitem->next) {
		object = oitem->data;

		if(object->hide)
			continue;

		g_debug("object: %s", (object->name ? object->name : "unnamed"));

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
				
				tessverts[i].v[0] = verts[i * 3 + 0];
				tessverts[i].v[1] = verts[i * 3 + 1];
				tessverts[i].v[2] = verts[i * 3 + 2];
				tessverts[i].i = i;
				tessverts[i].face = face;
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
			}
			break;
		case GL_TRIANGLE_STRIP:
			for(i = 0; (i + 2) < self->priv->last_count; i ++) {
				if(i % 2) { /* "even" in OpenGL language (2, 4, 6...) */
					tri[0] = baseindex + i + 1;
					tri[0] = baseindex + i + 0;
					tri[0] = baseindex + i + 2;
				} else { /* "odd" in OpenGL language (1, 3, 5...) */
					tri[0] = baseindex + i + 0;
					tri[0] = baseindex + i + 1;
					tri[0] = baseindex + i + 2;
				}
#if DEBUG > 2
				g_debug("[1] tri = (%d, %d, %d)", tri[0], tri[1], tri[2]);
#endif
				g_array_append_vals(self->priv->index_array, tri, 3);
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
			}
			break;
		default:
			g_warning("unhandled polygon type: %d", self->priv->last_type);
			return;
	}
}

static void g3d_gl_tess_vertex_data(G3DGLTessRendererVertex *v, G3DFace *face)
{
#if DEBUG > 2
	g_debug("VERTEX_DATA (%0.2f, %0.2f, %0.2f), %p",
		v->v[0], v->v[1], v->v[2], v->face);
#endif
	g_array_append_vals(v->priv->vertex_array, v->v, 3);
	g_array_append_vals(v->priv->normal_array, v->n, 3);
	v->priv->last_count ++;
}

static void g3d_gl_tess_error_data(GLenum errno, G3DGLTessRenderer *self)
{
	g_warning("ERROR_DATA: errno = %d", errno);
}
