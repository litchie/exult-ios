/*
 *	This source file contains the code for the keyring item.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Keyring shape#(0x44C) ()
{
	var target;
	var target_quality;
	var keyfits;
	static var arraycreated;
	static var keys;
	
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