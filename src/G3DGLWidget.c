#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include <g3d/quat.h>
#include <GL/gl.h>

#include "G3DGLWidget.h"
#include "G3DGLWidgetPriv.h"
#include "G3DGLWidgetRender.h"

static gboolean g3d_gl_widget_configure_cb(G3DGLWidget *self,
	GdkEventConfigure *event);
static gboolean g3d_gl_widget_expose_cb(G3DGLWidget *self, GdkEventExpose *e);

static void g3d_gl_widget_class_init(G3DGLWidgetClass *klass)
{
	g_type_class_add_private(klass, sizeof(G3DGLWidgetPriv));
}

static void g3d_gl_widget_init(G3DGLWidget *self)
{
	self->priv = G3D_GL_WIDGET_GET_PRIVATE(self);

	/* set GL caps to drawing area */
    self->priv->glconfig = gdk_gl_config_new_by_mode(
        GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE |
        GDK_GL_MODE_STENCIL);
	self->priv->bgcolor[0] = 1.0;
	self->priv->bgcolor[1] = 0.4;
	self->priv->bgcolor[2] = 0.4;
	self->priv->bgcolor[3] = 0.0;

	self->priv->gloptions = g_new0(G3DGLRenderOptions, 1);
	self->priv->gloptions->zoom = 45;
	g3d_quat_trackball(self->priv->gloptions->quat, 0.0, 0.0, 0.0, 0.0, 0.8);

    if(self->priv->glconfig == NULL) {
        self->priv->glconfig = gdk_gl_config_new_by_mode(
            GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH |
            GDK_GL_MODE_ALPHA | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_STENCIL);
    }

	gtk_widget_set_gl_capability(GTK_WIDGET(self),
		self->priv->glconfig, NULL, TRUE,
        GDK_GL_RGBA_TYPE);

	/* can focus */
    GTK_WIDGET_SET_FLAGS(self, GTK_CAN_FOCUS);

	/* event filter */
    gtk_widget_set_events(GTK_WIDGET(self),
        GDK_EXPOSURE_MASK |
        GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
        GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
        GDK_SCROLL_MASK |
        GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

	g_signal_connect(G_OBJECT(self), "configure_event",
		G_CALLBACK(g3d_gl_widget_configure_cb), NULL);
	g_signal_connect(G_OBJECT(self), "expose-event",
		G_CALLBACK(g3d_gl_widget_expose_cb), NULL);

}

GtkWidget *g3d_gl_widget_new(void)
{
	GtkWidget *widget;

	widget = g_object_new(G3D_GL_TYPE_WIDGET, NULL);
	return widget;
}

gboolean g3d_gl_widget_set_model(G3DGLWidget *self, G3DModel *model)
{
	self->priv->model = model;
	self->priv->gloptions->updated = TRUE;

	return TRUE;
}

static gboolean g3d_gl_widget_configure_cb(G3DGLWidget *self,
	GdkEventConfigure *event)
{
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;
	G3DGLRenderOptions *options = self->priv->gloptions;

	gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(self));
	glcontext = gtk_widget_get_gl_context(GTK_WIDGET(self));

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return TRUE;

	options->width = GTK_WIDGET(self)->allocation.width;
	options->height = GTK_WIDGET(self)->allocation.height;
	glViewport(0,0, options->width, options->height);
	options->aspect = (gfloat)options->width / (gfloat)options->height;

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

static gboolean g3d_gl_widget_expose_cb(G3DGLWidget *self, GdkEventExpose *e)
{
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;

	if (e->count > 0)
		return TRUE;

	gldrawable = gtk_widget_get_gl_drawable(GTK_WIDGET(self));
	glcontext = gtk_widget_get_gl_context(GTK_WIDGET(self));

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return TRUE;

	if(!self->priv->initialized) {
		g3d_gl_widget_render_init(self);
		self->priv->initialized = TRUE;
	}
	g3d_gl_widget_render_setup_view(self);
	g3d_gl_widget_render(self);

	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

G_DEFINE_TYPE(G3DGLWidget, g3d_gl_widget, GTK_TYPE_DRAWING_AREA)

