/*
 *	This source file contains modified code for Jaana to allow her
 *	to cast spells.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-02-03
 */

Jaana 0x405 ()
{
	var avatartitle;
	var avatarname;
	var party = UI_get_party_list();
	var partysize;
	var choice;
	var spellbook;
	var maxmana = getNPCLevel(item) * 3;
	var talk_cast = ["@Dost thou wish me to cast a spell of which circle?@",
					 "@What spell wouldst thou like me to cast?@",
					 "@Maybe thou dost wish for a different circle?@",
					 "@Some other time, perhaps...@",
					 "@Alas, I don't have enough reagents for that spell.",
					 "@I am lacking ",
					 "@Alas, that spell is beyond my power. I must train somewhat to be able to cast it.@",
					 "@I must rest a while before I can cast this spell.@"];

	if (event == DOUBLECLICK)
	{
		avatartitle = getPoliteTitle();
		avatarname = getAvatarName();

		add(["name", "job", "bye"]);
		if (item in party)
			add("leave");

		if (!((item in party) && gflags[MET_JAANA]))
			add("join");

		if ((item in party) && (gflags[BROKE_TETRAHEDRON]))
			add("Cast spell");
		
		if (gflags[KNOWS_COVE_GOSSIP])
		{
			if (gflags[ASKED_JAANA_NAME])
				add("Lord Heather");
		}
		
		if (gflags[KNOWS_JAANA_IS_HEALER])
			add("heal");

		if (!gflags[MET_JAANA])
		{
			item.say("You are surprised to see your old companion Jaana, looking only slightly aged since your last visit.");
			gflags[MET_JAANA] = true;
			
			spellbook = UI_create_new_object(SHAPE_JAANAS_SPELLBOOK);
			give_last_created();
			
			var reagent_shapes = [SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT];
			var reagent_frames = [3, 4, 5, 6, 7];
			var reagent_quantity = [15, 30, 30, 15, 10];
		
			var pouch = createContainerWithObjects(SHAPE_BAG, reagent_shapes, reagent_frames, reagent_quantity, [0, 0, 0, 0, 0]);
			pouch->set_last_created();
			give_last_created();
		}
		else
		{
			item.say("@Yes, " + avatarname + "?@ Jaana asks.");
			spellbook = get_cont_items(SHAPE_JAANAS_SPELLBOOK, QUALITY_ANY, FRAME_ANY);
		}

		converse (0)
		{
			case "name" (remove):
				say("@Why, I am Jaana. Thou shouldst remember me!@");

				if (gflags[KNOWS_COVE_GOSSIP])
					add("Lord Heather");

				gflags[ASKED_JAANA_NAME] = true;
			
			case "job":
				say("@I have been the Cove Healer for some time now, and can provide thee with mine healing services. Since magic is not reliable, I have been yearning to join a party of adventurers, such as mine old friends. I miss the old life!@");
				add(["heal", "friends", "magic"]);

				gflags[KNOWS_JAANA_IS_HEALER] = true;

				if (!(item in party))
					add("join");
			
			case "heal":
				if (spellbook)
				{
					var healing_spells = getLeveledSpellList(item,
						false,
						["Cure", "Mass cure", "Heal", "Great heal", "Restoration", "Resurrect"],
						[1, 2, 3, 5, 7, 8],
						[]);
					var price = 0;
					var doheal = true;
					
					while (true)
					{
						say("@In which service art thou interested?@");
						choice = askForResponse(["none", healing_spells]);
						if (choice != "none")
						{
							if (item in party)
								say("@Since I am travelling in thy group, I shall waive my fee.@");
							else
							{
								if (choice == "Cure") price = 15;
								else if (choice == "Mass cure") price = 45;
								else if (choice == "Heal") price = 15;
								else if (choice == "Great heal") price = 30;
								else if (choice == "Restoration") price = 150;
								else if (choice == "Resurrect") price = 400;
								say("@My price is " + price + " gold. Is this price agreeable?@");
								
								doheal = false;
								if (askYesNo())
								{
									var partygold = PARTY->count_objects(SHAPE_GOLD, QUALITY_ANY, FRAME_ANY);
									if (partygold >= price)
									{
										doheal = true;
										UI_remove_party_items(price, SHAPE_GOLD, QUALITY_ANY, FRAME_ANY, true);
									}
									else
										say("@Thou dost not have that much gold! Mayhaps thou couldst return with more and purchase the service then.@");
								}
								else
									say("@Then thou must look elsewhere for that service.@");
							}
	
							if (doheal)
							{
								if ((choice == "Restoration") || (choice == "Mass cure"))
									say("@I am glad to help, " + avatartitle + "!@");
	
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
									talk_cast);
							}
						}	
						else
							break;
					}	
					say("@If thou hast need of my services later, I will be here.@");
				}
				else
					say("@I would be happy to, but thou hast taken away my spellbook.@");
				
			case "Cast spell":
				if (spellbook)
				{
					event = DOUBLECLICK;
					spellbook->Jaanas_Spellbook();
				}
				else
					say("@I would be happy to, but thou hast taken away my spellbook.@");
			
			case "friends" (remove):
				say("@Our old friends -- Iolo, Shamino, and Dupre. The men who conquer evil in the name of Lord British!@");
				add(["Iolo", "Shamino", "Dupre", "Lord British"]);
			
			case "join":
				partysize = UI_get_array_size(party);
				if (partysize < 8)
				{
					say("@I would be honored to join thee, " + avatartitle + "!@");
					add_to_party();
					add("leave");
					remove("join");
				}
				else
					say("@I do believe thou dost have too many members travelling in thy group. I shall wait until someone leaves and thou dost ask me again.@");
			
			case "leave":
				say("@Dost thou want me to wait here or should I go home?@");
				UI_clear_answers();
				choice = askForResponse(["wait here", "go home"]);
				if (choice == "wait here")
				{
					say("@Very well. I shall wait until thou dost return.@*");
					remove_from_party();
					set_schedule_type(WAIT);
					abort;
				}
				else
				{
					say("@I shall obey thy wish. I would be happy to re-join if thou shouldst ask. Goodbye.@*");
					remove_from_party();
					set_schedule_type(LOITER);
					abort;
				}
			
			case "magic" (remove):
				if (!gflags[BROKE_TETRAHEDRON])
					say("@My magic has been affected by something in the air, but I have found that my senses are still with me. Hast thou noticed that the mages in the land are afflicted in the head? It is most disconcerting. Nevertheless, I can manage to cast a spell or two most of the time.@");
				else
					say("@I feel that the ether is flowing smoothly now. Magic is alive again!@");
			
			case "Lord Heather" (remove):
				say("Jaana blushes. @Yes, I have been seeing our Town Mayor for some time now.@");

				if (isNearby(LORD_HEATHER))
				{
					LORD_HEATHER.say("@I see that thou art leaving Cove for a while, my dear?@*");

					item.say("@Yes, milord. But I shall return. I promise thee.@*");

					LORD_HEATHER.say("@I shall try not to worry about thee, but it will be difficult.@*");

					item.say("@Do not worry. I shall be safe with the Avatar.@*");

					LORD_HEATHER.say("@I do hope so.@ The Mayor embraces Jaana.*");
					LORD_HEATHER.hide();
				}
			
			case "Iolo" (remove):
				if (!isNearby(IOLO))
					say("@Where is he? 'Twould be good to see him!@");

				else
				{
					say("@He looks the same to me! Perhaps he has a little more waistline than before... but that is to be expected if one stays away from adventuring for too long!@*");
					
					IOLO.say("@What dost thou mean? 'Little more waistline' indeed!@*");
					IOLO.hide();
					
					say("@No offense intended, Iolo!@");
				}
			
			case "Shamino" (remove):
				if (!isNearby(SHAMINO))
					say("@Oh, I would love to see him. I wonder where he might be.@");

				else
				{
					say("@Shamino, thou dost not look like a 'kid' anymore! What didst happen? Didst thou reach the venerable age of thirty?@*");
					
					SHAMINO.say("@Hmph. I am still a kid at heart.@*");
					SHAMINO.hide();
					
					say("@That is a relief.@ She grins cheekily.");
				}
			
			case "Dupre" (remove):
				if (!isNearby(DUPRE))
					say("@I miss having a drink or two with that rogue! Let's go find that knight!@");

				else
				{
					say("@For someone recently knighted, he has retained his good looks and boyish charm, hasn't he?@*");

					DUPRE.say("@Thou dost mean 'mannish' charm, dost thou not?@*");

					item.say("@Oh, pardon -me-, sir. Thine immaturity confused me for a moment.@*");
					
					DUPRE.say("@Art thou going to let her get away with that, " + avatarname + "?@");

					if (askYesNo())
						say("Dupre is speechless and turns away in a huff.*");
					else
						say("@Good!@ Jaana winks at you from behind his back.*");
					
					DUPRE.hide();
				}
			
			case "Lord British" (remove):
				say("@I have not seen our liege in many years.@");
			
			case "bye":
				break;
			
		}
		say("@Goodbye, " + avatartitle + ".@*");
	}
	else if (event == PROXIMITY)
		scheduleBarks(item);
}
