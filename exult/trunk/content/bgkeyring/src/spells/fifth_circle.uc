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
 *	This source file contains some reimplementations of all fifth
 *	circle spells.
 *
 *	There is also a new spell in the list: 'Summon Skeletons'.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

/*
	Fifth circle Spells
	
	extern spellCharm ();
	extern spellDance ();
	extern spellDispelField ();
	extern spellExplosion ();
	extern spellFireField ();
	extern spellGreatHeal ();
	extern spellInvisibility ();
	extern spellMassSleep ();
	extern spellSummonSkeletons ();
*/

enum fifth_circle_spells
{
	SPELL_CHARM						= 0,
	SPELL_DANCE						= 1,
	SPELL_DISPEL_FIELD				= 2,
	SPELL_EXPLOSION					= 3,
	SPELL_FIRE_FIELD				= 4,
	SPELL_GREAT_HEAL				= 5,
	SPELL_INVISIBILITY				= 6,
	SPELL_MASS_SLEEP				= 7,
	SPELL_SUMMON_SKELETONS			= 8
};

spellCharm (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@An Xen Ex@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_SPELL_CHARM);
			script item
			{	nohalt;						face dir;
				sfx 68;						actor frame SWING_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}

spellDance ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Por Xen@");
		if (inMagicStorm())
		{
			item_say("@Everybody DANCE now!@");
			script item
			{	nohalt;						sfx 67;
				actor frame SWING_1;		actor frame CAST_2;
				actor frame SWING_2H_3;		call spellDance;}
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
		var dist = 25;
		var nonparty_npcs = getNearbyNonPartyNPCs(dist);
		for (npc in nonparty_npcs)
		{
			var intelligence = npc->get_npc_prop(INTELLIGENCE);
			if ((intelligence > 5) && (intelligence < 25))
			{
				var pos = npc->get_object_position();
				UI_sprite_effect(16, pos[X], pos[Y], 0, 0, 0, -1);
				setNonpartySchedule(npc, DANCE);
				npc->set_item_flag(DANCING);
				var barks = ["@Dance!@", "@Yeah!@", "@Huh!@", "@Oh, yeah!@", "@I'm bad!@", "@Boogie!@", "@Yow!@"];
				var rand = UI_die_roll(1, 7);
				var delay = UI_die_roll(10, 40);
				delayedBark(npc, barks[rand], delay);
				delay = UI_die_roll(50, 75);
				script npc after delay ticks
				{	nohalt;						call stopDancing;}
			}
		}
	}
}

spellDispelField (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		var fields = [SHAPE_ENERGY_FIELD, SHAPE_FIRE_FIELD, SHAPE_POISON_FIELD, SHAPE_SLEEP_FIELD];
		item_say("@An Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				face dir;					actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;}
				
			if (target->get_item_shape() in fields)
			{
				target->halt_scheduled();
				target->remove_item();
			}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}

spellExplosion (var target)
{
	if ((event == DOUBLECLICK) || (event == WEAPON))
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@Vas Flam Hur@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_SPELL_EXPLOSION);
			script item
			{	nohalt;						face dir;
				sfx 65;						actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_3;
				actor frame SWING_3;		attack;
				actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_3;		call spellFails;}
		}
	}
}

spellFireField (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@In Flam Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				face dir;					actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;}
				
			var field = UI_create_new_object(SHAPE_FIRE_FIELD);
			if (field)
			{
				var field_x = (target[X + 1] + 1);
				var field_y = (target[Y + 1] + 1);
				var field_z = target[Z + 1];
				var pos = [field_x, field_y, field_z];
				UI_update_last_created(pos);
				var duration = 100;
				field->set_item_quality(duration);
				field->set_item_flag(TEMPORARY);
				script field after duration ticks
				{	nohalt;						remove;}
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

spellGreatHeal (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		target->halt_scheduled();
		var dir = direction_from(target);
		item_say("@Vas Mani@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;		sfx 64;}
			if (target->is_npc())
			{
				var str = target->get_npc_prop(STRENGTH);
				var hps = target->get_npc_prop(HEALTH);
				target->set_npc_prop(HEALTH, (str - hps));
			}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
}

spellInvisibility (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@Sanct Lor@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						face dir;
				sfx 67;						actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;}
				
			script target after 4 ticks
			{	nohalt;						call spellInvisibility;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}

	else if (event == SCRIPTED)
		set_item_flag(INVISIBLE);
}

spellMassSleep ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		var pos = get_object_position();
		item_say("@Vas Zu@");
		if (inMagicStorm())
		{
			UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
			script item
			{	nohalt;						sfx 65;
				actor frame STAND;			actor frame SWING_1;
				actor frame CAST_1;			actor frame SWING_3;
				actor frame SWING_2H_3;		call spellMassSleep;}
		}
		else
		{
			script item
			{	nohalt;						actor frame STAND;
				actor frame SWING_1;		actor frame CAST_1;
				actor frame SWING_3;		actor frame SWING_2H_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var targets = getEnemyTargetList(item, 25);
		for (npc in targets)
		{
			npc->halt_scheduled();
			npc->set_item_flag(ASLEEP);
		}
	}
}

spellSummonSkeletons ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("Kal Corp Xen");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame KNEEL;
				actor frame STAND;			actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				sfx 65;						call spellSummonSkeletons;}
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
		var npclevel = getNPCLevel(item);
		if (npclevel < 2)
			npclevel = 2;

		var minroll = (npclevel / 2);
		if (minroll < 1)
			minroll = 1;

		var rand = UI_die_roll(minroll, npclevel);
		while (rand)
		{
			var summoned = SHAPE_SKELETON->summon(true);
			summoned->set_alignment(get_alignment());
			rand = (rand - 1);
		}
	}
}
