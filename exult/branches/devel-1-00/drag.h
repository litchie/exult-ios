/*
 *	drag.cc - Dragging objects in Game_window.
 *
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef INCL_DRAG_H
#define INCL_DRAG_H

/*
 *	Data needed when dragging an object.
 */
class Dragging_info
	{
	Game_object *obj;		// What's being dragged.
	Gump *gump;
	Gump_button *button;
	Tile_coord old_pos;		// Original pos. of object if it wasn't
					//   in a container.
	Rectangle old_foot;		// Original footprint.
	int quantity;			// Amount of object being moved.
	int readied_index;		// If it was a 'readied' item.
					// Last mouse, paint positions:
	int mousex, mousey, paintx, painty;
	Rectangle rect;			// Rectangle to repaint.
	Image_buffer *save;		// Image below dragged object.
	bool okay;			// True if drag constructed okay.

	bool start(int x, int y);	// First motion.
	void put_back();		// Put back object.
	bool drop(int x, int y);	// Drop obj. at given position.
public:
	friend class Game_window;
	Dragging_info(int x, int y);	// Create for given mouse position.
	~Dragging_info();
	bool moved(int x, int y);	// Mouse moved.
					// Mouse button released.
	bool mouse_up(int x, int y, bool moved);
	};

#endif	/* INCL_DRAG_H */
