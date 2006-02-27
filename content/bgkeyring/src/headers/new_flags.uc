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
 *	This header file contains the flags used in the Keyring mod.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

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
const int LAURIANNA_DRANK_POTION			= 1535;
const int RECEIVED_ZAURIEL_REWARD			= 1536;
const int LAURIANNA_CURED					= 1537;
const int READ_ZAURIEL_JOURNAL				= 1538;
const int LAURIANNA_HAS_JOURNAL				= 1539;
const int LAURIANNA_IN_YEW					= 1540;

//some of the above flags are cleared again when Laurianna
//moves to Yew.  This is for economy of global flags.
const int LAURIANNA_READY					= 1535;
const int LAURIANNA_WILL_JOIN				= 1536;

//The following flags are used for meditating in shrines:
const int MEDITATED_AT_SACRIFICE			= 1545;
const int MEDITATED_AT_JUSTICE				= 1546;
const int MEDITATED_AT_HUMILITY				= 1547;
const int MEDITATED_AT_SPIRITUALITY			= 1548;
const int MEDITATED_AT_VALOR				= 1549;
const int MEDITATED_AT_COMPASSION			= 1550;
const int MEDITATED_AT_HONOR				= 1551;
const int MEDITATED_AT_HONESTY				= 1552;
//When the following flags are set, the above flags
//are cleared; thus, we can control the quest with
//only two flags per shrine.
const int VIEWED_CODEX_FOR_SACRIFICE		= 1553;
const int VIEWED_CODEX_FOR_JUSTICE			= 1554;
const int VIEWED_CODEX_FOR_HUMILITY			= 1555;
const int VIEWED_CODEX_FOR_SPIRITUALITY		= 1556;
const int VIEWED_CODEX_FOR_VALOR			= 1557;
const int VIEWED_CODEX_FOR_COMPASSION		= 1558;
const int VIEWED_CODEX_FOR_HONOR			= 1559;
const int VIEWED_CODEX_FOR_HONESTY			= 1560;

//Used to prevent cheaters from getting anything
//out of the Codex:
const int IN_CODEX_QUEST					= 1561;
const int SEEN_CODEX_ONCE					= 1562;
const int SPIRITUALITY_STONE_QUEST			= 1563;
const int ATTUNED_SPIRITUALITY_STONE		= 1564;
const int CODEX_ALL_EIGHT_SHRINES			= 1565;
const int CODEX_ALL_ITEMS_IN_PLACE			= 1566;
const int RELOCATE_CODEX_QUEST				= 1567;

//New flags for Mack's key when Lock Lake is cleaned:
const int MACKS_KEY_WITH_COVE_MAYOR			= 0x350;
const int KNOWS_MACKS_KEY_WITH_COVE_MAYOR	= 0x351;
