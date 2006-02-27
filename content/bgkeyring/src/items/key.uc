/*
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *	This source file contains the code for normal keys for compatibility
 *	with the keyring.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

Key shape#(0x281) ()
{
	var target;
	var target_quality;

	//If we didn't came here via double-click, leave:
	if (!(event == DOUBLECLICK)) return;
	
	//Prompt player for target:
	target = UI_click_on_item();
	
	//Target is the keyring:
	if (target->get_item_shape() == SHAPE_KEYRING)
	{
		target_quality = get_item_quality();
		
		//Refuse Inn keys:
		if (target_quality == KEY_INN)
		{
			avatarSay("@I don't think that the innkeeper would like that...@");
			return;
		}
		
		//Refuse Alagner's storeroom key:
		else if (target_quality == KEY_ALAGNER)
		{
			avatarSay("@I am supposed to return this key to Alagner.@");
			return;
		}	
		
		//Play the key sound:
		UI_play_sound_effect2(SOUND_KEY, item);

		//Call standard keyring function using the key as "keyring"
		event = SCRIPTED;
		//Keyring();
		item->Keyring();
		
		//The keyring function stored the new frame number of the keyring
		//as the item's quality. This is done so we can display the correct
		//frame for the keyring; it won't be a problem because the key
		//has already been added to the keyring and will be deleted soon.

		//Display new graphic for keyring:
		var new_frame = get_item_quality();
		if (new_frame > 4) new_frame = 4;
		if (!(target->get_item_frame() == 4)) target->set_item_frame(new_frame);

		//Delete the key (it has been added to the keyring after all...):
		remove_item();

		return;
	}
	
	KeyInternal(target,
				(get_item_quality() == target->get_item_quality()),
				["@The key doesn't fit.@", "@Maybe it is another key.@"]);
}
