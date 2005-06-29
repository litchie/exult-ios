//I decided to reimplement Dupre's usecode entirely. I could have left some things
//for the original to handle, but it wouldn't allow me to do what I wanted: which
//was to make him refuse to leave while on Spinebreaker Mountains.

extern askDupreBelongings 0x829 ();

Dupre 0x401 ()
{
	var dupre_id;
	var avatar_title;
	var avatar_name;
	var trainer;
	
	dupre_id = UI_get_npc_id(DUPRE);
	avatar_title = getPoliteTitle();
	avatar_name = getAvatarName();

	if (event == DEATH)
	{
		if (gflags[MONITOR_TRAINING])
		{
			trainer = UI_get_oppressor(DUPRE);
			trainer = (0 - trainer);
			if (!gflags[TEMP_FLAG_1])
			{
				gflags[TEMP_FLAG_1] = true;
				trainingEndDialog(UI_get_npc_object(trainer), UI_get_npc_object(DUPRE));
				return;
			}
			endMonitorTraining(item);
			return;
		}
	}
	
	else if (event == DOUBLECLICK)
	{
		UI_item_say(AVATAR, "@Dupre...@");
		DUPRE->makePartyFaceNPC();
		if (!UI_get_item_flag(DUPRE, SI_ZOMBIE))
		{
			delayedBark(DUPRE, "@Yes, " + avatar_title + "?@", 2);
			UI_set_schedule_type(DUPRE, TALK);
		}
		else
		{
			var barks = ["@Bring to me a woman!@", "@I will slay you all!@", "@Fulfill thy desires!@", "@I must have ale!@"];
			var rand1 = UI_get_random(UI_get_array_size(barks));
			delayedBark(DUPRE, barks[rand1], 2);
		}
	}
	
	else if (event == STARTED_TALKING)
	{
		UI_show_npc_face0(DUPRE, 0);
		UI_clear_item_say(DUPRE);
		
		if (UI_get_item_flag(DUPRE, IN_PARTY))
		{
			UI_set_schedule_type(DUPRE, FOLLOW_AVATAR);
			add("leave");
		}
		else
		{
			UI_run_schedule(DUPRE);
			add("join");
		}
		
		if (dupre_id == BOOTED_FOR_FREEDOM)
		{
			say("@How good to see thee again, " + avatar_title + "! Knowing that thou wouldst soon return, I have waited for thee at this establishment.@");

			if (npcNearbyAndVisible(ROCCO))
			{
				npcSpeakIfNearby(ROCCO, "@And he hath developed quite a bar tab!@");
				UI_set_conversation_slot(0);
				say("@One that I shall pay, worry thou not!@");
			}
			if (!gflags[AFTER_FREEDOM_NEWS])
			{
				say("@And I have good news for thee, " + avatar_title + ".@");
				add("good news");
			}
			UI_set_npc_id(DUPRE, 0);
			UI_add_to_party(DUPRE);
			gflags[DUPRE_HAS_BELONGINGS] = true;
			remove("join");
			add(["leave"]);
		}
		else if (dupre_id == CURED_OF_INSANITY)
		{
			say("@I am not as good with words as our friend Iolo, Avatar, so I shall thank thee by fighting at thy side...@");
			UI_set_npc_id(DUPRE, 0);
			UI_add_to_party(DUPRE);
			script DUPRE after 15 ticks call xenkaReturns;
			remove("join");
			add(["leave"]);
		}
		else
		{
			if (gflags[EQUIPMENT_EXCHANGED] && (!gflags[DUPRE_MADE_EQUIPMENT_LIST]))
			{
				say("@" + avatar_name + "! I thought I would never find thee!@");
				say("@When that strange storm struck, there was a flash, and then I found myself in a wilderness.@");
				say("@Fearing to be slain by the Goblins patrolling the woods, I instead took shelter in this walled city. But these Pikemen insist on escorting me to their leader!@");

				if (UI_npc_nearby(MARSTEN))
				{
					UI_show_npc_face1(MARSTEN, 0);
					say("@That is all right. If thou art with my friend here, thou needest not speak with me.@");
					UI_remove_npc_face1();
					UI_set_conversation_slot(0);
				}
				UI_add_to_party(DUPRE);
				remove("join");
				add("leave");
				UI_set_new_schedules(DUPRE, MIDNIGHT, EAT_AT_INN, [0x097C, 0x0464]);
			}
			else
			{
				var lines = ["@This is no time for idle chat. We're on a quest!@", "@This must wait. There is no time for delay!@", "@We must press on, Avatar!@", "@The Sands of Time are dwindling! We must hurry!@"];
				var rand2 = UI_get_random(UI_get_array_size(lines));
				say(lines[rand2]);
			}
		}

		if (UI_get_npc_id(SHAMINO) == BOOTED_FOR_FREEDOM)
			add("Shamino's whereabouts");

		if (UI_get_npc_id(IOLO) == BOOTED_FOR_FREEDOM)
			add("Iolo's whereabouts");

		if (UI_get_npc_id(BOYDON) == BOOTED_FOR_FREEDOM)
			add("Boydon's whereabouts");

		if (gflags[DUPRE_HAS_BELONGINGS] && (!UI_get_item_flag(DUPRE, IN_PARTY)) && UI_get_cont_items(DUPRE, SHAPE_ANY, QUALITY_ANY, FRAME_ANY))
			add("belongings");

		add(["bye"]);
		converse (0)
		{
			case "belongings" (remove):
				askDupreBelongings();
			
			case "good news" (remove):
				if (UI_get_item_flag(IOLO, IN_PARTY) || (UI_get_npc_id(IOLO) == BOOTED_FOR_FREEDOM))
					say("@But I should let Iolo tell thee...@");

				else
				{
					say("@The wizard Gustacio hath agreed to aid us, if we will but assist him in his experiments.@");
					say("@He bet with me that thou wouldst not survive Freedom, but I told him that thou wert made of tougher stuff than any dungeon!@");
					say("@I think thy survival shall impress him...@");
					gflags[AFTER_FREEDOM_NEWS] = true;
				}
			
			case "join" (remove):
				if (gflags[BEGAN_KNIGHTS_TEST] && (!gflags[SLAIN_WOLF]))
				{
					say("@But Sir Shmed said that the Test is only for one, " + avatar_title + "! I cannot come with thee.@");
					npcSpeakIfNearby(SCHMED, "@Thou must enter alone, stranger.@");
					delayedBark(SCHMED, "@Alone!@", 0);
					delayedBark(DUPRE, "@Be brave!@", 0);
					abort;
				}

				if (UI_get_array_size(UI_get_party_list2()) < 5)
				{
					add("leave");
					say("@With great pride!@");
					UI_add_to_party(DUPRE);
					gflags[DUPRE_HAS_BELONGINGS] = true;
				}
				else
				{
					say("@But thou hast so many companions, " + avatar_name + "! I shall only be in thy way.@");
					say("@'Twould be better for me to remain where I am, " + avatar_title + ".@");
				}
			
			case "leave" (remove):
				if (!gflags[EQUIPMENT_EXCHANGED])
					say("@Leave thee at a time such as this? Surely, thou dost jest. Onward!@");

				else if (getAvatarLocationID() == SPINEBREAKER_MOUNTAINS)
					say("@Leave when we are so close to Batlin? Surely, thou dost jest. Onward!@");
					
				else
				{
					add("join");
					say("@I hesitate to leave thee. But if thou dost insist...@");
					UI_remove_from_party(DUPRE);
					askDupreBelongings();
					npcAskWhereToWait(DUPRE);
				}
			
			case "Shamino's whereabouts" (remove):
				if (gflags[SHAMINO_RESURRECTED_BY_MONKS])
					say("@I have exciting news -- the monks have returned Shamino to us again, healthy and whole! This he can explain to thee.@");

				say("@Not being as patient as I am, " + avatar_title + ", our good friend Shamino hath gone into the woods to hunt wild game.@");
				say("@He went west from here, towards the magic woods.@");
			
			case "Iolo's whereabouts" (remove):
				say("@The good bard was very disturbed by thy sudden departure, and hath been questioning the Mages constantly as to thy fate.@");
				say("@He hath gained the friendship of the Sorcerer Gustacio, and hath taken to aiding that worthy in his studies. We can find him there.@");
			
			case "Boydon's whereabouts" (remove):
				say("@Thou wouldst never believe it! Boydon hath become very friendly with young Bucia of the Canton, and spends much time with her.@");
				say("@If we can find her, then we shall find him!@");
			
			case "bye":
				if (gflags[EQUIPMENT_EXCHANGED] && (!gflags[DUPRE_MADE_EQUIPMENT_LIST]))
				{
					say("@Before we go any farther, Avatar, I think we should take stock of our supplies.@");
					say("@That blasted storm exchanged all of mine equipment for useless refuse! Even mine enchanted shield!@");
					say("@We cannot hope to survive long without the proper equipment. Perhaps if we can find where this rubbish came from, we can find our good equipment.@");
					say("@I shall prepare a list.@");
					if (UI_npc_nearby(SHAMINO))
					{
						npcSpeakIfNearby(SHAMINO, "@I have already begun one.@");
						UI_set_conversation_slot(0);
						say("@Then I shall add to it.@");
					}
					setExchangedItemFlags();
					gflags[DUPRE_MADE_EQUIPMENT_LIST] = true;
				}
				UI_remove_npc_face0();
				delayedBark(AVATAR, "@That's all for now.@", 0);
				delayedBark(DUPRE, "@Yes, " + avatar_title + ".", 2);
				break;
			
		}
	}
	
	else if (event == EGG)
	{
		if (getAvatarLocationID() == TOLERANCE)
		{
			UI_show_npc_face0(DUPRE, 0);
			say("@Unholy creatures! We must destroy these foul skeletons, Avatar!@");
			UI_remove_npc_face0();
			delayedBark(DUPRE, "@I hate skeletons!@", 2);
			abort;
		}
	}
}
