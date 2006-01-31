/* $Id: gui_glade.h,v 1.1.2.2 2006/01/23 23:44:01 dahms Exp $ */

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

#ifndef _GUI_GLADE_H
#define _GUI_GLADE_H

#include "main.h"

gboolean gui_glade_init(G3DViewer *viewer);
gboolean gui_glade_load(G3DViewer *viewer);
gboolean gui_glade_open_dialog(G3DViewer *viewer);

#endif /* _GUI_GLADE_H */
