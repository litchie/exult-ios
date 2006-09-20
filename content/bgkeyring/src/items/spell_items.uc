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
 *	This source file contains code for the NPC spellcasting items.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

const int MAX_PREPARED_SPELLS				= 6;

//The list of known spells
static var spells_unknown_jaana;
static var spells_unknown_mariah;
static var spells_unknown_laurianna;
static var spells_unknown_iolo;
static var spells_unknown_shamino;
static var spells_unknown_dupre;
static var spells_unknown_julia;
//The list of favorite spells
static var fav_spells_jaana;
static var fav_spells_mariah;
static var fav_spells_laurianna;
static var fav_spells_iolo;
static var fav_spells_shamino;
static var fav_spells_dupre;
static var fav_spells_julia;
static var fav_spells_british;

static var initialized;

var prepareSpell (var npc, var spell_array, var talk, var removespells)
{
	var spells = spell_array;
	var circle_list = getCircleList(npc);
	var spells_size;
	
	say(talk[1]);
	say(talk[2]);
	while (true)
	{
		var circle = chooseFromMenu2(circle_list) - 2;
		
		if (circle == -1)
			break;
		
		else
		{
			var spelllist = removeItemsFromList(getSpellList(circle), removespells);
			
			say(talk[3]);
			var spell = askForResponse(spelllist);
			if (spell == "none")
				say(talk[4]);
			else
			{
				//See if the spell is already prepared:
				if (spell in spells)
					//It is, so remove it before adding the spell again:
					spells = removeFromArray(spells, spell);
				
				spells = [spells, spell];
				
				spells_size = UI_get_array_size(spells);
				if (spells_size == MAX_PREPARED_SPELLS)
				{
					say(talk[5]);
					if (!askYesNo())
						break;
				}
				say(talk[6]);
			}
		}
	}
	
	say(talk[7]);
	
	var ret_array = [];
	spells_size = UI_get_array_size(spells);
	var array_size = UI_get_array_size(spell_array);
	
	if (spells_size >= 1)
	{
		if (spells_size > MAX_PREPARED_SPELLS)
		{
			var index = spells_size - MAX_PREPARED_SPELLS;
			while (index < spells_size)
			{
				index = index + 1;
				ret_array = [ret_array, spells[index]];
			}
		}
		else
			ret_array = spells;
	}
	
	else
		ret_array = spell_array;

	return ret_array;
}

var prepareCombatSpell (var npc, var stored_spell, var removespells, var talk)
{
	if (!stored_spell)
		message(talk[1]);
	else
	{
		message(talk[2]);
		message(stored_spell);
		message(".");
	}
	
	message(talk[3]);
	say();
	
	if (askYesNo())
	{
		say(talk[4]);
		var combat_spells = getLeveledSpellList(npc,
			false,
			["Fire blast", "Lightning", "Explosion", "Death bolt", "Swordstrike"],
			[2, 4, 5, 7, 8],
			removespells);
		
		var choice = askForResponse(["none", combat_spells]);
		if (choice != "none")
			return choice;
	}
	
	return stored_spell;
}

var getFavoriteNameList (var index_array)
{
	var ret_array = [];
	var circle;
	var spell_index;
	var spell_names;
	
	if (UI_get_array_size(index_array) == 1)
		index_array = index_array[1];
	
	var circle_nums = [getSpellFunction(0, 0),
					   getSpellFunction(1, 0),
					   getSpellFunction(2, 0),
					   getSpellFunction(3, 0),
					   getSpellFunction(4, 0),
					   getSpellFunction(5, 0),
					   getSpellFunction(6, 0),
					   getSpellFunction(7, 0),
					   getSpellFunction(8, 0)];
	circle = 9;
	while (circle > 0)
	{
		circle_nums[circle] = circle_nums[circle] - circle_nums[1];
		circle = circle - 1;
	}
	
	for (element in index_array)
	{
		circle = 9;
		while (circle > 0)
		{
			if (element >= circle_nums[circle])
			{
				spell_index = element - circle_nums[circle];
				circle = circle - 1;
				break;
			}
			circle = circle - 1;
		}
		
		spell_names = getSpellList(circle);
		ret_array = [ret_array, spell_names[spell_index + 2]];
	}
	
	if (UI_get_array_size(ret_array) == 1)
		return ret_array[1];
	else
		return ret_array;
}

var getFavoriteIndexList (var name_array)
{
	var ret_array = [];
	var circle;
	var spell_index;
	
	if (UI_get_array_size(name_array) == 1)
		name_array = name_array[1];
	
	for (element in name_array)
	{
		circle = 0;
		while (circle < 8)
		{
			if (element in getSpellList(circle))
				break;
			circle = circle + 1;
		}
		spell_index = getIndexForSpell(circle, element);
		ret_array = [ret_array, getSpellFunction(circle, 0) - getSpellFunction(0, 0) + spell_index];
	}
	
	if (UI_get_array_size(ret_array) == 1)
		return ret_array[1];
	else
		return ret_array;
}

var spellitemGetTalkMain (var npcnum)
{
	if (npcnum > 0)
		npcnum = -npcnum;
		
	if (npcnum == JAANA)
		return ["@That is my spellbook, Avatar.@",
				"@That is Jaana's spellbook, Avatar.@",
				"@This is Jaana's spellbook.@",
				"@How may I help, Avatar?@",
				"@If thou hast need of my services later, I will be here.@",
				"@Anything else I can do for thee, Avatar?@"];
	else if (npcnum == MARIAH)
		return ["@That is my spellbook, Avatar.@",
				"@That is Mariah's spellbook, Avatar.@",
				"@This is Mariah's spellbook.@",
				"@What dost thou wish me to do, Avatar?@",
				"@Oh. Never mind, then.@",
				"@What else dost thou wish me to do, Avatar?@"];
	else if (npcnum == LAURIANNA)
		return ["@That is my spellbook, Avatar.@",
				"@That is Laurianna's spellbook, Avatar.@",
				"@This is Laurianna's spellbook.@",
				"@What can I do for thee, Avatar?@",
				"@Oh. Never mind, then.@",
				"@Can I do anything else for thee, Avatar?@"];
	else if (npcnum == IOLO)
		return ["@That is my lute, Avatar.@",
				"@That is Iolo's lute, Avatar.@",
				"@This is Iolo's lute.@",
				"@What can I do for thee, old friend?@",
				"@Oh. Never mind, then.@",
				"@What else dost thou wish me to do, old friend?@"];
	else if (npcnum == SHAMINO)
		return ["@That is my ankh, Avatar.@",
				"@That is Shamino's ankh, Avatar.@",
				"@This is Shamino's ankh.@",
				"@Yes, " + getPoliteTitle() + "?@",
				"@Another time, then.@",
				"@What else should I do, " + getPoliteTitle() + "?@"];
	else if (npcnum == DUPRE)
		return ["@That is my amulet, Avatar.@",
				"@That is Dupre's amulet, Avatar.@",
				"@This is Dupre's amulet.@",
				"@How may I assist thee, Avatar?@",
				"@I shall speak with thee another time, then.@",
				"@Anything else, Avatar?@"];
	else if (npcnum == JULIA)
		return ["@That is my hammer, Avatar.@",
				"@That is Julia's hammer, Avatar.@",
				"@This is Julia's hammer.@",
				"@Anything I can help you with, Avatar?@",
				"@Goodbye, " + getAvatarName() + ".@",
				"@Anything else?@"];
}
var spellitemGetTalkCast (var npcnum)
{
	if (npcnum > 0)
		npcnum = -npcnum;
		
	if (npcnum == JAANA)
		return ["@Dost thou wish me to cast a spell of which circle?@",
				"@What spell wouldst thou like me to cast?@",
				"@Maybe thou dost wish for a different circle?@",
				"@Some other time, perhaps...@",
				"@Alas, I don't have enough reagents for that spell.",
				"@I am lacking ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else if (npcnum == MARIAH)
		return ["@Dost thou wish me to cast a spell of which circle?@",
				"@What spell wouldst thou like me to cast?@",
				"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
				"@Oh. Some other time, perhaps...@",
				"@Alas, I don't have enough reagents for that spell.",
				"@I am lacking ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else if (npcnum == LAURIANNA)
		return ["@Dost thou wish me to cast a spell of which circle?@",
				"@Which spell dost thou wish me to cast?@",
				"@Thou hast changed thy mind? Fine. Dost thou wish for a different circle perhaps?@",
				"@Oh. Some other time, perhaps...@",
				"@Alas, I don't have enough reagents for that spell.",
				"@I am lacking ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else if (npcnum == IOLO)
		return ["@Should I cast a spell of which circle?@",
				"@What spell should I cast?@",
				"@Maybe thou art thinking of a different circle?@",
				"@Perhaps a song, instead?@",
				"@Forgive me, but I don't have enough reagents.",
				"@I need ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else if (npcnum == SHAMINO)
		return ["@Should I cast a spell of which circle?@",
				"@What spell should I cast?@",
				"@Perhaps thou wishest a different circle?@",
				"@Very well.@",
				"@I don't have enough reagents for the spell.",
				"@I need ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else if (npcnum == DUPRE)
		return ["@Should I cast a spell of which circle?@",
				"@What spell should I cast?@",
				"@Perhaps a different circle?@",
				"@Ask me another time, then.@",
				"@I don't have enough reagents for the spell.",
				"@I need ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else if (npcnum == JULIA)
		return ["@Should I cast a spell of which circle?@",
				"@What spell dost thou wish?@",
				"@A different circle, maybe?@",
				"@Another time, then.@",
				"@I don't have the proper reagents for that spell.",
				"@I am in need of ",
				"@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				"@I must rest a while before I can cast this spell.@"];
	else
	{
		avatarSpeak("Invalid NPC ID! Stop cheating!");
		abort;
	}
}
var spellitemGetTalkPrepare (var npcnum)
{
	if (npcnum > 0)
		npcnum = -npcnum;
		
	if (npcnum == JAANA)
		return ["@I can prepare up to six spells for quick use.@",
				"@Dost thou wish me to prepare a spell of which circle?@",
				"@What spell wouldst thou like me to prepare?@",
				"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
				"@I have already prepared six spells; should I prepare more and ignore those I prepared earlier?@",
				"@It is done. Should I prepare any other spells?@",
				"@Fine. Is there anything else?@"];
	else if (npcnum == MARIAH)
		return ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
				"@Dost thou wish me to prepare a spell of which circle?@",
				"@What spell wouldst thou like me to prepare?@",
				"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
				"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
				"@It is done. Should I prepare any other spells?@",
				"@Fine. Is there anything else?@"];
	else if (npcnum == LAURIANNA)
		return ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
				"@Dost thou wish me to prepare a spell of which circle?@",
				"@What spell wouldst thou like me to prepare?@",
				"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
				"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
				"@It is done. Should I prepare any other spells?@",
				"@Fine. Is there anything else?@"];
	else if (npcnum == IOLO)
		return ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
				"@Should I prepare a spell of which circle?@",
				"@What spell should I prepare?@",
				"@Maybe thou art thinking of a different circle?@",
				"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
				"@Very well. Should I prepare any other spells?@",
				"@Perhaps a song, instead?@"];
	else if (npcnum == SHAMINO)
		return ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
				"@Should I prepare a spell of which circle?@",
				"@What spell should I prepare?@",
				"@Perhaps thou wishest a different circle?@",
				"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
				"@Very well. Should I prepare any other spells?@",
				"@Very well.@"];
	else if (npcnum == DUPRE)
		return ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
				"@Should I prepare a spell of which circle?@",
				"@What spell should I prepare?@",
				"@Perhaps thou wishest a different circle?@",
				"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
				"@Very well. Should I prepare any other spells?@",
				"@Very well.@"];
	else if (npcnum == JULIA)
		return ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
				"@Should I prepare a spell of which circle?@",
				"@What spell should I prepare?@",
				"@Perhaps thou wishest a different circle?@",
				"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
				"@Very well. Should I prepare any other spells?@",
				"@Fair enough. Anything else@"];
}
var spellitemGetTalkHeal (var npcnum)
{
	if (npcnum > 0)
		npcnum = -npcnum;
		
	if (npcnum == JAANA)
		return ["@In which service art thou interested?@",
				"@I am glad to oblige, " + getPoliteTitle() + "!@",
				"@Who dost thou wish to be "];
	else if (npcnum == MARIAH)
		return ["@What can I do for thee, avatar?@",
				"@Of course, Avatar!@",
				"@Who should be "];
	else if (npcnum == LAURIANNA)
		return ["@Which mode of healing dost thou wish?@",
				"@It is my pleasure, Avatar!@",
				"@Who dost thou wish to be "];
	else if (npcnum == IOLO)
		return ["@What dost thou need, my friend?@",
				"@It is always a pleasure to help, my friend!@",
				"@Who should be "];
	else if (npcnum == SHAMINO)
		return ["@Which spell should I cast?@",
				"@Glad to help, " + getPoliteTitle() + "@",
				"@Who should be "];
	else if (npcnum == DUPRE)
		return ["@What should I do, " + getPoliteTitle() + "?@",
				"@Gladly, Avatar.@",
				"@Who should be "];
	else if (npcnum == JULIA)
		return ["@How may I help?@",
				"@Glad to be of assistance, Avatar.@",
				"@Who should be "];
	else if (npcnum == LORD_BRITISH)
		return ["@Of which service dost thou have need?@",
				"@Glad to be of assistance, Avatar.@",
				"@Who dost thou wish to be "];
}

var spellitemGetSpellsUnknown (var npcnum)
{
	if (npcnum > 0)
		npcnum = -npcnum;
	
	if (!initialized)
	{
		initialized = true;
		spells_unknown_jaana		= ["Mass resurrect"];
		spells_unknown_mariah		= ["Mass resurrect"];
		spells_unknown_laurianna	= [];
		spells_unknown_iolo			= ["Mass resurrect"];
		spells_unknown_shamino		= ["Mass resurrect"];
		spells_unknown_dupre		= ["Mass resurrect"];
		spells_unknown_julia		= ["Mass resurrect"];

		fav_spells_jaana = [];    
		fav_spells_mariah = [];   
		fav_spells_laurianna = [];
		fav_spells_iolo = [];     
		fav_spells_shamino = [];  
		fav_spells_dupre = [];    
		fav_spells_julia = [];
		fav_spells_british = [];

	}

	var ret_spells = ["Mass resurrect"];
	
	if (npcnum == JAANA)
		ret_spells = spells_unknown_jaana;
	else if (npcnum == MARIAH)
		ret_spells = spells_unknown_mariah;
	else if (npcnum == LAURIANNA)
		ret_spells = spells_unknown_laurianna;
	else if (npcnum == IOLO)
		ret_spells = spells_unknown_iolo;
	else if (npcnum == SHAMINO)
		ret_spells = spells_unknown_shamino;
	else if (npcnum == DUPRE)
		ret_spells = spells_unknown_dupre;
	else if (npcnum == JULIA)
		ret_spells = spells_unknown_julia;
	else if (npcnum == LORD_BRITISH)
		ret_spells = [];	//LB knows all spells.

	return ret_spells;
}

spellitemSaveSpellsUnknown (var npcnum, var spells_unknown)
{
	if (npcnum > 0)
		npcnum = -npcnum;
	
	if (npcnum == JAANA)
		spells_unknown_jaana = spells_unknown;
	else if (npcnum == MARIAH)
		spells_unknown_mariah = spells_unknown;
	else if (npcnum == LAURIANNA)
		spells_unknown_laurianna = spells_unknown;
	else if (npcnum == IOLO)
		spells_unknown_iolo = spells_unknown;
	else if (npcnum == SHAMINO)
		spells_unknown_shamino = spells_unknown;
	else if (npcnum == DUPRE)
		spells_unknown_dupre = spells_unknown;
	else if (npcnum == JULIA)
		spells_unknown_julia = spells_unknown;
	else if (npcnum == LORD_BRITISH)
		return;		//LB knows all spells, no need to save anything.
}

var spellitemGetFavoriteSpells (var npcnum)
{
	if (npcnum > 0)
		npcnum = -npcnum;
	
	var ret_spells;
	
	if (npcnum == JAANA)
		ret_spells = fav_spells_jaana;
	else if (npcnum == MARIAH)
		ret_spells = fav_spells_mariah;
	else if (npcnum == LAURIANNA)
		ret_spells = fav_spells_laurianna;
	else if (npcnum == IOLO)
		ret_spells = fav_spells_iolo;
	else if (npcnum == SHAMINO)
		ret_spells = fav_spells_shamino;
	else if (npcnum == DUPRE)
		ret_spells = fav_spells_dupre;
	else if (npcnum == JULIA)
		ret_spells = fav_spells_julia;
	else if (npcnum == LORD_BRITISH)
		ret_spells = fav_spells_british;

	return ret_spells;
}

spellitemSaveFavoriteSpells (var npcnum, var fav_spells)
{
	if (npcnum > 0)
		npcnum = -npcnum;
	
	if (npcnum == JAANA)
		fav_spells_jaana = fav_spells;
	else if (npcnum == MARIAH)
		fav_spells_mariah = fav_spells;
	else if (npcnum == LAURIANNA)
		fav_spells_laurianna = fav_spells;
	else if (npcnum == IOLO)
		fav_spells_iolo = fav_spells;
	else if (npcnum == SHAMINO)
		fav_spells_shamino = fav_spells;
	else if (npcnum == DUPRE)
		fav_spells_dupre = fav_spells;
	else if (npcnum == JULIA)
		fav_spells_julia = fav_spells;
	else if (npcnum == LORD_BRITISH)
		fav_spells_british = fav_spells;
}

spellitem_Main ()
{
	var npcnum = -get_item_quality();
	var npc = npcnum->get_npc_object();
	
	var removespells = spellitemGetSpellsUnknown(npcnum);
	var fav_spells = spellitemGetFavoriteSpells(npcnum);
	
	
	var talk_main = spellitemGetTalkMain(npcnum);
	var talk_cast = spellitemGetTalkCast(npcnum);
	var talk_prepare = spellitemGetTalkPrepare(npcnum);
	var talk_heal = spellitemGetTalkHeal(npcnum);
	
	var item_shape = get_item_shape();
	var cont;
	var spellitem;
	
	var fav_spell_names = [];
	
	if (fav_spells)
		fav_spell_names = getFavoriteNameList(fav_spells);
	
	if (event == DOUBLECLICK)
	{
		cont = getOuterContainer(item);
		var party = UI_get_party_list();
		
		if (cont != npc)
		{
			if (npc in party)
				npc.say(talk_main[1]);
			else
			{
				npc = randomPartyMember();
				if (npc != AVATAR)
					npc.say(talk_main[2]);
				else
					avatarSpeak(talk_main[3]);
			}
		}
		
		else
		{
			var healing_spells = getLeveledSpellList(npc,
				npc->get_item_flag(HEALER),
				["Cure", "Mass cure", "Heal", "Great heal", "Restoration", "Resurrect", "Mass resurrect"],
				[1, 2, 3, 5, 7, 8, 8],
				removespells);
			var choice;
			
			if (gflags[BROKE_TETRAHEDRON] || !(npc->get_map_num() in [0, 1]))
			{
				npc.say(talk_main[4]);
				while (true)
				{
					var choicelist = ["nothing", "Cast spell", "Prepare spell", healing_spells];
					
					if (UI_get_array_size(fav_spell_names) >= 1)
						choicelist = [choicelist, fav_spell_names];
						
					choice = askForResponse(choicelist);
					if (choice == "nothing")
					{
						say(talk_main[5]);
						break;
					}
					
					else if (choice == "Cast spell")
						npcAskSpellToCast(npc,
							talk_cast,
							removespells
							);
					else if (choice == "Prepare spell")
					{
						fav_spell_names = prepareSpell(npc,
							fav_spell_names,
							talk_prepare,
							[removespells, healing_spells]
							);
						fav_spells = getFavoriteIndexList(fav_spell_names);
						spellitemSaveFavoriteSpells(npcnum, fav_spells);
					}
					
					else
						npcCastSpellDialog(npc,
							choice,
							talk_cast);
					npc.say(talk_main[6]);
				}
			}
			
			else
			{
				npc.say("@Due to the problems with the Ether, I dare not cast anything but healing spells for now.@");
				npcCastHealing(npc,
							   talk_cast,
							   talk_heal,
							   healing_spells);
			}
		}
	}
}

spellitem_Spellbook shape#(0x455) ()
{	var npcs = [JAANA, MARIAH, LAURIANNA];
	set_item_quality(-npcs[get_item_frame()+1]);
	item->spellitem_Main();	}

spellitem_Iolos_Lute shape#(0x456) ()
{	set_item_quality(-IOLO);
	item->spellitem_Main();	}

spellitem_Spell_Amulet shape#(0x457) ()
{	var npcs = [DUPRE, SHAMINO];
	set_item_quality(-npcs[get_item_frame()+1]);
	item->spellitem_Main();	}

spellitem_Julias_Hammer shape#(0x458) ()
{	set_item_quality(-JULIA);
	item->spellitem_Main();	}
