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
 *	This source file contains some reimplementations of all first
 *	circle spells.
 *
 *	There is also a new spell in the list: 'Translate'.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-03-19
 */

/*
	First circle Spells
	
	extern spellAwakenAll ();
	extern spellCreateFood ();
	extern spellCure (var target);
	extern spellDetectTrap ();
	extern spellGreatDouse ();
	extern spellGreatIgnite ();
	extern spellLight ();
	extern spellLocate ();
	extern spellTranslate ();
*/

enum first_circle_spells
{
	SPELL_AWAKEN_ALL				= 0,
	SPELL_CREATE_FOOD				= 1,
	SPELL_CURE						= 2,
	SPELL_DETECT_TRAP				= 3,
	SPELL_GREAT_DOUSE				= 4,
	SPELL_GREAT_IGNITE				= 5,
	SPELL_LIGHT						= 6,  
	SPELL_LOCATE					= 7,
	SPELL_TRANSLATE					= 8
};

spellAwakenAll ()
{
	if (event == DOUBLECLICK)
	{
		var pos = get_object_position();
		halt_scheduled();
		item_say("@Vas An Zu@");
		if (inMagicStorm())
		{
			UI_sprite_effect(7, (pos[X] - pos[Z]/2), (pos[Y] - pos[Z]/2), 0, 0, 0, -1);
			script item
			{	nohalt;						sfx 68;
				actor frame SWING_1;		actor frame SWING_3;}
			var dist = 25;
			var nearby_npcs = find_nearby(-1, dist, MASK_NPC);
			for (npc in nearby_npcs)
			{
				script npc
				{	nohalt;						call spellAwakenAll;}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		halt_scheduled();
		clear_item_flag(ASLEEP);
	}
}

spellCreateFood ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@In Mani Ylem@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 68;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;		call spellCreateFood;}
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
		//If the caster is not in party, this was just for 'show', i.e.,
		//a usecode-schedule display.
		if (!get_item_flag(IN_PARTY))
			return;

		var party = UI_get_party_list();
		
		for (npc in party)
		{
			var pos = npc->get_object_position();
			var fooditem = UI_create_new_object(SHAPE_FOOD);
			if (fooditem)
			{
				fooditem->set_item_frame(UI_die_roll(0, 31));
				fooditem->set_item_flag(TEMPORARY);
				UI_update_last_created(pos);
			}
		}
	}
}

spellCure (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@An Nox@");
		if (inMagicStorm())
		{
			if (target->is_npc())
			{
				script item
				{	nohalt;						face dir;
					sfx 64;						actor frame SWING_2;
					actor frame SWING_1;		actor frame SWING_3;}
				script target after 6 ticks
				{	nohalt;						call spellCure;}
				return;
			}
		}
		script item
		{	nohalt;						face dir;
			actor frame SWING_2;		actor frame SWING_1;
			actor frame SWING_3;		call spellFails;}
	}
	
	else if (event == SCRIPTED)
	{
		clear_item_flag(POISONED);
		clear_item_flag(PARALYZED);
	}
}

spellDetectTrap ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Wis Jux@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 66;
				actor frame SWING_1;		actor frame SWING_3;}
			var npclevel = getNPCLevel(item);
			var dist = (21 + npclevel);
			var nearby_traps = find_nearby(SHAPE_TRAP, dist, MASK_ALL_UNSEEN);
			for (trap in nearby_traps)
			{
				script trap after 5 ticks
				{	nohalt;						call spellDetectTrap, SCRIPTED;}
			}
			
			var openchests = find_nearby(SHAPE_CHEST, dist, MASK_ALL_UNSEEN);
			var closedchests = find_nearby(SHAPE_LOCKED_CHEST, dist, MASK_ALL_UNSEEN);
			var chests = (openchests & closedchests);
			for (trap in chests)
			{
				if (trap->get_item_quality() == KEY_PICKABLE_TRAPPED)
				{
					script trap after 5 ticks
					{	nohalt;						call spellDetectTrap, SCRIPTED;}
				}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var pos = get_object_position();
		UI_sprite_effect(16, pos[X], pos[Y], 0, 0, 0, -1);
	}
}

spellGreatDouse ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas An Flam@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;}
			
			var dousables = [SHAPE_TORCH_LIT, SHAPE_LIT_LAMP, SHAPE_LIGHTSOURCE_LIT, SHAPE_SCONCE_LIT];
			item->greatDouseIgnite(dousables);
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellGreatIgnite ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas In Flam@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;}
			
			var ignitables = [SHAPE_TORCH, SHAPE_LAMPPOST, SHAPE_LIGHTSOURCE, SHAPE_SCONCE];
			item->greatDouseIgnite(ignitables);
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_2;		actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellLight ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@In Lor@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 68;
				actor frame SWING_1;		actor frame SWING_3;
				call spellLight;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
		UI_cause_light(500);
}

spellLocate ()
{
	if (event == DOUBLECLICK)
	{
		item_say("@In Wis@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 67;
				actor frame KNEEL;			actor frame STAND;
				actor frame CAST_1;			wait 4;
				call spellLocate;}
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
		var pos = get_object_position();
		var long = ((pos[X] - 0x3A5) / 10);
		var lat = ((pos[Y] - 0x46E) / 10);
		var longstr;
		var latstr;

		if (long < 0)
			longstr = " " + absoluteValueOf(long) + " West";
		else
			longstr = " " + absoluteValueOf(long) + " East";

		if (lat < 0)
			latstr = " " + absoluteValueOf(lat) + " North";
		else
			latstr = " " + absoluteValueOf(lat) + " South";

		item_say((latstr + longstr));
	}
}

spellTranslate ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Rel Wis@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		actor frame SWING_3;
				actor frame STAND;}
			AVATAR->set_item_flag(READ);
			script AVATAR after 10000 ticks
			{	nohalt;						finish;
				call spellTranslate;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	else if (event == SCRIPTED)
		AVATAR->clear_item_flag(READ);
}
