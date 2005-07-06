eggBaneHolocaust 0x6B1 ()
{
	if (gflags[BANES_RELEASED])
	{
		var npcnum = -1 * get_item_quality();
		var inn_keepers = [JENDON,
						   DEVRA, ARGUS,
						   ROCCO];
		var qualities = [11,
						 3, 3,
						 184];
		if (npcnum in inn_keepers)
		{
			var key = UI_create_new_object(SHAPE_KEY);
			key->set_item_quality(qualities[getIndexForElement(npcnum, inn_keepers)]);
			npcnum->give_last_created();
		}
		
		eggBaneHolocaust.original();
	}
}

