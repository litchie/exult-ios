/**
 **	Servemsg.h - Server msgs.
 **	NOTE:	This is for inclusion by both client and server.
 **
 **	Written: 5/3/2001 - JSF
 **/

#ifndef INCL_SERVEMSG
#define INCL_SERVEMSG 1
/*
Copyright (C) 2000-2001 The Exult Team

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


/*
 *	An entry sent between client and server will have the following format:
 *
Bytes	Description
0-1	Magic.
2-3	Length of the data, low-byte first.
4	Message type.  Defined below.
5-	Data.
 */

namespace Exult_server
{
const unsigned short magic = 0xf381;
const int maxlength = 600;		// Big enough to hold a 'terrain'.
const int hdrlength = 5;

enum Msg_type
	{
	say = 0,			// For testing.  Data is text.
	obj = 1,			// Generic object.
	egg = 2,			// Egg_object.
	npc = 3,			// Editing an NPC.
	user_responded = 4,		// User performed requested action.
	cancel = 5,			// Cancel operation.
	num_npcs = 6,			// Get/return # of NPC's.
	write_map = 7,			// Save map.
	read_map = 8,			// Read map.
	map_editing_mode = 9,		// 1 to turn it on, 0 for off.
	tile_grid = 10,			// 1 to show grid, 0 for off.
	edit_lift = 11,			// Lift passed: 0-13.
	reload_usecode = 12,		// Reload patched usecode.
	locate_terrain = 13,		// Locate desired chunk terrain.
	swap_terrain = 14,		// Swap two terrains.
	insert_terrain = 15,		// Insert new chunk terrain
	send_terrain = 16,		// Send 512-byte terrain to client.
	terrain_editing_mode = 17,	// 1 to turn on, 0 for off, -1 to undo.
	set_edit_shape = 18,		// Set shape/frame to 'paint' with.
	view_pos = 19			// Tile loc., size, scale of what's
					//   shown in gamewin.  Sent both ways.
	};

// I/O routines:
int Send_data
	(
	int socket,
	Msg_type id,
	unsigned char *data = 0,	// For just sending id.
	int datalen = 0
	);
int Receive_data
	(
	int& socket,			// Closed, set to -1 if disconnected.
	Msg_type& id,			// ID returned.
	unsigned char *data,
	int datalen
	);

// Wait for given ms for a response. return false if no response
bool wait_for_response(int socket, int ms);

}// Exult_server namespace.

#endif	/* INCL_SERVEMSG */
