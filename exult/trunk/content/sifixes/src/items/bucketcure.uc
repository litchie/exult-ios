/*
 *  This file has been created from usecode found in the Exult CVS snapshot.
 *  I include it here only for convenience; I have edited it to fit the
 *  conventions used in the rest of the mod.
 *
 *	The original code was written by Jeff Freedman (aka "DrCode").
 */

extern CureCantra 0xCA3 ();

DumpBucket 0x88A ()
{
	var target = UI_click_on_item();
	if (!target)
		return;
		
	UI_printf(["The shape clicked on is %s", UI_get_item_shape(target)]);
	
	var npc = target[1];		// It's item, position.
	if (npc != UI_get_npc_object(CANTRA) ||
		    UI_get_item_quality(item) != 13 ||	// "Logic".
	    	UI_get_item_frame(item) >= 6)
	{
		// Let orig. function handle it.
		UI_set_intercept_item(target);
		DumpBucket.original();
	}
	else
	{
		UI_set_schedule_type(CANTRA, WAIT);
		//Without UI_set_path_failure, Cantra's would be stuck in waiting mode if
		//the player moved the Avatar before reaching Cantra:
		if (UI_path_run_usecode([target[2], target[3], target[4]], CureCantra, item, PATH_SUCCESS))
			UI_set_path_failure(CureCantra, item, PATH_FAILURE);
	}
}

CureCantra 0xCA3 ()
{
	if (event == PATH_SUCCESS)
	{
		UI_close_gumps();
		UI_set_item_frame(item, 0);	// Now empty.
	
		UI_obj_sprite_effect(CANTRA, ANIMATION_TELEPORT, 0, 0, 0, 0, 0, 0);
		UI_clear_item_flag(CANTRA, SI_ZOMBIE);	// No longer crazy.
		UI_set_schedule_type(CANTRA, TALK);
		gflags[CURED_CANTRA] = true;			// We've done it.
	}
	
	else if (event == PATH_FAILURE)
		UI_set_schedule_type(CANTRA, WANDER);
}
