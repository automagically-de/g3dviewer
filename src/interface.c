/* $Id: interface.c,v 1.6.2.1.2.8 2006/01/23 23:44:01 dahms Exp $ */

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

/* evil hack? */
#include <g3d/plugins.h>

#include "main.h"
#include "model.h"
#include "interface.h"
#include "infowin.h"
#include "glarea.h"
#include "gl.h"
#include "texture.h"


GtkWidget *interface_create_menu(G3DViewer *viewer);

int interface_init(G3DViewer *viewer)
{
	GtkWidget *vbox, *popupmenu;

	/* window */
	viewer->interface.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_signal_connect_object(GTK_OBJECT(viewer->interface.window),
		"delete-event", gtk_main_quit, (gpointer)viewer->interface.window);
	gtk_window_set_default_size(GTK_WINDOW(viewer->interface.window), 400, 440);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(viewer->interface.window), vbox);

	/* glarea */
	viewer->interface.glarea = interface_create_glarea(viewer);
	gtk_box_pack_start(GTK_BOX(vbox), viewer->interface.glarea, TRUE, TRUE, 0);

	/* status bar */
	viewer->interface.statushbox = gtk_hbox_new(FALSE, 1);
	viewer->interface.progressbar = gtk_progress_bar_new();
	gtk_progress_bar_set_bar_style(
		GTK_PROGRESS_BAR(viewer->interface.progressbar),
		GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_box_pack_start(GTK_BOX(viewer->interface.statushbox),
		viewer->interface.progressbar, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), viewer->interface.statushbox,
		FALSE, TRUE, 0);

	gtk_widget_show_all(viewer->interface.window);

	viewer->interface.infowin = infowin_create(viewer);

	/* popup menu */
	popupmenu = interface_create_menu(viewer);
	gtk_object_set_data(GTK_OBJECT(viewer->interface.glarea), "menu",
		popupmenu);

	if(viewer->interface.show_statusbar == 0)
	{
		gtk_widget_hide(viewer->interface.statushbox);
	}

	return EXIT_SUCCESS;
}

GtkWidget *interface_create_glarea(G3DViewer *viewer)
{
	GtkWidget *glarea;
	GdkGLConfig *glconfig;

	glconfig = gdk_gl_config_new_by_mode(
		GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

	if(glconfig == NULL)
	{
		g_printerr("can't get double-buffered visual\n");
		glconfig = gdk_gl_config_new_by_mode(
			GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_ALPHA | GDK_GL_MODE_DOUBLE);
	}

	glarea = gtk_drawing_area_new();

	gtk_widget_set_gl_capability(glarea, glconfig, NULL, TRUE,
		GDK_GL_RGBA_TYPE);


	if(glarea == NULL) return NULL;

	gtk_widget_set_events(glarea,
		GDK_EXPOSURE_MASK |
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		GDK_SCROLL_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

	gtk_signal_connect(GTK_OBJECT(glarea), "scroll_event",
		GTK_SIGNAL_FUNC(glarea_scroll), NULL);

	gtk_signal_connect(GTK_OBJECT(glarea), "expose_event",
		GTK_SIGNAL_FUNC(glarea_expose), NULL);
	gtk_signal_connect(GTK_OBJECT(glarea), "motion_notify_event",
		GTK_SIGNAL_FUNC(glarea_motion_notify), NULL);
	gtk_signal_connect(GTK_OBJECT(glarea), "button_press_event",
		GTK_SIGNAL_FUNC(glarea_button_pressed), NULL);
	gtk_signal_connect(GTK_OBJECT(glarea), "configure_event",
		GTK_SIGNAL_FUNC(glarea_configure), NULL);
	gtk_signal_connect(GTK_OBJECT(glarea), "destroy",
		GTK_SIGNAL_FUNC(glarea_destroy), NULL);

	gtk_object_set_data(GTK_OBJECT(glarea), "viewer", viewer);

	return glarea;
}

void interface_open_cb(GtkWidget *widget, gpointer user_data)
{
	interface_showopendialog((G3DViewer*)user_data);
}

void interface_selectcolor_cb(GtkWidget *widget, gpointer user_data)
{
	GtkColorSelectionDialog *dialog = (GtkColorSelectionDialog*)user_data;
	G3DViewer *viewer = gtk_object_get_data(GTK_OBJECT(dialog), "viewer");
	gdouble color[4] = {0.9, 0.8, 0.6, 1.0};

	gtk_color_selection_get_color(
		GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel),
		color);

	viewer->bgcolor[0] = color[0];
	viewer->bgcolor[1] = color[1];
	viewer->bgcolor[2] = color[2];
	viewer->bgcolor[3] = color[3];
}

void interface_color_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer = (G3DViewer*)user_data;
	GtkWidget *dialog;
	gdouble color[4];

	dialog = gtk_color_selection_dialog_new("Background color...");
	gtk_object_set_data(GTK_OBJECT(dialog), "viewer", viewer);

	/* GLfloat[] => gdouble[] */
	color[0] = viewer->bgcolor[0];
	color[1] = viewer->bgcolor[1];
	color[2] = viewer->bgcolor[2];
	color[3] = viewer->bgcolor[3];

	gtk_color_selection_set_color(
		GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel),
		color);
	gtk_signal_connect(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(dialog)->ok_button), "clicked",
		GTK_SIGNAL_FUNC(interface_selectcolor_cb), (gpointer)dialog);
	gtk_signal_connect_object_after(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(dialog)->ok_button), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)dialog);
	gtk_signal_connect_object_after(
		GTK_OBJECT(GTK_COLOR_SELECTION_DIALOG(dialog)->cancel_button),
		"clicked",
		GTK_SIGNAL_FUNC(gtk_widget_destroy), (gpointer)dialog);

	gtk_widget_show(dialog);
}

void interface_togglemode_cb(GtkWidget *widget, gpointer user_data)
{
	int wf = GTK_CHECK_MENU_ITEM(widget)->active;
	if(wf == 1)
	{
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}
#if DEBUG > 3
	g_printerr("interface_togglemode_cb: Wireframe: %d\n", wf);
#endif
}

void interface_togglespecular_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer = (G3DViewer*)user_data;
	if(GTK_CHECK_MENU_ITEM(widget)->active)
		viewer->glflags |= G3D_FLAG_GL_SPECULAR;
	else
		viewer->glflags &= ~G3D_FLAG_GL_SPECULAR;
}

void interface_toggleshininess_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer = (G3DViewer*)user_data;
	if(GTK_CHECK_MENU_ITEM(widget)->active)
		viewer->glflags |= G3D_FLAG_GL_SHININESS;
	else
		viewer->glflags &= ~G3D_FLAG_GL_SHININESS;
}

void interface_toggletwosided_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer = (G3DViewer*)user_data;
	if(GTK_CHECK_MENU_ITEM(widget)->active)
		viewer->glflags |= G3D_FLAG_GL_ALLTWOSIDE;
	else
		viewer->glflags &= ~G3D_FLAG_GL_ALLTWOSIDE;

	gl_set_twoside(viewer->glflags & G3D_FLAG_GL_ALLTWOSIDE);
}

void interface_toggletextures_cb(GtkWidget *widget, gpointer user_data)
{
	G3DViewer *viewer = (G3DViewer*)user_data;
	if(GTK_CHECK_MENU_ITEM(widget)->active)
		viewer->glflags |= G3D_FLAG_GL_TEXTURES;
	else
		viewer->glflags &= ~G3D_FLAG_GL_TEXTURES;

	gl_set_textures(GTK_CHECK_MENU_ITEM(widget)->active);
}

GtkWidget *interface_create_menu(G3DViewer *viewer)
{
	GtkWidget *menu = gtk_menu_new();
	GtkWidget *mitem;

	mitem = gtk_menu_item_new_with_label("Open new model...");
	gtk_signal_connect(GTK_OBJECT(mitem), "activate",
		GTK_SIGNAL_FUNC(interface_open_cb), viewer);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_object_set_data(GTK_OBJECT(menu), "viewer", viewer);
	gtk_widget_show(mitem);

	mitem = gtk_menu_item_new_with_label("Select background color...");
	gtk_signal_connect(GTK_OBJECT(mitem), "activate",
		GTK_SIGNAL_FUNC(interface_color_cb), viewer);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_check_menu_item_new_with_label("Wireframe");
	gtk_signal_connect(GTK_OBJECT(mitem), "toggled",
		GTK_SIGNAL_FUNC(interface_togglemode_cb), viewer);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_check_menu_item_new_with_label("Specular lighting");
	gtk_signal_connect(GTK_OBJECT(mitem), "toggled",
		GTK_SIGNAL_FUNC(interface_togglespecular_cb), viewer);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem),
		viewer->glflags & G3D_FLAG_GL_SPECULAR);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_check_menu_item_new_with_label("Shininess");
	gtk_signal_connect(GTK_OBJECT(mitem), "toggled",
		GTK_SIGNAL_FUNC(interface_toggleshininess_cb), viewer);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem),
		viewer->glflags & G3D_FLAG_GL_SHININESS);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_check_menu_item_new_with_label("Two-sided faces");
	gtk_signal_connect(GTK_OBJECT(mitem), "toggled",
		GTK_SIGNAL_FUNC(interface_toggletwosided_cb), viewer);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem),
		viewer->glflags & G3D_FLAG_GL_ALLTWOSIDE);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_check_menu_item_new_with_label("Textures");
	gtk_signal_connect(GTK_OBJECT(mitem), "toggled",
		GTK_SIGNAL_FUNC(interface_toggletextures_cb), viewer);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mitem), TRUE);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_menu_item_new_with_label("Info");
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_signal_connect_object(GTK_OBJECT(mitem), "activate",
		GTK_SIGNAL_FUNC(gtk_widget_show),
		(gpointer)viewer->interface.infowin);
	gtk_widget_show(mitem);

	mitem = gtk_menu_item_new();
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	mitem = gtk_menu_item_new_with_label("Quit");
	gtk_signal_connect_object(GTK_OBJECT(mitem), "activate",
		GTK_SIGNAL_FUNC(gtk_main_quit), (gpointer)viewer->interface.window);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	return menu;
}

void interface_load(GtkFileSelection *selector, gpointer user_data)
{
	const char *filename = gtk_file_selection_get_filename(selector);
	G3DViewer *viewer = (G3DViewer*)gtk_object_get_data(GTK_OBJECT(selector),
		"viewer");

#if DEBUG > 0
	g_printerr("interface_load: loading %s ...\n", filename);
#endif

	if(viewer->filename)
		g_free(viewer->filename);
	viewer->filename = g_strdup(filename);

	model_load(viewer);
}

void interface_changefiletype_cb(GtkWidget *menuitem, gpointer user_data)
{
	GtkFileSelection *dialog = (GtkFileSelection*)user_data;
	GtkOptionMenu *omenu = (GtkOptionMenu*)gtk_object_get_data(
		GTK_OBJECT(menuitem), "omenu");
	int index = GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(menuitem),
		"index"));
	char *extension = (char*)gtk_object_get_data(GTK_OBJECT(menuitem),
		"extension");
	gtk_option_menu_set_history(omenu, index);
	if(extension != NULL)
	{
		char pattern[256];
		g_snprintf(pattern, 256, "*.%s", extension);
		gtk_file_selection_complete(dialog, pattern);
	}
	else gtk_file_selection_complete(dialog, "*");
}

/* FIXME: GtkOptionMenu is deprecated */

int interface_showopendialog(G3DViewer *viewer)
{
	GtkWidget *dialog, *omenu, *menu, *mitem, *hbox, *label;
	GSList *plist;
	int index = 0;
	char name[256];

	if(viewer->interface.opendialog)
	{
		omenu = viewer->interface.filesel_omenu;
		menu = gtk_option_menu_get_menu(GTK_OPTION_MENU(omenu));
		mitem = gtk_menu_get_active(GTK_MENU(menu));
		interface_changefiletype_cb(mitem, viewer->interface.opendialog);
		gtk_widget_show(viewer->interface.opendialog);
		return EXIT_SUCCESS;
	}

	dialog = gtk_file_selection_new("open 3D object file...");
	viewer->interface.opendialog = dialog;

	gtk_signal_connect_object(
		GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
		"clicked", GTK_SIGNAL_FUNC(interface_load), (gpointer)dialog);
	gtk_object_set_data(GTK_OBJECT(dialog), "viewer", viewer);

	gtk_signal_connect_object_after(
		GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
		"clicked", GTK_SIGNAL_FUNC(gtk_widget_hide), (gpointer)dialog);
	gtk_signal_connect_object_after(
		GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),
		"clicked", GTK_SIGNAL_FUNC(gtk_widget_hide), (gpointer)dialog);

	omenu = gtk_option_menu_new();
	viewer->interface.filesel_omenu = omenu;
	menu = gtk_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(omenu), menu);

	mitem = gtk_menu_item_new_with_label("all files");
	gtk_object_set_data(GTK_OBJECT(mitem), "index", GINT_TO_POINTER(index));
	gtk_object_set_data(GTK_OBJECT(mitem), "omenu", omenu);
	gtk_signal_connect(GTK_OBJECT(mitem), "activate",
		GTK_SIGNAL_FUNC(interface_changefiletype_cb), dialog);
	gtk_menu_append(GTK_MENU(menu), mitem);
	gtk_widget_show(mitem);

	plist = viewer->g3dcontext->plugins;
	while(plist != NULL)
	{
		G3DPlugin *plugin = (G3DPlugin*)plist->data;
		gchar **ext, **extp;

		if(plugin->type != G3D_PLUGIN_IMPORT)
		{
			plist = plist->next;
			continue;
		}

		if(!plugin->ext_func)
		{
			plist = plist->next;
			continue;
		}

		extp = ext = plugin->ext_func(viewer->g3dcontext);
		if(ext == NULL)
		{
			plist = plist->next;
			continue;
		}

		while(*extp != NULL)
		{
			index++;
			g_snprintf(name, 256, "*.%s (%s)", *extp, plugin->name);
			mitem = gtk_menu_item_new_with_label(name);
			gtk_object_set_data(GTK_OBJECT(mitem), "extension",
				g_strdup(*extp));
			gtk_object_set_data(GTK_OBJECT(mitem), "omenu", omenu);
			gtk_object_set_data(GTK_OBJECT(mitem), "index",
				GINT_TO_POINTER(index));
			gtk_signal_connect(GTK_OBJECT(mitem), "activate",
				GTK_SIGNAL_FUNC(interface_changefiletype_cb), dialog);
			gtk_menu_append(GTK_MENU(menu), mitem);
			gtk_widget_show(mitem);
			extp ++;
		}
		plist = plist->next;

		g_strfreev(ext);
	}

	gtk_option_menu_set_history(GTK_OPTION_MENU(omenu), 0);

	hbox = gtk_hbox_new(TRUE, 0);
	label = gtk_label_new("show only these files:");
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), omenu, TRUE, TRUE, 0);
	gtk_widget_show(label);
	gtk_widget_show(hbox);

	gtk_box_pack_start(GTK_BOX(GTK_FILE_SELECTION(dialog)->action_area),
		hbox, FALSE, TRUE, 2);
	gtk_widget_show(omenu);

	gtk_widget_show(dialog);

	return EXIT_SUCCESS;
}

void interface_update_window(G3DViewer *viewer)
{
	GtkWidget *widget;

	widget = viewer->interface.glarea;

	gtk_widget_queue_draw_area(widget, 0, 0,
		widget->allocation.width, widget->allocation.height);
}

/*****************************************************************************/

void g3d_interface_update(void)
{
	while (gtk_events_pending()) gtk_main_iteration();
}

void g3d_interface_progress_init(G3DViewer *viewer)
{
	if(viewer->interface.show_statusbar == 0)
	{
		gtk_widget_show(viewer->interface.statushbox);
	}
	gtk_progress_bar_update(GTK_PROGRESS_BAR(viewer->interface.progressbar),
		0.0);
}

void g3d_interface_progress_update(G3DViewer *viewer, gfloat percentage)
{
	gtk_progress_bar_update(GTK_PROGRESS_BAR(viewer->interface.progressbar),
		percentage);
	g3d_interface_update();
}

void g3d_interface_progress_finish(G3DViewer *viewer)
{
	if(viewer->interface.show_statusbar == 0)
	{
		gtk_widget_hide(viewer->interface.statushbox);
	}
}

