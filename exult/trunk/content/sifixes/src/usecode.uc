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

//Tells the compiler the game type
#game "serpentisle"							//Tells the compiler the game type

//Starts autonumbering at function number 0xC00.
//I leave function numbers in the range 0xA00 to
//0xBFF for eggs and weapon functions; this is a
//total of 512 unique functions. That (hopefully)
//is enough...
#autonumber 0xC00

#include "header/constants.uc"				//Misc constants used everywhere
#include "header/si/si_gflags.uc"			//SI Global Flags
#include "header/si/si_externals.uc"		//Calls defined in SI Usecode
#include "header/si/si_shapes.uc"			//SI Shapes
#include "header/si/si_npcs.uc"				//SI NPCs

#include "header/functions.uc"				//New functions

//From here down, all functions have preassigned function numbers:
#include "misc/fawn_tower_cleanup.uc"		//Fixes a few bugs in the cleanup of Fawn Tower
#include "misc/luther_return_shield.uc"		//Fixes a few bugs when returning the shield to Luther
#include "misc/exchanged_item_list.uc"		//Fixes a few bugs in the exchanged item list
#include "misc/resurrect.uc"				//Prevents Companions from being "resurrected" after banes are released,
											//Dupre from being resurrected after sacrifice and Gwenno from talking
											//to the Avatar while insane
#include "misc/location_ids.uc"				//Fixes a few wrongly identified locations

#include "misc/inn_keys.uc"					//Inn keys are now deleted/doors locked
#include "misc/egg_starting_hints.uc"		//Set bear skull flag when Shamino sees the bear
#include "misc/egg_bane_holocaust.uc"		//Modifies the bane holocaust to give inn keys to innkeepers
#include "misc/egg_gorlab_swamp_sleep.uc"	//Prevents player from taking companions to dream world

#include "npcs/dupre.uc"					//Dupre now refuses to leave in Spinebreaker mountains
#include "npcs/shamino.uc"					//Fixing the exchanged item list; also, refuses to leave in Spinebreaker mountains
#include "npcs/iolo.uc"						//Iolo now refuses to leave in Spinebreaker mountains
#include "npcs/baiyanda.uc"					//She now really gives dried fish when asked
#include "npcs/cantra.uc"					//For curing Cantra, from exult/content/si
#include "npcs/frigidazzi.uc"				//Fixes fur cap/misplaced item list bug
#include "npcs/ghost.uc"					//Removes the False Chaos Hierophant bug
#include "npcs/goblin_simon.uc"				//Gives the inn keys to Simon before he dies
#include "npcs/gwenno.uc"					//Fixing the diamond necklace thingy
#include "npcs/thoxa.uc"					//Prevents resurrecting companions after banes are released

#include "items/bucket_cure.uc"				//For curing Cantra, from exult/content/si; modified to allow companions
											//to thank you (and rejoin) after you cure them but before Xenka returns
#include "items/hourglass.uc"				//Fixes Shrine of Order issues
#include "items/pillar.uc"					//Can no longer get to Test of Purity from SS
#include "items/potion.uc"					//Iolo, Shamino and Dupre refuse blue potions in Spinebreaker mountains
#include "items/scroll.uc"					//Fixes bugs with the misplaced item list
//#include "items/time_tellers.uc"			//Changes watches/sundials to 24 hour time

#include "spells/spells.uc"					//Fixes to spells the Avatar can cast

#include "cutscenes/fawn_storm.uc"			//Fixes the Fawn storm so that Iolo's lute is not duplicated
#include "cutscenes/monitor_banquet.uc"		//Prevents deletion of the training pikeman egg
#include "cutscenes/wall_of_lights.uc"		//Absolutely force companions to be there and force-kills them after

