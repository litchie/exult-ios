eggBaneHolocaust 0x6B1 ()
{
	if (gflags[BANES_RELEASED])
	{
		var npcnum = -1 * UI_get_item_quality(item);
		var inn_keepers = [JENDON,
						   DEVRA, ARGUS,
						   ROCCO];
		var qualities = [11,
						 3, 3,
						 184];
		if (npcnum in inn_keepers)
		{
			var key = UI_create_new_object(SHAPE_KEY);
			UI_set_item_quality(key, qualities[getIndexForElement(npcnum, inn_keepers)]);
			UI_give_last_created(npcnum);
		}
		
		eggBaneHolocaust.original();
	}
}

