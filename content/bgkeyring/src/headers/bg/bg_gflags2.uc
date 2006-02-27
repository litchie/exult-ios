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
 *	This header file contains several more flags from Black Gate.
 *	It is mostly unsorted... although generally grouped by similarity.
 *
 *	Author: Marzo Junior
 *	Last Modified: 2006-02-27
 */

//FoV flags:
const int BANISHED_EXODUS					= 0x030C;
const int SCROLL_OF_INFINITY				= 0x030E;
const int MET_ERETHIAN						= 0x0310;
const int MET_ARCADION						= 0x0313;
const int BROKE_MIRROR						= 0x032F;
const int COMMANDED_BOND					= 0x0330;
const int REFUSED_HELP_ARCADION				= 0x0331;
const int HELPING_ARCADION					= 0x0332;
const int TALKED_ARCADION_WITH_GEM			= 0x0333;
const int ACCEPTED_GEM_POWER				= 0x0333;
const int ARCADION_SLAVE					= 0x0334;
const int REFUSED_GEM_POWER					= 0x0335;
const int TALKED_ABOUT_BINDING_GEM			= 0x0338;
const int BEGIN_BOND						= 0x0343;
const int FINISHED_BOND						= 0x0344;

//Some Cove flags:
const int MET_JAANA							= 0x0018;
const int ASKED_JAANA_NAME					= 0x00EF;
const int KNOWS_COVE_GOSSIP					= 0x00E4;
const int KNOWS_JAANA_IS_HEALER				= 0x0028;
const int HEARD_ABOUT_NATASSIA				= 0x00E0;
const int HAS_MIRANDAS_BILL					= 0x006A;
const int MET_LORD_HEATHER					= 0x00EA;
const int KNOWS_DEMARIA_KNOWS				= 0x00E3;
const int LOCK_LAKE_BILL_SIGNED				= 0x00DE;

//Some metaplot-related flags:
const int TALKED_TO_GYPSY					= 256;	//0x100;
const int HAD_FIRST_WISP_TALK				= 336;	//0x150;
const int WISP_OFFERED_TRADE				= 307;	//0x133;
const int DELIVERED_NOTEBOOK_TO_WISPS		= 343;	//0x157;
const int LEARNED_ABOUT_ALIGNMENT			= 295;	//0x127

//Flags used in the Improved Orb of the Moons:
const int BROKE_SPHERE						= 4;	//0x0004
const int LEFT_TRINSIC						= 87;	//0x0057

//Some Yew flags, used in reimplementation of Perrin and Reyna
//for the end of the Keyring Quest:
const int MET_PERRIN						= 0x0145;
const int KNOWS_REYNA_LOVES_ANIMALS			= 0x013B;
const int MET_REYNA							= 0x0146;
const int ASKED_REYNA_ABOUT_MOTHER			= 0x0128;
const int KNOWS_REYNA_JOB					= 0x0163;
const int REYNA_EMERGENCY					= 0x013A;
const int GAVE_REYNA_FLOWERS				= 0x0139;

//Set when the Magic Storm spell is cast:
const int MAGIC_STORM_SPELL					= 0x02ED;
//Prevents casting of spells because the Avatar is inside
//one of the generators:
const int INSIDE_GENERATOR					= 0x39;

//Seance flags; I have NO idea why they didn't go with just
//ONE flag instead...
const int SEANCE_CAINE						= 0x01B2;
const int SEANCE_FERRYMAN					= 0x01B3;
const int SEANCE_MARKHAM					= 0x01B4;
const int SEANCE_HORANCE					= 0x01B5;
const int SEANCE_TRENT						= 0x01B6;
const int SEANCE_MORDRA						= 0x01B7;
const int SEANCE_ROWENA						= 0x01B8;
const int SEANCE_PAULETTE					= 0x01B9;
const int SEANCE_QUENTON					= 0x01BA;
const int SEANCE_FORSYTHE					= 0x01BB;

//Mariah's flags
const int MET_MARIAH						= 0x01FB;
