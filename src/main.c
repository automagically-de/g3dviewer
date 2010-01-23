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

#include <g3d/material.h>
#include <g3d/primitive.h>
#include <g3d/model.h>

#include "main.h"
#include "model.h"
#include "gui_glade.h"

static gboolean main_cwiid_cb(G3DViewer *viewer);
static void main_cleanup(G3DViewer *viewer);

int main(int argc, char **argv)
{
	G3DViewer *viewer;
	gboolean opt_debug_tree = FALSE;
	gboolean opt_debug_data = FALSE;
	gboolean opt_parse_only = FALSE;
	GError *error = NULL;
	GOptionContext *opt_ctxt;
	GOptionEntry opt_entries[] = {
		{ "debug-tree", 't', 0, G_OPTION_ARG_NONE, &opt_debug_tree,
			_("show hierarchical debugging data in info window"), NULL },
		{ "debug-data", 'd', 0, G_OPTION_ARG_NONE, &opt_debug_data,
			_("show additional debugging data in info window"), NULL },
		{ "parse-only", 'p', 0, G_OPTION_ARG_NONE, &opt_parse_only,
			_("only parse model and quit (deprecated, use g3d-stat)"), NULL },
		{ NULL }
	};

	/* localization stuff */
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

#if DEBUG > 3
	/* memory debugging */
	atexit(g_mem_profile);
	g_mem_set_vtable(glib_mem_profiler_table);
#endif

	opt_ctxt = g_option_context_new("[<filename>] - inspect 3D models");
	g_option_context_add_main_entries(opt_ctxt, opt_entries, PACKAGE);
	g_option_context_parse(opt_ctxt, &argc, &argv, &error);
	if(error) {
		g_warning("option parser: %s", error->message);
		g_error_free(error);
		error = NULL;
	}
	g_option_context_free(opt_ctxt);

	gtk_init(&argc, &argv);

#if DEBUG > 0
	/* gdb stack trace in case of error */
	g_on_error_stack_trace(NULL);
#endif

	gtk_gl_init(&argc, &argv);

	/* create viewer and fill with defaults */
	viewer = g_new0(G3DViewer, 1);

	viewer->filename = (argc > 1) ? g_strdup(argv[1]) : NULL;
	viewer->debug_flags =
		(opt_debug_tree & G3DV_FLAG_DEBUG_TREE) |
		(opt_debug_data & G3DV_FLAG_DEBUG_TREE_DATA);

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
	} else {
		/* try to show example model */
#ifdef G_OS_WIN32
		viewer->filename = g_strdup("examples\\g3d.ac");
#else
		viewer->filename = g_strdup(DATA_DIR "/examples/g3d.ac");
#endif
		if(model_load(viewer)) {
			g_object_set(G_OBJECT(viewer->interface.glarea),
				"rotation-x", 45.0,
				"rotation-y", 45.0,
				NULL);
		} else {
			/* show "open" dialog */
			gui_glade_open_dialog(viewer);
		}
	}

	/* for debugging reasons */
	if(opt_parse_only) {
		main_cleanup(viewer);
		return EXIT_SUCCESS;
	}

#if HAVE_CWIID
	g_idle_add((GSourceFunc)main_cwiid_cb, viewer);
#endif

	/* ... aaaand go! */
	gtk_main();

#if HAVE_CWIID
	if(viewer->cwiid.wiimote)
		cwiid_close(viewer->cwiid.wiimote);
#endif

	/* output timing statistics */
#if 0
#if DEBUG > 0
	if(viewer->gl.options->avg_msec != 0) {
		g_printerr("STAT: average time to render frame in µs: %u\n",
			viewer->gl.options->avg_msec);
	}
#endif
#endif

	/* cleaning up :-/ */
	main_cleanup(viewer);

	return EXIT_SUCCESS;
}

#if HAVE_CWIID
static gboolean main_cwiid_cb(G3DViewer *viewer)
{
	struct cwiid_state state;
	gdouble a_x, a_y, a_z, roll, pitch;

	if(!viewer->cwiid.wiimote || !viewer->model) {
		g_usleep(10000);
		return TRUE;
	}

	if(cwiid_get_state(viewer->cwiid.wiimote, &state) == 0) {
		a_x = ((gdouble)(state.acc[CWIID_X] - viewer->cwiid.cal.zero[CWIID_X]) /
			(viewer->cwiid.cal.one[CWIID_X] - viewer->cwiid.cal.zero[CWIID_X]));
		a_y = ((gdouble)(state.acc[CWIID_Y] - viewer->cwiid.cal.zero[CWIID_Y]) /
			(viewer->cwiid.cal.one[CWIID_Y] - viewer->cwiid.cal.zero[CWIID_Y]));
		a_z = ((gdouble)(state.acc[CWIID_Z] - viewer->cwiid.cal.zero[CWIID_Z]) /
			(viewer->cwiid.cal.one[CWIID_Z] - viewer->cwiid.cal.zero[CWIID_Z]));
		roll = atan(a_x / a_z);
		if(a_z <= 0.0) {
			roll += G_PI * ((a_x > 0.0) ? 1 : -1);
		}
		roll *= -1;

		pitch = atan(a_y / a_z * cos(roll));

#if DEBUG > 2
		g_print("x: %0.2f, y: %0.2f, z: %0.2f, pitch: %0.2f, roll: %0.2f\n",
			a_x, a_y, a_z, pitch, roll);
#endif
		if(pitch == pitch) {
			pitch *= -1;
			g_object_set(G_OBJECT(viewer->interface.glarea),
				"rotation-x", (pitch < 0) ? 360.0 + pitch * 45 : pitch * 45,
				NULL);
		}
		if(roll == roll) {
			g_object_set(G_OBJECT(viewer->interface.glarea),
				"rotation-z", (roll < 0) ? 360.0 + roll * 45 : roll * 45,
				NULL);
		}

	}

	g_usleep(10000);

	return TRUE;
}
#endif

static void main_cleanup(G3DViewer *viewer)
{
	g3d_context_free(viewer->g3dcontext);
	if(viewer->filename != NULL) g_free(viewer->filename);
	g_free(viewer);
}

