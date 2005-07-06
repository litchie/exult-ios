DeskItem shape#(0x2A3) ()
{
	if (event == DOUBLECLICK)
	{
		var framenum = UI_get_item_frame(item);
		if (framenum == 21)
		{

			var hour = UI_game_hour();
			var minute = UI_game_minute();
			if (minute <= 9)
				minute = ("0" + minute);

			var bark = " " + hour + ":" + minute;
			if (UI_in_gump_mode())
				UI_item_say(item, bark);

			else
			{
				bark = "@" + bark + "@";
				UI_item_say(0xFE9C, bark);
			}
			return;
		}
	}
	DeskItem.original();
}

Sundial shape#(0x11C) ()
{
	if (event == DOUBLECLICK)
	{
		var hour = UI_game_hour();
		if (hour == 12)
			item_say("Noon");
		else if ((hour >= 6) && (hour <= 20))
			item_say(" " + UI_game_hour() + " o'clock");
		else
			var bark = "@^<Avatar>, I believe the important part of the word sundial is `sun'.@";
			partyUtters(1, bark, bark, false);
	}
}
