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

/* Butter-churning behaviours */

//Create the new butter object, beside the churn
createButter ()
{
	//create the new butter object
	var butter = UI_create_new_object(SHAPE_FOOD);
	if (!butter)
		return;
	butter->set_item_frame(FRAME_BUTTER);

	butter->set_item_flag(TEMPORARY);
	butter->set_item_flag(OKAY_TO_TAKE);

	var target_pos = get_object_position();
	//place it on the middle right-hand side of the churn
	target_pos[X] = target_pos[X] + 1;
	//target_pos[Y] = target_pos[Y] - 3;

	UI_update_last_created(target_pos);

	//Get a random party member to say something nice about butter
	if (UI_get_random(10) > 5)
		randomPartyBark("@Mmmm, rich creamery butter.@");
}

churnButter ()
{
	//the source of the milk (either milk bottle or bucket of milk)
	var source;

	//churned from bottle of milk
	if (event == CHURN_WITH_BOTTLE)
	{
		//Remove milk from the party's stash
		source = AVATAR->get_cont_items(SHAPE_BOTTLE, QUALITY_ANY, FRAME_MILK);
		source->remove_item();
	}
	//churned from bucket of milk
	else if (event == CHURN_WITH_BUCKET)
	{
		source = AVATAR->get_cont_items(SHAPE_BUCKET, QUALITY_ANY, FRAME_BUCKET_MILK);
		//empty the bucket
		source->set_item_frame(FRAME_BUCKET_EMPTY);
	}
	//churned from pitcher of milk
	else if (event == CHURN_WITH_PITCHER)
	{
		source = AVATAR->get_cont_items(SHAPE_KITCHEN_ITEM, QUALITY_ANY, FRAME_PITCHER_MILK);
		//empty the pitcher
		source->set_item_frame(FRAME_PITCHER);
	}

	//Play the avatar's animation
	script AVATAR
	{
		nohalt;						call freeze;
		face directionFromAvatar(item);

		//Lean down to put the milk into the churn
		actor frame STAND;			actor frame LEAN;			wait 3;
		actor frame STAND;			wait 1;

		//pump and grind
		repeat 3
		{	wait 1;						actor frame SWING_3;
			wait 1;						actor frame USE;
			wait 1;						actor frame SWING_2;};
		actor frame STAND;			call unfreeze;
	}

	//Play the churn animation
	script item
	{
		wait 7;	//wait for the avatar to get up from putting the milk in

		frame FRAME_CHURN;
		repeat 3
		{	sfx CHURN_SOUND;			frame FRAME_CHURN_3;
			wait 1;						frame FRAME_CHURN_2;
			wait 1;						frame FRAME_CHURN;};

		//now, create the butter next to the churn
		call createButter;
	}
}

//go to the churn (called by Bottle and Bucket)
gotoChurn (var churn, var milktype)
{
	//User tried to use a churn that was inside something
	if (churn->get_container())
	{
		randomPartySay("@Thou must first place the butter churn on the ground.@");
		return;
	}

	UI_close_gumps();

	var offsetx = [-1, -1, -1, 0, 0];
	var offsety = [0, 1, -1, 1, -1];

	//go to the churn and call churnButter() once you get there
	gotoObject(churn, offsetx, offsety, -3, churnButter, churn, milktype);
}

//This had to be overridden in order to separate the choosing of the target
//(now handled by UseEdible) from the act of eating the food
consumeEdible (var food, var npc, var nutritional_value, var sound_effect)
{
	if (!inParty(npc) || npc->get_item_flag(ASLEEP) || npc->get_item_flag(DEAD) ||
			npc->get_item_flag(PARALYZED))
		return;


	//Added in custom behavior for some NPCs, when faced with the daunting prospect
	//of milk products
	//In future, lactose-intolerance could be handled by a custom flag
	var shapenum = food->get_item_shape();
	var framenum = food->get_item_frame();
	var cheesy_frames = [FRAME_BUTTER, FRAME_CHEESE1, FRAME_CHEESE2, FRAME_CHEESE3];

	//Poor Iolo is lactose intolerant.
	if (npc == IOLO->get_npc_object() &&
		((shapenum == SHAPE_FOOD && framenum in cheesy_frames) ||
		(shapenum == SHAPE_BOTTLE && framenum == FRAME_MILK)))
	{
		npc->item_say("@Nay, I am lactose-intolerant.@");
		return;
	}

	//Jaana's just health-conscious. Tch, these healers.
	//Tweaked 2005-03-17: Jaana has decided that cheese is ok,
	//so she only objects to butter now
	else if (npc == JAANA->get_npc_object() &&
		shapenum == SHAPE_FOOD && framenum == FRAME_BUTTER)
	{
		//sanctimonious cow
		npc.say("@No thank thee, I am watching my cholesterol. As shouldst thou!@");
		return;
	}

	//The current hunger level of the character
	var npc_food_level = npc->get_npc_prop(FOODLEVEL);
	//What hunger level the character will be at if fed this item
	var level_after_feeding = npc_food_level + nutritional_value;

	//Character is full already, do not pass go
	if (npc_food_level >= FOODLEVEL_FULL)
	{
		npc->item_say("@No, thank thee.@");
		return;
	}

	stealItem(food);	//Make sure it has been properly nicked
	UI_play_sound_effect2(sound_effect, npc);	//Homph homph homph
	food->remove_item();


	var rand = UI_get_random(10);	//Used for randomising the barks

	//What the character will say when fed
	var bark;					

	//measure the NPC's current food level and their resulting food level to
	//determine barks

	//character was stuffed to start with
	if (npc_food_level >= FOODLEVEL_STUFFED)
	{
		if (framenum == FRAME_BUTTER && rand < 6)
			bark = "@Urgh, my arteries...@";

		//no idea why it's checking a global flag - was this a debug
		//flag to test barks or something?
		else if (rand > 4 || (gflags[0x9B] && rand < 2))
			bark = "@I'll soon be plump.@";
	}

	//character wasn't terribly hungry to start with
	else if (npc_food_level >= FOODLEVEL_PECKISH)
	{
		//chowed down on garlic
		if (shapenum == SHAPE_REAGENT)
			bark = "@Yum, garlic!@";

		//fed with something really filling
		else if (level_after_feeding >= FOODLEVEL_FULL && rand < 3)
			bark = "@Belch@";

		else
			bark = "@Ahh, very tasty.@";	
	}

	//for the following cases, the character was hungry to start with

	//character is now pretty full
	else if (level_after_feeding >= FOODLEVEL_STUFFED)
	{
		if (rand > 5)
			bark = "@That hit the spot!@";
		else
			bark = "@Burp@";	
	}

	//character is now happily fed
	else if (level_after_feeding >= FOODLEVEL_WELLFED)
	{
		if (shapenum == SHAPE_REAGENT)
			bark = "@Yum, garlic!@";
		else
			bark = "@Ah yes, much better.@";		
	}

	//character is still peckish
	else if (level_after_feeding >= FOODLEVEL_PECKISH)
	{
		if (rand > 5 || isAvatar(npc))
			bark = "@I am still hungry.@";
		else
			bark = "@May I have some more?@";	
	}

	//character is still starving
	else
	{
		if (rand > 5)
			bark = "@More!@";
		else
			bark = "@I must have more!@";
	}

	if (bark != "" && canTalk(npc))
		npc->item_say(bark);

	npc->set_npc_prop(FOODLEVEL, nutritional_value);
	return;
}

//Called when food is clicked on - picks target and then feeds
//the food to the target. This has been rewritten so that the
//actual eating is handled by another function - this just
//controls the click behavior
useEdible 0x813 (var sound_effect, var nutritional_value, var food)
{
	var npc = UI_click_on_item();
	consumeEdible(food, npc, nutritional_value, sound_effect);
	return;
}
