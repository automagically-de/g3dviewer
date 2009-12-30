#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>

#include <g3d/quat.h>
#include <GL/gl.h>

#include "G3DGLWidget.h"
#include "G3DGLWidgetPriv.h"
#include "G3DGLWidgetProperties.h"
#include "G3DGLWidgetRender.h"

static gboolean g3d_gl_widget_configure_cb(G3DGLWidget *self,
	GdkEventConfigure *event);
static gboolean g3d_gl_widget_expose_cb(G3DGLWidget *self, GdkEventExpose *e);
static gboolean g3d_gl_widget_keypress_cb(G3DGLWidget *self,
	GdkEventKey *event, gpointer user_data);
static gboolean g3d_gl_widget_focus_cb(G3DGLWidget *self, GdkEventFocus *event,
	gpointer user_data);
static gboolean g3d_gl_widget_scroll_cb(G3DGLWidget *self,
	GdkEventScroll *event);
static gboolean g3d_gl_widget_button_pressed_cb(G3DGLWidget *self,
	GdkEventButton *event);
static gboolean g3d_gl_widget_button_released_cb(G3DGLWidget *self,
	GdkEventButton *event);
static gboolean g3d_gl_widget_motion_notify_cb(G3DGLWidget *self,
	GdkEventMotion *event);

static void g3d_gl_widget_class_init(G3DGLWidgetClass *klass)
{
	g_type_class_add_private(klass, sizeof(G3DGLWidgetPriv));
	g3d_widget_properties_init(klass);
}

static void g3d_gl_widget_init(G3DGLWidget *self)
{
	GtkStyle *style = NULL;

	self->priv = G3D_GL_WIDGET_GET_PRIVATE(self);

	/* set GL caps to drawing area */
    self->priv->glconfig = gdk_gl_config_new_by_mode(
        GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE |
        GDK_GL_MODE_STENCIL);

	/* ugly "default", overwritten by style */
	self->priv->bgcolor[0] = 1.0;
	self->priv->bgcolor[1] = 0.5;
	self->priv->bgcolor[2] = 0.5;
	self->priv->bgcolor[3] = 0.0;

	g_object_get(G_OBJECT(self), "style", &style, NULL);
	if(style) {
		self->priv->bgcolor[0] = (gdouble)style->bg[0].red   / 65536.0;
		self->priv->bgcolor[1] = (gdouble)style->bg[0].green / 65536.0;
		self->priv->bgcolor[2] = (gdouble)style->bg[0].blue  / 65536.0;
	}

	self->priv->gloptions = g_new0(G3DGLRenderOptions, 1);
	self->priv->gloptions->zoom = 45;
	self->priv->gloptions->glflags =
		G3D_FLAG_GL_COLORS |
		G3D_FLAG_GL_TEXTURES;
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
	g_signal_connect(G_OBJECT(self), "key-press-event",
		G_CALLBACK(g3d_gl_widget_keypress_cb), NULL);
	g_signal_connect(G_OBJECT(self), "focus-in-event",
		G_CALLBACK(g3d_gl_widget_focus_cb), NULL);
	g_signal_connect(G_OBJECT(self), "focus-out-event",
		G_CALLBACK(g3d_gl_widget_focus_cb), NULL);
	g_signal_connect(G_OBJECT(self), "scroll_event",
		G_CALLBACK(g3d_gl_widget_scroll_cb), NULL);
	g_signal_connect(G_OBJECT(self), "button_press_event",
		G_CALLBACK(g3d_gl_widget_button_pressed_cb), NULL);
	g_signal_connect(G_OBJECT(self), "button_release_event",
		G_CALLBACK(g3d_gl_widget_button_released_cb), NULL);
	g_signal_connect(G_OBJECT(self), "motion_notify_event",
		G_CALLBACK(g3d_gl_widget_motion_notify_cb), NULL);
}

GtkWidget *g3d_gl_widget_new(void)
{
	GtkWidget *widget;

	widget = g_object_new(G3D_GL_TYPE_WIDGET, NULL);
	return widget;
}

gboolean g3d_gl_widget_update_textures(G3DGLWidget *self, GHashTable *hash)
{
	self->priv->texture_hash = hash;
	self->priv->texture_hash_updated = TRUE;

	return TRUE;
}

static inline void g3d_gl_widget_invalidate(G3DGLWidget *self)
{
	gtk_widget_queue_draw_area(GTK_WIDGET(self), 0, 0,
		GTK_WIDGET(self)->allocation.width,
		GTK_WIDGET(self)->allocation.height);
}

static void g3d_gl_widget_trackball(G3DGLWidget *self,
	G3DFloat x1, G3DFloat y1, G3DFloat x2, G3DFloat y2)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	gfloat spin_quat[4];
	G3DFloat rx, ry, rz;
#if 0
	gchar *text;
#endif

	g3d_quat_trackball(spin_quat, x1, y1, x2, y2, 0.8);
	g3d_quat_add(options->quat, spin_quat, options->quat);
	/* normalize quat some times */
	options->norm_count ++;
	if(options->norm_count > 97) {
		options->norm_count = 0;
		g3d_quat_normalize(options->quat);
	}

	g3d_quat_to_rotation_xyz(options->quat, &rx, &ry, &rz);
#if 0
	text = g_strdup_printf("%-.2f°, %-.2f°, %-.2f°",
		rx * 180.0 / G_PI, ry * 180.0 / G_PI, rz * 180.0 / G_PI);
	gui_glade_status(viewer, text);
	g_free(text);
#endif
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

#define GLAREA_KEYPRESS_ROTATE_STEP 0.3
#define GLAREA_KEYPRESS_PAN_STEP 0.5

static gboolean g3d_gl_widget_keypress_cb(G3DGLWidget *self,
	GdkEventKey *event, gpointer user_data)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	gfloat offx = 0.0, offy = 0.0;
	gfloat panx = 0.0, pany = 0.0;
	gint32 zoom = 0;
	enum {
		A_NONE,
		A_TRACK,
		A_PAN,
		A_ZOOM
	} action = A_NONE;

	switch(event->keyval) {
		case GDK_Left:
			offx = GLAREA_KEYPRESS_ROTATE_STEP;
			panx = -GLAREA_KEYPRESS_PAN_STEP;
			action = (event->state & GDK_SHIFT_MASK) ? A_PAN : A_TRACK;
			break;
		case GDK_Right:
			offx = -GLAREA_KEYPRESS_ROTATE_STEP;
			panx = GLAREA_KEYPRESS_PAN_STEP;
			action = (event->state & GDK_SHIFT_MASK) ? A_PAN : A_TRACK;
			break;
		case GDK_Up:
			offy = -GLAREA_KEYPRESS_ROTATE_STEP;
			pany = GLAREA_KEYPRESS_PAN_STEP;
			action = (event->state & GDK_SHIFT_MASK) ? A_PAN : A_TRACK;
			break;
		case GDK_Down:
			offy = GLAREA_KEYPRESS_ROTATE_STEP;
			pany = -GLAREA_KEYPRESS_PAN_STEP;
			action = (event->state & GDK_SHIFT_MASK) ? A_PAN : A_TRACK;
			break;
		case GDK_minus:
			zoom = 10;
			action = A_ZOOM;
			break;
		case GDK_plus:
			zoom = -10;
			action = A_ZOOM;
			break;
	}

	switch(action) {
		case A_TRACK:
			g3d_gl_widget_trackball(self, 0, 0, offx, offy);
			g3d_gl_widget_invalidate(self);
			return TRUE;
		case A_PAN:
			options->offx += panx;
			options->offy += pany;
			g3d_gl_widget_invalidate(self);
			return TRUE;
		case A_ZOOM:
			zoom += options->zoom;
			options->zoom = MIN(120, MAX(1, zoom));
			g3d_gl_widget_invalidate(self);
			return TRUE;
		case A_NONE:
			return FALSE;
	}	

	return FALSE;
}

static gboolean g3d_gl_widget_focus_cb(G3DGLWidget *self, GdkEventFocus *event,
	gpointer user_data)
{
	self->priv->focused = event->in;
	return FALSE;
}

static gboolean g3d_gl_widget_scroll_cb(G3DGLWidget *self,
	GdkEventScroll *event)
{
	G3DGLRenderOptions *options = self->priv->gloptions;

#define ZOOM_BY 10
	if(event->direction == GDK_SCROLL_DOWN)
		options->zoom += ZOOM_BY;
	else
		options->zoom -= ZOOM_BY;
#undef ZOOM_BY

	options->zoom = MIN(120, MAX(1, options->zoom));

	g3d_gl_widget_invalidate(self);

	return FALSE;
}

static gboolean g3d_gl_widget_button_pressed_cb(G3DGLWidget *self,
	GdkEventButton *event)
{
	/* left mouse buttom: rotate object */
	if(event->button == 1) {
		self->priv->show_trackball = TRUE;
		gtk_widget_grab_focus(GTK_WIDGET(self));
		self->priv->drag_start_x = event->x;
		self->priv->drag_start_y = event->y;
		return TRUE;
	}
	/* right mouse button: pop-up menu */
	else if(event->button == 3)	{
		if(event->type == GDK_BUTTON_PRESS)	{
#if 0
			GtkWidget *menu =
				(GtkWidget*)g_object_get_data(G_OBJECT(widget),
					"menu");
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				event->button, event->time);
#endif
			return TRUE;
		}
	}

	return FALSE;
}

static gboolean g3d_gl_widget_button_released_cb(G3DGLWidget *self,
	GdkEventButton *event)
{
	self->priv->show_trackball = FALSE;
	g3d_gl_widget_invalidate(self);

	return TRUE;
}

static gboolean g3d_gl_widget_motion_notify_cb(G3DGLWidget *self,
	GdkEventMotion *event)
{
	gint32 x, y;
	GdkRectangle area;
	GdkModifierType state;
	G3DGLRenderOptions *options = self->priv->gloptions;

	if(event->is_hint) {
		gdk_window_get_pointer(event->window, &x, &y, &state);
	} else {
		x = event->x;
		y = event->y;
		state = event->state;
	}

	area.x = 0;
	area.y = 0;
	area.width = GTK_WIDGET(self)->allocation.width;
	area.height = GTK_WIDGET(self)->allocation.height;

	/* left button pressed */
	if(state & GDK_BUTTON1_MASK) {
		if(state & GDK_SHIFT_MASK) {
			/* shift pressed, translate view */
			options->offx +=
				(gdouble)(x - self->priv->drag_start_x) /
				(gdouble)(options->zoom * 10);
			options->offy -=
				(gdouble)(y - self->priv->drag_start_y) /
				(gdouble)(options->zoom * 10);
		} else {
			/* rotate view */
			g3d_gl_widget_trackball(self,
				(2.0 * self->priv->drag_start_x - area.width) / area.width,
				(area.height - 2.0 * self->priv->drag_start_y) / area.height,
				(2.0 * x - area.width) / area.width,
				(area.height - 2.0 * y) / area.height);
		}

		g3d_gl_widget_invalidate(self);
	}

	/* middle mouse button */
	if(state & GDK_BUTTON2_MASK) {
		options->zoom +=
			((y - self->priv->drag_start_y) / (gfloat)area.height) * 40;
		if(options->zoom < 1)
			options->zoom = 1;
		if(options->zoom > 120)
			options->zoom = 120;

		g3d_gl_widget_invalidate(self);
	}
	self->priv->drag_start_x = x;
	self->priv->drag_start_y = y;

	return TRUE;
}

G_DEFINE_TYPE(G3DGLWidget, g3d_gl_widget, GTK_TYPE_DRAWING_AREA)

void g3d_gl_widget_zoomfit(G3DGLWidget *self)
{
	G3DGLRenderOptions *options = self->priv->gloptions;
	
	options->offx = 0;
	options->offy = 0;
	options->zoom = 45;

	g3d_quat_trackball(options->quat, 0.0, 0.0, 0.0, 0.0, 0.8);
	g3d_gl_widget_invalidate(self);
}

