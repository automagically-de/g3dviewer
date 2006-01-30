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

enum
{
	COL_TYPE,
	COL_TITLE,
	COL_VALUE,
	N_COLUMNS
};

GtkTreeStore *gui_infowin_create_model(void)
{
	GtkTreeStore *treestore;

	treestore = gtk_tree_store_new(N_COLUMNS,
		G_TYPE_INT, /* type of node */
		G_TYPE_STRING, /* title */
		G_TYPE_INT /* number */
		);

	return treestore;
}

gboolean gui_infowin_create_columns(GtkWidget *treeview)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	/* title column */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Title",
		renderer,
		"text", COL_TITLE,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	/* value column */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Value",
		renderer,
		"text", COL_VALUE,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	return TRUE;
}

gboolean gui_infowin_initialize(G3DViewer *viewer, GtkWidget *treeview)
{
	GtkTreeStore *treestore;
	GtkTreeSelection *select;
	GtkTreeIter iter;

	treestore = gui_infowin_create_model();
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview),
		GTK_TREE_MODEL(treestore));

	if(gui_infowin_create_columns(treeview) == FALSE)
		return FALSE;

	/* selection */
	select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	/* initial nodes */
	gtk_tree_store_append(treestore, &iter, NULL);
	gtk_tree_store_set(treestore, &iter,
		COL_TYPE, 0,
		COL_TITLE, "objects",
		COL_VALUE, 0,
		-1);

	gtk_tree_store_append(treestore, &iter, NULL);
	gtk_tree_store_set(treestore, &iter,
		COL_TYPE, 0,
		COL_TITLE, "materials",
		COL_VALUE, 0,
		-1);

	return TRUE;
}
