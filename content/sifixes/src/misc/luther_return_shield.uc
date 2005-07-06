returnDupresShield 0x848 ()
{
	say("@I shall certainly be sad to part with it, but if thou dost claim it, then I must give it to thee.@");
	LUTHER->remove_cont_items(1, SHAPE_DUPRE_SHIELD, QUALITY_ANY, 0, 0);
	giveItemsToPartyMember(AVATAR, 1, SHAPE_DUPRE_SHIELD, QUALITY_ANY, FRAME_ANY, 0, true);
	gflags[HAS_DUPRE_SHIELD] = true;
	say("@Now I shall have to find myself another shield...@");
	
	if (gflags[STORM_MONITOR_SHIELD] && hasItemCount(PARTY, 1, SHAPE_MONITOR_SHIELD, QUALITY_ANY, 0))
	{
		partyUtters(1, "@But thou canst have this shield which we found!@", 0, false);
		UI_set_conversation_slot(0);
		say("@That is most gracious of thee! Yes, this is my shield which so strangely disappeared during that storm!@");
		gflags[KNOWS_MONITOR_SHIELD_ORIGIN] = true;
		
		UI_remove_party_items(1, SHAPE_MONITOR_SHIELD, QUALITY_ANY, 0, true);
		if (LUTHER->add_cont_items(1, SHAPE_MONITOR_SHIELD, QUALITY_ANY, 0, 18))
		{
			var shield = UI_create_new_object(SHAPE_MONITOR_SHIELD);
			if (shield)
			{
				shield->set_item_flag(TEMPORARY);
				var pos = LUTHER->get_object_position();
				UI_update_last_created(pos);
			}
		}
	}
}
