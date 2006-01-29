/*
 *	This source file contains usecode for the new plaques.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Plaque shape#(0x334) ()
{
	var qual = get_item_quality();
	if (qual < 100)
		Plaque.original();
	else
	{
		var msg;
		if (qual == 100)
			msg = ["(e", "flame", "of", "tru("];
		else if (qual == 101)
			msg = ["(e", "flame", "of", "love"];
		else if (qual == 102)
			msg = ["(e", "flame", "of", "courage"];
		else if (qual == 103)
			msg = ["(e", "flame", "of", "infinity"];
		else if (qual == 104)
			msg = ["(e", "flame", "of", "si*ularity"];
		else if (qual == 105)
			msg = ["shrine", "of", "(e", "codex"];
		else if (qual == 106)
			msg = ["ch+ters", "never", "win"];
		else if (qual == 107)
			msg = ["(e", "book", "of", "tru("];
		else if (qual == 108)
			msg = ["(e", "candle", "of", "love"];
		else if (qual == 109)
			msg = ["(e", "bell", "of", "courage"];
		UI_display_runes(0x0033, msg);
	}
}