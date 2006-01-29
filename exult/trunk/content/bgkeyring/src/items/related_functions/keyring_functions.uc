/*
 *	This source file contains the additional code for keys and the
 *	keyring item.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

//Constants
const int KEY_INN					= 255;	//0x0FF, keys for inn doors.
const int KEY_ALAGNER				= 254;	//0x0FE, keys for inn doors.

const int KEY_CHRISTOPHERS_CHEST	= 253;	//Key for Christopher's chest

UseKeyOnChest (var chest)
{
	var item_shape;
	var item_quality;
	var target_quality;
	var msg;

	var cont;
	var cont_coll;
	var cont_count;
	var for_counter;
	var gump_objs = [761, 799, 801, 802, 803];
	var gump;
	var container_count;
	var loop_counter;

	item_quality = get_item_quality();
	target_quality = chest->get_item_quality();

	//key was used in a locked chest:
	if (chest->get_item_shape() == SHAPE_LOCKED_CHEST)
	{
		chest->set_item_shape(SHAPE_CHEST);
		chest->item_say("Unlocked");

		if (target_quality == KEY_CHRISTOPHERS_CHEST) gflags[UNLOCKED_CHRISTOPHERS_CHEST] = true;
	}
	
	//key was used in an unlocked chest:
	else
	{
		//Check to see if the key is inside the chest:
		if (containedBy(item, chest))
		{
			//To differentiate between a key and a keyring:
			item_shape = get_item_shape();
	
			//Complain that the key is inside the chest:
			if (item_shape == SHAPE_KEYRING) msg = "The keyring";
			else if (item_shape == SHAPE_KEY) msg = "The key";
			AVATAR->item_say(msg + " is inside the chest!");
		}
		else
		{
			//Lock the chest:

			//Closes the chest's gumps, if open, and those of all contained containers:
			chest->close_gump();
			for (gump in gump_objs with loop_counter to container_count)
			{
				cont_coll = chest->get_cont_items(gump, QUALITY_ANY, FRAME_ANY);
				for (cont in cont_coll with for_counter to cont_count)
					cont->close_gump();
			}
			chest->set_item_shape(SHAPE_LOCKED_CHEST);
			chest->item_say("Locked");
		}
	}
}

KeyInternal (var target, var keyfits, var barks)
{
	var lockables;
	var target_shape;

	lockables = [
				 SHAPE_LOCKED_CHEST,
				 SHAPE_CHEST,
				 SHAPE_DOOR_HORIZONTAL,
				 SHAPE_DOOR_VERTICAL,
				 SHAPE_DOOR2_HORIZONTAL,
				 SHAPE_DOOR2_VERTICAL];

	target_shape = target->get_item_shape();

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
	var party;
	var party_count;
	var loop_counter;
	var key;
	var key_count;
	var for_counter;
	var key_coll;
	var party_key_count;
	var quality;

	//Get party list and size:
	party = UI_get_party_list();
	party_count = UI_get_array_size(party);
	
	event = SCRIPTED;

	loop_counter = 0;
	//Count party keys:
	party_key_count = PARTY->count_objects(SHAPE_KEY, QUALITY_ANY, FRAME_ANY);
	while (loop_counter < party_count)
	{
		//For each party member,
		loop_counter = loop_counter + 1;
		//Get contained keys
		key_coll = party[loop_counter]->get_cont_items(SHAPE_KEY, QUALITY_ANY, FRAME_ANY);
		for (key in key_coll with for_counter to key_count)
		{
			//For each key found, get key quality:
			quality = key->get_item_quality();
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
