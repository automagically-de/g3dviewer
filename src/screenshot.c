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

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <g3dgl.h>

gboolean screenshot_save_from_pixels(guint8 *pixels, const gchar *filename,
	guint32 width, guint32 height)
{
	GdkPixbuf *pixbuf, *flipped;

	g_return_val_if_fail(pixels != NULL, FALSE);

	pixbuf = gdk_pixbuf_new_from_data(pixels,
		GDK_COLORSPACE_RGB, TRUE,
		8, width, height, width * 4,
		NULL, NULL);
	if(pixbuf == NULL)
		return FALSE;

	flipped = gdk_pixbuf_flip(pixbuf, FALSE);
	if(flipped == NULL) {
		gdk_pixbuf_unref(pixbuf);
		return FALSE;
	}

	gdk_pixbuf_save(flipped, filename, "png", NULL, NULL);
	gdk_pixbuf_unref(flipped);
	gdk_pixbuf_unref(pixbuf);

	return TRUE;
}

gboolean screenshot_save(const gchar *filename, guint32 width, guint32 height)
{
	guint8 *pixels;
	gboolean retval;

	pixels = g3d_gl_get_pixels(width, height);
	retval = screenshot_save_from_pixels(pixels, filename, width, height);
	g_free(pixels);
	return retval;
}

