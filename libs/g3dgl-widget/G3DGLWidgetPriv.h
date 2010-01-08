#ifndef _G3DGLWIDGETPRIV_H
#define _G3DGLWIDGETPRIV_H

#define G3D_GL_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	G3D_GL_TYPE_WIDGET, G3DGLWidgetPriv))

#include <g3d/types.h>

#include "g3dgl.h"
#include "G3DGLRenderer.h"

#include "G3DGLWidget.h"

struct _G3DGLWidgetPriv {
	G3DGLRenderer *renderer;
	G3DModel *model;

	gboolean initialized;
	gboolean focused;
	GdkGLConfig *glconfig;
	G3DGLRenderOptions *gloptions;
	GHashTable *texture_hash;
	gboolean texture_hash_updated;
	GtkWidget *popup_menu;

	gboolean show_trackball;
	gint32 drag_start_x;
	gint32 drag_start_y;

	G3DFloat model_min_y;
	G3DFloat rotation[3];
};

#endif /* _G3DGLWIDGETPRIV_H */
