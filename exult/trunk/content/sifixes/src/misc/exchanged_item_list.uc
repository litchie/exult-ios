/*
 *
 *  Copyright (C) 2006  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

setExchangedItemFlags 0x92B ()
{
	var item_shapes;
	var item_qualities;
	var item_frames;
	var index;
	var count;
	item_shapes = exchangedItemList(1);
	item_qualities = exchangedItemList(2);
	item_frames = exchangedItemList(3);
	index = 1;
	count = (UI_get_array_size(item_shapes) + 1);
	while (index < count)
	{
		//Iterate through the list of exchanged items and set the flag for
		//each item possessed by the party:
		if (hasItemCount(PARTY, 1, item_shapes[index], item_qualities[index], item_frames[index]))
		{
			if (item_shapes[index] == SHAPE_PINECONE)
				gflags[STORM_PINECONE] = true;

			else if (item_shapes[index] == SHAPE_NEST_EGG)
				gflags[STORM_BLUE_EGG] = true;

			else if (item_shapes[index] == SHAPE_REAGENT)
				gflags[STORM_RUDDY_ROCK] = true;

			else if (item_shapes[index] == SHAPE_ALCHEMY_APPARATUS)
				gflags[STORM_LAB_APPARATUS] = true;

			else if (item_shapes[index] == SHAPE_PUMICE)
				gflags[STORM_PUMICE] = true;

			else if (item_shapes[index] == SHAPE_BRUSH)
				gflags[STORM_GOBLIN_BRUSH] = true;
			
			else if (item_shapes[index] == SHAPE_RING)
				gflags[STORM_WEDDING_RING] = true;
			
			else if (item_shapes[index] == SHAPE_STOCKINGS)
				gflags[STORM_STOCKINGS] = true;
			
			else if (item_shapes[index] == SHAPE_LARGE_SKULL)
				gflags[STORM_BEAR_SKULL] = true;
			
			else if (item_shapes[index] == SHAPE_MONITOR_SHIELD)
				gflags[STORM_MONITOR_SHIELD] = true;
			
			else if (item_shapes[index] == SHAPE_BOTTLE)
				gflags[STORM_ICEWINE] = true;
			
			else if (item_shapes[index] == SHAPE_URN)
				gflags[STORM_URN] = true;
			
			else if (item_shapes[index] == SHAPE_BOOTS)
				gflags[STORM_SLIPPERS] = true;
			
			else if (item_shapes[index] == SHAPE_FILARI)
				gflags[STORM_FILARI] = true;
			
			else if (item_shapes[index] == SHAPE_SEVERED_LIMB)
				gflags[STORM_SEVERED_HAND] = true;
			
			else if (item_shapes[index] == SHAPE_LEATHER_HELM)
				gflags[STORM_FUR_CAP] = true;
			
			else if (item_shapes[index] == SHAPE_BREAST_PLATE)
				gflags[STORM_BREAST_PLATE] = true;
		}
		index = (index + 1);
	}
}

var exchangedItemList 0x92C (var index)
{
	var rr;
	if (index == 1)
		return [SHAPE_PINECONE, SHAPE_NEST_EGG, SHAPE_REAGENT, SHAPE_ALCHEMY_APPARATUS, SHAPE_PUMICE, SHAPE_BRUSH, SHAPE_RING, SHAPE_STOCKINGS, SHAPE_FOOD, SHAPE_LARGE_SKULL, SHAPE_MONITOR_SHIELD, SHAPE_BOTTLE, SHAPE_URN, SHAPE_BOOTS, SHAPE_FILARI, SHAPE_SEVERED_LIMB, SHAPE_LEATHER_HELM, SHAPE_BREAST_PLATE];

	else if (index == 2)
		return [false, false, false, false, false, false, false, false, false, false, false, false, 255, false, QUALITY_ANY, false, false, false];

	else if (index == 3)
		return [0, 3, 15, 1, 0, 6, 0, 0, 21, 0, 0, 16, 0, 5, FRAME_ANY, 0, 4, 0];
}
