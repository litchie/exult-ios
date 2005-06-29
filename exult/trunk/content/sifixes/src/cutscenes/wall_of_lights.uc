//I partially reimplemented the Wall of Lights for two reasons:
//(1) absolutelly force the companions to be there and
//(2) so that when they leave as the banes, their equipment is
//	  not dumped item by item in the ground, but remains in its
//	  respective containers.
//I haven't succeeded yet in preventing their presence there after
//the changes I made. As for (2), you can now leave the equipment
//with them as it will not be so annoying to equip the automatons
//that will carry the equipment back to the companions.

// externs
extern partySurroundBatlin 0x61D ();
extern killEveryone 0x7D8 ();

const int BATLIN_FACE				= -287;

BatlinAtWallOfLights 0x73B ()
{
	var pos;
	var companions = [SHAMINO, DUPRE, IOLO];
	var baneshapes = [SHAPE_ANTISHAMINO, SHAPE_ANTIDUPRE, SHAPE_MAD_IOLO];
	var npc;
	var newbaneshape;

	if ((UI_get_item_quality(item) == 255) && (event == EGG))
	{
		var index;
		var max;
		
		pos = UI_get_object_position(AVATAR);
		pos[X] = pos[X] - 4;
		pos[Y] = pos[Y] + 10;
		for (npc in companions with index to max)
		{
			//Absolutely ensure that the companions will be there for the scene:
			UI_clear_item_flag(npc, DEAD);
			UI_clear_item_flag(npc, ASLEEP);
			UI_clear_item_flag(npc, CHARMED);
			UI_clear_item_flag(npc, CURSED);
			UI_clear_item_flag(npc, PARALYZED);
			UI_clear_item_flag(npc, POISONED);
			UI_clear_item_flag(npc, DANCING);
			UI_clear_item_flag(npc, DONT_MOVE);
			UI_clear_item_flag(npc, CONFUSED);
			
			//Do not let even the Avatar prevent them from being there:
			UI_set_item_flag(npc, SI_TOURNAMENT);
			
			if (UI_get_distance(npc, AVATAR) > 15)
			{
				pos[X] = pos[X] + index * 2;
				UI_move_object(npc, pos);
			}
		}
		BatlinAtWallOfLights.original();
		abort;
	}
	
	else if (event == EGG)
	{
		pos = UI_get_object_position(AVATAR);
		pos[X] = pos[X] - 4;
		pos[Y] = pos[Y] + 5;
		for (npc in companions with index to max)
		{
			if (!UI_npc_nearby(npc))
			{
				UI_set_last_created(npc);
				pos[X] = pos[X] + index * 2;
				UI_update_last_created(pos);
			}
		}
		BatlinAtWallOfLights.original();
		abort;
	}	
		
	else if (event == SCRIPTED)
	{
		var pathegg = getPathEgg(2, 1);
		var pathegg_quality = UI_get_item_quality(pathegg);
		
		if ((pathegg_quality > 3) && (pathegg_quality < 7))
		{
			npc = companions[(pathegg_quality - 3)];
			UI_play_sound_effect(0x77);
			
			if (pathegg_quality > 4)
			{
				pos = UI_get_object_position(npc);
				UI_sprite_effect(7, pos[X], pos[Y], 0, 0, 0, -1);
			}
			
			var body = UI_find_object(-359, SHAPE_BODIES_2, QUALITY_ANY, 16);
			var dir = UI_find_direction(npc, body);
			var offsets = [0, 0];
			
			if (dir in [NORTHWEST, NORTH, NORTHEAST])
				offsets[Y] = -1;
			else if (dir in [SOUTHWEST, SOUTH, SOUTHEAST])
				offsets[Y] = 1;
				
			if (dir in [NORTHEAST, EAST, SOUTHEAST])
				offsets[X] = 1;
			else if (dir in [NORTHWEST, WEST, SOUTHWEST])
				offsets[X] = -1;

			var dist = UI_get_distance(npc, body);
			offsets[X] = offsets[X] * dist;
			offsets[Y] = offsets[Y] * dist;
			pos = UI_get_object_position(npc);
			pos[X] = pos[X] + offsets[X];
			pos[Y] = pos[Y] + offsets[Y];
			offsets[X] = -1 * offsets[X];
			offsets[Y] = -1 * offsets[Y];
			UI_sprite_effect(32, pos[X], pos[Y], offsets[X], offsets[Y], 0, dist);
			UI_set_item_quality(pathegg, pathegg_quality + 1);
			
			script pathegg
			{	nohalt;						wait (dist + 10);
				call BatlinAtWallOfLights;}
			
			newbaneshape = baneshapes[(pathegg_quality - 3)];
			
			UI_remove_from_party(npc);
			pos = UI_get_object_position(npc);
			var objpos = [0x6B + 3 * npc, 0x36, 0];

			UI_move_object(npc, objpos);
			setNewSchecules(npc, objpos[X], objpos[Y], WAIT);
			
			var baneframe = UI_get_item_frame(npc);
			var baneobj = UI_create_new_object2(newbaneshape, pos);
			UI_set_item_frame(baneobj, baneframe);
			UI_set_schedule_type(baneobj, WAIT);
			UI_sprite_effect(7, pos[X], pos[Y], 0, 0, 0, -1);
			abort;
		}
			
		else if (pathegg_quality == 7)
		{
			UI_show_npc_face0(0xFEDE, 0x0000);
			say("@I am Anarchy! Hahaha!@");
			say("@The world shall quail before me! Wrong shall become right! And right shall become lost! Nothing shall escape my touch!@");
			UI_remove_npc_face0();
			
			UI_show_npc_face0(0xFEDD, 0x0000);
			say("@Hahaha! I am the Wantonness Bane!@");
			say("@Wherever I pass, people shall frolic and sate their wild desires! I shall drive thee to feed thy darkest hungers!@");
			UI_remove_npc_face0();
			
			UI_show_npc_face0(0xFEDC, 0x0000);
			say("@I -- hahaha! -- am the Insanity Bane! Hahaha!@");
			say("@All those who fall within my shadow shall have their reason clouded and their wits addled! Their greatest love shall lie in the rubbish they once loathed! Hahaha!@");
			UI_remove_npc_face0();
			
			pos = UI_get_object_position(AVATAR);
			UI_sprite_effect(7, pos[X], pos[Y], 0, 0, 0, -1);
			
			var banenpc;
			
			for (newbaneshape in baneshapes with index to max)
			{
				banenpc = UI_find_nearby(AVATAR, newbaneshape, 40, 0);
				if (banenpc)
				{
					pos = UI_get_object_position(banenpc);
					UI_remove_item(banenpc);
					dropAllItems(companions[index], pos);
					UI_clear_item_flag(npc, SI_TOURNAMENT);
					UI_kill_npc(npc);
				}
			}

			if (UI_npc_nearby(BOYDON))
			{
				pos = UI_get_object_position(BOYDON);
				script BOYDON
				{	nohalt;						wait 7;
					hit 55;//, BLEED;
				}
			}
			UI_play_sound_effect(0x2A);
			gflags[SERPENT_GWANI_HORN_SPEECH] = true;
			script getPathEgg(5, 4) after 40 ticks
			{	nohalt;						call startSerpentSpeechViaRing;}
			UI_set_item_quality(pathegg, 0);
			UI_clear_item_flag(AVATAR, DONT_MOVE);
			UI_set_weather(0);
			abort;
		}
		else
			BatlinAtWallOfLights.original();
	}
}
