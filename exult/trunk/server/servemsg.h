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
 *	For the time being, we'll only inflict this on X users.
 */
#ifdef XWIN

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
const int maxlength = 256;
const int hdrlength = 5;
enum Msg_type
	{
	say = 0,			// For testing.  Data is text.
	egg = 1,			// Egg_object.
	npc = 2,			// Editing an NPC.
	user_responded = 3,		// User performed requested action.
	cancel = 4,			// Cancel operation.
	num_npcs = 5,			// Get/return # of NPC's.
	write_map = 6,			// Save map.
	read_map = 7,			// Read map.
	map_editing_mode = 8,		// 1 to turn it on, 0 for off.
	reload_usecode = 9		// Reload patched usecode.
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

}// Exult_server namespace.

#endif	/* XWIN */

#endif	/* INCL_SERVEMSG */
