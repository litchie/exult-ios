FawnStorm 0x6BC ()
{
	var pathegg = getPathEgg(2, 1);
	var lute;
	
	if (event == SCRIPTED)
	{
		var quality = pathegg->get_item_quality();
		if (quality == 6)
		{
			AVATAR->set_item_flag(DONT_MOVE);
			lute = UI_create_new_object(SHAPE_LUTE);
			if (lute)
			{
				lute->set_item_frame(0);
				var pos = IOLO->get_object_position();
				pos[Y] = pos[Y] + 1;
				if (UI_update_last_created(pos))
				{
					pos[X] = pos[X] + (pos[Z] / 2);
					pos[Y] = pos[Y] + (pos[Z] / 2);
					UI_sprite_effect(21, pos[X], pos[Y], 0, 0, 0, -1);
					
					var dir = IOLO->find_direction(lute);
					script IOLO
					{	nohalt;					wait 1;
						face dir;				say "@Look, Avatar!@";
						wait 5;  				actor frame LEAN;
						wait 9;					call FawnStorm;}
					pathegg->set_item_quality(quality + 1);
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
			lute = IOLO->find_nearby(SHAPE_LUTE, 25, 0);
			if (lute)
			{
				lute->set_last_created();
				IOLO->give_last_created();
			}
			
			script IOLO
			{	nohalt;					actor frame STAND;}
			
			script pathegg after 3 ticks
			{	nohalt;					call FawnStorm;}
			
			pathegg->set_item_quality(quality + 1);
			AVATAR->clear_item_flag(DONT_MOVE);
			delayedBark(IOLO, "@A lute!@", 0);
			abort;
		}
	}
	FawnStorm.original();
}
