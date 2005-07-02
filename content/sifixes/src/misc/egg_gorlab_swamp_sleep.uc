eggGorlabSwampSleep 0x6B3 ()
{
	eggGorlabSwampSleep.original();
	if ((getAvatarLocationID() == DREAM_WORLD) && UI_get_item_flag(AVATAR, POLYMORPH))
		UI_set_polymorph(UI_get_npc_object(AVATAR), SHAPE_MALE_AVATAR);
}
