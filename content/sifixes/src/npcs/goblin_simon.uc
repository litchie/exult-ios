GoblinSimon 0x442 ()
{
	if ((event == 0x0007) && (!UI_get_item_flag(0xFFBE, 0x001C)))
	{
		var key = UI_create_new_object(SHAPE_KEY);
		UI_set_item_quality(key, 74);
		UI_give_last_created(G_SIMON);
	}
	
	GoblinSimon.original();
}
