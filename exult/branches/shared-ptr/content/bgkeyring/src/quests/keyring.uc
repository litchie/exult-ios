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
 *
 *
 *	This header files has includes for all the files of the Keyring Quest.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

//Constants needed by the Keyring Quest:
#include "quests/keyring/constants.uc"

//Cutscenes of the Keyring Quest:
#include "quests/keyring/cutscenes/cheaters_cataclysm.uc"	//The cataclysm triggered by hackmocing Laurianna with the amulet near to her father
#include "quests/keyring/cutscenes/mage_and_goons.uc"		//The cutscene with Laundo and his goons
#include "quests/keyring/cutscenes/zauriel_make_potion.uc"	//Zauriel makes the Blackrock potion
#include "quests/keyring/cutscenes/zauriel_ritual.uc"		//Zauriel performs the curing ritual

//Miscellaneous functions for the Keyring Quest:
#include "quests/keyring/functions.uc"

//The functions for the eggs in the Keyring Quest:
#include "quests/keyring/eggs.uc"				//The keyring quest egg functions

//NPCs added/modified for the Keyring Quest:
#include "npcs/perrin.uc"						//Modified for compatibility with Keyring quest
#include "npcs/reyna.uc"						//Modified for compatibility with Keyring quest
#include "npcs/zauriel.uc"						//Keyring quest NPC
#include "npcs/laurianna.uc"					//Keyring quest NPC

//Items added/modified for the Keyring Quest:
#include "items/keyring.uc"						//The keyring function
#include "items/key.uc"							//The key function
#include "items/zauriel_journal.uc"				//Zauriel's journal
#include "items/blacksword.uc"					//New blacksword function
#include "items/gem_of_dispelling.uc"			//For the keyring quest
#include "items/blackrock_potion.uc"			//For the keyring quest
