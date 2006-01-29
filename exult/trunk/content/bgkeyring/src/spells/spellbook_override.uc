/*
 *	This header file overrides the spellbook's spells 'Help' (for multimap
 *	support) and 'Mark' (to prevent marking a virtue stone inside the shrine
 *	of the Codex)
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

spellHelp_Override				0x645 ()
{
	if (event == DOUBLECLICK)
		//If we came here from a doubleclick, forward to original:
		spellHelp_Override.original();
	else if (event == SCRIPTED)
	{
		//We got here from a Usecode script, so we override it
		//for multi-map support:
		var pos = [0x03A8, 0x047A, 0x0, 0x0];
		PARTY->move_object(pos);
	}
}

spellMark_Override				0x662 ()
{
	var target;
	var pos;
	var in_codex_shrine;

	if (event == DOUBLECLICK)
	{
		//Stop Avatar's usecode:
		halt_scheduled();
		//Prompt for a target:
		target = UI_click_on_item();
		//For Codex Quest: of the Virtue Stone of Spirituality has been
		//reattuned to the Shrine of Spirituality, clear the flag if the
		//player decides to cast Mark on it. This allows the Codex to restore
		//the stone once again if needed.
		if ((target->get_item_frame() == 7) && gflags[ATTUNED_SPIRITUALITY_STONE])
			gflags[ATTUNED_SPIRITUALITY_STONE] = false;
		
		//Get the stone's position:
		pos = target->get_object_position();

		//Check to see if the STONE is inside the area of the Shrine of the Codex: 
		if (!UI_is_pc_inside() && (pos[X] >= 0xA50) && (pos[Y] >= 0xABC) && (pos[X] <= 0xAE0) && (pos[Y] <= 0xB1D))
			in_codex_shrine = true;
		if ((pos[X] >= 0xA80) && (pos[Y] >= 0xADC) && (pos[X] <= 0xABB) && (pos[Y] <= 0xB0D))
			in_codex_shrine = true;

		if (!in_codex_shrine)
		{
			//The stone is not inside the area guarded by the Guardians;
			//forward to original usecode:
			//Whoever invented this intrinsic deserves commendation:
			target->set_intercept_item();
			spellMark_Override.original();
		}
		else
		{
			//The stone is in the forbidden zone, so the spell fails:
			item_say("@Kal Por Ylem@");
			script item
			{	actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}
