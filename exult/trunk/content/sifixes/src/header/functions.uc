freeze 0xC00 (){UI_set_item_flag(item, DONT_MOVE);}
unfreeze (){UI_clear_item_flag(item, DONT_MOVE);}

addShaminoToParty ()
{
	UI_add_to_party(SHAMINO);
	gflags[SHAMINO_HAS_BELONGINGS] = true;
	UI_set_npc_id(SHAMINO, 0);
	
	script getPathEgg(5, 4) after 10 ticks
	{
		nohalt;
		call startSerpentSpeechViaRing;
	}
	delayedBark(SHAMINO, "@Welcome back!@", 0);
	abort;
}

//Unlike the other two companions, Iolo didn't have one for himself:
askIoloBelongings ()
{
	if (UI_get_cont_items(IOLO, SHAPE_ANY, QUALITY_ANY, FRAME_ANY))
	{
		say("@I am carrying many items, some of which may be of use to thee. Wouldst thou care to have these?@");
		if (askYesNo())
		{
			say("@Here they are.@");
			gflags[IOLO_HAS_BELONGINGS] = false;
	
			var iolo_items = UI_get_cont_items(IOLO, SHAPE_ANY, QUALITY_ANY, FRAME_ANY);
			var obj;
			var index;
			var max;
			var give_return;
			var cumulative_result = [false, 0];
			
			for (obj in iolo_items with index to max)
			{
				if (!((UI_get_item_shape(obj) == SHAPE_PLAINRING) && (UI_get_item_frame(obj) == 2)))
				{
					give_return = giveItemsToPartyMember(AVATAR, UI_get_item_quantity(obj, 0), UI_get_item_shape(obj), UI_get_item_quality(obj), UI_get_item_frame(obj), UI_get_item_flag(obj, TEMPORARY), false);
					if (cumulative_result[1] == 0)
						cumulative_result[1] = give_return[1];
	
					cumulative_result[2] = (cumulative_result[2] + give_return[getArraySize(give_return)]);
					UI_remove_item(obj);
				}
			}
	
			if (cumulative_result[1] != 0)
				say("@Thy friends will have to help carry these things.@");
	
			if (cumulative_result[2] > 0)
			{
				give_return = cumulative_result[2];
				if (give_return > 1)
					say("@Since thou canst not carry these remaining " + give_return + " items, I will place them at thy feet.@");
	
				else
					say("@Since thou dost not have enough room for this last item, I will place it at thy feet.@");
			}
		}
		else
		{
			gflags[IOLO_HAS_BELONGINGS] = true;
			say("@If thou changest thy mind, thou hast but to return and ask again.@");
		}
	}
}

setFurcapFlag ()
{
	//We should only be here if the Avatar knew the furcap was
	//Frigidazzi's and talked to her. If he gave the furcap back,
	//gflags[KNOWS_FURCAP_OWNER] is false; in this case, we set
	//the new flag to indicate the quest completion:
	if (!gflags[KNOWS_FURCAP_OWNER])
		gflags[GAVE_FURCAP_BACK] = true;
}

/*
 *  The following function has been created from usecode found in the
 *	Exult CVS snapshot. I include it here only for convenience; I have
 *	edited it to fit the conventions used in the rest of the mod.
 *
 *	The original code was written by Jeff Freedman (aka "DrCode").
 *
 */
const int QUALITY_LOGIC					= 13;
const int QUALITY_ETHICALITY			= 14;
const int QUALITY_DISCIPLINE			= 15;

CureCantra ()
{
	if (event == PATH_SUCCESS)
	{
		UI_close_gumps();
		UI_set_item_frame(item, 0);	// Now empty.
		UI_set_item_quality(item, 0);	//Just for safety
	
		UI_obj_sprite_effect(CANTRA, ANIMATION_TELEPORT, 0, 0, 0, 0, 0, 0);
		UI_clear_item_flag(CANTRA, SI_ZOMBIE);	// No longer crazy.
		UI_set_schedule_type(CANTRA, TALK);
		gflags[CURED_CANTRA] = true;			// We've done it.
	}
	
	else if (event == PATH_FAILURE)
		UI_set_schedule_type(CANTRA, WANDER);
}

CureCompanion ()
{
	var npcnum;
	var bucket_quality = UI_get_item_quality(item);
	
	//Through the bucket's quality, determine which companion
	//is being cured:
	if (bucket_quality == QUALITY_LOGIC)
		npcnum = IOLO;
	else if (bucket_quality == QUALITY_ETHICALITY)
		npcnum = SHAMINO;
	else if (bucket_quality == QUALITY_DISCIPLINE)
		npcnum = DUPRE;
	
	if (event == PATH_SUCCESS)
	{
		UI_close_gumps();
		UI_set_item_frame(item, 0);	// Now empty.
		UI_set_item_quality(item, 0);	//Just for safety
	
		UI_obj_sprite_effect(npcnum, ANIMATION_TELEPORT, 0, 0, 0, 0, 0, 0);
		UI_clear_item_flag(npcnum, SI_ZOMBIE);
		
		//This is for the new conversation, and only happens once per companion:
		UI_set_npc_id(npcnum, CURED_OF_INSANITY);
		
		if (!UI_get_item_flag(npcnum, IN_PARTY))
			UI_set_schedule_type(npcnum, TALK);
	}
	
	else if (event == PATH_FAILURE)
		UI_set_schedule_type(npcnum, WANDER);
}

firesnakeExplode ()
{
	var obj;
	var index;
	var max;
	var damage;
	var nearbyobjs = UI_find_nearby(item, SHAPE_ANY, 8, 0);
	var party = UI_get_party_list();
	var pos = UI_get_object_position(item);
	var vertoff = (pos[Z] + 1) / 2;
	UI_sprite_effect(1, (pos[X] + vertoff), (pos[Y] + vertoff), 0, 0, 0, -1);
	UI_play_sound_effect(42);

	for (obj in nearbyobjs with index to max)
	{
		//Party safety:
		if (!(obj in party))
		{
			damage = UI_die_roll(10, 20);
			if (UI_is_npc(obj))
			{
				script obj hit damage;//BLEED
				if (UI_get_alignment(obj) != 2)
					UI_set_alignment(obj, 2);
				UI_set_schedule_type(obj, IN_COMBAT);
			}
			else
				script obj hit damage;//DONTBLEED
		}
	}
}

dropAllItems (var npc, var pos)
{
	var obj;
	var index;
	var max;
	
	var cont_collection = [SHAPE_CRATE, SHAPE_BARREL, SHAPE_CHEST,
						   SHAPE_LOCKEDCHEST, SHAPE_BACKPACK, SHAPE_BAG,
						   SHAPE_BASKET];
	var count;
	var containers;
	var flag;
	
	//While there is some extra work done here, it is better for the player.
	//Here, we first drop all CONTAINERS of the character, dropping only
	//those directly held by the character:
	while (count < UI_get_array_size(cont_collection))
	{
		count = count + 1;
		containers = UI_get_cont_items(npc, cont_collection[count], QUALITY_ANY, FRAME_ANY);
		if (containers)
		{
			for (obj in containers with index to max)
			{
				//Only directly drop those directly held by the NPC:
				if (UI_is_npc(getOuterContainer(obj)))
				{
					flag = UI_set_last_created(obj);
					if (flag)
						UI_update_last_created(pos);
				}
			}
		}
	}

	//Now that most objects are in the ground, inside containers, it is
	//time to drop the rest:
	var objects = UI_get_cont_items(npc, SHAPE_ANY, QUALITY_ANY, FRAME_ANY);
	if (objects)
	{
		for (obj in objects with index to max)
		{
			flag = UI_set_last_created(obj);
			if (flag)
				UI_update_last_created(pos);
		}
	}
}
