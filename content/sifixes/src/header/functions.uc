freeze 0xC00 (){set_item_flag(DONT_MOVE);}
unfreeze (){clear_item_flag(DONT_MOVE);}

addShaminoToParty ()
{
	SHAMINO->add_to_party();
	gflags[SHAMINO_HAS_BELONGINGS] = true;
	SHAMINO->set_npc_id(0);
	
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
	if (IOLO->get_cont_items(SHAPE_ANY, QUALITY_ANY, FRAME_ANY))
	{
		say("@I am carrying many items, some of which may be of use to thee. Wouldst thou care to have these?@");
		if (askYesNo())
		{
			say("@Here they are.@");
			gflags[IOLO_HAS_BELONGINGS] = false;
	
			var iolo_items = IOLO->get_cont_items(SHAPE_ANY, QUALITY_ANY, FRAME_ANY);
			var obj;
			var index;
			var max;
			var give_return;
			var cumulative_result = [false, 0];
			
			for (obj in iolo_items with index to max)
			{
				if (!((obj->get_item_shape() == SHAPE_PLAINRING) && (obj->get_item_frame() == 2)))
				{
					give_return = giveItemsToPartyMember(AVATAR, obj->get_item_quantity(0), obj->get_item_shape(), obj->get_item_quality(), obj->get_item_frame(), obj->get_item_flag(TEMPORARY), false);
					if (cumulative_result[1] == 0)
						cumulative_result[1] = give_return[1];
	
					cumulative_result[2] = (cumulative_result[2] + give_return[getArraySize(give_return)]);
					obj->remove_item();
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
		set_item_frame(0);	// Now empty.
		set_item_quality(0);	//Just for safety
	
		CANTRA->obj_sprite_effect(ANIMATION_TELEPORT, 0, 0, 0, 0, 0, 0);
		CANTRA->clear_item_flag(SI_ZOMBIE);	// No longer crazy.
		CANTRA->set_schedule_type(TALK);
		gflags[CURED_CANTRA] = true;			// We've done it.
	}
	
	else if (event == PATH_FAILURE)
		CANTRA->set_schedule_type(WANDER);
}

CureCompanion ()
{
	var npcnum;
	var bucket_quality = get_item_quality();
	
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
		set_item_frame(0);	// Now empty.
		set_item_quality(0);	//Just for safety
	
		npcnum->obj_sprite_effect(ANIMATION_TELEPORT, 0, 0, 0, 0, 0, 0);
		npcnum->clear_item_flag(SI_ZOMBIE);
		
		//This is for the new conversation, and only happens once per companion:
		npcnum->set_npc_id(CURED_OF_INSANITY);
		
		if (!npcnum->get_item_flag(IN_PARTY))
			npcnum->set_schedule_type(TALK);
	}
	
	else if (event == PATH_FAILURE)
		npcnum->set_schedule_type(WANDER);
}

firesnakeExplode ()
{
	var obj;
	var index;
	var max;
	var damage;
	var nearbyobjs = find_nearby(SHAPE_ANY, 8, 0);
	var party = UI_get_party_list();
	var pos = get_object_position();
	var vertoff = (pos[Z] + 1) / 2;
	UI_sprite_effect(1, (pos[X] + vertoff), (pos[Y] + vertoff), 0, 0, 0, -1);
	UI_play_sound_effect(42);

	for (obj in nearbyobjs with index to max)
	{
		//Party safety:
		if (!(obj in party))
		{
			damage = UI_die_roll(10, 20);
			if (obj->is_npc())
			{
				script obj hit damage;//BLEED
				if (obj->get_alignment() != 2)
					obj->set_alignment(2);
				obj->set_schedule_type(IN_COMBAT);
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
		containers = npc->get_cont_items(cont_collection[count], QUALITY_ANY, FRAME_ANY);
		if (containers)
		{
			for (obj in containers with index to max)
			{
				//Only directly drop those directly held by the NPC:
				if (getOuterContainer(obj)->is_npc())
				{
					flag = obj->set_last_created();
					if (flag)
						UI_update_last_created(pos);
				}
			}
		}
	}

	//Now that most objects are in the ground, inside containers, it is
	//time to drop the rest:
	var objects = npc->get_cont_items(SHAPE_ANY, QUALITY_ANY, FRAME_ANY);
	if (objects)
	{
		for (obj in objects with index to max)
		{
			flag = obj->set_last_created();
			if (flag)
				UI_update_last_created(pos);
		}
	}
}
