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

#ifdef XWIN

#include <iostream.h>			/* Debugging messages */
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include "xdrag.h"
#include "u7drag.h"

/*
 *	Initialize.
 */

Xdnd::Xdnd
	(
	Display *d,
	Window xw,			// Window-manager window.
	Window xgw,			// Game's display window in xw.
	Drop_handler_fun dropfun
	) : display(d), xwmwin(xw), xgamewin(xgw),
		num_types(0), lastx(-1), lasty(-1),
		drop_handler(dropfun)
	{
	shapeid_atom = XInternAtom(display, U7_TARGET_SHAPEID_NAME, 0);
					// Atom for Xdnd protocol:
	xdnd_aware = XInternAtom(display, "XdndAware", 0);
	xdnd_enter = XInternAtom(display, "XdndEnter", 0);
	xdnd_leave = XInternAtom(display, "XdndLeave", 0);
	xdnd_position = XInternAtom(display, "XdndPosition", 0);
	xdnd_drop = XInternAtom(display, "XdndDrop", 0);
	xdnd_status = XInternAtom(display, "XdndStatus", 0);
	xdnd_copy = XInternAtom(display, "XdndActionCopy", 0);
	xdnd_ask = XInternAtom(display, "XdndActionAsk", 0);
	xdnd_typelist = XInternAtom(display, "XdndTypeList", 0);
	xdnd_selection = XInternAtom(display, "XdndSelection", 0);
	xdnd_version = 3;
					// Create XdndAware property.
	if (xwmwin)
		XChangeProperty(display, xwmwin, xdnd_aware, XA_ATOM, 32,
			PropModeReplace, (unsigned char *) &xdnd_version, 1);
	}

/*
 *	Handle drag-and-drop.
 */

void Xdnd::client_msg
	(
	XClientMessageEvent& cev	// Message received.
	)
	{
	cout << "Xwin client msg. received." << endl;
	char *nm = XGetAtomName(display, cev.message_type);
	if (nm)
		cout << "Type = " << nm << endl;
	XEvent xev;			// Return event.
	xev.xclient.type = ClientMessage;
	Window drag_win = cev.data.l[0];// Where drag comes from.
	xev.xclient.format = 32;
	xev.xclient.window = drag_win;
	xev.xclient.data.l[0] = xwmwin;
	if (cev.message_type == xdnd_enter)
		{
		if (cev.data.l[1]&1)	// More than 3 types?
			{
			Atom type;
			int format;
			unsigned long nitems, after;
			Atom *data;
			XGetWindowProperty(display, drag_win,
				xdnd_typelist, 0, 65536,
				false, XA_ATOM, &type, &format, &nitems,
			  	&after, (unsigned char **)&data);
			if (format != 32 || type != XA_ATOM)
				return;	// No good.
			if (nitems > max_types)
				nitems = max_types;
			for (num_types = 0; num_types < nitems; num_types++)
				drag_types[num_types] = data[num_types];
			}
		else
			{
			num_types = 0;
			for (int i = 0; i < 3; i++)
				if (cev.data.l[2+i])
					drag_types[num_types++] = 
							cev.data.l[2+i];
			cout << "num_types = " << num_types << endl;
			}
		}
	else if (cev.message_type == xdnd_position)
		{
		int i;			// For now, just do shapeid.
		for (i = 0; i < num_types; i++)
			if (drag_types[i] == shapeid_atom)
				break;
		xev.xclient.message_type = xdnd_status;
					// Flags??:  3=good, 0=can't accept.
		xev.xclient.data.l[1] = i < num_types ? 3 : 0;
					// I think next 2 should be a rect.??
		xev.xclient.data.l[2] = 0;
		xev.xclient.data.l[3] = 0;
		xev.xclient.data.l[4] = xdnd_copy;
		XSendEvent(display, drag_win, false, 0, &xev);
					// Save mouse position.
		lastx = (cev.data.l[2]>>16)&0xffff;
		lasty = cev.data.l[2]&0xffff;
		}
	else if (cev.message_type == xdnd_leave)
		num_types = 0;		// Clear list.
	else if (cev.message_type == xdnd_drop)
		{
		int i;			// For now, just do shapeid.
		for (i = 0; i < num_types; i++)
			if (drag_types[i] == shapeid_atom)
				break;
		bool okay = i < num_types;
		num_types = 0;
		if (!okay)
			return;
					// Get timestamp.
		unsigned long time = cev.data.l[2];
					// Tell owner we want it.
		XConvertSelection(display, xdnd_selection, shapeid_atom,
			xdnd_selection, xwmwin, time);
		}
	}

/*
 *	Get a window's screen coords.
 */

static void Get_window_coords
	(
	Display *display,
	Window win,
	int &sx, int& sy		// Coords. returned.
	)
	{
	Window root, parent;		// Get parent window.
	Window *children;
	unsigned int nchildren;
	XQueryTree(display, win, &root, &parent, &children, &nchildren);
	if (children)
		XFree(children);
	if (parent && parent != root)	// Recurse on parent.
		Get_window_coords(display, parent, sx, sy);
	else
		sx = sy = 0;
	XWindowAttributes atts;		// Get position within parent.
	XGetWindowAttributes(display, win, &atts);
	sx += atts.x;
	sy += atts.y;
	}

/*
 *	Get the selection and paste it in.
 */

void Xdnd::select_msg
	(
	XSelectionEvent& sev
	)
	{
	cout << "SelectionEvent received with target type: " <<
		XGetAtomName(display, sev.target) << endl;
	if (sev.selection != xdnd_selection || sev.target != shapeid_atom ||
	    sev.property == None)
		return;			// Wrong type.
	Atom type = None;		// Get data.
	int format;
	unsigned long nitems, after;
	unsigned char *data;		
	if (XGetWindowProperty(display, sev.requestor, sev.property,
		      0, 65000, False, AnyPropertyType,
		      &type, &format, &nitems, &after, &data) != Success)
		{
		cout << "Error in getting selection" << endl;
		return;
		}
	int file, shape, frame;		// Get shape info.
	Get_u7_shapeid(data, file, shape, frame);
	XFree(data);
	if (file == U7_SHAPE_SHAPES)	// For now, just allow "shapes.vga".
		{
		int x, y;		// Figure relative pos. within window.
		Get_window_coords(display, xgamewin, x, y);
		(*drop_handler)(shape, frame, lastx - x, lasty - y);
		}
	}

#endif	/* XWIN */


