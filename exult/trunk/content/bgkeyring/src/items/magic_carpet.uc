/*
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *	This source file contains usecode to prevent the Magic Carpet from
 *	landing in the area of the Shrine of the Codex.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

Magic_Carpet shape#(0x348) ()
{
	var barge = get_barge();
	if ((event == DOUBLECLICK) && barge)
	{
		var pos = AVATAR->get_object_position();
		if (AVATAR->get_map_num() ==0 &&
			(pos[X] >= 0xA50) && (pos[Y] >= 0xABC) && (pos[X] <= 0xAE0) && (pos[Y] <= 0xB2D))
			//Avatar is over the area of the Shrine of the Codex; prevent landing
			AVATAR.say("There is a strange force preventing you from landing in this area.");
		else
			//Forward to original:
			Magic_Carpet.original();
	}
}
