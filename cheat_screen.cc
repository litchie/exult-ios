/*
 *  Copyright (C) 2000-2001  The Exult Team
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

#include "SDL_events.h"
#include "files/U7file.h" 
#include "gamewin.h"
#include "game.h"
#include "cheat_screen.h"
#include "font.h"
#ifndef ALPHA_LINUX_CXX
#  include "cstring"
#endif
#include "actors.h"
#include "cheat.h"
#include "exult.h"
#include "imagewin.h"
#include "vgafile.h"
#include "gameclk.h"
#include "schedule.h"
#include "ucmachine.h"
#include "Configuration.h"
#include "SDL.h"
#include "party.h"

const char *CheatScreen::schedules[33] = {
	"Combat",
	"Hor. Pace",
	"Ver. Pace",
	"Talk",
	"Dance",
	"Eat",
	"Farm",
	"Tend Shop",
	"Miner",
	"Hound",
	"Stand",
	"Loiter",
	"Wander",
	"Blacksmith",
	"Sleep",
	"Wait",
	"Major Sit",
	"Graze",
	"Bake",
	"Sew",
	"Shy",
	"Lab",
	"Thief",
	"Waiter",
	"Special",
	"Kid Games",
	"Eat at Inn",
	"Duel",
	"Preach",
	"Patrol",
	"Desk Work",
	"Follow Avt",
	"Move2Sched"
};

const char *CheatScreen::flag_names[64] = {
	"invisible",		// 0x00
	"asleep",		// 0x01
	"charmed",		// 0x02
	"cursed",		// 0x03
	0,			// 0x04
	0,			// 0x05
	"in_party",		// 0x06
	"paralyzed",		// 0x07

	"poisoned",		// 0x08
	"protection",		// 0x09
	"on_moving_barge",	// 0x0A
	"okay_to_take",		// 0x0B
	"tremor",		// 0x0C
	"cant_die",		// 0x0D
	"dancing",		// 0x0E
	"dont_move",		// 0x0F

	0,			// 0x10
	"si_on_moving_barge",	// 0x11
	"is_temporary",		// 0x12
	0,			// 0x13
	0,			// 0x14
	"okay_to_land",		// 0x15
	0,			// 0x16
	0,			// 0x17

	0,			// 0x18
	"confused",		// 0x19
	"in_motion",		// 0x1A
	0,			// 0x1B
	"met",			// 0x1C
	"si_tournament",	// 0x1D
	"misc",			// 0x1E
	0,			// 0x1F

	0,			// 0x20
	"tattooed",		// 0x21
	"read",			// 0x22
	"petra",		// 0x23
	0,			// 0x24
	0,			// 0x25
	0,			// 0x26
	0,			// 0x27

	0,			// 0x28
	0,			// 0x29
	0,			// 0x2A
	0,			// 0x2B
	0,			// 0x2C
	0,			// 0x2D
	0,			// 0x2E
	0,			// 0x2F

	0,			// 0x30
	0,			// 0x31
	0,			// 0x32
	0,			// 0x33
	0,			// 0x34
	0,			// 0x35
	0,			// 0x36
	0,			// 0x37

	0,			// 0x38
	0,			// 0x39
	0,			// 0x3A
	0,			// 0x3B
	0,			// 0x3C
	0,			// 0x3D
	0,			// 0x3E
	0,			// 0x3F
};
	
CheatScreen::CheatScreen() : grabbed(NULL)
	{
	}

CheatScreen::~CheatScreen()
	{
	}

	
void CheatScreen::show_screen()
{
		
	gwin = Game_window::get_instance();
	ibuf = gwin->get_win()->get_ib8();
	font = fontManager.get_font("MENU_FONT");
	clock = gwin->get_clock();
	maxx = gwin->get_width();
	maxy = gwin->get_height();
	centerx = maxx/2;
	centery = maxy/2;

	// Pause the game
	gwin->get_tqueue()->pause(SDL_GetTicks());

	str_int_pair pal_tuple = game->get_resource("palettes/0");
	pal.load(pal_tuple.str,pal_tuple.num);

	// Start the loop
	NormalLoop();

	// Resume the game clock
	gwin->get_tqueue()->resume(SDL_GetTicks());

	// Reset the palette
	clock->set_palette();
}


//
// DISPLAYS
//

//
// Shared
//

void CheatScreen::SharedPrompt (char *input, const Cheat_Prompt &mode)
{
	char buf[512];

	font->paint_text_fixedwidth(ibuf, "Select->", 0, maxy-18, 8);

	if (input && std::strlen(input))
	{
		font->paint_text_fixedwidth(ibuf, input, 64, maxy-18, 8);
		font->paint_text_fixedwidth(ibuf, "_", 64+std::strlen(input)*8, maxy-18, 8);
	}
	else
		font->paint_text_fixedwidth(ibuf, "_", 64, maxy-18, 8);

	// ...and Prompt Message
	switch (mode)
	{
		default:
		case CP_Command:
		font->paint_text_fixedwidth(ibuf, "Enter Command.", 0, maxy-9, 8);
		break;

		case CP_HitKey:
		font->paint_text_fixedwidth(ibuf, "Hit a key.", 0, maxy-9, 8);
		break;

		case CP_NotAvail:
		font->paint_text_fixedwidth(ibuf, "Not yet available. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_InvalidNPC:
		font->paint_text_fixedwidth(ibuf, "Invalid NPC. Hit a key", 0, maxy-9, 8);
		break;

		case CP_InvalidCom:
		font->paint_text_fixedwidth(ibuf, "Invalid Command. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_Canceled:
		font->paint_text_fixedwidth(ibuf, "Canceled. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_ClockSet:
		font->paint_text_fixedwidth(ibuf, "Clock Set. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_InvalidTime:
		font->paint_text_fixedwidth(ibuf, "Invalid Time. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_InvalidShape:
		font->paint_text_fixedwidth(ibuf, "Invalid Shape. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_InvalidValue:
		font->paint_text_fixedwidth(ibuf, "Invalid Value. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_Created:
		font->paint_text_fixedwidth(ibuf, "Item Created. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_ShapeSet:
		font->paint_text_fixedwidth(ibuf, "Shape Set. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_ValueSet:
		font->paint_text_fixedwidth(ibuf, "Clock Set. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_NameSet:
		font->paint_text_fixedwidth(ibuf, "Name Changed. Hit a key.", 0, maxy-9, 8);
		break;

		case CP_WrongShapeFile:
		font->paint_text_fixedwidth(ibuf, "Wrong shape file. Must be SHAPES.VGA. Hit a key.", 0, maxy-9, 8);
		break;


		case CP_ChooseNPC:
		font->paint_text_fixedwidth(ibuf, "Which NPC? (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_EnterValue:
		font->paint_text_fixedwidth(ibuf, "Enter Value. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_Minute:
		font->paint_text_fixedwidth(ibuf, "Enter Minute. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_Hour:
		font->paint_text_fixedwidth(ibuf, "Enter Hour. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_Day:
		font->paint_text_fixedwidth(ibuf, "Enter Day. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_Shape:
		font->paint_text_fixedwidth(ibuf, "Enter Shape (B=Browse or -1=Cancel)", 0, maxy-9, 8);
		break;

		case CP_Activity:
		font->paint_text_fixedwidth(ibuf, "Enter Activity 0-31. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_XCoord:
		snprintf (buf, 512, "Enter X Coord. Max %i (-1 to cancel.)", c_num_tiles);
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-9, 8);
		break;

		case CP_YCoord:
		snprintf (buf, 512, "Enter Y Coord. Max %i (-1 to cancel.)", c_num_tiles);
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-9, 8);
		break;

		case CP_Lift:
		font->paint_text_fixedwidth(ibuf, "Enter Lift. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_GFlagNum:
		font->paint_text_fixedwidth(ibuf, "Enter Global Flag 0-1023. (-1 to cancel.)", 0, maxy-9, 8);
		break;

		case CP_NFlagNum:
		font->paint_text_fixedwidth(ibuf, "Enter NPC Flag 0-63. (-1 to cancel.)", 0, maxy-9, 8);
		break;


		case CP_Name:
		font->paint_text_fixedwidth(ibuf, "Enter a new Name...", 0, maxy-9, 8);
		break;

	}
}

bool CheatScreen::SharedInput (char *input, int len, int &command, Cheat_Prompt &mode, bool &activate)
{
	SDL_Event event;

  	do {
		SDL_WaitEvent(&event);
	} while (event.type!=SDL_KEYDOWN);

	bool shift = false;
	if (event.key.keysym.mod & KMOD_SHIFT)
		shift = true;
	if ((event.key.keysym.sym == SDLK_s) && (event.key.keysym.mod & KMOD_ALT) && (event.key.keysym.mod & KMOD_CTRL))
	{
		make_screenshot(true);
		return false;
	}


	if (mode >= CP_Name)		// Want Text input (len chars)
	{
		if (event.key.keysym.sym == SDLK_RETURN)
		{
			activate = true;
		}
		else if((event.key.keysym.sym >= '0' && event.key.keysym.sym <= 'z') || event.key.keysym.sym == ' ')
		{
			int curlen = std::strlen(input);
			char chr = event.key.keysym.sym;
			if (event.key.keysym.mod & KMOD_SHIFT) {
#if (defined(BEOS) || defined(OPENBSD) || defined(CYGWIN) || defined(__MORPHOS__))
				if ((chr >= 'a') && (chr <= 'z')) chr -= 32;
#else
				chr = std::toupper(chr);
#endif         
			}
			if(curlen<(len-1))
			{
				input[curlen] = chr;
				input[curlen+1] = 0;
			}
		}
		else if (event.key.keysym.sym == SDLK_BACKSPACE)
		{
			int curlen = std::strlen(input);
			if (curlen) input[curlen-1] = 0;
		}
	}
	else if (mode >= CP_ChooseNPC)	// Need to grab numerical input
	{
		// Browse shape
		if (mode == CP_Shape && !input[0] && event.key.keysym.sym == 'b')
		{
			cheat.shape_browser();
			input[0] = 'b';
			activate = true;
		}

		// Activate (if possible)
		if (event.key.keysym.sym == SDLK_RETURN)
		{
			activate = true;
		}
		else if (event.key.keysym.sym == '-' && !input[0])
		{
			input[0] = '-';
		}
		else if (event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9')
		{
			int curlen = std::strlen(input);
			if(curlen<(len-1))
			{
				input[curlen] = event.key.keysym.sym;
				input[curlen+1] = 0;
			}
		}
		else if (event.key.keysym.sym == SDLK_BACKSPACE)
		{
			int curlen = std::strlen(input);
			if (curlen) input[curlen-1] = 0;
		}
	}
	else if (mode)			// Just want a key pressed
	{
		mode = CP_Command;
		for (int i = 0; i < len; i++) input[i] = 0;
		command = 0;
	}
	else				// Need the key pressed
	{
		command = event.key.keysym.sym;
		return true;
	}
	return false;
}


//
// Normal
//

void CheatScreen::NormalLoop ()
{
	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[5] = { 0, 0, 0, 0, 0 };
	int command;
	bool activate = false;
		
	while (looping)
	{
		gwin->clear_screen();

		// First the display
		NormalDisplay();

		// Now the Menu Column
		NormalMenu();

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			NormalActivate(input, command, mode);
			activate = false;
			continue;			
		}

		if (SharedInput (input, 5, command, mode, activate))
			looping = NormalCheck(input, command, mode, activate);
	}
}

void CheatScreen::NormalDisplay ()
{
	char	buf[512];
	Tile_coord t = gwin->get_main_actor()->get_tile();

	font->paint_text_fixedwidth(ibuf, "Colourless' Advanced Option Cheat Screen", 0, 0, 8);

	if (Game::get_game_type() == BLACK_GATE)
		snprintf (buf, 512, "Running \"Ultima 7: The Black Gate\"");
	else if (Game::get_game_type() == SERPENT_ISLE)
		snprintf (buf, 512, "Running \"Ultima 7: Part 2: Serpent Isle\"");
	else
		snprintf (buf, 512, "Running Unknown Game Type %i", Game::get_game_type());
	
	font->paint_text_fixedwidth(ibuf, buf, 0, 18, 8);

	snprintf (buf, 512, "Exult Version %s", VERSION);
	font->paint_text_fixedwidth(ibuf, buf, 0, 27, 8);



	snprintf (buf, 512, "Current time: %i:%02i %s  Day: %i",
			((clock->get_hour()+11)%12)+1,
			clock->get_minute(),
			clock->get_hour()<12 ? "AM":"PM",
			clock->get_day());
	font->paint_text_fixedwidth(ibuf, buf, 0, 45, 8);

	snprintf (buf, 512, "Coords in hex (%04x, %04x, %02x)", 
							t.tx, t.ty, t.tz);
	font->paint_text_fixedwidth(ibuf, buf, 0, 63, 8);

	snprintf (buf, 512, "Coords in dec (%04i, %04i, %02i)",
							t.tx, t.ty, t.tz);
	font->paint_text_fixedwidth(ibuf, buf, 0, 72, 8);

}

void CheatScreen::NormalMenu ()
{
	char	buf[512];

	// Left Column

	// Use
	Shape_manager *sman = Shape_manager::get_instance();
	if (Game::get_game_type() == SERPENT_ISLE || (sman->can_use_paperdolls() && sman->get_bg_paperdolls()))
		snprintf (buf, 512, "[P]aperdolls..: Yes");
	else
		snprintf (buf, 512, "[P]aperdolls..:  No");		
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-99, 8);

	// GodMode
	snprintf (buf, 512, "[G]od Mode....: %3s", cheat.in_god_mode()?"On":"Off");
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-90, 8);

	// Archwizzard Mode
	snprintf (buf, 512, "[W]izard Mode.: %3s", cheat.in_wizard_mode()?"On":"Off");
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-81, 8);

	// Infravision
	snprintf (buf, 512, "[I]nfravision.: %3s", cheat.in_infravision()?"On":"Off");
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-72, 8);

	// Hackmover
	snprintf (buf, 512, "[H]ack Mover..: %3s", cheat.in_hack_mover()?"Yes":"No");
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-63, 8);

	// Eggs
	snprintf (buf, 512, "[E]ggs Visible: %3s", gwin->paint_eggs?"Yes":"No");
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-54, 8);

	// Set Time
	font->paint_text_fixedwidth(ibuf, "[S]et Time", 0, maxy-45, 8);

	// Time Rate
	snprintf (buf, 512, "[+-] Time Rate: %3i", clock->get_time_rate());
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-36, 8);


	// Right Column

	// NPC Tool
	font->paint_text_fixedwidth(ibuf, "[N]PC Tool", 160, maxy-99, 8);

	// Global Flag Modify
	font->paint_text_fixedwidth(ibuf, "[F]lag Modifier", 160, maxy-90, 8);

#if 0
	// Teleport
	font->paint_text_fixedwidth(ibuf, "[T]eleport", 160, maxy-81, 8);
#endif

#if 0
	// Create Item
	font->paint_text_fixedwidth(ibuf, "[C]reate Item", 160, maxy-72, 8);
#endif

	// eXit
	font->paint_text_fixedwidth(ibuf, "[X]it", 160, maxy-36, 8);

}

void CheatScreen::NormalActivate (char *input, int &command, Cheat_Prompt &mode)
{
	int npc = std::atoi(input);
	Shape_manager *sman = Shape_manager::get_instance();

	mode = CP_Command;

	switch (command)
	{
		// God Mode
		case 'g':
		cheat.toggle_god();
		break;

		// Wizard Mode
		case 'w':
		cheat.toggle_wizard();
		break;

		// Infravision
		case 'i':
		cheat.toggle_infravision();
		break;

		// Eggs
		case 'e':
		cheat.toggle_eggs();
		break;

		// Hack mover
		case 'h':
		cheat.toggle_hack_mover();
		break;

		// Set Time
		case 's':
		mode = TimeSetLoop();
		break;

		// - Time Rate
		case '-':
		if (clock->get_time_rate() > 0) 
			clock->set_time_rate (clock->get_time_rate()-1);
		break;

		// + Time Rate
		case '+':
		if (clock->get_time_rate() < 20) 
			clock->set_time_rate (clock->get_time_rate()+1);
		break;

#if 0
		// Teleport
		case 't':
		mode = CP_NotAvail;
		break;
#endif

		// NPC Tool
		case 'n':
		if (npc < -1) mode = CP_InvalidNPC;
		else if (npc == -1) mode = CP_Canceled;
		else if (!input[0]) NPCLoop(-1);
		else mode = NPCLoop(npc);
		break;

#if 0
		// Create
		case 'c':
		mode = CP_NotAvail;
		break;
#endif

		// Global Flag Editor
		case 'f':
		if (npc < -1) mode = CP_InvalidValue;
		else if (npc > 1023) mode = CP_InvalidValue;
		else if (npc == -1 || !input[0]) mode = CP_Canceled;
		else mode = GlobalFlagLoop(npc);
		break;

		// Paperdolls
		case 'p':
			if (Game::get_game_type() == BLACK_GATE 
				&& sman->can_use_paperdolls()) {
				sman->set_bg_paperdolls (sman->get_bg_paperdolls()?false:true);
				config->set("config/gameplay/bg_paperdolls",
							sman->get_bg_paperdolls() ? "yes" : "no", true);
			}
		break;

		default:
		break;
	}

	input[0] = 0;
	input[1] = 0;
	input[2] = 0;
	input[3] = 0;
	command = 0;
}

// Checks the input
bool CheatScreen::NormalCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate)
{
	switch(command)
	{
		// Simple commands
		case 't':	// Teleport
		case 'g':	// God Mode
		case 'w':	// Wizard
		case 'i':	// iNfravision
		case 's':	// Set Time
		case 'e':	// Eggs
		case 'h':	// Hack Mover
		case 'c':	// Create Item
		case 'p':	// Paperdolls
		input[0] = command;
		activate = true;
		break;

		// - Time
		case SDLK_KP_MINUS:
		case '-':
		command = '-';
		input[0] = command;
		activate = true;
		break;

		// + Time
		case SDLK_KP_PLUS:
		case '=':
		command = '+';
		input[0] = command;
		activate = true;
		break;

		// NPC Tool
		case 'n':
		mode = CP_ChooseNPC;
		break;

		// Global Flag Editor
		case 'f':
		mode = CP_GFlagNum;
		break;

		// X and Escape leave
		case SDLK_ESCAPE:
		case 'x':
		input[0] = command;
		return false;

		default:
		mode = CP_InvalidCom;
		input[0] = command;
		command = 0;
		break;
	}

	return true;
}

//
// Activity Display
//

void CheatScreen::ActivityDisplay ()
{
	char	buf[512];
	int i;

	for (i = 0; i < 11; i++)
	{
		snprintf (buf, 512, "%2i %s", i, schedules[i]);
		font->paint_text_fixedwidth(ibuf, buf, 0, i*9, 8);

		snprintf (buf, 512, "%2i %s", i+11, schedules[i+11]);
		font->paint_text_fixedwidth(ibuf, buf, 112, i*9, 8);

		if (i != 10)
		{
			snprintf (buf, 512, "%2i %s", i+22, schedules[i+22]);
			font->paint_text_fixedwidth(ibuf, buf, 224, i*9, 8);
		}
	}

}


//
// TimeSet
//

CheatScreen::Cheat_Prompt CheatScreen::TimeSetLoop ()
{
	// This is for the prompt message
	Cheat_Prompt mode = CP_Day;

	// This is the command
	char input[5] = { 0, 0, 0, 0, 0 };
	int command;
	bool activate = false;
	
	int day, hour;

	while (1)
	{
		gwin->clear_screen();

		// First the display
		NormalDisplay();

		// Now the Menu Column
		NormalMenu();

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			int val = std::atoi(input);
			
			if (val == -1)
			{
				return CP_Canceled;
			}
			else if (val < -1)
			{
				return CP_InvalidTime;
			}
			else if (mode == CP_Day)
			{
				day = val;
				mode = CP_Hour;
			}
			else if (val > 59)
			{
				return CP_InvalidTime;
			}
			else if (mode == CP_Minute)
			{
				clock->set_day(day);
				clock->set_hour(hour);
				clock->set_minute(val);
				break;
			}
			else if (val > 23)
			{
				return CP_InvalidTime;
			}
			else if (mode == CP_Hour)
			{
				hour = val;
				mode = CP_Minute;
			}

			activate = false;
			input[0] = 0;
			input[1] = 0;
			input[2] = 0;
			input[3] = 0;
			command = 0;
			continue;			
		}

		SharedInput(input, 5, command, mode, activate);
	}

	return CP_ClockSet;
}


//
// Global Flags
//

CheatScreen::Cheat_Prompt CheatScreen::GlobalFlagLoop (int num)
{
	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[17];
	int i;
	int command;
	bool activate = false;
	char	buf[512];
		
	for (i = 0; i < 17; i++) input[i] = 0;

	Usecode_machine *usecode = Game_window::get_instance()->get_usecode();

	while (looping)
	{
		gwin->clear_screen();


		NormalDisplay();


		// First the info
		snprintf (buf, 512, "Global Flag %i", num);
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-99, 8);

		snprintf (buf, 512, "Flag is %s", usecode->get_global_flag(num)?"SET":"UNSET");
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-90, 8);


		// Now the Menu Column
		if (!usecode->get_global_flag(num)) font->paint_text_fixedwidth(ibuf, "[S]et Flag", 160, maxy-99, 8);
		else font->paint_text_fixedwidth(ibuf, "[U]nset Flag", 160, maxy-99, 8);

		// Change Flag
		font->paint_text_fixedwidth(ibuf, "[*] Change Flag", 0, maxy-72, 8);
		if (num > 0 && num < 1023) font->paint_text_fixedwidth(ibuf, "[+-] Scroll Flags", 0, maxy-63, 8);
		else if (num == 0) font->paint_text_fixedwidth(ibuf, "[+] Scroll Flags", 0, maxy-63, 8);
		else font->paint_text_fixedwidth(ibuf, "[-] Scroll Flags", 0, maxy-63, 8);

		font->paint_text_fixedwidth(ibuf, "[X]it", 0, maxy-36, 8);

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			if (command == '-')		// Decrement
			{
				num--;
				if (num < 0) num = 0;
			}
			else if (command == '+')	// Increment
			{
				num++;
				if (num > 1023) num = 1023;
			}
			else if (command == '*')	// Change Flag
			{
				i = std::atoi(input);
				if (i < -1 || i > 1023) mode = CP_InvalidValue;
				else if (i == -1) mode = CP_Canceled;
				else if (input[0]) num = i;
			}
			else if (command == 's')	// Set
			{
				usecode->set_global_flag(num, 1);
			}
			else if (command == 'u')	// Unset
			{
				usecode->set_global_flag(num, 0);
			}

			for (i = 0; i < 17; i++) input[i] = 0;
			mode = CP_Command;
			command = 0;
			activate = false;
			continue;			
		}

		if (SharedInput (input, 17, command, mode, activate))
		{
			switch(command)
			{
				// Simple commands
				case 's':	// Set Flag
				case 'u':	// Unset flag
				input[0] = command;
				activate = true;
				break;

				// Decrement
				case SDLK_KP_MINUS:
				case '-':
				command = '-';
				input[0] = command;
				activate = true;
				break;

				// Increment
				case SDLK_KP_PLUS:
				case '=':
				command = '+';
				input[0] = command;
				activate = true;
				break;

				// * Change Flag
				case SDLK_KP_MULTIPLY:
				case '8':
				command = '*';
				input[0] = 0;
				mode = CP_GFlagNum;
				break;

				// X and Escape leave
				case SDLK_ESCAPE:
				case 'x':
				input[0] = command;
				looping = false;
				break;

				default:
				mode = CP_InvalidCom;
				input[0] = command;
				command = 0;
				break;
			}
		}
	}
	return CP_Command;
}

//
// NPCs
//

CheatScreen::Cheat_Prompt CheatScreen::NPCLoop (int num)
{
	Actor *actor;

	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[17];
	int i;
	int command;
	bool activate = false;
		
	for (i = 0; i < 17; i++) input[i] = 0;

	while (looping)
	{
		if (num == -1) actor = grabbed;
		else actor = gwin->get_npc (num);
		grabbed = actor;
		if (actor) num = actor->get_npc_num();

		gwin->clear_screen();

		// First the display
		NPCDisplay(actor, num);

		// Now the Menu Column
		NPCMenu(actor, num);

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			NPCActivate(input, command, mode, actor, num);
			activate = false;
			continue;			
		}

		if (SharedInput (input, 17, command, mode, activate))
			looping = NPCCheck(input, command, mode, activate, actor, num);
	}
	return CP_Command;
}

void CheatScreen::NPCDisplay (Actor *actor, int &num)
{
	char	buf[512];
	if (actor)
	{
		Tile_coord t = actor->get_tile();
	
		// Paint the actors shape
		Shape_frame *shape = actor->get_shape();
		if (shape) actor->paint_shape (shape->get_xright()+240, shape->get_yabove());

		// Now the info
		std::string namestr = actor->get_npc_name();
		snprintf (buf, 512, "NPC %i - %s", num, namestr.c_str());
		font->paint_text_fixedwidth(ibuf, buf, 0, 0, 8);

		snprintf (buf, 512, "Loc (%04i, %04i, %02i)", 
							t.tx, t.ty, t.tz);
		font->paint_text_fixedwidth(ibuf, buf, 0, 9, 8);

		snprintf (buf, 512, "Shape %04i:%02i  %s", actor->get_shapenum(), actor->get_framenum(), actor->get_flag(Obj_flags::met)?"Met":"Not Met");
		font->paint_text_fixedwidth(ibuf, buf, 0, 18, 8);

		snprintf (buf, 512, "Current Activity: %2i - %s", actor->get_schedule_type(), schedules[actor->get_schedule_type()]);
		font->paint_text_fixedwidth(ibuf, buf, 0, 36, 8);

		snprintf (buf, 512, "Experience: %i", actor->get_property(Actor::exp));
		font->paint_text_fixedwidth(ibuf, buf, 0, 45, 8);
		snprintf (buf, 512, "Level: %i", actor->get_level());
		font->paint_text_fixedwidth(ibuf, buf, 144, 45, 8);

		snprintf (buf, 512, "Training: %2i  Health: %2i", actor->get_property(Actor::training), actor->get_property(Actor::health));
		font->paint_text_fixedwidth(ibuf, buf, 0, 54, 8);

		if (num != -1)
		{
			int ucitemnum = 0x10000-num;
			if (!num) ucitemnum = 0xfe9c;
			snprintf (buf, 512, "Usecode item %4x function %x", ucitemnum, actor->get_usecode());
			font->paint_text_fixedwidth(ibuf, buf, 0, 63, 8);
		}
		else
		{
			snprintf (buf, 512, "Usecode function %x", actor->get_usecode());
			font->paint_text_fixedwidth(ibuf, buf, 0, 63, 8);
		}

		if (actor->get_polymorph() != -1)
		{
			snprintf (buf, 512, "Polymorphed from %04i", actor->get_polymorph());
			font->paint_text_fixedwidth(ibuf, buf, 0, 72, 8);
		}
	}
	else
	{
		snprintf (buf, 512, "NPC %i - Invalid NPC!", num);
		font->paint_text_fixedwidth(ibuf, buf, 0, 0, 8);
	}
}

void CheatScreen::NPCMenu (Actor *actor, int &num)
{
	// Left Column

#if 0
	// Attack Mode
	if (actor) font->paint_text_fixedwidth(ibuf, "[A]ttack Mode", 0, maxy-99, 8);
#endif

	// Business Activity
	if (actor) font->paint_text_fixedwidth(ibuf, "[B]usiness Activity", 0, maxy-99, 8);

	// Change Shape
	if (actor) font->paint_text_fixedwidth(ibuf, "[C]hange Shape", 0, maxy-90, 8);

	// XP
	if (actor) font->paint_text_fixedwidth(ibuf, "[E]xperience", 0, maxy-81, 8);

	// NPC Flags
	if (actor) font->paint_text_fixedwidth(ibuf, "[N]pc Flags", 0, maxy-72, 8);

	// Name
	if (actor) font->paint_text_fixedwidth(ibuf, "[\\]ame", 0, maxy-63, 8);

#if 0
	// Pop Weapon
	if (actor) font->paint_text_fixedwidth(ibuf, "[P]op Weapon", 0, maxy-45, 8);
#endif

	// eXit
	font->paint_text_fixedwidth(ibuf, "[X]it", 0, maxy-36, 8);


	// Right Column

	// Stats
	if (actor) font->paint_text_fixedwidth(ibuf, "[S]tats", 160, maxy-99, 8);

#if 0
	// Target
	if (actor) font->paint_text_fixedwidth(ibuf, "[T]arget", 160, maxy-90, 8);
#endif

	// Training Points
	if (actor) font->paint_text_fixedwidth(ibuf, "[~]raining Points", 160, maxy-90, 8);

	// Teleport
	if (actor) font->paint_text_fixedwidth(ibuf, "[']eleport", 160, maxy-81, 8);

	// Change NPC
	font->paint_text_fixedwidth(ibuf, "[*] Change NPC", 160, maxy-45, 8);

	// Change NPC
	font->paint_text_fixedwidth(ibuf, "[+-] Scroll NPCs", 160, maxy-36, 8);

}

void CheatScreen::NPCActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor, int &num)
{
	int i = std::atoi(input);
	int nshapes = 
		Shape_manager::get_instance()->get_shapes().get_num_shapes();

	mode = CP_Command;

	if (command == '-')
	{
		num--;
		if (num < 0) num = 0;
	}
	else if (command == '+')
	{
		num++;
	}
	else if (command == '*')	// Change NPC
	{
		if (i < -1) mode = CP_InvalidNPC;
		else if (i == -1) mode = CP_Canceled;
		else if (input[0]) num = i;
	}
	else if (actor) switch (command)
	{
#if 0
		case 'a':	// Attack mode
		mode = CP_NotAvail;
		break;
#endif

		case 'b':	// Business
		BusinessLoop(actor);
		break;

		case 'n':	// Npc flags
		FlagLoop(actor);
		break;

#if 0
		case 'p':	// pop weapon
		mode = CP_NotAvail;
		break;
#endif

		case 's':	// stats
		StatLoop(actor);
		break;

#if 0
		case 't':	// Target
		mode = CP_NotAvail;
		break;
#endif

		case '\'':	// Teleport
		Game_window::get_instance()->teleport_party(actor->get_tile());
		break;


		case 'e':	// Experience
		if (i < 0) mode = CP_InvalidNPC;
		else actor->set_property (Actor::exp, i);
		break;

		case '`':	// Training Points
		if (i < 0) mode = CP_InvalidNPC;
		else actor->set_property (Actor::training, i);
		break;

		case 'c':	// Change shape
		if (input[0] == 'b')	// Browser
		{
			int n;
			if (!cheat.get_browser_shape(i, n))
			{
				mode = CP_WrongShapeFile;
				break;
			}
		}

		if (i == -1) mode = CP_Canceled;
		else if (i < 0) mode = CP_InvalidShape;
		else if (i >= nshapes) mode = CP_InvalidShape;
		else if (input[0] && (input[0] != '-' || input[1]))
		{
			actor->set_shape(i);
			mode = CP_ShapeSet;
		}
		break;

		case '\\':	// Name
		if (!std::strlen(input)) mode = CP_Canceled;
		else
		{
			actor->set_npc_name(input);
			mode = CP_NameSet;
		}
		break;

		default:
		break;
	}
	for (i = 0; i < 17; i++) input[i] = 0;
	command = 0;
}

// Checks the input
bool CheatScreen::NPCCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor, int &num)
{
	switch(command)
	{
		// Simple commands
		case 'a':	// Attack mode
		case 'b':	// BUsiness
		case 'n':	// Npc flags
		case 'p':	// pop weapon
		case 's':	// stats
		case 't':	// Target
		case '\'':	// Teleport
		input[0] = command;
		if (!actor) mode = CP_InvalidCom;
		else activate = true;
		break;

		// Value entries
		case 'e':	// Experience
		case '`':	// Training Points
		if (!actor) mode = CP_InvalidCom;
		else mode = CP_EnterValue;
		break;

		// Change shape
		case 'c':
		if (!actor) mode = CP_InvalidCom;
		else mode = CP_Shape;
		break;

		// Name
		case '\\':
		if (!actor) mode = CP_InvalidCom;
		else mode = CP_Name;
		break;

		// - NPC
		case SDLK_KP_MINUS:
		case '-':
		command = '-';
		input[0] = command;
		activate = true;
		break;

		// + NPC
		case SDLK_KP_PLUS:
		case '=':
		command = '+';
		input[0] = command;
		activate = true;
		break;

		// * Change NPC
		case SDLK_KP_MULTIPLY:
		case '8':
		command = '*';
		input[0] = 0;
		mode = CP_ChooseNPC;
		break;

		// X and Escape leave
		case SDLK_ESCAPE:
		case 'x':
		input[0] = command;
		return false;

		default:
		mode = CP_InvalidCom;
		input[0] = command;
		command = 0;
		break;
	}

	return true;
}

//
// NPC Flags
//

void CheatScreen::FlagLoop (Actor *actor)
{
	int num = actor->get_npc_num();
	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[17];
	int i;
	int command;
	bool activate = false;
		
	for (i = 0; i < 17; i++) input[i] = 0;

	while (looping)
	{
		gwin->clear_screen();

		// First the display
		NPCDisplay(actor, num);

		// Now the Menu Column
		FlagMenu(actor);

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			FlagActivate(input, command, mode, actor);
			activate = false;
			continue;			
		}

		if (SharedInput (input, 17, command, mode, activate))
			looping = FlagCheck(input, command, mode, activate, actor);
	}
}

void CheatScreen::FlagMenu (Actor *actor)
{
	char	buf[512];

	// Left Column

	// Asleep
	snprintf (buf, 512, "[A] Asleep.%c", actor->get_flag(Obj_flags::asleep)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-108, 8);

	// Charmed
	snprintf (buf, 512, "[B] Charmd.%c", actor->get_flag(Obj_flags::charmed)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-99, 8);

	// Cursed
	snprintf (buf, 512, "[C] Cursed.%c", actor->get_flag(Obj_flags::cursed)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-90, 8);

	// Paralyzed
	snprintf (buf, 512, "[D] Prlyzd.%c", actor->get_flag(Obj_flags::paralyzed)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-81, 8);

	// Poisoned
	snprintf (buf, 512, "[E] Poisnd.%c", actor->get_flag(Obj_flags::poisoned)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-72, 8);

	// Protected
	snprintf (buf, 512, "[F] Prtctd.%c", actor->get_flag(Obj_flags::protection)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-63, 8);

	// Advanced Editor
	font->paint_text_fixedwidth(ibuf, "[*] Advanced", 0, maxy-45, 8);

	// Exit
	font->paint_text_fixedwidth(ibuf, "[X]it", 0, maxy-36, 8);


	// Center Column

	// Party
	snprintf (buf, 512, "[I] Party..%c", actor->get_flag(Obj_flags::in_party)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-108, 8);

	// Invisible
	snprintf (buf, 512, "[J] Invsbl.%c", actor->get_flag(Obj_flags::invisible)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-99, 8);

	// Fly
	snprintf (buf, 512, "[K] Fly....%c", actor->get_type_flag(Actor::tf_fly)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-90, 8);

	// Walk
	snprintf (buf, 512, "[L] Walk...%c", actor->get_type_flag(Actor::tf_walk)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-81, 8);

	// Swim
	snprintf (buf, 512, "[M] Swim...%c", actor->get_type_flag(Actor::tf_swim)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-72, 8);

	// Ethereal
	snprintf (buf, 512, "[N] Ethrel.%c", actor->get_type_flag(Actor::tf_ethereal)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-63, 8);

	// Protectee
	snprintf (buf, 512, "[O] Prtcee.%c", '?');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-54, 8);

	// Conjured
	snprintf (buf, 512, "[P] Conjrd.%c", actor->get_type_flag(Actor::tf_conjured)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-45, 8);

	// Tournament (SI ONLY)
	snprintf (buf, 512, "[3] Tourna.%c", actor->get_flag(
		Obj_flags::si_tournament)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 104, maxy-36, 8);

	// Naked (AV SI ONLY)
	if (Game::get_game_type() == SERPENT_ISLE && !actor->get_npc_num())
	{
		snprintf (buf, 512, "[7] Naked..%c", actor->get_siflag(Actor::naked)?'Y':'N');
		font->paint_text_fixedwidth(ibuf, buf, 104, maxy-27, 8);
	}


	// Right Column

	// Summoned
	snprintf (buf, 512, "[Q] Summnd.%c", actor->get_type_flag(Actor::tf_summonned)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 208, maxy-108, 8);

	// Bleeding
	snprintf (buf, 512, "[R] Bleedn.%c", actor->get_type_flag(Actor::tf_bleeding)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 208, maxy-99, 8);

	if (!actor->get_npc_num())	// Avatar
	{
		// Sex
		snprintf (buf, 512, "[S] Sex....%c", actor->get_type_flag(Actor::tf_sex)?'F':'M');
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-90, 8);

		// Skin
		snprintf (buf, 512, "[1] Skin...%s", "??");
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-81, 8);

		// Read
		snprintf (buf, 512, "[4] Read...%c", actor->get_flag(Obj_flags::read)?'Y':'N');
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-72, 8);
	}
	else	// Not Avatar
	{
		// Met
		snprintf (buf, 512, "[T] Met....%c", actor->get_flag(Obj_flags::met)?'Y':'N');
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-90, 8);

		// NoCast
		snprintf (buf, 512, "[U] NoCast.%c", actor->get_flag(
					Obj_flags::no_spell_casting)?'Y':'N');
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-81, 8);

		// ID
		snprintf (buf, 512, "[V] ID#:%02i", actor->get_ident());
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-72, 8);
	}

	// Freeze
	snprintf (buf, 512, "[W] Freeze.%c", actor->get_flag(
						Obj_flags::freeze)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 208, maxy-63, 8);

	// Party
	if (actor->is_in_party())
	{
		// Temp
		snprintf (buf, 512, "[Y] Temp: %02i", 
						actor->get_temperature());
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-54, 8);

		// Conjured
		snprintf (buf, 512, "Warmth: %04i", 
						actor->figure_warmth());
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-45, 8);
	}

	// Polymorph
	snprintf (buf, 512, "[2] Polymo.%c", actor->get_flag(Obj_flags::polymorph)?'Y':'N');
	font->paint_text_fixedwidth(ibuf, buf, 208, maxy-36, 8);

	// Patra (AV SI ONLY)
	if (Game::get_game_type() == SERPENT_ISLE && !actor->get_npc_num())
	{
		snprintf (buf, 512, "[5] Petra..%c", actor->get_flag(Obj_flags::petra)?'Y':'N');
		font->paint_text_fixedwidth(ibuf, buf, 208, maxy-27, 8);
	}

}

void CheatScreen::FlagActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor)
{
	int i = std::atoi(input);
	int nshapes = 
		Shape_manager::get_instance()->get_shapes().get_num_shapes();

	mode = CP_Command;
	switch (command)
	{
		// Everyone

		// Toggles
		case 'a':	// Asleep
		if (actor->get_flag(Obj_flags::asleep))
			actor->clear_flag(Obj_flags::asleep);
		else
			actor->set_flag(Obj_flags::asleep);
		break;
		
		case 'b':	// Charmed
		if (actor->get_flag(Obj_flags::charmed))
			actor->clear_flag(Obj_flags::charmed);
		else
			actor->set_flag(Obj_flags::charmed);
		break;
		
		case 'c':	// Cursed
		if (actor->get_flag(Obj_flags::cursed))
			actor->clear_flag(Obj_flags::cursed);
		else
			actor->set_flag(Obj_flags::cursed);
		break;
		
		case 'd':	// Paralyzed
		if (actor->get_flag(Obj_flags::paralyzed))
			actor->clear_flag(Obj_flags::paralyzed);
		else
			actor->set_flag(Obj_flags::paralyzed);
		break;
		
		case 'e':	// Poisoned
		if (actor->get_flag(Obj_flags::poisoned))
			actor->clear_flag(Obj_flags::poisoned);
		else
			actor->set_flag(Obj_flags::poisoned);
		break;
		
		case 'f':	// Protected
		if (actor->get_flag(Obj_flags::protection))
			actor->clear_flag(Obj_flags::protection);
		else
			actor->set_flag(Obj_flags::protection);
		break;
		
		case 'j':	// Invisible
		if (actor->get_flag(Obj_flags::invisible))
			actor->clear_flag(Obj_flags::invisible);
		else
			actor->set_flag(Obj_flags::invisible);
		break;
		
		case 'k':	// Fly
		if (actor->get_type_flag(Actor::tf_fly))
			actor->clear_type_flag(Actor::tf_fly);
		else
			actor->set_type_flag(Actor::tf_fly);
		break;
		
		case 'l':	// Walk
		if (actor->get_type_flag(Actor::tf_walk))
			actor->clear_type_flag(Actor::tf_walk);
		else
			actor->set_type_flag(Actor::tf_walk);
		break;
		
		case 'm':	// Swim
		if (actor->get_type_flag(Actor::tf_swim))
			actor->clear_type_flag(Actor::tf_swim);
		else
			actor->set_type_flag(Actor::tf_swim);
		break;
		
		case 'n':	// Ethrel
		if (actor->get_type_flag(Actor::tf_ethereal))
			actor->clear_type_flag(Actor::tf_ethereal);
		else
			actor->set_type_flag(Actor::tf_ethereal);
		break;
		
		case 'p':	// Conjured
		if (actor->get_type_flag(Actor::tf_conjured))
			actor->clear_type_flag(Actor::tf_conjured);
		else
			actor->set_type_flag(Actor::tf_conjured);
		break;
		
		case 'q':	// Summoned
		if (actor->get_type_flag(Actor::tf_summonned))
			actor->clear_type_flag(Actor::tf_summonned);
		else
			actor->set_type_flag(Actor::tf_summonned);
		break;
		
		case 'r':	// Bleeding
		if (actor->get_type_flag(Actor::tf_bleeding))
			actor->clear_type_flag(Actor::tf_bleeding);
		else
			actor->set_type_flag(Actor::tf_bleeding);
		break;
		
		case '3':	// Tournament
		if (actor->get_flag(Obj_flags::si_tournament))
			actor->clear_flag(Obj_flags::si_tournament);
		else
			actor->set_flag(Obj_flags::si_tournament);
		break;
		
		case 's':	// Sex
		if (actor->get_type_flag(Actor::tf_sex))
			actor->clear_type_flag(Actor::tf_sex);
		else
			actor->set_type_flag(Actor::tf_sex);
		break;
		
		case '4':	// Read
		if (actor->get_flag(Obj_flags::read))
			actor->clear_flag(Obj_flags::read);
		else
			actor->set_flag(Obj_flags::read);
		break;
		
		case '5':	// Petra
		if (actor->get_flag(Obj_flags::petra))
			actor->clear_flag(Obj_flags::petra);
		else
			actor->set_flag(Obj_flags::petra);
		break;
		
		case '7':	// Naked
		if (actor->get_siflag(Actor::naked))
			actor->clear_siflag(Actor::naked);
		else
			actor->set_siflag(Actor::naked);
		break;
		
		case 't':	// Met
		if (actor->get_flag(Obj_flags::met))
			actor->clear_flag(Obj_flags::met);
		else
			actor->set_flag(Obj_flags::met);
		break;
		
		case 'u':	// No Cast
		if (actor->get_flag(Obj_flags::no_spell_casting))
			actor->clear_flag(Obj_flags::no_spell_casting);
		else
			actor->set_flag(Obj_flags::no_spell_casting);
		break;
		
		case 'z':	// Zombie
		if (actor->get_flag(Obj_flags::si_zombie))
			actor->clear_flag(Obj_flags::si_zombie);
		else
			actor->set_flag(Obj_flags::si_zombie);
		break;

		case 'w':	// Freeze
		if (actor->get_flag(Obj_flags::freeze))
			actor->clear_flag(Obj_flags::freeze);
		else
			actor->set_flag(Obj_flags::freeze);
		break;

		case 'i':	// Party
		if (actor->get_flag(Obj_flags::in_party))
			gwin->get_party_man()->remove_from_party(actor);
		else
			gwin->get_party_man()->add_to_party(actor);
		break;
		
		case 'o':	// Protectee
		break;
		
		// Value
		case 'v':	// ID
		if (i < -1) mode = CP_InvalidValue;
		else if (i > 63) mode = CP_InvalidValue;
		else if (i == -1 || !input[0]) mode = CP_Canceled;
		else actor->set_ident(i);
		break;
		
		case '1':	// Skin
		break;
		
		case 'y':	// Warmth
		break;
		
		case '2':	// Polymorph

		// Clear polymorph
		if (actor->get_polymorph() != -1)
		{
			actor->set_polymorph(actor->get_polymorph());
			break;
		}

		if (input[0] == 'b')	// Browser
		{
			int n;
			if (!cheat.get_browser_shape(i, n))
			{
				mode = CP_WrongShapeFile;
				break;
			}
		}

		if (i == -1) mode = CP_Canceled;
		else if (i < 0) mode = CP_InvalidShape;
		else if (i >= nshapes) mode = CP_InvalidShape;
		else if (input[0] && (input[0] != '-' || input[1]))
		{
			actor->set_polymorph(i);
			mode = CP_ShapeSet;
		}

		break;

		// Advanced Numeric Flag Editor
		case '*':
		if (i < -1) mode = CP_InvalidValue;
		else if (i > 63) mode = CP_InvalidValue;
		else if (i == -1 || !input[0]) mode = CP_Canceled;
		else mode = AdvancedFlagLoop(i, actor);
		break;

		default:
		break;
	}
	for (i = 0; i < 17; i++) input[i] = 0;
	command = 0;
}

// Checks the input
bool CheatScreen::FlagCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor)
{
	switch(command)
	{
		// Everyone

		// Toggles
		case 'a':	// Asleep
		case 'b':	// Charmed
		case 'c':	// Cursed
		case 'd':	// Paralyzed
		case 'e':	// Poisoned
		case 'f':	// Protected
		case 'i':	// Party
		case 'j':	// Invisible
		case 'k':	// Fly
		case 'l':	// Walk
		case 'm':	// Swim
		case 'n':	// Ethrel
		case 'o':	// Protectee
		case 'p':	// Conjured
		case 'q':	// Summoned
		case 'r':	// Bleedin
		case 'w':	// Freeze
		//case '3':	// Tournament
		activate = true;
		input[0] = command;
		break;

		// Value
		case '2':	// Polymorph
		if (actor->get_polymorph() == -1)
		{
			mode = CP_Shape;
			input[0] = 0;
		}
		else
		{
			activate = true;
			input[0] = command;
		}
		break;


		// Party Only

		// Value
		case 'y':	// Temp
		if (!actor->is_in_party()) command = 0;
		else  mode = CP_NotAvail;
		input[0] = command;
		break;


		// Avatar Only

		// Toggles
		case 's':	// Sex
		case '4':	// Read
		if (actor->get_npc_num()) command = 0;
		else activate = true;
		input[0] = command;
		break;

		// Toggles SI
		case '5':	// Petra
		case '7':	// Naked
		if (Game::get_game_type() != SERPENT_ISLE || actor->get_npc_num()) command = 0;
		else activate = true;
		input[0] = command;
		break; 

		// Value SI
		case '1':	// Skin
		if (Game::get_game_type() != SERPENT_ISLE || actor->get_npc_num()) command = 0;
		else mode = CP_NotAvail;
		input[0] = command;
		break;


		// Everyone but avatar

		// Toggles
		case 't':	// Met
		case 'u':	// No Cast
		case 'z':	// Zombie
		if (!actor->get_npc_num()) command = 0;
		else activate = true;
		input[0] = command;
		break;

		// Value
		case 'v':	// ID
		if (!actor->get_npc_num()) command = 0;
		else mode = CP_EnterValue;
		input[0] = 0; 
		break;

		// NPC Flag Editor

		case SDLK_KP_MULTIPLY:
		case '8':
		command = '*';
		input[0] = 0;
		mode = CP_NFlagNum;
		break;


		// X and Escape leave
		case SDLK_ESCAPE:
		case 'x':
		input[0] = command;
		return false;

		// Unknown
		default:
		command = 0;
		break;
	}

	return true;
}

//
// Business Schedules
//

void CheatScreen::BusinessLoop (Actor *actor)
{
	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[17];
	int i;
	int command;
	bool activate = false;
	int time = 0;
	int prev = 0;
		
	for (i = 0; i < 17; i++) input[i] = 0;

	while (looping)
	{
		gwin->clear_screen();

		// First the display
		if (mode == CP_Activity)
            ActivityDisplay();
		else
            BusinessDisplay(actor);

		// Now the Menu Column
		BusinessMenu(actor);

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			BusinessActivate(input, command, mode, actor, time, prev);
			activate = false;
			continue;			
		}

		if (SharedInput (input, 17, command, mode, activate))
			looping = BusinessCheck(input, command, mode, activate, actor, time);
	}
}

void CheatScreen::BusinessDisplay (Actor *actor)
{
	char	buf[512];
	Tile_coord t = actor->get_tile();

	// Now the info
	std::string namestr = actor->get_npc_name();
	snprintf (buf, 512, "NPC %i - %s", actor->get_npc_num(), namestr.c_str());
	font->paint_text_fixedwidth(ibuf, buf, 0, 0, 8);

	snprintf (buf, 512, "Loc (%04i, %04i, %02i)", t.tx, t.ty, t.tz);
	font->paint_text_fixedwidth(ibuf, buf, 0, 8, 8);

	snprintf (buf, 512, "Current Activity:  %2i - %s", actor->get_schedule_type(), schedules[actor->get_schedule_type()]);
	font->paint_text_fixedwidth(ibuf, buf, 0, 16, 8);
	

	// Avatar can't have schedules
	if (actor->get_npc_num() > 0)
	{
		font->paint_text_fixedwidth(ibuf, "Schedules:", 0, 28, 8);

		Schedule_change	*scheds;
		int num;
		int types[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
		int x[8];
		int y[8];
		int time;
		int i;
		Tile_coord tile;

		actor->get_schedules (scheds, num);

		for (i = 0; i < num; i++)
		{
			time = scheds[i].get_time();
			types[time] = scheds[i].get_type();
			tile = scheds[i].get_pos();
			x[time] = tile.tx;
			y[time] = tile.ty;
		}

		font->paint_text_fixedwidth(ibuf, "12 AM:", 0, 36, 8);
		font->paint_text_fixedwidth(ibuf, " 3 AM:", 0, 44, 8);
		font->paint_text_fixedwidth(ibuf, " 6 AM:", 0, 52, 8);
		font->paint_text_fixedwidth(ibuf, " 9 AM:", 0, 60, 8);
		font->paint_text_fixedwidth(ibuf, "12 PM:", 0, 68, 8);
		font->paint_text_fixedwidth(ibuf, " 3 PM:", 0, 76, 8);
		font->paint_text_fixedwidth(ibuf, " 6 PM:", 0, 84, 8);
		font->paint_text_fixedwidth(ibuf, " 9 PM:", 0, 92, 8);
	
		for (i = 0; i < 8; i++) if (types[i] != -1)
		{
			snprintf (buf, 512, "%2i (%4i,%4i) - %s", types[i], x[i], y[i], schedules[types[i]]);
			font->paint_text_fixedwidth(ibuf, buf, 56, 36+i*8, 8);
		}
	}
}

void CheatScreen::BusinessMenu (Actor *actor)
{
	// Left Column

	// Might break on monster npcs?
	if (actor->get_npc_num() > 0)
	{
		font->paint_text_fixedwidth(ibuf, "12 AM: [A] Set  [I] Location  [1] Clear", 0, maxy-96, 8);
		font->paint_text_fixedwidth(ibuf, " 3 AM: [B] Set  [J] Location  [2] Clear", 0, maxy-88, 8);
		font->paint_text_fixedwidth(ibuf, " 6 AM: [C] Set  [K] Location  [3] Clear", 0, maxy-80, 8);
		font->paint_text_fixedwidth(ibuf, " 9 AM: [D] Set  [L] Location  [4] Clear", 0, maxy-72, 8);
		font->paint_text_fixedwidth(ibuf, "12 PM: [E] Set  [M] Location  [5] Clear", 0, maxy-64, 8);
		font->paint_text_fixedwidth(ibuf, " 3 PM: [F] Set  [N] Location  [6] Clear", 0, maxy-56, 8);
		font->paint_text_fixedwidth(ibuf, " 6 PM: [G] Set  [O] Location  [7] Clear", 0, maxy-48, 8);
		font->paint_text_fixedwidth(ibuf, " 9 PM: [H] Set  [P] Location  [8] Clear", 0, maxy-40, 8);

	       font->paint_text_fixedwidth(ibuf, "[S]et Current Activity [X]it [R]evert", 0, maxy-30, 8);
	}
	else
	        font->paint_text_fixedwidth(ibuf, "[S]et Current Activity [X]it", 0, maxy-30, 8);
}

void CheatScreen::BusinessActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor, int &time, int &prev)
{
	int i = std::atoi(input);

	mode = CP_Command;
	int old = command;
	command = 0;
	switch (old)
	{
		case 'a':	// Set Activity
		if (i < -1 || i > 31) mode = CP_InvalidValue;
		else if (i == -1) mode = CP_Canceled;
		else if (!input[0])
		{
			mode = CP_Activity;
			command = 'a';
		}
		else
		{
			actor->set_schedule_time_type(time, i);
		}
		break;

		case 'i':	// X Coord
		if (i < -1 || i > c_num_tiles) mode = CP_InvalidValue;
		else if (i == -1) mode = CP_Canceled;
		else if (!input[0])
		{
			mode = CP_XCoord;
			command = 'i';
		}
		else
		{
			prev = i;
			mode = CP_YCoord;
			command = 'j';
		}
		break;

		case 'j':	// Y Coord
		if (i < -1 || i > c_num_tiles) mode = CP_InvalidValue;
		else if (i == -1) mode = CP_Canceled;
		else if (!input[0])
		{
			mode = CP_YCoord;
			command = 'j';
		}
		else
		{
			actor->set_schedule_time_location(time, prev, i);
		}
		break;


		case '1':	// Clear
		actor->remove_schedule(time);
		break;


		case 's':	// Set Current
		if (i < -1 || i > 31) mode = CP_InvalidValue;
		else if (i == -1) mode = CP_Canceled;
		else if (!input[0])
		{
			mode = CP_Activity;
			command = 's';
		}
		else
		{
			actor->set_schedule_type(i);
		}
		break;


		case 'r':	// Revert
		Game_window::get_instance()->revert_schedules(actor);
		break;


		default:
		break;
	}
	for (i = 0; i < 17; i++) input[i] = 0;
}

// Checks the input
bool CheatScreen::BusinessCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor, int &time)
{
	// Might break on monster npcs?
	if (actor->get_npc_num() > 0) switch(command)
	{
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		time = command -'a';
		command = 'a';
		mode = CP_Activity;
		return true;

		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		time = command -'i';
		command = 'i';
		mode = CP_XCoord;
		return true;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		time = command -'1';
		command = '1';
		activate = true;
		return true;

		case 'r':
		command = 'r';
		activate = true;
		return true;

		default:
		break;
	}

	switch(command)
	{
		// Set Current
		case 's':
		command = 's';
		input[0] = 0;
		mode = CP_Activity;
		break;
		

		// X and Escape leave
		case SDLK_ESCAPE:
		case 'x':
		input[0] = command;
		return false;

		// Unknown
		default:
		command = 0;
		mode = CP_InvalidCom;
		break;
	}

	return true;
}

//
// NPC Stats
//

void CheatScreen::StatLoop (Actor *actor)
{
	int num = actor->get_npc_num();
	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[17];
	int i;
	int command;
	bool activate = false;
		
	for (i = 0; i < 17; i++) input[i] = 0;

	while (looping)
	{
		gwin->clear_screen();

		// First the display
		NPCDisplay(actor, num);

		// Now the Menu Column
		StatMenu(actor);

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			StatActivate(input, command, mode, actor);
			activate = false;
			continue;			
		}

		if (SharedInput (input, 17, command, mode, activate))
			looping = StatCheck(input, command, mode, activate, actor);
	}
}

void CheatScreen::StatMenu (Actor *actor)
{
	char	buf[512];

	// Left Column

	// Dexterity
	snprintf (buf, 512, "[D]exterity....%3i", actor->get_property(Actor::dexterity));
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-108, 8);

	// Food Level
	snprintf (buf, 512, "[F]ood Level...%3i", actor->get_property(Actor::food_level));
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-99, 8);

	// Intelligence
	snprintf (buf, 512, "[I]ntellicence.%3i", actor->get_property(Actor::intelligence));
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-90, 8);

	// Strength
	snprintf (buf, 512, "[S]trength.....%3i", actor->get_property(Actor::strength));
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-81, 8);

	// Combat Skill
	snprintf (buf, 512, "[C]ombat Skill.%3i", actor->get_property(Actor::combat));
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-72, 8);

	// Hit Points
	snprintf (buf, 512, "[H]it Points...%3i", actor->get_property(Actor::health));
	font->paint_text_fixedwidth(ibuf, buf, 0, maxy-63, 8);

	// Magic - Avatar Only
	if (!actor->get_npc_num())
	{
		// Magic Points
		snprintf (buf, 512, "[M]agic Points.%3i", actor->get_property(Actor::magic));
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-54, 8);

		// Mana
		snprintf (buf, 512, "[V]ana Level...%3i", actor->get_property(Actor::mana));
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-45, 8);
	}

	// Exit
	font->paint_text_fixedwidth(ibuf, "[X]it", 0, maxy-36, 8);


}

void CheatScreen::StatActivate (char *input, int &command, Cheat_Prompt &mode, Actor *actor)
{
	int i = std::atoi(input);

	mode = CP_Command;
	switch (command)
	{
		case 'd':	// Dexterity
		actor->set_property(Actor::dexterity, i);
		break;
		
		case 'f':	// Food Level
		actor->set_property(Actor::food_level, i);
		break;
		
		case 'i':	// Intelligence
		actor->set_property(Actor::intelligence, i);
		break;
		
		case 's':	// Strength
		actor->set_property(Actor::strength, i);
		break;
		
		case 'c':	// Combat Points
		actor->set_property(Actor::combat, i);
		break;
		
		case 'h':	// Hit Points
		actor->set_property(Actor::health, i);
		break;
		
		case 'm':	// Magic
		actor->set_property(Actor::magic, i);
		break;
		
		case 'v':	// [V]ana
		actor->set_property(Actor::mana, i);
		break;
		

		default:
		break;
	}
	for (i = 0; i < 17; i++) input[i] = 0;
	command = 0;
}

// Checks the input
bool CheatScreen::StatCheck (char *input, int &command, Cheat_Prompt &mode, bool &activate, Actor *actor)
{
	switch(command)
	{
		// Everyone
		case 'd':	// Dexterity
		case 'f':	// Food Level
		case 'i':	// Intelligence
		case 's':	// Strength
		case 'c':	// Combat Points
		case 'h':	// Hit Points
		input[0] = 0;
		mode = CP_EnterValue;
		break;

		// Avatar Only
		case 'm':	// Magic
		case 'v':	// [V]ana
		if (actor->get_npc_num()) command = 0;
		else mode = CP_EnterValue;
		input[0] = 0;
		break;

		// X and Escape leave
		case SDLK_ESCAPE:
		case 'x':
		input[0] = command;
		return false;

		// Unknown
		default:
		command = 0;
		break;
	}

	return true;
}


//
// Advanced Flag Edition
//

CheatScreen::Cheat_Prompt CheatScreen::AdvancedFlagLoop (int num, Actor *actor)
{
	int npc_num = actor->get_npc_num();
	bool looping = true;

	// This is for the prompt message
	Cheat_Prompt mode = CP_Command;

	// This is the command
	char input[17];
	int i;
	int command;
	bool activate = false;
	char	buf[512];
		
	for (i = 0; i < 17; i++) input[i] = 0;

	while (looping)
	{
		gwin->clear_screen();

		NPCDisplay(actor, npc_num);

		if (num < 0) num = 0;
		else if (num > 63) num = 63;

		// First the info
		if (flag_names[num])
			snprintf (buf, 512, "NPC Flag %i: %s", num, flag_names[num]);
		else
			snprintf (buf, 512, "NPC Flag %i", num);

		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-108, 8);

		snprintf (buf, 512, "Flag is %s", actor->get_flag(num)?"SET":"UNSET");
		font->paint_text_fixedwidth(ibuf, buf, 0, maxy-90, 8);


		// Now the Menu Column
		if (!actor->get_flag(num)) font->paint_text_fixedwidth(ibuf, "[S]et Flag", 160, maxy-90, 8);
		else font->paint_text_fixedwidth(ibuf, "[U]nset Flag", 160, maxy-90, 8);

		// Change Flag
		font->paint_text_fixedwidth(ibuf, "[*] Change Flag", 0, maxy-72, 8);
		if (num > 0 && num < 63) font->paint_text_fixedwidth(ibuf, "[+-] Scroll Flags", 0, maxy-63, 8);
		else if (num == 0) font->paint_text_fixedwidth(ibuf, "[+] Scroll Flags", 0, maxy-63, 8);
		else font->paint_text_fixedwidth(ibuf, "[-] Scroll Flags", 0, maxy-63, 8);

		font->paint_text_fixedwidth(ibuf, "[X]it", 0, maxy-36, 8);

		// Finally the Prompt...
		SharedPrompt(input, mode);

		// Draw it!
		pal.apply();

		// Check to see if we need to change menus
		if (activate)
		{
			if (command == '-')		// Decrement
			{
				num--;
				if (num < 0) num = 0;
			}
			else if (command == '+')	// Increment
			{
				num++;
				if (num > 63) num = 63;
			}
			else if (command == '*')	// Change Flag
			{
				i = std::atoi(input);
				if (i < -1 || i > 63) mode = CP_InvalidValue;
				else if (i == -1) mode = CP_Canceled;
				else if (input[0]) num = i;
			}
			else if (command == 's')	// Set
			{
				actor->set_flag(num);
			}
			else if (command == 'u')	// Unset
			{
				actor->clear_flag(num);
			}

			for (i = 0; i < 17; i++) input[i] = 0;
			mode = CP_Command;
			command = 0;
			activate = false;
			continue;			
		}

		if (SharedInput (input, 17, command, mode, activate))
		{
			switch(command)
			{
				// Simple commands
				case 's':	// Set Flag
				case 'u':	// Unset flag
				input[0] = command;
				activate = true;
				break;

				// Decrement
				case SDLK_KP_MINUS:
				case '-':
				command = '-';
				input[0] = command;
				activate = true;
				break;

				// Increment
				case SDLK_KP_PLUS:
				case '=':
				command = '+';
				input[0] = command;
				activate = true;
				break;

				// * Change Flag
				case SDLK_KP_MULTIPLY:
				case '8':
				command = '*';
				input[0] = 0;
				mode = CP_NFlagNum;
				break;

				// X and Escape leave
				case SDLK_ESCAPE:
				case 'x':
				input[0] = command;
				looping = false;
				break;

				default:
				mode = CP_InvalidCom;
				input[0] = command;
				command = 0;
				break;
			}
		}
	}
	return CP_Command;
}
