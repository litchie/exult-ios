//I decided to reimplement Iolo's usecode entirely. I could have left some things
//for the original to handle, but it wouldn't allow me to do what I wanted: which
//was to make him refuse to leave while on Spinebreaker Mountains.

// externs
extern singSong 0x9B0 (var song);
extern ioloShape 0x1D1 ();

Iolo 0x403 ()
{
	var iolo_id;
	var avatar_title;
	var avatar_name;
	var trainer;
	
	iolo_id = UI_get_npc_id(IOLO);
	avatar_title = getPoliteTitle();
	avatar_name = getAvatarName();
	
	if (event == DEATH)
	{
		if (gflags[MONITOR_TRAINING])
		{
			trainer = UI_get_oppressor(IOLO);
			trainer = (0 - trainer);
			if (!gflags[TEMP_FLAG_1])
			{
				gflags[TEMP_FLAG_1] = true;
				trainingEndDialog(UI_get_npc_object(trainer), UI_get_npc_object(IOLO));
				return;
			}
			endMonitorTraining(item);
			return;
		}
	}
	
	else if (event == DOUBLECLICK)
	{
		delayedBark(AVATAR, "@Dear friend...@", 0);
		IOLO->makePartyFaceNPC();
		
		if (!UI_get_item_flag(IOLO, SI_ZOMBIE))
		{
			if (gflags[EQUIPMENT_EXCHANGED] && (!gflags[IOLO_MADE_EQUIPMENT_LIST]))
			{
				var freed = false;
				var doors = UI_find_nearby(IOLO, SHAPE_DOOR_VERTICAL, 15, 0);
				var door;
				var index;
				var max;
				
				for (door in doors with index to max)
					if ((UI_get_item_quality(door) == 104) && (getDoorState(door) != 2))
						freed = true;

				doors = UI_find_nearby(IOLO, SHAPE_DOOR_HORIZONTAL, 15, 0);
				for (door in doors with index to max)
					if (UI_get_item_quality(door) == 104)
						freed = true;

				if (freed)
				{
					delayedBark(IOLO, "@I am free!@", 2);
					UI_set_schedule_type(IOLO, TALK);
				}
				else
				{
					IOLO->faceAvatar();
					delayedBark(IOLO, "@Help me, Avatar!@", 2);
					script IOLO after 7 ticks
					{
						nohalt;
						call ioloShape;
					}
					UI_set_schedule_type(IOLO, WAIT);
				}
			}
			else
			{
				delayedBark(IOLO, "@Yes?@", 2);
				UI_set_schedule_type(IOLO, TALK);
			}
		}
		else
		{
			var barks = ["@Wrong is right!@", "@Life is a farce!@", "@Sing the dirge of love...@", "@Futility is the answer!@"];
			var rand1 = UI_get_random(UI_get_array_size(barks));
			delayedBark(IOLO, barks[rand1], 2);
		}
	}
	
	else if (event == STARTED_TALKING)
	{
		UI_clear_item_say(IOLO);
		UI_clear_item_say(AVATAR);
		if (!gflags[STARTING_SPEECH])
		{
			UI_init_conversation();
			UI_show_npc_face0(IOLO, 0);
			say("@'Twas a fearsome passage, " + avatar_name + ". After we sailed between the Serpent Pillars, I could have sworn that we were flying...@");
			say("@Yet here we are on the ship. I wonder if I lost anything...@");
			UI_show_npc_face1(DUPRE, 0);
			say("@We may be on the ship, but the ship is upon dry land! I think that thou art correct, Iolo. We did fly!@");
			UI_remove_npc_face1();
			UI_set_conversation_slot(0);
			say("@Brrr. Dost thou notice the chill in the air? 'Tis much colder here than at home.@");
			say("@I hope Gwenno brought enough warm clothing...@");
			UI_show_npc_face1(SHAMINO, 0);
			say("@Do not worry so, old friend. We shall find thy wife soon enough.@");
			UI_remove_npc_face1();
			UI_show_npc_face1(DUPRE, 0);
			say("@And that fiend, Batlin, I hope!@");
			UI_remove_npc_face1();
			UI_set_conversation_slot(0);
			say("@Look, " + avatar_name + "! A strange storm is nearly upon us. This is certainly not Britannia!@");
			UI_remove_npc_face0();
			delayedBark(SHAMINO, "@Where are we?@", 5);
			delayedBark(DUPRE, "@Let us find Batlin!@", 15);
			delayedBark(IOLO, "@Do not forget about Gwenno...@", 35);
			UI_add_to_party(IOLO);
			UI_add_to_party(DUPRE);
			UI_add_to_party(SHAMINO);
			gflags[STARTING_SPEECH] = true;
			UI_clear_item_flag(AVATAR, DONT_MOVE);
			abort;
		}
		if (gflags[EQUIPMENT_EXCHANGED] && (!gflags[IOLO_MADE_EQUIPMENT_LIST]))
		{
			UI_show_npc_face0(IOLO, 0);
			say("@I thank thee for freeing me from this hellhole! The natives of this place are ignorant sots. Imagine, thinking me to be a sorcerer!@");
			say("@Hast thou looked in thy packs since the storm, " + avatar_title + "? Nothing in my backpack is as it was when Lord British gave us the list, Avatar!@");
			if (npcNearbyAndVisible(SHAMINO))
			{
				UI_show_npc_face1(SHAMINO, 0);
				say("@I am making a list of the strange items which the storm gave us.@");
				UI_remove_npc_face1();
				UI_set_conversation_slot(0);
				say("@Then I shall add to it.@");
			}
			else
			{
				if (npcNearbyAndVisible(DUPRE))
				{
					UI_show_npc_face1(DUPRE, 0);
					say("@I have a list of the strange items which the storm gave us.@");
					UI_remove_npc_face1();
					UI_set_conversation_slot(0);
					say("@I shall add to it.@");
				}
				else
				{
					message("@I shall make a list of the strange things I have found in my packs. Perhaps these are clues to where our real belongings have gone.@");
					say();
				}
			}
			UI_add_to_party(IOLO);
			UI_set_new_schedules(IOLO, MIDNIGHT, EAT_AT_INN, [0x097C, 0x0464]);
			gflags[IOLO_HAS_BELONGINGS] = true;
			setExchangedItemFlags();
			gflags[IOLO_MADE_EQUIPMENT_LIST] = true;
			abort;
		}
		UI_show_npc_face0(IOLO, 0);
		if (UI_get_item_flag(IOLO, IN_PARTY))
		{
			UI_set_schedule_type(IOLO, FOLLOW_AVATAR);
			add("leave");
		}
		else
		{
			UI_run_schedule(IOLO);
			add("join");
		}
		
		if (iolo_id == BOOTED_FOR_FREEDOM)
		{
			UI_remove_answer("join");
			say("@Thank the Virtues! Thou art whole and hale, " + avatar_title + "!@");
			say("@I feared that I had lost both thee and my beloved wife, but thou hast survived the depths even as Dupre assured me that thou wouldst...@");
			if (npcNearbyAndVisible(GUSTACIO))
			{
				UI_show_npc_face1(GUSTACIO, 0);
				say("@'Tis said that only a superb Mage can leave the Mountains of Freedom alive.@");
				UI_remove_npc_face1();
				UI_set_conversation_slot(0);
			}
			
			say("@The Sorcerer Gustacio hath been instructing me somewhat in the magic of this land, " + avatar_title + ". I think that he doth have information that shall interest thee.@");
			gflags[AFTER_FREEDOM_NEWS] = true;
			UI_set_npc_id(IOLO, 0);
			UI_add_to_party(IOLO);
			gflags[IOLO_HAS_BELONGINGS] = true;
			remove("join");
			add("leave");
		}
		else if (iolo_id == CURED_OF_INSANITY)
		{
			say("@No truer friend can I have than thee, " + avatar_title + "! Not only thou didst save my wife, but thou hast saved me as well... Thank thee, my friend!@");
			UI_set_npc_id(IOLO, 0);
			UI_add_to_party(IOLO);
			script IOLO after 15 ticks call xenkaReturns;
			remove("join");
			add(["leave"]);
		}
		else
		{
			say("@Wouldst thou enjoy some idle conversation, " + avatar_title + ", or shall I embrace my lute and provide a musical interlude?@");
			add("a song");
		}
		
		if (UI_get_npc_id(SHAMINO) == BOOTED_FOR_FREEDOM)
			add("Shamino's whereabouts");

		if (UI_get_npc_id(DUPRE) == BOOTED_FOR_FREEDOM)
			add("Dupre's whereabouts");

		if (UI_get_npc_id(BOYDON) == BOOTED_FOR_FREEDOM)
			add("Boydon's whereabouts");

		if (gflags[IOLO_HAS_BELONGINGS] && (UI_get_cont_items(IOLO, SHAPE_ANY, QUALITY_ANY, FRAME_ANY) && !UI_get_item_flag(IOLO, IN_PARTY)))
			add("belongings");

		UI_show_npc_face0(IOLO, 0);
		add(["Gwenno", "bye"]);
		converse (0)
		{
			case "belongings" (remove):
				askIoloBelongings();
			
			case "join" (remove):
				if (gflags[BEGAN_KNIGHTS_TEST] && (!gflags[SLAIN_WOLF]))
				{
					if (npcNearbyAndVisible(SCHMED))
					{
						say("@But " + avatar_title + ", the Guardian of the Test is standing right here. If thou desirest to cheat, thou shouldst at least do so covertly...@");
						UI_show_npc_face1(SCHMED, 0);
						say("@No cheating, stranger!@");
						UI_remove_npc_face1();
						delayedBark(SCHMED, "@No cheating!@", 2);
					}
					else
						say("@I cannot join thee, friend. Even if thou wouldst cheat at the Test, I cannot aid thee in so doing. I am thy true friend.@");

					delayedBark(IOLO, "@Sorry...@", 0);
					abort;
				}
				if (UI_get_array_size(UI_get_party_list2()) < 5)
				{
					add("leave");
					say("@'Tis always an adventure to travel with thee, " + avatar_name + "! I shall be proud to accompany thee.@");
					UI_add_to_party(IOLO);
					gflags[IOLO_HAS_BELONGINGS] = true;
				}
				else
				{
					say("@I would be glad to accompany thee, " + avatar_title + ". However, I am an old man, and I can see that thou hast many companions at thy side.@");
					say("@I think that perhaps I should remain where I am...@");
				}
			
			case "leave" (remove):
				if (!gflags[EQUIPMENT_EXCHANGED])
					say("@Thy concern for an old man is appreciated, my old friend. However, we have barely begun this adventure, and I am spry enough to keep up with thee!@");
				
				else if (getAvatarLocationID() == SPINEBREAKER_MOUNTAINS)
					say("@Thy concern for an old man is appreciated, my old friend. However, we have very close to Batlin now, and I am spry enough to keep up with thee!@");
				
				else
				{
					add("join");
					message("@Whatever thou dost wish, Avatar.@");
					say();
					UI_remove_from_party(IOLO);
					askIoloBelongings();
					npcAskWhereToWait(IOLO);
				}
			
			case "Gwenno" (remove):
				if (UI_get_item_flag(GWENNO, IN_PARTY) || (npcNearbyAndVisible(GWENNO) && (!UI_get_item_flag(GWENNO, SI_ZOMBIE))))
					say("@No truer friend have I had in all of my life than thee, " + avatar_name + ". With my lady love Gwenno returned to my side where she doth belong, my life is once again complete.@");

				else if (!gflags[TALKED_TO_GWANI_ABOUT_GWENNO])
					say("@I miss Gwenno so much, Avatar. I hope that it is not long before we find her and I may hold her in mine arms again.@");

				else if (!gflags[FREED_GWENNOS_BODY])
					say("@Mine heart is broken! My life hath no meaning without my lady love! Oh, " + avatar_name + ", how could our good and noble quest have ended in such tragedy!@");

				else if (gflags[GWENNO_IS_DEAD])
					say("@Now that we have succeeded in freeing Gwenno's body, perhaps the Monks of Monk Isle -- the self-professed masters of life and death -- may be able to help her.@");

				else if (UI_get_item_flag(GWENNO, SI_ZOMBIE))
					say("@We must find some way of restoring Gwenno's mind! Unless we can do that her precious spirit is lost to me.@");

				else
					say("@My soul is at peace. Joy is to know Gwenno, and to have her once more in thriving good health.@");
			
			case "a song" (remove):
				if (UI_get_item_flag(BYRIN, MET))
				{
					say("@Dost thou wish me to repeat a song thou hast already heard? Or dost thou wish to hear a new song?@");
					var songlist = [];
					if (chooseFromMenu2(["old song", "new song"]) == 1)
					{
						if (gflags[HEARD_BEATRIX_SONG] == true)
							songlist = (songlist & 1);
						if (gflags[HEARD_MOUNTAIN_SONG] == true)
							songlist = (songlist & 2);
						if (gflags[HEARD_GWANI_SONG] == true)
							songlist = (songlist & 3);
						if (gflags[HEARD_FOREST_MASTER_SONG] == true)
							songlist = (songlist & 4);
						if (gflags[HEARD_DREAM_SONG] == true)
							songlist = (songlist & 5);
						if (gflags[HEARD_WHITE_DRAGON_SONG] == true)
							songlist = (songlist & 6);
					}
					else
					{
						if (gflags[HEARD_BEATRIX_SONG] == false)
							songlist = (songlist & 1);
						if (gflags[HEARD_MOUNTAIN_SONG] == false)
							songlist = (songlist & 2);
						if (gflags[HEARD_GWANI_SONG] == false)
							songlist = (songlist & 3);
						if (gflags[HEARD_FOREST_MASTER_SONG] == false)
							songlist = (songlist & 4);
						if (gflags[HEARD_DREAM_SONG] == false)
							songlist = (songlist & 5);
						if (gflags[HEARD_WHITE_DRAGON_SONG] == false)
							songlist = (songlist & 6);
					}
					
					if (songlist == [])
						say("@I'm sorry... Thou hast heard all of my songs.@");
					else
					{
						var rand = UI_get_random(UI_get_array_size(songlist));
						singSong(songlist[rand]);
					}
				}
				else
					say("@I cannot think of anything to sing at the moment, " + avatar_title + ". Perhaps if thou didst ask me later...@");
			
			case "Shamino's whereabouts" (remove):
				if (gflags[SHAMINO_RESURRECTED_BY_MONKS])
				{
					say("@Thou hast no doubt heard by now the news? That Shamino doth live again! The monks brought him to us.@");
					say("@I think he was more disturbed by thy departure than he cared to show. Or else they have told him something...@");
				}
				else
					say("@I think he was more disturbed by thine abrupt departure than he cared to show.@");

				say("@He went west into the woods, to be alone.@");
			
			case "Dupre's whereabouts" (remove):
				say("@At the nearest tavern, of course!@");
			
			case "Boydon's whereabouts" (remove):
				say("@Um... well, I am not sure. Perhaps Dupre or Shamino might know.@");
			
			case "bye":
				delayedBark(AVATAR, "@Thanks!@", 0);
				delayedBark(IOLO, "@A pleasure!@", 2);
				UI_remove_npc_face0();
				break;
			
		}
	}
	
	else if (event == EGG)
	{
		var avatar_location = getAvatarLocationID();
		if (avatar_location == DREAM_WORLD)
		{
			UI_show_npc_face0(IOLO, 0);
			say("@Avatar! Why art thou in my dream? I was looking for Gwenno...@");
			say("@Oh. Perhaps I have intruded upon thy dream.@");
			say("@Forgive me. I shall go seeking my wife and leave thee to thy dream.@");

			UI_remove_npc_face0();
			delayedBark(IOLO, "@Pleasant dreams!@", 2);
			
			var pos = UI_get_object_position(CLONE_IOLO);
			UI_sprite_effect(0x1A, pos[X], pos[Y], 0, 0, 0, -1);
			UI_play_sound_effect(0x51);
			UI_set_item_quality(item, 3);
			
			script item after 10 ticks
			{
				nohalt;
				call 0x6D9;		//Note: this function is HEAVILY overused. I am hesitant to give it a name...
								//If the items quality is 3 (as is the case here), it moves CLONE_IOLO elsewhere
								//(likely back to the Test of Purity). Note that is moves *CLONE_IOLO*, not item!
			}
			abort;
		}
		else if (avatar_location == TOLERANCE)
		{
			UI_show_npc_face0(IOLO, 0);
			say("@More snakes! Destroy the vile creatures, Avatar!@");
			UI_remove_npc_face0();
			delayedBark(IOLO, "@I hate snakes!@", 2);
			abort;
		}
		else if (avatar_location == ICE_PLAINS)
		{
			UI_show_npc_face0(IOLO, 0);
			say("@Oh, my poor Gwenno!@ *@Now she is truly frigid.@");
			UI_remove_npc_face0();
			delayedBark(IOLO, "@My love hath now departed!@", 2);
			abort;
		}
	}
	
	else if (event == PROXIMITY)
	{
		if (UI_get_schedule_type(IOLO) == PATROL)
		{
			if (gflags[EQUIPMENT_EXCHANGED] && (!gflags[IOLO_MADE_EQUIPMENT_LIST]))
			{
				var barks2 = ["@Woe is me!@", "@I feel cold.@", "@I am hungry.@", "@Release me!@", "@I'm innocent!@", "@Pity an old man...@"];
				delayedBark(IOLO, barks2[UI_get_random(UI_get_array_size(barks2))], 0);
			}
		}
	}
}
