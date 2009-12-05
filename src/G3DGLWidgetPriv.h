#ifndef _G3DGLWIDGETPRIV_H
#define _G3DGLWIDGETPRIV_H

#define G3D_GL_WIDGET_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	G3D_GL_TYPE_WIDGET, G3DGLWidgetPriv))

#include <g3d/types.h>

#include "gl.h"

enum {
	G3DGLW_DLIST_MODEL,
	G3DGLW_DLIST_SHADOW,
	G3DGLW_N_DLISTS
};

struct _G3DGLWidgetPriv {
	gboolean initialized;
	gboolean focused;
	GdkGLConfig *glconfig;
	gfloat bgcolor[4];
	G3DGLRenderOptions *gloptions;
	G3DModel *model;

	/* from G3DGLRenderOptions, used only internally */
	G3DFloat model_min_y;

	/* from G3DGLRenderState */
	gint32 dlists[G3DGLW_N_DLISTS];
	gboolean dlists_valid;
	guint32 prev_texid;
	G3DMaterial *prev_material;
};

#endif /* _G3DGLWIDGETPRIV_H */
