/*
 *
 *  Copyright (C) 2006  Alun Bestor/The Exult Team
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
 *	This header file contains the flags used in the mod.
 *
 *	Author: Marzo Junior/Alun Bestor
 *	Last Modified: 2006-03-19
 */

enum new_misc_flags
{
	//The player has heard a Fellowship member rabbit on about
	//their philosophy. Used for new conversation behaviour by
	//askAboutPhilosophy() in npcs.uc.
	HEARD_PHILOSOPHY						= 0x350,
	//Gave Camille the widow Thurston's payment for her wheat delivery
	GAVE_CAMILLE_PAYMENT					= 0x351,
	//Thurston is angry about the avatar using his mill
	THURSTON_WARNED_ABOUT_MILLING			= 0x352
};

//Flags used by the selling-fish-to-Gordon conversation option
//(use static vars instead?)
enum gordon_fish_sales
{
	ASKED_GORDON_ABOUT_FISHING				= 0x360,
	HIRED_BY_GORDON							= 0x361,
	GOT_GORDONS_ROD							= 0x362
};

enum menion_swordmaking
{
	//Menion is waiting to give you a sword you were carrying
	//too much to accept
	MENION_HAS_SWORD						= 0x363,
	//Player got experience reward for forging sword
	GOT_SWORDSMITHING_XP					= 0x364
};

//Flags used by the Rune of Honor quest
enum rune_of_honor_flags
{
	//Talked to Finnigan about Trinsic and the Rune of
	//Honor (starts off quest)
	TOLD_ABOUT_RUNE							= 0x370,
	
	//Talked to Candice about Rune and need LB's permission
	//to return it
	NEED_PERMISSION_FOR_RUNE				= 0x371,
	//Got LB's permission to return Rune
	GOT_PERMISSION_FOR_RUNE					= 0x372,
	
	//Candice wants a replica made
	NEED_REPLICA_RUNE						= 0x373,
	//Found out who can make the replica
	KNOWS_ARTISAN_FOR_RUNE					= 0x374,
	//Received the replica rune (prevents it being given twice)
	GOT_REPLICA_RUNE						= 0x375,
	
	//The replica is on the pedestal in Trinsic
	FAKE_RUNE_IN_TRINSIC					= 0x376,
	//The real rune is on the pedestal in Trinsic
	REAL_RUNE_IN_TRINSIC					= 0x377,
	
	//The replica is on the pedestal in Britain
	FAKE_RUNE_IN_BRITAIN					= 0x378,
	//The real rune has been removed from the pedestal in Britain
	//(needs to be inverted because it will start out false)
	RUNE_TAKEN_FROM_BRITAIN					= 0x379,
	
	//The theft discovery timer has been started (prevents
	//it restarting)
	RUNE_THEFT_TIMER_RUNNING				= 0x37A,
	//Finnigan has got angry letter, demands you return the rune
	RUNE_THEFT_DISCOVERED					= 0x37B,
	
	//Returned the real rune to Britain after stealing it
	//(failed end of quest)
	RETURNED_STOLEN_RUNE					= 0x37C,
	//Returned the real rune to Trinsic after getting
	//replica (successful end of quest)
	RETURNED_RUNE_PROPERLY					= 0x37D,
	//Got the XP reward for returning the Rune
	REWARDED_FOR_RUNE						= 0x37E,
	RUNE_QUEST_COMPLETE						= 0x37F
};

//The following flags are all for the keyring quest:
//Global Flags:
const int ACCEPTED_ZAURIEL_QUEST			= 1535;
const int ZAURIEL_TOLD_LOCATION				= 1536;
const int ZAURIEL_TELEPORTED				= 1537;
const int ISLAND_NO_ONE_THERE				= 1538;
const int GAVE_GEM_SUBQUEST					= 1539;
const int PLAYER_USED_GEM					= 1540;
const int MAGE_KILLED						= 1541;

//Some of the above flags are cleared when the player meets
//Laurianna. This is for economy of global flags, as the
//quest is reaching its end:
enum laurianna_metaplot_flags
{
	LAURIANNA_DRANK_POTION					= 1535,
	RECEIVED_ZAURIEL_REWARD					= 1536,
	LAURIANNA_CURED							= 1537,
	READ_ZAURIEL_JOURNAL					= 1538,
	LAURIANNA_HAS_JOURNAL					= 1539,
	LAURIANNA_IN_YEW						= 1540
};

//some of the above flags are cleared again when Laurianna
//moves to Yew.  This is for economy of global flags.
enum laurianna_yew_flags
{                                               
	LAURIANNA_READY							= 1535,
	LAURIANNA_WILL_JOIN						= 1536
};

//The following flags are used for meditating in shrines:
enum shrine_meditation_flags
{
	// This first value is never actually used as a flag;
	// it has been defined for convenience only
	MEDITATED_AT_SHRINE_BASE				= 1544,
	MEDITATED_AT_SACRIFICE					= 1545,
	MEDITATED_AT_JUSTICE					= 1546,
	MEDITATED_AT_HUMILITY					= 1547,
	MEDITATED_AT_SPIRITUALITY				= 1548,
	MEDITATED_AT_VALOR						= 1549,
	MEDITATED_AT_COMPASSION					= 1550,
	MEDITATED_AT_HONOR						= 1551,
	MEDITATED_AT_HONESTY					= 1552
};

//When the following flags are set, the above flags
//are cleared; thus, we can control the quest with
//only two flags per shrine.
enum codex_quest_flags
{
	// This first value has been defined for convenience only,
	// and is actually used for meditation at the Honesty shrine
	VIEWED_CODEX_BASE						= 1552,
	VIEWED_CODEX_FOR_SACRIFICE				= 1553,
	VIEWED_CODEX_FOR_JUSTICE				= 1554,
	VIEWED_CODEX_FOR_HUMILITY				= 1555,
	VIEWED_CODEX_FOR_SPIRITUALITY			= 1556,
	VIEWED_CODEX_FOR_VALOR					= 1557,
	VIEWED_CODEX_FOR_COMPASSION				= 1558,
	VIEWED_CODEX_FOR_HONOR					= 1559,
	VIEWED_CODEX_FOR_HONESTY				= 1560
};

//Used to prevent cheaters from getting anything
//out of the Codex:
enum codex_anti_cheat_flags
{
	IN_CODEX_QUEST							= 1561,
	SEEN_CODEX_ONCE							= 1562,
	SPIRITUALITY_STONE_QUEST				= 1563,
	ATTUNED_SPIRITUALITY_STONE				= 1564,
	CODEX_ALL_EIGHT_SHRINES					= 1565,
	CODEX_ALL_ITEMS_IN_PLACE				= 1566,
	RELOCATE_CODEX_QUEST					= 1567
};

//New flags for Mack's key when Lock Lake is cleaned:
const int MACKS_KEY_WITH_COVE_MAYOR			= 0x350;
const int KNOWS_MACKS_KEY_WITH_COVE_MAYOR	= 0x351;
