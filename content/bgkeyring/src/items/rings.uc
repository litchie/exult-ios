/*
 *	This source file contains a 'fix' for rings which prevents them
 *	from taking effect if you are merely *holding* them.
 *	keyring item.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

Invisibility_Ring shape#(0x128) ()
{
	var cont;
	if (event == READIED)
	{
		//Get ring container:
		cont = get_container();
		if (cont->is_npc() && (item == cont->get_readied(6) || item == cont->get_readied(7)))
			//If the container is an NPC, and if the item is equipped in a finger,
			//make user invisible:
			cont->set_item_flag(INVISIBLE);
	}
	else if (event == UNREADIED)
	{
		//Stop invisibility:
		cont = getOuterContainer(item);
		cont->clear_item_flag(INVISIBLE);
	}
}

Regeneration_Ring shape#(0x12A) ()
{
	var cont;
	if (event == READIED)
	{
		//Get ring container:
		cont = get_container();
		if (cont->is_npc() && (item == cont->get_readied(6) || item == cont->get_readied(7)))
		{
			//If the container is an NPC, and if the item is equipped in a finger,
			//make user regenerate:
			halt_scheduled();
			script item after 100 ticks
				call Regeneration_Ring;
		}
	}
	else if (event == UNREADIED)
		//Stop regeneration:
		halt_scheduled();
		
	else if (event == SCRIPTED)
	{
		//Halt ring scripts:
		halt_scheduled();
		//Get ring container:
		cont = get_container();
		if (cont)
		{
			//Get Health and Strength:
			var max = cont->get_npc_prop(STRENGTH);
			var current = cont->get_npc_prop(HEALTH);
			if (current < max)
			{
				//NPC is hurt; regenerate 1 HP:
				cont->set_npc_prop(HEALTH, 1);
				if (UI_die_roll(1, 100) == 1)
				{
					//1% change for the ring to vanish:
					remove_item();
					return;
				}
			}
			//Call this function again:
			script item after 100 ticks
				call Regeneration_Ring;
		}
	}
}
