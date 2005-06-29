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
	
	if ((event == SCRIPTED) && (UI_get_npc_id(MARSTEN) == 0))
	{
		UI_play_music(15, getPathEgg(5, 1));
		UI_set_item_flag(AVATAR, DONT_MOVE);
		var health = UI_get_npc_prop(SHAZZANA, HEALTH);
		UI_set_npc_prop(SHAZZANA, HEALTH, (UI_get_npc_prop(SHAZZANA, STRENGTH) - health));
		health = UI_get_npc_prop(LUTHER, HEALTH);
		UI_set_npc_prop(LUTHER, HEALTH, (UI_get_npc_prop(SHAZZANA, STRENGTH) - health));

		npc = partyUtters(1, 0, 0, true);
		
		var partytoken = "We";
		if (UI_get_array_size(UI_get_party_list()) < 3)
			partytoken = "I";

		delayedBark(npc, "@" + partytoken + " shall wait for thee here!@", 5);
		
		var partymembers = removeFromArray(UI_get_npc_object(AVATAR), UI_get_party_list2());
		var pos;
		for (npc in partymembers with index to max)
		{
			UI_remove_from_party(npc);
			pos = UI_get_object_position(npc);
			UI_set_new_schedules(npc, MIDNIGHT, STANDTHERE, [pos[X], pos[Y]]);
			UI_run_schedule(npc);
		}
		
		UI_item_say(AVATAR, "@This must be the place!@");
		UI_si_path_run_usecode(AVATAR, [0x41F, 0xA7C, 0x0], PATH_SUCCESS, item, MonitorBanquet, true);
		UI_set_path_failure(MonitorBanquet, item, PATH_FAILURE);
		gflags[MONITOR_BANQUET_STARTED] = true;
		//In the original, the avatar's position is used instead of
		//a hardcoded one. I preferred the hardcoded one -- this
		//allows higher resolutions to work if the player is too far
		//away when double-clicking the doors.
		//var objpos = [UI_get_object_position(AVATAR), QUALITY_ANY, 6];
		var objpos = [0x41F, 0xA7C, 0x0, QUALITY_ANY, 6];
		var eggs = UI_find_nearby(objpos, SHAPE_EGG, 40, 16);
		var egg;
		var dir;
		var eggquality;
		var directions = [4,	   4,		0, 4,		 4,		  2,
						  6, 	   2,	   6,		2,	   6,	   0];
		var npcids =	 [MARSTEN, SPEKTOR, 0, SHAZZANA, FLICKEN, BRENDANN,
						  CALADIN, CELLIA, TEMPLAR, KRAYG, LUTHER, 0];
		for (egg in eggs with index to max)
		{
			pos = UI_get_object_position(egg);
			eggquality = UI_get_item_quality(egg);
			npc = false;
			if (eggquality == 12)
			{
				UI_move_object(LUCILLA, pos);
				UI_set_schedule_type(LUCILLA, WAIT);
				script LUCILLA
				{	face WEST;					actor frame STAND;}
			}
			else
			{
				dir = directions[eggquality];
				npc = npcids[eggquality];
				if (npc && (!UI_get_item_flag(npc, DEAD)))
				{
					UI_move_object(npc, pos);
					UI_set_schedule_type(npc, WAIT);
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
		//objpos = [UI_get_object_position(AVATAR), 0, 7];
		objpos = [0x41F, 0xA7C, 0x0, QUALITY_ANY, 6];
		eggs = UI_find_nearby(objpos, SHAPE_EGG, 40, 16);
		for (egg in eggs with index to max)
			UI_remove_item(egg);
		abort;
	}
	else
		MonitorBanquet.original();
}
