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
	
	extern spellCurse (var target);
	extern spellHeal (var target);
	extern spellParalyze (var target);
	extern spellPeer ();
	extern spellPoison (var target);
	extern spellProtectAll ();
	extern spellSleep (var target);
	extern spellSwarm ();
	extern spellRemoveCurse (var target);
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
				sfx 67;						actor frame raise_1h;
				actor frame cast_out;			actor frame strike_2h;
				attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame raise_1h;		actor frame cast_out;
				actor frame strike_2h;		call spellFails;}
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
				actor frame kneeling;			sfx 64;
				actor frame standing;			actor frame reach_1h;
				actor frame raise_1h;		actor frame strike_1h;}
				
			script target after 5 ticks
			{	nohalt;						call spellHealEffect;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame kneeling;			actor frame standing;
				actor frame reach_1h;		actor frame raise_1h;
				actor frame strike_1h;		call spellFails;}
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
				actor frame raise_1h;		actor frame strike_1h;
				actor frame strike_1h;		attack;
				actor frame standing;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame raise_1h;		actor frame strike_1h;
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
				actor frame cast_up;			actor frame cast_out;
				actor frame cast_up;			call spellShowMap;}
		}
		else
		{
			script item
			{	nohalt;						actor frame cast_up;
				actor frame cast_out;			actor frame cast_up;
				call spellFails;}
		}
	}
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
				sfx 110;					actor frame raise_1h;
				actor frame strike_1h;		actor frame strike_1h;
				attack;						actor frame standing;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame raise_1h;		actor frame strike_1h;
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
			{	nohalt;						actor frame cast_up;
				actor frame cast_out;			actor frame cast_up;
				actor frame strike_2h;		call spellProtectAllEffect;}
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
				actor frame raise_1h;		actor frame strike_1h;
				actor frame strike_1h;		attack;
				actor frame standing;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame raise_1h;		actor frame strike_1h;
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
			{	nohalt;						actor frame cast_up;
				actor frame cast_out;			actor frame strike_2h;
				sfx 65;						call spellSwarmEffect;}
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
				sfx 109;					actor frame raise_1h;
				actor frame cast_out;			actor frame strike_2h;}

			script target after 5 ticks
			{	nohalt;
				call spellClearFlag, CURSED;
				call spellClearFlag, PARALYZED;}
			
			obj_sprite_effect(13, -2, -2, 0, 0, 0, -1);
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame raise_1h;		actor frame cast_out;
				actor frame strike_2h;		call spellFails;}
		}
	}
}
