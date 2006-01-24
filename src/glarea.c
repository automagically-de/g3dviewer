/* $Id: glarea.c,v 1.3.4.2 2006/01/23 23:44:01 dahms Exp $ */

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

#include "main.h"
#include "gl.h"
#include "glarea.h"
#include "trackball.h"

gint glarea_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;
	G3DViewer *viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(widget),
		"viewer");

	if(event->count > 0) return TRUE;

	gldrawable = gtk_widget_get_gl_drawable(widget);
	glcontext = gtk_widget_get_gl_context(widget);

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) return TRUE;
	gl_draw(viewer);
	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

gint glarea_configure(GtkWidget *widget, GdkEventConfigure *event)
{
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;
	G3DViewer *viewer;

	gldrawable = gtk_widget_get_gl_drawable(widget);
	glcontext = gtk_widget_get_gl_context(widget);

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) return TRUE;

	viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(widget), "viewer");
	glViewport(0,0, widget->allocation.width, widget->allocation.height);
	viewer->aspect = (float)widget->allocation.width /
		(float)widget->allocation.height;
#if DEBUG > 3
		g_printerr("DEBUG: glarea_configure (%f)\n", viewer->aspect);
#endif

	gdk_gl_drawable_gl_end(gldrawable);

	return TRUE;
}

gint glarea_destroy(GtkWidget *widget)
{
	G3DViewer *viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(widget),
		"viewer");
	if(viewer->interface.glarea)
	{
		/* FIXME: */
	}

	return TRUE;
}

gint glarea_scroll(GtkWidget *widget, GdkEventScroll *event)
{
	GdkRectangle area;
	G3DViewer *viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(widget),
		"viewer");

#define ZOOM_BY 10
	if(event->direction == GDK_SCROLL_DOWN) viewer->zoom += ZOOM_BY;
	else viewer->zoom -= ZOOM_BY;
#undef ZOOM_BY

	area.x = 0;
	area.y = 0;
	area.width = widget->allocation.width;
	area.height = widget->allocation.height;

	if(viewer->zoom < 1)   viewer->zoom = 1;
	if(viewer->zoom > 120) viewer->zoom = 120;

	gtk_widget_draw(widget, &area);

	return FALSE;
}

gint glarea_button_pressed(GtkWidget *widget, GdkEventButton *event)
{
	G3DViewer *viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(widget),
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
				(GtkWidget*)gtk_object_get_data(GTK_OBJECT(widget),
					"menu");
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
				event->button, event->time);
			return TRUE;
		}
	}

	return FALSE;
}

gint glarea_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
  int x, y;
  GdkRectangle area;
  GdkModifierType state;
  G3DViewer *viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(widget),
		"viewer");

  if(event->is_hint) gdk_window_get_pointer(event->window, &x, &y, &state);
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

  if(state & GDK_BUTTON1_MASK)
  {
    float spin_quat[4];
    trackball(spin_quat,
      (2.0*viewer->mouse.beginx -              area.width) / area.width,
      (            area.height - 2.0*viewer->mouse.beginy) / area.height,
      (                  2.0*x -              area.width) / area.width,
      (            area.height -                   2.0*y) / area.height);
    add_quats(spin_quat, viewer->quat, viewer->quat);
    gtk_widget_draw(widget, &area);
  }

  if(state & GDK_BUTTON2_MASK)
  {
    viewer->zoom += ((y - viewer->mouse.beginy) / (float)area.height) * 40;
    if(viewer->zoom < 1)   viewer->zoom = 1;
    if(viewer->zoom > 120) viewer->zoom = 120;
    gtk_widget_draw(widget, &area);
  }
  viewer->mouse.beginx = x;
  viewer->mouse.beginy = y;

  return TRUE;
}

