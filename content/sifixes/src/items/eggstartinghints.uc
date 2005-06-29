eggStartingHints 0x7AE ()
{
	if ((event == EGG) && (UI_get_item_quality(item) == 6))
		gflags[KNOWS_BEAR_SKULL_ORIGIN] = true;
	eggStartingHints.original();
}

