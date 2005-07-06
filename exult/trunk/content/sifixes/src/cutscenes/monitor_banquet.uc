//This file exists to prevent the List Field bug, where the pikeman goes AWOL
//if you are too close to him when you double-click the Banquet Hall doors.
//I forward everything else to the original.

// externs
extern var knightsTestReaction 0x8ED ();

MonitorBanquet 0x1C3 ()
{
	var index;
	var max;
	var npc;
	
	if ((event == SCRIPTED) && (MARSTEN->get_npc_id() == 0))
	{
		UI_play_music(15, getPathEgg(5, 1));
		AVATAR->set_item_flag(DONT_MOVE);
		var health = SHAZZANA->get_npc_prop(HEALTH);
		SHAZZANA->set_npc_prop(HEALTH, (SHAZZANA->get_npc_prop(STRENGTH) - health));
		health = LUTHER->get_npc_prop(HEALTH);
		LUTHER->set_npc_prop(HEALTH, (LUTHER->get_npc_prop(STRENGTH) - health));

		npc = partyUtters(1, 0, 0, true);
		
		var partytoken = "We";
		if (UI_get_array_size(UI_get_party_list()) < 3)
			partytoken = "I";

		delayedBark(npc, "@" + partytoken + " shall wait for thee here!@", 5);
		
		var partymembers = removeFromArray(AVATAR->get_npc_object(), UI_get_party_list2());
		var pos;
		for (npc in partymembers with index to max)
		{
			npc->remove_from_party();
			pos = npc->get_object_position();
			npc->set_new_schedules(MIDNIGHT, STANDTHERE, [pos[X], pos[Y]]);
			npc->run_schedule();
		}
		
		AVATAR->item_say("@This must be the place!@");
		AVATAR->si_path_run_usecode([0x41F, 0xA7C, 0x0], PATH_SUCCESS, item, MonitorBanquet, true);
		UI_set_path_failure(MonitorBanquet, item, PATH_FAILURE);
		gflags[MONITOR_BANQUET_STARTED] = true;
		//In the original, the avatar's position is used instead of
		//a hardcoded one. I preferred the hardcoded one -- this
		//allows higher resolutions to work if the player is too far
		//away when double-clicking the doors.
		//var objpos = [AVATAR->get_object_position(), QUALITY_ANY, 6];
		var objpos = [0x41F, 0xA7C, 0x0, QUALITY_ANY, 6];
		var eggs = objpos->find_nearby(SHAPE_EGG, 40, 16);
		var egg;
		var dir;
		var eggquality;
		var directions = [4,	   4,		0, 4,		 4,		  2,
						  6, 	   2,	   6,		2,	   6,	   0];
		var npcids =	 [MARSTEN, SPEKTOR, 0, SHAZZANA, FLICKEN, BRENDANN,
						  CALADIN, CELLIA, TEMPLAR, KRAYG, LUTHER, 0];
		for (egg in eggs with index to max)
		{
			pos = egg->get_object_position();
			eggquality = egg->get_item_quality();
			npc = false;
			if (eggquality == 12)
			{
				LUCILLA->move_object(pos);
				LUCILLA->set_schedule_type(WAIT);
				script LUCILLA
				{	face WEST;					actor frame STAND;}
			}
			else
			{
				dir = directions[eggquality];
				npc = npcids[eggquality];
				if (npc && (!npc->get_item_flag(DEAD)))
				{
					npc->move_object(pos);
					npc->set_schedule_type(WAIT);
					script npc
					{	face dir;					continue;
						actor frame SIT;}
				}
			}
		}
		
		//This is the offending code which deletes the trainer's egg.
		//It seems specifically tailored to delete -it- in special,
		//as there are no other nearby eggs that fit. It is likely
		//an error; I have assumed that they meant to delete the
		//nearby eggs with frame *6* and any quality -- which were
		//used to create the NPCs above anyway, and never get used
		//in the game again.
		//objpos = [AVATAR->get_object_position(), 0, 7];
		objpos = [0x41F, 0xA7C, 0x0, QUALITY_ANY, 6];
		eggs = objpos->find_nearby(SHAPE_EGG, 40, 16);
		for (egg in eggs with index to max)
			egg->remove_item();
		abort;
	}
	else
		MonitorBanquet.original();
}
