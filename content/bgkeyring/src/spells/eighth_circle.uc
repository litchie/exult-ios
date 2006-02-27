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
 *	eighth circle spells. The exception is 'Armaggedon'.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

/*
	Eighth circle Spells
	
	extern spellDeathVortex ();
	extern spellInvisibilityAll ();
	extern spellMassDeath ();
	extern spellResurrect ();
	extern spellSummon ();
	extern spellSwordStrike ();
	extern spellTimeStop ();
	extern spellMassResurrect ();
*/

enum eighth_circle_spells
{
	SPELL_DEATH_VORTEX				= 0,
	SPELL_INVISIBILITY_ALL			= 1,
	SPELL_MASS_DEATH				= 2,
	SPELL_RESURRECT					= 3,
	SPELL_SUMMON					= 4,
	SPELL_SWORDSTRIKE				= 5,
	SPELL_TIME_STOP					= 6,			//NPC-only spell
	SPELL_MASS_RESURRECT			= 7			//Special NPC-only spell
};

spellDeathVortex (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@Vas Corp Hur@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_DEATH_VORTEX);
			script item
			{	nohalt;						face dir;
				sfx 65;						actor frame CAST_1;
				actor frame SWING_3;		attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellInvisibilityAll ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Sact Lor@");
		if (inMagicStorm())
		{
			var pos = get_object_position();
			UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
			script item
			{	nohalt;						actor frame SWING_1;
				sfx 67;						actor frame STAND;
				actor frame CAST_1;			actor frame STAND;
				actor frame SWING_2H_3;}
			var targets = getFriendlyTargetList(item, 25);
			var index;
			var max;
			var npc;
			for (npc in targets with index to max)
			{
				var delay = ((get_distance(npc) / 3) + 5);
				script npc after delay ticks
				{	nohalt;						call spellInvisibilityAll;}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame STAND;			actor frame CAST_1;
				actor frame STAND;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
		set_item_flag(INVISIBLE);
}

spellMassDeath ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Corp@");
		if (inMagicStorm())
		{
			var pos = get_object_position();
			UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
			script item
			{	nohalt;						sfx 65;
				actor frame KNEEL;			actor frame STAND;
				actor frame CAST_1;			actor frame CAST_2;
				sfx 67;}
			var nearby_npcs = find_nearby(-1, 25, MASK_NPC);
			var safenpcs = [UI_get_party_list2(), UI_get_npc_object(LORD_BRITISH),
							UI_get_npc_object(BATLIN)];
			var killed_anyone = false;
			var index;
			var max;
			var npc;
			for (npc in nearby_npcs with index to max)
			{
				if (!(npc in safenpcs))
				{
					var delay = ((get_distance(npc) / 3) + 5);
					npc->halt_scheduled();
					script npc after delay ticks
					{	nohalt;					call spellMassDeath;}
					killed_anyone = true;
				}
			}
			
			if (killed_anyone == true)
			{
				var party = UI_get_party_list();
				for (npc in party with index to max)
				{
					var hps = npc->get_npc_prop(HEALTH);
					hurtNPC(npc, (hps - 2));
				}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame KNEEL;
				actor frame STAND;			actor frame CAST_1;
				actor frame CAST_2;			call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var cantdie = get_item_flag(CANT_DIE);
		if (!cantdie)
		{
			var hps = get_npc_prop(HEALTH);
			hurtNPC(item, (hps - 2));
			hurtNPC(item, 50);
		}
	}
}

spellResurrect (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var target_shape = target->get_item_shape();
		var pos = target->get_object_position();
		var dir = direction_from(target);
		target->halt_scheduled();
		halt_scheduled();
		var canresurrect = false;
		if (target->get_item_shape() in [SHAPE_BODIES_1, SHAPE_BODIES_2, SHAPE_LARGE_BODIES])
		{
			var target_quality = target->get_item_quality();
			var quant = target->get_item_quantity(target_shape);
			if ((target_quality == 0) && (quant == 0))
				canresurrect = false;

			else
				canresurrect = target->resurrect();
		}
		else
			canresurrect = false;

		item_say("@In Mani Corp@");
		if (inMagicStorm() && canresurrect)
		{
			script item
			{	nohalt;						face dir;
				sfx 64;						actor frame KNEEL;
				actor frame STAND;			actor frame CAST_1;}
			UI_play_music(15, 0);
			UI_sprite_effect(17, pos[X], pos[Y], 0, 0, 0, -1);
			UI_sprite_effect(13, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame KNEEL;			actor frame STAND;
				actor frame CAST_1;			call spellFails;}
		}
	}
}

spellSummon ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("Kal Vas Xen");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame KNEEL;
				actor frame STAND;			actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				sfx 65;						call spellSummon;}
		}
		else
		{
			script item
			{	nohalt;						actor frame KNEEL;
				actor frame STAND;			actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var summonables = [SHAPE_SKELETON, SHAPE_DRAGON, SHAPE_GHOST3, SHAPE_MONSTER_TROLL,
						   SHAPE_MONSTER_MAGE, SHAPE_MONSTER_CYCLOPS, SHAPE_DRAKE,
						   SHAPE_MONSTER_GARGOYLE, SHAPE_GARGOYLE_WINGED, SHAPE_HEADLESS,
						   SHAPE_LICHE, SHAPE_MONGBAT, SHAPE_SCORPION];
		var max_rand2 = [5, 15, 5, 5, 5, 5, 5, 5, 14, 5, 10, 5, 5];
		var creature_count = [5, 1, 5, 1, 2, 2, 2, 1, 1, 3, 1, 5, 2];
		var array_size = UI_get_array_size(summonables);
		
		var rand1 = UI_die_roll(1, array_size);
		var rand2 = UI_die_roll(1, 100);
		while (max_rand2[rand1] < rand2)
		{
			rand1 = UI_die_roll(1, array_size);
			rand2 = UI_die_roll(1, 100);
		}
			
		var counter = creature_count[rand1];
		var randmax = (counter / 2);
		if (randmax < 1)
			randmax = 1;

		var rand3 = UI_die_roll(1, randmax);
		if (UI_die_roll(1, 2) == 1)
			rand3 = (-1 * rand3);

		counter = (counter + rand3);
		while (counter)
		{
			var summoned = summonables[rand1]->summon(true);
			summoned->set_alignment(get_alignment());
			counter = (counter - 1);
		}
	}
}

spellSwordStrike (var target)
{
	if ((event == DOUBLECLICK) || (event == WEAPON))
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@In Jux Por Ylem@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_SWORDSTRIKE);
			script item
			{	nohalt;						face dir;
				sfx 65;						actor frame SWING_1;
				actor frame STAND;			actor frame CAST_1;
				actor frame STAND;			actor frame SWING_3;
				actor frame SWING_2H_3;		attack;
				actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame STAND;
				actor frame CAST_1;			actor frame STAND;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}

spellTimeStop ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@An Tym@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 67;
				actor frame SWING_2H_3;		actor frame CAST_2;
				call spellTimeStop;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2H_3;
				actor frame CAST_2;			call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
		UI_stop_time(100);
}

spellMassResurrect ()
{
	if (event == DOUBLECLICK)
	{
		item_say("@Vas Mani Corp Hur@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 64;
				actor frame KNEEL;			actor frame STAND;
				actor frame CAST_1;			call spellMassResurrect;}
			UI_play_music(15, 0);
		}
		else
		{
			script item
			{	nohalt;						actor frame KNEEL;
				actor frame STAND;			actor frame CAST_1;
				call spellFails;}
		}
	}
	else if (event == SCRIPTED)
	{
		var bodyshapes = [SHAPE_BODIES_1, SHAPE_BODIES_2, SHAPE_LARGE_BODIES];
		var shnum;
		var index;
		var max;
		var bodies = [];
		for (shnum in bodyshapes with index to max)
			bodies = [bodies, find_nearby(shnum, 25, MASK_NONE)];
		var body;
		var xoff = [0, 1, 2, 1, 0, -1, -2, -1];
		var yoff = [2, 1, 0, -1, -2, -1, 0, 1];
		for (body in bodies with index to max)
		{
			var qual = body->get_item_quality();
			var quant = body->get_item_quantity(1);
			if ((qual != 0) || (quant != 0))
			{
				var pos = get_object_position();
				UI_sprite_effect(ANIMATION_LIGHTNING, pos[X], pos[Y], 0, 0, 0, -1);
				UI_sprite_effect(ANIMATION_GREEN_BUBBLES, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
				var dist = get_distance(body);
				script body after 2+dist/3 ticks call spellMassResurrect, EGG;
			}
		}
	}
	else
		resurrect();
}	
