Baiyanda 0x48F ()
{
	if (event == DOUBLECLICK)
	{
		UI_item_say(AVATAR, "@Greetings@");
		delayedBark(BAYANDA, "@We knew thou would come.@", 3);
		UI_set_schedule_type(BAYANDA, TALK);
	}
	if (event == STARTED_TALKING)
	{
		UI_run_schedule(BAYANDA);
		UI_clear_item_say(BAYANDA);
		UI_show_npc_face0(BAYANDA, 0);

		if (UI_get_item_flag(BAYANDA, MET))
			say("@We meet again.@");
		else
			say("@Thou art Avatar!@");

		add(["name", "bye"]);
		if (hasItemCount(PARTY, 1, SHAPE_BUCKET, 9, 2))
			add("got blood");

		converse (0)
		{
			case "got blood" (remove):
				say("@Hurry! Take Ice Dragon blood to Yenani! No time to delay!@");
				abort;
			
			case "name" (remove):
				say("@I am Baiyanda, mate of Mwaerno and healer for Gwani people.@");
				UI_set_item_flag(BAYANDA, MET);
				add(["Mwaerno", "healer"]);
			
			case "Mwaerno" (remove):
				say("@He great hunter. Mwaerno and Baiyanda joined by Yenani many years ago.@");
				add(["hunter", "Yenani"]);
			
			case "hunter" (remove):
				say("@Myauri Master Hunter of Gwani. He know where game are and how to find. But Mwaerno best at catching game. I very proud of him.@");
			
			case "Yenani" (remove):
				say("@She our chieftain. Gwani people always led by females. Yenani good friend of Gwenno.@");
				add("Gwenno");
			
			case "Gwenno" (remove):
				if (UI_get_schedule_type(GWENNO) == WAIT)
				{
					say("@Gwenno dead. She good woman. Very generous. She once make gift of bucket to Baiyanda. Baiyanda place body at sacred Gwani Death Temple.@");
					gflags[TALKED_TO_GWANI_ABOUT_GWENNO] = true;
					add("Where is the temple?");
				}
				else
					say("@Baiyanda happy that Gwenno alive again!@");
			
			case "Where is the temple?" (remove):
				say("@It not thing for thou to know. Gwenno must rest now, her soul rest. Leave her. I know it painful, but thou must do this. Even if thou found her thou could not unseal her body without sacred horn of Gwani. Long ago it taken by fiend who steals our dead from us.@");
			
			case "healer" (remove):
				say("@I treat all wounds and illness of Gwani people. Baiyanda not use ways of human healers. Gwani live simple in balance with nature, not force on nature. Gwani healers learn secrets of herbs and animals. If thou need healer, I will help thou.@");
				add(["I need a healer", "herbs and animals"]);
			
			case "I need a healer" (remove):
				say("@Rest easy. I will help thou.@");
				say("@Who thou want to heal?@");
				var living_npcs = getNonAutomatonPartyMembers();
				var namelist = ["nobody"];
				var npc;
				var index;
				var max;
				
				for (npc in living_npcs with index to max)
					namelist = (namelist & UI_get_npc_name(npc));

				living_npcs = [0, living_npcs];
				
				var choice = chooseFromMenu2(namelist);
				choice = living_npcs[choice];
				if (choice == 0)
					say("@Maybe thou return later.@");
				else
				{
					var npcnumber = UI_get_npc_number(choice);
					var poisoned = UI_get_item_flag(choice, POISONED);
					var str = UI_get_npc_prop(choice, STRENGTH);
					var hps = UI_get_npc_prop(choice, HEALTH);
					var npcname = UI_get_npc_name(choice);
					if (str > hps)
					{
						UI_set_npc_prop(npcnumber, HEALTH, str - hps);
						say("@All right, " + npcname + " healed now!@");
					}
					else
					{
						if (!poisoned)
						{
							if (npcnumber == AVATAR)
								say("@Thou not hurt!@");
							else
								say("@" + npcname + " not hurt! Thou play trick?@");
						}
						else if (npcnumber == AVATAR)
							say("@Thou poisoned bad! I fix.@");
						else
							say("@" + npcname + " poisoned bad! I fix.@");
					}
					if (poisoned == true)
					{
						UI_clear_item_flag(choice, POISONED);
						say("@Good! Poison gone now.@");
					}
				}
			
			case "herbs and animals" (remove):
				say("@Magic corrupts balance of nature. Gwani healers learn things like prepare dried fish and use Ice Dragon blood.@");
				add(["balance of nature", "dried fish", "Ice Dragon blood"]);
			
			case "balance of nature" (remove):
				say("@To Gwani, best way to live -- only way -- to live in harmony with nature. And so we not do anything that force nature.@");
			
			case "dried fish" (remove):
				say("@Dried fish very good food. It keeps thou from hunger longer than other food.@");
				say("@Would thou like some?@");
				if (askYesNo())
				{
					say("@Here, have some. It good.@");
					giveItemsToPartyMember(AVATAR, 1, SHAPE_FOOD, QUALITY_ANY, FRAME_DRIED_FISH, false, true);
				}
				else
					say("@Too bad. Thou should try some.@");
			
			case "Ice Dragon blood" (remove):
				say("@Special things about blood of Ice Dragon that can cure almost any sickness.@");
				say("@But Ice Dragons very rare creatures. Gwani honor all life -- try everything before we hunt them.@");
				say("@One did live north of our village. We drove different one east many years ago.@");

				if (UI_get_item_flag(NEYOBI, SI_ZOMBIE))
				{
					say("@Ice Dragon blood maybe only thing powerful enough to cure Neyobi. But it so rare that it very hard to find. Five of our hunters looking for it.@");
					say("@Gwenno said thou help people in need. Thou must find some Ice Dragon blood for Neyobi! It is the last hope!@");
					if (!gflags[BAYANDA_GAVE_BUCKET])
					{
						say("@Here, take bucket of mine. If thou find and slay Ice Dragon, please bring bucket of blood. Take to Yenani, she know what to do with it.@");
						giveItemsToPartyMember(AVATAR, 1, SHAPE_BUCKET, 0, 0, false, true);
						say("@Good luck. Neyobi's life depend on it.@");
						gflags[BAYANDA_GAVE_BUCKET] = true;
					}
					else
						say("@Give Yenani bucket of Ice Dragon blood. She know what to do with it.@");
				}
				else
					say("@Ice Dragon blood saved Neyobi's life. Great many thanks, Avatar.@");

				add(["north dragon", "east dragon", "Neyobi"]);
			
			case "north dragon" (remove):
				say("@I do not know exactly where. We heard nothing for many years.@");
			
			case "east dragon" (remove):
				say("@Years ago, Gwani attacked by dragon. Myauri and Mwaerno led hunters to fight it. Gwani drove it east beyond mountains. It very old dragon and still unmated. Very rare.@");
			
			case "Neyobi" (remove):
				if (UI_get_item_flag(NEYOBI, SI_ZOMBIE))
					say("@Neyobi ill from strange sickness. Baiyanda never seen before. Nothing Baiyanda tried help her. Ice Dragon blood only thing that could save her.@");
				else
					say("@No magic in whole world would have saved Neyobi's life. But when Gwani way of healing with balance of nature done, she better.@");
			
			case "bye":
				UI_remove_npc_face0();
				UI_remove_npc_face1();
				delayedBark(AVATAR, "@I thank thee.@", 0);
				delayedBark(BAYANDA, "@Very good.@", 3);
				break;
			
		}
	}
}
