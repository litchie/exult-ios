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
 *	This source file contains some reimplementations of all third
 *	circle spells.
 *
 *	There is also a new spell in the list: 'Remove Curse'.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

/*
	Third circle Spells
	
	extern spellCurse ();
	extern spellHeal ();
	extern spellParalyze ();
	extern spellPeer ();
	extern spellPoison ();
	extern spellProtectAll ();
	extern spellSleep ();
	extern spellSwarm ();
	extern spellRemoveCurse ();
*/

enum third_circle_spells
{
	SPELL_CURSE						= 0,
	SPELL_HEAL						= 1,
	SPELL_PARALYZE					= 2,
	SPELL_PEER						= 3,
	SPELL_POISON					= 4,
	SPELL_PROTECT_ALL				= 5,
	SPELL_SLEEP						= 6,
	SPELL_SWARM						= 7,
	SPELL_REMOVE_CURSE				= 8
};

spellCurse (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@Des Sanct@");
		if (inMagicStorm() && (target[X] != 0))
		{
			set_to_attack(target, SHAPE_CURSE);
			script item
			{	nohalt;						face dir;
				sfx 67;						actor frame SWING_1;
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

spellHeal (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@Mani@");
		if (inMagicStorm() && target->is_npc())
		{
			script item
			{	nohalt;						face dir;
				actor frame KNEEL;			sfx 64;
				actor frame STAND;			actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;}
				
			script target after 5 ticks
			{	nohalt;						call spellHeal;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame KNEEL;			actor frame STAND;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var str = get_npc_prop(STRENGTH);
		var hps = get_npc_prop(HEALTH);
		if (hps <= str)
		{
			var healquant = ((str - hps) / 2);
			set_npc_prop(HEALTH, healquant);
		}
	}
}

spellParalyze (var target)
{
	if ((event == DOUBLECLICK) || (event == WEAPON))
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@An Por@");
		if (inMagicStorm() && (target[X] != 0))
		{
			set_to_attack(target, SHAPE_PARALYZE);
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				actor frame SWING_3;		attack;
				actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellPeer ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Wis@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 67;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame CAST_1;			call spellPeer;}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame CAST_1;
				call spellFails;}
		}
	}

	else if (event == SCRIPTED)
		UI_display_map();
}

spellPoison (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@In Nox@");
		if (inMagicStorm() && (target[X] != 0))
		{
			set_to_attack(target, SHAPE_POISON);
			script item
			{	nohalt;						face dir;
				sfx 110;					actor frame SWING_1;
				actor frame SWING_3;		actor frame SWING_3;
				attack;						actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellProtectAll ()
{
	if (event == DOUBLECLICK)
	{
		item_say("@Vas Uus Sanct@");
		if (inMagicStorm())
		{
			halt_scheduled();
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame CAST_1;
				actor frame SWING_2H_3;		call spellProtectAll;}
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
		UI_play_sound_effect(109);
		var pos = get_object_position();
		UI_sprite_effect(7, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
		var targets = getFriendlyTargetList(item, 25);
		for (npc in targets)
			npc->set_item_flag(PROTECTION);
	}
}

spellSleep (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@In Zu@");
		if (inMagicStorm() && (target[X] != 0))
		{
			set_to_attack(target, SHAPE_SPELL_SLEEP);
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				actor frame SWING_3;		attack;
				actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
}

spellSwarm ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Kal Bet Xen@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				sfx 65;						call spellSwarm;}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var rand = UI_die_roll(7, 10);
		while (rand > 0)
		{
			rand = (rand - 1);
			UI_summon(SHAPE_FLY, false);
		}
	}
}

spellRemoveCurse (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@An Des Sanct@");
		if (inMagicStorm() && target->is_npc())
		{
			script item
			{	nohalt;						face dir;
				sfx 109;					actor frame SWING_1;
				actor frame CAST_2;			actor frame SWING_2H_3;}

			script target after 5 ticks
			{	nohalt;						call spellRemoveCurse;}
			
			obj_sprite_effect(13, -2, -2, 0, 0, 0, -1);
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
	else if (event == SCRIPTED)
	{
		clear_item_flag(CURSED);
		clear_item_flag(PARALYZED);
	}
}
