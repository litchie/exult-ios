/*
 *	This source file contains the code for innkeepers reclaiming the Inn Keys.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

//Inn identifiers:
const int INN_WAYFARERS_NORTH			= 1;
const int INN_WAYFARERS_SOUTH			= 2;
const int INN_FALLEN_VIRGIN				= 3;
const int INN_OUT_N_INN					= 4;
const int INN_BUNK_AND_STOOL			= 5;
const int INN_MODEST_DAMSEL				= 6;
const int INN_SALTY_DOG					= 7;
const int INN_HONORABLE_HOUND			= 8;
const int INN_CHEQUERED_CORK_EAST		= 9;
const int INN_CHEQUERED_CORK_SOUTH		= 10;

//To make innkeeper move towards Avatar:
const int EVENT_WALKING					= 12;

//For locking doors:
enum door_states
{
	DOOR_UNLOCKED						= 0,
	DOOR_OPEN							= 1,
	DOOR_LOCKED							= 2,
	DOOR_MAGIC_LOCK						= 3
};

//For making beds:
enum bed_states
{
	BED_MADE							= 0,
	BED_UNMADE							= 1
};

eggLockInnDoors 0xBFC ()
{
	var polite_title = getPoliteTitle();
	var inn_keepers = [JAMES, JAMES, MANDY, PAMELA, OPHELIA, BORIS, POLLY,
					  APOLLONIA, RUTHERFORD, RUTHERFORD];
	var inn_names = ["Wayfarer's Inn", "Wayfarer's Inn", "Fallen Virgin",
					 "Out 'n' Inn", "Bunk and Stool", "Modest Damsel",
					 "Salty Dog", "Honorable Hound", "Chequered Cork", "Chequered Cork"];
	var step_directions = [SOUTH, NORTH, NORTH, NORTHEAST, WEST, WEST,
						   NORTH, WEST, WEST, NORTH];
	
	//Get the innkeeper's index:
	var egg_quality = get_item_quality();
	var inn_keeper = inn_keepers[egg_quality];
	
	if (event == EVENT_WALKING)
	{
		//The innkeeper should have started moving towards the
		//Avatar; give him a boost:
		var keeper_pos = inn_keeper->get_object_position();
		var avatar_pos = AVATAR->get_object_position();
		//Vector from innkeeper to Avatar 
		var delta_pos = [keeper_pos[X] - avatar_pos[X],
						 keeper_pos[Y] - avatar_pos[Y],
						 keeper_pos[Z] - avatar_pos[Z]];
		//Length of said vector:
		var len = inn_keeper->get_distance(AVATAR);
		//Renormalized vector with length 5:
		delta_pos = [(5 * delta_pos[X]) / len,
					 (5 * delta_pos[Y]) / len,
					 (5 * delta_pos[Z]) / len];
		//Innkeeper's final destination:
		var dest_pos = [avatar_pos[X] + delta_pos[X],
						avatar_pos[Y] + delta_pos[Y],
						avatar_pos[Z] + delta_pos[Z]];
		
		//Force innkeeper to move to Avatar (or call usecode
		//anyway if not possible):
		inn_keeper->si_path_run_usecode(dest_pos, PATH_SUCCESS, item, eggLockInnDoors, true);
		//LEAVE:
		abort;
	}
	
	//Get the egg's position:
	var pos = get_object_position();
	
	//Use the egg's position to determine the offsets to the doors
	// and beds
	if (egg_quality == INN_WAYFARERS_NORTH)
		pos[Y] = pos[Y] - 15;
	else if (egg_quality == INN_OUT_N_INN)
	{
		pos[X] = pos[X] + 8;
		pos[Y] = pos[Y] - 16;
	}
	else if (egg_quality == INN_BUNK_AND_STOOL)
	{
		pos[X] = pos[X] - 48;
		pos[Y] = pos[Y] - 8;
	}
	else if (egg_quality == INN_MODEST_DAMSEL)
		pos[X] = pos[X] - 24;
	else if (egg_quality == INN_SALTY_DOG)
	{
		pos[X] = pos[X] - 18;
		pos[Y] = pos[Y] - 16;
	}
	else if (egg_quality == INN_HONORABLE_HOUND)
		pos[Y] = pos[Y] + 27;
	else if (egg_quality == INN_CHEQUERED_CORK_EAST)
	{
		pos[X] = pos[X] - 47;
		pos[Y] = pos[Y] + 9;
	}
	else if (egg_quality == INN_CHEQUERED_CORK_SOUTH)
	{
		pos[X] = pos[X] - 7;
		pos[Y] = pos[Y] - 15;
	}
	
	var index;
	var max;
	var key;
	//Find all inn keys owner by the party:
	var inn_keys = PARTY->count_objects(SHAPE_KEY, KEY_INN, FRAME_ANY);
	//Find all keys in the ground:
	var ground_keys = pos->find_nearby(SHAPE_KEY, 50, MASK_NONE);
	for (key in ground_keys with index to max)
	{
		//See how many are inn keys:
		if (key->get_item_quality() == KEY_INN)
			inn_keys = inn_keys + 1;
	}
	
	if (inn_keys)
	{
		//If there are any inn keys at all,
		if (UI_is_pc_inside() || (event == PATH_SUCCESS))
		{
			//And if the Avatar is inside OR the innkeeper reached the Avatar,
			if (event == EGG)
			{
				//If the Avatar is inside and innkeeper hasn't started
				//moving towards him yet
				//Halt scripts for Avatar and innkeeper:
				AVATAR->halt_scheduled();
				inn_keeper->halt_scheduled();
				//Freeze Avatar:
				AVATAR->trueFreeze();
				
				//Start to move the innkeeper towards the Avatar:
				inn_keeper->approach_avatar(0, 0);
				
				var delay = inn_keeper->get_distance(AVATAR) / 5;
				//Innkeeper calls out to Avatar:
				script inn_keeper after (delay - 2) ticks
				{	nohalt;						call trueFreeze;
					say "@Hold on a bit...@";	call trueUnfreeze;}
				
				//Avatar replies:
				script AVATAR after 2 * delay ticks
				{	face AVATAR->direction_from(inn_keeper);
					wait 2;						say "@Yes?@";}
				
				//After a little while, call this function again:
				script item after 4 ticks
				{	call eggLockInnDoors, EVENT_WALKING;}
				
				abort;
			}
			
			else if (event == PATH_SUCCESS)
			{
				//The innkeeper has reached destination (or gave up
				//trying and called the usecode anyway...)
				inn_keeper.say("@I take it that thou art checking out then, " + polite_title + "?@");
				//Ask Avatar is he is checking out:
				if (askYesNo())
				{
					//Yes he is;
					say("@Here, let me have the room keys then. Worry not, I shall lock the doors myself.@");
					say("@I hope thou didst enjoy thy stay at the " + inn_names[egg_quality] + "!@");
					
					//Unfreeze Avatar:
					AVATAR->trueUnfreeze();
					
					//Make innkeeper revert to normal schedule and unfreeze him:
					inn_keeper->run_schedule();
					script inn_keeper after 2 ticks
					{	nohalt;						call trueUnfreeze;
						actor frame STAND;			say "@Do come back!!@";}
				}
				else
				{
					//No, he is not;
					say("@Come back inside, then, and enjoy thy room, " + polite_title + "!@");
					var step_to = step_directions[egg_quality];
					//Halt scripts:
					AVATAR->halt_scheduled();
					
					//Unfreeze innkeeper:
					script inn_keeper after 2 ticks
					{	nohalt;						call trueUnfreeze;
						actor frame STAND;			say "@Enjoy thy stay!@";}
					
					//Determine were the Avatar is going to:
					pos = get_object_position();
					var dir = step_directions[egg_quality];
					if (dir == NORTH)
						pos[Y] = pos[Y] - 8;
					else if (dir == EAST)
						pos[X] = pos[X] + 8;
					else if (dir == SOUTH)
						pos[Y] = pos[Y] + 8;
					else if (dir == WEST)
						pos[X] = pos[X] - 8;
					
					//Force Avatar back inside the inn:
					AVATAR->si_path_run_usecode(pos, PATH_SUCCESS, AVATAR, trueUnfreeze, true);
					UI_set_path_failure(trueUnfreeze, AVATAR, PATH_FAILURE);
					abort;
				}
			}
		}
	}
	
	//Remove inn keys from party:
	inn_keys->remove_party_items(SHAPE_KEY, KEY_INN, FRAME_ANY, true);
	//Delete innkeys from the ground:
	for (key in ground_keys with index to max)
	{
		if (key->get_item_quality() == KEY_INN)
			key->remove_item();
	}

	
	var inn_doors = [];
	var door_shapes = [
			SHAPE_DOOR_HORIZONTAL,
			SHAPE_DOOR_VERTICAL,
			SHAPE_DOOR2_HORIZONTAL,
			SHAPE_DOOR2_VERTICAL
			];
	
	var door;
	//Find unlocked doors
	for (door in door_shapes with index to max)
		inn_doors = inn_doors & pos->find_nearby(door, 20, MASK_NONE);
	
	var door_state;
	var door_function;

	for (door in inn_doors with index to max)
	{
		//For each nearby door,
		if (door->get_item_quality() == KEY_INN)
		{
			//If it is in inn door,
			door_state = (door->get_item_frame() % 4);
			if (door_state == DOOR_OPEN)
			{
				//and if it is open,
				//get usecode function #:
				door_function = door->get_usecode_fun();
				
				//Set event to DOUBLECLICK:
				event = DOUBLECLICK;
				//Close door; this is an indirect calle:
				door->(*door_function)();
				//Lock door
				UseKeyOnDoor(door);
			}
	
			else if (door_state == DOOR_UNLOCKED)
				//If the door is closed, lock it:
				UseKeyOnDoor(door);
		}
	}
	
	//Find all nearby beds:
	var inn_beds = pos->find_nearby(SHAPE_BED_HORIZONTAL, 20, MASK_NONE);
	inn_beds = inn_beds & pos->find_nearby(SHAPE_BED_VERTICAL, 20, MASK_NONE);
	var bed;
	var bed_state;
	for (bed in inn_beds with index to max)
	{
		//For each bed found,
		bed_state = bed->get_item_frame();
		if (bed_state > 2)
		{
			//If the bed is actually a sheet, get its state:
			bed_state = (bed_state - 3) % 2;
			if (bed_state == BED_UNMADE)
				//Bed not made, so make it:
				bed->set_item_frame(bed->get_item_frame() - 1);
		}
	}
}
