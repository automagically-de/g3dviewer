/* $Id: main.c,v 1.5.4.6 2006/01/23 23:44:01 dahms Exp $ */

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
#include <gtk/gtkgl.h>

#include <g3d/types.h>

#include "main.h"
#include "model.h"
#include "gui_glade.h"
#include "trackball.h"

static gboolean parse_only = FALSE;

int main_parseargs(int *argc, char ***argv, G3DViewer *viewer);

int main(int argc, char **argv)
{
	G3DViewer *viewer;

	gtk_init(&argc, &argv);
	gtk_gl_init(&argc, &argv);

	viewer = g_new0(G3DViewer, 1);
	viewer->zoom = 45;
	viewer->mouse.beginx = 0;
	viewer->mouse.beginy = 0;
	viewer->filename = NULL;
	viewer->bgcolor[0] = 0.9;
	viewer->bgcolor[1] = 0.8;
	viewer->bgcolor[2] = 0.6;
	viewer->bgcolor[3] = 1.0;
	viewer->glflags =
		/* G3D_FLAG_GL_SPECULAR | */
		G3D_FLAG_GL_SHININESS |
		G3D_FLAG_GL_ALLTWOSIDE |
		G3D_FLAG_GL_TEXTURES;

	viewer->g3dcontext = g3d_context_new();

	main_parseargs(&argc, &argv, viewer);

	trackball(viewer->quat, 0.0, 0.0, 0.0, 0.0);

	gui_glade_init(viewer);
	gui_glade_load(viewer);

	if(viewer->filename != NULL)
	{
		model_load(viewer);
	}
	else
		gui_glade_open_dialog(viewer);

	if(parse_only)
		return EXIT_SUCCESS;

	gtk_main();

	g3d_context_free(viewer->g3dcontext);

	return EXIT_SUCCESS;
}

void main_showhelp(void)
{
	printf(
		"g3dviewer - a 3D model viewer\n"
		"\n"
		"usage: g3dviewer [--option ...] [<filename>]\n"
		"where option can be:\n"
		"  --help               show this help screen\n"
		);
	exit(1);
}

int main_parseargs(int *argc, char ***argv, G3DViewer *viewer)
{
	/* program name */
	(*argc)--;
	(*argv)++;
	while(*argc > 0)
	{
#if DEBUG > 3
		g_printerr("arg: %s\n", **argv);
#endif
		if(strcmp(**argv, "--help") == 0) main_showhelp();
		else if(strcmp(**argv, "--parse-only") == 0)
		{
			parse_only = TRUE;
		}
		else
		{
			viewer->filename = **argv;
		}
		(*argv)++;
		(*argc)--;
	}

	return EXIT_SUCCESS;
}

