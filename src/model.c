/* $Id: model.c,v 1.1.2.2 2006/01/23 23:44:01 dahms Exp $ */

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

#include <g3d/g3d.h>

#include "main.h"
#include "texture.h"
#include "infowin.h"

gboolean model_load(G3DViewer *viewer)
{
	G3DModel *model;
	gchar *title;
	gboolean retval = FALSE;

	/* free old model */
	model = viewer->model;
	viewer->model = NULL;
	if(model)
		g3d_model_free(model);

	model = g3d_model_load(viewer->g3dcontext, viewer->filename);
	if(model)
	{
		viewer->model = model;
		texture_load_all_textures(viewer->model);
		retval = TRUE;
	}

	/* update model information */
	infowin_modeltab_fill(viewer);

	/* update window title */
	title = g_strdup_printf("g3dviewer%s%s",
		viewer->model ? " - " : "",
		viewer->filename ? viewer->filename : "");
	gtk_window_set_title(GTK_WINDOW(viewer->interface.window), title);
	g_free(title);

	return retval;
}
