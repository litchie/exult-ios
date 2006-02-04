/*
 *	This source file contains some reimplementations of all second
 *	circle spells.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-02-03
 */

/*
	Second circle Spells
	
	extern spellDestroyTrap ();
	extern spellEnchant ();
	extern spellFireBlast ();
	extern spellGreatLight ();
	extern spellMassCure ();
	extern spellProtection ();
	extern spellTelekinesis ();
	extern spellWizardEye ();
*/

enum second_circle_spells
{
	SPELL_DESTROY_TRAP				= 0,
	SPELL_ENCHANT					= 1,
	SPELL_FIRE_BLAST				= 2,
	SPELL_GREAT_LIGHT				= 3,
	SPELL_MASS_CURE					= 4,
	SPELL_PROTECTION				= 5,
	SPELL_TELEKINESIS				= 6,
	SPELL_WIZARD_EYE				= 7
};

spellDestroyTrap (var target)
{
	if (event == DOUBLECLICK)
	{
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		halt_scheduled();
		item_say("@An Jux@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;}
			var nearby_traps = target->find_nearby(SHAPE_TRAP, 2, MASK_EGG + MASK_INVISIBLE + MASK_TRANLUCENT);
			var index;
			var max;
			var trap;
			for (trap in nearby_traps with index to max)
			{
				target = trap->get_object_position();
				
				trap->halt_scheduled();
				trap->remove_item();
				UI_sprite_effect(13, target[X], target[Y], 0, 0, 0, -1);
				UI_play_sound_effect(66);
			}
			nearby_traps = find_nearby(SHAPE_LOCKED_CHEST, 2, MASK_EGG + MASK_INVISIBLE + MASK_TRANLUCENT);
			for (trap in nearby_traps with index to max)
			{
				target = trap->get_object_position();
				if (trap->get_item_quality() == KEY_PICKABLE_TRAPPED)
				{
					trap->set_item_quality(0);
					UI_sprite_effect(13, target[X], target[Y], 0, 0, 0, -1);
					UI_play_sound_effect(66);
				}
			}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				call spellFails;}
		}
	}
}

spellEnchant (var target)
{
	var normal_missiles = [SHAPE_ARROWS, SHAPE_BOLTS];
	var magic_missiles = [SHAPE_MAGIC_ARROWS, SHAPE_MAGIC_BOLTS];
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var target_shape = target->get_item_shape();
		var dir = direction_from(target);
		item_say("@Ort Ylem@");
		if (inMagicStorm() && (target_shape in normal_missiles))
		{
			script item
			{	nohalt;						face dir;
				sfx 67;						actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;}
			script target after 4 ticks
			{	nohalt;						call spellEnchant;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_2;		actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var counter = 0;
		var index;
		var max;
		var missile;
		for (missile in normal_missiles with index to max)
		{
			counter = (counter + 1);
			if (missile == get_item_shape())
				set_item_shape(magic_missiles[counter]);
		}
	}
}

spellFireBlast (var target)
{
	if ((event == DOUBLECLICK) || (event == WEAPON))
	{
		//var target = UI_click_on_item();
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@Vas Flam@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_FIREBOLT);
			script item
			{	nohalt;						face dir;
				sfx 65;						actor frame CAST_1;
				actor frame CAST_2;			actor frame SWING_2H_3;
				actor frame SWING_2H_3;		attack;
				actor frame STAND;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}

spellGreatLight ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Lor@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 68;
				actor frame SWING_1;		actor frame SWING_3;
				call spellGreatLight;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
		UI_cause_light(5000);
}

spellMassCure ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		var pos = get_object_position();
		UI_sprite_effect(7, pos[X], pos[Y], 0, 0, 0, -1);
		item_say("@Vas An Nox@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;
				sfx 64;						call spellMassCure;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var index;
		var max;
		var npc;
		var targets = getFriendlyTargetList(item, 25);
		for (npc in targets with index to max)
		{
				npc->clear_item_flag(POISONED);
				npc->clear_item_flag(PARALYZED);
		}
	}
}

spellProtection (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@Uus Sanct@");
		if (inMagicStorm() && target->is_npc())
		{
			script item
			{	nohalt;						face dir;
				sfx 109;					actor frame CAST_1;
				actor frame SWING_1;		actor frame SWING_2H_3;}
			
			script target after 5 ticks
			{	nohalt;						call spellProtection;}
			
			obj_sprite_effect(13, -2, -2, 0, 0, 0, -1);
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame SWING_1;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}

	else if (event == SCRIPTED)
		set_item_flag(PROTECTION);
}

spellTelekinesis (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		if (target[X] == 0)
			return;

		var dir = direction_from(target);
		item_say("@Ort Por Ylem@");
		if (inMagicStorm() && ((!target->is_npc()) && (target[X] != 0)))
		{
			set_to_attack(target, SHAPE_TELEKINESIS);
			script item
			{	nohalt;						face dir;
				sfx 67;						actor frame SWING_1;
				actor frame SWING_3;		attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}

	else if (event == WEAPON)
	{
		var usables = [SHAPE_LOOM, SHAPE_THREAD, SHAPE_SPINNING_WHEEL, SHAPE_WOOL, SHAPE_KITE, SHAPE_BUCKET, SHAPE_BELLOWS, SHAPE_KEG, SHAPE_CASK, SHAPE_STRENGTH_TESTER, SHAPE_WELLBASE, SHAPE_WELL, SHAPE_CHAIR, SHAPE_BEDROLL, SHAPE_BED_HORIZONTAL, SHAPE_BED_VERTICAL, SHAPE_ORB];
		var usables2 = [SHAPE_WINCH_HORIZONTAL, SHAPE_WINCH_VERTICAL, SHAPE_SWITCH, SHAPE_LEVER];
		var target_shape = get_item_shape();
		if (target_shape in usables2)
			script item
				call get_usecode_fun(), DOUBLECLICK;

		else if (!(target_shape in usables))
			script item
				call get_usecode_fun(), DOUBLECLICK;
	}
}

spellWizardEye ()
{
	if (event == DOUBLECLICK)
	{
		item_say("@Por Ort Wis@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame CAST_1;
				sfx 67;						actor frame CAST_2;
				actor frame CAST_1;			actor frame SWING_2H_3;
				call spellWizardEye;}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame CAST_1;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}

	else if (event == SCRIPTED)
		UI_wizard_eye(45, 200);
}
