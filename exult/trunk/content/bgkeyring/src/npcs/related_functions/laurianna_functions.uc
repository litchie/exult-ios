/*
 *	This source file contains usecode for the Keyring Quest.
 *	Specifically, it is has functions used by Laurianna.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

switchFace (var facenum) {LAURIANNA.hide();	LAURIANNA->show_npc_face(facenum);}

var lauriannaAskJoin ()
{
	var party = UI_get_party_list();
	var party_size = UI_get_array_size(party);
	
	if (party_size < 8)
	{
		say("@Wilt thou bring me to my father? Wilt thou let me join thee?@");
		if (askYesNo())
		{
			add_to_party();
			add("leave");
			return true;
		}
		else
			say("@I shall wait here until thou dost change thy mind, then.@");
	}
	else
		say("@Alas, thy party has no room for me... I shall await thee here shouldst thou make room for me.@");
		
	return false;
}

lauriannaAskJournal ()
{
	var journal;
	
	if (PARTY->count_objects(SHAPE_JOURNAL, QUALITY_ANY, FRAME_ANY))
	{
		say("@Thou hast the journal in thy possession; wilt thou give it to me?@");
		if (askYesNo())
			UI_remove_party_items(1, SHAPE_JOURNAL, QUALITY_ANY, FRAME_ANY);
		else
		{
			say("@Shouldst thou change thy mind, talk to me again.@");
			add("Give journal");
			return;
		}
	}
	else
	{
		say("@There is the journal; I shall take it with me.@");
		journal = find_nearest(SHAPE_JOURNAL, 50);
		journal->remove_item();
	}
	say("@There, I have it now.@");
	gflags[LAURIANNA_HAS_JOURNAL] = true;
}

lauriannaGiveKeyring ()
{
	if (gflags[LAURIANNA_HAS_JOURNAL])
	{
		say("@Thank thee for all thy help, " + getPoliteTitle() + ". As a token of my appreciation, I shall give thee this keyring.");
		say("@I had crafted it for father, since he always lost his keys -- but alas, I never had the opportunity to give it to him.");
		say("@It is magical -- any keys added to it will disappear. Whenever thou hast need of a key, it will magically reappear, ready for use.");
		say("@I hope that thou dost find it useful...@");
		
		giveExperience(300);
		
		var keyring = UI_create_new_object(SHAPE_KEYRING);
		if (!AVATAR->give_last_created())
		{
			say("@Since thou art so overburdened, I shall place it on the ground.@");
			UI_update_last_created(AVATAR->get_object_position());
		}
	}
} 

lauriannaSellReagents ()
{
	sellItems(
				//Names
				["nothing",
				 "Black pearl", "Blood moss", "Nightshade",
				 "Mandrake root", "Garlic", "Ginseng",
				 "Spider silk", "Sulphurous ash"],
				//Shapes
				[SHAPE_REAGENT,
				 SHAPE_REAGENT, SHAPE_REAGENT, SHAPE_REAGENT,
				 SHAPE_REAGENT, SHAPE_REAGENT, SHAPE_REAGENT,
				 SHAPE_REAGENT, SHAPE_REAGENT],
				//Frames
				[0,
				 0, 1, 2,
				 3, 4, 5,
				 6, 7],
				//Price per unit
				[0,
				 5, 2, 3,
				 5, 1, 1,
				 3, 3],
				 //Quantities
				 [0,
				  1, 1, 1,
				  1, 1, 1,
				  1, 1],
				  //Articles
				 ["",
				  "", "", "",
				  "", "", "",
				  "", ""],
				  //Quantity strings
				 ["",
				  " each", " for one portion", " for one button",
				  " each", " for one clove", " for one portion",
				  " for one portion", " for one portion"],
				  //Quantity tokens
				 ["",
				  "", "", "",
				  "", "", "",
				  "", ""],
				 //Dialog strings
				 [" Dost thou still want it?@",
				  "@How many ", " dost thou want?@",
				  "@Here thou art. Dost thou wish anything else?@",
				  "@Alas, thou canst not carry that much. Wouldst thou be interested in anything else?@",
				  "@Thou dost not have enough gold to buy all that... May I interest thee in anything else?@",
				  "@If thou art sure of that... Dost thou want anything else?@",
				  "@I can understand thy decision. Wilt thou buy anything else?@",
				  "@Let me now if thou dost change thy mind, Avatar. What were we talking about?@"]);
}

lauriannaMassResurrect ()
{
	if (event == DOUBLECLICK)
	{
		item_say("@Vas Mani Corp Hur@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 64;
				actor frame KNEEL;			actor frame STAND;
				actor frame CAST_1;			call lauriannaMassResurrect;}
			UI_play_music(15, 0);
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
		var bodyshapes = [SHAPE_BODIES_1, SHAPE_BODIES_2, SHAPE_LARGE_BODIES];
		var shnum;
		var index;
		var max;
		var bodies = [];
		for (shnum in bodyshapes with index to max)
			bodies = [bodies, find_nearby(shnum, 25, MASK_NONE)];
		var body;
		var xoff = [0, 1, 2, 1, 0, -1, -2, -1];
		var yoff = [2, 1, 0, -1, -2, -1, 0, 1];
		for (body in bodies with index to max)
		{
			var qual = body->get_item_quality();
			var quant = body->get_item_quantity(1);
			if ((qual != 0) || (quant != 0))
			{
				var pos = get_object_position();
				UI_sprite_effect(ANIMATION_LIGHTNING, pos[X], pos[Y], 0, 0, 0, -1);
				UI_sprite_effect(ANIMATION_GREEN_BUBBLES, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
				var dist = get_distance(body);
				script body after 2+dist/3 ticks call lauriannaMassResurrect, EGG;
			}
		}
	}
	else
		resurrect();
}	

lauriannaHeal ()
{
	/*	DISABLED
	UI_push_answers();
	
	var healing_spells = getLeveledSpellList(item,
		false,
		["Cure", "Mass cure", "Heal", "Great heal", "Restoration", "Resurrect"],
		[1, 2, 3, 5, 7, 8],
		[]);

	while (true)
	{
		say("@Which spell dost thou wish me to cast?@");
		var choice = askForResponse(["none", healing_spells]);
		if (choice != "none")
		{
			if ((choice == "Restoration") || (choice == "Mass cure"))
				say("@It is my pleasure, Avatar!@");

			else
			{
				message("@Who dost thou wish to be ");
				if ((choice == "Heal") || (choice == "Great heal")) message("healed");
				else if (choice == "Cure") message("cured of poison");
				else message("resurrected");
				message("?@");
				say();
			}
			
			npcCastSpellDialog(item,
				choice,
				false,
				["@Dost thou wish me to cast a spell of which circle?@",
				 "@Which spell dost thou wish me to cast?@",
				 "@Thou hast changed thy mind? Fine. Dost thou wish for a different circle perhaps?@",
				 "@Oh. Some other time, perhaps...@",
				 "@Alas, I don't have enough reagents for that spell.",
				 "@I am lacking ",
				 "@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
				 "@I must rest a while before I can cast this spell.@"]);
		}	
		else
			break;
	}	
	say("@I am glad that thou art not in need of healing!@");
	UI_pop_answers();
	*/
	while (true)
	{
		say("@What service wilt thou want?@");
		var choice = askForResponse(["none", "healing", "resurrection"]);
		if (choice == "none")
			break;
		else if (choice == "healing")
		{
			halt_scheduled();
			item_say("@Vas Mani Hur@");
			if (inMagicStorm())
			{
				script item
				{	nohalt;						actor frame SWING_1;
					actor frame CAST_2;			actor frame SWING_2H_3;
					sfx 64;}
				var targets = UI_get_party_list();
				var index;
				var max;
				var npc;
				for (npc in targets with index to max)
				{
					npc->clear_item_flag(ASLEEP);
					npc->clear_item_flag(CHARMED);
					npc->clear_item_flag(CURSED);
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
				{	nohalt;						actor frame SWING_1;
					actor frame CAST_2;			actor frame SWING_2H_3;
					call spellFails;}
			}
			abort;
		}
		else if (choice == "resurrection")
		{
			event = DOUBLECLICK;
			LAURIANNA->lauriannaMassResurrect();
			abort;
		}
	}
	say("@I am glad that thou art not in need of healing!@");
}
