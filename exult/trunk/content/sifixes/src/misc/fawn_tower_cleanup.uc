mightyGoblinDies 0x9B7 ()
{
	mightyGoblinDies.original();
	if (gflags[CLEARED_FAWN_TOWER])
	{
		var objpos = [0x3E3, 0x821, 0x0, QUALITY_ANY, 0];
		var goblineggs = objpos->find_nearby(SHAPE_EGG, 50, 16);
		var index;
		var max;
		var egg;
		for (egg in goblineggs with index to max)
			script egg setegg EXTERNAL_CRITERIA, 1;
	}
}

eggFawnTowerCleanup 0x72D ()
{
	if ((event == EGG) && gflags[CLEARED_FAWN_TOWER])
	{
		var objpos = [get_object_position(), QUALITY_ANY, 0];
		var goblineggs = objpos->find_nearby(SHAPE_EGG, 50, 16);
		var index;
		var max;
		
		if (!gflags[BANES_RELEASED])
		{
			var dishes = find_nearby(SHAPE_BROKEN_DISH, 50, 0);
			var dish;
			for (dish in dishes with index to max)
				dish->remove_item();
			
			var egg;
			for (egg in goblineggs with index to max)
				script egg setegg EXTERNAL_CRITERIA, 1;
		}
		else
		{
			for (egg in goblineggs with index to max)
				script egg
				{	setegg CACHED_IN, 1;	hatch;}
		}
		eggFawnTowerCleanup.original();
	}
}
