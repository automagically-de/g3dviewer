/* $Id: gui_glade.c,v 1.1.2.4 2006/01/23 23:44:01 dahms Exp $ */

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

GtkWidget *gui_glade_custom_handler_func(GladeXML *xml,
	gchar *func_name, gchar *name,
	gchar *string1, gchar *string2,
	gint int1, gint int2,
	gpointer user_data);
void gui_glade_clone_menuitem(GtkWidget *menuitem, gpointer user_data);

/**
 * initializes libglade
 */

gboolean gui_glade_init(G3DViewer *viewer)
{
	glade_set_custom_handler(gui_glade_custom_handler_func, viewer);
	return TRUE;
}

/**
 * loads interface from .glade file
 */

gboolean gui_glade_load(G3DViewer *viewer)
{
	GladeXML *xml;
	GtkWidget *window, *widget, *popupmenu, *propwin;
	gint i;
	static const gchar *viewer_widgets[] = {
		"mi_file_open",
		"mi_properties",
		"mi_show_menubar",
		"mi_show_toolbar",
		"mi_wireframe",
		"mi_specular",
		"mi_shininess",
		"mi_textures",
		"mi_twosided",
		"tb_file_open",
		"tb_properties",
		NULL };

	/* load main window from xml */
	xml = glade_xml_new(DATA_DIR "/glade/g3dviewer.glade",
		NULL, NULL);

	viewer->interface.xml = xml;

	glade_xml_signal_autoconnect(xml);
	window = glade_xml_get_widget(xml, "main_window");

	propwin = glade_xml_get_widget(xml, "properties_window");
	gtk_widget_hide(propwin);

	/* connect viewer pointer to objects */
	i = 0;
	while(viewer_widgets[i] != NULL)
	{
		widget = glade_xml_get_widget(xml, viewer_widgets[i]);
		if(widget)
		{
#if DEBUG > 1
			g_printerr("D: viewer_widgets[%d]: %s\n", i, viewer_widgets[i]);
#endif
			g_object_set_data(G_OBJECT(widget), "viewer", viewer);
		}
		i ++;
	}

	/* connect main menu to glarea */
	popupmenu = gtk_menu_new();
	gtk_container_foreach(
		GTK_CONTAINER(glade_xml_get_widget(xml, "menu_main")),
		gui_glade_clone_menuitem,
		popupmenu);
	g_object_set_data(G_OBJECT(viewer->interface.glarea), "menu",
		popupmenu);
	gtk_widget_show_all(popupmenu);

	/* show main window */
	viewer->interface.window = window;
	gtk_widget_show_all(window);

	return TRUE;
}

/*
 * custom log handler to suppress warning in gui_glade_clone_menuitem
 */
static void gui_glade_null_logger(const gchar *log_domain,
	GLogLevelFlags log_level,
	const gchar *message, gpointer unused_data)
{
	; /* do nothing */
}

/*
 * clones one main menu item and attaches it to a popup menu
 */
void gui_glade_clone_menuitem(GtkWidget *menuitem, gpointer user_data)
{
	GtkWidget *menu, *newmi;
	GList *children;
	const gchar *label;

	menu = (GtkWidget *)user_data;

	children = gtk_container_get_children(GTK_CONTAINER(menuitem));
	if(children)
	{
		label = gtk_label_get_text(GTK_LABEL(g_list_nth_data(children, 0)));
	}
	else return;

#if DEBUG > 1
	g_printerr("D: label: %s\n", label);
#endif
	newmi = gtk_menu_item_new_with_label(label);
	gtk_menu_append(GTK_MENU(menu), newmi);

	/* disable warning */
	g_log_set_default_handler(gui_glade_null_logger, NULL);

	/* this normally generates a warning:
	 * gtk_menu_attach_to_widget(): menu already attached to GtkMenuItem
	 */
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(newmi),
		gtk_menu_item_get_submenu(GTK_MENU_ITEM(menuitem)));

	/* reenable logging */
	g_log_set_default_handler(g_log_default_handler, NULL);
}

/**
 * custom handler for libglade to provide interface_create_glarea with
 * a pointer
 */

GtkWidget *gui_glade_custom_handler_func(GladeXML *xml,
	gchar *func_name, gchar *name,
	gchar *string1, gchar *string2,
	gint int1, gint int2,
	gpointer user_data)
{
	G3DViewer *viewer;
	GtkWidget *glarea;

	viewer = (G3DViewer *)user_data;

	if(0 == strcmp("interface_create_glarea", func_name))
	{
		glarea = interface_create_glarea(viewer);
		viewer->interface.glarea = glarea;
		return glarea;
	}

	return NULL;
}

