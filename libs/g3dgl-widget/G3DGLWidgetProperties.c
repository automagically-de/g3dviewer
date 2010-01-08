#include "G3DGLWidget.h"
#include "G3DGLWidgetPriv.h"

#include <g3d/quat.h>
#include <g3dgl.h>

enum {
	ID_0,
	G3DGL_PROP_BACKGROUND_COLOR,
	G3DGL_PROP_COLORS,
	G3DGL_PROP_ISOMETRIC,
	G3DGL_PROP_MODEL,
	G3DGL_PROP_POPUP_MENU,
	G3DGL_PROP_ROTATION_X,
	G3DGL_PROP_ROTATION_Y,
	G3DGL_PROP_ROTATION_Z,
	G3DGL_PROP_SHADOW,
	G3DGL_PROP_SHININESS,
	G3DGL_PROP_SPECULAR,
	G3DGL_PROP_TEXTURES,
	G3DGL_PROP_TWOSIDED,
	G3DGL_PROP_WIREFRAME
};

static inline void g3d_gl_flag_to_value(G3DGLRenderOptions *options,
	GValue *value, guint32 flag)
{
	g_value_set_boolean(value, options->glflags & flag);
}

static inline void g3d_gl_value_to_flag(G3DGLRenderOptions *options,
	const GValue *value, guint32 flag)
{
	if(g_value_get_boolean(value)) {
		options->glflags |= flag;
	} else {
		options->glflags &= ~flag;
	}
	options->updated = TRUE;
}

static void g3d_gl_widget_get_property(GObject *object,
	guint32 property_id, GValue *value, GParamSpec *pspec)
{
	G3DGLWidget *self = G3D_GL_WIDGET(object);
	G3DGLRenderOptions *options = self->priv->gloptions;
	gchar *stmp;

	switch(property_id) {
		case G3DGL_PROP_BACKGROUND_COLOR:
			stmp = g_strdup_printf("#%02X%02X%02X",
				(guint32)(options->bgcolor[0] * 0xFF),
				(guint32)(options->bgcolor[1] * 0xFF),
				(guint32)(options->bgcolor[2] * 0xFF));
			g_value_set_string(value, stmp);
			g_free(stmp);
			break;
		case G3DGL_PROP_COLORS:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_COLORS);
			break;
		case G3DGL_PROP_ISOMETRIC:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_ISOMETRIC);
			break;
		case G3DGL_PROP_MODEL:
			g_value_set_pointer(value, self->priv->model);
			break;
		case G3DGL_PROP_POPUP_MENU:
			g_value_set_pointer(value, self->priv->popup_menu);
			break;
		case G3DGL_PROP_ROTATION_X:
			g_value_set_float(value, self->priv->rotation[0]);
			break;
		case G3DGL_PROP_ROTATION_Y:
			g_value_set_float(value, self->priv->rotation[1]);
			break;
		case G3DGL_PROP_ROTATION_Z:
			g_value_set_float(value, self->priv->rotation[2]);
			break;
		case G3DGL_PROP_SHADOW:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_SHADOW);
			break;
		case G3DGL_PROP_SHININESS:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_SHININESS);
			break;
		case G3DGL_PROP_SPECULAR:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_SPECULAR);
			break;
		case G3DGL_PROP_TEXTURES:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_TEXTURES);
			break;
		case G3DGL_PROP_TWOSIDED:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_TWOSIDED);
			break;
		case G3DGL_PROP_WIREFRAME:
			g3d_gl_flag_to_value(options, value, G3D_FLAG_GL_WIREFRAME);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

static void g3d_gl_widget_set_rotation(G3DGLWidget *self)
{
	G3DQuat *tquat, *quat = self->priv->gloptions->quat;
	gint32 i;
	G3DVector a[3][3] = {
		{ 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 } };

#if 0
	g_debug("quat: [0] = {%.2f, %.2f, %.2f, %.2f}",
		quat[0], quat[1], quat[2], quat[3]);
#endif
	g3d_quat_trackball(quat, 0.0, 0.0, 0.0, 0.0, 0.8);
	for(i = 0; i < 3; i ++) {
		tquat = g_new0(G3DQuat, 4);
		g3d_quat_rotate(tquat, a[i], - self->priv->rotation[i] * G_PI / 180.0);
#if 0
		g_debug("tquat: [%d] = {%.2f, %.2f, %.2f, %.2f} (%.2f)",
			i, tquat[0], tquat[1], tquat[2], tquat[3],
			self->priv->rotation[i]);
#endif
		g3d_quat_add(quat, quat, tquat);
		g_free(tquat);
	}
#if 0
	g_debug("quat: [1] = {%.2f, %.2f, %.2f, %.2f}",
		quat[0], quat[1], quat[2], quat[3]);
#endif
	g3d_gl_widget_invalidate(self);
}

static void g3d_gl_widget_set_property(GObject *object,
	guint32 property_id, const GValue *value, GParamSpec *pspec)
{
	G3DGLWidget *self = G3D_GL_WIDGET(object);
	G3DGLRenderOptions *options = self->priv->gloptions;
	GdkColor color;

	switch(property_id) {
		case G3DGL_PROP_BACKGROUND_COLOR:
			if(gdk_color_parse(g_value_get_string(value), &color)) {
				options->bgcolor[0] = (gdouble)color.red   / 65536.0;
				options->bgcolor[1] = (gdouble)color.green / 65536.0;
				options->bgcolor[2] = (gdouble)color.blue  / 65536.0;
			}
			break;
		case G3DGL_PROP_COLORS:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_COLORS);
			break;
		case G3DGL_PROP_ISOMETRIC:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_ISOMETRIC);
			break;
		case G3DGL_PROP_MODEL:
			self->priv->model = g_value_get_pointer(value);
			self->priv->gloptions->updated = TRUE;
			break;
		case G3DGL_PROP_POPUP_MENU:
			self->priv->popup_menu = g_value_get_pointer(value);
			break;
		case G3DGL_PROP_ROTATION_X:
			self->priv->rotation[0] = g_value_get_float(value);
			g3d_gl_widget_set_rotation(self);
			break;
		case G3DGL_PROP_ROTATION_Y:
			self->priv->rotation[1] = g_value_get_float(value);
			g3d_gl_widget_set_rotation(self);
			break;
		case G3DGL_PROP_ROTATION_Z:
			self->priv->rotation[2] = g_value_get_float(value);
			g3d_gl_widget_set_rotation(self);
			break;
		case G3DGL_PROP_SHADOW:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_SHADOW);
			break;
		case G3DGL_PROP_SHININESS:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_SHININESS);
			break;
		case G3DGL_PROP_SPECULAR:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_SPECULAR);
			break;
		case G3DGL_PROP_TEXTURES:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_TEXTURES);
			break;
		case G3DGL_PROP_TWOSIDED:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_TWOSIDED);
			break;
		case G3DGL_PROP_WIREFRAME:
			g3d_gl_value_to_flag(options, value, G3D_FLAG_GL_WIREFRAME);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
	gtk_widget_queue_draw_area(GTK_WIDGET(self), 0, 0,
		GTK_WIDGET(self)->allocation.width,
		GTK_WIDGET(self)->allocation.height);
}

void g3d_widget_properties_init(G3DGLWidgetClass *klass)
{
	GObjectClass *oc;
	GParamSpec *pspec;

	oc = G_OBJECT_CLASS(klass);
	oc->set_property = g3d_gl_widget_set_property;
	oc->get_property = g3d_gl_widget_get_property;

	pspec = g_param_spec_string("background-color", "background-color",
		"set background color", "#FFF", G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_BACKGROUND_COLOR, pspec);

	pspec = g_param_spec_boolean("enable-colors", "enable-colors",
		"enable colors", TRUE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_COLORS, pspec);

	pspec = g_param_spec_boolean("enable-isometric", "enable-isometric",
		"enable isometric mode", FALSE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_ISOMETRIC, pspec);

	pspec = g_param_spec_boolean("enable-shadow", "enable-shadow",
		"enable ground plane & shadow", FALSE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_SHADOW, pspec);

	pspec = g_param_spec_boolean("enable-shininess", "enable-shininess",
		"enable shininess", TRUE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_SHININESS, pspec);

	pspec = g_param_spec_boolean("enable-specular", "enable-specular",
		"enable specular colors", FALSE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_SPECULAR, pspec);

	pspec = g_param_spec_boolean("enable-textures", "enable-textures",
		"enable textures", TRUE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_TEXTURES, pspec);

	pspec = g_param_spec_boolean("enable-twosided", "enable-twosided",
		"enable twosided faces", TRUE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_TWOSIDED, pspec);

	pspec = g_param_spec_boolean("enable-wireframe", "enable-wireframe",
		"wireframe mode", FALSE, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_WIREFRAME, pspec);

	pspec = g_param_spec_pointer("model", "model",
		"set model", G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_MODEL, pspec);

	pspec = g_param_spec_pointer("popup-menu", "popup-menu",
		"set popup menu", G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_POPUP_MENU, pspec);

	pspec = g_param_spec_float("rotation-x", "rotation-x",
		"set rotation around x axis", 0.0, 360.0, 0.0, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_ROTATION_X, pspec);

	pspec = g_param_spec_float("rotation-y", "rotation-y",
		"set rotation around y axis", 0.0, 360.0, 0.0, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_ROTATION_Y, pspec);

	pspec = g_param_spec_float("rotation-z", "rotation-z",
		"set rotation around z axis", 0.0, 360.0, 0.0, G_PARAM_READWRITE);
	g_object_class_install_property(oc, G3DGL_PROP_ROTATION_Z, pspec);
}

