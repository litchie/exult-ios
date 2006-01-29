/*
 *	This source file contains usecode for the Shrines of the Virtues.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

const int SHRINE_FACES					= 0xFED8;	//-296;
//Each shrine with its own 'face':
enum shrines
{
	SHRINE_SACRIFICE					= 1,
	SHRINE_JUSTICE						= 2,
	SHRINE_HUMILITY						= 3,
	SHRINE_SPIRITUALITY					= 4,
	SHRINE_VALOR						= 5,
	SHRINE_COMPASSION					= 6,
	SHRINE_HONOR						= 7,
	SHRINE_HONESTY						= 8,
	SHRINE_SACRIFICE_DEFILED			= 9
};

enum shrine_codex_quest_levels
{
	CODEX_NOT_STARTED					= 0,
	GOT_FROM_SHRINE						= 1,
	WENT_TO_CODEX						= 2,
	FINISHED_QUEST						= 3
};
//Easter egg:
const int BOOK_OF_FORGOTTEN_MANTRAS		= 29;

Shrine shape#(0x463) ()
{
	var dir;
	var pathegg;
	var pos;
	var shrine_frame;
	var shrines = ["Sacrifice", "Justice", "Humility", "Spirituality",
				   "Valor", "Compassion", "Honor", "Honesty"];
	var mantras = ["Cah", "Beh", "Lum", "Om",
				   "Ra", "Mu", "Summ", "Ahm"];
	var forgotten_mantras = ["Akk" , "Hor", "Kra", "Maow", "Detra", "Sa"   , "Nok", "Spank", "A"   , "Mi"   , "Ah"  ,
							 "Xiop", "Yof", "Ow" , "Ta"  , "Goo"  , "Si"   , "Yam", "Vil"  , "Wez" , "Forat", "Asg" ,
							 "Sem" , "Tex", "As" , "Hiy" , "Eyac" , "Hodis", "Ni" , "Baw"  , "Fes" , "Upa"  , "Yuit",
							 "Swer", "Xes", "Led", "Zep" , "Bok"  , "Mar"  , "Sak", "Ces"  , "Blah", "Swu"];
	var words_of_power = ["Avidus", "Malum", "Ignavus", "Veramocor",
						  "Inopia", "Vilis", "Infama", "Fallax"];
	var chosen_mantra;
	var cycles;
	
	if (event == DOUBLECLICK)
	{
		//Determine shrine's face and position:
		shrine_frame = get_item_frame() + 1;
		var pos = get_object_position();
		var runes;
		//Determine if the party has the correct rune:
		if (shrine_frame == SHRINE_SACRIFICE_DEFILED)
		 	runes = PARTY->count_objects(SHAPE_RUNE, QUALITY_ANY, 0);
		else
		 	runes = PARTY->count_objects(SHAPE_RUNE, QUALITY_ANY, shrine_frame - 1);
		
		if (!runes)
		{
			//Nope, doesn't have it:
			randomPartySay("@Thou dost need the appropriate rune...@");
			return;
		}
		else
		{
			//Player has the correct rune; close gumps and start
			//playing 'Stones':
			UI_close_gumps();
			UI_play_music(22, 0);
			
			if (shrine_frame == SHRINE_SACRIFICE_DEFILED)
			{
				//The shrine of sacrifice starts defiled, and covered in blood
				AVATAR.say("You stand before the Shrine of Sacrifice. The Shrine is broken and defiled, with blood stains all over the altar.~Whatever peace there once was here is now long gone.");
				say("Do you wish to say anything?");
				//Not saying anything leaves it defiled
				if (askYesNo())
				{
					//Avatar will try to fix the shrine:
					say("What do you want to say?");
					var choice = chooseFromMenu2(["nothing", "mantra", "Word of Power"]);
					if (choice == 1)
						//Or maybe not...
						say("You walk away from the shrine, leaving it in its present state.");
					else if (choice == 2)
					{
						//Poor, poor Avatar...
						say("Which mantra do you speak?");
						var choices = [mantras];
						if (PARTY->count_objects(SHAPE_BOOK, BOOK_OF_FORGOTTEN_MANTRAS, FRAME_ANY))
							//Easter egg: the Book of Forgotten Mantras adds more options
							choices = [choices, forgotten_mantras]; 
						
						choice = askForResponse(choices);
						say("In an ominous tone, you speak the mantra: @" + choice + "!@~After waiting for a while, you realize nothing has happened.");
						if (isNearby(DUPRE) && choice == "Ni")
							DUPRE.say("@We are the knights who say... Ni!@");
						
					}
					else
					{
						//This is a wise Avatar!
						say("Which Word of Power do you speak?");
						choice = askForResponse(words_of_power);
						say("In an ominous tone, you speak the Word of Power: @" + choice + "!@ The ground trembles for a while afterwards.");
						UI_earthquake(12);
						
						if (choice == "Avidus")
						{
							//The right choice, of course.
							giveExperience(50);
							AVATAR.hide();
							//Cleanse the shrine:
							obj_sprite_effect(ANIMATION_TELEPORT, 0, 0, 0, 0, 0, -1);
							set_item_frame(SHRINE_SACRIFICE - 1);
							play_sound_effect2(64, item);
							
							var bloodstains = pos->find_nearby(SHAPE_BLOOD, 20, MASK_TRANLUCENT);
							var blood;
							var index;
							var max;
							for (blood in bloodstains with index to max)
								script blood after (3*get_distance(blood))/2 ticks remove;
						}
						else
							say("After a while, you realize that nothing else happened.");
					}
				}
				//LEAVE
				abort;
			}
			//Normal shrines:
			SHRINE_FACES->show_npc_face(shrine_frame - 1);
			say("You stand before the Shrine of " + shrines[shrine_frame] + ". The Shrine is a quiet and peaceful place, amidst the turmoil that is Britannia nowadays.");
			say("A mystical voice sounds inside your head as you approach the altar. @Welcome, seeker. Dost thou wish to meditate at this altar?@");
			if (!askYesNo())
				//Avatar doesn't wish to meditate, so leave:
				abort;
			
			//Ask for mantra...
			say("@Upon which mantra wilt thou meditate, seeker?@");
			chosen_mantra = chooseFromMenu2(mantras);
			//... and for number of cycles:
			say("@For how many cycles wilt thou meditate, seeker?@");
			cycles = chooseFromMenu2(["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]) - 1;
			if (!cycles)
			{
				//0 cycles...
				SHRINE_FACES.hide();
				AVATAR.say("@I think I will meditate at another time.@");
				//Avatar doesn't wish to meditate, so leave:
				abort;
			}
			
			//Determine the place where the Avatar will meditate:
			dir = direction_from(AVATAR);
			pos[Z] = pos[Z] - 4;
			if (dir in [NORTHWEST, NORTH, NORTHEAST])
				pos[Y] = pos[Y] - 2;
			else if (dir in [SOUTHWEST, SOUTH, SOUTHEAST])
				pos[Y] = pos[Y] + 2;
			else if (dir == EAST)
				pos[X] = pos[X] + 4;
			else
				pos[X] = pos[X] - 4;
			
			//Make him go there:
			if (AVATAR->si_path_run_usecode(pos, PATH_SUCCESS, AVATAR, Shrine, true));
			{
				UI_set_path_failure(Shrine, AVATAR, PATH_FAILURE);
				//Create a path egg containing the info about mantra
				//in the frame and cycles in the quality:
				pathegg = UI_create_new_object(SHAPE_PATH_EGG);
				pathegg->set_item_frame(chosen_mantra);
				pathegg->set_item_flag(TEMPORARY);
				pathegg->set_item_quality(cycles);
				pos = get_object_position();
				UI_update_last_created(pos);

				//NPC banthers:
				var noun = "he";
				if (UI_is_pc_female()) noun = "she";
				
				if (inParty(DUPRE) && (UI_get_random(3) == 1))
					script DUPRE after 5 ticks say "@Is there a pub nearby?@";
				if (inParty(SHAMINO) && (UI_get_random(3) == 1))
					script SHAMINO after 8 ticks say "@There " + noun + " goes again...@";
				if (inParty(IOLO) && (UI_get_random(3) == 1))
					script IOLO after 11 ticks say "@Where is my lute?@";

				abort;
			}
		}
	}

	else if (event == PATH_SUCCESS)
	{
		//Avatar reached destination
		//Find the path egg:
		pathegg = find_nearest(SHAPE_PATH_EGG, 5);
		if (!pathegg)
			//Not found means nothing to do; shouldn't happen...
			abort;
		
		dir = direction_from(pathegg);
		//Retrieve mantra and # of cycles:
		chosen_mantra = pathegg->get_item_frame();
		var mantra = "@" + mantras[chosen_mantra] + "@";
		cycles = pathegg->get_item_quality();
		
		//Meditation with a frozen Avatar:
		script item
		{	nohalt;						finish;
			call trueFreeze;			face dir;
			actor frame STAND;			wait 2;
			actor frame LEAN;			wait 2;
			repeat cycles - 1
			{	actor frame KNEEL;			wait 3;
				say mantra;					wait 3;
				actor frame KNEEL;			wait 3;
				actor frame KNEEL;			wait 3;
				//actor frame KNEEL;			wait 3;
			};
			actor frame LEAN;			wait 2;
			actor frame STAND;			wait 2;
			call Shrine, SCRIPTED;
			call trueUnfreeze;
		}
	}

	else if (event == PATH_FAILURE)
		//Avatar failed to get there
		item_say("@I can't get there@");

	else if (event == SCRIPTED)
	{
		//Once again, find path egg:
		pathegg = find_nearest(SHAPE_PATH_EGG, 5);
		if (!pathegg)
			//Once again, should never happen...
			abort;
		
		//Retrieve mantra and # of cycles
		chosen_mantra = pathegg->get_item_frame();
		cycles = pathegg->get_item_quality();
		//destroy path egg (no longer needed):
		pathegg->remove_item();
		//Find shrine and determine which shrine it is:
		var shrine = find_nearest(SHAPE_SHRINE, 5);
		shrine_frame = shrine->get_item_frame() + 1;
		
		if (chosen_mantra != shrine_frame)
		{
			//Wrong mantra for shrine
			AVATAR.say("You had difficulty focusing your thoughts, and could not meditate.");
			abort;
		}
		else if (cycles < 3)
		{
			//Too few cycles
			AVATAR.say("After a while, you feel a sense of calm and inner peace.");
			abort;
		}
		else if (cycles > 3)
		{
			//Too many cycles
			AVATAR.say("You meditated for too long and eventually lost your focus.");
			abort;
		}
		
		var codex_quest_level = CODEX_NOT_STARTED;
		//Get the Codex Quest level for the shrine. Would be much simpler
		//if gflags accepted variables as well as constants...
		//Basically, there are two flags per shrine which are used in bynary
		if (shrine_frame == SHRINE_SACRIFICE)
		{
			if (gflags[VIEWED_CODEX_FOR_SACRIFICE] && gflags[MEDITATED_AT_SACRIFICE])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_SACRIFICE])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_SACRIFICE])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_SACRIFICE] = true;
		}
		else if (shrine_frame == SHRINE_JUSTICE)
		{
			if (gflags[VIEWED_CODEX_FOR_JUSTICE] && gflags[MEDITATED_AT_JUSTICE])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_JUSTICE])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_JUSTICE])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_JUSTICE] = true;
		}
		else if (shrine_frame == SHRINE_HUMILITY)
		{
			if (gflags[VIEWED_CODEX_FOR_HUMILITY] && gflags[MEDITATED_AT_HUMILITY])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_HUMILITY])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_HUMILITY])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_HUMILITY] = true;
		}
		else if (shrine_frame == SHRINE_SPIRITUALITY)
		{
			if (gflags[VIEWED_CODEX_FOR_SPIRITUALITY] && gflags[MEDITATED_AT_SPIRITUALITY])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_SPIRITUALITY])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_SPIRITUALITY])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_SPIRITUALITY] = true;
		}
		else if (shrine_frame == SHRINE_VALOR)
		{
			if (gflags[VIEWED_CODEX_FOR_VALOR] && gflags[MEDITATED_AT_VALOR])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_VALOR])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_VALOR])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_VALOR] = true;
		}
		else if (shrine_frame == SHRINE_COMPASSION)
		{
			if (gflags[VIEWED_CODEX_FOR_COMPASSION] && gflags[MEDITATED_AT_COMPASSION])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_COMPASSION])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_COMPASSION])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_COMPASSION] = true;
		}
		else if (shrine_frame == SHRINE_HONOR)
		{
			if (gflags[VIEWED_CODEX_FOR_HONOR] && gflags[MEDITATED_AT_HONOR])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_HONOR])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_HONOR])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_HONOR] = true;
		}
		else if (shrine_frame == SHRINE_HONESTY)
		{
			if (gflags[VIEWED_CODEX_FOR_HONESTY] && gflags[MEDITATED_AT_HONESTY])
				codex_quest_level = FINISHED_QUEST;
			else if (gflags[VIEWED_CODEX_FOR_HONESTY])
				codex_quest_level = WENT_TO_CODEX;
			else if (gflags[MEDITATED_AT_HONESTY])
				codex_quest_level = GOT_FROM_SHRINE;
			
			if ((codex_quest_level == WENT_TO_CODEX) || (codex_quest_level == CODEX_NOT_STARTED))
				gflags[MEDITATED_AT_HONESTY] = true;
		}
		
		var xpbonus = 0;
		//The experience bonus from meditating:
		if (codex_quest_level == CODEX_NOT_STARTED)
			xpbonus = 25;
		else if (codex_quest_level == WENT_TO_CODEX)
			xpbonus = 75;

		giveExperience(xpbonus);
		
		SHRINE_FACES->show_npc_face(shrine_frame - 1);
		say("After a while, you feel a sense of calm and inner peace.");
		if (codex_quest_level == CODEX_NOT_STARTED)
		{
			//Got a sacred quest to see the Codex:
			say("A mystical voice sounds inside your head, and a sacred quest is ordained. @Go thou to the Codex, seeker, to learn about " + shrines[shrine_frame] + "!@");
		}
		else if (codex_quest_level == WENT_TO_CODEX)
		{
			//Returning from Codex:
			say("Once again, the mystical voice sounds inside your head. @Well done, seeker! Use well thy newfound enlightenment!@");
		}	

		//NPC banther:
		if (UI_get_array_size(UI_get_party_list()) > 1)
		{
			var barks = ["@It's about time!@",
						 "@We should be going...@"];
			var rand = UI_get_random(2 * UI_get_array_size(barks));
			if (rand > UI_get_array_size(barks))
				abort;
	
			var npc = randomPartyMember();
			while (npc->get_npc_number() == AVATAR)
				npc = randomPartyMember();
			
			script npc after 8 ticks say barks[rand];
		}
		abort;
	}
}