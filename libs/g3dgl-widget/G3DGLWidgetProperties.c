#include "G3DGLWidget.h"
#include "G3DGLWidgetPriv.h"

#include "../g3dgl/g3dgl.h"

enum {
	ID_0,
	G3DGL_PROP_BACKGROUND_COLOR,
	G3DGL_PROP_COLORS,
	G3DGL_PROP_ISOMETRIC,
	G3DGL_PROP_MODEL,
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
				(guint32)(self->priv->bgcolor[0] * 0xFF),
				(guint32)(self->priv->bgcolor[1] * 0xFF),
				(guint32)(self->priv->bgcolor[2] * 0xFF));
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

static void g3d_gl_widget_set_property(GObject *object,
	guint32 property_id, const GValue *value, GParamSpec *pspec)
{
	G3DGLWidget *self = G3D_GL_WIDGET(object);
	G3DGLRenderOptions *options = self->priv->gloptions;
	GdkColor color;

	switch(property_id) {
		case G3DGL_PROP_BACKGROUND_COLOR:
			if(gdk_color_parse(g_value_get_string(value), &color)) {
				self->priv->bgcolor[0] = (gdouble)color.red   / 65536.0;
				self->priv->bgcolor[1] = (gdouble)color.green / 65536.0;
				self->priv->bgcolor[2] = (gdouble)color.blue  / 65536.0;
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

	pspec = g_param_spec_boolean("enable-shininess", "enable-shinines",
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
}

