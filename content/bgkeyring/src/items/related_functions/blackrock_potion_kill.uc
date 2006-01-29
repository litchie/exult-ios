/*
 *	This source file contains usecode for the Blackrock potion's kill ability.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

killTargetNPC ()
{
	var npcshape;
	var npclist;
	var npc;
	var pos;
	var body;
	var scroll;
	var alignment;
	var ownedscrolls;
	var scrollquality;

	npcshape = get_item_shape();
	if (npcshape == SHAPE_LORD_BRITISH)
	{
		//The victim is the King of Britannia
		//Make all friendly NPCs around hostile and attack Avatar:
		npclist = AVATAR->find_nearby(-1, 80, MASK_NPC2);
		for (npc in npclist with index to count)
		{
			npcshape = npc->get_item_shape();
			if (!((npcshape == SHAPE_MALE_AVATAR) || (npcshape == SHAPE_FEMALE_AVATAR)))
			{
				if (npc->get_alignment() == 0)
				{
					npc->set_alignment(2);
					npc->set_schedule_type(IN_COMBAT);
				}
			}
		}
		//Kill Lord British:
		pos = get_object_position();
		//Create LB's body (the bastard cannot be killed...):
		body = UI_create_new_object(SHAPE_BODIES_2);
		body->set_item_frame_rot(get_item_frame_rot());
		body->set_item_frame(FRAME_LBBODY2);
		//Remove LB:
		LORD_BRITISH->remove_npc();
		//Place the body in LB's old position:
		UI_update_last_created(pos);
		//Create LB's will:
		scroll = UI_create_new_object(SHAPE_SCROLL);
		scroll->set_item_frame(0);
		scroll->set_item_quality(43);
		body->give_last_created();
		//Dying sequence:
		script body
		{
			wait 5;
			frame CAST_2_SOUTH;
			wait 2;
			frame KNEEL_NORTH;
		}
	}
	else
	{
		//Get the NPC's alignment:
		alignment = get_alignment();
		if (!get_item_flag(TEMPORARY))
		{
			//Permanent NPCs work differently;
			//kill NPC:
			kill_npc();
			remove_npc();
			//Make all NPCs of the same alignment as victim hostile:
			npclist = AVATAR->find_nearby(-1, 80, MASK_NPC2);
			for (npc in npclist with index to count)
			{
				npcshape = npc->get_item_shape();
				if (!((npcshape == SHAPE_MALE_AVATAR) || (npcshape == SHAPE_FEMALE_AVATAR)))
				{
					if (npc->get_alignment() == alignment)
					{
						npc->set_alignment(2);
						npc->set_schedule_type(IN_COMBAT);
					}
				}
			}
		}
		else
		{
			//FoV 'NPCs' are temporary monsters carrying scrolls:
			ownedscrolls = get_cont_items(SHAPE_SCROLL, QUALITY_ANY, 4);
			if (ownedscrolls)
			{
				scrollquality = ownedscrolls->get_item_quality();
				body = false;
				if (scrollquality == 240)
				{
					//Erethian
					//Set flag to prevent respawning:
					gflags[0x02EE] = true;
					pos = get_object_position();
					//Create his body:
					body = UI_create_new_object(SHAPE_BODIES_3);
					body->set_item_frame_rot(get_item_frame_rot());
					body->set_item_frame(FRAME_MAGEBODY);
					//Clear inventory:
					clearInventory(item);
					UI_update_last_created(pos);
				}
				if (scrollquality == 241)
				{
					//Dracothraxus
					//Set flag to prevent respawning:
					gflags[0x02EF] = true;
					pos = get_object_position();
					//Create his body:
					body = UI_create_new_object(SHAPE_LARGE_BODIES);
					body->set_item_frame_rot(get_item_frame_rot());
					body->set_item_frame(FRAME_DRAGON_BODY);
					//Clear inventory:
					clearInventory(item);
					UI_update_last_created(pos);
				}
				//Create another scroll, with the same quality
				//as the one that was owned by Erethian or
				//Dracothraxus:
				scroll = UI_create_new_object(SHAPE_SCROLL);
				scroll->set_item_flag(TEMPORARY);
				scroll->set_item_quality(scrollquality);
				scroll->set_item_frame(4);
				//Put it in the body:
				body->give_last_created();
				script scroll after 1 ticks
				{
					finish;
					nohalt;
					//Not sure what it does:
					call makeStuffToBodies;
				}
				//Make all NPCs of the same alignment as victim hostile:
				npclist = AVATAR->find_nearby(-1, 80, MASK_NPC2);
				for (npc in npclist with index to count)
				{
					npcshape = npc->get_item_shape();
					if (!((npcshape == SHAPE_MALE_AVATAR) || (npcshape == SHAPE_FEMALE_AVATAR)))
					{
						if (npc->get_alignment() == alignment)
						{
							npc->set_alignment(2);
							npc->set_schedule_type(IN_COMBAT);
						}
					}
				}
			}
			else
			{
				//Kill monster npc:
				kill_npc();
				//Make all NPCs of the same alignment as victim hostile:
				npclist = AVATAR->find_nearby(-1, 80, MASK_NPC2);
				for (npc in npclist with index to count)
				{
					npcshape = npc->get_item_shape();
					if (!((npcshape == SHAPE_MALE_AVATAR) || (npcshape == SHAPE_FEMALE_AVATAR)))
					{
						if (npc->get_alignment() == alignment)
						{
							npc->set_alignment(2);
							npc->set_schedule_type(IN_COMBAT);
						}
					}
				}
			}
		}
	}
}
