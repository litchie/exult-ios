/*
 *	Xdrag.h - Drag-and-drop under X.
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

#ifndef INCL_XDRAG
#define INCL_XDRAG

const int max_types = 15;

typedef void (*Move_shape_handler_fun)(int shape, int frame, int x, int y,
					int prevx, int prevy, bool show);
typedef void (*Drop_shape_handler_fun)(int shape, int frame, int x, int y, void *data);
typedef void (*Drop_chunk_handler_fun)(int chunk, int x, int y, void *data);

/*
 *	This supports the 'drop' side of Xdnd:
 */
class Xdnd
	{
	Display *display;
	Window xwmwin;			// Gets WM window.
	Window xgamewin;		// Game window within xwmwin.
	Atom shapeid_atom;		// For drag-and-drop of shapes.
	Atom chunkid_atom;		// For chunks.
	Atom xdnd_aware;		// For XdndAware.
	Atom xdnd_enter;
	Atom xdnd_leave;
	Atom xdnd_position;
	Atom xdnd_drop;
	Atom xdnd_status;
	Atom xdnd_copy;
	Atom xdnd_ask;
	Atom xdnd_typelist;
	Atom xdnd_selection;
	unsigned long xdnd_version;
	int num_types;
	Atom drag_types[max_types];	// Data type atoms source can supply.
					// Current drag info:
	int lastx, lasty;		// Last mouse pos. during drag, within
					//   our window.
	int winx, winy;			// Window coords. at start of drag.
	int file, shape, frame;		// Set when a shape is being dragged.
	int chunknum;			// Set when a chunk is dragged.
	bool data_valid;		// True when the above is retrieved.

	Move_shape_handler_fun move_handler;	// For dragging shapes.
	Drop_shape_handler_fun shape_handler;	// For dropping shapes.
	Drop_chunk_handler_fun chunk_handler;	// For dropping chunks.
public:
	Xdnd(Display *d, Window xw, Window xgw, Move_shape_handler_fun movefun,
		Drop_shape_handler_fun shapefun, Drop_chunk_handler_fun cfun);
	void client_msg(XClientMessageEvent& cev);
	void select_msg(XSelectionEvent& sev);
	};

#endif
