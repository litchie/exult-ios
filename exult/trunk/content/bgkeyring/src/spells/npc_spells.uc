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
 *	This source file contains functions used by NPCs for spellcasting.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

enum cast_spell_results
{
	CASTING_SUCCESSFUL				= 0,
	NOT_ENOUGH_REAGENTS				= 1,
	NOT_ENOUGH_MANA					= 2,
	NO_SUCH_SPELL					= 3
};

var isTargetedSpell(var circle, var spell)
{
	var result;
	if (circle == 0)	//linear spells
		//				 Awaken			Douse			Fireworks		Glimmer
		//				 Ignite			Thunder			Weather			Detect charges
		result = 		[true,			true,			false,			false,
						 true,			false,			false,			true];
	else if (circle == 1)
		//				 Awaken all		Create food		Cure			Detect trap
		//				 Great douse	Great ignite	Light			Locate
		//				 Translate
		result = 		[false,			false,			true,			false,
						 false,			false,			false,			false,
						 false];
	else if (circle == 2)
		//				 Destroy trap	Enchant			Fire blast		Great light
		//				 Mass cure		Protection		Telekinesis		Wizard eye
		result = 		[true,			true,			true,			false,
						 false,			true,			true,			false];
	else if (circle == 3)
		//				 Curse			Heal			Paralyze		Peer
		//				 Poison			Protect all		Sleep			Swarm
		//				 Remove Curse
		result = 		[true,			true,			true,			false,
						 true,			false,			true,			false,
						 true];
	else if (circle == 4)
		//				 Conjure		Lightning		Mass curse		Reveal
		//				 Seance			Unlock magic	Recharge Magic	Blink
		result = 		[false,			true,			false,			false,
						 false,			true,			true,			true];
	else if (circle == 5)
		//				 Charm			Dance			Dispel field	Explosion
		//				 Fire field		Great heal		Invisibility	Mass sleep
		//			Summon Skeletons
		result = 		[true,			false,			true,			true,
						 true,			true,			true,			false,
						 false];
	else if (circle == 6)
		//				 Cause fear		Clone			Fire ring		Flame strike
		//				 Magic storm	Poison field	Sleep field		Tremor
		result = 		[false,			true,			true,			false,
						 false,			true,			true,			false];
	else if (circle == 7)
		//				 Create gold	Death bolt		Delayed blast	Energy field
		//				 Energy mist	Mass charm		Mass might		Restoration
		//			Mass Dispel Field
		result = 		[true,			true,			true,			true,
						 true,			false,			false,			false,
						 false];
	else if (circle == 8)
		//				 Death vortex	Invis. all		Mass death	Resurrect
		//				 Summon			Swordstrike		Time stop		Fire snake
		//			Mass Resurrect
		result = 		[true,			false,			false,			true,
						 false,			true,			false,			true,
						 false];
	return result[spell + 1];
}

var getSpellList(var circle)
{
	var spell_list;

	if (circle == 0)	//linear spells
		spell_list = ["none",
					  "Awaken", "Douse", "Fireworks", "Glimmer",
					  "Ignite", "Thunder", "Weather", "Detect charges"];
	else if (circle == 1)
		spell_list = ["none",
					  "Awaken all", "Create food", "Cure", "Detect trap",
					  "Great douse", "Great ignite", "Light", "Locate",
					  "Translate"];
	else if (circle == 2)
		spell_list = ["none",
					  "Destroy trap", "Enchant", "Fire blast", "Great light",
					  "Mass cure", "Protection", "Telekinesis", "Wizard eye"];
	else if (circle == 3)
		spell_list = ["none",
					  "Curse", "Heal", "Paralyze", "Peer",
					  "Poison", "Protect all", "Sleep", "Swarm",
					  "Remove Curse"];
	else if (circle == 4)
		spell_list = ["none",
					  "Conjure", "Lightning", "Mass curse", "Reveal",
					  "Seance", "Unlock magic", "Recharge Magic", "Blink"];
	else if (circle == 5)
		spell_list = ["none",
					  "Charm", "Dance", "Dispel field", "Explosion",
					  "Fire field", "Great heal", "Invisibility", "Mass sleep",
					  "Summon Skeletons"];
	else if (circle == 6)
		spell_list = ["none",
					  "Cause fear", "Clone", "Fire ring", "Flame strike",
					  "Magic storm", "Poison field", "Sleep field", "Tremor"];
	else if (circle == 7)
		spell_list = ["none",
					  "Create gold", "Death bolt", "Delayed blast", "Energy field",
					  "Energy mist", "Mass charm", "Mass might", "Restoration",
					  "Mass Dispel Field"];
	else if (circle == 8)
		spell_list = ["none",
					  "Death vortex", "Invisibility all", "Mass death", "Resurrect",
					  "Summon", "Swordstrike", "Time stop", "Fire snake",
					  "Mass resurrect"];
	return spell_list;
}

var getSpellReagents (var circle, var spell)
{
	if (circle == 0)	//linear spells
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		return [0, 0, 0, 0,
				0, 0, 0, 0];
	}
	else if (circle == 1)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_AWAKEN_ALL)
			return [0, 0, 0, 0,
					1, 1, 0, 0];
		else if (spell == SPELL_CREATE_FOOD)
			return [0, 0, 0, 1,
					1, 1, 0, 0];
		else if (spell == SPELL_CURE)
			return [0, 0, 0, 0,
					1, 1, 0, 0];
		else if (spell == SPELL_DETECT_TRAP)
			return [0, 0, 1, 0,
					0, 0, 1, 0];
		else if (spell == SPELL_GREAT_DOUSE)
			return [0, 0, 0, 0,
					1, 0, 1, 0];
		else if (spell == SPELL_GREAT_IGNITE)
			return [0, 0, 0, 0,
					0, 0, 1, 1];
		else if (spell == SPELL_LIGHT)
			return [0, 0, 0, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_LOCATE)
			return [0, 0, 1, 0,
					0, 0, 0, 0];
		else if (spell == SPELL_TRANSLATE)
			return [0, 0, 1, 0,
					0, 0, 0, 0];
	}
	else if (circle == 2)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_DESTROY_TRAP)
			return [0, 1, 0, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_ENCHANT)
			return [1, 0, 0, 1,
					0, 0, 0, 0];
		else if (spell == SPELL_FIRE_BLAST)
			return [1, 0, 0, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_GREAT_LIGHT)
			return [0, 0, 0, 1,
					0, 0, 0, 1];
		else if (spell == SPELL_MASS_CURE)
			return [0, 0, 0, 1,
					1, 1, 0, 0];
		else if (spell == SPELL_PROTECTION)
			return [0, 0, 0, 0,
					1, 1, 0, 1];
		else if (spell == SPELL_TELEKINESIS)
			return [1, 1, 0, 1,
					0, 0, 0, 0];
		else if (spell == SPELL_WIZARD_EYE)
			return [1, 1, 1, 1,
					0, 0, 1, 1];
	}
	else if (circle == 3)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_CURSE)
			return [0, 0, 1, 0,
					1, 0, 0, 1];
		else if (spell == SPELL_HEAL)
			return [0, 0, 0, 0,
					1, 1, 1, 0];
		else if (spell == SPELL_PARALYZE)
			return [0, 0, 1, 0,
					0, 0, 1, 0];
		else if (spell == SPELL_PEER)
			return [0, 0, 1, 1,
					0, 0, 0, 0];
		else if (spell == SPELL_POISON)
			return [1, 1, 1, 0,
					0, 0, 0, 0];
		else if (spell == SPELL_PROTECT_ALL)
			return [0, 0, 0, 1,
					1, 1, 0, 1];
		else if (spell == SPELL_SLEEP)
			return [1, 0, 1, 0,
					0, 0, 1, 0];
		else if (spell == SPELL_SWARM)
			return [0, 1, 1, 1,
					0, 0, 0, 0];
		else if (spell == SPELL_REMOVE_CURSE)
			return [0, 0, 0, 0,
					1, 1, 0, 1];
	}
	else if (circle == 4)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_CONJURE)
			return [0, 0, 0, 1,
					0, 0, 1, 0];
		else if (spell == SPELL_LIGHTNING)
			return [1, 0, 0, 1,
					0, 0, 0, 1];
		else if (spell == SPELL_MASS_CURSE)
			return [0, 0, 1, 1,
					1, 0, 0, 1];
		else if (spell == SPELL_REVEAL)
			return [0, 1, 0, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_SEANCE)
			return [0, 1, 1, 1,
					0, 0, 1, 1];
		else if (spell == SPELL_UNLOCK_MAGIC)
			return [0, 1, 0, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_RECHARGE_MAGIC)
			return [0, 0, 0, 1,
					0, 0, 1, 1];
		else if (spell == SPELL_BLINK)
			return [0, 1, 0, 1,
					0, 0, 0, 0];
	}
	else if (circle == 5)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_CHARM)
			return [1, 0, 1, 0,
					0, 0, 1, 0];
		else if (spell == SPELL_DANCE)
			return [0, 1, 0, 1,
					1, 0, 0, 0];
		else if (spell == SPELL_DISPEL_FIELD)
			return [1, 0, 0, 0,
					1, 0, 1, 1];
		else if (spell == SPELL_EXPLOSION)
			return [1, 1, 0, 1,
					0, 0, 0, 1];
		else if (spell == SPELL_FIRE_FIELD)
			return [1, 0, 0, 0,
					0, 0, 1, 1];
		else if (spell == SPELL_GREAT_HEAL)
			return [0, 0, 0, 1,
					1, 1, 1, 0];
		else if (spell == SPELL_INVISIBILITY)
			return [0, 1, 1, 0,
					0, 0, 0, 0];
		else if (spell == SPELL_MASS_SLEEP)
			return [0, 0, 1, 0,
					0, 1, 1, 0];
		else if (spell == SPELL_SUMMON_SKELETONS)
			return [0, 1, 0, 1,
					1, 0, 0, 0];
	}
	else if (circle == 6)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_CAUSE_FEAR)
			return [0, 0, 1, 1,
					1, 0, 0, 0];
		else if (spell == SPELL_CLONE)
			return [0, 1, 1, 1,
					0, 1, 1, 1];
		else if (spell == SPELL_FIRE_RING)
			return [1, 0, 0, 1,
					0, 0, 1, 1];
		else if (spell == SPELL_FLAME_STRIKE)
			return [1, 1, 0, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_MAGIC_STORM)
			return [0, 1, 1, 1,
					0, 0, 0, 1];
		else if (spell == SPELL_POISON_FIELD)
			return [1, 0, 1, 0,
					0, 0, 1, 0];
		else if (spell == SPELL_SLEEP_FIELD)
			return [1, 0, 0, 0,
					0, 1, 1, 0];
		else if (spell == SPELL_TREMOR)
			return [0, 1, 0, 1,
					0, 0, 0, 1];
	}
	else if (circle == 7)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_CREATE_GOLD)
			return [0, 0, 0, 1,
					0, 0, 1, 0];
		else if (spell == SPELL_DEATH_BOLT)
			return [1, 0, 1, 0,
					0, 0, 0, 1];
		else if (spell == SPELL_DELAYED_BLAST)
			return [1, 1, 0, 1,
					0, 0, 1, 1];
		else if (spell == SPELL_ENERGY_FIELD)
			return [1, 0, 0, 1,
					0, 0, 1, 1];
		else if (spell == SPELL_ENERGY_MIST)
			return [0, 1, 1, 1,
					0, 0, 0, 1];
		else if (spell == SPELL_MASS_CHARM)
			return [1, 0, 1, 1,
					0, 0, 1, 0];
		else if (spell == SPELL_MASS_MIGHT)
			return [1, 0, 0, 1,
					0, 1, 0, 0];
		else if (spell == SPELL_RESTORATION)
			return [0, 0, 0, 1,
					1, 1, 0, 1];
		else if (spell == SPELL_MASS_DISPEL_FIELD)
			return [1, 0, 0, 1,
					1, 0, 1, 1];
	}
	else if (circle == 8)
	{
		//black pearl,	blood moss,	nightshade,		mandrake root,
		//garlic,		ginseng,	spider silk,	sulphurous ash
		if (spell == SPELL_DEATH_VORTEX)
			return [0, 1, 1, 1,
					0, 0, 0, 1];
		else if (spell == SPELL_INVISIBILITY_ALL)
			return [1, 1, 1, 1,
					0, 0, 0, 0];
		else if (spell == SPELL_MASS_DEATH)
			return [0, 1, 1, 1,
					1, 1, 0, 0];
		else if (spell == SPELL_RESURRECT)
			return [0, 0, 0, 0,
					1, 1, 1, 1];
		else if (spell == SPELL_SUMMON)
			return [0, 1, 0, 1,
					1, 0, 0, 0];
		else if (spell == SPELL_SWORDSTRIKE)
			return [1, 0, 1, 1,
					0, 0, 0, 0];
		else if (spell == SPELL_TIME_STOP)
			return [0, 1, 0, 1,
					1, 0, 0, 0];
		else if (spell == SPELL_MASS_RESURRECT)
			return [0, 0, 0, 1,
					1, 1, 1, 1];
			//return [NO_SUCH_SPELL];
	}
}

var getCircleList(var npc)
{
	var circles;
	
	circles = ["Linear", "First"];

	var npclevel = getNPCLevel(npc);
	if (npclevel > 8 || npc->get_item_flag(ARCHWIZARD))
		npclevel = 8;
	if (npc->get_item_flag(BARD_CLASS))
		npclevel = (npclevel+1)/2;	//Effectively rounding up
	
	if (npclevel > 1)
		circles = [circles, "Second"];
	if (npclevel > 2)
		circles = [circles, "Third"];
	if (npclevel > 3)
		circles = [circles, "Fourth"];
	if (npclevel > 4)
		circles = [circles, "Fifth"];
	if (npclevel > 5)
		circles = [circles, "Sixth"];
	if (npclevel > 6)
		circles = [circles, "Seventh"];
	if (npclevel > 7)
		circles = [circles, "Eighth"];
	
	circles = ["none", circles];
	return circles;
}

var getSpellCircle (var spellname)
{
	var circle;
	var circlelist;

	while (circle < 8)
	{
		circlelist = getSpellList(circle);
		if (spellname in circlelist)
			break;

		circle = circle + 1;
	}
	return circle;
}

var getIndexForSpell(var circle, var spellname)
{
	var spelllist = getSpellList(circle);
	for (spell in spelllist with index)
		if (spell == spellname)
			return index - 2;
}

var getLeveledSpellList (var npc, var ignore_npc_level, var spelllist, var levels, var removespells)
{
	var list = [];
	var index;
	
	var npclevel = getNPCLevel(npc);
	if (ignore_npc_level || npclevel > 8 || npc->get_item_flag(ARCHWIZARD))
		npclevel = 8;
	if (npc->get_item_flag(BARD_CLASS))
		npclevel = (npclevel+1)/2;	//Effectively rounding up
	
	index = 1;
	while (index <= UI_get_array_size(spelllist))
	{
		if (levels[index] > npclevel)
			break;
			
		if (!(spelllist[index] in removespells))
			list = [list, spelllist[index]];
		
		index = index + 1;
	}
	return list;
}

var getSpellFunction (var circle, var spell)
{
	if (circle == 0)	//linear spells
		return &spellAwaken + spell;
	else if (circle == 1)
		return &spellAwakenAll + spell;
	else if (circle == 2)
		return &spellDestroyTrap + spell;
	else if (circle == 3)
		return &spellCurse + spell;
	else if (circle == 4)
		return &spellConjure + spell;
	else if (circle == 5)
		return &spellCharm + spell;
	else if (circle == 6)
		return &spellCauseFear + spell;
	else if (circle == 7)
		return &spellCreateGold + spell;
	else if (circle == 8)
		return &spellDeathVortex + spell;
}	

var npcCastSpell (var npc, var target, var circle, var spell)
{
	var party = UI_get_party_list();
	var npcmana = npc->get_npc_prop(MANA);
	var maxmana = npc->get_npc_prop(MAX_MANA);

	if (((npc in party) && !npc->get_item_flag(ARCHWIZARD) ||
		(!(npc in party) && (npc->get_schedule_type() == WAIT))))
	{
		var needed_reagents = getSpellReagents(circle, spell);
		
		var reagent_names =			["black pearl",	"blood moss",	"nightshade",	"mandrake root",
									 "garlic",		"ginseng",		"spider silk",	"sulphurous ash"];
		var singular_prefixes =		["",			"portion of",	"button of",	"",
									 "clove of",	"portion of",	"portion of",	"portion of"];
		var plural_prefixes =		["",			"portions of",	"buttons of",	"",
									 "cloves of",	"portions of",	"portions of",	"portions of"];
		var plural_suffixes = 		["s",			"",				"",				"s",
									 "",			"",				"",				""];
		var reagent_frames = [1, 2, 3, 4, 5, 6, 7, 8];
		var lacking_reagents = [0, 0, 0, 0, 0, 0, 0, 0];
		var reagent_count;
		var missing_count;
		var ret_str;
		
		for (reagent in reagent_frames)
		{
			reagent_count = npc->count_objects(SHAPE_REAGENT, QUALITY_ANY, reagent - 1);
			if (!(reagent_count >= needed_reagents[reagent]))
			{
				missing_count = missing_count + 1;
				lacking_reagents[reagent] = needed_reagents[reagent] - reagent_count;
			}
		}
	
		if (missing_count)
		{
			var currcount = 0;
			for (reagent in reagent_frames)
			{
				if (lacking_reagents[reagent])
				{
					currcount = currcount + 1;
					if ((currcount == missing_count) && (currcount != 1))
						ret_str = ret_str + " and ";
					else if (currcount != 1)
						ret_str = ret_str + ", ";
						
					ret_str = ret_str + lacking_reagents[reagent];
					ret_str = ret_str + " ";
					if (lacking_reagents[reagent] > 1)
						ret_str = ret_str + plural_prefixes[reagent];
					else
						ret_str = ret_str + singular_prefixes[reagent];
						
					ret_str = ret_str + " ";
					ret_str = ret_str + reagent_names[reagent];
					if (lacking_reagents[reagent] > 1)
					{
						ret_str = ret_str + " ";
						ret_str = ret_str + plural_suffixes[reagent];
					}
				}
			}
			ret_str = ret_str + ".@";
			return [NOT_ENOUGH_REAGENTS, ret_str];
		}
		
		if (npcmana < circle)
			return [NOT_ENOUGH_MANA, (maxmana < circle)];
		
		for (reagent in reagent_frames)
			npc->remove_cont_items(needed_reagents[reagent], SHAPE_REAGENT, QUALITY_ANY, reagent - 1, true);
	}
	
	npc->begin_casting_mode();
	
	npc->halt_scheduled();
	var spell_function = getSpellFunction(circle, spell);
	event = DOUBLECLICK;
	if (isTargetedSpell(circle, spell))
	{
		if (!target)
			target = UI_click_on_item();
		npc->(*spell_function)(target);
	}
	else
		npc->(*spell_function)();
	
	//Return NPC to standing frame once spellcasting is done
	script npc after 12 ticks actor frame STAND;
	if (!(npc in party) || npc->get_item_flag(ARCHWIZARD)) circle = 0;
	return [CASTING_SUCCESSFUL, circle];
}

npcAskSpellToCast (var npc, var talk, var removespells)
{
	UI_push_answers();
	
	var circle;
	var spelllist;
	var spell;
	var casting_return;
	var casting_status;
	var circle_list;

	circle_list = getCircleList(npc);
	
	say(talk[1]);
	while (true)
	{
		circle = chooseFromMenu2(circle_list) - 2;

		if (circle == -1)
			break;
		else
		{
			spelllist = removeItemsFromList(getSpellList(circle), removespells);

			say(talk[2]);
			spell = chooseFromMenu2(spelllist);
			if (spell == 1)
				say(talk[3]);
			else
			{
				//Determine the true spell index:
				spell = getIndexForSpell(circle, spelllist[spell]);
				casting_return = npcCastSpell(npc, 0, circle, spell);
				var casting_result = casting_return[1];
				if (casting_result == CASTING_SUCCESSFUL)
				{
					//casting_return[2] is the mana cost of the spell.
					//Only do this if the spell cost any mana at all.
					if (casting_return[2] != 0)
						npc->set_npc_prop(MANA, -casting_return[2]);

					//The casting succeeded; abort so the spell can be cast:
					abort;
				}
				else if (casting_result == NOT_ENOUGH_REAGENTS)
				{
					//Not enough reagents; casting_return[2] is the string telling which
					//reagents are missing:
					say(talk[5]);
					say(talk[6] + casting_return[2]);
				}
				else if (casting_result == NOT_ENOUGH_MANA)
				{
					//Not enough mana; casting_return[2] tells if the mana cost is higher
					//than the npc's max mana, case in which he couldn't cast the spell
					//even after resting:
					if (casting_return[2])
						say(talk[7]);
					else
						say(talk[8]);
				}
				else if (casting_result == NO_SUCH_SPELL)
					//This should never happen...
					say("@Alas, I am afraid that spell doesn't exist. Warn the programmer, something is amiss!@");
			}
		}
	}
	
	say(talk[4]);
	UI_pop_answers();
}

npcCastSpellDialog (var npc, var spell, var talk)
{
	var circle = getSpellCircle(spell);
	
	npc.hide();
	var casting_return = npcCastSpell(npc, 0, circle, getIndexForSpell(circle, spell));
	var casting_result = casting_return[1];
	
	if (casting_result == CASTING_SUCCESSFUL)
	{
		
		//casting_return[2] is the mana cost of the spell.
		//Only do this if the spell cost any mana at all.
		if (casting_return[2] != 0)
			npc->set_npc_prop(MANA, -casting_return[2]);
		
		//The casting succeeded; abort so the spell can be cast:
		abort;
	}
	else if (casting_result == NOT_ENOUGH_REAGENTS)
	{
		//Not enough reagents; casting_return[2] is the string telling which
		//reagents are missing:
		npc.say(talk[5]);
		say(talk[6] + casting_return[2]);
	}
	else if (casting_result == NOT_ENOUGH_MANA)
	{
		if (casting_return[2])
			npc.say(talk[7]);
		else
			npc.say(talk[8]);
	}
	else if (casting_result == NO_SUCH_SPELL)
		//This should never happen...
		npc.say("@Alas, I am afraid that spell doesn't exist. Warn the programmer, something is amiss!@");
	npc.hide();
}

npcCastSpellBark (var npc, var target, var circle, var spell)
{
	var casting_return = npcCastSpell(npc, target, circle, spell);
	var casting_result = casting_return[1];
	
	if (casting_result == CASTING_SUCCESSFUL)
	{
		//casting_return[2] is the mana cost of the spell.
		//Only do this if the spell cost any mana at all.
		if (casting_return[2] != 0)
			npc->set_npc_prop(MANA, -casting_return[2]);
		
		//The casting succeeded; abort so the spell can be cast:
		abort;
	}

	else if (casting_result == NO_SUCH_SPELL)
		//This should never happen...
		say("@Alas, I am afraid that spell doesn't exist. Warn the programmer, something is amiss!@");
}

npcCastWeaponSpell (var npc, var target, var spellitem, var spell, var barks)
{
	if (!spellitem)
	{
		if (npc in UI_get_party_list())
			npc->item_say("@That is -my- spellbook!@");
		else
			randomPartyMember()->item_say(barks[1]);
		flashBlocked(0);
		abort;
	}
	
	else if (!spell)
	{
		npc->item_say(barks[2]);
		flashBlocked(0);
		abort;
	}
	
	var circle;
	if (spell == "Fire blast")
		circle = 2;
	else if (spell == "Lightning")
		circle = 4;
	else if (spell == "Explosion")
		circle = 5;
	else if (spell == "Death bolt")
		circle = 7;
	else if (spell == "Swordstrike")
		circle = 8;
	
	var casting_return = npcCastSpell(npc, target, circle, getIndexForSpell(circle, spell));
	var casting_result = casting_return[1];
	
	if (casting_result == CASTING_SUCCESSFUL)
	{
		//casting_return[2] is the mana cost of the spell.
		//Only do this if the spell cost any mana at all.
		if (casting_return[2] != 0)
			npc->set_npc_prop(MANA, -casting_return[2]);

		//The casting succeeded; abort so the spell can be cast:
		abort;
	}
	else if (casting_result == NOT_ENOUGH_REAGENTS)
		npc->item_say("Need reagents");

	else if (casting_result == NOT_ENOUGH_MANA)
	{
		if (casting_return[2])
			npc->item_say("Can't cast");
		else
			npc->item_say("Must rest");
	}
	else if (casting_result == NO_SUCH_SPELL)
		//This should never happen...
		npc->item_say("No spell");
}

npcCastHealing (var npc, var talk_cast, var talk, var healing_spells)
{
	var choice;
	while (true)
	{
		npc.say(talk[1]);
		choice = askForResponse(["none", healing_spells]);
		if (choice != "none")
		{
			/*if ((choice == "Restoration") || (choice == "Mass cure"))
				say(talk[2]);

			else
			{
				message(talk[3]);
				if ((choice == "Heal") || (choice == "Great heal")) message("healed");
				else if (choice == "Cure") message("cured of poison");
				else message("resurrected");
				message("?@");
				say();
			}*/
			
			npcCastSpellDialog(npc,
				choice,
				talk_cast);
		}	
		else
			break;
	}	
}
