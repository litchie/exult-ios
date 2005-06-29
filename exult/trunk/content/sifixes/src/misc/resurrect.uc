Resurrect 0x8FE ()
{
	var resurrectlist;
	var resurrectables;
	var resurrectablenames;
	var index;
	var max;
	var npc;
	var body;
	var itemindex;
	var name;
	var pos;
	var foodlevel;

	resurrectlist = getNearbyBodies();
	
	resurrectables = getJoinableNPCNumbers();
	resurrectables = [resurrectables, CANTRA];
	
	resurrectablenames = getJoinableNPCNames();
	resurrectablenames = [resurrectablenames, "Cantra"];
	
	var msg = "son";
	if (UI_is_pc_female())
		msg = "daughter";
	
	for (body in resurrectlist with index to max)
	{
		if (body != UI_get_npc_object(AVATAR))
		{
			itemindex = getIndexForElement(npc, resurrectables);
			name = resurrectablenames[itemindex];
			var flag_dont_resurrect = false;
			npc = UI_get_body_npc(body);
			if (gflags[BANES_RELEASED])
			{
				//Prevent "resurrection" of the three Banes:
				if ((npc == DUPRE) && !gflags[WANTONESS_BANE_DEAD])
					flag_dont_resurrect = true;
				else if ((npc == IOLO) && !gflags[INSANITY_BANE_DEAD])
					flag_dont_resurrect = true;
				else if ((npc == SHAMINO) && !gflags[ANARCHY_BANE_DEAD])
					flag_dont_resurrect = true;
				
				if (flag_dont_resurrect)
					say("@I am sorry, my "+ msg +", but " + name + "'s body vanished when I tried to raise it! It seems that it was but a cruel of illusion...@");
			}
			
			if (!flag_dont_resurrect && (npc == DUPRE) && gflags[DUPRE_IS_TOAST] && gflags[WANTONESS_BANE_DEAD])
			{
				//Prevent resurrection of Dupre after he burns in the Crematorium:
				flag_dont_resurrect = true;
				say("@I am sorry, my " + msg + ", but I cannot -- this isn't Dupre's body, but merely shadow of him. See how it diappears when I try to rause it...@");
			}
			
			if (!flag_dont_resurrect && !UI_is_dead(npc))
			{
				//Prevent resurrection of a live NPC:
				flag_dont_resurrect = true;
				say("@I know not who this is, but it isn't thy friend -- he is still alive somewhere. The similarity is remarkable, though... But look! The body disapeared! I wonder what has happened to it?@");
			}
			
			if (flag_dont_resurrect)
				UI_remove_item(body);

			else
			{
				itemindex = getIndexForElement(npc, resurrectables);
				name = resurrectablenames[itemindex];
				pos = UI_get_object_position(npc);
				
				if (UI_resurrect(body))
				{
					say("@Now thy friend " + name + " doth live again.@");
					if (UI_get_item_flag(npc, IN_PARTY) && UI_get_item_flag(npc, SI_ZOMBIE))
						UI_remove_from_party(npc);
	
					UI_set_new_schedules(npc, MIDNIGHT, WAIT, [pos[X], pos[Y]]);
					UI_run_schedule(npc);
					UI_set_schedule_type(npc, WAIT);
	
					foodlevel = getPropValue(npc, FOODLEVEL);
					foodlevel = (31 - foodlevel);
					setPropValue(npc, FOODLEVEL, foodlevel);
					
					if (npc == GWENNO)
						gflags[GWENNO_IS_DEAD] = false;
				}
				else
					say("@Thy friend " + name + " hath been lost forever.@");
			}
		}
	}
}
