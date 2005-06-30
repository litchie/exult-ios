//externs:
extern spellFails 0x606 ();
extern createImbalanceFields 0x688 ();
extern serpentbondRemoveNPCsFromParty 0x7D6 ();
extern serpentbondAddNPCsBackToParty 0x7D7 ();

/*
	Fifth Circle Spells
*/

spellSurprise 0x667 ()
{
	var nearbynpcs;
	var party;
	var index;
	var max;
	var npc;
	var delay;
	var rand;
	
	if (event == DOUBLECLICK)
	{
		UI_halt_scheduled(item);
		UI_item_say(item, "@Ex Jux Hur@");
		if (notInMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_2H_3;
				actor frame SWING_3;		actor frame SWING_2H_2;
				sfx 20;						wait 5;
				call spellSurprise;}
			nearbynpcs = UI_find_nearby(item, SHAPE_ANY, 30, 8);
			party = UI_get_party_list();
			for (npc in nearbynpcs with index to max)
			{
				if (!(npc in party))
				{
					delay = (UI_get_distance(item, npc) + 15);
					script npc after delay ticks
					{	nohalt;						call spellSurprise;}
				}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2H_3;
				actor frame SWING_3;		actor frame SWING_2H_2;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		if (item == UI_get_npc_object(AVATAR))
		{
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0, -3, -3, 4, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0,  0, -4, 4, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0,  3, -3, 3, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0, -4,  0, 2, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0,  4,  0, 1, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0, -3,  3, 3, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0,  0,  4, 1, 25);
			UI_obj_sprite_effect(item, ANIMATION_CLOUDS, 0, 0,  3,  3, 1, 25);
		}
		else
		{
			rand = UI_get_random(3);
			if (rand == 1)
			{
				UI_set_schedule_type(item, IN_COMBAT);
				UI_set_attack_mode(item, FLEE);
				UI_set_opponent(item, AVATAR);
			}
			else if (rand == 2)
			{
				UI_halt_scheduled(item);
				UI_set_item_flag(item, ASLEEP);
			}
			else if (rand == 3)
			{
				UI_halt_scheduled(item);
				UI_set_item_flag(item, POISONED);
			}
		}
	}
}


/*
	Sixth Circle Spells
*/

spellCreateAmmo 0x66E ()
{
	var obj;
	var bows;
	var magicbows;
	var crossbows;
	var missile_shape;
	var pos;
	var amount;
	var rand;
	if (event == DOUBLECLICK)
	{
		UI_halt_scheduled(item);
		UI_item_say(item, "@In Jux Ylem@");
		if (notInMagicStorm())
		{
			script item
			{	nohalt;						actor frame LEAN;
				actor frame CAST_2;			actor frame LEAN;
				actor frame CAST_2;			sfx 67;}
			obj = getPathEgg(0, 4);
			script obj after 10 ticks
			{	nohalt;						call spellCreateAmmo;}
		}
		else
		{
			script item
			{	nohalt;						actor frame LEAN;
				actor frame CAST_2;			actor frame LEAN;
				actor frame CAST_2;			call spellFails;}
		}
	}
	if (event == SCRIPTED)
	{
		bows = UI_count_objects(PARTY, SHAPE_BOW, QUALITY_ANY, FRAME_ANY);
		magicbows = UI_count_objects(PARTY, SHAPE_MAGIC_BOW, QUALITY_ANY, FRAME_ANY);
		crossbows = UI_count_objects(PARTY, SHAPE_CROSSBOW, QUALITY_ANY, FRAME_ANY);
		bows = (bows + magicbows);
		missile_shape = SHAPE_ARROW;
		if (crossbows > bows)
			missile_shape = SHAPE_BOLT;

		obj = UI_create_new_object(missile_shape);
		if (obj)
		{
			pos = UI_get_object_position(AVATAR);
			amount = (getNPCLevel(AVATAR) * 8);
			rand = UI_die_roll(amount, (amount + 20));
			
			//Original:
			//const int MAX_AMMO				= 99;
			//Why not allow a level 10 avatar create 100 arrows?
			const int MAX_AMMO				= 100;
			if (rand > MAX_AMMO)
				rand = MAX_AMMO;

			UI_set_item_flag(obj, TEMPORARY);
			UI_set_item_quantity(obj, rand);
			UI_update_last_created(pos);
		}
	}
}


/*
	Seventh Circle Spells
*/

spellVibrate 0x676 ()
{
	var target;
	var target_shape;
	var dir;
	
	if (event == DOUBLECLICK)
	{
		target = UI_click_on_item();
		target_shape = UI_get_item_shape(target);
		dir = directionFromAvatar(item);
		UI_halt_scheduled(item);
		UI_item_say(item, "@Uus Des Por Grav@");
		if (notInMagicStorm() && UI_is_npc(target) && (target[X] != 0))
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_2H_1;		actor frame SWING_2H_2;
				actor frame SWING_1;		actor frame SWING_2;
				actor frame SWING_2H_3;		sfx 67;}
			script target after 10 ticks
			{	nohalt;						call spellVibrate;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_2H_1;		actor frame SWING_2H_2;
				actor frame SWING_1;		actor frame SWING_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
	if (event == SCRIPTED)
	{
		var curritem;
		var pos;
		var updworked;
		var npcitems;
		npcitems = UI_get_cont_items(item, SHAPE_ANY, QUALITY_ANY, FRAME_ANY);
		var array_size = UI_get_array_size(npcitems);
		var index;
		
		if (((UI_get_npc_number(item) == AVATAR) && (array_size > 33)) || (array_size > 0))
		{
			while (index < array_size)
			{
				index = index + 1;
				target_shape = UI_get_item_shape(npcitems[index]);
				if ((target_shape != SHAPE_PATH_EGG) && (target_shape != SHAPE_USECODE_CONTAINER))
					break;
			}
			
			if (index <= array_size)
			{
				curritem = UI_set_last_created(npcitems[index]);
				if (curritem)
				{
					
					pos = UI_get_object_position(item);
					updworked = UI_update_last_created(pos);
	
					if (!updworked)
						updworked = UI_update_last_created([(pos[X] - 2), pos[Y], pos[Z]]);
	
					if (!updworked)
						updworked = UI_update_last_created([pos[X], (pos[Y] - 2), pos[Z]]);
	
					if (!updworked)
						updworked = UI_update_last_created([(pos[X] + 2), pos[Y], pos[Z]]);
	
					if (!updworked)
						updworked = UI_update_last_created([pos[X], (pos[Y] + 2), pos[Z]]);
	
					if (!updworked)
						UI_remove_item(curritem);
					else
					{
						script item
						{	nohalt;						call spellVibrate;}
					}
				}
			}
		}
	}
}


/*
	Eighth Circle Spells
*/

spellCreateIce 0x678 ()
{
	var failed;
	var target;
	var field_x;
	var field_y;
	var field_z;
	var pos;
	var field;

	failed = false;
	if (event == DOUBLECLICK)
	{
		UI_halt_scheduled(item);
		target = UI_click_on_item();
		UI_item_say(item, "@In Frio@");
		field_x = (target[X + 1] + 1);
		field_y = (target[Y + 1] + 1);
		field_z = target[Z + 1];
		pos = [field_x, field_y, field_z];
		if (notInMagicStorm())
		{
			field = UI_create_new_object(SHAPE_MONOLITH);
			if (field)
			{
				UI_set_item_frame(field, 9);
				if (UI_update_last_created(pos))
				{
					script item
					{	nohalt;						actor frame SWING_1;
						actor frame SWING_2;		actor frame SWING_3;}
						
					UI_set_item_flag(field, TEMPORARY);
					script field after 200 ticks
						remove;
				}
				else
					failed = true;
			}
			else
				failed = true;
		}
		else
			failed = true;

		if (failed)
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellFetch 0x67B ()
{
	var target;
	var isgrabbable;
	var dir;
	if (event == DOUBLECLICK)
	{
		target = UI_click_on_item();
		isgrabbable = UI_get_item_usability(target);
		dir = directionFromAvatar(item);
		UI_halt_scheduled(item);
		UI_item_say(item, "@Por Ylem@");
		//Original:
		//if (notInMagicStorm() && ((isgrabbable < 2) && (isgrabbable > 0)))
		//It seems that at some point, they were considering to have more
		//usability flags in SI... but thanks to EA, we'll never know.
		if (notInMagicStorm() && (isgrabbable == 1))
		{
			script item
			{	nohalt;						face dir;
				sfx 67;}
			if (UI_set_last_created(target))
				UI_update_last_created(UI_get_object_position(item));
		}
		else
		{
			script item
			{	nohalt;						face dir;
				call spellFails;}
		}
	}
}

spellSerpentBond 0x67D ()
{
	var pos;
	var snakepos;
	var insnakemaze;
	if (event == DOUBLECLICK)
	{
		UI_halt_scheduled(item);
		UI_item_say(item, "@Kal Frio Xen Ex@");
		if (notInMagicStorm() && (!UI_get_item_flag(AVATAR, PETRA)))
		{
			script item
			{	nohalt;						actor frame SWING_2H_1;
				actor frame SWING_2H_2;		actor frame SWING_2H_3;
				actor frame SWING_2H_2;		actor frame STAND;
				actor frame KNEEL;			actor frame LIE;
				actor frame LIE;			call serpentbondRemoveNPCsFromParty;
				sfx 67;}
			
			script getPathEgg(5, 1) after 300 ticks
			{	nohalt;						finish;
				call spellSerpentBond;}
			UI_set_polymorph(UI_get_npc_object(AVATAR), SHAPE_SNAKE);
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2H_1;
				actor frame SWING_2H_2;		actor frame SWING_2H_3;
				actor frame SWING_2H_2;		actor frame STAND;
				actor frame KNEEL;			actor frame LIE;
				actor frame LIE;			call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		pos = UI_get_object_position(item);
		snakepos = [pos[X] + 2, pos[Y] + 2, pos[Z]];
		
		insnakemaze = false;
		//Check to see if the Avatar is inside the snake maze:
		if ((pos[X] > 0x550) && (pos[X] < 0x573) && (pos[Y] > 0x1EA) && (pos[Y] < 0x212))
			insnakemaze = true;

		if ((insnakemaze == false) && UI_is_not_blocked(snakepos, SHAPE_MALE_AVATAR, 1))
		{
			UI_set_polymorph(UI_get_npc_object(AVATAR), SHAPE_MALE_AVATAR);
			item->serpentbondAddNPCsBackToParty();
		}
		else
		{
			script getPathEgg(5, 1) after 10 ticks
			{	nohalt;						finish;
				call spellSerpentBond;}
		}
	}
}

spellFireSnake 0x67E ()
{
	var target;
	var dir;
	var field;
	var field_x;
	var field_y;
	var field_z;
	var pos;
	var duration = 4;
	var oldfield;
	
	if (event == DOUBLECLICK)
	{
		target = UI_click_on_item();
		dir = UI_direction_from(item, target);
		UI_halt_scheduled(item);
		UI_item_say(item, "@Kal Vas Frio Grav@");
		if (notInMagicStorm())
		{
			if (target[X] == 0)
			{
				pos = [target[X + 1], target[Y + 1], target[Z + 1]];
				target = UI_create_new_object(SHAPE_PATH_EGG);
				UI_set_item_frame(target, 31);
				UI_set_item_flag(target, TEMPORARY);
				UI_update_last_created(pos);
				
				script target after (UI_get_distance(target, item) + 3) ticks
				{	nohalt;						remove;}
			}
			
			script item
			{	face dir;					actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;}
			field = UI_create_new_object(SHAPE_FIRE_FIELD);
			if (field)
			{
				pos = UI_get_object_position(item);
				if (dir in [NORTHWEST, NORTH, NORTHEAST])
					field_y = pos[Y] - 2;
				else if (dir in [SOUTHWEST, SOUTH, SOUTHEAST])
					field_y = pos[Y] + 2;
				else
					field_y = pos[Y];
				
				if (dir in [NORTHWEST, WEST, SOUTHWEST])
					field_x = pos[X] - 2;
				else if (dir in [NORTHEAST, EAST, SOUTHEAST])
					field_x = pos[X] + 2;
				else
					field_x = pos[X];
				
				field_z = pos[Z];
				pos = [field_x, field_y, field_z];
				UI_update_last_created(pos);
				UI_set_item_quality(field, duration);
				UI_set_item_flag(field, TEMPORARY);
				script field after duration ticks
				{	nohalt;						remove;}
					
				script target
				{	nohalt;						call spellFireSnake;						
					sfx 65;}
			}
		}
		else
		{
			script item
			{	face dir;					actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		oldfield = UI_find_nearest(item, SHAPE_FIRE_FIELD, 36);

		dir = UI_direction_from(oldfield, item);
		field = UI_create_new_object(SHAPE_FIRE_FIELD);
		if (field)
		{
			pos = UI_get_object_position(oldfield);
			if (dir in [NORTHWEST, NORTH, NORTHEAST])
				field_y = pos[Y] - 1;
			else if (dir in [SOUTHWEST, SOUTH, SOUTHEAST])
				field_y = pos[Y] + 1;
			else
				field_y = pos[Y];
			
			if (dir in [NORTHWEST, WEST, SOUTHWEST])
				field_x = pos[X] - 1;
			else if (dir in [NORTHEAST, EAST, SOUTHEAST])
				field_x = pos[X] + 1;
			else
				field_x = pos[X];
			
			field_z = pos[Z];
			pos = [field_x, field_y, field_z];
			
			
			if (!UI_is_not_blocked(pos, SHAPE_FIRE_FIELD, 0) && UI_get_distance(oldfield, item) > 1)
			{
				//Try going up:
				pos[Z] = field_z + 1;
				if (!UI_is_not_blocked(pos, SHAPE_FIRE_FIELD, 0))
				{
					//Try going down:
					pos[Z] = field_z - 1;
					if (!UI_is_not_blocked(pos, SHAPE_FIRE_FIELD, 0))
					{
						//Force the issue:
						pos[Z] = field_z;
					}
				}
			}
			
			UI_update_last_created(pos);
			UI_set_item_quality(field, duration);
			UI_set_item_flag(field, TEMPORARY);
			
			script field after duration ticks
			{	nohalt;						remove;}
			
			if (UI_get_distance(oldfield, item) == 0)
			{
				oldfield->firesnakeExplode();//UI_explode(oldfield, oldfield, 0x20);
				return;
			}
			else if (UI_get_distance(field, item) == 0)
			{
				field->firesnakeExplode();//UI_explode(field, field, 0x20);
				return;
			}
			
			script item
			{	nohalt;						call spellFireSnake;						
				sfx 65;}
		}
	}
}

/*
	Ninth Circle Spells
*/

spellStopStorm 0x684 ()
{
	if (event == DOUBLECLICK)
	{
		UI_halt_scheduled(item);
		UI_item_say(item, "@An Hur@");
		if (notInMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		sfx 57;}
			
			if (UI_get_weather() != 3)
				UI_set_weather(0);
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
}

spellImbalance 0x687 ()
{
	var pos;
	var delay;
	if (event == DOUBLECLICK)
	{
		UI_halt_scheduled(item);
		UI_item_say(item, "@Kal Vas An Grav@");
		if (notInMagicStorm())
		{
			UI_obj_sprite_effect(item, ANIMATION_TELEPORT, -2, -2, -3, -3, 0, -1);
			UI_obj_sprite_effect(item, ANIMATION_TELEPORT, -2, -2,  3, -3, 0, -1);
			UI_obj_sprite_effect(item, ANIMATION_TELEPORT, -2, -2,  3,  3, 0, -1);
			UI_obj_sprite_effect(item, ANIMATION_TELEPORT, -2, -2, -3,  3, 0, -1);
			UI_obj_sprite_effect(item, 26,				   -2, -2,  0,  0, 0, -1);
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2H_2;		actor frame KNEEL;
				actor frame KNEEL;			actor frame SWING_2H_2;
				sfx 67;}
			delay = (15 + getNPCLevel(AVATAR));
			while (delay != 0)
			{
				delay = (delay - 1);
				script AVATAR after (delay + 10) ticks
				{	nohalt;						call createImbalanceFields;}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2H_2;		actor frame KNEEL;
				actor frame KNEEL;			actor frame SWING_2H_2;
				call spellFails;}
		}
	}
}
