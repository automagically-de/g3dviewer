#ifndef _G3DGLWIDGETRENDER_H
#define _G3DGLWIDGETRENDER_H

#include "G3DGLWidget.h"

gboolean g3d_gl_widget_render_init(G3DGLWidget *self);
gboolean g3d_gl_widget_render_setup_view(G3DGLWidget *self);
void g3d_gl_widget_render(G3DGLWidget *self);

#endif /* _G3DGLWIDGETRENDER_H */
