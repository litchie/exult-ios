/*
Copyright (C) 2000 The Exult Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "game.h"
#include "gamewin.h"
#include "Paperdoll_gump.h"
#include "actors.h"
#include "exult_bg_flx.h"

/*
 *
 *	SERPENT ISLE GUMPS
 *
 */

/*
 *
 *	NPC and Item Info Statics
 *
 */

Paperdoll_gump::Paperdoll_npc Paperdoll_gump::Characters[] = {

	// Shp, fmale,                   bsh, bf, hds, hf, hh, ash, af, a2, as
	
	// Avatar Female White
	{ 1029,  true, SF_PAPERDOL_VGA,   13,  0,   4,  0,  1,   7,  0,  2,  4 },
	{ 1035,  true, SF_PAPERDOL_VGA,   13,  0,   4,  0,  1,   7,  0,  2,  4 },
	{  989,  true, SF_PAPERDOL_VGA,   13,  0,   4,  0,  1,   7,  0,  2,  4 },

	// Avatar Female Brown
	{ 1027,  true, SF_PAPERDOL_VGA,   13,  1,   4,  2,  3, 130,  0,  2,  4 },
	{ 1033,  true, SF_PAPERDOL_VGA,   13,  1,   4,  2,  3, 130,  0,  2,  4 },

	// Avatar Female Black
	{ 1025,  true, SF_PAPERDOL_VGA,   13,  2,   4,  4,  5, 131,  0,  2,  4 },
	{ 1031,  true, SF_PAPERDOL_VGA,   13,  2,   4,  4,  5, 131,  0,  2,  4 },
	
	// Avatar Male White
	{ 1028, false, SF_PAPERDOL_VGA,   14,  0, 128,  0,  1,   7,  1,  3,  5 },
	{ 1034, false, SF_PAPERDOL_VGA,   14,  0, 128,  0,  1,   7,  1,  3,  5 },
	{  721, false, SF_PAPERDOL_VGA,   14,  0, 128,  0,  1,   7,  1,  3,  5 },

	// Avatar Male Brown
	{ 1026, false, SF_PAPERDOL_VGA,   14,  1, 128,  2,  3, 130,  1,  3,  5 },
	{ 1032, false, SF_PAPERDOL_VGA,   14,  1, 128,  2,  3, 130,  1,  3,  5 },

	// Avatar Male Black
	{ 1024, false, SF_PAPERDOL_VGA,   14,  2, 128,  4,  5, 131,  1,  3,  5 },
	{ 1030, false, SF_PAPERDOL_VGA,   14,  2, 128,  4,  5, 131,  1,  3,  5 },

	// Iolo
	{  465, false, SF_PAPERDOL_VGA,   14,  0, 125,  0,  1,   7,  1,  3,  5 },

	// Shamino
	{  487, false, SF_PAPERDOL_VGA,   14,  0, 126,  0,  1,   7,  1,  3,  5 },

	// Dupre
	{  488, false, SF_PAPERDOL_VGA,   14,  0, 124,  0,  1,   7,  1,  3,  5 },

	// Automaton
	{  747, false, SF_PAPERDOL_VGA,   14,  3, 145,  0,  1, 146,  1,  3,  5 },

	// Boyden
	{  815, false, SF_PAPERDOL_VGA,   14,  0, 158,  0,  1,   7,  1,  3,  5 },

	// Petra
	{  658,  true, SF_PAPERDOL_VGA,   13,  3, 137,  0,  1, 136,  0,  2,  4 },

	// Gwenno
	{  669,  true, SF_PAPERDOL_VGA,   13,  0, 127,  0,  1,   7,  0,  2,  4 },

	// Sethys
	{  817, false, SF_PAPERDOL_VGA,   14,  0, 159,  0,  1,   7,  1,  3,  5 },

	// Wilfred
	{  816, false, SF_PAPERDOL_VGA,   14,  0, 154,  0,  1,   7,  1,  3,  5 },

	// Selina
	{  652,  true, SF_PAPERDOL_VGA,   13,  0, 155,  0,  1,   7,  0,  2,  4 },

	// Mortegro
	{  809, false, SF_PAPERDOL_VGA,   14,  0, 156,  0,  1,   7,  1,  3,  5 },

	// Stefano
	{  451, false, SF_PAPERDOL_VGA,   14,  0, 157,  0,  1, 130,  1,  3,  5 },
	
	// Terminator
	{ 0 }
};

Paperdoll_gump::Paperdoll_item Paperdoll_gump::Items[] =
{
	// wshape, wf,        Equip spot, Object Type, Gender, shape, frame, frame2,  f3,  f4

	// MISC ITEMS
	
	// Usecode container (not drawn normally)
	{     486, -1, Actor::ucont_spot,   OT_Normal,   false, SF_PAPERDOL_VGA,     6,     0 },
	// Bed roll
	{     583, -1,       Actor::back,   OT_Normal,   false, SF_PAPERDOL_VGA,      9,     0 },
	{     583, -1,Actor::back2h_spot,   OT_Normal,   false, SF_PAPERDOL_VGA,      9,     1 },
	// Kidney Belt
	{     584, -1,       Actor::belt,   OT_Normal,   true, SF_PAPERDOL_VGA,     54,     0 },
	// Serpent Earings
	{     635, -1,  Actor::ears_spot,   OT_Normal,   true, SF_PAPERDOL_VGA,     39,     0 },
	// Backpack
	{     801, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,      6,     0 },
	// Bag
	{     802, -1,       Actor::belt,   OT_Normal,  false, SF_PAPERDOL_VGA,     89,     0 },
	// Belt of Strength
	{     996, -1,       Actor::belt,   OT_Normal,   true, SF_PAPERDOL_VGA,    166,     0 },
	// Spell book
	{     761, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     15,     0 },
	// Lit Torch
	{     701, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     63,     0 },
	{     701, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,     63,     1 },
	// Torch
	{     595, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    113,     0 },
	{     595, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    113,     1 },
	{     595, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    113,     2 },
	// Serpent Sceptre
	{     926, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    144,     0 },
	{     926, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    144,     1 },
	// Body
	{     400, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     0 },
	{     400, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     1 },
	{     402, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     0 },
	{     402, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     1 },
	{     414, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     0 },
	{     414, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     1 },


	// AMULETS

	// Ankh
	{     955,  0,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     0 },
	// Fellowship
	{     955,  1,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     1 },
	// Serpent Neclace
	{     955,  2,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     2 },
	// Amulet of Protection
	{     955,  3,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     3 },
	// Batlin's
	{     955,  4,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     4 },
	// Diamond Necklace
	{     955,  5,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     5 },
	// Necklace
	{     955,  6,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     6 },
	// Necklace of Bones
	{     955,  7,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     7 },
	// White Diamond Necklace
	{     955,  8,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     8 },
	// Amulet of Balance
	{     955,  9,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     9 },


	// CLOAKS
	
	// Red
	{     227,  0, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,      28,    0,     -1 },
	{     227,  0, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,      28,    1,     -1 },
	// Bear
	{     227,  1, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,       8,    0,     -1 },
	{     227,  1, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,       8,    1,     -1 },
	// Snow Leopard
	{     227,  2, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,      61,    0,     -1 },
	{     227,  2, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,      61,    1,     -1 },
	// Wolf
	{     227,  3, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,     116,    0,     -1 },
	{     227,  3, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,     116,    1,     -1 },
	// Gwani
	{     227,  4, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,     164,    0,     -1 },
	{     227,  4, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,     164,    1,     -1 },
	// 'Cloak'
	{     403,  0, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,      28,    0,     -1 },
	{     403,  0, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,      28,    1,     -1 },


	// ARMOUR
	
	// Breast Plate
	{     419, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,    140,     0,      2,   4,  6 },
	// Leather
	{     569, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     57,     0,      2,   4,  6 },
	// Scale
	{     570, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     92,     0,      2,   4,  6 },
	// Chain
	{     571, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     22,     0,      2,   4,  6 },
	// Plate
	{     573, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     85,     0,      2,   4,  6 },
	// Serpent
	{     638, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     94,     0,      2,   4,  6 },
	// Magic
	{     666, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,      2,     0,      2,   4,  6 },
	// Antique
	{     836, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,      1,     0,      2,   4,  6 },


	// HELMS

	// Magic
	{     383, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     74,     0 },
	// Helm Of Courage
	{     383, 1,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,    162,     0 },
	// Chain Coif
	{     539, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     24,     0 },
	// Great Helm
	{     541, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     30,     0 },
	// Crested Helm
	{     542, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     31,     0 },
	// Plummed Helm
	{     542, 1,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     88,     0 },
	// Viking Helm
	{     542, 2,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,    114,     0 },
	// Leather Helm
	{    1004, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     59,     0 },
	// Fur Cap
	{    1004, 1,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,    129,     0 },
	// Serpent Crown
	{    1004, 2,        Actor::head,   OT_Normal,   true, SF_PAPERDOL_VGA,     33,     0 },
	// Living Toupe
	{    1004, 3,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,    165,     0 },
	// Womans Fur Cap
	{    1004, 4,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,    129,     0 },
	// Helm of Light
	{    1013, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,    169,     0 },

	
	// LEGGINGS

	// Leather
	{     574, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Chain
	{     575, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     26,     0 },
	// Plate
	{     576, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     87,     0 },
	// Fine Stockings
	{     677, 0,         Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,    152,     0 },
	// Fishnet Stockings
	{     677, 1,         Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,    153,     0 },
	// Magic Leggings
	{     686, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     72,     0 },

	
	// GLOVES

	// Leather
	{     579, -1, Actor::hands2_spot,  OT_Normal,  false, SF_PAPERDOL_VGA,     47,     0,      1,   2 },
	// Gauntlets
	{     580, -1, Actor::hands2_spot,  OT_Normal,  false, SF_PAPERDOL_VGA,     25,     0,      1,   2 },
	// Magic Gauntlets
	{     835, -1, Actor::hands2_spot,  OT_Normal,  false, SF_PAPERDOL_VGA,     70,     0,      1,   2 },
	// Gauntlets of Quickness
	{    1001, -1, Actor::hands2_spot,  OT_Normal,  false, SF_PAPERDOL_VGA,    167,     0,      1,   2 },


	// RINGS

	// Invisibilty Ring
	{     296, 0,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    120,     0,      2,   4 },
	{     296, 0,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    120,     1,      3,   5 },
	// Ring of Regeneration
	{     296, 1,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    122,     0,      2,   4 },
	{     296, 1,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    122,     1,      3,   5 },
	// Blink Ring
	{     296, 2,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    148,     0,      2,   4  },
	{     296, 2,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    148,     1,      3,   5 },
	// Ring of Reagents
	{     296, 3,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    148,     0,      2,   4 },
	{     296, 3,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    148,     1,      3,   5 },
	// Ring
	{     887, 0,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    147,     0,      2,   4 },
	{     887, 0,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    147,     1,      3,   5 },
	// Serpent Ring
	{     887, 1,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    149,     0,      2,   4 },
	{     887, 1,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    149,     1,      3,   5 },


	// BOOTS
	
	// Leather
	{     587, 0,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     58,     0 },
	// Magic
	{     587, 1,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     73,     0 },
	// Armoured
	{     587, 2,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     86,     0 },
	// Fur
	{     587, 3,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     46,     0 },
	// Black
	{     587, 4,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,    150,     0 },
	// Slippers
	{     587, 5,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,    151,     0 },
	// Swamp
	{     587, 6,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,    106,     0 },
	

	// SHIELDS (behind back = 99)

	// Dupre's
	{     490, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,    141,     0 },
	{     490, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Buckler
	{     543, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     20,     0 },
	{     543, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Curved Heater
	{     545, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     34,     0 },
	{     545, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Wooden
	{     572, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,    117,     0 },
	{     572, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Spiked
	{     578, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,    104,     0 },
	{     578, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Door
	{     585, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     38,     0 },
	{     585, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Brass
	{     586, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     19,     0 },
	{     586, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Kite
	{     609, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     37,     0 },
	{     609, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Magic
	{     663, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     75,     0 },
	{     663, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Shield of Monitor
	{     729, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,    142,     0 },
	{     729, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },

	// WEAPONS 1 H
	
	// Magebane
	{     231, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     67,     0},
	{     231, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     67,     1},
	// Spear
	{     592, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,    103,     0 },
	{     592, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,    103,     1 },
	// Blowgun
	{     563, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     12,     0 },
	{     563, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     12,     1 },
	// Boomerang
	{     605, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     16,     0 },
	{     605, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     16,     1 },
	// Club
	{     590, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     29,     0 },
	{     590, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     29,     1 },
	// Dagger
	{     594, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     35,     0 },
	{     594, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     35,     1 },
	// Poisoned Dagger
	{     564, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     36,     0 },
	{     564, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     36,     1 },
	// Fire Sword
	{     551, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     41,     0 },
	{     551, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     41,     1 },
	// Fire Wand
	{     630, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     42,     0 },
	{     630, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     42,     1 },
	// Glass Sword
	{     604, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     48,     0 },
	{     604, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     48,     1 },
	// Hammer
	{     623, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     50,     0 },
	{     623, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     50,     1 },
	// Lightning Whip
	{     549, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     62,     0 },
	{     549, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     62,     1 },
	// Mace
	{     659, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     66,     0 },
	{     659, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     66,     1 },
	// Magic Sword
	{     547, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     76,     0 },
	{     547, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     76,     1 },
	// Magic Axe
	{     552, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     77,     0 },
	{     552, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     77,     1 },
	// Magician's Wand
	{     792, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     78,     0 },
	{     792, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     78,     1 },
	// Morning Star
	{     596, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     81,     0 },
	{     596, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     81,     1 },
	// Serpentine Dagger
	{     636, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     95,     0 },
	{     636, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     95,     1 },
	// Serpent Sword
	{     637, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     97,     0 },
	{     637, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     97,     1 },
	// Ophidian Sword
	{     710, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     97,     0 },
	{     710, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     97,     1 },
	// Shears 
	{     698, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     98,     0 },
	{     698, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     98,     1 },
	// Magic Sling
	{     474, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    102,     0 },
	{     474, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    102,     1 },
	// Sword
	{     599, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    107,     0 },
	{     599, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    107,     1 },
	// Decorative Sword
	{     608, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    109,     0 },
	{     608, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    109,     1 },
	// Sword of Defense
	{     567, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    110,     0 },
	{     567, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    110,     1 },
	// Throwing Axe
	{     593, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    111,     0 },
	{     593, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    111,     1 },
	// Tongs
	{     994, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    112,     0 },
	{     994, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    112,     1 },
	// Whip
	{     622, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    115,     0 },
	{     622, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    115,     1 },
	// Lightning Wand
	{     629, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    118,     0 },
	{     629, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    118,     1 },
	// Rudyom's Wand
	{     771, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    134,     0 },
	{     771, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    134,     1 },
	// Dragon Slayer Sword
	{     535, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    143,     0 },
	{     535, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    143,     1 },
	// Wooden Sword
	{     520, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    161,     0 },
	{     520, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    161,     1 },
	// Hammer
	{     508, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    163,     0 },
	{     508, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    163,     1 },
	// Erinons Axe
	{     990, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    168,     0 },
	{     990, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    168,     1 },

	
	// WEAPONS 2H

	// The Hammer of Dedication
	{     942, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,    138,     0 },
	{     942, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,    138,     1 },
	// Two Handed Sword
	{     602, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,    108,     0 },
	{     602, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,    108,     1 },
	// Staff
	{     241, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,    105,     0 },
	{     241, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,    105,     1 },
	// Shovel
	{     625, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,    100,     0 },
	{     625, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,    100,     1 },
	// Scythe
	{     618, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     93,     0 },
	{     618, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     93,     1 },
	// Rake
	{     620, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     91,     0 },
	{     620, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     91,     1 },
	// Pitchfork
	{     589, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     84,     0 },
	{     589, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     84,     1 },
	// Pick
	{     624, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,     83,     0 },
	{     624, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,     83,     1 },
	// Juggernaught Hammer
	{     557, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,     53,     0 },
	{     557, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,     53,     1 },
	// Hoe
	{     626, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     52,     0 },
	{     626, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     52,     1 },
	// Halberd
	{     603, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     49,     0 },
	{     603, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     49,     1 },
	// Fishing Rod
	{     662, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     44,     0 },
	{     662, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     44,     1 },
	// Firedoom Staff
	{     553, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     43,     0 },
	{     553, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     43,     1 },
	// Two Handed Axe
	{     601, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,      5,     0 },
	{     601, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,      5,     1 },
	// Serpent Staff
	{     640, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,      96,     0 },
	{     640, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,      96,     1 },
	// Two Handed Hammer
	{     600, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,      51,     0 },
	{     600, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,      51,     1 },
	// The Black Sword
	{     806, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,     139,     0 },
	{     806, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,     139,     1 },

	// CROSSBOWS & BOLTS

	// Magic Bolts
	{     417, 25,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    2,      2, 0 }, // 2
	{     417, 26,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    3,      2, 0 }, // 3
	{     417, 27,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    3,      3, 0 }, 
	{     417, 28,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      3, 0 },
	{     417, 29,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      4, 0 },
	{     417, 30,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      4, 0 },
	{     417, 31,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      4, 0 },
	{     417, -1,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    2,      1, 0 }, // 1
	// Bolts
	{     723, 25,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    2,      2, 0 },
	{     723, 26,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    3,      2, 0 },
	{     723, 27,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    3,      3, 0 },
	{     723, 28,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      3, 0 },
	{     723, 29,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     723, 30,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     723, 31,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     723, -1,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    2,      1, 0 },
	// Crossbow
	{     598, -1,      Actor::lhand, OT_Crossbow,  false, SF_PAPERDOL_VGA,      32,    0 },
	{     598, -1,       Actor::belt, OT_Crossbow,  false, SF_PAPERDOL_VGA,      32,    1 },

	
	// BOWS & ARROWS

	// Burst Arrows
	{     554, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    2,      2, 0 },
	{     554, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    3,      2, 0 },
	{     554, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    3,      3, 0 },
	{     554, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      3, 0 },
	{     554, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      4, 0 },
	{     554, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      4, 0 },
	{     554, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      4, 0 },
	{     554, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    2,      1, 0 },
	// Magic Arrows
	{     556, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    2,      2, 0 },
	{     556, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    3,      2, 0 },
	{     556, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    3,      3, 0 },
	{     556, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      3, 0 },
	{     556, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      4, 0 },
	{     556, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      4, 0 },
	{     556, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      4, 0 },
	{     556, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    2,      1, 0 },
	// Lucky Arrows
	{     558, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    2,      2, 0 },
	{     558, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    3,      2, 0 },
	{     558, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    3,      3, 0 },
	{     558, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      3, 0 },
	{     558, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      4, 0 },
	{     558, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      4, 0 },
	{     558, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      4, 0 },
	{     558, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    2,      1, 0 },
	// Sleep Arrows
	{     568, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    2,      2, 0 },
	{     568, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    3,      2, 0 },
	{     568, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    3,      3, 0 },
	{     568, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      3, 0 },
	{     568, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      4, 0 },
	{     568, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      4, 0 },
	{     568, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      4, 0 },
	{     568, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    2,      1, 0 },
	// Serpent Arrows
	{     591, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    2,      2, 0 },
	{     591, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    3,      2, 0 },
	{     591, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    3,      3, 0 },
	{     591, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      3, 0 },
	{     591, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      4, 0 },
	{     591, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      4, 0 },
	{     591, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      4, 0 },
	{     591, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    2,      1, 0 },
	// Arrows
	{     722, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    2,      2, 0 },
	{     722, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    3,      2, 0 },
	{     722, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    3,      3, 0 },
	{     722, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      3, 0 },
	{     722, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      4, 0 },
	{     722, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      4, 0 },
	{     722, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      4, 0 },
	{     722, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    2,      1, 0 },
	// Bow
	{     597, -1,      Actor::lhand,      OT_Bow,  false, SF_PAPERDOL_VGA,     17,     0 },
	{     597, -1,Actor::back2h_spot,      OT_Bow,  false, SF_PAPERDOL_VGA,     17,     1 },
	// Magic Bow
	{     606, -1,      Actor::lhand,      OT_Bow,  false, SF_PAPERDOL_VGA,     69,     0 },
	{     606, -1,Actor::back2h_spot,      OT_Bow,  false, SF_PAPERDOL_VGA,     69,     1 },
	// Infinity Bow
	{     711, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    160,     0 },
	{     711, -1,Actor::back2h_spot,   OT_Single,  false, SF_PAPERDOL_VGA,    160,     1 },

	// Terminator
	{ 0 }
};

//
// Black Gate
//

Paperdoll_gump::Paperdoll_npc Paperdoll_gump::Characters_BG[] = {

	// Shp, fmale, head_file,       bsh, bf, hds, hf, hh,  ash, af, a2, as
	
	// Avatar Female White
	{ 1029,  true, SF_PAPERDOL_VGA,   13,  0,   4,  0,  1,   7,  0,  2,  4 },
	{ 1035,  true, SF_PAPERDOL_VGA,   13,  0,   4,  0,  1,   7,  0,  2,  4 },
	{  989,  true, SF_GAME_FLX,      13,  0, EXULT_BG_FLX_FEM_AV_FACE_SHP,
	                                               0,  1,   7,  0,  2,  4 },
	// Avatar Female Brown
	{ 1027,  true, SF_PAPERDOL_VGA,   13,  1,   4,  2,  3, 130,  0,  2,  4 },
	{ 1033,  true, SF_PAPERDOL_VGA,   13,  1,   4,  2,  3, 130,  0,  2,  4 },

	// Avatar Female Black
	{ 1025,  true, SF_PAPERDOL_VGA,   13,  2,   4,  4,  5, 131,  0,  2,  4 },
	{ 1031,  true, SF_PAPERDOL_VGA,   13,  2,   4,  4,  5, 131,  0,  2,  4 },
	
	// Avatar Male White
	{ 1028, false, SF_PAPERDOL_VGA,   14,  0, 128,  0,  1,   7,  1,  3,  5 },
	{ 1034, false, SF_PAPERDOL_VGA,   14,  0, 128,  0,  1,   7,  1,  3,  5 },
	{  721, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_MALE_AV_FACE_SHP,
	                                               0,  1,   7,  1,  3,  5 },
	// Avatar Male Brown
	{ 1026, false, SF_PAPERDOL_VGA,   14,  1, 128,  2,  3, 130,  1,  3,  5 },
	{ 1032, false, SF_PAPERDOL_VGA,   14,  1, 128,  2,  3, 130,  1,  3,  5 },

	// Avatar Male Black
	{ 1024, false, SF_PAPERDOL_VGA,   14,  2, 128,  4,  5, 131,  1,  3,  5 },
	{ 1030, false, SF_PAPERDOL_VGA,   14,  2, 128,  4,  5, 131,  1,  3,  5 },

	// Iolo
	{  465, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_IOLO_FACE_SHP,  
	                                               0,  1,   7,  1,  3,  5 },
	// Shamino
	{  487, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_SHAMINO_FACE_SHP,  
	                                               0,  1,   7,  1,  3,  5 },
	// Dupre
	{  488, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_DUPRE_FACE_SHP,  
	                                               0,  1,   7,  1,  3,  5 },
	// Jaana
	{  490,  true, SF_GAME_FLX,      13,  0, EXULT_BG_FLX_FACES_SHP,  
	                                               0,  1,   7,  0,  2,  4 },
	// Sentri
	{  462, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_FACES2_SHP,  
	                                               0,  1,   7,  1,  3,  5 },
	// Julia
	{  454,  true, SF_GAME_FLX,      13,  0, EXULT_BG_FLX_FACES_SHP,  
	                                               2,  3,   7,  0,  2,  4 },
	// Katrina
	{  452,  true, SF_GAME_FLX,      13,  0, EXULT_BG_FLX_FACES_SHP,  
	                                               4,  5,   7,  0,  2,  4 },
	// Tseramed
	{  460, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_FACES2_SHP,  
	                                               2,  3,   7,  1,  3,  5 },
	// Spark
	{  489, false, SF_GAME_FLX,      14,  0, EXULT_BG_FLX_FACES2_SHP,  
	                                               4,  5,   7,  1,  3,  5 },
	// Female Ghost/Rowena
	{  299,  true, SF_GAME_FLX,      13,  0, EXULT_BG_FLX_FEM_AV_FACE_SHP,
	                                               2,  1,   7,  0,  2,  4 },


	// For Pick pocket mode

	// Mage
	{  154, false, SF_PAPERDOL_VGA,  14,  0, 125,  0,  1,   7,  1,  3,  5 },
	// Ferry man
	{  155, false, SF_PAPERDOL_VGA,  14,  0, 156,  0,  1,   7,  1,  3,  5 },
	// flying gargoyle
	{  226, false, SF_PAPERDOL_VGA,  14,  0, 156,  0,  1,   7,  1,  3,  5 },
	// Allanger
	{  227, false, SF_PAPERDOL_VGA,  14,  0, 159,  0,  1,   7,  1,  3,  5 },
	// Papa
	{  228, false, SF_PAPERDOL_VGA,  14,  0, 159,  0,  1,   7,  1,  3,  5 },
	// Mama
	{  229,  true, SF_PAPERDOL_VGA,  13,  0,   4,  0,  1,   7,  0,  2,  4 },
	// Paladin
	{  247, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Fighter
	{  259, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Townsman
	{  259, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Blacksmith
	{  304, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Ghost
	{  317, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Sage
	{  318, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Peasant
	{  319, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Ghost
	{  337, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Liche
	{  354, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Kissme
	{  382,  true, SF_PAPERDOL_VGA,  13,  0, 127,  0,  1,   7,  0,  2,  4 },
	// Guard
	{  394, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Pirate
	{  401, false, SF_PAPERDOL_VGA,  14,  0, 126,  0,  1,   7,  1,  3,  5 },
	// Batlin
	{  403, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Mage
	{  445, false, SF_PAPERDOL_VGA,  14,  0, 125,  0,  1,   7,  1,  3,  5 },
	// Mage
	{  446,  true, SF_PAPERDOL_VGA,  13,  0,   4,  0,  1,   7,  0,  2,  4 },
	// Sage
	{  448,  true, SF_PAPERDOL_VGA,  13,  0, 155,  0,  1,   7,  0,  2,  4 },
	// Beggar
	{  449, false, SF_PAPERDOL_VGA,  14,  0, 154,  0,  1,   7,  1,  3,  5 },
	// Beggar
	{  450, false, SF_PAPERDOL_VGA,  14,  0, 154,  0,  1,   7,  1,  3,  5 },
	// Noble
	{  451, false, SF_PAPERDOL_VGA,  14,  0, 157,  0,  1, 130,  1,  3,  5 },
	// Shopkeeper
	{  455, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Noble
	{  456,  true, SF_PAPERDOL_VGA,  13,  0, 155,  0,  1,   7,  0,  2,  4 },
	// Gypsy
	{  457, false, SF_PAPERDOL_VGA,  14,  1, 128,  2,  3, 130,  1,  3,  5 },
	// Pirate
	{  458, false, SF_PAPERDOL_VGA,  14,  0, 126,  0,  1,   7,  1,  3,  5 },
	// Wench
	{  459,  true, SF_PAPERDOL_VGA,  13,  0, 127,  0,  1,   7,  0,  2,  4 },
	// Ranger
	{  461,  true, SF_PAPERDOL_VGA,  13,  0, 127,  0,  1,   7,  0,  2,  4 },
	// Ranger
	{  463,  true, SF_PAPERDOL_VGA,  13,  1,   4,  2,  3, 130,  0,  2,  4 },
	// Paladin
	{  464, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Lord British
	{  466, false, SF_PAPERDOL_VGA,  14,  0, 126,  0,  1,   7,  1,  3,  5 },
	// Jester	
	{  467, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Entertainer
	{  469,  true, SF_PAPERDOL_VGA,  13,  0, 127,  0,  1,   7,  0,  2,  4 },
	// Jester	
	{  471, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Jester	
	{  472, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Batlin
	{  482, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Highwayman
	{  484, false, SF_PAPERDOL_VGA,  14,  0, 158,  0,  1,   7,  1,  3,  5 },
	// Hook
	{  506, false, SF_PAPERDOL_VGA,  14,  0, 159,  0,  1,   7,  1,  3,  5 },
	// Liche
	{  519, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Skeleton
	{  528, false, SF_PAPERDOL_VGA,  14,  3, 145,  0,  1, 146,  1,  3,  5 },
	// Harpie
	{  532,  true, SF_PAPERDOL_VGA,  13,  1,   4,  2,  3, 130,  0,  2,  4 },
	// Guard
	{  720, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Stone Harpie
	{  753,  true, SF_PAPERDOL_VGA,  13,  1,   4,  2,  3, 130,  0,  2,  4 },
	// Guard
	{  806, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Elizabeth
	{  881,  true, SF_PAPERDOL_VGA,  13,  0, 155,  0,  1,   7,  0,  2,  4 },
	// Abraham
	{  882, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Fellowship Member
	{  884, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Fellowship Member
	{  929,  true, SF_PAPERDOL_VGA,  13,  1,   4,  2,  3, 130,  0,  2,  4 },
	// Guard
	{  946, false, SF_PAPERDOL_VGA,  14,  0, 124,  0,  1,   7,  1,  3,  5 },
	// Ferry man
	{  952, false, SF_PAPERDOL_VGA,  14,  0, 156,  0,  1,   7,  1,  3,  5 },
	// Barkeep
	{  957, false, SF_PAPERDOL_VGA,  14,  0, 126,  0,  1,   7,  1,  3,  5 },

	{ 0 }
};

Paperdoll_gump::Paperdoll_item Paperdoll_gump::Items_BG[] =
{
	// wshape, wf,        Equip spot, Object Type, Gender, file, shape, frame, frame2,  f3,  f4

	// MISC ITEMS

	// Body
	{     400, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     0 },
	{     400, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     1 },
	{     402, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     0 },
	{     402, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     1 },
	{     414, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     0 },
	{     414, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    135,     1 },
	// Bed roll
	{     583, -1,       Actor::back,   OT_Normal,   false, SF_PAPERDOL_VGA,      9,     0 },
	{     583, -1,Actor::back2h_spot,   OT_Normal,   false, SF_PAPERDOL_VGA,      9,     1 },
	// Kidney Belt
	{     584, -1,       Actor::belt,   OT_Normal,   true, SF_PAPERDOL_VGA,     54,     0 },
	// Torch
	{     595, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    113,     0 },
	{     595, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,    113,     1 },
	{     595, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    113,     2 },
	// Lit Torch
	{     701, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     63,     0 },
	{     701, -1,      Actor::rhand,   OT_Single,  false, SF_PAPERDOL_VGA,     63,     1 },
	// Spell book
	{     761, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     15,     0 },
	// Backpack
	{     801, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,      6,     0 },
	// Bag (Belt)
	{     802, -1,       Actor::belt,   OT_Normal,  false, SF_PAPERDOL_VGA,     89,     0 },
	// Bag (Behind back) **NEEDS ART**
	{     802, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Barrel (Behind back) **NEEDS ART**
	{     819, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Sealed Box (Behind back) **NEEDS ART**
	{     798, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Box (Behind back) **NEEDS ART**
	{     799, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Locked Chest (Behind back) **NEEDS ART**
	{     522, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Chest (Behind back) **NEEDS ART**
	{     800, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Basket (Behind back) **NEEDS ART**
	{     803, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Crate (Behind back) **NEEDS ART**
	{     804, -1,       Actor::back,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },


	// AMULETS and COLLARS

	// Leather Collar **NEEDS ART**
	{     582,  0,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,     -1,     0 },
	// Gorget
	{     586,  0,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_GORGET_SHP,     0 },
	// Magic Gorget
	{     843,  0,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_MAGICGORGET_SHP,     0 },
	// Ankh
	{     955,  0,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_AMULETS_SHP,     5 },
	// Fellowship
	{     955,  1,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     4 },
	// White Heart
	{     955,  2,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_AMULETS_SHP,     0 },
	// Amulet of Protection???
	{     955,  3,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     3 },
	// Amulet of ???
	{     955,  4,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     3 },
	// Amulet of ???
	{     955,  5,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     9 },
	// Amulet of ???
	{     955,  6,       Actor::neck,   OT_Normal,  false, SF_PAPERDOL_VGA,      0,     3 },
	// Forge Amulet
	{     955,  7,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_AMULETS_SHP,     1 },
	// Forge Amulet
	{     955,  8,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_AMULETS_SHP,     2 },
	// Forge Amulet
	{     955,  9,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_AMULETS_SHP,     3 },
	// Forge Amulet
	{     955, 10,       Actor::neck,   OT_Normal,  false, SF_GAME_FLX,        EXULT_BG_FLX_AMULETS_SHP,     4 },


	// CLOAKS
	
	// Grey Cloak
	{     285,  0, Actor::cloak_spot,   OT_Normal,  false, SF_GAME_FLX,      EXULT_BG_FLX_GREYCLOAK_SHP,    0,     -1 },
	{     285,  0, Actor::special_spot, OT_Normal,   true, SF_GAME_FLX,      EXULT_BG_FLX_GREYCLOAK_SHP,    1,     -1 },
	// Green Cloak
	{     285,  1, Actor::cloak_spot,   OT_Normal,  false, SF_GAME_FLX,      EXULT_BG_FLX_GREENCLOAK_SHP,    0,     -1 },
	{     285,  1, Actor::special_spot, OT_Normal,   true, SF_GAME_FLX,      EXULT_BG_FLX_GREENCLOAK_SHP,    1,     -1 },
	// Cloak
	{     285,  2, Actor::cloak_spot,   OT_Normal,  false, SF_PAPERDOL_VGA,       8,    0,     -1 },
	{     285,  2, Actor::special_spot, OT_Normal,   true, SF_PAPERDOL_VGA,       8,    1,     -1 },


	// ARMOUR
	
	// Top **NEEDS ART**
	{     249,  0,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     -1,     0,      2,   4,  6 },
	// Top **NEEDS ART**
	{     249,  1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     -1,     0,      2,   4,  6 },
	// Top **NEEDS ART**
	{     249,  2,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     -1,     0,      2,   4,  6 },
	// Leather
	{     569, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     57,     0,      2,   4,  6 },
	// Scale
	{     570, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     92,     0,      2,   4,  6 },
	// Chain
	{     571, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     22,     0,      2,   4,  6 },
	// Plate
	{     573, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,     85,     0,      2,   4,  6 },
	// Magic (shape is different)
	{     666, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,      2,     0,      2,   4,  6 },
	// Antique (shape is different)
	{     836, -1,      Actor::torso,   OT_Shield,   true, SF_PAPERDOL_VGA,      1,     0,      2,   4,  6 },


	// HELMS

	// Magic (shape is different)
	{     383, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     74,     0 },
	// Hood **NEEDS ART**
	{     444, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     59,     0 },
	// Hood **NEEDS ART**
	{     444, 1,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     59,     0 },
	// Chain Coif
	{     539, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     24,     0 },
	// Great Helm
	{     541, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     30,     0 },
	// Crested Helm
	{     542, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     31,     0 },
	// Caddellite Helmet
	{     638, 0,        Actor::head,     OT_Helm,   true, SF_GAME_FLX,        EXULT_BG_FLX_CADDELLITE_HELMET_SHP,     0 },
	// Leather Helm
	{    1004, 0,        Actor::head,     OT_Helm,   true, SF_PAPERDOL_VGA,     59,     0 },

	// LEGGINGS

	// Greaves
	{     353, -1,        Actor::legs,     OT_Normal,   true, SF_GAME_FLX,     EXULT_BG_FLX_GREAVES_SHP,     0 },
	// Leather
	{     574, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Chain
	{     575, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     26,     0 },
	// Plate
	{     576, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     87,     0 },
	// Magic Leggings
	{     686, -1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     72,     0 },
	// Pants **NEEDS ART**
	{     738,  0,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Pants **NEEDS ART**
	{     738,  1,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Pants **NEEDS ART**
	{     738,  2,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Pants **NEEDS ART**
	{     738,  3,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Pants **NEEDS ART**
	{     738,  4,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },
	// Pants **NEEDS ART**
	{     738,  5,        Actor::legs,     OT_Normal,   true, SF_PAPERDOL_VGA,     60,     0 },

	
	// GLOVES

	// Leather
	{     579, -1,     Actor::lfinger,  OT_Normal,  false, SF_PAPERDOL_VGA,     47,     0,      1,   2 },
	// Gauntlets
	{     580, -1,     Actor::lfinger,  OT_Normal,  false, SF_PAPERDOL_VGA,     25,     0,      1,   2 },
	// Magic Gauntlets
	{     835, -1,     Actor::lfinger,  OT_Normal,  false, SF_PAPERDOL_VGA,     70,     0,      1,   2 },


	// RINGS

	// Invisibility Ring
	{     296, 0,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    120,     0,      2,   4 },
	{     296, 0,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    120,     1,      3,   5 },
	// Ring of Protection
	{     297, 0,      Actor::lfinger,   OT_Normal,  false, SF_GAME_FLX,    EXULT_BG_FLX_RING_OF_PROTECTION_SHP,     0,      2,   4  },
	{     297, 0,      Actor::rfinger,   OT_Normal,  false, SF_GAME_FLX,    EXULT_BG_FLX_RING_OF_PROTECTION_SHP,     1,      3,   5 },
	// Ring of Regeneration
	{     298, 0,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    122,     0,      2,   4 },
	{     298, 0,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    122,     1,      3,   5 },
	// Wedding Ring
	{     295, 0,      Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    147,     0,      2,   4 },
	{     295, 0,      Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    147,     1,      3,   5 },
	// Ring
	{     640, -1,     Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    147,     0,      2,   4 },
	{     640, -1,     Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    147,     1,      3,   5 },
	// Ethereal Ring
	{     759, -1,     Actor::lfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    148,     0,      2,   4 },
	{     759, -1,     Actor::rfinger,   OT_Normal,  false, SF_PAPERDOL_VGA,    148,     1,      3,   5 },


	// BOOTS
	
	// Shoes **NEEDS ART**
	{     585, 0,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     -1,     0 },
	// Leather
	{     587, 0,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     58,     0 },
	// Magic
	{     587, 1,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,     73,     0 },
	// Swamp
	{     588, 0,         Actor::feet,   OT_Normal,   true, SF_PAPERDOL_VGA,    106,     0 },


	// SHIELDS

	// Buckler
	{     543, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     20,     0 },
	{     543, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Curved Heater
	{     545, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     34,     0 },
	{     545, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Wooden
	{     572, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,    117,     0 },
	{     572, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Spiked
	{     578, -1,       Actor::rhand,   OT_Shield,  false, SF_GAME_FLX,        EXULT_BG_FLX_SPIKED_SHIELD_SHP,     0 },
	{     578, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Kite
	{     609, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     37,     0 },
	{     609, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },
	// Magic
	{     663, -1,       Actor::rhand,   OT_Shield,  false, SF_PAPERDOL_VGA,     75,     0 },
	{     663, -1, Actor::shield_spot,   OT_Shield,  false, SF_PAPERDOL_VGA,     99,     0 },


	// WEAPONS 1 H
	
	// Sling
	{     474, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    102,     0 },
	{     474, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    102,     1 },
	// Magic Sword
	{     547, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     76,     0 },
	{     547, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     76,     1 },
	// Lightning Whip
	{     549, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     62,     0 },
	{     549, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     62,     1 },
	// Magic Boomerang
	{     550, -1,      Actor::lhand,   OT_Single,  false, SF_GAME_FLX,        EXULT_BG_FLX_MAGIC_BOOMERANG_SHP,     0 },
	{     550, -1,       Actor::belt,   OT_Single,  false, SF_GAME_FLX,        EXULT_BG_FLX_MAGIC_BOOMERANG_SHP,     1 },
	// Fire Sword
	{     551, -1,      Actor::lhand,   OT_Single,  false, SF_GAME_FLX,     EXULT_BG_FLX_FIRESWORD_SHP,     0 },
	{     551, -1,       Actor::belt,   OT_Single,  false, SF_GAME_FLX,     EXULT_BG_FLX_FIRESWORD_SHP,     1 },
	// Magic Axe
	{     552, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     77,     0 },
	{     552, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     77,     1 },
	// Hawk **NEEDS ART**
	{     555, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     -1,     0 },
	{     555, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     -1,     1 },
	// Magebane
	{     559, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     67,     0},
	{     559, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     67,     1},
	// Great Dagger
	{     561, -1,      Actor::lhand,   OT_Single,  false, SF_GAME_FLX,        EXULT_BG_FLX_GREAT_DAGGER_SHP,     0 },
	{     561, -1,       Actor::belt,   OT_Single,  false, SF_GAME_FLX,        EXULT_BG_FLX_GREAT_DAGGER_SHP,     1 },
	// Blowgun
	{     563, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     12,     0 },
	{     563, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     12,     1 },
	// Poisoned Dagger
	{     564, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     36,     0 },
	{     564, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     36,     1 },
	// Star Bursts **NEEDS ART**
	{     565, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     -1,     0 },
	{     565, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     -1,     1 },
	// Sword of Defense
	{     567, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    110,     0 },
	{     567, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    110,     1 },
	// Club
	{     590, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     29,     0 },
	{     590, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     29,     1 },
	// Main Gauche **NEEDS ART**
	{     591, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    107,     0 },
	{     591, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    107,     1 },
	// Spear
	{     592, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,    103,     0 },
	{     592, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,    103,     1 },
	// Throwing Axe
	{     593, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    111,     0 },
	{     593, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    111,     1 },
	// Dagger
	{     594, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     35,     0 },
	{     594, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     35,     1 },
	// Morning Star
	{     596, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     81,     0 },
	{     596, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     81,     1 },
	// Sword
	{     599, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    107,     0 },
	{     599, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    107,     1 },
	// Glass Sword
	{     604, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     48,     0 },
	{     604, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     48,     1 },
	// Boomerang
	{     605, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     16,     0 },
	{     605, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     16,     1 },
	// Decorative Sword
	{     608, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    109,     0 },
	{     608, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    109,     1 },
	// Cleaver
	{     614, -1,      Actor::lhand,   OT_Single,  false, SF_GAME_FLX,        EXULT_BG_FLX_CLEAVER_SHP,     0 },
	{     614, -1,       Actor::belt,   OT_Single,  false, SF_GAME_FLX,        EXULT_BG_FLX_CLEAVER_SHP,     1 },
	// Knives **NEEDS ART**
	{     615, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     -1,     0 },
	{     615, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     -1,     1 },
	// Whip
	{     622, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    115,     0 },
	{     622, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    115,     1 },
	// Hammer
	{     623, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     50,     0 },
	{     623, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     50,     1 },
	// Lightning Wand
	{     629, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    118,     0 },
	{     629, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    118,     1 },
	// Fire Wand
	{     630, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     42,     0 },
	{     630, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     42,     1 },
	// Custom sword **NEEDS ART** ??? (currently Dragon Slayer Sword)
	{     635, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    143,     0 },
	{     635, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    143,     1 },
	// Serpentine Dagger
	{     636, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     95,     0 },
	{     636, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     95,     1 },
	// Serpentine Sword
	{     637, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     97,     0 },
	{     637, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     97,     1 },
	// Mace
	{     659, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     66,     0 },
	{     659, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     66,     1 },
	// Shears 
	{     698, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     98,     0 },
	{     698, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     98,     1 },
	// Rudyom's Wand
	{     771, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    134,     0 },
	{     771, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    134,     1 },
	// Magician's Wand
	{     792, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,     78,     0 },
	{     792, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,     78,     1 },
	// Tongs
	{     994, -1,      Actor::lhand,   OT_Single,  false, SF_PAPERDOL_VGA,    112,     0 },
	{     994, -1,       Actor::belt,   OT_Single,  false, SF_PAPERDOL_VGA,    112,     1 },

	
	// WEAPONS 2H

	// Hoe of Destruction
	{     548, -1,      Actor::lhand,    OT_Staff,  false, SF_GAME_FLX,        EXULT_BG_FLX_HOE_OF_DESTRUCTION_SHP,     0 },
	{     548, -1,Actor::back2h_spot,    OT_Staff,  false, SF_GAME_FLX,        EXULT_BG_FLX_HOE_OF_DESTRUCTION_SHP,     1 },
	// Firedoom Staff
	{     553, -1,      Actor::lhand,    OT_Staff,  false, SF_GAME_FLX,       EXULT_BG_FLX_BGFIREDOOM_SHP,     0 },
	{     553, -1,Actor::back2h_spot,    OT_Staff,  false, SF_GAME_FLX,       EXULT_BG_FLX_BGFIREDOOM_SHP,     1 },
	// Juggernaught Hammer
	{     557, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,     53,     0 },
	{     557, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,     53,     1 },
	// Death Scythe **NEEDS ART**
	{     562, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     93,     0 },
	{     562, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     93,     1 },
	// Pitchfork
	{     589, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     84,     0 },
	{     589, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     84,     1 },
	// Two Handed Hammer
	{     600, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,      51,     0 },
	{     600, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,      51,     1 },
	// Two Handed Axe
	{     601, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,      5,     0 },
	{     601, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,      5,     1 },
	// Two Handed Sword
	{     602, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,    108,     0 },
	{     602, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,    108,     1 },
	// Halberd
	{     603, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     49,     0 },
	{     603, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     49,     1 },
	// Scythe
	{     618, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     93,     0 },
	{     618, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     93,     1 },
	// Rake
	{     620, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     91,     0 },
	{     620, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     91,     1 },
	// Pick
	{     624, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,     83,     0 },
	{     624, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,     83,     1 },
	// Shovel
	{     625, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,    100,     0 },
	{     625, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,    100,     1 },
	// Hoe
	{     626, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     52,     0 },
	{     626, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     52,     1 },
	// Fishing Rod
	{     662, -1,      Actor::lhand,    OT_Staff,  false, SF_PAPERDOL_VGA,     44,     0 },
	{     662, -1,Actor::back2h_spot,    OT_Staff,  false, SF_PAPERDOL_VGA,     44,     1 },
	// The Black Sword
	{     707, -1,      Actor::lhand,   OT_Double,  false, SF_PAPERDOL_VGA,    139,     0 },
	{     707, -1,Actor::back2h_spot,   OT_Double,  false, SF_PAPERDOL_VGA,    139,     1 },
	// Fallowship Staff
	{     885, -1,      Actor::lhand,    OT_Staff,  false, SF_GAME_FLX,       EXULT_BG_FLX_FELLOWSHIPSTAFF_SHP,     0 },
	{     885, -1,Actor::back2h_spot,    OT_Staff,  false, SF_GAME_FLX,       EXULT_BG_FLX_FELLOWSHIPSTAFF_SHP,     1 },


	// CROSSBOWS & BOLTS

	// Magic Bolts
	{     417, 25,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    2,      2, 0 }, // 2
	{     417, 26,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    3,      2, 0 }, // 3
	{     417, 27,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    3,      3, 0 }, 
	{     417, 28,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      3, 0 },
	{     417, 29,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      4, 0 },
	{     417, 30,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      4, 0 },
	{     417, 31,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    4,      4, 0 },
	{     417, -1,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     133,    2,      1, 0 }, // 1
	// Crossbow
	{     598, -1,      Actor::lhand, OT_Crossbow,  false, SF_PAPERDOL_VGA,      32,    0 },
	{     598, -1,       Actor::belt, OT_Crossbow,  false, SF_PAPERDOL_VGA,      32,    1 },
	// Tripple Crossbow **NEEDS ART**
	{     647, -1,      Actor::lhand, OT_Crossbow,  false, SF_PAPERDOL_VGA,      32,    0 },
	{     647, -1,       Actor::belt, OT_Crossbow,  false, SF_PAPERDOL_VGA,      32,    1 },
	// Bolts
	{     723, 25,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    2,      2, 0 },
	{     723, 26,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    3,      2, 0 },
	{     723, 27,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    3,      3, 0 },
	{     723, 28,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      3, 0 },
	{     723, 29,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     723, 30,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     723, 31,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     723, -1,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    2,      1, 0 },
	// Triple Crossbow Bolts
	{     948, 25,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    2,      2, 0 },
	{     948, 26,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    3,      2, 0 },
	{     948, 27,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    3,      3, 0 },
	{     948, 28,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      3, 0 },
	{     948, 29,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     948, 30,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     948, 31,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    4,      4, 0 },
	{     948, -1,       Actor::ammo, OT_Crossbow,  false, SF_PAPERDOL_VGA,     132,    2,      1, 0 },

	
	// BOWS & ARROWS

	// Burst Arrows
	{     554, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    2,      2, 0 },
	{     554, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    3,      2, 0 },
	{     554, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    3,      3, 0 },
	{     554, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      3, 0 },
	{     554, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      4, 0 },
	{     554, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      4, 0 },
	{     554, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    4,      4, 0 },
	{     554, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     21,    2,      1, 0 },
	// Magic Arrows
	{     556, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    2,      2, 0 },
	{     556, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    3,      2, 0 },
	{     556, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    3,      3, 0 },
	{     556, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      3, 0 },
	{     556, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      4, 0 },
	{     556, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      4, 0 },
	{     556, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    4,      4, 0 },
	{     556, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     68,    2,      1, 0 },
	// Lucky Arrows
	{     558, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    2,      2, 0 },
	{     558, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    3,      2, 0 },
	{     558, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    3,      3, 0 },
	{     558, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      3, 0 },
	{     558, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      4, 0 },
	{     558, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      4, 0 },
	{     558, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    4,      4, 0 },
	{     558, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     65,    2,      1, 0 },
	// Love Arrows **NEEDS ART** ??? Is Currently Serpent Arrows
	{     560, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    2,      2, 0 },
	{     560, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    3,      2, 0 },
	{     560, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    3,      3, 0 },
	{     560, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      3, 0 },
	{     560, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      4, 0 },
	{     560, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      4, 0 },
	{     560, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    4,      4, 0 },
	{     560, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,     64,    2,      1, 0 },
	// Sleep (Tseramed) Arrows
	{     568, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    2,      2, 0 },
	{     568, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    3,      2, 0 },
	{     568, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    3,      3, 0 },
	{     568, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      3, 0 },
	{     568, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      4, 0 },
	{     568, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      4, 0 },
	{     568, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    4,      4, 0 },
	{     568, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,    101,    2,      1, 0 },
	// Bow
	{     597, -1,      Actor::lhand,      OT_Bow,  false, SF_PAPERDOL_VGA,     17,     0 },
	{     597, -1,Actor::back2h_spot,      OT_Bow,  false, SF_PAPERDOL_VGA,     17,     1 },
	// Magic Bow
	{     606, -1,      Actor::lhand,      OT_Bow,  false, SF_PAPERDOL_VGA,     69,     0 },
	{     606, -1,Actor::back2h_spot,      OT_Bow,  false, SF_PAPERDOL_VGA,     69,     1 },
	// Arrows
	{     722, 25,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    2,      2, 0 },
	{     722, 26,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    3,      2, 0 },
	{     722, 27,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    3,      3, 0 },
	{     722, 28,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      3, 0 },
	{     722, 29,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      4, 0 },
	{     722, 30,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      4, 0 },
	{     722, 31,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    4,      4, 0 },
	{     722, -1,       Actor::ammo,      OT_Bow,  false, SF_PAPERDOL_VGA,      3,    2,      1, 0 },


	// Musket **NEEDS ART**
	{     278, -1,      Actor::lhand,   OT_Musket,  false, SF_PAPERDOL_VGA,     -1,     0 },
	{     278, -1,Actor::back2h_spot,   OT_Musket,  false, SF_PAPERDOL_VGA,     -1,     1 },
	// Ammunition **NEEDS ART**
	{     581, -1,       Actor::ammo,   OT_Musket,  false, SF_PAPERDOL_VGA,     -1,    2,      1, 0 },

	// Terminator
	{ 0 }
};

Paperdoll_gump::Paperdoll_npc *Paperdoll_gump::GetCharacterInfo(int shape)
{
	int i=0;

	Paperdoll_npc *ch;

	if (Game::get_game_type() == BLACK_GATE)
		ch = Characters_BG;
	else
		ch = Characters;

	while (ch[i].npc_shape)
	{
		if (ch[i].npc_shape == shape)
			return ch+i;
		i++;
	}
	
	return NULL;
}

Paperdoll_gump::Paperdoll_npc *Paperdoll_gump::GetCharacterInfoSafe(int shape)
{
	Paperdoll_npc *ch = GetCharacterInfo(shape);

	if (ch) return ch;
	else if (Game::get_game_type() == BLACK_GATE) return Characters_BG;
	
	return Characters;
}

Paperdoll_gump::Paperdoll_item *Paperdoll_gump::GetItemInfo(int shape, int frame, int spot)
{
	int i=0;

	Paperdoll_item *it;

	if (Game::get_game_type() == BLACK_GATE)
		it = Items_BG;
	else
		it = Items;

	while (it[i].world_shape)
	{
		if (it[i].world_shape == shape
			&& (frame == -1 || it[i].world_frame == -1 || it[i].world_frame == frame)
			&& (spot == -1 || it[i].spot == spot)
			)
			return it+i;
		i++;
	}
	
	return NULL;
}

