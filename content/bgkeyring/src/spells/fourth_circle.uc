/*
 *
 *  Copyright (C) 2006  The Exult Team
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
 *
 *	This source file contains some reimplementations of almost all
 *	fourth circle spells. The exceptions are 'Mark' and 'Recall' (but
 *	see the spellbook override for the 'Mark' reimplementation).
 *
 *	There is also 2 new spells in the list: 'Recharge Magic', 'Blink'
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

/*
	Fourth circle Spells
	
	extern spellConjure ();
	extern spellLightning ();
	extern spellMassCurse ();
	extern spellReveal ();
	extern spellSeance ();
	extern spellUnlockMagic ();
	extern spellRechargeMagic ();
	extern spellBlink ();
*/

enum fourth_circle_spells
{
	SPELL_CONJURE					= 0,
	SPELL_LIGHTNING					= 1,
	SPELL_MASS_CURSE				= 2,
	SPELL_REVEAL					= 3,
	SPELL_SEANCE					= 4,
	SPELL_UNLOCK_MAGIC				= 5,
	SPELL_RECHARGE_MAGIC			= 6,		//NPC-only spell
	SPELL_BLINK						= 7			//NPC-only spell
};

spellConjure ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Kal Xen@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				actor frame SWING_1;		actor frame SWING_2;
				actor frame SWING_3;		call spellConjure;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var conjurables = [SHAPE_BIRD, SHAPE_RABBIT, SHAPE_RAT, SHAPE_FOX, SHAPE_SNAKE, SHAPE_DEER, SHAPE_WOLF];
		var arraysize = UI_get_array_size(conjurables);
		var npclevel = getNPCLevel(item);
		if (npclevel > arraysize)
			npclevel = arraysize;

		if (npclevel < 2)
			npclevel = 2;

		var minroll = (npclevel / 2);
		if (minroll < 1)
			minroll = 1;

		var rand = UI_die_roll(minroll, npclevel);
		
		while (rand > 0)
		{
			rand = (rand - 1);
			var rand2 = UI_die_roll(minroll, npclevel);
			var summoned = conjurables[rand2]->summon(false);
			summoned->set_alignment(get_alignment());
		}
	}
}

spellLightning (var target)
{
	if ((event == DOUBLECLICK) || (event == WEAPON))
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@Ort Grav@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_LIGHTNING);
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame CAST_1;
				sfx 65;						actor frame CAST_2;
				actor frame SWING_2H_3;		actor frame SWING_2H_3;
				attack;						actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}
}

spellMassCurse ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Des Sanct@");
		if (inMagicStorm())
		{
			var pos = get_object_position();
			UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
			script item
			{	nohalt;						actor frame CAST_1;
				sfx 65;						actor frame CAST_2;
				actor frame CAST_1;			actor frame SWING_2H_3;}
			var index;
			var max;
			var npc;
			var targets = getEnemyTargetList(item, 31);
			for (npc in targets with index to max)
				if (!(UI_die_roll(1, 3) == 1))
				{
					var delay = ((get_distance(npc) / 3) + 5);
					script npc after delay ticks
					{	nohalt;						call spellMassCurse;}
				}
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
		set_item_flag(CURSED);
}

spellReveal ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		var pos = get_object_position();
		item_say("@Wis Quas@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 67;
				actor frame SWING_1;		actor frame SWING_2;
				actor frame SWING_3;}
				
			var findpos = get_object_position();
			var offset_x = [-15, -15, -15, -5, -5, -5, 5, 5, 5, 15, 15, 15];
			var offset_y = [-7, 2, 11, -7, 2, 11, -7, 2, 11, -7, 2, 11];
			var dist = 7;
			var counter = 0;
			var revealables = [];
			var obj;
			var index;
			var max;
			while (counter != 12)
			{
				counter = (counter + 1);
				var find_x = (pos[X] + offset_x[counter]);
				var find_y = (pos[Y] + offset_y[counter]);
				findpos = [find_x, find_y, 0];
				var invisibles = findpos->find_nearby(SHAPE_ANY, dist, MASK_INVISIBLE);
				for (obj in invisibles with index to max)
				{
					if (obj->get_item_flag(INVISIBLE) && (!(obj in revealables)))
						revealables = (revealables & obj);
				}
			}
			if (revealables)
			{
				for (obj in revealables with index to max)
				{
					script obj after 5 ticks
					{	nohalt;						call spellReveal;}
					obj->obj_sprite_effect(13, -1, -1, 0, 0, 0, -1);
				}
			}
			else
				obj_sprite_effect(13, -1, -1, 0, 0, 0, -1);
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
		clear_item_flag(INVISIBLE);
}

spellSeance ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Kal Wis Corp@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 67;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;}
				
			var pos = get_object_position();
			var sprite_x = (pos[X] - 2);
			var sprite_y = (pos[Y] - 2);
			UI_sprite_effect(13, sprite_x, sprite_y, 0, 0, 0, -1);
			UI_sprite_effect(7, sprite_x, sprite_y, 0, 0, 0, -1);
			//I have NO idea why they had one flag per ghost,
			//instead of having one for them all...
			gflags[SEANCE_CAINE] = true;
			gflags[SEANCE_FERRYMAN] = true;
			gflags[SEANCE_MARKHAM] = true;
			gflags[SEANCE_HORANCE] = true;
			gflags[SEANCE_TRENT] = true;
			gflags[SEANCE_MORDRA] = true;
			gflags[SEANCE_ROWENA] = true;
			gflags[SEANCE_PAULETTE] = true;
			gflags[SEANCE_QUENTON] = true;
			gflags[SEANCE_FORSYTHE] = true;
			
			var hour = UI_game_hour();
			var minutes = UI_game_minute();
			var delay;
			
			if (hour < 6)
			{
				delay = ((6 - hour) * 60);
				delay = (delay + (60 - minutes));
				delay = (delay * 25);
			}
			else
			{
				delay = ((23 - hour) * 60);
				delay = (delay + (60 - minutes));
				delay = (delay * 25);
			}
			
			script item after delay ticks
			{	nohalt;						finish;
				call spellSeance;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		//I have NO idea why they had one flag per ghost,
		//instead of having one for them all...
		gflags[SEANCE_CAINE] = false;
		gflags[SEANCE_FERRYMAN] = false;
		gflags[SEANCE_MARKHAM] = false;
		gflags[SEANCE_HORANCE] = false;
		gflags[SEANCE_TRENT] = false;
		gflags[SEANCE_MORDRA] = false;
		gflags[SEANCE_ROWENA] = false;
		gflags[SEANCE_PAULETTE] = false;
		gflags[SEANCE_QUENTON] = false;
		gflags[SEANCE_FORSYTHE] = false;
	}
}

spellUnlockMagic (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var target_shape = target->get_item_shape();
		var dir = direction_from(target);
		halt_scheduled();
		var unlockables = [SHAPE_DOOR_HORIZONTAL, SHAPE_DOOR_VERTICAL,
						   SHAPE_DOOR2_HORIZONTAL, SHAPE_DOOR2_VERTICAL,
						   SHAPE_FALSE_WALL_HORIZONTAL, SHAPE_FALSE_WALL_VERTICAL];
		item_say("@Ex Por@");
		if (inMagicStorm())
		{
			if (target_shape in unlockables)
			{
				var target_frame = target->get_item_frame();
				if (((target_frame + 1) % 4) == 0)
				{
					script item
					{	nohalt;						face dir;
						actor frame SWING_2;		actor frame SWING_1;
						actor frame SWING_3;		sfx 66;}
						
					script target after 6 ticks
					{	nohalt;						call spellUnlockMagic;}
					
					return;
				}
			}
		}
		script item
		{	nohalt;						face dir;
			actor frame SWING_2;		actor frame SWING_1;
			actor frame SWING_3;		call spellFails;}
	}
	
	else if (event == SCRIPTED)
	{
		var target_frame = get_item_frame() - 3;
		set_item_frame(target_frame);
	}
}

spellRechargeMagic (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var target_shape = target->get_item_shape();
		var dir = direction_from(target);
		item_say("@Uus Ort@");
		var charges = target->get_item_quality();
		if (inMagicStorm() && (target_shape in [SHAPE_LIGHTNING_WAND, SHAPE_FIRE_WAND, SHAPE_FIREDOOM_STAFF]) && (charges < 100))
		{
			if (charges < 50)
				charges = charges + UI_die_roll(12, 25);
			else if (charges < 70)
				charges = charges + UI_die_roll(6, 18);
			else if (charges < 200)
				charges = charges + UI_die_roll(3, 9);

			target->set_item_quality(charges);
			UI_sprite_effect(ANIMATION_PURPLE_BUBBLES, target[X + 1], target[Y + 1], 0, 0, 0, -1);
			script item
			{	nohalt;						face dir;
				sfx 67;						actor frame CAST_2;
				actor frame CAST_1;			actor frame USE;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_2;			actor frame CAST_1;
				actor frame USE;			call spellFails;}
		}
	}
}

const int CREATE_CLOUDS					= 15;
spellBlink (var target)
{

	var nearbyobjs;
	var party;
	var index;
	var max;
	var obj;
	var delay;
	var rand;
	var pathegg;
	
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		var caster_shape = get_item_shape();
		var destpos = getClickPosition(target);
		var dir = direction_from(target);
		UI_item_say(item, "@Rel Por@");
		if (inMagicStorm() && UI_is_not_blocked(destpos, caster_shape, 0))
		{
			var party = UI_get_party_list();
			var dest_reachable = true;
			var move_party = false;
			
			//Special rules and restrictions for party members:
			if (get_npc_object() in party)
			{
				//HACK: Here I use path_run_usecode to see if the target destination
				//is reachable, and then I use it again to cancel the first pathfind.
				//This prevents bypassing of locked doors and such.
				dest_reachable = UI_path_run_usecode(destpos, spellBlink, item, PATH_SUCCESS);
				UI_path_run_usecode(AVATAR->get_object_position(), spellBlink, item, PATH_SUCCESS);
				//See if the whole party should be moved or just the caster:
				if (get_schedule_type() != IN_COMBAT)
					move_party = true;
			}
			if (dest_reachable)
			{
				script item
				{	nohalt;						face dir;
					actor frame SWING_1;		actor frame SWING_2H_2;
					actor frame SWING_1;		actor frame STAND;}
				
				var npc;
				var index;
				var max;
				var targets;
				if (move_party)
					targets = party;
				else
					targets = [item];
				
				for (npc in targets with index to max)
				{
					var pos = npc->get_object_position();
					//UI_sprite_effect(ANIMATION_TELEPORT, pos[X], pos[Y], 0, 0, 0, -1);
					npc->obj_sprite_effect(ANIMATION_TELEPORT, 0, 0, 0, 0, 0, -1);
					UI_play_sound_effect2(SOUND_TELEPORT, npc);
					var field = UI_create_new_object(SHAPE_FIRE_FIELD);
					if (field)
					{
						//var fieldpos = [pos[X] + 1, pos[Y] + 1, pos[Z]];
						var fieldpos = [pos[X], pos[Y], pos[Z]];
						UI_update_last_created(fieldpos);
						var duration = 50;
						field->set_item_quality(duration);
						field->set_item_flag(TEMPORARY);
						script field after duration ticks
						{	nohalt;						remove;}
					}
				}
				
				if (move_party)
					PARTY->move_object(destpos);
				else
					move_object(destpos);
				return;
			}			
		}

		script item
		{	nohalt;						face dir;
			actor frame SWING_1;		actor frame SWING_2H_2;
			actor frame SWING_1;		call spellFails;}
	}
	
	else if (event == SCRIPTED)
		clear_item_flag(BG_DONT_MOVE);
}
