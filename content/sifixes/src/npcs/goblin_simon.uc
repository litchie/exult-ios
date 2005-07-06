GoblinSimon 0x442 ()
{
	if ((event == DEATH) && (!G_SIMON->get_item_flag(0x001C)))
	{
		var key = UI_create_new_object(SHAPE_KEY);
		key->set_item_quality(74);
		G_SIMON->give_last_created();
	}
	
	GoblinSimon.original();
}
