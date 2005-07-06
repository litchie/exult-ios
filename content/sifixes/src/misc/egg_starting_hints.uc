eggStartingHints 0x7AE ()
{
	if ((event == EGG) && (get_item_quality() == 6))
		gflags[KNOWS_BEAR_SKULL_ORIGIN] = true;
	eggStartingHints.original();
}

