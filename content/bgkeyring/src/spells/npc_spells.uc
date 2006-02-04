/*
 *	This source file contains functions used by NPCs for spellcasting.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-02-03
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
		result = 		[true,			false,			false,			true,
						 false,			true,			false,			true];
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
					  "Summon", "Swordstrike", "Time stop", "Fire snake"];
	return spell_list;
}

var removeSpellsFromList(var spell_list, var spells_to_remove)
{
	var spell;
	var index;
	var max;
	var newarray = [];

	if (UI_get_array_size(spells_to_remove) > 1)
	{
		for (spell in spell_list with index to max)
		{
			if (!(spell in spells_to_remove))
				newarray = [newarray, spell];
		}
	}
	else if (spells_to_remove)
	{
		for (spell in spell_list with index to max)
		{
			if (!(spell == spells_to_remove[1]))
				newarray = [newarray, spell];
		}
	}
	else
		newarray = spell_list;
	
	return newarray;
}

var getCircleList(var npc, var ignore_npc_level)
{
	var circles;
	
	if (ignore_npc_level)
		circles = ["none", "Linear", "First", "Second", "Third", "Fourth", "Fifth", "Sixth", "Seventh", "Eighth"];
	
	else
	{
		circles = ["Linear", "First"];
	
		var npclevel = getNPCLevel(npc);
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
	}
		
	return circles;
}

var getIndexForSpell(var circle, var spellname)
{
	var spell;
	var spelllist = getSpellList(circle);
	var index;
	var max;
	
	for (spell in spelllist with index to max)
		if (spell == spellname)
			return index - 2;
}

var getLeveledSpellList (var npc, var ignore_npc_level, var spelllist, var levels, var removespells)
{
	var list = [];
	var index;
	var npclevel;
	
	if (ignore_npc_level)
		npclevel = 8;
	else
		npclevel = getNPCLevel(npc);
	
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

var npcCastSpell (var npc, var target, var circle, var spell, var archwizard)
{
	var party = UI_get_party_list();
	var npcmana = npc->get_npc_prop(MANA);
	var maxmana = npc->get_npc_prop(MAX_MANA);
	
	var spell_function = getSpellFunction(circle, spell);
	
	if ((npc in party) && !archwizard)
	{
		var needed_reagents;
		
		if (circle == 0)	//linear spells
		{
			needed_reagents = [0, 0, 0, 0,
							   0, 0, 0, 0];
		}
		else if (circle == 1)
		{
			if (spell == SPELL_AWAKEN_ALL)
				needed_reagents = [0, 0, 0, 0,
								   1, 1, 0, 0];
			else if (spell == SPELL_CREATE_FOOD)
				needed_reagents = [0, 0, 0, 1,
								   1, 1, 0, 0];
			else if (spell == SPELL_CURE)
				needed_reagents = [0, 0, 0, 0,
								   1, 1, 0, 0];
			else if (spell == SPELL_DETECT_TRAP)
				needed_reagents = [0, 0, 1, 0,
								   0, 0, 1, 0];
			else if (spell == SPELL_GREAT_DOUSE)
				needed_reagents = [0, 0, 0, 0,
								   1, 0, 1, 0];
			else if (spell == SPELL_GREAT_IGNITE)
				needed_reagents = [0, 0, 0, 0,
								   0, 0, 1, 1];
			else if (spell == SPELL_LIGHT)
				needed_reagents = [0, 0, 0, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_LOCATE)
				needed_reagents = [0, 0, 1, 0,
								   0, 0, 0, 0];
			else if (spell == SPELL_TRANSLATE)
				needed_reagents = [0, 0, 1, 0,
								   0, 0, 0, 0];
		}
		else if (circle == 2)
		{
			if (spell == SPELL_DESTROY_TRAP)
				needed_reagents = [0, 1, 0, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_ENCHANT)
				needed_reagents = [1, 0, 0, 1,
								   0, 0, 0, 0];
			else if (spell == SPELL_FIRE_BLAST)
				needed_reagents = [1, 0, 0, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_GREAT_LIGHT)
				needed_reagents = [0, 0, 0, 1,
								   0, 0, 0, 1];
			else if (spell == SPELL_MASS_CURE)
				needed_reagents = [0, 0, 0, 1,
								   1, 1, 0, 0];
			else if (spell == SPELL_PROTECTION)
				needed_reagents = [0, 0, 0, 0,
								   1, 1, 0, 1];
			else if (spell == SPELL_TELEKINESIS)
				needed_reagents = [1, 1, 0, 1,
								   0, 0, 0, 0];
			else if (spell == SPELL_WIZARD_EYE)
				needed_reagents = [1, 1, 1, 1,
								   0, 0, 1, 1];
		}
		else if (circle == 3)
		{
			if (spell == SPELL_CURSE)
				needed_reagents = [0, 0, 1, 0,
								   1, 0, 0, 1];
			else if (spell == SPELL_HEAL)
				needed_reagents = [0, 0, 0, 0,
								   1, 1, 1, 0];
			else if (spell == SPELL_PARALYZE)
				needed_reagents = [0, 0, 1, 0,
								   0, 0, 1, 0];
			else if (spell == SPELL_PEER)
				needed_reagents = [0, 0, 1, 1,
								   0, 0, 0, 0];
			else if (spell == SPELL_POISON)
				needed_reagents = [1, 1, 1, 0,
								   0, 0, 0, 0];
			else if (spell == SPELL_PROTECT_ALL)
				needed_reagents = [0, 0, 0, 1,
								   1, 1, 0, 1];
			else if (spell == SPELL_SLEEP)
				needed_reagents = [1, 0, 1, 0,
								   0, 0, 1, 0];
			else if (spell == SPELL_SWARM)
				needed_reagents = [0, 1, 1, 1,
								   0, 0, 0, 0];
			else if (spell == SPELL_REMOVE_CURSE)
				needed_reagents = [0, 0, 0, 0,
								   1, 1, 0, 1];
		}
		else if (circle == 4)
		{
			if (spell == SPELL_CONJURE)
				needed_reagents = [0, 0, 0, 1,
								   0, 0, 1, 0];
			else if (spell == SPELL_LIGHTNING)
				needed_reagents = [1, 0, 0, 1,
								   0, 0, 0, 1];
			else if (spell == SPELL_MASS_CURSE)
				needed_reagents = [0, 0, 1, 1,
								   1, 0, 0, 1];
			else if (spell == SPELL_REVEAL)
				needed_reagents = [0, 1, 0, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_SEANCE)
				needed_reagents = [0, 1, 1, 1,
								   0, 0, 1, 1];
			else if (spell == SPELL_UNLOCK_MAGIC)
				needed_reagents = [0, 1, 0, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_RECHARGE_MAGIC)
				needed_reagents = [0, 0, 0, 1,
								   0, 0, 1, 1];
			else if (spell == SPELL_BLINK)
				needed_reagents = [0, 1, 0, 1,
								   0, 0, 0, 0];
		}
		else if (circle == 5)
		{
			if (spell == SPELL_CHARM)
				needed_reagents = [1, 0, 1, 0,
								   0, 0, 1, 0];
			else if (spell == SPELL_DANCE)
				needed_reagents = [0, 1, 0, 1,
								   1, 0, 0, 0];
			else if (spell == SPELL_DISPEL_FIELD)
				needed_reagents = [1, 0, 0, 0,
								   1, 0, 1, 1];
			else if (spell == SPELL_EXPLOSION)
				needed_reagents = [1, 1, 0, 1,
								   0, 0, 0, 1];
			else if (spell == SPELL_FIRE_FIELD)
				needed_reagents = [1, 0, 0, 0,
								   0, 0, 1, 1];
			else if (spell == SPELL_GREAT_HEAL)
				needed_reagents = [0, 0, 0, 1,
								   1, 1, 1, 0];
			else if (spell == SPELL_INVISIBILITY)
				needed_reagents = [0, 1, 1, 0,
								   0, 0, 0, 0];
			else if (spell == SPELL_MASS_SLEEP)
				needed_reagents = [0, 0, 1, 0,
								   0, 1, 1, 0];
			else if (spell == SPELL_SUMMON_SKELETONS)
				needed_reagents = [0, 1, 0, 1,
								   1, 0, 0, 0];
		}
		else if (circle == 6)
		{
			if (spell == SPELL_CAUSE_FEAR)
				needed_reagents = [0, 0, 1, 1,
								   1, 0, 0, 0];
			else if (spell == SPELL_CLONE)
				needed_reagents = [0, 1, 1, 1,
								   0, 1, 1, 1];
			else if (spell == SPELL_FIRE_RING)
				needed_reagents = [1, 0, 0, 1,
								   0, 0, 1, 1];
			else if (spell == SPELL_FLAME_STRIKE)
				needed_reagents = [1, 1, 0, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_MAGIC_STORM)
				needed_reagents = [0, 1, 1, 1,
								   0, 0, 0, 1];
			else if (spell == SPELL_POISON_FIELD)
				needed_reagents = [1, 0, 1, 0,
								   0, 0, 1, 0];
			else if (spell == SPELL_SLEEP_FIELD)
				needed_reagents = [1, 0, 0, 0,
								   0, 1, 1, 0];
			else if (spell == SPELL_TREMOR)
				needed_reagents = [0, 1, 0, 1,
								   0, 0, 0, 1];
		}
		else if (circle == 7)
		{
			if (spell == SPELL_CREATE_GOLD)
				needed_reagents = [0, 0, 0, 1,
								   0, 0, 1, 0];
			else if (spell == SPELL_DEATH_BOLT)
				needed_reagents = [1, 0, 1, 0,
								   0, 0, 0, 1];
			else if (spell == SPELL_DELAYED_BLAST)
				needed_reagents = [1, 1, 0, 1,
								   0, 0, 1, 1];
			else if (spell == SPELL_ENERGY_FIELD)
				needed_reagents = [1, 0, 0, 1,
								   0, 0, 1, 1];
			else if (spell == SPELL_ENERGY_MIST)
				needed_reagents = [0, 1, 1, 1,
								   0, 0, 0, 1];
			else if (spell == SPELL_MASS_CHARM)
				needed_reagents = [1, 0, 1, 1,
								   0, 0, 1, 0];
			else if (spell == SPELL_MASS_MIGHT)
				needed_reagents = [1, 0, 0, 1,
								   0, 1, 0, 0];
			else if (spell == SPELL_RESTORATION)
				needed_reagents = [0, 0, 0, 1,
								   1, 1, 0, 1];
			else if (spell == SPELL_MASS_DISPEL_FIELD)
				needed_reagents = [1, 0, 0, 1,
								   1, 0, 1, 1];
		}
		else if (circle == 8)
		{
			if (spell == SPELL_DEATH_VORTEX)
				needed_reagents = [0, 1, 1, 1,
								   0, 0, 0, 1];
			else if (spell == SPELL_INVISIBILITY_ALL)
				needed_reagents = [1, 1, 1, 1,
								   0, 0, 0, 0];
			else if (spell == SPELL_MASS_DEATH)
				needed_reagents = [0, 1, 1, 1,
								   1, 1, 0, 0];
			else if (spell == SPELL_RESURRECT)
				needed_reagents = [0, 0, 0, 0,
								   1, 1, 1, 1];
			else if (spell == SPELL_SUMMON)
				needed_reagents = [0, 1, 0, 1,
								   1, 0, 0, 0];
			else if (spell == SPELL_SWORDSTRIKE)
				needed_reagents = [1, 0, 1, 1,
								   0, 0, 0, 0];
			else if (spell == SPELL_TIME_STOP)
				needed_reagents = [0, 1, 0, 1,
								   1, 0, 0, 0];
				//return [NO_SUCH_SPELL];
		}
		
		var reagent_names =			["black pearl",	"blood moss",	"nightshade",	"mandrake root",
									 "garlic",		"ginseng",		"spider silk",	"sulphurous ash"];
		var singular_prefixes =		["",			"portion of",	"button of",	"",
									 "clove of",	"portion of",	"portion of",	"portion of"];
		var plural_prefixes =		["",			"portions of",	"buttons of",	"",
									 "cloves of",	"portions of",	"portions of",	"portions of"];
		var plural_suffixes = 		["s",			"",				"",				"s",
									 "",			"",				"",				""];
		var reagent_frames = [1, 2, 3, 4, 5, 6, 7, 8];
		var reagent;
		var index;
		var max;
		var lacking_reagents = [0, 0, 0, 0, 0, 0, 0, 0];
		var reagent_count;
		var missing_count;
		var ret_str;
		
		for (reagent in reagent_frames with index to max)
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
			for (reagent in reagent_frames with index to max)
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
		
		for (reagent in reagent_frames with index to max)
			npc->remove_cont_items(needed_reagents[reagent], SHAPE_REAGENT, QUALITY_ANY, reagent - 1, true);
	}

	npc->begin_casting_mode();
	if (isTargetedSpell(circle, spell))
	{
		if (!target)
			target = UI_click_on_item();
		npc->(*spell_function)(target);
	}
	else
		npc->(*spell_function)();
	
	if (!(npc in party) || archwizard) circle = 0;
	return [CASTING_SUCCESSFUL, circle];
}

npcAskSpellToCast (var npc, var ignorenpclevel, var archwizard, var talk, var removespells)
{
	UI_push_answers();
	
	var circle;
	var spelllist;
	var spell;
	var casting_return;
	var casting_status;
	var circle_list;

	circle_list = getCircleList(npc, ignorenpclevel);
	
	say(talk[1]);
	while (true)
	{
		circle = chooseFromMenu2(circle_list) - 2;

		if (circle == -1)
			break;
		else
		{
			spelllist = removeSpellsFromList(getSpellList(circle), removespells);

			say(talk[2]);
			spell = chooseFromMenu2(spelllist);
			if (spell == 1)
				say(talk[3]);
			else
			{
				//Determine the true spell index:
				spell = getIndexForSpell(circle, spelllist[spell]);
				casting_return = npcCastSpell(npc, 0, circle, spell, archwizard);
				var casting_result = casting_return[1];
				if (casting_result == CASTING_SUCCESSFUL)
				{
					//casting_return[2] is the mana cost of the spell.
					//Only do this if the spell cost any mana at all.
					if (casting_return[2] != 0)
						npc->set_npc_prop(MANA, -1*casting_return[2]);

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

npcCastSpellDialog (var npc, var spell, var archwizard, var talk)
{
	var circle;
	var circlelist;

	while (circle < 8)
	{
		circlelist = getSpellList(circle);
		if (spell in circlelist)
			break;

		circle = circle + 1;
	}
	
	var casting_return = npcCastSpell(npc, 0, circle, getIndexForSpell(circle, spell), archwizard);
	var casting_result = casting_return[1];
	
	if (casting_result == CASTING_SUCCESSFUL)
	{
		//casting_return[2] is the mana cost of the spell.
		//Only do this if the spell cost any mana at all.
		if (casting_return[2] != 0)
			npc->set_npc_prop(MANA, -1*casting_return[2]);
		
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
		if (casting_return[2])
			say(talk[7]);
		else
			say(talk[8]);
		}
	else if (casting_result == NO_SUCH_SPELL)
		//This should never happen...
		say("@Alas, I am afraid that spell doesn't exist. Warn the programmer, something is amiss!@");
}

npcCastWeaponSpell (var npc, var target, var spellitem, var spell, var archwizard, var barks)
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
	
	var casting_return = npcCastSpell(npc, target, circle, getIndexForSpell(circle, spell), archwizard);
	var casting_result = casting_return[1];
	
	if (casting_result == CASTING_SUCCESSFUL)
	{
		//casting_return[2] is the mana cost of the spell.
		//Only do this if the spell cost any mana at all.
		if (casting_return[2] != 0)
			npc->set_npc_prop(MANA, -1*casting_return[2]);

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

npcCastHealing (var npc, var archwizard, var talk_cast, var talk, var healing_spells)
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
				archwizard,
				talk_cast);
		}	
		else
			break;
	}	
}
