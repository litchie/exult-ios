/*
 *  Copyright (C) 2002  The Exult Team
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdio>

#include "objs.h"
#include "game.h"
#include "items.h"
#include "gamewin.h"
#include "actors.h"

using std::string;
#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
namespace std {
using ::snprintf;
}
#else
#endif
#ifndef UNDER_CE
using std::strchr;
#endif

/*
 *	For objects that can have a quantity, the name is in the format:
 *		%1/%2/%3/%4
 *	Where
 *		%1 : singular prefix (e.g. "a")
 *		%2 : main part of name
 *		%3 : singular suffix
 *		%4 : plural suffix (e.g. "s")
 */

/*
 *	Extracts the first, second and third parts of the name string
 */
static void get_singular_name
	(
	const char *name,		// Raw name string from TEXT.FLX
	string& output_name		// Output string
	)
{
	if(*name != '/')		// Output the first part
		{
		do
			output_name += *name++;
		while(*name != '/' && *name != '\0');
		if(*name == '\0')	// should not happen
			{
			output_name = "?";
			return;
			}
		// If there is a first part it is followed by a space
		output_name += ' ';
		}
	name++;

					// Output the second part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;

					// Output the third part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
}

/*
 *	Extracts the second and fourth parts of the name string
 */
static void get_plural_name
	(
	const char *name,
	int quantity,
	string& output_name
	)
{
	char buf[20];

	snprintf(buf, 20, "%d ", quantity);	// Output the quantity
	output_name = buf;

					// Skip the first part
	while(*name != '/' && *name != '\0')
		name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
					// Output the second part
	while(*name != '/' && *name != '\0')
		output_name += *name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
					// Skip the third part
	while(*name != '/' && *name != '\0')
		name++;
	if(*name == '\0')		// should not happen
		{
		output_name = "?";
		return;
		}
	name++;
	while(*name != '\0')		// Output the last part
		output_name += *name++;
}

/*
 *	Returns the string to be displayed when the item is clicked on
 */
string Game_object::get_name
	(
	) const
{
	int quantity;
	string display_name;
	int shnum = get_shapenum();
	int frnum = get_framenum();
	int qual = get_quality();
	int index = -1;
	if (GAME_BG) {
		//TODO: yourself 
		switch (shnum)			// Some special cases!
		{
		case 0x34a:			// Reagents
			index = 0x500 + frnum;
			break;
		case 0x3bb:			// Amulets
			if (frnum < 3)
				index = 0x508 + frnum;
			else
				index = shnum;
			break;
		case 0x179:			// Food items
			index = 0x50b + frnum;
			break;
		case 0x28a:			// Sextants
			index = 0x52c - frnum;
			break;
		case 0x2a3:			// Desk items
			index = 0x52d + frnum;
			break;
		case 0x390:         // "Blood"
			if (frnum < 4)
				index = 0x390;
			else
				index = -1;
			break;
		default:
			index = shnum;
			break;
		}


	} else if (GAME_SI) {
		//TODO: yourself, misc. checks/fixes
		
		switch (shnum)			// More special cases!
		{
		case 0x097:         // wall
			if (frnum == 17)
				index = 0x681;  // door
			else
				index = shnum;
			break;
		case 0x98:          // wall
			if (frnum == 22)
				index = 0x681;  // door
			else
				index = shnum;
			break;
		case 0x0b2:			// Cloth maps
			if (frnum < 5 || GAME_SS)
				index = 0x596 + frnum;
			else
				index = shnum; // only SS has a description for frame 5
			break;
		case 0x0d1:			// Artifacts
			index = 0x5c0 + frnum;
			break;
		case 0xd2:          // chicken coop
			if (frnum == 4)
				index = 0x680; // platform
			else
				index = shnum;
			break;
		case 0x0e3:      // cloak
			if (frnum <= 4)
				index = 0x5ff + frnum;
			else
				index = shnum;
			break;
		case 0x0f3:      // coffin
			if (frnum >= 7)
				index = 0x605; // stone bier
			else
				index = shnum;
			break;
		case 0x0f4:			// Large skulls
			if (frnum < 2)
				index = 0x5d8 + frnum;
			else
				index = shnum;
			break;
		case 0x106:			// Blackrock Serpents
			if (frnum < 4)
				index = 0x592 + frnum;
			else
				index = shnum;
			break;
		case 0x11a:       // painting
			if (frnum >= 6 && frnum <= 10)
				index = 0x60c; // ruined painting
			else
				index = shnum;
			break;
		case 0x11d:      // brush
			if (frnum == 6)
				index = 0x5fa;
			else
				index = shnum;
			break;
		case 0x11e:      // drum
			if (frnum == 1 || frnum == 2)
				index = 0x669; // drumsticks
			else
				index = shnum;
			break;
		case 0x124:      // seat
			if (frnum == 17 || frnum == 18 || frnum == 19 || frnum == 22)
				index = 0x5fc;
			else
				index = shnum;
			break;
		case 0x128:			// Rings
			index = 0x5b8 + frnum;
			break;
		case 0x12c:         // cooking utensil
			if (frnum <= 7)
				index = 0x636 + (frnum/2);
			else if (frnum >= 8 && frnum <= 23)
				index = 0x63a + (frnum%4);
			else if (frnum >= 24)
				index = 0x63e + (frnum%2);
			break;
		case 0x12f:
			if (frnum >= 10 && frnum <= 14)
				index = 0x60b;
			else
				index = shnum;
			break;
		case 0x135:        // fallen tree
			if (frnum == 2)
				index = 0x635; // large skull
			else
				index = shnum;
			break;
		case 0x141:         // reeds
			if (frnum == 11)
				index = 0x61c; // crucifix
			else
				index = shnum;
			break;
		case 0x14a:         // serpent rune
			if (frnum < 8)
				index = 0x607; // stones
			else
				index = shnum;
			break;
		case 0x150:         // light source
			index = 0x646 + frnum;
			break;
		case 0x152:         // lit light source
			{
			if (num_item_names <= 0x657)
				break;
			string tmp = item_names[0x657]; // lit
			tmp += item_names[0x646 + frnum];
			return tmp;
			}
			break;
		case 0x172:         // floor
			if (frnum == 19)
				index = 0x60e; // wall
			else
				index = shnum;
			break;
		case 0x179:			// Food items
			index = 0x510 + frnum;
			break;
		case 0x17a:         // fence
			if (frnum == 3 || (frnum > 7 && frnum < 12))
				index = 0x60e; // wall
			else
				index = shnum;
			break;
		case 0x17f:         // magic helm
			if (frnum == 1)
				index = 0x614; // helm of courage
			else
				index = shnum;
			break;
		case 0x194:         // food
			index = 0x640 + frnum;
			break;
		case 0x19f:         // garbage
			if (frnum > 20 && frnum < 24)
				index = 0x60f; // skull
			else
				index = shnum;
			break;
		case 0x1ba:         // fireplace
			if (frnum == 2)
				index = 0x620; // wooden post
			else
				index = shnum;
			break;
		case 0x1bd:			// Soul prisms
			index = 0x56b + frnum;
			break;
		case 0x1c2:			// Orbs
			index = 0x585 + frnum;
			break;
		case 0x1d3:			// Magic plants
			index = 0x581 + frnum/2;
			break;
		case 0x1db:         // wall of lights
			if (frnum == 10)
				index = 0x683; // wall
			else
				index = shnum;
			break;
		case 0x1df:         // claw + gwani amulet
			index = 0x5f8 + frnum/2;
			break;
		case 0x1e3:         // rug
			if (frnum >= 2)
				index = 0x608; // meditation mat
			else
				index = shnum;
			break;
		case 0x1f9:         // magic lens
			if (frnum == 2)
				index = 0x615; // lens of translating
			else
				index = shnum;
			break;
		case 0x201:         // floor
			if (frnum == 5)
				index = 0x60d; // bookshelf
			else
				index = shnum;
			break;
		case 0x206:         // glass counter top
			if (frnum == 26)
				index = 0x682; // table
			else
				index = shnum;
			break;
		case 0x207:         // Moon's Eye
			if (frnum == 1)
				index = 0x616; // crystal ball
			else
				index = shnum;
			break;
		case 0x21a:         // fence
			if (frnum == 3 || (frnum > 7 && frnum < 12))
				index = 0x60e; // wall
			else
				index = shnum;
			break;
		case 0x21e:			// Crested Helmets
			index = 0x5a3 + frnum;
			break;
		case 0x220:         // broken column
			if (frnum == 14)
				index = 0x611; // pillar of purity
			else
				index = shnum;
			break;
		case 0x241:			// Nests
			if (frnum < 6)
				index = 0x5ab + frnum/3;
			else
				index = shnum;
			break;
		case 0x24b:			// Boots
			index = 0x59c + frnum;
			break;
		case 0x268:        // Bottles
			switch (frnum) {
			case 1:
				index = 0x5f2; // wine decanter
				break;
			case 9:
				index = 0x5f3; // fawnish ale
				break;
			case 16:
				index = 0x5f4; // ice wine
				break;
			case 17:
				index = 0x5f5; // vintage wine
				break;
			case 18:
				index = 0x5f6; // wineskin
				break;
		    case 20:
				index = 0x5f7; // everlasting goblet
				break;
			default:
				index = shnum;
			}
			break;
		case 0x281:         // key
			switch (frnum) {
			case 21:
				index = 0x630; // key of fire
				break;
			case 22:
				index = 0x631; // key of ice
				break;
			case 23:
				index = 0x62f; // blackrock key
				break;
			default:
				index = shnum;
			}
			break;
		case 0x288:         // sleeping powder
			if (frnum >= 2)
				index = 0x65c - 2 + frnum;
			else
				index = shnum;
			break;
		case 0x289:			// Artifacts
			index = 0x573 + frnum;
			break;
		case 0x28a:			// Sextants
			index = 0x531 - frnum;
			break;
		case 0x290:         // wine press
			if (frnum == 1)
				index = 0x634; // wine vat
			else
				index = shnum;
			break;
		case 0x2a1:         // strange plant
			if (frnum > 8)
				index = 0x62c; // snowy plant
			else
				index = shnum;
			break;
		case 0x2a3:			// Desk items
			index = 0x532 + frnum;
			break;
		case 0x2a5:         // stockings
			if (frnum < 2)
				index = 0x62a + frnum;
			else
				index = shnum;
			break;
		case 0x2af:         // pillar
			if (frnum == 11)
				index = 0x611; // pillar of purity
			else
				index = shnum;
			break;
		case 0x2b0:         // serpent carving
			if (frnum > 1)
				index = 0x621 - 1 + (frnum/2);
			else
				index = shnum;
			break;
		case 0x2b4:			// Lute
			if (frnum == 2)
				index = 0x5be;
			else
				index = shnum;
			break;
		case 0x2b5:        // whistle
			if (frnum == 1)
				index = 0x617; // whistle
			else if (frnum == 4 || frnum == 5)
				index = 0x618; // bone flute
			else
				index = shnum;
			break;
		case 0x2b8:        // bed
			if (frnum == 1)
				index = 0x5da; // stone bier
			else if (frnum == 2)
				index = 0x5db; // cot
			else if (frnum == 4)
				index = 0x5dc; // pallet
			else if (frnum == 5)
				index = 0x5dd; // fur pallet
			else
				index = shnum;
			break;
		case 0x2cd:        // plate
			if (frnum == 9)
				index = 0x61a; // platter of replenishment
			else
				index = shnum;
			break;
		case 0x2ce:        // pedestal
			if (frnum == 8)
				index = 0x619; // blackrock obelisk
			else
				index = shnum;
			break;
		case 0x2d7:        // Dream Crystal
			if (frnum <= 1)
				index = 0x659; // dream crystal
			else if (frnum == 2)
				index = 0x65a; // mirror rock
			else if (frnum >= 3)
				index = 0x65b; // icy column
			break;
		case 0x2d8:        // Force_Wall
			// TODO: CHECK
			if (frnum > 0)
				index = 0x683; // wall
			else
				index = shnum;
			break;
		case 0x2f4:        // caltrops
			if (frnum == 2 || frnum == 3)
				index = 0x5fb; // broken glass
			else
				index = shnum;
			break;
		case 0x313:        // lever
			if (frnum == 8 || frnum == 9)
				index = 0x613; // button
			else
				index = shnum;
			break;
		case 0x314:        // switch
			if (frnum >= 6 && frnum <= 9)
				index = 0x613; // button
			else
				index = shnum;
			break;
		case 0x31f:        // Body parts
			index = 0x5df + frnum;
			break;
		case 0x320:      // chest
			if (frnum == 4 || frnum == 5)
				index = 0x5fd;
			else if (frnum == 6 || frnum == 7)
				index = 0x5fe;
			else
				index = shnum;
			break;
		case 0x32a:			// Bucket
			if (frnum == 1 && qual >= 10 && qual <= 15)
				index = 0x55b + qual;
			else if (frnum == 2 && qual == 9)
				index = 0x55b + qual;
			else
				index = 0x55b + frnum;
			break;
		case 0x347:			// Hourglass
			if (frnum == 1)
				index = 0x5bf;
			else
				index = shnum;
			break;
		case 0x34a:			// Reagents
			index = 0x500 + frnum;
			break;
		case 0x35f:         // kitchen items
			index = 0x66b + frnum;
			break;
		case 0x365:         // wall
			if (frnum == 11)
				index = 0x610; // spirit wall
			else
				index = shnum;
			break;
		case 0x36c:         // sliding door
			if (frnum >= 10 && frnum <= 14)
				index = 0x60b; // iris door
			else
				index = shnum;
			break;
		case 0x377:			// More rings
			index = 0x5bc + frnum;
			break;
		case 0x37a:         // table
			if (frnum == 6)
				index = 0x62e; // torture table
			else
				index = shnum;
			break;
		case 0x390:         // blood
			if (frnum == 24)
				index = 0x5de; // acid
			else if (frnum < 4)
				index = 0x390; // blood
			else
				index = -1;
			break;
		case 0x392:         // urn
			if (frnum <= 1) {
				// when frame = 0,1, the quality is an NPC num
				// the item name is then the name of that NPC + "'s ashes"
				// (quality == 0,255: 'urn of ashes')
				Actor* npc = gwin->get_npc(get_quality());
				if (get_quality() > 0 && npc && !npc->is_unused()) {
					string tmp = npc->get_npc_name();
					if (tmp != "" && num_item_names>0x65e) {
						tmp += item_names[0x65e]; // 's ashes
						return tmp;
					}
				}
				index = 0x632; // urn with ashes
			} else if (frnum == 2)
				index = 0x633; // pot of unguent
			else
				index = shnum;
			break;
		case 0x397:         // bookshelf
			if (frnum == 6)
				index = 0x60a; // slate mantle
			else
				index = shnum;
			break;
		case 0x39f:         // serpent slot
			if (frnum >= 4 && frnum <= 7)
				index = 0x61e; // slotted serpent
			else
				index = shnum;
			break;
		case 0x3a7:         // sliding door
			if (frnum == 2)
				index = 0x60b; // iris door
			else
				index = shnum;
			break;
		case 0x3a8:         // sliding door
			if (frnum == 2)
				index = 0x60b; // iris door
			else
				index = shnum;
			break;
		case 0x3b0:         // pot
			if (frnum == 5)
				index = 0x61b; // cuspidor
			else
				index = shnum;
			break;
		case 0x3bb:			// Amulets
			index = 0x5ae + frnum;
			break;
		case 0x3d1:         // fur pelt
			if (frnum == 0 || frnum == 8)
				index = 0x61f; // leopard rug
			else
				index = shnum;
			break;
		case 0x3e5:         // spent light source
			{
			if (num_item_names <= 0x658)
				break;
			string tmp = item_names[0x658]; // spent
			tmp += item_names[0x646 + frnum];
			return tmp;
			}
			break;
		case 0x3e6:         // statue
			if (frnum <= 3)
				index = 0x628 + (frnum/2);
			else
				index = shnum;
			break;
		case 0x3e7:         // plant
			index = 0x65f + frnum;
			break;
		case 0x3ea:         // table
			if (frnum == 1)
				index = 0x62d; // embalming table
			else
				index = shnum;
			break;
 		case 0x3ec:			// Helmets
			index = 0x5a6 + frnum;
			break;
		case 0x3f3:			// Beds
			if (frnum == 1)
				index = 0x5da; // stone bier
			else if (frnum == 2)
				index = 0x5db; // cot
			else if (frnum == 4)
				index = 0x5dc; // pallet
			else if (frnum == 5)
				index = 0x5dd; // fur pallet
			else
				index = shnum;
			break;
		case 0x3f8:         // floor
			if (frnum == 20 || frnum == 21)
				index = 0x623; // symbol of balance
			else if (frnum == 22)
				index = 0x624; // symbol of order
			else if (frnum == 23)
				index = 0x625; // symbol of chaos
			else
				index = shnum;
			break;
		default:
			index = shnum;
			break;
		}

		// no Silver Seed?
		if (!GAME_SS) {
			if (index >= 0x605) {
				index -= 11;
			} else if (index >= 0x5bc) {
				index -= 10;
			} else if (index >= 0x5b8) {
				index -= 9;
			} else if (index >= 0x59c) {
				index -= 8;
			} else if (index >= 0x592) {
				index -= 7;
			} else if (index >= 0x581) {
				index -= 2;
			}
		}

	} else {
		index = shnum;
	}

	const char *name = (index >= 0 && index < num_item_names) ?
						item_names[index] : 0;
	if(name == 0) {
		return "";
//		return "?";
	}

    if (ShapeID::get_info(shnum).has_quantity())
		quantity = quality & 0x7f;
	else
		quantity = 1;

	// If there are no slashes then it is simpler
	if(strchr(name, '/') == 0)
		{
		if(quantity <= 1)
			display_name = name;
		else
			{
			char buf[50];

			snprintf(buf, 50, "%d %s", quantity, name);
			display_name = buf;
			}
		}
	else if(quantity <= 1)		// quantity might be zero?
		get_singular_name(name, display_name);
	else
		get_plural_name(name, quantity, display_name);
	return display_name;
}

