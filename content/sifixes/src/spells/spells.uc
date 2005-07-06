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
		halt_scheduled();
		item_say("@Ex Jux Hur@");
		if (notInMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_2H_3;
				actor frame SWING_3;		actor frame SWING_2H_2;
				sfx 20;						wait 5;
				call spellSurprise;}
			nearbynpcs = find_nearby(SHAPE_ANY, 30, 8);
			party = UI_get_party_list();
			for (npc in nearbynpcs with index to max)
			{
				if (!(npc in party))
				{
					delay = (get_distance(npc) + 15);
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
		if (item == AVATAR->get_npc_object())
		{
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0, -3, -3, 4, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0,  0, -4, 4, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0,  3, -3, 3, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0, -4,  0, 2, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0,  4,  0, 1, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0, -3,  3, 3, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0,  0,  4, 1, 25);
			obj_sprite_effect(ANIMATION_CLOUDS, 0, 0,  3,  3, 1, 25);
		}
		else
		{
			rand = UI_get_random(3);
			if (rand == 1)
			{
				set_schedule_type(IN_COMBAT);
				set_attack_mode(FLEE);
				set_opponent(AVATAR);
			}
			else if (rand == 2)
			{
				halt_scheduled();
				set_item_flag(ASLEEP);
			}
			else if (rand == 3)
			{
				halt_scheduled();
				set_item_flag(POISONED);
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
		halt_scheduled();
		item_say("@In Jux Ylem@");
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
		bows = PARTY->count_objects(SHAPE_BOW, QUALITY_ANY, FRAME_ANY);
		magicbows = PARTY->count_objects(SHAPE_MAGIC_BOW, QUALITY_ANY, FRAME_ANY);
		crossbows = PARTY->count_objects(SHAPE_CROSSBOW, QUALITY_ANY, FRAME_ANY);
		bows = (bows + magicbows);
		missile_shape = SHAPE_ARROW;
		if (crossbows > bows)
			missile_shape = SHAPE_BOLT;

		obj = missile_shape->create_new_object();
		if (obj)
		{
			pos = AVATAR->get_object_position();
			amount = (getNPCLevel(AVATAR) * 8);
			rand = UI_die_roll(amount, (amount + 20));
			
			//Original:
			//const int MAX_AMMO				= 99;
			//Why not allow a level 10 avatar create 100 arrows?
			const int MAX_AMMO				= 100;
			if (rand > MAX_AMMO)
				rand = MAX_AMMO;

			obj->set_item_flag(TEMPORARY);
			obj->set_item_quantity(rand);
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
		target_shape = target->get_item_shape();
		dir = directionFromAvatar(item);
		halt_scheduled();
		item_say("@Uus Des Por Grav@");
		if (notInMagicStorm() && target->is_npc() && (target[X] != 0))
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
		npcitems = get_cont_items(SHAPE_ANY, QUALITY_ANY, FRAME_ANY);
		var array_size = UI_get_array_size(npcitems);
		var index;
		var mincount;
		mincount = count_objects(SHAPE_USECODE_CONTAINER, QUALITY_ANY, FRAME_ANY) +
				   count_objects(SHAPE_PATH_EGG, QUALITY_ANY, FRAME_ANY);
		
		if (array_size > mincount)
		{
			while (index < array_size)
			{
				index = index + 1;
				target_shape = npcitems[index]->get_item_shape();
				if ((target_shape != SHAPE_PATH_EGG) && (target_shape != SHAPE_USECODE_CONTAINER))
					break;
			}
			
			//Just for safety:
			target_shape = npcitems[index]->get_item_shape();
			if ((target_shape == SHAPE_PATH_EGG) || (target_shape == SHAPE_USECODE_CONTAINER))
				return;
			
			if (index <= array_size)
			{
				curritem = npcitems[index]->set_last_created();
				if (curritem)
				{
					
					pos = get_object_position();
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
						curritem->remove_item();
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
		halt_scheduled();
		target = UI_click_on_item();
		item_say("@In Frio@");
		field_x = (target[X + 1] + 1);
		field_y = (target[Y + 1] + 1);
		field_z = target[Z + 1];
		pos = [field_x, field_y, field_z];
		if (notInMagicStorm())
		{
			field = UI_create_new_object(SHAPE_MONOLITH);
			if (field)
			{
				field->set_item_frame(9);
				if (UI_update_last_created(pos))
				{
					script item
					{	nohalt;						actor frame SWING_1;
						actor frame SWING_2;		actor frame SWING_3;}
						
					field->set_item_flag(TEMPORARY);
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
		isgrabbable = target->get_item_usability();
		dir = directionFromAvatar(item);
		halt_scheduled();
		item_say("@Por Ylem@");
		//Original:
		//if (notInMagicStorm() && ((isgrabbable < 2) && (isgrabbable > 0)))
		//It seems that at some point, they were considering to have more
		//usability flags in SI... but thanks to EA, we'll never know.
		if (notInMagicStorm() && (isgrabbable == 1))
		{
			script item
			{	nohalt;						face dir;
				sfx 67;}
			if (target->set_last_created())
				UI_update_last_created(get_object_position());
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
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Kal Frio Xen Ex@");
		if (notInMagicStorm() && (!AVATAR->get_item_flag(PETRA)))
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
			AVATAR->set_polymorph(SHAPE_SNAKE);
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
		pos = get_object_position();
		snakepos = [pos[X] + 2, pos[Y] + 2, pos[Z]];
		
		var insnakemaze = false;
		//Check to see if the Avatar is inside the snake maze:
		if ((pos[X] > 0x550) && (pos[X] < 0x573) && (pos[Y] > 0x1EA) && (pos[Y] < 0x212))
			insnakemaze = true;

		if ((getAvatarLocationID() != DREAM_WORLD) && (insnakemaze == false) && UI_is_not_blocked(snakepos, SHAPE_MALE_AVATAR, 1))
		{
			if (get_item_flag(POLYMORPH))
				AVATAR->set_polymorph(SHAPE_MALE_AVATAR);
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
		dir = direction_from(target);
		halt_scheduled();
		item_say("@Kal Vas Frio Grav@");
		if (notInMagicStorm())
		{
			if (target[X] == 0)
			{
				pos = [target[X + 1], target[Y + 1], target[Z + 1]];
				target = UI_create_new_object(SHAPE_PATH_EGG);
				target->set_item_frame(31);
				target->set_item_flag(TEMPORARY);
				UI_update_last_created(pos);
				
				script target after (get_distance(target) + 3) ticks
				{	nohalt;						remove;}
			}
			
			script item
			{	face dir;					actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;}
			field = UI_create_new_object(SHAPE_FIRE_FIELD);
			if (field)
			{
				pos = get_object_position();
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
				field->set_item_quality(duration);
				field->set_item_flag(TEMPORARY);
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
		oldfield = find_nearest(SHAPE_FIRE_FIELD, 36);

		dir = oldfield->direction_from(item);
		field = UI_create_new_object(SHAPE_FIRE_FIELD);
		if (field)
		{
			pos = oldfield->get_object_position();
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
			
			
			if (!UI_is_not_blocked(pos, SHAPE_FIRE_FIELD, 0) && get_distance(oldfield) > 1)
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
			field->set_item_quality(duration);
			field->set_item_flag(TEMPORARY);
			
			script field after duration ticks
			{	nohalt;						remove;}
			
			if (get_distance(oldfield) == 0)
			{
				//oldfield->firesnakeExplode();
				UI_explode(oldfield, oldfield, 0x20);
				return;
			}
			else if (UI_get_distance(field, item) == 0)
			{
				//field->firesnakeExplode();
				UI_explode(field, field, 0x20);
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
		halt_scheduled();
		item_say("@An Hur@");
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
		halt_scheduled();
		item_say("@Kal Vas An Grav@");
		if (notInMagicStorm())
		{
			obj_sprite_effect(ANIMATION_TELEPORT, -2, -2, -3, -3, 0, -1);
			obj_sprite_effect(ANIMATION_TELEPORT, -2, -2,  3, -3, 0, -1);
			obj_sprite_effect(ANIMATION_TELEPORT, -2, -2,  3,  3, 0, -1);
			obj_sprite_effect(ANIMATION_TELEPORT, -2, -2, -3,  3, 0, -1);
			obj_sprite_effect(26,				   -2, -2,  0,  0, 0, -1);
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
