const int INN_SLEEPING_SOLDIER			= 1;
const int INN_BROKEN_OAR_SOUTHEAST		= 2;
const int INN_BROKEN_OAR_EAST			= 3;
const int INN_BROKEN_OAR_WEST			= 4;
const int INN_SLEEPING_BULL_MAIN		= 5;
const int INN_SLEEPING_BULL_NORTH		= 6;
const int INN_SLEEPING_BULL_EAST_S		= 7;
const int INN_SLEEPING_BULL_EAST_N		= 8;
const int INN_BLUE_BOAR					= 9;

const int EVENT_WALKING					= 20;

enum door_states
{
	DOOR_UNLOCKED						= 0,
	DOOR_OPEN							= 1,
	DOOR_LOCKED							= 2,
	DOOR_MAGIC_LOCK						= 3
};
enum bed_states
{
	BED_MADE							= 0,
	BED_UNMADE							= 1
};

extern doorHorizontal 270 ();
extern doorVertical 376 ();
extern UseKeyOnDoor 0x281 ();

const int SHAPE_BED_HORIZONTAL			= 696;
const int SHAPE_BED_VERTICAL			= 1011;

eggLockInnDoors 0xCB0 ()
{
	var egg_quality = UI_get_item_quality(item);
	if (gflags[BANES_RELEASED] || ((egg_quality == INN_SLEEPING_SOLDIER) && gflags[GOBLIN_SIMON_DEAD]))
	{
		UI_remove_item(item);
		abort;
	}
	
	var inn_keepers = [SIMON,
					   JENDON, JENDON, JENDON,
					   DEVRA, ARGUS, DEVRA, ARGUS,
					   PETRA];
	var inn_names = ["Blue Boar Inn",
					 "Broken Oar Inn", "Broken Oar Inn", "Broken Oar Inn",
					 "Sleeping Bull Inn", "Sleeping Bull Inn", "Sleeping Bull Inn", "Sleeping Bull Inn",
					 "Blue Boar Inn"];
	var step_directions = [NORTH,
						   WEST, WEST, EAST,
						   EAST, SOUTH, WEST, WEST,
						   WEST];
	var qualities = [74,
					 11, 11, 11,
					 3, 3, 3, 3,
					 184];
	
	var dist = 30;
	var inn_keeper = inn_keepers[egg_quality];
	var polite_title = getPoliteTitle();

	if (event == EVENT_WALKING)
	{
		if (UI_get_distance(inn_keeper, AVATAR) < 10)
		{
			script item after 2 ticks
			{	call eggLockInnDoors, STARTED_TALKING;}
			
			script inn_keeper
			{	call freeze;				face UI_direction_from(inn_keeper, AVATAR);
				actor frame STAND;}
		}
		
		else
		{
			//Just to ensure that the innkeeper *will* reach the Avatar:
			UI_approach_avatar(inn_keeper, 0, 0);
			script item after 2 ticks
			{	call eggLockInnDoors, EVENT_WALKING;}
		}
		abort;
	}
	
	var pos = UI_get_object_position(item);

	if (egg_quality == INN_SLEEPING_SOLDIER)
	{
		pos[X] = pos[X] - 8;
		pos[Y] = pos[Y] - 16;
	}
	else if (egg_quality == INN_BROKEN_OAR_SOUTHEAST)
	{
		pos[X] = pos[X] - 16;
		pos[Y] = pos[Y] + 7;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_BROKEN_OAR_EAST)
	{
		pos[X] = pos[X] - 15;
		pos[Y] = pos[Y] + 25;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_BROKEN_OAR_WEST)
	{
		pos[X] = pos[X] + 16;
		pos[Y] = pos[Y] + 25;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_SLEEPING_BULL_MAIN)
	{
		pos[X] = pos[X] + 9;
		pos[Y] = pos[Y] - 19;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_SLEEPING_BULL_NORTH)
	{
		pos[X] = pos[X] + 16;
		pos[Y] = pos[Y] + 20;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_SLEEPING_BULL_EAST_S)
	{
		pos[X] = pos[X] - 38;
		pos[Y] = pos[Y] - 19;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_SLEEPING_BULL_EAST_N)
	{
		pos[X] = pos[X] - 38;
		pos[Y] = pos[Y] - 3;
		pos[Z] = 6;
	}
	else if (egg_quality == INN_BLUE_BOAR)
	{
		pos[X] = pos[X] - 46;
		pos[Y] = pos[Y] + 8;
	}
	
	var index;
	var max;
	var key;
	var inn_keys = UI_count_objects(PARTY, SHAPE_KEY, qualities[egg_quality], FRAME_ANY);
	var ground_keys = UI_find_nearby(pos, SHAPE_KEY, 50, 0);
	for (key in ground_keys with index to max)
	{
		if (UI_get_item_quality(key) == qualities[egg_quality])
			inn_keys = inn_keys + 1;
	}

	var dir;
	
	if (inn_keys || UI_is_on_keyring(qualities[egg_quality]))
	{
		var msg;
		if (event == EGG)
		{
			UI_halt_scheduled(AVATAR);
			AVATAR->freeze();
			UI_halt_scheduled(inn_keeper);
			UI_approach_avatar(inn_keeper, 0, 0);
			
			if (UI_is_pc_inside())
				msg = "@Yes?@";
			else
				msg = "@Oops...@";
			
			script AVATAR after 4 ticks
			{	face UI_direction_from(AVATAR, inn_keeper);
				wait 2;						say msg;}

			if (UI_is_pc_inside())
				msg = "@Hold on a bit...@";
			else
				msg = "@There thou art!@";
			
			script inn_keeper after 2 ticks
			{	nohalt;				say msg;}
			
			script item after 2 ticks
			{	call eggLockInnDoors, EVENT_WALKING;}
			
			abort;
		}
		
		else if (event == STARTED_TALKING)
		{
			if (UI_is_pc_inside())
			{
				inn_keeper.say("@I take it that thou art checking out then, " + polite_title + "?@");
				if (askYesNo())
				{
					say("@Here, let me have the room keys then. Worry not, I shall lock the doors myself.@");
					say("@I hope thou didst enjoy thy stay at the " + inn_names[egg_quality] + "!@");
					msg = "@Do come back!!@";
				}
				else
				{
					say("@Come back inside, then, and enjoy thy room, " + polite_title + "!@");

					UI_run_schedule(inn_keeper);
					script inn_keeper after 2 ticks
					{	nohalt;						call unfreeze;
						actor frame STAND;			say "@Enjoy thy stay!@";}

					UI_halt_scheduled(AVATAR);
					pos = UI_get_object_position(item);
					dir = step_directions[egg_quality];
					if (dir == NORTH)
						pos[Y] = pos[Y] - 8;
					else if (dir == EAST)
						pos[X] = pos[X] + 8;
					else if (dir == SOUTH)
						pos[Y] = pos[Y] + 8;
					else if (dir == WEST)
						pos[X] = pos[X] - 8;
	
					UI_si_path_run_usecode(AVATAR, pos, PATH_SUCCESS, AVATAR, unfreeze, true);
					UI_set_path_failure(unfreeze, AVATAR, PATH_FAILURE);
					abort;
				}
			}
			else
			{
				inn_keeper.say("@Where didst thou go carrying my keys? Here, let me have them at once!~@Thank thee. Please don't -ever- do this again!@");
				msg = "@Don't do this again!@";
			}
			
			AVATAR->unfreeze();
			UI_halt_scheduled(AVATAR);

			UI_run_schedule(inn_keeper);
			script inn_keeper after 2 ticks
			{	nohalt;						call unfreeze;
				actor frame STAND;			say msg;}
		}
	}
	
	UI_remove_party_items(inn_keys, SHAPE_KEY, qualities[egg_quality], FRAME_ANY, true);
	if (UI_is_on_keyring(qualities[egg_quality])) UI_remove_from_keyring(qualities[egg_quality]);
	
	for (key in ground_keys with index to max)
	{
		if (UI_get_item_quality(key) == qualities[egg_quality])
			UI_remove_item(key);
	}

	
	var inn_doors = [];
	var door_shapes = [SHAPE_DOOR_HORIZONTAL, SHAPE_DOOR_VERTICAL];
	
	var door;
	for (door in door_shapes with index to max)
		inn_doors = inn_doors & UI_find_nearby(pos, door, dist, 0);
	
	var door_state;
	var door_function;

	for (door in inn_doors with index to max)
	{
		event = DOUBLECLICK;
		if (UI_get_item_quality(door) == qualities[egg_quality])
		{
			door_state = (UI_get_item_frame(door) % 4);
			if (door_state == DOOR_OPEN)
			{
				door_function = UI_get_item_shape(door);
				if (door_function == SHAPE_DOOR_HORIZONTAL)
					door->doorHorizontal();
				else if (door_function == SHAPE_DOOR_VERTICAL)
					door->doorVertical();
				
				UI_set_intercept_item(door);
				door->UseKeyOnDoor();
			}
	
			else if (door_state == DOOR_UNLOCKED)
			{
				UI_set_intercept_item(door);
				door->UseKeyOnDoor();
			}
		}
	}
	
	var inn_beds = UI_find_nearby(pos, SHAPE_BED_HORIZONTAL, dist, 0);
	inn_beds = inn_beds & UI_find_nearby(pos, SHAPE_BED_VERTICAL, dist, 0);
	var bed;
	var bed_state;
	for (bed in inn_beds with index to max)
	{
		bed_state = UI_get_item_frame(bed);
		if (bed_state > 2)
		{
			bed_state = (bed_state - 3) % 2;
			if (bed_state == BED_UNMADE)
				UI_set_item_frame(bed, UI_get_item_frame(bed) - 1);
		}
	}
}
