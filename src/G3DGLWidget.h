#ifndef _G3DGLWIDGET_H
#define _G3DGLWIDGET_H

#include <glib-object.h>
#include <gtk/gtkgl.h>
#include <g3d/types.h>

G_BEGIN_DECLS

#define G3D_GL_TYPE_WIDGET            (g3d_gl_widget_get_type())
#define G3D_GL_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	G3D_GL_TYPE_WIDGET, G3DGLWidget))
#define G3D_GL_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), \
	G3D_GL_TYPE_WIDGET, G3DGLWidgetClass))
#define G3D_GL_IS_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	G3D_GL_TYPE_WIDGET))
#define G3D_GL_IS_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
	G3D_GL_TYPE_WIDGET))
#define G3D_GL_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
	G3D_GL_TYPE_WIDGET, G3DGLWidgetClass))

typedef struct _G3DGLWidget G3DGLWidget;
typedef struct _G3DGLWidgetClass G3DGLWidgetClass;
typedef struct _G3DGLWidgetPriv G3DGLWidgetPriv;

struct _G3DGLWidget {
	GtkDrawingArea parent_instance;
	G3DGLWidgetPriv *priv;
};

struct _G3DGLWidgetClass {
	GtkDrawingAreaClass parent_class;
};

GType g3d_gl_widget_get_type(void) G_GNUC_CONST;
GtkWidget *g3d_gl_widget_new(void);

G_END_DECLS

#endif /* _G3DGLWIDGET_H */
