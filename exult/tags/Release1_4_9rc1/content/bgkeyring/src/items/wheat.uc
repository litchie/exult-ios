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

/* This include file contains behaviours for grinding flour from wheat */

//Wheat can be used on a millstone to create a sack of flour
extern void Millstone shape#(0x2C7) ();
void Wheat shape#(0x2A5) ()
{
	//Player double-clicked on wheat-sack - it must first be picked up
	//in order to be used. Once the player has it, this function gets
	//called again with event = SCRIPTED
	if (event == DOUBLECLICK)
		gotoAndGet(item);

	//Wheat-sack has been acquired and is now ready to use on millstone
	else if (event == SCRIPTED)
	{
		UI_close_gumps();
		var target = UI_click_on_item();
		if (target->get_item_shape() != SHAPE_MILLSTONE)
		{
			randomPartySay("@Why not try grinding the wheat in a millstone?@");
			return;
		}

		//pathfind to the top edge of the millstone
		var target_offsetx = [-1, 0, 1];
		var target_offsety = [-5, -5, -5];

		//call Millstone() to handle the actual grinding of the wheat
		gotoObject(target, target_offsetx, target_offsety, -3, Millstone, target, SCRIPTED);
	}
}

//Create a new flour sack
void createFlour object#() ()
{
	//create the new flour object
	var flour = UI_create_new_object(SHAPE_KITCHEN_ITEM);
	if (!flour) return;
	flour->set_item_frame(FRAME_FLOURSACK_OPEN);	//changed from FRAME_FLOURSACK

	flour->set_item_flag(TEMPORARY);
	flour->set_item_flag(OKAY_TO_TAKE);

	var target_pos = get_object_position();
	//place it on the middle right-hand side of the millstone
	target_pos[X] = target_pos[X] + 1;
	target_pos[Y] = target_pos[Y] - 3;

	UI_update_last_created(target_pos);
	//Added: Check if Thurston is nearby, since he'll object
	if (isNearby(THURSTON) && UI_get_random(2) == 1)
	{
		if (!gflags[THURSTON_WARNED_ABOUT_MILLING])
		{
			THURSTON.say("Thurston approaches you, obviously angry and embarrassed of his anger.",
			             "~@I do not want to be rude ", getPoliteTitle(),
			             "...but it is my livelihood to run the mill and grind the wheat. So I ask thou, leavest the job to me!");
			THURSTON.say("@I shall be happy to sell thee all the flour thou couldst want.@");
			THURSTON.hide();
			gflags[THURSTON_WARNED_ABOUT_MILLING] = true;
		}
	}
}

//Performs grindage upon a sack of wheat
const int SOUND_GRIND = 26;
void Millstone shape#(0x2C7) ()
{
	//player double-clicked on the millstone
	if (event == DOUBLECLICK)
	{
		randomPartySay("@Try grinding wheat upon this millstone.@");
		return;
	}

	//player used wheat on the millstone
	else if (event == SCRIPTED)
	{
		//find the first wheat sack the player is carrying and remove it
		var wheat = AVATAR->get_cont_items(SHAPE_WHEAT, QUALITY_ANY, FRAME_ANY);
		wheat->remove_item();

		//animate the avatar doing things to the millstone
		script AVATAR
		{
			nohalt;
			call freeze;	//prevent player movement
			face directionFromAvatar(item);
			repeat 3
			{
				sfx SOUND_GRIND;
				actor frame ready; wait 2;
				actor frame standing; wait 2;
			};
			call unfreeze;
		}

		if (isNearby(THURSTON) && gflags[THURSTON_WARNED_ABOUT_MILLING] &&
				UI_get_random(2) == 1)
			delayedBark(THURSTON, "I have told thee before!", 10);

		//call createFlour() at end of animation
		//(we do this in a separate script block, so that the itemref
		//is set properly in the called function)
		script item after 28 ticks	{ call createFlour; }
	}
}
