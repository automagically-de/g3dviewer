/* $Id:$ */

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

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include <g3d/types.h>
#include <g3d/quat.h>

#include "main.h"
#include "model.h"
#include "glarea.h"
#include "gui_glade.h"
#include "gl.h"

static gboolean parse_only = FALSE;

static gboolean main_parseargs(int *argc, char ***argv, G3DViewer *viewer);
static void main_cleanup(G3DViewer *viewer);

int main(int argc, char **argv)
{
	G3DViewer *viewer;

	/* localization stuff */
	setlocale(LC_ALL, "");
	bindtextdomain("g3dviewer", LOCALEDIR);
	textdomain("g3dviewer");

#if DEBUG > 3
	/* memory debugging */
	atexit(g_mem_profile);
	g_mem_set_vtable(glib_mem_profiler_table);
#endif

	gtk_init(&argc, &argv);

#if DEBUG > 0
	/* gdb stack trace in case of error */
	g_on_error_stack_trace(NULL);
#endif

	gtk_gl_init(&argc, &argv);

	/* create viewer and fill with defaults
	 * TODO: move to separate function */
	viewer = g_new0(G3DViewer, 1);

	viewer->mouse.beginx = 0;
	viewer->mouse.beginy = 0;
	viewer->filename = NULL;

	viewer->renderoptions = g_new0(G3DGLRenderOptions, 1);
	viewer->renderoptions->updated = TRUE;
	viewer->renderoptions->initialized = FALSE;
	viewer->renderoptions->zoom = 45;
	viewer->renderoptions->bgcolor[0] = 0.9;
	viewer->renderoptions->bgcolor[1] = 0.8;
	viewer->renderoptions->bgcolor[2] = 0.6;
	viewer->renderoptions->bgcolor[3] = 1.0;
	viewer->renderoptions->glflags =
		G3D_FLAG_GL_SHININESS |
		G3D_FLAG_GL_TEXTURES |
		G3D_FLAG_GL_ALLTWOSIDE |
		G3D_FLAG_GL_COLORS;

	main_parseargs(&argc, &argv, viewer);

	g3d_quat_trackball(viewer->renderoptions->quat, 0.0, 0.0, 0.0, 0.0, 0.8);

	/* initialize libg3d */
	viewer->g3dcontext = g3d_context_new();

	/* the gui related stuff */
	gui_glade_init(viewer);
	gui_glade_load(viewer);

	gui_glade_add_open_filters(viewer);

	/* register gui callbacks */
	g3d_context_set_set_bgcolor_func(viewer->g3dcontext,
		gui_glade_set_bgcolor_cb, viewer);
	g3d_context_set_update_interface_func(viewer->g3dcontext,
		gui_glade_update_interface_cb, viewer);
	g3d_context_set_update_progress_bar_func(viewer->g3dcontext,
		gui_glade_update_progress_bar_cb, viewer);

	/* load model or show open dialog */
	if(viewer->filename != NULL) {
		gui_glade_set_open_path(viewer, viewer->filename);

		model_load(viewer);
		glarea_update(viewer->interface.glarea);
	} else {
		/* try to show example model */
#ifdef G_OS_WIN32
		viewer->filename = g_strdup("examples\\g3d.ac");
#else
		viewer->filename = g_strdup(DATA_DIR "/examples/g3d.ac");
#endif
		if(model_load(viewer)) {
			/* rotate a little bit */
			gfloat q1[4], q2[4];
			gfloat a1[3] = { 0.0, 1.0, 0.0 }, a2[3] = {1.0, 0.0, 1.0};

			g3d_quat_rotate(q1, a1, - 45.0 * G_PI / 180.0);
			g3d_quat_rotate(q2, a2, - 45.0 * G_PI / 180.0);
			g3d_quat_add(viewer->renderoptions->quat, q1, q2);

			glarea_update(viewer->interface.glarea);
		} else {
			/* show "open" dialog */
			gui_glade_open_dialog(viewer);
		}
	}

	/* for debugging reasons */
	if(parse_only) {
		main_cleanup(viewer);
		return EXIT_SUCCESS;
	}

	/* ... aaaand go! */
	gtk_main();

	/* output timing statistics */
#if DEBUG > 0
	if(viewer->renderoptions->avg_msec != 0) {
		g_printerr("STAT: average time to render frame in µs: %u\n",
			viewer->renderoptions->avg_msec);
	}
#endif

	/* cleaning up :-/ */
	main_cleanup(viewer);

	return EXIT_SUCCESS;
}

static void main_showhelp(void)
{
	g_print(
		"g3dviewer - a 3D model viewer\n"
		"\n"
		"usage: g3dviewer [--option ...] [<filename>]\n"
		"where option can be:\n"
		"  --help               show this help screen\n"
		"  --debug-tree         show file debug tree\n"
		"  --debug-tree-data    show file debug data\n"
		);
	exit(1);
}

static gboolean main_parseargs(int *argc, char ***argv, G3DViewer *viewer)
{
	/* program name */
	(*argc)--;
	(*argv)++;
	while(*argc > 0) {
#if DEBUG > 3
		g_printerr("arg: %s\n", **argv);
#endif
		if(strcmp(**argv, "--help") == 0) main_showhelp();
		else if(strcmp(**argv, "--parse-only") == 0) {
			parse_only = TRUE;
		} else if(strcmp(**argv, "--debug-tree") == 0) {
			viewer->debug_flags |= G3DV_FLAG_DEBUG_TREE;
		} else if(strcmp(**argv, "--debug-tree-data") == 0) {
			viewer->debug_flags |= G3DV_FLAG_DEBUG_TREE |
				G3DV_FLAG_DEBUG_TREE_DATA;
		} else {
			viewer->filename = g_strdup(**argv);
		}
		(*argv)++;
		(*argc)--;
	}
	return TRUE;
}

static void main_cleanup(G3DViewer *viewer)
{
	g3d_context_free(viewer->g3dcontext);
	if(viewer->filename != NULL) g_free(viewer->filename);
	g_free(viewer);
}
