const int SHAPE_TORCH		= 595;
const int SHAPE_LIT_TORCH	= 701;

Torch shape#(0x253) ()
{
	if (event != DOUBLECLICK && event != SCRIPTED) return;
	LightTorch(item, SHAPE_LIT_TORCH);
}

LitTorch shape#(0x2BD)()
{
	var var0000;
	var var0001;
	var var0002;

	if (event == DOUBLECLICK || event == SCRIPTED)
	{
		DouseTorch(item, SHAPE_TORCH, event);
	}
	else if (event == 7)
	{
		var0000 = Func0827(AVATAR, item);
		var0001 = UI_execute_usecode_array(AVATAR, [0x59, var0000, 0x01, 0x6A, 0x61]);
		DouseTorch(item, SHAPE_TORCH, event);
	}
	else if (event == 5)
	{
		Func0905(item);
	}
}


DouseTorch 0x839 (var eventid, var shape, var object)
{
	if(eventid == DOUBLECLICK || eventid == SCRIPTED)
	{
		object->set_item_shape(shape);
		object->halt_scheduled();
		UI_play_sound_effect(0x002E);
		//UNKNOWN_82(true);
	}
	else if (eventid == 7)
	{
		object->set_item_shape(shape);
		object->halt_scheduled();
		UI_play_sound_effect(0x002E);
		//UNKNOWN_82(true);
	}
	else if (eventid == 5)
	{
		//UNKNOWN_82(true);
	}
	else if (eventid == 6)
	{
		//UNKNOWN_82(false);
	}
}


LightTorch 0x942 (var shape, var object)
{
	var framenum;
	var container;

	framenum = object->get_item_frame();
	if (!object->get_item_quality())
	{
		object->set_item_quality(UI_die_roll(0x001E, 0x003C));
	}

	if (object->get_item_shape() == SHAPE_TORCH)
	{
		if (object->get_item_quality() == 0x00FF)
		{
			object->item_say("Spent");
			return;
		}
	}
	
	container = object->get_container();
	if (container == 0 || container->is_npc())
	{
		set_item_shape(shape);
		if (container in UI_get_party_list()) Func0905(object);
	}
	else UI_flash_mouse(0);

	//object->UNKNOWN_82(true);
	//UI_UNKNOWN_84();
	return;
}


Func0905 0x905 (var object)
{
	object->halt_scheduled();
	//UNKNOWN_82(true);
	//UI_UNKNOWN_84();
	UI_delayed_execute_usecode_array(object, [0x23, 0x55, 0x0600], 50);
}


Func0600 0x600 ()
{
	var reduced_quality;
	var new_quality;
	var shape;
	var var0003;
	var var0004;

	reduced_quality	= get_item_quality() - 1;
	new_quality		= set_item_quality(reduced_quality);
	if (reduced_quality == 0)
	{
		halt_scheduled();
		shape = get_item_shape();

		if(shape == 0x0152)) set_item_shape(0x03E5);
		else if (shape == 0x02BD)
		{
			set_item_shape(0x0253);
			new_quality = set_item_quality(0x00FF);
		}
		else if (shape == 0x01B3) set_item_shape(0x0217);
			
		UI_play_sound_effect_0F(0x006A);
	}
	else
	{
		var0003 = UI_delayed_execute_usecode_array(item, [0x55, 0x0600], 0x0032);
	}
}
