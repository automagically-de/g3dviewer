#ifndef _G3DGLWIDGETPRIV_H
#define _G3DGLWIDGETPRIV_H

#define G3D_GL_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	G3D_GL_TYPE_WIDGET, G3DGLWidgetPriv))

struct _G3DGLWidgetPriv {
	GdkGLConfig *glconfig;
};

#endif /* _G3DGLWIDGETPRIV_H */
