/* to be directly included */
/* FIXME: bad style */

#define TRAP_GL_ERROR(text) \
	error = glGetError(); \
	if(error != GL_NO_ERROR) \
		g_printerr("[gl] %s: E: %d\n", text, error);


static inline void gl_update_material(G3DGLRenderOptions *options,
	G3DMaterial *material)
{
	GLenum facetype;
	GLfloat normspec[4] = { 0.0, 0.0, 0.0, 1.0 };

	g_return_if_fail(material != NULL);

	if(options->glflags & G3D_FLAG_GL_ALLTWOSIDE)
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


static inline void gl_may_end(gint32 ftype)
{
	if(ftype != -1)
		glEnd();
}

static inline void gl_may_begin(gint32 ftype)
{
	if(ftype != -1)
		glBegin(ftype);
}

static inline void gl_draw_face_list(G3DGLRenderOptions *options,
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

			gl_may_end(prev_ftype);
			gl_update_material(options, face->material);
			gl_may_begin(prev_ftype);

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

					gl_may_end(prev_ftype);
#if DEBUG > 5
					g_debug("binding texture %d", *prev_texid_p);
#endif
					glBindTexture(GL_TEXTURE_2D, *prev_texid_p);
					gl_may_begin(prev_ftype);
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
			gl_may_end(prev_ftype);
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

	gl_may_end(prev_ftype);
}


static inline void gl_draw_objects(G3DGLRenderOptions *options,
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

		gl_draw_face_list(options, prev_material_p, prev_texid_p,
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
		gl_draw_objects(options, prev_material_p, prev_texid_p,
			object->objects, min_a, max_a, is_shadow);

		glPopMatrix();

	} /* while olist != NULL */
}

static G3DFloat gl_min_y(GSList *objects)
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
		tmp_y = gl_min_y(object->objects);
		if(tmp_y < min_y)
			min_y = tmp_y;
	}
	return min_y;
}

static void gl_draw_plane(G3DGLRenderOptions *options)
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

static void gl_setup_floor_stencil(G3DGLRenderOptions *options)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	gl_draw_plane(options);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

static void gl_setup_shadow_stencil(G3DGLRenderOptions *options,
	gint32 dlist_shadow)
{
	glClear(GL_STENCIL_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	glCallList(dlist_shadow);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glStencilFunc(GL_EQUAL, 1, 0xffffffff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

static inline void matrix_g3d_to_gl(G3DMatrix *g3dm, GLfloat glm[4][4])
{
	guint32 i, j;

	for(i = 0; i < 4; i ++)
		for(j = 0; j < 4; j ++)
			glm[i][j] = g3dm[i * 4 + j];
}

