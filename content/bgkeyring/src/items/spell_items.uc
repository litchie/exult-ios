/*
 *	This source file contains code for the NPC spellcasting items.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-02-03
 */

const int MAX_PREPARED_SPELLS				= 6;

var prepareSpell (var npc, var ignore_npc_level, var spell_array, var talk, var removespells)
{
	var spells = spell_array;
	var circle_list = getCircleList(npc, ignore_npc_level);
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
			var spelllist = removeSpellsFromList(getSpellList(circle), removespells);

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

prepareCombatSpell (var npc, var stored_spell, var ignorelevel, var removespells, var talk)
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
			ignorelevel,
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
	var index;
	var max;
	var ret_array = [];
	var element;
	var circle;
	var spell_index;
	var spell_names;
	
	if (UI_get_array_size(index_array) == 1)
		index_array = index_array[1];
	
	for (element in index_array with index to max)
	{
		circle = element / 8;
		spell_index = element % 8;
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
	var index;
	var max;
	var ret_array = [];
	var element;
	var circle;
	var spell_index;
	
	if (UI_get_array_size(name_array) == 1)
		name_array = name_array[1];
	
	for (element in name_array with index to max)
	{
		circle = 0;
		while (circle < 8)
		{
			if (element in getSpellList(circle))
				break;
			circle = circle + 1;
		}
		spell_index = getIndexForSpell(circle, element);
		ret_array = [ret_array, circle * 8 + spell_index];
	}
	
	if (UI_get_array_size(ret_array) == 1)
		return ret_array[1];
	else
		return ret_array;
}

Jaanas_Spellbook shape#(0x455) ()
{
	var talk_main = ["@That is my spellbook, Avatar.@",
					 "@That is Jaana's spellbook, Avatar.@",
					 "@This is Jaana's spellbook.@",
					 "@What can I do for thee, Avatar?@",
					 "@If thou hast need of my services later, I will be here.@",
					 "@Dost thou wish me to prepare a spell for combat casting or for quick use?@",
					 "@Anything else I can do for thee, Avatar?@"];
	var talk_cast = ["@Dost thou wish me to cast a spell of which circle?@",
					 "@What spell wouldst thou like me to cast?@",
					 "@Maybe thou dost wish for a different circle?@",
					 "@Some other time, perhaps...@",
					 "@Alas, I don't have enough reagents for that spell.",
					 "@I am lacking ",
					 "@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
					 "@I must rest a while before I can cast this spell.@"];
	var talk_prepare = ["@I can prepare up to six spells for quick use.@",
						"@Dost thou wish me to prepare a spell of which circle?@",
						"@What spell wouldst thou like me to prepare?@",
						"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
						"@I have already prepared six spells; should I prepare more and ignore those I prepared earlier?@",
						"@It is done. Should I prepare any other spells?@",
						"@Fine. Is there anything else?@"];
	var talk_combat = ["@I don't have a combat spell ready yet.",
					   "@The current readied spell is ",
					   " Dost thou wish to change it?@",
					   "@To which spell?@"];
	var talk_heal = ["@In which service art thou interested?@",
					 "@I am glad to oblige, " + getPoliteTitle() + "!@",
					 "@Who should be "];
	var barks = ["@It's Jaana's@", "@No spell@"];
	
	var npc = JAANA->get_npc_object();
	var item_shape = SHAPE_JAANAS_SPELLBOOK;
	var cont;
	var ignorelevel = false;
	var archwizard = false;
	var spellitem;
	
	var removespells = [];
	var spell_names = [];
	var stored_spell = "";
	
	static var initialized;
	static var weapon_spell;
	static var fav_spells;

	if (!initialized)
	{
		initialized = true;
		weapon_spell = -1;
		fav_spells = [];
	}
	
	if (weapon_spell != -1)
		stored_spell = getFavoriteNameList(weapon_spell);
	if (fav_spells)
		spell_names = getFavoriteNameList(fav_spells);

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
				true,	// Let Jaana be a decent healer
				["Cure", "Mass cure", "Heal", "Great heal", "Restoration", "Resurrect"],
				[1, 2, 3, 5, 7, 8],
				removespells);
			var choice;
			
			if (gflags[BROKE_TETRAHEDRON])
			{
				npc.say(talk_main[4]);
				while (true)
				{
					var choicelist = ["nothing", "Cast spell", "Prepare spell", healing_spells];

					if (UI_get_array_size(spell_names) >= 1)
						choicelist = [choicelist, spell_names];
						
					if (stored_spell != "")
						choicelist = [choicelist, stored_spell];
						
					choice = askForResponse(choicelist);
					if (choice == "nothing")
					{
						say(talk_main[5]);
						break;
					}
					
					else if (choice == "Cast spell")
						npcAskSpellToCast(npc,
							ignorelevel,
							archwizard,
							talk_cast,
							removespells
							);
					else if (choice == "Prepare spell")
					{
						say(talk_main[6]);
						choice = chooseFromMenu2(["Combat casting", "Quick use"]);
						if (choice == 2)
							spell_names = prepareSpell(npc,
								ignorelevel,
								spell_names,
								talk_prepare,
								[removespells, healing_spells]
								);
						else
							stored_spell = prepareCombatSpell(npc,
															  stored_spell,
															  ignorelevel,
															  removespells,
															  talk_combat);
						if (stored_spell != "")
							weapon_spell = getFavoriteIndexList(stored_spell);
						fav_spells = getFavoriteIndexList(spell_names);
					}
					
					else
					{
						npcCastSpellDialog(npc,
							choice,
							archwizard,
							talk_cast);
					}
					npc.say(talk_main[7]);
				}
			}
			
			else
			{
				npcCastHealing(npc,
							   archwizard,
							   talk_cast,
							   talk_heal,
							   healing_spells);
			}
		}
	}
	
	else if (event == WEAPON)
	{
		spellitem = npc->get_cont_items(item_shape, QUALITY_ANY, FRAME_ANY);
		npcCastWeaponSpell(npc, item, spellitem, stored_spell, archwizard, barks);
	}
}

Mariahs_Spellbook shape#(0x457) ()
{
	var talk_main = ["@That is my spellbook, Avatar.@",
					 "@That is Mariah's spellbook, Avatar.@",
					 "@This is Mariah's spellbook.@",
					 "@What dost thou wish me to do, Avatar?@",
					 "@Oh. Never mind, then.@",
					 "@Dost thou wish me to prepare a spell for combat casting or for quick use?@",
					 "@What else dost thou wish me to do, Avatar?@"];
	var talk_cast = ["@Dost thou wish me to cast a spell of which circle?@",
					 "@What spell wouldst thou like me to cast?@",
					 "@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
					 "@Oh. Some other time, perhaps...@",
					 "@Alas, I don't have enough reagents for that spell.",
					 "@I am lacking ",
					 "@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
					 "@I must rest a while before I can cast this spell.@"];
	var talk_prepare = ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
						"@Dost thou wish me to prepare a spell of which circle?@",
						"@What spell wouldst thou like me to prepare?@",
						"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
						"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
						"@It is done. Should I prepare any other spells?@",
						"@Fine. Is there anything else?@"];
	var talk_combat = ["@I don't have a combat spell ready yet.",
					   "@The current readied spell is ",
					   " Dost thou wish to change it?@",
					   "@To which spell?@"];
	var barks = ["@It's Mariah's@", "@No spell@"];
	
	var npc = MARIAH->get_npc_object();
	var item_shape = SHAPE_MARIAHS_SPELLBOOK;
	var cont;
	var ignorelevel = false;
	var archwizard = false;
	var spellitem;
	
	var removespells = [];
	var spell_names = [];
	var stored_spell = "";
	
	static var initialized;
	static var weapon_spell;
	static var fav_spells;

	if (!initialized)
	{
		initialized = true;
		weapon_spell = -1;
		fav_spells = [];
	}
	
	if (weapon_spell != -1)
		stored_spell = getFavoriteNameList(weapon_spell);
	if (fav_spells)
		spell_names = getFavoriteNameList(fav_spells);

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
				ignorelevel,
				["Cure", "Mass cure", "Heal", "Great heal", "Restoration", "Resurrect"],
				[1, 2, 3, 5, 7, 8],
				removespells);
			var choice;
			
			npc.say(talk_main[4]);
			while (true)
			{
				var choicelist = ["nothing", "Cast spell", "Prepare spell", healing_spells];
				
				if (UI_get_array_size(spell_names) >= 1)
					choicelist = [choicelist, spell_names];
					
				if (stored_spell != "")
					choicelist = [choicelist, stored_spell];
					
				choice = askForResponse(choicelist);
				if (choice == "nothing")
				{
					say(talk_main[5]);
					break;
				}
				
				else if (choice == "Cast spell")
					npcAskSpellToCast(npc,
						ignorelevel,
						archwizard,
						talk_cast,
						removespells
						);
				else if (choice == "Prepare spell")
				{
					say(talk_main[6]);
					choice = chooseFromMenu2(["Combat casting", "Quick use"]);
					if (choice == 2)
						spell_names = prepareSpell(npc,
							ignorelevel,
							spell_names,
							talk_prepare,
							[removespells, healing_spells]
							);
					else
						stored_spell = prepareCombatSpell(npc,
														  stored_spell,
														  ignorelevel,
														  removespells,
														  talk_combat);
					if (stored_spell != "")
						weapon_spell = getFavoriteIndexList(stored_spell);
					fav_spells = getFavoriteIndexList(spell_names);
				}
				
				else
				{
					npcCastSpellDialog(npc,
						choice,
						archwizard,
						talk_cast);
				}
				npc.say(talk_main[7]);
			}
		}
	}
	
	else if (event == WEAPON)
	{
		spellitem = npc->get_cont_items(item_shape, QUALITY_ANY, FRAME_ANY);
		npcCastWeaponSpell(npc, item, spellitem, stored_spell, archwizard, barks);
	}
}

Lauriannas_Spellbook shape#(0x458) ()
{
	var talk_main = ["@That is my spellbook, Avatar.@",
					 "@That is Laurianna's spellbook, Avatar.@",
					 "@This is Laurianna's spellbook.@",
					 "@What can I do for thee, Avatar?@",
					 "@Oh. Never mind, then.@",
					 "@Should I prepare a spell for combat casting or for quick use?@",
					 "@Can I do anything else for thee, Avatar?@"];
	var talk_cast = ["@Dost thou wish me to cast a spell of which circle?@",
					 "@Which spell dost thou wish me to cast?@",
					 "@Thou hast changed thy mind? Fine. Dost thou wish for a different circle perhaps?@",
					 "@Oh. Some other time, perhaps...@",
					 "@Alas, I don't have enough reagents for that spell.",
					 "@I am lacking ",
					 "@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
					 "@I must rest a while before I can cast this spell.@"];
	var talk_prepare = ["@I can prepare up to " + MAX_PREPARED_SPELLS + " spells for quick use.@",
						"@Dost thou wish me to prepare a spell of which circle?@",
						"@What spell wouldst thou like me to prepare?@",
						"@Thou hast changed thy mind? Maybe thou dost wish for a different circle?@",
						"@I have already prepared " + MAX_PREPARED_SPELLS + " spells; should I prepare more and ignore those I prepared earlier?@",
						"@It is done. Should I prepare any other spells?@",
						"@Fine. Is there anything else?@"];
	var talk_combat = ["@I don't have a combat spell ready yet.",
					   "@The current readied spell is ",
					   " Dost thou wish to change it?@",
					   "@To which spell?@"];
	var talk_heal = ["@Which spell dost thou wish me to cast?@",
					 "@It is my pleasure, Avatar!@",
					 "@Who dost thou wish to be "];
	var barks = ["@It's Laurianna's@", "@No spell@"];
	
	var npc = LAURIANNA->get_npc_object();
	var item_shape = SHAPE_LAURIANNAS_SPELLBOOK;
	var cont;
	var ignorelevel = !gflags[LAURIANNA_IN_YEW];
	var archwizard = !gflags[LAURIANNA_IN_YEW];
	var spellitem;
	
	var removespells = [];
	var spell_names = [];
	var stored_spell = "";
	
	static var initialized;
	static var weapon_spell;
	static var fav_spells;

	if (!initialized)
	{
		initialized = true;
		weapon_spell = -1;
		fav_spells = [];
	}
	
	if (weapon_spell != -1)
		stored_spell = getFavoriteNameList(weapon_spell);
	if (fav_spells)
		spell_names = getFavoriteNameList(fav_spells);

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
				true,	// Let Laurianna be a decent healer
				["Cure", "Mass cure", "Heal", "Great heal", "Restoration", "Resurrect"],
				[1, 2, 3, 5, 7, 8],
				removespells);
			var choice;
			
			if (gflags[BROKE_TETRAHEDRON] || !gflags[LAURIANNA_IN_YEW])
			{
				npc.say(talk_main[4]);
				while (true)
				{
					var choicelist = ["nothing", "Cast spell", "Prepare spell", healing_spells];

					if (UI_get_array_size(spell_names) >= 1)
						choicelist = [choicelist, spell_names];
						
					if (stored_spell != "")
						choicelist = [choicelist, stored_spell];
						
					choice = askForResponse(choicelist);
					if (choice == "nothing")
					{
						say(talk_main[5]);
						break;
					}
					
					else if (choice == "Cast spell")
						npcAskSpellToCast(npc,
							ignorelevel,
							archwizard,
							talk_cast,
							removespells
							);
					else if (choice == "Prepare spell")
					{
						say(talk_main[6]);
						choice = chooseFromMenu2(["Combat casting", "Quick use"]);
						if (choice == 2)
							spell_names = prepareSpell(npc,
								ignorelevel,
								spell_names,
								talk_prepare,
								[removespells, healing_spells]
								);
						else
							stored_spell = prepareCombatSpell(npc,
															  stored_spell,
															  ignorelevel,
															  removespells,
															  talk_combat);
						if (stored_spell != "")
							weapon_spell = getFavoriteIndexList(stored_spell);
						fav_spells = getFavoriteIndexList(spell_names);
					}
					
					else
					{
						npcCastSpellDialog(npc,
							choice,
							archwizard,
							talk_cast);
					}
					npc.say(talk_main[7]);
				}
			}
			
			else
			{
				npcCastHealing(npc,
							   archwizard,
							   talk_cast,
							   talk_heal,
							   healing_spells);
			}
		}
	}
	
	else if (event == WEAPON)
	{
		spellitem = npc->get_cont_items(item_shape, QUALITY_ANY, FRAME_ANY);
		npcCastWeaponSpell(npc, item, spellitem, stored_spell, archwizard, barks);
	}
}