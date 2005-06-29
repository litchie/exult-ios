FawnStorm 0x6BC ()
{
	var pathegg = getPathEgg(2, 1);
	var lute;
	
	if (event == SCRIPTED)
	{
		var quality = UI_get_item_quality(pathegg);
		if (quality == 6)
		{
			UI_set_item_flag(AVATAR, DONT_MOVE);
			lute = UI_create_new_object(SHAPE_LUTE);
			if (lute)
			{
				UI_set_item_frame(lute, 0);
				var pos = UI_get_object_position(IOLO);
				pos[Y] = pos[Y] + 1;
				if (UI_update_last_created(pos))
				{
					pos[X] = pos[X] + (pos[Z] / 2);
					pos[Y] = pos[Y] + (pos[Z] / 2);
					UI_sprite_effect(21, pos[X], pos[Y], 0, 0, 0, -1);
					
					var dir = UI_find_direction(IOLO, lute);
					script IOLO
					{	nohalt;					wait 1;
						face dir;				say "@Look, Avatar!@";
						wait 5;  				actor frame LEAN;
						wait 9;					call FawnStorm;}
					UI_set_item_quality(pathegg, quality + 1);
					UI_set_weather(0);
				}
				else
				{
					script pathegg after 10 ticks
					{	nohalt;					call FawnStorm;}
				}
			}
			else
			{
				script pathegg after 10 ticks
				{	nohalt;					call FawnStorm;}
			}
			abort;
		}
		else if (quality == 7)
		{
			lute = UI_find_nearby(IOLO, SHAPE_LUTE, 25, 0);
			if (lute)
			{
				UI_set_last_created(lute);
				UI_give_last_created(IOLO);
				//UI_remove_item(lute);
				//giveItemsToPartyMember(IOLO, 1, SHAPE_LUTE, 0, 0, 0, false);
			}
			
			script IOLO
			{	nohalt;					actor frame STAND;}
			
			script pathegg after 3 ticks
			{	nohalt;					call FawnStorm;}
			
			UI_set_item_quality(pathegg, quality + 1);
			UI_clear_item_flag(AVATAR, DONT_MOVE);
			delayedBark(IOLO, "@A lute!@", 0);
			abort;
		}
	}
	FawnStorm.original();
}
