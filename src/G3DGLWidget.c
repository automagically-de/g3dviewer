#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include "G3DGLWidget.h"
#include "G3DGLWidgetPriv.h"

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
}

GtkWidget *g3d_gl_widget_new(void)
{
	GtkWidget *widget;

	widget = g_object_new(G3D_GL_TYPE_WIDGET, NULL);
	return widget;
}

G_DEFINE_TYPE(G3DGLWidget, g3d_gl_widget, GTK_TYPE_DRAWING_AREA)

