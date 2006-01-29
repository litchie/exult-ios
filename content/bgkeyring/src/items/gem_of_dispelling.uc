/*
 *	This source file contains the usecode for Gems of Dispelling.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Gem_Of_Dispelling shape#(0x451) ()
{
	if (event != DOUBLECLICK) return;

	//Get gem's quality:
	var gem_quality = get_item_quality();
	//Get quest state:
	var quest_state = getQuestState();
	if ((quest_state == PLAYER_HAS_GEM) && (gem_quality == 0))
	{
		//If the gem is good, and Zauriel has fixed/made/told so to the player,
		if (AVATAR->get_distance(LAURIANNA) <= 25)
		{
			//and if the Avatar is close enough to Laurianna,
			//close all gumps:
			UI_close_gumps();
			//Description of action:
			avatarSpeak("You pick up the gem with your hand and firmly crush it -- although, to your surprise, the gem is far easier to crush than you had anticipated.");
			avatarSpeak("You wait for the spell to take effect as the magical energies course through the Ether.");
	
			//Give the amulet to Laurianna:
			var amulet = UI_create_new_object(SHAPE_AMULET);
			LAURIANNA->give_last_created();
			//Make Laurianna have roots:
			var pos = LAURIANNA->get_object_position();
			LAURIANNA->set_item_shape(SHAPE_LAURIANNA_ROOTED);
			LAURIANNA->set_last_created();
			UI_update_last_created(pos);
			
			//Create Laundo and his goons:
			createMageAndGoons();
			
			//Done here since the mage & goons must be invisible
			//at the start of the cutscene:
			gflags[PLAYER_USED_GEM] = true;
			
			//Remove the gem:
			remove_item();
			//Crushing animation and start of cutscene:
			script AVATAR
			{
				nohalt;						finish;
				wait 1;						actor frame STAND;
				wait 2;						actor frame SWING_2H_1;
				sfx SOUND_GLASS_SHATTER;
				wait 2;						actor frame SWING_2H_3;
				wait 2;						actor frame SWING_2H_1;
				wait 1;						actor frame STAND;
				call beginCutsceneMageAndGoons, BEGIN_CUTSCENE;
			}
		}
		else
			//Reproach Avatar for trying to waste the gem:
			randomPartySay("@Gems like this are -very- rare; we should not waste it without being sure!@");
	}
	
	else if (quest_state == TOLD_ABOUT_GEM)
		//Otherwise, warn player to seek Zauriel:
		randomPartySay("@These should be the gems mentioned by Zauriel. We should take them to him quickly!@");
}
