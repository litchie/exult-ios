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
 *	This source file contains some reimplementations of all seventh
 *	circle spells.
 *
 *	There is also a new spell in the list: 'Mass Dispel Field'.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

/*
	Seventh circle Spells
	
	extern spellCreateGold (var target);
	extern spellDeathBolt (var target);
	extern spellDelayedBlast (var target);
	extern spellEnergyField (var target);
	extern spellEnergyMist (var target);
	extern spellMassCharm ();
	extern spellMassMight ();
	extern spellRestoration ();
	extern spellMassDispelField ();
*/

enum seventh_circle_spells
{
	SPELL_CREATE_GOLD				= 0,
	SPELL_DEATH_BOLT				= 1,
	SPELL_DELAYED_BLAST				= 2,
	SPELL_ENERGY_FIELD				= 3,
	SPELL_ENERGY_MIST				= 4,
	SPELL_MASS_CHARM				= 5,
	SPELL_MASS_MIGHT				= 6,
	SPELL_RESTORATION				= 7,
	SPELL_MASS_DISPEL_FIELD			= 8
};

spellCreateGold (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@Rel Ylem@");
		if (inMagicStorm() && (target->get_item_shape() == SHAPE_LEAD_ORE))
		{
			script item
			{	nohalt;					face dir;
				sfx 66;					actor frame raise_1h;
				actor frame strike_1h;}
			var pos = target->get_object_position();
			UI_sprite_effect(13, pos[X], pos[Y], 0, 0, 0, -1);
			script target after 5 ticks
			{	nohalt;			call spellSetShape, SHAPE_GOLD_NUGGET;}
		}
		else
		{
			script item
			{	nohalt;					face dir;
				actor frame raise_1h;	actor frame strike_1h;
				call spellFails;}
		}
	}
}

spellDeathBolt (var target)
{
	if ((event == DOUBLECLICK) || (event == WEAPON))
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@Corp Por@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_DEATH_BOLT_NEW);
			script item
			{	nohalt;						face dir;
				actor frame cast_up;			sfx 65;
				actor frame cast_out;			actor frame cast_up;
				actor frame strike_2h;		attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame cast_up;			actor frame cast_out;
				actor frame cast_up;			actor frame strike_2h;
				call spellFails;}
		}
	}
}

spellDelayedBlast (var target)
{
	if (event == DOUBLECLICK)
	{
		var failed = false;
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		var sprite_pos = [target[X + 1], target[Y + 1], target[Z + 1]];
		item_say("@Tym Vas Flam@");
		if (inMagicStorm())
		{
			var sprite = UI_create_new_object(SHAPE_DELAYED_EXPLOSION);
			if (sprite)
			{
				sprite->set_item_flag(TEMPORARY);
				sprite->set_item_flag(INVISIBLE);
				var updlast = UI_update_last_created(sprite_pos);
				if (updlast)
				{
					sprite->set_npc_prop(HEALTH, 1);
					set_to_attack(sprite, SHAPE_DELAYED_EXPLOSION);
					script item after 12 ticks
					{	nohalt;						attack;}
					
					script sprite after 14 ticks
					{	nohalt;						remove;}
					
					script item
					{	nohalt;						face dir;
						sfx 65;						actor frame strike_2h;
						actor frame cast_out;			actor frame cast_up;}
					UI_sprite_effect(13, sprite_pos[X], sprite_pos[Y], 0, 0, 0, -1);
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
			{	nohalt;						face dir;
				actor frame strike_2h;		actor frame cast_out;
				actor frame cast_up;			call spellFails;}
		}
	}
}

spellEnergyField (var target)
{
	if (event == DOUBLECLICK)
	{
		var failed = false;
		halt_scheduled();
		//var target = UI_click_on_item();
		item_say("@In Sanct Grav@");
		var pos_x = (target[X + 1] + 1);
		var pos_y = (target[Y + 1] + 1);
		var pos_z = target[Z + 1];
		var pos = [pos_x, pos_y, pos_z];
		var notblocked = UI_is_not_blocked(pos, SHAPE_ENERGY_FIELD, 0);
		if (inMagicStorm() && notblocked)
		{
			script item
			{	nohalt;						actor frame raise_1h;
				actor frame reach_1h;		actor frame strike_1h;}
			var field = UI_create_new_object(SHAPE_ENERGY_FIELD);
			if (field)
			{
				var updlast = UI_update_last_created(pos);
				if (updlast)
				{
					var delay = 200;
					field->set_item_quality(delay);
					script field after delay ticks
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
			{	nohalt;						actor frame raise_1h;
				actor frame reach_1h;		actor frame strike_1h;
				call spellFails;}
		}
	}
}

spellEnergyMist (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@In Hur Grav Ylem@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_ENERGY_MYST);
			script item
			{	nohalt;						face dir;
				sfx 65;						actor frame cast_up;
				actor frame cast_out;			actor frame cast_up;
				actor frame strike_2h;		attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame cast_up;			actor frame cast_out;
				actor frame cast_up;			actor frame strike_2h;
				call spellFails;}
		}
	}
}

spellMassCharm ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas An Xen Ex@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				actor frame cast_up;			actor frame cast_out;
				actor frame cast_up;			actor frame strike_2h;}

			var pos = get_object_position();
			UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
			var dist = 25;
			var nonparty_npcs = getNearbyNonPartyNPCs(dist);
			for (npc in nonparty_npcs)
			{
				if (UI_die_roll(1, 3) == 1)
					continue;
				if (npc->get_npc_number() in [ANMANIVAS, FORANAMO, AVATAR])
					continue;
				var scr = new script {nohalt;};
				if (npc->get_alignment())
					scr << {call spellSetFlag, CHARMED;};
				else
					scr << {call spellClearFlag, CHARMED;};
				var delay = ((get_distance(npc) / 4) + 4);
				npc.run_script(scr, delay);
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame cast_up;
				actor frame cast_out;			actor frame cast_up;
				actor frame strike_2h;		call spellFails;}
		}
	}
}

spellMassMight ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@In Vas Por@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame cast_up;
				sfx 64;						actor frame cast_out;
				actor frame cast_up;			actor frame strike_2h;
				call  spellSetFlag, MIGHT;}
			var pos = get_object_position();
			UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);

			var targets = getFriendlyTargetList(item, 25);
			for (npc in targets)
			{
				var delay = ((get_distance(npc) / 3) + 5);
				script npc after delay ticks
				{	nohalt;			call spellSetFlag, MIGHT;}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame cast_up;
				actor frame cast_out;			actor frame cast_up;
				actor frame strike_2h;		call spellFails;}
		}
	}
}

spellRestoration ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Mani Hur@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame raise_1h;
				actor frame cast_out;			actor frame strike_2h;
				sfx 64;}
			var targets = getFriendlyTargetList(item, 25);
			for (npc in targets)
			{
				npc->clear_item_flag(PARALYZED);
				npc->clear_item_flag(POISONED);
				var str = npc->get_npc_prop(STRENGTH);
				var hps = npc->get_npc_prop(HEALTH);
				npc->set_npc_prop(HEALTH, (str - hps));
				npc->obj_sprite_effect(13, -1, -1, 0, 0, 0, -1);
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame raise_1h;
				actor frame cast_out;			actor frame strike_2h;
				call spellFails;}
		}
	}
}

spellMassDispelField ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		var field_shapes = [SHAPE_ENERGY_FIELD, SHAPE_FIRE_FIELD, SHAPE_POISON_FIELD, SHAPE_SLEEP_FIELD];
		item_say("@Vas An Grav@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 65;
				actor frame cast_up;			actor frame cast_out;
				actor frame strike_2h;}
				
			var dist = 25;
			for (obj in field_shapes)
			{
				var fields = find_nearby(obj, dist, MASK_NONE);
				for (field in fields)
				{
					var objshape = field->get_item_shape();
					var delay = ((get_distance(field) / 3) + 2);
					field->halt_scheduled();
					script field after delay ticks
					{	nohalt;						remove;}
				}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame cast_up;
				actor frame cast_out;			actor frame strike_2h;
				call spellFails;}
		}
	}
}
