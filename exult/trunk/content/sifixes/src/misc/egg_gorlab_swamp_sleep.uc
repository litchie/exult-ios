eggGorlabSwampSleep 0x6B3 ()
{
	eggGorlabSwampSleep.original();
	if ((getAvatarLocationID() == DREAM_WORLD) && AVATAR->get_item_flag(POLYMORPH))
		AVATAR->set_polymorph(SHAPE_MALE_AVATAR);
}
