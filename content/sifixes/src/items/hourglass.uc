//Reimplemented so that Thoxa no longer gets summoned in Spinebreaker Mountains
//unless there is a corpse there. Selina's body is there, so this isn't much of
//an issue...

Hourglass shape#(0x347) ()
{
	var pos;
	var notincombat;
	var nearbynpcs;
	var index;
	var max;
	var npc;
	var pathegg;

	//Hourglass of the Timelord...errr, Fate
	if (event == DOUBLECLICK)
	{
		if (get_item_frame() == 1)
		{
			pos = get_object_position();
			//Check to see if the banes have been released. If they have, do not
			//force summon Thoxa, but summon her in the normal way
			if (pointInsideRect(pos, [0x910, 0x170], [0x93F, 0x19F]) && !gflags[BANES_RELEASED])
			{
				UI_close_gumps();
				UI_play_sound_effect(0x30);
				obj_sprite_effect(9, 0, 0, 0, 0, 0, -1);
				abort;
			}
			
			notincombat = true;
			nearbynpcs = AVATAR->find_nearby(SHAPE_ANY, 10, 4);
			for (npc in nearbynpcs with index to max)
			{
				if (npc->get_schedule_type() == IN_COMBAT)
				{
					notincombat = false;
					break;
				}
			}
			
			if (notincombat)
			{
				if (areThereBodiesNearby())
				{
					pos = AVATAR->get_object_position();
					//Offset Thoxa a bit so that she isn't INSIDE avatar:
					var framerot = AVATAR->get_item_frame_rot();
					if (framerot - EAST_FRAMESET >= 0)
						pos[X] = pos[X] + 5;
					else if (framerot - WEST_FRAMESET >= 0)
						pos[X] = pos[X] - 5;
					else if (framerot - SOUTH_FRAMESET >= 0)
						pos[Y] = pos[Y] + 5;
					else
						pos[Y] = pos[Y] - 5;
					
					THOXA->move_object(pos);
					THOXA->faceAvatar();
					THOXA->set_schedule_type(TALK);
					THOXA->obj_sprite_effect(7, 0, 0, 0, 0, 0, -1);
					UI_play_sound_effect(0x0051);
					
					pathegg = getPathEgg(2, 1);
					if (pathegg)
						UI_play_music(0x3F, pathegg);

					UI_close_gumps();
				}
				else
				{
					UI_play_sound_effect(0xE);
					obj_sprite_effect(13, 0, 0, 0, 0, 0, -1);
				}
			}
			else
			{
				UI_play_sound_effect(0x30);
				obj_sprite_effect(9, 0, 0, 0, 0, 0, -1);
			}
		}
	}
}
