#include <gtk/gtk.h>

#include <g3d/g3d.h>

#include "G3DGLWidget.h"

int main(int argc, char *argv[])
{
	GtkWidget *window, *g3dw1, *g3dw2, *hbox;
	G3DContext *ctx;
	G3DModel *model = NULL;
	gchar *stmp;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
	g_signal_connect(G_OBJECT(window), "delete-event",
		G_CALLBACK(gtk_main_quit), NULL);

	hbox = gtk_hbox_new(TRUE, 5);
	gtk_container_add(GTK_CONTAINER(window), hbox);

	g3dw1 = g3d_gl_widget_new();
	g3dw2 = g3d_gl_widget_new();

	g_object_get(G_OBJECT(g3dw1), "background-color", &stmp, NULL);
	g_debug("background-color: %s", stmp);

	g_object_set(G_OBJECT(g3dw1),
		"enable-wireframe", TRUE,
		"enable-colors", FALSE,
		"enable-textures", FALSE,
		"enable-isometric", TRUE,
		"background-color", "#123456",
		NULL);

	gtk_box_pack_start(GTK_BOX(hbox), g3dw1, 5, TRUE, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), g3dw2, 5, TRUE, FALSE);

	ctx = g3d_context_new();
	if(argc > 1) {
		model = g3d_model_load(ctx, argv[1]);

		g_object_set(G_OBJECT(g3dw1), "model", model, NULL);
		g3d_gl_widget_update_textures(G3D_GL_WIDGET(g3dw1),
			model->tex_images);

		g_object_set(G_OBJECT(g3dw1),
			"rotation-x", 45, NULL);

		g_object_set(G_OBJECT(g3dw2), "model", model, NULL);
		g3d_gl_widget_update_textures(G3D_GL_WIDGET(g3dw2),
			model->tex_images);
	}
	
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
