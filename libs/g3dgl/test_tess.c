#include <stdlib.h>

#include <glib-object.h>

#include <g3d/primitive.h>
#include <g3d/material.h>

#include <G3DGLTessRenderer.h>

int main(int argc, char *argv[])
{
	G3DModel *model;
	G3DMaterial *material;
	G3DObject *object;
	G3DGLRenderOptions *options;
	G3DGLRenderer *renderer;

	g_type_init();

	model = g_new0(G3DModel, 1);
	material = g3d_material_new();
	object = g3d_primitive_box(1.0, 1.0, 1.0, material);

	model->objects = g_slist_append(model->objects, object);

	options = g_new0(G3DGLRenderOptions, 1);

	renderer = g3d_gl_tess_renderer_new(options);
	g3d_gl_renderer_prepare(renderer, model);

	g_object_unref(renderer);

	return EXIT_SUCCESS;
}
