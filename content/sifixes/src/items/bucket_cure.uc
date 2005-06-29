/*
 *  This file has been created from usecode found in the Exult CVS snapshot.
 *  I include it here only for convenience; I have edited it to fit the
 *  conventions used in the rest of the mod.
 *
 *	The original code was written by Jeff Freedman (aka "DrCode").
 *
 *	I modified the code so that the companions will have time to join
 *	before Xenka returns, as well as having a better post-resurrection,
 *	post-insanity dialog.
 *
 */

DumpBucket 0x88A ()
{
	var target = UI_click_on_item();
	if (!target)
		return;
		
	UI_printf(["The shape clicked on is %s", UI_get_item_shape(target)]);
	
	var npcnum = UI_get_npc_number(target[1]);		// It's item, position.
	var bucket_quality = UI_get_item_quality(item);
	var handled = false;
	
	if (UI_get_item_frame(item) == 1)
	{
		if ((npcnum == CANTRA) &&  (bucket_quality == QUALITY_LOGIC))
		{
			UI_set_schedule_type(CANTRA, WAIT);
			//Without UI_set_path_failure, Cantra's would be stuck in waiting mode if
			//the player moved the Avatar before reaching Cantra:
			if (UI_path_run_usecode([target[2], target[3], target[4]], CureCantra, item, PATH_SUCCESS))
				UI_set_path_failure(CureCantra, item, PATH_FAILURE);
			abort;
		}
		else if (((npcnum == DUPRE)		&& (bucket_quality == QUALITY_DISCIPLINE)) ||
				 ((npcnum == SHAMINO)	&& (bucket_quality == QUALITY_ETHICALITY)) ||
				 ((npcnum == IOLO)		&& (bucket_quality == QUALITY_LOGIC)))
		{
			UI_set_schedule_type(npcnum, WAIT);
			
			if (UI_path_run_usecode([target[2], target[3], target[4]], CureCompanion, item, PATH_SUCCESS))
				UI_set_path_failure(CureCompanion, item, PATH_FAILURE);
			abort;
		}
	}

	// Let orig. function handle it.
	UI_set_intercept_item(target);
	DumpBucket.original();
}
