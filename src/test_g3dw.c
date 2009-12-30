#include <gtk/gtk.h>

#include <g3d/g3d.h>

#include "texture.h"
#include "G3DGLWidget.h"

int main(int argc, char *argv[])
{
	GtkWidget *window, *g3dw1, *g3dw2, *hbox;
	G3DContext *ctx;
	G3DModel *model = NULL;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
	g_signal_connect(G_OBJECT(window), "delete-event",
		G_CALLBACK(gtk_main_quit), NULL);

	hbox = gtk_hbox_new(TRUE, 5);
	gtk_container_add(GTK_CONTAINER(window), hbox);

	g3dw1 = g3d_gl_widget_new();
	g3dw2 = g3d_gl_widget_new();

	gtk_box_pack_start(GTK_BOX(hbox), g3dw1, 5, TRUE, FALSE);
	gtk_box_pack_start(GTK_BOX(hbox), g3dw2, 5, TRUE, FALSE);

	ctx = g3d_context_new();
	if(argc > 1) {
		model = g3d_model_load(ctx, argv[1]);

		g3d_gl_widget_set_model(G3D_GL_WIDGET(g3dw1), model);
		g3d_gl_widget_update_textures(G3D_GL_WIDGET(g3dw1),
			model->tex_images);

		g3d_gl_widget_set_model(G3D_GL_WIDGET(g3dw2), model);
		g3d_gl_widget_update_textures(G3D_GL_WIDGET(g3dw2),
			model->tex_images);
	}
	
	gtk_widget_show_all(window);

	gtk_main();

	return 0;
}
