//I did this so the companions can refuse Blue potions while on Spinebreaker Mountains.
//It also is a good learning exercise...

extern healByAmount 0x976 (var target, var amount);

enum potions
{
	SLEEP_POTION		= 0,
	HEALING_POTION		= 1,
	CURING_POTION		= 2,
	POISION_POTION		= 3,
	AWAKENING_POTION	= 4,
	PROTECTION_POTION	= 5,
	LIGHT_POTION		= 6,
	INVISIBILITY_POTION	= 7,
	MANA_POTION			= 8,
	WARMTH_POTION		= 9
};

Potion shape#(0x154) ()
{
	var itemframe;
	var target;
	
	if (event == DOUBLECLICK)
	{
		itemframe = UI_get_item_frame(item);
		target = UI_click_on_item();

		UI_play_sound_effect2(0x24, item);
		if (UI_is_npc(target))
		{
			UI_play_sound_effect(0x39);
			if (itemframe == SLEEP_POTION)
			{
				var npcnum = UI_get_npc_number(target);
				if ((getAvatarLocationID() == SPINEBREAKER_MOUNTAINS) && ((npcnum == IOLO) || (npcnum == SHAMINO) || (npcnum == DUPRE)))
				{
					npcnum.say("@But we are so close to Batlin now, Avatar! I don't want to miss all the action!@");
					abort;
				}
				UI_set_item_flag(target, ASLEEP);
			}
			
			else if (itemframe == HEALING_POTION)
				healByAmount(target, UI_die_roll(3, 12));
			
			else if (itemframe == CURING_POTION)
			{
				UI_clear_item_flag(target, POISONED);
				UI_clear_item_flag(target, PARALYZED);
				UI_clear_item_flag(target, ASLEEP);
				UI_clear_item_flag(target, CHARMED);
				UI_clear_item_flag(target, CURSED);
			}
			
			else if (itemframe == POISION_POTION)
				UI_set_item_flag(target, POISONED);

			else if (itemframe == AWAKENING_POTION)
				UI_clear_item_flag(target, ASLEEP);

			else if (itemframe == PROTECTION_POTION)
				UI_set_item_flag(target, PROTECTION);

			else if (itemframe == LIGHT_POTION)
				UI_cause_light(200);

			else if (itemframe == INVISIBILITY_POTION)
				UI_set_item_flag(target, INVISIBLE);

			else if (itemframe == MANA_POTION)
			{
				if (target == UI_get_npc_object(AVATAR))
				{
					var mana = UI_get_npc_prop(AVATAR, MANA);
					var recover = UI_get_random(10);
					if ((mana + recover) > 31)
						recover = (31 - mana);

					UI_set_npc_prop(AVATAR, MANA, recover);
				}
			}
			
			else if (itemframe == WARMTH_POTION)
				UI_set_temperature(target, 0);

			else if (itemframe >= 10)
			{
				partyUtters(1, "@What is this!@", 0, false);
				abort;
			}
			UI_remove_item(item);
			abort;
		}
		else
		{
			var pos = [target[X + 1], target[Y + 1], target[Z + 1]];
			partyUtters(1, "@Please, " + getPoliteTitle() + ", waste them not!@", 0, false);
			var spill = UI_create_new_object(SHAPE_SPILL);
			if (spill)
			{
				UI_set_item_flag(spill, TEMPORARY);
				UI_set_item_frame(spill, UI_die_roll(4, 23));
				UI_update_last_created(pos);
			}
			UI_remove_item(item);
		}
	}
}
