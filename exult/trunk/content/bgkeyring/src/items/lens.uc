/*
 *	This source file contains usecode for the Britannian and Gargoyle lenses.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Lens shape#(0x2D6) ()
{
	if (event == DOUBLECLICK)
	{
		//Get rotated frame #:
		var framenum = get_item_frame_rot();
		//Rotate by 90 degrees:
		set_item_frame_rot((framenum + 32) % 64);
		var pos = get_object_position();
		framenum = framenum % 2;
		if ((pos[X] == 0xA9C + (framenum * 7)) && (pos[Y] == 0xAE7) && (pos[Z] == 4))
		{
			//This was done in the correct place (in the Codex Shrine),
			//so find the right egg and activate it:
			var egg = pos->find_nearby(SHAPE_EGG, 1, MASK_EGG);
			event = EGG;
			egg->eggCodexLenses();
		}
	}
}