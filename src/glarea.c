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

	gl_draw(viewer->renderoptions, viewer->model);
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
	viewer->renderoptions->aspect = (gfloat)widget->allocation.width /
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
		viewer->renderoptions->zoom += ZOOM_BY;
	else
		viewer->renderoptions->zoom -= ZOOM_BY;
#undef ZOOM_BY

	area.x = 0;
	area.y = 0;
	area.width = widget->allocation.width;
	area.height = widget->allocation.height;

	if(viewer->renderoptions->zoom < 1)   viewer->renderoptions->zoom = 1;
	if(viewer->renderoptions->zoom > 120) viewer->renderoptions->zoom = 120;

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

/*
 * handler for "motion notify" event
 */
gint glarea_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y;
	gchar *text;
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
	if(state & GDK_BUTTON1_MASK)
	{
		if(state & GDK_SHIFT_MASK)
		{
			/* shift pressed, translate view */
			viewer->renderoptions->offx +=
				(gdouble)(x - viewer->mouse.beginx) /
				(gdouble)(viewer->renderoptions->zoom * 10);
			viewer->renderoptions->offy -=
				(gdouble)(y - viewer->mouse.beginy) /
				(gdouble)(viewer->renderoptions->zoom * 10);
		}
		else
		{
			/* rotate view */
			gfloat spin_quat[4];
			g3d_quat_trackball(spin_quat,
				(2.0 * viewer->mouse.beginx - area.width) / area.width,
				(area.height - 2.0 * viewer->mouse.beginy) / area.height,
				(2.0 * x - area.width) / area.width,
				(area.height - 2.0 * y) / area.height,
				0.8 /* trackball radius */);
			g3d_quat_add(viewer->renderoptions->quat,
				spin_quat, viewer->renderoptions->quat);
			/* normalize quat some times */
			viewer->renderoptions->norm_count ++;
			if(viewer->renderoptions->norm_count > 97) {
				viewer->renderoptions->norm_count = 0;
				g3d_quat_normalize(viewer->renderoptions->quat);
			}

			text = g_strdup_printf("quat: %-.2f, %-.2f, %-.2f, %-.2f",
				viewer->renderoptions->quat[0],
				viewer->renderoptions->quat[1],
				viewer->renderoptions->quat[2],
				viewer->renderoptions->quat[3]);
			gui_glade_status(viewer, text);
			g_free(text);
		}

		glarea_update(widget);
	}

	/* middle mouse button */
	if(state & GDK_BUTTON2_MASK)
	{
		viewer->renderoptions->zoom +=
			((y - viewer->mouse.beginy) / (gfloat)area.height) * 40;
		if(viewer->renderoptions->zoom < 1)
			viewer->renderoptions->zoom = 1;
		if(viewer->renderoptions->zoom > 120)
			viewer->renderoptions->zoom = 120;

		glarea_update(widget);
	}
	viewer->mouse.beginx = x;
	viewer->mouse.beginy = y;

	return TRUE;
}

