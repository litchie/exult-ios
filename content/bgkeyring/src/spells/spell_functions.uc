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
 *	This source file contains some spellcasting related constants, as well as
 *	a few general-purpose spell functions. IT IS NOT FINISHED YET!
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

const int SCRIPT_FACE_DIR			= 0x59;
const int SCRIPT_NPC_FRAME			= 0x61;

//Cool new spell stuff:
enum npc_wizard_flags
{
	//New spell effects:
	CONFUSION						= 40,	//Under the effects of Consufion spell
	DEATH_PROTECTION				= 41,	//Protected from death spells
	//Spellcasting power:
	ARCHWIZARD						= 45,	//Cheat mode
	MAGE_CLASS						= 46,	//Full spellcasting capabilities
	BARD_CLASS						= 47,	//1/2 level spellcasting
	WARMAGE							= 48,	//Combat spells cost half as much mana (round up)
	SUMMONER						= 49,	//Summoning spells cost half as much mana (round up)
	HEALER							= 50,	//Casts healing as if level 8, no death spells
	NECROMANCER						= 51,	//Casts death as if level 8, no healing spells
	//The next constants are used to control spellcaster AI; the AI is NOT
	//IMPLEMENTED YET.
	AI_CAST_DONT_CAST				= 52,	//AI will not cast any spells
	AI_CAST_VERY_FEW				= 53,	//AI will cast every 32-36 ticks
	AI_CAST_FEW						= 54,	//AI will cast every 24-28 ticks
	//AI_CAST_DEFAULT						//AI will cast every 16-20 ticks
	AI_CAST_MANY					= 55,	//AI will cast every 8-12 ticks
	AI_HEALING						= 56,	//AI will try to cast healing spells first
	AI_SUPPORT						= 57,	//AI will try to cast support spells first
	AI_OFFENSE						= 58,	//AI will try to cast damaging combat first
	AI_SUMMON						= 59,	//AI will try to summon creatures first
	AI_ENCHANT						= 60,	//AI will try to cast non-damage combat spells
	AI_DEATH						= 61,	//AI will try to cast death spells first
	AI_MISDIRECTION					= 62,	//AI will try to use invisibility+blink+mass confusion very early and often
	AI_TEMPORARY					= 63	//AI will stop casting when it runs out of targets
};

greatDouseIgnite (var shapes)
{
	var dist = 25;
	var index;
	var max;
	var obj;
	
	for (obj in shapes with index to max)
	{
		//For each shape # in the array,
		var index2;
		var max2;
		var lightsource;
		//Find all nearby objects with that shape...
		var lsources = find_nearby(obj, dist, MASK_NONE);
		for (lightsource in lsources with index2 to max2)
		{
			//For each object in the array,
			//calculate a delay based on distance:
			var delay = ((get_distance(lightsource) / 3) + 2);
			//Call the obj's usecode function after delay ticks:
			script lightsource after delay ticks
			{	nohalt;					call lightsource->get_usecode_fun(), DOUBLECLICK;}
		}
	}
}

DeathBoltHit 0xB7F ()
{
	//Bail out unless an NPC is hit:
	if (is_npc())
	{
		//Get NPC's intelligence:
		var targetint = get_npc_prop(INTELLIGENCE);
		//See if he can't die:
		var cantdie = get_item_flag(CANT_DIE);
		//Opposed roll to see if we have a corpse in hands:
		if (UI_roll_to_win(20, targetint) && !cantdie)
			script item hit 127;
	}
}
