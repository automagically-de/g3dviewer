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
#include <string.h>

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <glade/glade-xml.h>

#include <g3d/plugins.h>

#include <G3DGLWidget.h>

#include "main.h"
#include "model.h"

#include "gui_glade.h"
#include "gui_infowin.h"
#include "gui_log.h"

static void gui_glade_clone_menuitem(GtkWidget *menuitem, gpointer user_data);

/*
 * initializes libglade
 */

gboolean gui_glade_init(G3DViewer *viewer)
{
	return TRUE;
}

gboolean gui_glade_add_open_filters(G3DViewer *viewer)
{
	GtkWidget *opendialog;
	GtkFileFilter *filter;
	GSList *plugins;
	G3DPlugin *plugin;
	gchar *name, *exts, **ext, *glob, *tmp;

	g_return_val_if_fail(viewer->g3dcontext != NULL, FALSE);

	opendialog = glade_xml_get_widget(viewer->interface.xml, "open_dialog");

	/* "all files" filter */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("all files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(opendialog), filter);

	/* FIXME: evil hack? */
	plugins = viewer->g3dcontext->plugins;
	while(plugins)
	{
		plugin = (G3DPlugin *)plugins->data;
		plugins = plugins->next;

		if(plugin->type != G3D_PLUGIN_IMPORT) continue;

		filter = gtk_file_filter_new();
		exts = g_strjoinv(", ", plugin->extensions);
		if(strlen(exts) > 30)
		{
			tmp = exts;
			exts = g_strdup_printf("%.*s...", 30, tmp);
			g_free(tmp);
		}
		name = g_strdup_printf("%s (%s)", plugin->name, exts);
		gtk_file_filter_set_name(filter, name);
		g_free(name);
		g_free(exts);

		ext = plugin->extensions;
		while(ext && *ext)
		{
			glob = g_strdup_printf("*.%s", *ext);
			gtk_file_filter_add_pattern(filter, glob);
			g_free(glob);

			ext ++;
		}

		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(opendialog), filter);
	}
	return TRUE;
}

/*
 * loads interface from .glade file
 */

gboolean gui_glade_load(G3DViewer *viewer)
{
	GladeXML *xml;
	GtkWidget *window, *widget, *popupmenu, *glarea, *statusbar;
	gint i;
	static const gchar *viewer_widgets[] = {
		"main_window",
		"mi_file_open",
		"mi_connect_wiimote",
		"mi_disconnect_wiimote",
		"mi_properties",
		"mi_screenshot",
		"mi_show_menubar",
		"mi_show_toolbar",
		"mi_fullscreen",
		"mi_zoomfit",
		"mi_isometric",
		"mi_wireframe",
		"mi_specular",
		"mi_shininess",
		"mi_shadow",
		"mi_colors",
		"mi_points",
		"mi_axes",
		"mi_textures",
		"mi_twosided",
		"mi_bgcolor",
		"mi_info",
		"tb_file_open",
		"tb_properties",
		"tb_zoomfit",
		"tb_screenshot",
		"tb_log_clear",
		"tb_log_save",
		"cs_background",
		"gtkglext1",
		NULL };

	/* load main window from xml */
#ifdef G_OS_WIN32
	xml = glade_xml_new("glade\\g3dviewer.glade", NULL, NULL);
#else
	xml = glade_xml_new(DATA_DIR "/glade/g3dviewer.glade", NULL, NULL);
#endif

	viewer->interface.xml = xml;

	glade_xml_signal_autoconnect(xml);
	window = glade_xml_get_widget(xml, "main_window");

	/* connect glarea (TODO: remove link) */
	glarea = glade_xml_get_widget(xml, "gtkglext1");
	viewer->interface.glarea = glarea;

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

	/* initialize infowin */
	gui_infowin_initialize(viewer, glade_xml_get_widget(xml, "proptree"));

	/* connect main menu to glarea */
	popupmenu = gtk_menu_new();
	gtk_container_foreach(
		GTK_CONTAINER(glade_xml_get_widget(xml, "menu_main")),
		gui_glade_clone_menuitem,
		popupmenu);
	gtk_widget_show_all(popupmenu);
	g_object_set(G_OBJECT(glarea), "popup-menu", popupmenu, NULL);

	/* get statusbar context id */
	statusbar = glade_xml_get_widget(viewer->interface.xml, "statusbar");
	viewer->interface.status_context_id = gtk_statusbar_get_context_id(
		GTK_STATUSBAR(statusbar), "default");

	/* initialize log */
	gui_log_initialize(viewer, glade_xml_get_widget(xml, "logtree"));
	g_log_set_handler("LibG3D", G_LOG_LEVEL_MASK, gui_log_handler, viewer);

	/* show main window */
	viewer->interface.window = window;
	gtk_widget_show_all(window);

	/* hide disabled features */
#if !HAVE_CWIID
	widget = glade_xml_get_widget(xml, "mi_connect_wiimote");
	gtk_widget_hide(widget);
	widget = glade_xml_get_widget(xml, "mi_disconnect_wiimote");
	gtk_widget_hide(widget);
	widget = glade_xml_get_widget(xml, "ms_connect_wiimote");
	gtk_widget_hide(widget);
#else
	widget = glade_xml_get_widget(xml, "mi_disconnect_wiimote");
	gtk_widget_set_sensitive(widget, FALSE);
#endif

	/* hide progress bar */
	gui_glade_update_progress_bar_cb(0.0, FALSE, viewer);

#if 0
	glarea_update(viewer->interface.glarea);
#endif
	return TRUE;
}

/*
 * set the file chooser in open dialog to a specific path, useful
 * when first opening a file by command line or drag and drop.
 */
gboolean gui_glade_set_open_path(G3DViewer *viewer, const gchar *path)
{
	GtkWidget *opendialog;
	gchar *tmp, *dir;
	gboolean retval;

	opendialog = glade_xml_get_widget(viewer->interface.xml, "open_dialog");

	dir = g_path_get_dirname(path);
	if(g_path_is_absolute(path))
		tmp = g_strdup(dir);
	else
		tmp = g_strdup_printf("%s%c%s",
			g_get_current_dir(), G_DIR_SEPARATOR, dir);
	g_free(dir);

	g_printerr("D: setting open path to '%s'\n", tmp);

	retval = gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(opendialog),
		tmp);
	g_free(tmp);

	return retval;
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
static void gui_glade_clone_menuitem(GtkWidget *menuitem, gpointer user_data)
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
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), newmi);

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

/*
 * show "open" dialog
 */
gboolean gui_glade_open_dialog(G3DViewer *viewer)
{
	GtkWidget *opendialog;
	gchar *filename;
	gint retval;

	opendialog = glade_xml_get_widget(viewer->interface.xml, "open_dialog");

	retval = gtk_dialog_run(GTK_DIALOG(opendialog));
	gtk_widget_hide(opendialog);
	if(retval == GTK_RESPONSE_OK)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(opendialog));

		if(viewer->filename)
			g_free(viewer->filename);
		viewer->filename = filename;

		retval = model_load(viewer);

		return retval;
	}

	return FALSE;
}

/*
 * show text in status bar
 */

gboolean gui_glade_status(G3DViewer *viewer, const gchar *text)
{
	GtkWidget *statusbar;

	statusbar = glade_xml_get_widget(viewer->interface.xml, "statusbar");
	gtk_statusbar_push(GTK_STATUSBAR(statusbar),
		viewer->interface.status_context_id, text);

	return TRUE;
}

static void drop_file_cb(GtkWidget *widget, GdkDragContext *drag_context,
	gint x, gint y, GtkSelectionData *data, guint info, guint time)
{
	G3DViewer *viewer;
	gchar *filename;

	viewer = (G3DViewer *)g_object_get_data(G_OBJECT(widget), "viewer");
	g_assert(viewer != NULL);

	filename = g_filename_from_uri((gchar *)data->data, NULL, NULL);
	if(filename)
	{
		if(viewer->filename)
			g_free(viewer->filename);
		g_strchomp(filename);
		viewer->filename = filename;

#if DEBUG > 0
		g_printerr("D: loading '%s' from dropped URI\n", viewer->filename);
#endif

		model_load(viewer);

		gui_glade_set_open_path(viewer, viewer->filename);
		gtk_drag_finish(drag_context, TRUE, FALSE, time);
	}
	else
	{
		g_warning("unhandled drop event received (%.*s)\n",
			data->length, (gchar *)data->data);
		gtk_drag_finish(drag_context, FALSE, FALSE, time);
	}
}

/*
 * create GL widget (called from libglade)
 */

GtkWidget *gui_glade_create_glwidget(void)
{
	GtkWidget *glarea;

	glarea = g3d_gl_widget_new();

	/* drag and drop stuff */
	gtk_drag_dest_set(glarea,
		GTK_DEST_DEFAULT_ALL,
		NULL, 0,
		GDK_ACTION_COPY );
	gtk_drag_dest_add_text_targets(glarea);
	g_signal_connect(G_OBJECT(glarea), "drag-data-received",
		G_CALLBACK(drop_file_cb), NULL);

	return glarea;
}

/*
 * set background color callback (G3DSetBgColorFunc)
 */
gboolean gui_glade_set_bgcolor_cb(
	gfloat r, gfloat g, gfloat b, gfloat a,
	gpointer user_data)
{
	G3DViewer *viewer;

	viewer = (G3DViewer *)user_data;
	g_assert(viewer);

	return TRUE;
}

/*
 * update interface callback (G3DUpdateInterfaceFunc)
 */
gboolean gui_glade_update_interface_cb(gpointer user_data)
{
	while(gtk_events_pending())
		gtk_main_iteration();

	return TRUE;
}

/*
 * update progress bar callback (G3DUpdateProgressBarFunc)
 */
gboolean gui_glade_update_progress_bar_cb(gfloat percentage,
	gboolean show, gpointer user_data)
{
	G3DViewer *viewer;
	GtkWidget *pbar;
	gchar *text;

	viewer = (G3DViewer *)user_data;
	g_assert(viewer);

	pbar = glade_xml_get_widget(viewer->interface.xml, "main_progressbar");
	g_assert(pbar);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pbar), percentage);

	text = g_strdup_printf("%.1f%%", percentage * 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(pbar), text);
	g_free(text);

	if(show)
		gtk_widget_show(pbar);
	else
		gtk_widget_hide(pbar);

	gui_glade_update_interface_cb(user_data);
	return TRUE;
}

