/*
 *	This source file contains usecode for several items that allow remote
 *	viewing. This is for multimap support, basically.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

// externs
extern startSpeech 0x940 (var var0000);

Crystal_Ball shape#(0x2D9) ()
{
	if (event == DOUBLECLICK)
	{
		UI_close_gumps();
		var qual = get_item_quality();
		var pos;
		if ((qual == 0) || (qual > 7))
		{
			startSpeech(0x19);
			return;
		}
		else if (qual == 1)
			if (gflags[0x157])
				pos = [0xA82, 0x970, 0x3, 0x0];
			else
				pos = [0xA82, 0x9A0, 0x3, 0x0];
		else if (qual == 2)
			pos = [0x38F, 0x47D, 0x3, 0x0];
		else if (qual == 3)
			pos = [0xA28, 0xA25, 0x3, 0x0];
		else if (qual == 4)
			pos = [0xAF7, 0xD7, 0x3, 0x0];
		else if (qual == 5)
			pos = [0x9F, 0x540, 0x3, 0x0];
		else if (qual == 6)
			pos = [0x609, 0x773, 0x3, 0x0];
		else if (qual == 7)
			pos = [0x928, 0x597, 0x3, 0x0];
		else if (qual == 255)
			return;
		UI_display_area(pos);
	}
}

Gem shape#(0x2F8) ()
{
	var framenum = get_item_frame();
	if (event == DOUBLECLICK)
	{
		if (framenum == 14)
		{
			if (!inIsleOfFire())
				UI_display_area([0x88F, 0x610, 0x6, 0x0]);
			else
			{
				remove_item();
				UI_play_sound_effect(SOUND_GLASS_SHATTER);
			}
		}
		else
			Gem.original();
	}
	else
		Gem.original();
}

extern setOrreryPosition 0x6E1 ();

Orrery_Viewer shape#(0x302) ()
{
	if ((event == DOUBLECLICK) && inMagicStorm())
	{
		UI_close_gumps();
		UI_center_view([0xB4C, 0x58C]);
		event = EGG;
		0->setOrreryPosition();
		UI_display_area([0xB4C, 0x58C, 0x3, 0x0]);
	}
}
