/* $Id: infowin.c,v 1.2.4.5 2006/01/23 23:44:01 dahms Exp $ */

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
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <g3d/plugins.h>

#include "main.h"
#include "infowin.h"

GtkWidget *infowin_imgtab(const char *imgfile, const char *label,
	G3DViewer *viewer);
GtkWidget *infowin_create_modeltab(G3DViewer *viewer);
GtkWidget *infowin_create_plugintab(G3DViewer *viewer);

/*****************************************************************************/

GtkWidget *infowin_create(G3DViewer *viewer)
{
	GtkWidget *win, *vbox, *notebook, *bbox, *bclose;

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win), "Info Window");
	gtk_window_set_default_size(GTK_WINDOW(win), 570, 380);

	gtk_signal_connect(GTK_OBJECT(win), "delete-event",
		GTK_SIGNAL_FUNC(gtk_widget_hide), NULL);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(win), vbox);

	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 2);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
		infowin_create_modeltab(viewer),
		infowin_imgtab(DATA_DIR "/pixmaps/icon16_model.xpm", "Model ", viewer));

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
		infowin_create_plugintab(viewer),
		infowin_imgtab(DATA_DIR "/pixmaps/icon16_plugins.xpm", "Plugins ",
			viewer));

	bbox = gtk_hbutton_box_new();
	gtk_container_set_border_width(GTK_CONTAINER(bbox), 3);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
	gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, TRUE, 0);

	bclose = gtk_button_new_with_label("close");
	gtk_signal_connect_object(GTK_OBJECT(bclose), "clicked",
		GTK_SIGNAL_FUNC(gtk_widget_hide), (gpointer)win);
	gtk_box_pack_start(GTK_BOX(bbox), bclose, FALSE, TRUE, 0);

	gtk_widget_show_all(vbox);

	return win;
}

/*****************************************************************************/

GtkWidget *infowin_imgtab(const char *imgfile, const char *label,
	G3DViewer *viewer)
{
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	GtkWidget *wimg;
	GtkWidget *hbox, *wlabel;

	pixmap = gdk_pixmap_create_from_xpm(viewer->interface.window->window,
		&mask, NULL, imgfile);
	hbox = gtk_hbox_new(FALSE, 0);
	wimg = gtk_pixmap_new(pixmap, mask);
	gtk_box_pack_start(GTK_BOX(hbox), wimg, TRUE, TRUE, 0);
	wlabel = gtk_label_new(label);
	gtk_box_pack_start(GTK_BOX(hbox), wlabel, TRUE, TRUE, 0);

	gtk_widget_show_all(hbox);

	return hbox;
}

/*****************************************************************************/
/* viewer tab                                                                 */
/*****************************************************************************/

void infowin_modeltab_tree_selectrow_cb(GtkCTree *tree, GList *node,
	gint column, gpointer data)
{

}

void infowin_modeltab_delete_cb(GtkCTree *tree, GList *node,
	gint column, gpointer data)
{
	gtk_ctree_remove_node(tree, GTK_CTREE_NODE(node));
}

int infowin_modeltab_fill(G3DViewer *viewer)
{
	GtkCTree *tree = (GtkCTree*)viewer->interface.modeltab.tree;
	GtkCTreeNode *node_objects, *node_mats, *node;
	GSList *mlist, *objects;
	GdkPixmap *pixmap, *pm_mat;
	GdkBitmap *mask, *bm_mat;
	gchar *text[1], *u8text[1];

	g_return_val_if_fail(viewer->model != NULL, EXIT_FAILURE);
	g_return_val_if_fail(tree != NULL, EXIT_FAILURE);

	objects = viewer->model->objects;

	/* remove all nodes */
	gtk_clist_freeze(GTK_CLIST(tree));
	gtk_ctree_post_recursive(tree, NULL,
		GTK_CTREE_FUNC(infowin_modeltab_delete_cb), NULL);
	gtk_clist_thaw(GTK_CLIST(tree));

	text[0] = g_malloc0(1024 * sizeof(char));
	pixmap = gdk_pixmap_create_from_xpm(viewer->interface.window->window,
		&mask, NULL, DATA_DIR "/pixmaps/icon16_model.xpm");
	pm_mat = gdk_pixmap_create_from_xpm(viewer->interface.window->window,
		&bm_mat, NULL, DATA_DIR "/pixmaps/icon16_material.xpm");

	gtk_clist_freeze(GTK_CLIST(tree));

	/* create objects */
	g_snprintf(text[0], 1024, "objects");
	node_objects = gtk_ctree_insert_node(tree, NULL, NULL,
		text, 2, NULL, NULL, NULL, NULL, FALSE, TRUE);

	while(objects != NULL)
	{
		G3DObject *object = (G3DObject*)objects->data;

		mlist = object->materials;
		g_snprintf(text[0], 1024, "%s", object->name);
		u8text[0] = g_locale_to_utf8(text[0], -1, NULL, NULL, NULL);
		node = gtk_ctree_insert_node(tree, node_objects, NULL,
			u8text[0] ? u8text : text,
			2, pixmap, mask, pixmap, mask, FALSE, FALSE);
		if(u8text[0] != NULL) g_free(u8text[0]);
		while(mlist != NULL)
		{
			G3DMaterial *material = (G3DMaterial*)mlist->data;
			GtkCTreeNode *mnode;

			if(material->name != NULL)
			{
				g_snprintf(text[0], 1024, "%s", material->name);
				u8text[0] = g_locale_to_utf8(text[0], -1, NULL, NULL, NULL);
				mnode = gtk_ctree_insert_node(tree, node, NULL,
					u8text[0] ? u8text : text,
					2, pm_mat, bm_mat, pm_mat, bm_mat, TRUE, TRUE);
				if(u8text[0] != NULL) g_free(u8text[0]);
			}
			mlist = mlist->next;
		}
		objects = objects->next;
	}

	/* add materials */
	if(viewer->model->materials != NULL)
	{
		g_snprintf(text[0], 1024, "materials");
		node_mats = gtk_ctree_insert_node(tree, NULL, NULL, text, 2,
			NULL, NULL, NULL, NULL,	FALSE, TRUE);
		mlist = viewer->model->materials;
		while(mlist != NULL)
		{
			G3DMaterial *material = (G3DMaterial*)mlist->data;
			GtkCTreeNode *mnode;

			g_snprintf(text[0], 1024, "%s", material->name);
			u8text[0] = g_locale_to_utf8(text[0], -1, NULL, NULL, NULL);
			mnode = gtk_ctree_insert_node(tree, node_mats, NULL,
				u8text[0] ? u8text : text,
				2, pm_mat, bm_mat, pm_mat, bm_mat, TRUE, TRUE);
			if(u8text[0] != NULL) g_free(u8text[0]);
			mlist = mlist->next;
		}
	}
	gtk_clist_thaw(GTK_CLIST(tree));
	return EXIT_SUCCESS;
}

GtkWidget *infowin_create_modeltab(G3DViewer *viewer)
{
	GtkWidget *tab, *hpaned, *scrolltree;

	tab = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(tab), 4);

	hpaned = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(hpaned), 200);
	gtk_box_pack_start(GTK_BOX(tab), hpaned, TRUE, TRUE, 0);

	viewer->interface.modeltab.tree = gtk_ctree_new(1, 0);
	scrolltree = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolltree),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_clist_set_column_auto_resize(GTK_CLIST(viewer->interface.modeltab.tree),
		0, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(scrolltree), 0);
	gtk_container_add(GTK_CONTAINER(scrolltree),
		viewer->interface.modeltab.tree);
	gtk_clist_set_selection_mode(GTK_CLIST(viewer->interface.modeltab.tree),
		GTK_SELECTION_BROWSE);
	gtk_signal_connect(GTK_OBJECT(viewer->interface.modeltab.tree),
		"tree-select-row",
		GTK_SIGNAL_FUNC(infowin_modeltab_tree_selectrow_cb), viewer);
	gtk_paned_add1(GTK_PANED(hpaned), scrolltree);

	return tab;
}

/*****************************************************************************/
/* plugin tab                                                                */
/*****************************************************************************/

void infowin_plugintab_tree_selectrow_cb(GtkCTree *tree, GList *node,
	gint column, gpointer data)
{
	G3DViewer *viewer = (G3DViewer*)data;
	GSList *plugins;
	gchar *desc = NULL;

	plugins = viewer->g3dcontext->plugins;

	while(plugins != NULL)
	{
		G3DPlugin *plugin = (G3DPlugin*)plugins->data;
		gchar *text;

		if(gtk_ctree_node_get_pixtext(tree, GTK_CTREE_NODE(node), 0, &text,
			NULL, NULL, NULL))
		{
#if DEBUG > 2
			g_printerr("tree-select-row: %s\n", text);
#endif
			if(strcmp(plugin->name, text) == 0)
			{
				if(plugin->desc_func)
					desc = plugin->desc_func(viewer->g3dcontext);

				gtk_label_set_text(
					GTK_LABEL(viewer->interface.plugintab.label_desc),
					desc ? desc : "(no description)");

				if(desc)
					g_free(desc);
#if 0
				gtk_label_set_text(GTK_LABEL(labelexts), plugin->extensions);
#endif
				return;
			}
		}
		plugins = plugins->next;
	}
}

GtkWidget *infowin_create_plugintab(G3DViewer *viewer)
{
	GtkWidget *tab, *hpaned, *tree, *scrolltree, *table, *widget;
	GtkCTreeNode *import, *image, *unknown;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	gchar *titles[1];
	GSList *plugins;

	tab = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(tab), 4);
	pixmap = gdk_pixmap_create_from_xpm(viewer->interface.window->window,
		&bitmap, NULL, DATA_DIR "/pixmaps/icon16_plugins.xpm");

	hpaned = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(hpaned), 200);
	gtk_box_pack_start(GTK_BOX(tab), hpaned, TRUE, TRUE, 0);

	tree = gtk_ctree_new(1, 0);
	scrolltree = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolltree),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_clist_set_column_auto_resize(GTK_CLIST(tree), 0, TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(scrolltree), 0);
	gtk_container_add(GTK_CONTAINER(scrolltree), tree);
	gtk_clist_set_selection_mode(GTK_CLIST(tree), GTK_SELECTION_BROWSE);
	gtk_signal_connect(GTK_OBJECT(tree), "tree-select-row",
		GTK_SIGNAL_FUNC(infowin_plugintab_tree_selectrow_cb), viewer);
	gtk_paned_add1(GTK_PANED(hpaned), scrolltree);

	/* table */
	table = gtk_table_new(2, 2, FALSE);
	widget = gtk_label_new("description");
	gtk_table_attach(GTK_TABLE(table), widget, 0,1, 0,1,
									 GTK_FILL, GTK_SHRINK, 0, 0);
	widget = gtk_label_new("extensions");
	gtk_table_attach(GTK_TABLE(table), widget, 0,1, 1,2,
									 GTK_FILL, GTK_SHRINK, 0, 8);

	viewer->interface.plugintab.label_desc = gtk_label_new("");
	gtk_label_set_line_wrap(
		GTK_LABEL(viewer->interface.plugintab.label_desc), TRUE);
	gtk_label_set_justify(
		GTK_LABEL(viewer->interface.plugintab.label_desc), GTK_JUSTIFY_LEFT);
	gtk_table_attach(GTK_TABLE(table), viewer->interface.plugintab.label_desc,
									 1,2, 0,1, GTK_FILL, GTK_SHRINK, 8, 0);
	viewer->interface.plugintab.label_exts = gtk_label_new("");
	gtk_label_set_line_wrap(
		GTK_LABEL(viewer->interface.plugintab.label_exts), TRUE);
	gtk_label_set_justify(
		GTK_LABEL(viewer->interface.plugintab.label_exts), GTK_JUSTIFY_LEFT);
	gtk_table_attach(GTK_TABLE(table), viewer->interface.plugintab.label_exts,
									 1,2, 1,2, GTK_FILL, GTK_SHRINK, 8, 8);

	gtk_paned_add2(GTK_PANED(hpaned), table);

	/* fill tree */
	titles[0] = g_malloc(1024 * sizeof(char));
	g_snprintf(titles[0], 1024, "import");
	import = gtk_ctree_insert_node(GTK_CTREE(tree), NULL, NULL,
		 titles, 2, NULL, NULL, NULL, NULL, FALSE, TRUE);
	g_snprintf(titles[0], 1024, "image");
	image = gtk_ctree_insert_node(GTK_CTREE(tree), NULL, NULL, titles,
		2, NULL, NULL, NULL, NULL, FALSE, TRUE);
	g_snprintf(titles[0], 1024, "unknown");
	unknown = gtk_ctree_insert_node(GTK_CTREE(tree), NULL, NULL, titles,
		2, NULL, NULL, NULL, NULL, FALSE, TRUE);

	plugins = viewer->g3dcontext->plugins;
	while(plugins != NULL)
	{
		GtkCTreeNode *node, *parent;
		G3DPlugin *plugin = (G3DPlugin *)plugins->data;
		if(plugin != NULL)
		{
			switch(plugin->type)
			{
				case G3D_PLUGIN_IMPORT: parent = import; break;
				case G3D_PLUGIN_IMAGE: parent = image; break;
				default: parent = unknown;
			}
			g_snprintf(titles[0], 1024, "%s", plugin->name);
			node = gtk_ctree_insert_node(GTK_CTREE(tree), parent, NULL,
				titles, 2, pixmap, bitmap, pixmap, bitmap,
				TRUE, FALSE);
		}
		plugins = plugins->next;
	}
	g_free(titles[0]);

	return tab;
}


