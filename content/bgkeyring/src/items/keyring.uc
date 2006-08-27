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
 *	This source file contains the code for the keyring item.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

//This stores the keys in the keyring:
static var arraycreated;
static var keys;

//Constants
const int KEY_ALAGNER				= 254;	//0x0FE, keys for inn doors.

const int KEY_CHRISTOPHERS_CHEST	= 253;	//Key for Christopher's chest

UseKeyOnChest (var chest)
{
	//key was used in a locked chest:
	if (chest->get_item_shape() == SHAPE_LOCKED_CHEST)
	{
		chest->set_item_shape(SHAPE_CHEST);
		chest->item_say("Unlocked");
	
		if (chest->get_item_quality() == KEY_CHRISTOPHERS_CHEST) gflags[UNLOCKED_CHRISTOPHERS_CHEST] = true;
	}
	
	//key was used in an unlocked chest:
	else
	{
		//Check to see if the key is inside the chest:
		if (containedBy(item, chest))
		{
			//To differentiate between a key and a keyring:
			var item_shape = get_item_shape();
			
			//Complain that the key is inside the chest:
			var msg;
			if (item_shape == SHAPE_KEYRING)
				msg = "The keyring";
			else if (item_shape == SHAPE_KEY)
				msg = "The key";
			AVATAR->item_say(msg + " is inside the chest!");
		}
		else
		{
			//Lock the chest:
			
			//Closes the chest's gumps, if open, and those of all contained containers:
			chest->close_gump();
			var gump_objs = [761, 799, 801, 802, 803];
			for (gump in gump_objs)
			{
				var cont_coll = chest->get_cont_items(gump, QUALITY_ANY, FRAME_ANY);
				for (cont in cont_coll)
					cont->close_gump();
			}
			chest->set_item_shape(SHAPE_LOCKED_CHEST);
			chest->item_say("Locked");
		}
	}
}

KeyInternal (var target, var keyfits, var barks)
{
	var lockables = [
				 SHAPE_LOCKED_CHEST,
				 SHAPE_CHEST,
				 SHAPE_DOOR_HORIZONTAL,
				 SHAPE_DOOR_VERTICAL,
				 SHAPE_DOOR2_HORIZONTAL,
				 SHAPE_DOOR2_VERTICAL];
	
	var target_shape = target->get_item_shape();
	
	//The target cannot be unlocked:
	if (!(target_shape in lockables)) return;
	
	//Key fits whatever it is being used on:
	if (keyfits) 
	{
		//Play the key sound:
		UI_play_sound_effect2(SOUND_KEY, item);
		
		//Key was used in a door:
		if (target_shape in [SHAPE_DOOR_HORIZONTAL, SHAPE_DOOR_VERTICAL, SHAPE_DOOR2_HORIZONTAL, SHAPE_DOOR2_VERTICAL])
			UseKeyOnDoor(target);
		
		//Key was used in a chest:
		else  UseKeyOnChest(target);
	}
	
	//key does not fit target:
	else
	{
		flashBlocked(0);
		randomPartyBark(barks[UI_get_random(UI_get_array_size(barks))]);
	}
}

AddPartyKeysToKeyring ()
{
	event = SCRIPTED;
	
	//Count party keys:
	var party_key_count = PARTY->count_objects(SHAPE_KEY, QUALITY_ANY, FRAME_ANY);
	var party = UI_get_party_list();
	for (npc in party)
	{
		//For each party member, get contained keys
		var key_coll = npc->get_cont_items(SHAPE_KEY, QUALITY_ANY, FRAME_ANY);
		for (key in key_coll)
		{
			//For each key found, get key quality:
			var quality = key->get_item_quality();
			if ((quality != KEY_INN) && (quality != KEY_ALAGNER))
			{
				//Do not add inn keys or Alagner's key!
				//Add key to keyring:
				key->Keyring();
				//Remove key:
				key->remove_item();
			}
		}
	}
	
	//See how many keys were added to the keyring:
	party_key_count = party_key_count - PARTY->count_objects(SHAPE_KEY, QUALITY_ANY, FRAME_ANY);
	
	//Set the frame appropriatelly:
	if ((party_key_count >= 4) && !(get_item_frame() == 4))
		set_item_frame(4);
	else if (party_key_count <= 3)
		set_item_frame(party_key_count);
	
	//Have someone say how many keys were added:
	randomPartyBark("@" + party_key_count + " keys have been added to the keyring@");
}

Keyring shape#(0x44C) ()
{
	var target;
	var target_quality;
	var keyfits;
	
	//We arrived here from Key function: a key was double-clicked and the
	//target was the keyring; the item var is the key, not the keyring.
	//Check the shape of item just in case:
	if ((event == SCRIPTED) && (get_item_shape() == SHAPE_KEY))
	{
		//Get key quality:
		target_quality = get_item_quality();
		
		//Check to see if the key array has been initialized yet:
		if (!(arraycreated))
		{
			//Initialize array with current key and set flag:
			arraycreated = true;
			keys = [target_quality];
		}
		
		//array has been initialized, so add key if it is not there already:
		else if (!(target_quality in keys)) keys = keys & target_quality;
		
		//Hack to display new frame:
		target_quality = UI_get_array_size(keys);
		if (target_quality > 4) target_quality = 4;
		set_item_quality(target_quality);
	
		//Has been moved to the Key function, to display new keyring frame:
		////Delete the key (it has been added to the keyring after all...):
		//UI_remove_item(item);
		return;
	}
	
	//If we did not arive here due to a double-click, leave
	else if (!(event == DOUBLECLICK)) return;
	
	//Prompt user for target:
	target = UI_click_on_item();
	
	//Sunce BG has no native support for adding keys to the keyring when you
	//add the key to the container, we include this here: using the keyring
	//on the avatar adds all keys to the keyring.
	if (isAvatar(target))
	{
		item->AddPartyKeysToKeyring();
		return;
	}
	
	//Get target quality:
	target_quality = target->get_item_quality();
	
	//Ensure that the keyring will work:
	set_item_quality(target_quality);
	
	//See if the keyring has a key fits the "lock":
	if (!(arraycreated)) keyfits = false;
	else keyfits = (target_quality in keys);
	
	KeyInternal(target,
				keyfits,
				["@We don't have that key.@", "@It's not on the keyring.@"]);
}
