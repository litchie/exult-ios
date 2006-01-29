/*
 *	This source file contains usecode for the Keyring Quest.
 *	Specifically, it contains some functions for Zauriel.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2001-01-20
 */

zaurielSellPotions ()
{
	sellItems(
				//Names
				["nothing",
				 "sleep potion", "healing potion", "curative potion",
				 "vial of poison", "awakening potion", "protection potion",
				 "illumination potion", "invisibility potion"],
				//Shapes
				[0,
				 SHAPE_POTION, SHAPE_POTION, SHAPE_POTION,
				 SHAPE_POTION, SHAPE_POTION, SHAPE_POTION,
				 SHAPE_POTION, SHAPE_POTION],
				//Frames
				[0,
				 0, 1, 2,
				 3, 4, 5,
				 6, 7],
				//Price per unit
				[0,
				 20, 160, 160,
				 20, 10, 10,
				 40, 80],
				 //Quantities
				 [0,
				  1, 1, 1,
				  1, 1, 1,
				  1, 1],
				  //Articles
				 ["",
				  "A ", "A ", "A ",
				  "A ", "An ", "A ",
				  "An ", "An "],
				  //Quantity strings
				 ["",
				  " per vial", " per vial", " per vial",
				  " each", " per vial", " per vial",
				  " per vial", " per vial"],
				  //Quantity tokens
				 ["",
				  "", "", "",
				  "", "", "",
				  "", ""],
				 //Dialog strings
				 [" Dost thou still want it?@",
				  "@How many ", "dost thou want?@",
				  "@Done! What else dost thou want?@",
				  "@If thou couldst carry that much, I would sell it to thee. Anything else?@",
				  "@Thou dost not have sufficient funds. Earn some gold and then return. Anything else?@",
				  "@Fine by me. Dost thou wish anything else?@",
				  "@Fair enough. Anything else?@",
				  "@I have no problems with that. Where were we?@"]);
}

zaurielSellReagents ()
{
	sellItems(
				//Names
				["nothing",
				 "Black pearl", "Blood moss", "Nightshade",
				 "Mandrake root", "Garlic", "Ginseng",
				 "Spider silk", "Sulphurous ash"],
				//Shapes
				[SHAPE_REAGENT,
				 SHAPE_REAGENT, SHAPE_REAGENT, SHAPE_REAGENT,
				 SHAPE_REAGENT, SHAPE_REAGENT, SHAPE_REAGENT,
				 SHAPE_REAGENT, SHAPE_REAGENT],
				//Frames
				[0,
				 0, 1, 2,
				 3, 4, 5,
				 6, 7],
				//Price per unit
				[0,
				 6, 1, 5,
				 7, 1, 1,
				 3, 2],
				 //Quantities
				 [0,
				  1, 1, 1,
				  1, 1, 1,
				  1, 1],
				  //Articles
				 ["",
				  "", "", "",
				  "", "", "",
				  "", ""],
				  //Quantity strings
				 ["",
				  " each", " for one portion", " for one button",
				  " each", " for one clove", " for one portion",
				  " for one portion", " for one portion"],
				  //Quantity tokens
				 ["",
				  "", "", "",
				  "", "", "",
				  "", ""],
				 //Dialog strings
				 [" Dost thou still want it?@",
				  "@How many ", " dost thou want?@",
				  "@Done! What else dost thou want?@",
				  "@If thou couldst carry that much, I would sell it to thee. Anything else?@",
				  "@Thou dost not have sufficient funds. Earn some gold and then return. Anything else?@",
				  "@Fine by me. Dost thou wish anything else?@",
				  "@Fair enough. Anything else?@",
				  "@I have no problems with that. Where were we?@"]);
}

zaurielGiveAdvance ()
{
	var advance_reagent = [SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT,SHAPE_REAGENT];
	var advance_potions = [SHAPE_POTION,SHAPE_POTION,SHAPE_POTION,SHAPE_POTION,SHAPE_POTION,SHAPE_POTION,SHAPE_POTION,SHAPE_POTION];
	var advance_item_frames =	[0, 1, 2, 3, 4, 5, 6, 7];
	var advance_reagent_quantity = [20, 20, 20, 20, 20, 20, 20, 20];
	var advance_potions_quantity = [1, 4, 4, 0, 2, 2, 1, 2];
	var advance_quality = [0, 0, 0, 0, 0, 0, 0, 0];

	var pouch1 = createContainerWithObjects(SHAPE_BAG, advance_reagent, advance_item_frames, advance_reagent_quantity, advance_quality);
	var pouch2 = createContainerWithObjects(SHAPE_BAG, advance_potions, advance_item_frames, advance_potions_quantity, advance_quality);
	
	say("@In any case, since thou wilt help me, I shall now give thee a selection of reagents and potions to assist thee.");
	say("@This is an advance payment, but it shall not be deducted from thy upcoming reward. Consider it as a bonus...@");
	
	var gavereags = giveToParty(pouch1);
	var gavepotns = giveToParty(pouch2);
	
	if (gavereags && gavepotns)
		say("@There is the advance. I have placed it in a couple of bags for thy convenience.@");
	else
	{
		if (!gavereags) pouch1->set_last_created();
		UI_update_last_created(AVATAR->get_object_position());
		
		if (!gavepotns) pouch2->set_last_created();
		UI_update_last_created(AVATAR->get_object_position());
		
		var msg = "it";
		if ((!gavereags) && (!gavepotns)) msg = "them";
		
		say("@I placed the advance in these bags. Since thee and thy party are too encumbered, I shall place " + msg + " on the ground.@");
	}
}

zaurielCreateComponents (var quantities)
{
	var cont = createContainerWithObjects(SHAPE_CHEST,
										  [SHAPE_SPIDER_EGG, SHAPE_BEE_STINGER, SHAPE_INVISIBILITY_DUST],
										  [0,				0,					0],
										  quantities,
										  [0,				0,					0]);
	cont->set_last_created();
	ZAURIEL->give_last_created();
}

zaurielDestroyComponents ()
{
	var shapes = [SHAPE_SPIDER_EGG, SHAPE_BEE_STINGER, SHAPE_INVISIBILITY_DUST];
	
	var total;
	var counter;
	var shp;
	
	var pouch = ZAURIEL->get_cont_items(SHAPE_CHEST, QUALITY_ANY, FRAME_ANY);
	
	var cont_items;
	
	for (shp in shapes with counter to total)
	{
		cont_items = ZAURIEL->count_objects(shp[counter], QUALITY_ANY, FRAME_ANY);
		pouch->remove_cont_items(cont_items, shp[counter], QUALITY_ANY, FRAME_ANY, true);
	}
	pouch->remove_item();
}
