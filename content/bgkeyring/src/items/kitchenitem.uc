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

/*	This script contains behaviors for kitchen items and buttern churning.
 *	Since bucket and milk-bottle behaviours use constants defined in this
 *	script, it must be included before bucket.uc and food.uc.
 */

KitchenItem shape#(0x35F) ()
{
	var framenum = get_item_frame();
	var target;

	if (event == DOUBLECLICK)
	{
		//flour sack behaviour
		if (framenum == FRAME_FLOURSACK_OPEN)
		{
			target = UI_click_on_item();
			var target_pos = target->get_object_position();
			var target_shape = target->get_item_shape();

			if (target_shape == SHAPE_WORKTABLE_HORIZONTAL)
			{
				target_pos[X] = target_pos[X] - UI_die_roll(0, 3);
				//vertical offset ensures the flour ends up on top of the table
				target_pos[Z] = target_pos[Z] + 2;
			}
			else if (target_shape == SHAPE_WORKTABLE_VERTICAL)
			{
				target_pos[Y] = target_pos[Y] - UI_die_roll(0, 2);
				//vertical offset ensures the flour ends up on top of the table
				target_pos[Z] = target_pos[Z] + 2;
			}
			//only those two kinds of tables are supported, oddly
			else
			{
				randomPartySay("@Why not put the flour on the table first?@");
				return;
			}

			//create and position the new flour object
			var flour = SHAPE_DOUGH->create_new_object();
			if (!flour) return;
			flour->set_item_frame(FRAME_FLOUR);
			flour->set_item_flag(TEMPORARY);	//now what the hell is this flag?
			target_pos->update_last_created();
		}

		//empty pitcher behaviour - go and pick up the pitcher
		//then rerun the function with event = SCRIPTED
		else if (framenum == FRAME_PITCHER) gotoAndGet(item);

		//rolling pin behaviour
		else if (framenum in [FRAME_ROLLINGPIN, FRAME_ROLLINGPIN_2])
		{
			target = UI_click_on_item();

			//rolling pin was used on a ball of dough
			if (target->get_item_shape() == SHAPE_DOUGH && target->get_item_frame() == FRAME_DOUGH_BALL)
				//roll it out into flat dough
				target->set_item_frame(FRAME_DOUGH_FLAT);

			//rolling pin was used on a person (donk!)
			else if (canTalk(target))
				target->item_say("@Hey! That really hurt!@");
		}

		//closed floursack behaviour (no idea why there are two identical
		//frames for this...)
		else if (framenum in [FRAME_FLOURSACK, FRAME_FLOURSACK_2])
			set_item_frame(FRAME_FLOURSACK_OPEN);

		//churn behaviour (simple prompt for milk)
		else if (framenum == FRAME_CHURN)
			randomPartySay("@Thou shan't get far without some milk to churn!@");

		//full pitcher behaviour
		else if (framenum == FRAME_PITCHER_MILK)
		{
			target = UI_click_on_item();
			if (target->get_item_shape() == SHAPE_KITCHEN_ITEM && target->get_item_frame() == FRAME_CHURN)
				gotoChurn(target, CHURN_WITH_PITCHER);
			else
				randomPartySay("@Why not churn that milk into butter?@");
		}
	}

	//currently only pitchers use this, since they need to be manually picked up
	//Note that this calls milkCow(), which lives in cow.uc
	else if (event == SCRIPTED)
	{
		if (framenum == FRAME_PITCHER)
		{
			target = UI_click_on_item();
			if (target->get_item_shape() == SHAPE_COW)
				gotoCow(target, MILK_WITH_PITCHER);
			else
				randomPartySay("@Thou couldst always milk a cow to fill yon pitcher... if thou don't mind getting thy hands dirty, that is!@");
		}
	}
}
