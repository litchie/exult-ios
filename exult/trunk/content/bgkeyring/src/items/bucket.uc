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

/* Bucket behaviours */

//The sound made when a bucket is emptied/filled
const int SOUND_BUCKET_EMPTY = 40;

//Rewrote Bucket to offload specific target behaviours to separate functions. 
//New behaviour functions must still be called from this function, by putting
//the appropriate code in the SCRIPTED code block.

//reimplementation of internal function - creates blood/winestains
//Added in special stain for milk
makeSpill object#() ()
{
	var bucket_frame = get_item_frame();
	var min_frame = 0;
	var max_frame = 0;

	if (bucket_frame == FRAME_BUCKET_WATER)
	{
		min_frame = 16;
		max_frame = 19;
	}
	else if (bucket_frame == FRAME_BUCKET_BLOOD)
	{
		min_frame = 0;
		max_frame = 3;
	}
	else if (bucket_frame == FRAME_BUCKET_WINE)
	{
		min_frame = 20;
		max_frame = 23;
	}
	else if (bucket_frame in [FRAME_BUCKET_BEER, FRAME_BUCKET_BEER2, FRAME_BUCKET_MILK])
	{
		min_frame = 12;
		max_frame = 15;
	}
	else if (bucket_frame == FRAME_WATERINGCAN)
		return;

	var avatar_pos = AVATAR->get_object_position();
	var target_pos = [avatar_pos[X], avatar_pos[Y] - 1, 0];

	var spill = UI_create_new_object(SHAPE_SPILL);
	spill->set_item_frame(UI_die_roll(min_frame, max_frame));	//choose a random frame
	spill->set_item_flag(TEMPORARY);	//added to the original, prevents them sticking around FOREVER AND EVER

	UI_update_last_created(target_pos);

	//for FV quest: decrement the quality (ie, remaining spills) of the bucket
	var bucket_quality = get_item_quality();
	if(bucket_frame == FRAME_BUCKET_BLOOD && bucket_quality)
		set_item_quality(bucket_quality - 1);

	//switch the bucket to the empty frame
	else set_item_frame(FRAME_BUCKET_EMPTY);

	//ffssshhhh
	UI_play_sound_effect(SOUND_BUCKET_EMPTY);
}


//Bucket was used on an NPC (this behaviour was moved from Bucket() to
//aid code organisation). Todo: use event to specify what kind of bucket
//was used - so we can avoid 'accidental' dousings in blood
//Updated 2005-03-18 to a) wake NPC up and b) make sure NPC can talk
//before making them bark
useBucketOnNPC ()
{
	//Note: item is the NPC clicked on

	//get the first bucket we can find that the avatar is carrying
	var bucket = AVATAR->get_cont_items(SHAPE_BUCKET, QUALITY_ANY, FRAME_ANY);
	var bucket_frame = bucket->get_item_frame();

	var direction = directionFromAvatar(item);
	var towards_avatar = invertDirection(direction);

	//wake the person up
	//Todo: this should happen after the dousing, rather than before
	//Note: most of the time it's impossible to actually get to an NPC
	//who's sleeping in a bed
	clear_item_flag(ASLEEP);

	var bark;				//what the NPC says when you try it on
	//make sure NPC can talk
	if (canTalk(item))
	{
		if (bucket_frame == FRAME_BUCKET_BLOOD)
			bark = "@Foul miscreant!@";
		else
			bark = "@Hey, stop that!@";
	}

	//Plays the avatar tossing water onto the NPC
	script AVATAR
	{
		face direction;
		actor frame SWING_3;
		actor frame USE;
		actor frame STAND;
	}

	//target getting peevish
	script item
	{
		face towards_avatar;
		wait 2;
		say bark;
		wait 5;
	}

	//empty the bucket
	//Added: water is now spilled!
	script bucket	{ wait 2; call makeSpill; }
}

//Bucket was used on a trough (this behaviour was moved from Bucket()
//to aid code organisation)
useBucketOnTrough ()
{
	var trough = false;
	//try to find the nearest horizontal trough
	trough = AVATAR->find_nearest(SHAPE_TROUGH_HORIZONTAL, 5);
	//if we couldn't find one, try the nearest vertical trough
	if (!trough)
		trough = AVATAR->find_nearest(SHAPE_TROUGH_VERTICAL, 5);
	//no available trough, alas!
	if (!trough)
		return;

	var bucket_frame = get_item_frame();
	var trough_frame = trough->get_item_frame();
	var trough_new_frame;	//the trough's frame after the bucket has been used on it
	var bucket_new_frame;	//the bucket's frame after it was used

	//bucket is full of water, fill the trough from the bucket
	if (bucket_frame == FRAME_BUCKET_WATER)
	{
		//trough is already full
		if (trough_frame == 3 || trough_frame == 7)
		{
			avatarBark("@The trough is full.@");
			return;
		}

		//increment the trough's fullness
		trough_new_frame = trough_frame + 1;
		bucket_new_frame = FRAME_BUCKET_EMPTY;
	}

	//bucket is empty, fill the bucket from the trough
	//(I can't believe someone got paid to implement this level of
	//interactivity.)
	else if (bucket_frame == FRAME_BUCKET_EMPTY)
	{
		//no more water for you!
		if (trough_frame == 0 || trough_frame == 4)
		{
			avatarBark("@The trough is empty.@");
			return;
		}
		
		//decrement the trough's fullness
		trough_new_frame = trough_frame - 1;
		bucket_new_frame = FRAME_BUCKET_WATER;
	}
	//bucket was full of something other than water, bail out
	else
		return;

	//lean down in the direction of the trough
	script AVATAR
	{
		face directionFromAvatar(trough);
		actor frame LEAN;
		wait 2;
		actor frame STAND;
	}

	//change the trough's frame
	script trough
	{
		wait 2;
		frame trough_new_frame;
		continue;
		sfx SOUND_BUCKET_EMPTY;
	}

	//change the bucket's frame
	script item	{ wait 2; frame bucket_new_frame; }
}

douseAnimation object#() ()
{
	var pos = get_object_position();
	var dest = [pos[X] - 3, pos[Y] - 4];
	UI_sprite_effect(ANIMATION_POOF, dest[X], dest[Y], 0, 0, 0, -1);
	UI_play_sound_effect(0x2E);
}

//Bucket was used on a dousable item
useBucketOnDousable ()
{
	//Note: item is the object clicked on

	//get the first bucket of water we can find in the player's inventory
	var bucket = AVATAR->get_cont_items(SHAPE_BUCKET, QUALITY_ANY, FRAME_BUCKET_WATER);
	var bucket_frame = bucket->get_item_frame();

	var target_shape = get_item_shape();
	var target_frame = get_item_frame();

	var target_direction = directionFromAvatar(item);
	var target_pos = get_object_position();

	if (target_shape == SHAPE_FIREPIT)
	{
		if (target_frame == 4)
		{
			avatarBark("@There are only coals.@");
			return;
		}
		else if (target_frame == 7)
			script item
			{	call douseAnimation;			continue;
				previous frame cycle;			previous frame cycle;
				previous frame cycle;}
		else if (target_frame == 6)
			script item
			{	call douseAnimation;			continue;
				previous frame cycle;			previous frame cycle;}
		else if (target_frame == 5)
			script item
			{	call douseAnimation;			continue;
				previous frame cycle;}

		script AVATAR
		{	face target_direction;			actor frame LEAN;
			wait 2;							actor frame STAND;}
	}

	else if (target_shape == SHAPE_LIGHTSOURCE_LIT || target_shape == SHAPE_SCONCE_LIT || target_shape == SHAPE_TORCH_LIT)
	{
		var sprite_offset;
		//the last style of light source has some special behaviour (FV item?)
		if (target_frame == 16)
		{
			sprite_offset = 2;
			script AVATAR
			{	face target_direction;			actor frame SWING_3;
				actor frame STAND;				wait 1;
				say "@I can't douse it.@";}
		}
		else
		{
			sprite_offset = 3;
			var lightsource_quality = get_item_quality();
		 	var new_shape;

			//determine the correct doused shape
			if (target_shape == SHAPE_LIGHTSOURCE_LIT)
				new_shape = SHAPE_LIGHTSOURCE;
			else if	(target_shape == SHAPE_SCONCE_LIT)
				new_shape = SHAPE_SCONCE;
			else if	(target_shape == SHAPE_TORCH_LIT)
				new_shape = SHAPE_TORCH;

			//remove the old lightsource and put a new one in its place
			remove_item();

			var new_lightsource = UI_create_new_object(new_shape);
			new_lightsource->set_item_quality(lightsource_quality);
			new_lightsource->set_item_frame(target_frame);
			UI_update_last_created(target_pos);

			//play lean-down animation?
			script AVATAR
			{	face target_direction;			actor frame SWING_3;
				actor frame USE;				actor frame STAND;}
				
		}

		//place the extinguishing effect relative to the elevation of the
		//light source. Todo: separate this as a generic function, since
		//I can see this being used often for sprite effects
		var sprite_pos;
		sprite_offset = sprite_offset + (target_pos[Z] / 2);
		sprite_pos[X] = (target_pos[X] - sprite_offset);
		sprite_pos[Y] = (target_pos[Y] - sprite_offset);

		//display the puff of smoke
		UI_sprite_effect(9, sprite_pos[X], sprite_pos[Y], 0, 0, 0, -1);

		//tfssss
		UI_play_sound_effect(0x2E);
	}

	else if (target_shape == SHAPE_CAMPFIRE)
	{
		if (target_frame == 0)
		{
			avatarBark("@There are only coals.@");
			return;
		}

		//snuff out the fire
		script item frame 0;

		//bend down?
		script AVATAR
		{	face target_direction;			actor frame LEAN;
			wait 2;							actor frame STAND;}

		UI_sprite_effect(9, target_pos[X], target_pos[Y], 0, 0, 0, -1);
		UI_play_sound_effect(0x2E);
	}
}

//Bucket of water was used on dough flour
//Note: this doesn't actually decrement the bucket's quality or
//anything, which means you can use the bucket as many times as
//you like
useBucketOnDough ()
{
	//The dough has already been watered
	if (get_item_frame() != 0)
		return;

	//lean down in direction of dough?
	script AVATAR
	{	face directionFromAvatar(item);
		actor frame LEAN;					wait 2;
		actor frame STAND;}
	set_item_frame(FRAME_DOUGH_BALL);
}

fillBucketFromWell object#() ()
{
	var well = AVATAR->find_nearest(SHAPE_WELL, 10);
	if (well)
	{
		var well_frame = well->UI_get_item_frame() - 6;
		script well
		{	next frame cycle;		wait 1;
			next frame cycle;		wait 1;
			next frame cycle;		wait 1;
			next frame cycle;		wait 1;
			next frame cycle;		wait 2;
			frame well_frame;}
		script AVATAR
		{	actor frame USE;		wait 1;
			actor frame SWING_2;	wait 1;
			actor frame SWING_1;	wait 1;
			actor frame SWING_3;	wait 1;
			actor frame USE;		face EAST;
			wait 1;					actor frame SWING_3;
			wait 2;					actor frame STAND;}
		
		script item frame WALK_1_NORTH;
		UI_play_sound_effect(0x28);
	}
}

//Bucket of water was used on well
useBucketOnWell ()
{
	//find the nearest well
	var well = AVATAR->find_nearest(SHAPE_WELL, 10);
	var well_frame = well->get_item_frame();

	//well is of the regular style
	if (well_frame >= 0 && well_frame <= 11)
		well_frame = 1;

	//well is a golden FV jobbie
	else if (well_frame >= 12 || well_frame <= 23)
		well_frame = 13;

	//animate the well
	script well
	{	wait 1;						frame well_frame;
		wait 2;						next frame cycle;
		wait 1;						next frame cycle;
		wait 1;						next frame cycle;
		wait 1;						next frame cycle;
		wait 1;						next frame cycle;}

	//animate the avatar
	script AVATAR
	{	face EAST;					actor frame SWING_3;
		wait 2;						face SOUTH;
		wait 1;						actor frame SWING_1;
		wait 1;						actor frame SWING_2;
		wait 1;						actor frame USE;
		wait 1;						actor frame SWING_3;
		wait 4;}

	//place the full bucket nearby
	script item
	{	wait 17;					call fillBucketFromWell;}
}

useBucketOnGround ()
{
	script AVATAR
	{	face NORTH;					actor frame LEAN;
		wait 3;						actor frame STAND;}
	script item
	{	wait 3;						call makeSpill;}
}

Bucket shape#(0x32A) ()
{
	//User doubleclicked on bucket: march them to the bucket and pick
	//it up, then call this function as event = SCRIPTED
	if (event == DOUBLECLICK)
	{
		//watering cans don't do anything
		if (get_item_frame() == FRAME_WATERINGCAN)
			return;
		else
			gotoAndGet(item);
	}

	//This event level is triggered when the bucket is carried by the
	//avatar and is now ready to be used on something (it shows the
	//use cursor)
	if (event == SCRIPTED)
	{
		UI_close_gumps();

		var bucket_frame = get_item_frame();

		var target = UI_click_on_item();
		var target_shape = target->get_item_shape();
		var target_frame = target->get_item_frame();
		var target_offsetx;
		var target_offsety;

		//Player clicked on him/herself
		if (isAvatar(target))
		{
			if (bucket_frame == FRAME_BUCKET_BLOOD)
				avatarBark("@No, thank thee.@");
			else if (bucket_frame == FRAME_BUCKET_EMPTY)
				avatarBark("@The bucket is empty.@");
			else
			{
				avatarBark("@Ahhh, how refreshing.@");
				//Empty the bucket
				set_item_frame(0);
			}
			return;
		}

		//Player clicked on an NPC
		else if (target->is_npc())
		{
			//Special behaviour for cows
			if (target_shape == SHAPE_COW)
			{
				if (bucket_frame == FRAME_BUCKET_EMPTY)
					gotoCow(target, MILK_WITH_BUCKET);
				else
				{
					avatarBark("@The bucket is full.@");
					return;
				}
			}
			else
			{
				if (bucket_frame == FRAME_BUCKET_EMPTY)
				{
					avatarBark("@The bucket is empty.@");
					return;
				}

				target_offsetx = [0, 2, 0, -2];
				target_offsety = [2, 0, -2, 0];

				script target wait 50;
				gotoObject(target, target_offsetx, target_offsety, 0, useBucketOnNPC, target, SCRIPTED);
			}
		}

		//Player clicked on a churn
		else if (target_shape == SHAPE_KITCHEN_ITEM && target_frame == FRAME_CHURN)
		{
			if (bucket_frame == FRAME_BUCKET_MILK)
				gotoChurn(target, CHURN_WITH_BUCKET);
			else if (bucket_frame == FRAME_BUCKET_EMPTY)
			{
				avatarBark("@The bucket is empty.@");
				return;
			}
			else
				return;
		}

		//Player clicked on a trough
		else if (target_shape in [SHAPE_TROUGH_HORIZONTAL, SHAPE_TROUGH_VERTICAL])
		{
			if (!(bucket_frame in [FRAME_BUCKET_EMPTY, FRAME_BUCKET_WATER]))
				return;

			if (target_shape == SHAPE_TROUGH_HORIZONTAL)
			{
				target_offsetx = [-1, -2, -1, -2, 1, 1, -4, -4];
				target_offsety = [1, 1, -2, -2, 0, -1, 0, -1];
			}
			else
			{
				target_offsetx = [1, 1, -2, -2, 0, -1, 0, -1];
				target_offsety = [-1, -2, -1, -2, 1, 1, -4, -4];
			}
			gotoObject(target, target_offsetx, target_offsety, 0, useBucketOnTrough, item, SCRIPTED);
		}

		//player clicked on a firepit
		else if (target_shape == SHAPE_FIREPIT)
		{
			//Now this is odd...only golden firepits can be used this
			//way. It must be a FV quest thing.
			if (target->get_item_frame() < 4 || target->get_item_frame() > 7)
				return;

			if (bucket_frame == FRAME_BUCKET_EMPTY)
			{
				avatarBark("@The bucket is empty.@");
				return;
			}
			else if (bucket_frame != FRAME_BUCKET_WATER)
				return;

			target_offsetx = [-1, -2, 1, 1, -1, -2, -4, -4];
			target_offsety = [1, 1, -1, -2, -4, -4, -1, -2];

			gotoObject(target, target_offsetx, target_offsety, 0, useBucketOnDousable, target, SCRIPTED);
		}

		//player clicked on dousable lightsource
		else if (target_shape in [SHAPE_LIGHTSOURCE_LIT, SHAPE_SCONCE_LIT, SHAPE_TORCH_LIT, SHAPE_CAMPFIRE])
		{
			if (bucket_frame == FRAME_BUCKET_EMPTY)
			{
				avatarBark("@The bucket is empty.@");
				return;
			}
			else if (bucket_frame != FRAME_BUCKET_WATER)
				return;

			target_offsetx = [2, 0, -2, 0];
			target_offsety = [0, 2, 0, -2];

			gotoObject(target, target_offsetx, target_offsety, -5, useBucketOnDousable, target, SCRIPTED);
		}

		//player clicked on some flour
		else if (target_shape == SHAPE_DOUGH)
		{
			if (bucket_frame == FRAME_BUCKET_EMPTY)
			{
				avatarBark("@The bucket is empty.@");
				return;
			}
			else if (bucket_frame != FRAME_BUCKET_WATER)
				return;

			target_offsetx = [2, 0, -2, 0];
			target_offsety = [0, 2, 0, -2];

			gotoObject(target, target_offsetx, target_offsety, -5, useBucketOnDough, target, SCRIPTED);
		}


		//player clicked on well/well base
		else if (target_shape in [SHAPE_WELL, SHAPE_WELLBASE])
		{
			if (bucket_frame != FRAME_BUCKET_EMPTY)
			{
				avatarBark("@The bucket is full.@");
				return;
			}

			//they clicked on the well assembly - find the nearest
			//base instead
			if (target_shape == SHAPE_WELL)
			{
				target = target->find_nearest(SHAPE_WELLBASE, 3);
				if (!target)
					return;
			}

			target_offsetx = [-5, -5];
			target_offsety = [-1, -1];
			gotoObject(target, target_offsetx, target_offsety, 0, useBucketOnWell, item, SCRIPTED);
		}
		
		//player clicked on a rock (this is for FV quest) or on
		//the ground
		else if (target_shape == SHAPE_ROCK || target[1] == 0)
		{
			if (bucket_frame == FRAME_BUCKET_EMPTY)
			{
				avatarBark("@The bucket is empty.@");
				return;
			}

			//This strips the object reference from target, making
			//it a regular X,Y,Z coordinate array
			target = removeFromArray(target, target[1]);

			target[Y] = target[Y] + 1;
			UI_path_run_usecode(target, useBucketOnGround, item, SCRIPTED);
		}
	}
}
