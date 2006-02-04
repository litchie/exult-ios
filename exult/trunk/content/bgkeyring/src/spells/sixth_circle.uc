/*
 *	This source file contains some reimplementations of all sixth
 *	circle spells.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-02-03
 */

/*
	Sixth circle Spells
	
	extern spellCauseFear ();
	extern spellClone ();
	extern spellFireRing ();
	extern spellFlameStrike ();
	extern spellMagicStorm ();
	extern spellPoisonField ();
	extern spellSleepField ();
	extern spellTremor ();
*/

enum sixth_circle_spells
{
	SPELL_CAUSE_FEAR				= 0,
	SPELL_CLONE						= 1,
	SPELL_FIRE_RING					= 2,
	SPELL_FLAME_STRIKE				= 3,
	SPELL_MAGIC_STORM				= 4,
	SPELL_POISON_FIELD				= 5,
	SPELL_SLEEP_FIELD				= 6,
	SPELL_TREMOR					= 7
};

spellCauseFear ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Quas Wis@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				actor frame KNEEL;			actor frame STAND;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;		call spellCauseFear;}
		}
		else
		{
			script item
			{	nohalt;						actor frame KNEEL;
				actor frame STAND;			actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var targets = getEnemyTargetList(item, 25);
		var index;
		var max;
		var npc;
		for (npc in targets with index to max)
		{
			if (npc->get_npc_prop(INTELLIGENCE) > 5)
			{
				npc->set_schedule_type(IN_COMBAT);
				npc->set_attack_mode(FLEE);
				npc->set_opponent(item);
			}
		}
	}
}

spellClone (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@In Quas Xen@");
		if (inMagicStorm() && (target->is_npc() && (!(UI_get_item_flag(0, 27) == -1))))
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame CAST_2;			actor frame SWING_2H_3;}
			script target after 4 ticks
			{	nohalt;						call spellClone;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}

	else if (event == SCRIPTED)
	{
		var summoned = clone();
		summoned->set_alignment(get_alignment());
	}
}

spellFireRing (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@Kal Flam Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						face dir;
				sfx 65;						actor frame SWING_2;
				actor frame SWING_3;		actor frame SWING_1;}
				
			var offset_x = [-1, 0, 1, 2, 2, 2, 1, 0, -1, -2, -2, -2];
			var offset_y = [-2, -2, -2, -1, 0, 1, 2, 2, 2, 1, 0, -1];
			var counter = 0;
			while (counter < 12)
			{
				counter = (counter + 1);
				var ring_x = (target[X + 1] + offset_x[counter]);
				var ring_y = (target[Y + 1] + offset_y[counter]);
				var ring_z = target[Z + 1];
				var pos = [ring_x, ring_y, ring_z];
				var pos2 = [ring_x, ring_y, (ring_z + 1)];
				if (!UI_is_not_blocked(pos, SHAPE_DELAYED_EXPLOSION, 0))
					pos = pos2;
				if (UI_is_not_blocked(pos, SHAPE_DELAYED_EXPLOSION, 0))
				{
					var ring = UI_create_new_object(SHAPE_DELAYED_EXPLOSION);
					if (ring)
					{
						ring->set_item_flag(TEMPORARY);
						ring->set_item_flag(INVISIBLE);
						UI_update_last_created(pos);
						ring->set_npc_prop(HEALTH, 1);
						script ring after counter ticks
						{	nohalt;						call spellFireRing;}
					}
				}
			}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_2;		actor frame SWING_3;
				actor frame SWING_1;		call spellFails;}
		}
	}
	else if (event == SCRIPTED)
	{
		var pos = get_object_position();
		remove_item();
		var ring = UI_create_new_object(SHAPE_FIRE_FIELD);
		if (ring)
		{
			ring->set_item_flag(TEMPORARY);
			UI_update_last_created(pos);
			var delay = 31;
			delay = (delay + UI_die_roll(1, 5));
			script ring after delay ticks
			{	nohalt;						remove;}
		}
	}
}

spellFlameStrike ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas In Flam Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				actor frame LEAN;			actor frame KNEEL;
				actor frame LEAN;			actor frame CAST_2;
				actor frame SWING_2H_3;}
				
			var targets = getEnemyTargetList(item, 25);
			var index;
			var max;
			var npc;
			for (npc in targets with index to max)
			{
				var pos = npc->get_object_position();
				var strike_x = pos[X];
				var strike_y = pos[Y];
				var strike_z = pos[Z];
				pos = [strike_x, strike_y, 0];
				var strike = UI_create_new_object(SHAPE_FIRE_FIELD);
				if (strike)
				{
					var last_obj = UI_update_last_created(pos);
					var delay = 31 + UI_die_roll(1, 15);
					last_obj = strike->set_item_quality(delay);
					strike->set_item_flag(TEMPORARY);
					script strike after delay ticks
					{	nohalt;							remove;}
				}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame LEAN;
				actor frame KNEEL;			actor frame LEAN;
				actor frame CAST_2;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}
}

spellMagicStorm ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Oort Hur@");
		if (inMagicStorm())
		{
			var delay = 70;
			gflags[MAGIC_STORM_SPELL] = true;
			UI_set_weather(2);
			script item
			{	nohalt;						sfx 65;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame CAST_1;			actor frame SWING_2H_3;
				actor frame STAND;}
				
			script item after 8 ticks
			{	nohalt;						call spellMagicStorm;}
			
			script item after delay ticks
			{	nohalt;						call stopMagicStorm;}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame CAST_1;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		if (gflags[MAGIC_STORM_SPELL] == true)
		{
			var targets = getEnemyTargetList(item, 45);
			var npc = targets[UI_get_random(UI_get_array_size(targets))];
			if (npc)
			{
				script npc
				{	nohalt;						call callLightning;}
			}
			var rand = UI_die_roll(3, 8);
			script item after rand ticks
			{	nohalt;						call spellMagicStorm;}
		}
	}
}

spellPoisonField (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@In Nox Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						face dir;
				sfx 110;					actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;}
			var field = UI_create_new_object(SHAPE_POISON_FIELD);
			if (field)
			{
				var field_x = (target[X + 1] + 1);
				var field_y = (target[Y + 1] + 1);
				var field_z = target[Z + 1];
				var pos = [field_x, field_y, field_z];
				field->set_item_flag(TEMPORARY);
				UI_update_last_created(pos);
			}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_2;
				actor frame SWING_3;		call spellFails;}
		}
	}
}

spellSleepField (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@In Zu Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				face dir;					actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;}
				
			var field_x = (target[X + 1] + 1);
			var field_y = (target[Y + 1] + 1);
			var field_z = target[Z + 1];
			var pos = [field_x, field_y, field_z];
			var field = UI_create_new_object(SHAPE_SLEEP_FIELD);
			if (field)
			{
				field->set_item_flag(TEMPORARY);
				UI_update_last_created(pos);
			}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_2;
				actor frame SWING_3;		call spellFails;}
		}
	}
}

spellTremor ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Por Ylem@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_2H_1;
				actor frame STAND;			actor frame KNEEL;
				sfx 67;						call spellTremor;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2H_1;
				actor frame STAND;			actor frame KNEEL;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var targets = getEnemyTargetList(item, 40);
		var duration = 12;
		var index;
		var max;
		var npc;
		for (npc in targets with index to max)
		{
			var counter = 0;
			var usecodearray = [];
			var directions = [NORTH, EAST, SOUTH, WEST];
			while (counter < duration)
			{
				var rand = UI_die_roll(0, 8);
				var randusecodearray;
				var randopcode;

				if (rand >= 6)
					//Get one of the four cardinal directions:
					randopcode = (0x30 + directions[UI_get_random(4)]);
				
				if (rand == 0)
					randusecodearray = [SCRIPT_NPC_FRAME + KNEEL, SCRIPT_NPC_FRAME + LEAN, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 1)
					randusecodearray = [SCRIPT_NPC_FRAME + KNEEL, SCRIPT_NPC_FRAME + STAND, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 2)
					randusecodearray = [SCRIPT_NPC_FRAME + LEAN, SCRIPT_NPC_FRAME + LIE, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 3)
					randusecodearray = [SCRIPT_NPC_FRAME + STAND, SCRIPT_NPC_FRAME + STAND, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 4)
					randusecodearray = [SCRIPT_NPC_FRAME + KNEEL, SCRIPT_NPC_FRAME + USE, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 5)
					randusecodearray = [SCRIPT_NPC_FRAME + USE, SCRIPT_NPC_FRAME + KNEEL, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 6)
					randusecodearray = [SCRIPT_FACE_DIR, randopcode, SCRIPT_NPC_FRAME + LEAN, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 7)
					randusecodearray = [SCRIPT_FACE_DIR, randopcode, SCRIPT_NPC_FRAME + KNEEL, SCRIPT_NPC_FRAME + STAND];

				else if (rand == 8)
					randusecodearray = [SCRIPT_FACE_DIR, randopcode, SCRIPT_NPC_FRAME + USE, SCRIPT_NPC_FRAME + STAND];

				usecodearray = [usecodearray, randusecodearray];
				counter = (counter + 1);
			}
			npc->halt_scheduled();
			npc->execute_usecode_array(usecodearray);
		}
		UI_earthquake((duration * 3));
	}
}
