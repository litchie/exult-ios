/*
 *
 *  Copyright (C) 2006  Alun Bestor/The Exult Team
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
 *	Author: Marzo Junior (reorganizing/updating code by Alun Bestor)
 *	Last Modified: 2006-03-19
 */

Bellows shape#(0x1AF) ()
{
	//Go to the bellows to operate it
	if (event == DOUBLECLICK)
		gotoObject(item, 1, 0, -1, Bellows, item, SCRIPTED);
	else if (event == SCRIPTED)
	{
		var item_frame = get_item_frame();
		var starting_frame;
		//Forge of Virtue bellows
		if (item_frame >= 3 && item_frame <= 5)
			starting_frame = 3;
		//Regular bellows
		else
			starting_frame = 0;
	
		halt_scheduled();	//Stop whatever animation the bellows were doing
		AVATAR->halt_scheduled();	//And stop what the Avatar was doing too

		//Animate the bellows pumping
		script item
		{
			sfx SOUND_BELLOWS;
			continue;	//I actually have no idea what continue does, since it seems to animate regardless

			frame starting_frame;
			repeat (NUM_BELLOWS_PUMPS - 1)
			{
				next frame; wait 1;
				next frame; wait 1;
				previous frame; wait 1;
				previous frame; wait 1;
			};
		}

		//Animate the Avatar pumping them
		script AVATAR
		{
			face directionFromAvatar(item);
			repeat (NUM_BELLOWS_PUMPS - 1)
			{
				actor frame standing;
				actor frame bowing;
				wait 3;
				actor frame standing;
			};
		}

		var nearby_firepits = find_nearby(SHAPE_FIREPIT, 4, MASK_ALL_UNSEEN);
		//The firepit animation behaviour is now moved off to its own function,
		//as it *really* does not belong here
		for (firepit in nearby_firepits)
			script firepit { call Firepit; }
	}
}
