/* $Id: gui_callbacks.c,v 1.1.2.3 2006/01/23 23:44:01 dahms Exp $ */

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
#include <string.h>

#include <gtk/gtk.h>
#include <glade/glade-xml.h>

#include "main.h"
#include "interface.h"
#include "gl.h"

/*
 * File->Open
 */

void gui_on_open_cb(GtkWidget *widget, gpointer user_data)
{
	gpointer data;

	data = g_object_get_data(G_OBJECT(widget), "viewer");
	interface_showopendialog((G3DViewer*)data);
}

/*
 * File->Properties
 */

void gui_on_properties_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;
	GtkWidget *propwin;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	propwin = glade_xml_get_widget(viewer->interface.xml, "properties_window");

	gtk_widget_show_all(propwin);
}

/*
 * View->Show Menubar
 */

void gui_on_show_menubar_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;
	GtkWidget *menubar;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	menubar = glade_xml_get_widget(viewer->interface.xml, "menu_main");
	g_assert(menubar);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		gtk_widget_show(menubar);
	else
		gtk_widget_hide(menubar);
}

/*
 * View->Show Toolbar
 */

void gui_on_show_toolbar_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;
	GtkWidget *toolbar;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	toolbar = glade_xml_get_widget(viewer->interface.xml, "toolbar_main");
	g_assert(toolbar);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		gtk_widget_show(toolbar);
	else
		gtk_widget_hide(toolbar);
}

/*
 * View->Shininess
 */

void gui_on_shininess_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		viewer->glflags |= G3D_FLAG_GL_SHININESS;
	else
		viewer->glflags &= ~G3D_FLAG_GL_SHININESS;
}

/*
 * View->Two-sided Faces
 */

void gui_on_twosided_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	gl_set_twoside(
		gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)));
}

/*
 * View->Textures
 */

void gui_on_textures_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		viewer->glflags |= G3D_FLAG_GL_TEXTURES;
	else
		viewer->glflags &= ~G3D_FLAG_GL_TEXTURES;
}

/*
 * View->Specular Lighting
 */

void gui_on_specular_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
		viewer->glflags |= G3D_FLAG_GL_SPECULAR;
	else
		viewer->glflags &= ~G3D_FLAG_GL_SPECULAR;
}

/*
 * View->Wireframe
 */

void gui_on_wireframe_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer);

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget)))
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
}

/*
 * View->Background Color
 */

void gui_on_bgcolor_cb(GtkWidget *widget, gpointer user_data)
{
}

/*
 * Help->Info
 */

void gui_on_info_cb(GtkWidget *widget, gpointer user_data)
{
}

