/* $Id$ */

/*
	G3DViewer - 3D object viewer

	Copyright (C) 2005, 2006  Markus Dahms <mad@automagically.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdlib.h>

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <g3d/quat.h>

#include "main.h"
#include "gl.h"
#include "glarea.h"
#include "gui_glade.h"

/*
 * updates glarea widget (redraw)
 */
void glarea_update(GtkWidget *glarea)
{
	gtk_widget_queue_draw_area(glarea, 0, 0,
		glarea->allocation.width, glarea->allocation.height);
}

/*
 * handler for "expose" event
 */
gint glarea_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

	if(event->count > 0) return TRUE;

	gldrawable = gtk_widget_get_gl_drawable(widget);
	glcontext = gtk_widget_get_gl_context(widget);

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) return TRUE;

	/* prepare viewport */
	gl_setup_view(viewer->gl.options);
	gl_draw(viewer->gl.options, viewer->model);
	if(viewer->gl.show_trackball)
		gl_draw(viewer->gl.options_trackball, viewer->gl.model_trackball);
	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

/*
 * handler for "configure" event
 */
gint glarea_configure(GtkWidget *widget, GdkEventConfigure *event)
{
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;
	G3DViewer *viewer;

	gldrawable = gtk_widget_get_gl_drawable(widget);
	glcontext = gtk_widget_get_gl_context(widget);

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) return TRUE;

	viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget), "viewer");
	glViewport(0,0, widget->allocation.width, widget->allocation.height);
	viewer->gl.options->width = widget->allocation.width;
	viewer->gl.options->height = widget->allocation.height;
	viewer->gl.options->aspect = (gfloat)widget->allocation.width /
		(gfloat)widget->allocation.height;
#if DEBUG > 3
		g_printerr("DEBUG: glarea_configure (%f)\n", viewer->aspect);
#endif

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

/*
 * handler for "destroy" event
 */
gint glarea_destroy(GtkWidget *widget)
{
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");
	if(viewer->interface.glarea)
	{
		/* FIXME: */
	}

	return TRUE;
}

/*
 * handler for "scroll" event (mouse wheel)
 */
gint glarea_scroll(GtkWidget *widget, GdkEventScroll *event)
{
	GdkRectangle area;
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

#define ZOOM_BY 10
	if(event->direction == GDK_SCROLL_DOWN)
		viewer->gl.options->zoom += ZOOM_BY;
	else
		viewer->gl.options->zoom -= ZOOM_BY;
#undef ZOOM_BY

	area.x = 0;
	area.y = 0;
	area.width = widget->allocation.width;
	area.height = widget->allocation.height;

	if(viewer->gl.options->zoom < 1)   viewer->gl.options->zoom = 1;
	if(viewer->gl.options->zoom > 120) viewer->gl.options->zoom = 120;

	glarea_update(widget);

	return FALSE;
}

/*
 * handler for "button pressed" event
 */
gint glarea_button_pressed(GtkWidget *widget, GdkEventButton *event)
{
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

	/* left mouse buttom: rotate object */
	if(event->button == 1)
	{
		viewer->gl.show_trackball = TRUE;
		gtk_widget_grab_focus(widget);
		viewer->mouse.beginx = event->x;
		viewer->mouse.beginy = event->y;
		return TRUE;
	}
	/* right mouse button: pop-up menu */
	else if(event->button == 3)
	{
		if(event->type == GDK_BUTTON_PRESS)
		{
			GtkWidget *menu =
				(GtkWidget*)g_object_get_data(G_OBJECT(widget),
					"menu");
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				event->button, event->time);
			return TRUE;
		}
	}

	return FALSE;
}

gint glarea_button_released(GtkWidget *widget, GdkEventButton *event)
{
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

	viewer->gl.show_trackball = FALSE;
	glarea_update(widget);

	return TRUE;
}

static void glarea_trackball(G3DViewer *viewer,
	G3DFloat x1, G3DFloat y1, G3DFloat x2, G3DFloat y2)
{
	gfloat spin_quat[4];
	G3DFloat rx, ry, rz;
	gchar *text;

	g3d_quat_trackball(spin_quat, x1, y1, x2, y2, 0.8);
	g3d_quat_add(viewer->gl.options->quat,
		spin_quat, viewer->gl.options->quat);
	/* normalize quat some times */
	viewer->gl.options->norm_count ++;
	if(viewer->gl.options->norm_count > 97) {
		viewer->gl.options->norm_count = 0;
		g3d_quat_normalize(viewer->gl.options->quat);
	}

	g3d_quat_to_rotation_xyz(viewer->gl.options->quat, &rx, &ry, &rz);
	text = g_strdup_printf("%-.2f°, %-.2f°, %-.2f°",
		rx * 180.0 / G_PI, ry * 180.0 / G_PI, rz * 180.0 / G_PI);
	gui_glade_status(viewer, text);
	g_free(text);
}

/*
 * handler for "motion notify" event
 */
gint glarea_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;
	GdkRectangle area;
	GdkModifierType state;
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

	if(event->is_hint)
		gdk_window_get_pointer(event->window, &x, &y, &state);
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}

	area.x = 0;
	area.y = 0;
	area.width = widget->allocation.width;
	area.height = widget->allocation.height;

	/* left button pressed */
	if(state & GDK_BUTTON1_MASK) {
		if(state & GDK_SHIFT_MASK) {
			/* shift pressed, translate view */
			viewer->gl.options->offx +=
				(gdouble)(x - viewer->mouse.beginx) /
				(gdouble)(viewer->gl.options->zoom * 10);
			viewer->gl.options->offy -=
				(gdouble)(y - viewer->mouse.beginy) /
				(gdouble)(viewer->gl.options->zoom * 10);
		} else {
			/* rotate view */
			glarea_trackball(viewer,
				(2.0 * viewer->mouse.beginx - area.width) / area.width,
				(area.height - 2.0 * viewer->mouse.beginy) / area.height,
				(2.0 * x - area.width) / area.width,
				(area.height - 2.0 * y) / area.height);
		}

		glarea_update(widget);
	}

	/* middle mouse button */
	if(state & GDK_BUTTON2_MASK) {
		viewer->gl.options->zoom +=
			((y - viewer->mouse.beginy) / (gfloat)area.height) * 40;
		if(viewer->gl.options->zoom < 1)
			viewer->gl.options->zoom = 1;
		if(viewer->gl.options->zoom > 120)
			viewer->gl.options->zoom = 120;

		glarea_update(widget);
	}
	viewer->mouse.beginx = x;
	viewer->mouse.beginy = y;

	return TRUE;
}

#define GLAREA_KEYPRESS_ROTATE_STEP 0.3
#define GLAREA_KEYPRESS_PAN_STEP 0.5

gboolean glarea_keypress_cb(GtkWidget *widget, GdkEventKey *event,
	gpointer user_data)
{
	gfloat offx = 0.0, offy = 0.0;
	gfloat panx = 0.0, pany = 0.0;
	guint32 zoom = 0;
	enum {
		A_NONE,
		A_TRACK,
		A_PAN,
		A_ZOOM
	} action = A_NONE;
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

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
			glarea_trackball(viewer, 0, 0, offx, offy);
			glarea_update(widget);
			return TRUE;
		case A_PAN:
			viewer->gl.options->offx += panx;
			viewer->gl.options->offy += pany;
			glarea_update(widget);
			return TRUE;
		case A_ZOOM:
			zoom += viewer->gl.options->zoom;
			g_print("zoom = %d\n", zoom);
			viewer->gl.options->zoom = MIN(120, MAX(1, zoom));
			glarea_update(widget);
			return TRUE;
		case A_NONE:
			return FALSE;
	}	

	return FALSE;
}

gboolean glarea_focus_cb(GtkWidget *widget, GdkEventFocus *event,
	gpointer user_data)
{
	G3DViewer *viewer = (G3DViewer*)g_object_get_data(G_OBJECT(widget),
		"viewer");

	viewer->gl.options->focused = event->in;
	return FALSE;
}
