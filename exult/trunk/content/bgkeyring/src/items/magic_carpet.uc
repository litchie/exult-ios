/*
 *	This source file contains usecode to prevent the Magic Carpet from
 *	landing in the area of the Shrine of the Codex.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Magic_Carpet shape#(0x348) ()
{
	var barge = get_barge();
	if ((event == DOUBLECLICK) && barge)
	{
		var pos = AVATAR->get_object_position();
		if ((pos[X] >= 0xA50) && (pos[Y] >= 0xABC) && (pos[X] <= 0xAE0) && (pos[Y] <= 0xB2D))
			//Avatar is over the area of the Shrine of the Codex; prevent landing
			avatarSpeak("There is a strange force preventing you from landing in this area.");
		else
			//Forward to original:
			Magic_Carpet.original();
	}
}
