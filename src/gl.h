/* $Id: gl.h,v 1.3.4.2 2006/01/23 23:44:01 dahms Exp $ */

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

#ifndef _GL_H
#define _GL_H

#include "main.h"

void gl_set_twoside(gboolean twoside);
void gl_set_textures(gboolean textures);
void gl_load_texture(gpointer key, gpointer value, gpointer data);
void gl_draw(G3DViewer *viewer);

#endif
