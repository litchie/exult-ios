/*
 *	This source file contains some reimplementations of almost all
 *	linear spells. The exception is the 'Help' spell.
 *
 *	There is also a new spell in the list: 'Detect Charges'.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-02-03
 */

/*
	Linear Spells
	
	extern spellAwaken ();
	extern spellDouse ();
	extern spellFireworks ();
	extern spellGlimmer ();
	extern spellIgnite ();
	extern spellThunder ();
	extern spellWeather ();
	extern spellDetectCharges ();
*/

enum linear_spells
{
	SPELL_AWAKEN					= 0,
	SPELL_DOUSE						= 1,
	SPELL_FIREWORKS					= 2,
	SPELL_GLIMMER					= 3,
	SPELL_IGNITE					= 4,
	SPELL_THUNDER					= 5,
	SPELL_WEATHER					= 6,
	SPELL_DETECT_CHARGES			= 7			//NPC-only spell
};

spellAwaken (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		var dir = direction_from(target);
		item_say("@An Zu@");
		if (inMagicStorm() && (target[X] != 0))
		{
			script item
			{	nohalt;						face dir;
				sfx 64;						actor frame SWING_1;
				actor frame SWING_3;}
			script target after 5 ticks
			{	nohalt;						call spellAwaken;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				call spellFails;}
		}
	}

	else if (event == SCRIPTED)
	{
		if (is_npc())
			clear_item_flag(ASLEEP);
		else
			flashBlocked(60);
	}
}

spellDouse (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@An Flam@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_DOUSE);
			script item
			{	nohalt;						face dir;
				actor frame SWING_1;		actor frame SWING_3;
				attack;}
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
		var target_shape = get_item_shape();
		if (target_shape in [SHAPE_TORCH_LIT, SHAPE_LIT_LAMP, SHAPE_LIGHTSOURCE_LIT, SHAPE_SCONCE_LIT])
		{
			target_shape->telekenesis();
			script item
			{	nohalt;						call target_shape;}
			UI_play_sound_effect(46);
		}
		else
			flashBlocked(60);
	}
}

spellFireworks ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Bet Ort@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame CAST_2;
				actor frame CAST_1;			sfx 36;
				call spellFireworks;}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_2;
				actor frame CAST_1;			call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
	{
		var pos = get_object_position();
		UI_sprite_effect(12, (pos[X] - 2), (pos[Y] - 2), 0, 0, 0, -1);
	}
}

spellGlimmer ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Bet Lor@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						sfx 68;
				actor frame SWING_1;		actor frame SWING_3;
				call spellGlimmer;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
	
	else if (event == SCRIPTED)
		UI_cause_light(110);
}

spellIgnite (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var dir = direction_from(target);
		item_say("@In Flam@");
		if (inMagicStorm())
		{
			set_to_attack(target, SHAPE_IGNITE);
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		attack;}
		}
		else
		{
			script item
			{	nohalt;						face dir;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}

	else if (event == WEAPON)
	{
		var target_shape = get_item_shape();
		if (target_shape in [SHAPE_TORCH, SHAPE_LAMPPOST, SHAPE_LIGHTSOURCE, SHAPE_SCONCE])
		{
			target_shape->telekenesis();
			script item
			{	nohalt;						call target_shape;}
		}
		else
			flashBlocked(60);
	}
}

spellThunder ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Vas Kal@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_2H_2;
				actor frame SWING_2H_3;		sfx 62;}
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_2H_2;
				actor frame SWING_2H_3;		call spellFails;}
		}
	}
}

spellWeather ()
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		item_say("@Rel Hur@");
		if (inMagicStorm())
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		sfx 68;}
			var weather_array = [0, 1, 2];
			if (UI_get_weather() == 0)
				UI_set_weather(weather_array[UI_die_roll(2, 3)]);
			else if (UI_get_weather() != 3)
				UI_set_weather(0);
		}
		else
		{
			script item
			{	nohalt;						actor frame SWING_1;
				actor frame SWING_3;		call spellFails;}
		}
	}
}

spellDetectCharges (var target)
{
	if (event == DOUBLECLICK)
	{
		halt_scheduled();
		//var target = UI_click_on_item();
		var target_shape = target->get_item_shape();
		var dir = direction_from(target);
		item_say("@Wis Ort@");
		if (inMagicStorm() && (target_shape in [SHAPE_LIGHTNING_WAND, SHAPE_FIRE_WAND, SHAPE_FIREDOOM_STAFF]))
		{
			var bark = "@" + target->get_item_quality() + " charges left@";
			script item
			{	nohalt;						sfx 67;
				actor frame CAST_1;			actor frame CAST_2;
				actor frame USE;			wait 4;
				say bark;}
		}
		else
		{
			script item
			{	nohalt;						actor frame CAST_1;
				actor frame CAST_2;			actor frame USE;
				call spellFails;}
		}
	}
}
