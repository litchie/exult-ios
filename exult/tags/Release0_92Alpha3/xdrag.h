/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Xdrag.h - Drag-and-drop under X.
 **
 **	Written: 12/23/2000 - JSF
 **/

/*
Copyright (C) 2000 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef INCL_XDRAG
#define INCL_XDRAG

const int max_types = 15;

typedef void (*Drop_handler_fun)(int shape, int frame, int x, int y);

/*
 *	This supports the 'drop' side of Xdnd:
 */
class Xdnd
	{
	Display *display;
	Window xwmwin;			// Gets WM window.
	Window xgamewin;		// Game window within xwmwin.
	Atom shapeid_atom;		// For drag-and-drop.
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
	int lastx, lasty;		// Last mouse pos. during drag.public:
	Drop_handler_fun drop_handler;	// For dropping shapes.
public:
	Xdnd(Display *d, Window xw, Window xgw, Drop_handler_fun dropfun);
	void client_msg(XClientMessageEvent& cev);
	void select_msg(XSelectionEvent& sev);
	};

#endif
