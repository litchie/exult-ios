/*
Copyright (C) 2001-2011 The Exult Team

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

#ifdef UNDER_CE

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"

#include "Keyboard_gump.h"
#include "touchscreen.h"
#include "exult.h"
#include "exult_pocketpc_flx.h"
#include "keyactions.h"
#include <string>

using std::string;

static const int numrows = 4;
static const int colspace = 15;
static const int rowspace = 15;
static const int colx[4] = { 3, 3+(colspace/2), 3+colspace, 3+(colspace/2) };
static const int rowy = 4;
static const int keywidth = 13;
static const int keyheight = 13;
static const int maxkeysinrow = 14;
static const SDLKey keypadtable[13] = {
					SDLK_KP7, SDLK_KP8, SDLK_KP9, 
					SDLK_KP4, SDLK_KP5, SDLK_KP6, SDLK_KP_MINUS, SDLK_KP_PLUS,
					SDLK_KP1, SDLK_KP2, SDLK_KP3, 
					SDLK_KP0, SDLK_KP_PERIOD, 
};

typedef void(*ActionFunc)(int *);
static const int numhptablekeys = 11;
static const ActionFunc hotpadtable[numhptablekeys] = {
	ActionWalkNorthWest,	ActionWalkNorth,	ActionWalkNorthEast,
	ActionWalkWest,			ActionInventory,	ActionWalkEast,		ActionCombat,	ActionMinimizeGame,
	ActionWalkSouthWest,	ActionWalkSouth,	ActionWalkSouthEast,
	};

static const int hotpadtableparams[numhptablekeys] = {
	1,	1,	1,
	1,	0,	1,	-1,	-1,
	1,	1,	1,
};

static const char Ucharrows[numrows][maxkeysinrow] = { "~!@#$%^&*()_+", "QWERTYUIOP{}|", "ASDFGHJKL:\"", "ZXCVBNM<>?" };
static const char Lcharrows[numrows][maxkeysinrow] = { "`1234567890-=", "qwertyuiop[]\\", "asdfghjkl;'", "zxcvbnm,./" };

static const enum OtherButtons {	KEYG_STARTOFKEYBOARD,
									KEYG_CASECHANGE,
									KEYG_ENTER,
									KEYG_ALT,
									KEYG_CTRL,
									KEYG_SPACE,
									KEYG_BACKSPACE,
									KEYG_LARROW,
									KEYG_RARROW,
									KEYG_DONE,
									KEYG_SHOWPAD,
									KEYG_KEYBOARD_MOVEUPPERLEFT,
									KEYG_KEYBOARD_MOVEUPPERRIGHT,
									KEYG_KEYBOARD_MOVELOWERLEFT,
									KEYG_KEYBOARD_MOVELOWERRIGHT,
									KEYG_ENDOFKEYBOARD,
									KEYG_STARTOFMINIMIZED,
									KEYG_SHOW,
									KEYG_MIN_MOVEUPPERLEFT,
									KEYG_MIN_MOVEUPPERRIGHT,
									KEYG_MIN_MOVELOWERLEFT,
									KEYG_MIN_MOVELOWERRIGHT,
									KEYG_ENDOFMINIMIZED,
									KEYG_STARTOFKEYPAD,
									KEYG_DONEWITHKEYPAD,
									KEYG_KEYPAD_TOKEYBOARD,
									KEYG_KEYPAD_TOHOTPAD,
									KEYG_KEYPAD_MOVEUPPERLEFT,
									KEYG_KEYPAD_MOVEUPPERRIGHT,
									KEYG_KEYPAD_MOVELOWERLEFT,
									KEYG_KEYPAD_MOVELOWERRIGHT,
									KEYG_KP7,
									KEYG_KP8,
									KEYG_KP9,
									KEYG_KP4,
									KEYG_KP5,
									KEYG_KP6,
									KEYG_KPMINUS,
									KEYG_KPPLUS,
									KEYG_KP1,
									KEYG_KP2,
									KEYG_KP3,
									KEYG_KP0,
									KEYG_KPPERIOD,
									KEYG_KPCLICKDOUBLE,
									KEYG_KPCLICKRIGHT,
									KEYG_KPF5,
									KEYG_KPF6,
									KEYG_KPF7,
									KEYG_ENDOFKEYPAD,
									KEYG_STARTOFHOTPAD,
									KEYG_DONEWITHHOTPAD,
									KEYG_HOTPAD_TOKEYBOARD,
									KEYG_HOTPAD_TOKEYPAD,
									KEYG_HOTPAD_MOVEUPPERLEFT,
									KEYG_HOTPAD_MOVEUPPERRIGHT,
									KEYG_HOTPAD_MOVELOWERLEFT,
									KEYG_HOTPAD_MOVELOWERRIGHT,
									KEYG_HPNW,
									KEYG_HPN,
									KEYG_HPNE,
									KEYG_HPW,
									KEYG_HPPAPERDOLL,
									KEYG_HPE,
									KEYG_HPCOMBAT,
									KEYG_HPMINIMIZEGAME,
									KEYG_HPSW,
									KEYG_HPS,
									KEYG_HPSE,
									KEYG_HPF11,
									KEYG_HPF12,
									KEYG_HPCLICKDOUBLE,
									KEYG_HPCLICKRIGHT,
									KEYG_HPF5,
									KEYG_HPF6,
									KEYG_HPF7,
									KEYG_ENDOFHOTPAD,
									NUMOTHERBUTTONS,
								};
static const int OBI_LEN = 4;
static const int OtherButtonsInfo[NUMOTHERBUTTONS * OBI_LEN] = {
						 -1,  -1,  -1,  -1, // KEYG_STARTOFKEYBOARD
						  3,  64,  41,  76, // KEYG_CASECHANGE
					    160,  49, 174,  76, // KEYG_ENTER
						108,  64, 127,  76, // KEYG_ALT
						 44,  64,  69,  76, // KEYG_CTRL
						 72,  64, 105,  76, // KEYG_SPACE
						183,  34, 202,  46, // KEYG_BACKSPACE
						130,  64, 142,  76, // KEYG_LARROW
						145,  64, 157,  76, // KEYG_RARROW
						184,  61, 198,  69, // KEYG_DONE
						 3,   34,  15,  46, // KEYG_SHOWPAD
						179,  53, 190,  60, // KEYG_KEYBOARD_MOVEUPPERLEFT
						191,  53, 202,  60, // KEYG_KEYBOARD_MOVEUPPERRIGHT
						179,  70, 190,  77, // KEYG_KEYBOARD_MOVELOWERLEFT
						191,  70, 202,  77, // KEYG_KEYBOARD_MOVELOWERRIGHT
					 	 -1,  -1,  -1,  -1, // KEYG_ENDOFKEYBOARD
					 	 -1,  -1,  -1,  -1, // KEYG_STARTOFMINIMIZED
  						  5,   8,  19,  16, // KEYG_SHOW
						  0,   0,  11,   7, // KEYG_MIN_MOVEUPPERLEFT
						 12,   0,  24,   7, // KEYG_MIN_MOVEUPPERRIGHT
						  0,  17,  11,  24, // KEYG_MIN_MOVELOWERLEFT
						 12,  17,  24,  24, // KEYG_MIN_MOVELOWERRIGHT
						 -1,  -1,  -1,  -1, // KEYG_ENDOFMINIMIZED
						 -1,  -1,  -1,  -1, // KEYG_STARTOFKEYPAD
						 57,  45,  70,  53, // KEYG_DONEWITHKEYPAD
						 33,  48,  45,  60, // KEYG_KEYPAD_TOKEYBOARD
						 48,   3,  75,  15, // KEYG_KEYPAD_TOHOTPAD
						 52,  37,  62,  44, // KEYG_KEYPAD_MOVEUPPERLEFT
						 64,  37,  75,  44, // KEYG_KEYPAD_MOVEUPPERRIGHT
						 52,  54,  62,  61, // KEYG_KEYPAD_MOVELOWERLEFT
						 64,  54,  75,  61, // KEYG_KEYPAD_MOVELOWERRIGHT
						  3,   4,  15,  15,	// KEYG_KP7
						 18,   4,  30,  15,	// KEYG_KP8
						 33,   4,  45,  15,	// KEYG_KP9
						  3,  19,  15,  30,	// KEYG_KP4
						 18,  19,  30,  30,	// KEYG_KP5
						 33,  19,  45,  30,	// KEYG_KP6
						 48,  19,  60,  30,	// KEYG_KPMINUS
						 63,  19,  75,  30,	// KEYG_KPPLUS
						  3,  33,  15,  45,	// KEYG_KP1
						 18,  33,  30,  45, // KEYG_KP2
						 33,  33,  45,  45,	// KEYG_KP3
						  3,  48,  15,  60,	// KEYG_KP0
						 18,  48,  30,  60,	// KEYG_KPPERIOD
						  3,  63,  15,  75, // KEYG_KPCLICKDOUBLE
						 18,  63,  30,  75, // KEYG_KPCLICKRIGHT
						 33,  63,  45,  75, // KEYG_KPF5
						 48,  63,  60,  75, // KEYG_KPF6
						 63,  63,  75,  75, // KEYG_KPF7
						 -1,  -1,  -1,  -1, // KEYG_ENDOFKEYPAD
						 -1,  -1,  -1,  -1, // KEYG_STARTOFHOTPAD
						 57,  45,  70,  53, // KEYG_DONEWITHHOTPAD
						 33,  48,  45,  60, // KEYG_HOTPAD_TOKEYBOARD
						 48,   3,  75,  15, // KEYG_HOTPAD_TOKEYPAD
						 52,  37,  62,  44, // KEYG_HOTPAD_MOVEUPPERLEFT
						 64,  37,  75,  44, // KEYG_HOTPAD_MOVEUPPERRIGHT
						 52,  54,  62,  61, // KEYG_HOTPAD_MOVELOWERLEFT
						 64,  54,  75,  61, // KEYG_HOTPAD_MOVELOWERRIGHT
						  3,   4,  15,  15,	// KEYG_HPNW
						 18,   4,  30,  15,	// KEYG_HPN
						 33,   4,  45,  15,	// KEYG_HPNE
						  3,  19,  15,  30,	// KEYG_HPW
						 18,  19,  30,  30,	// KEYG_HPPAPERDOLL
						 33,  19,  45,  30,	// KEYG_HPE
						 48,  19,  60,  30,	// KEYG_HPCOMBAT
						 63,  19,  75,  30,	// KEYG_HPMINIMIZEGAME
						  3,  33,  15,  45,	// KEYG_HPSW
						 18,  33,  30,  45, // KEYG_HPS
						 33,  33,  45,  45,	// KEYG_HPSE
						  3,  48,  15,  60,	// KEYG_HPF11
						 18,  48,  30,  60,	// KEYG_HPF12
						  3,  63,  15,  75, // KEYG_HPCLICKDOUBLE
						 18,  63,  30,  75, // KEYG_HPCLICKRIGHT
						 33,  63,  45,  75, // KEYG_HPF5
						 48,  63,  60,  75, // KEYG_HPF6
						 63,  63,  75,  75, // KEYG_HPF7
						 -1,  -1,  -1,  -1, // KEYG_ENDOFHOTPAD
};

Keyboard_gump::Keyboard_gump(int placex, int placey, bool upperCase)
{
	autopaint = true;
	buttonDown[0] = -1;
	buttonDown[1] = -1;
	buttonDown[2] = -1;
	buttonDown[3] = -1;
	config->value("config/gameplay/onscreenkeyboardstate", laststate, KEYG_KEYBOARD);
	config->value("config/gameplay/onscreenkeyboardcorner", loccorner, 0);
	if (laststate < KEYG_KEYBOARD || laststate >= KEYG_MINIMIZED)
		laststate = KEYG_KEYBOARD;
	if (loccorner < 0 || loccorner > 4)
		loccorner = 0; // 0 = upper left, 1 = upper right, 2 = lower left, 3 = lower right
	//if (lastpadstate != KEYG_KEYPAD && lastpadstate != KEYG_HOTPAD)
	lastpadstate = KEYG_KEYPAD;
	altDown = false;
	ctrlDown = false;
	state = KEYG_HIDDEN;
	pocketpc_vga.load(POCKETPC_FLX);
	width = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, 0)->get_width();
	height = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, 0)->get_height();
	if (upperCase)
	{
		caseSet = &Ucharrows[0][0];
	}
	else
	{
		caseSet = &Ucharrows[0][0];
	}
	locx = placex;
	locy = placey;
	hide();
}

int Keyboard_gump::getState(void)
{
	return state;
}

int Keyboard_gump::getCorner(void)
{
	return loccorner;
}

void Keyboard_gump::show(int corner, int newstate)
{
	Game_window *gwin = Game_window::get_instance();
	if (state != KEYG_HIDDEN)
		gwin->add_dirty(RECTX(locx, locy, width+4, height+4));

	if (newstate == -1)
	{
		newstate = state;
		if (newstate < 0 || newstate > KEYG_HOTPAD)
			newstate = laststate;
	}
	if (newstate == KEYG_KEYBOARD)
	{
		width = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, 0)->get_width();
		height = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, 0)->get_height();
	}
	else if (newstate == KEYG_KEYPAD || newstate == KEYG_HOTPAD)
	{
		width = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYPAD_SHP, 0)->get_width();
		height = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYPAD_SHP, 0)->get_height();
	}
	if (corner == -1)
		corner = loccorner;

	state = newstate;
	moveToCorner(corner);
	config->set("config/gameplay/onscreenkeyboardcorner", loccorner, false);
	config->set("config/gameplay/onscreenkeyboardstate", state, true);
	gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
	if (autopaint)
		gwin->paint_dirty();
}

void Keyboard_gump::hide()
{
	if (state != KEYG_MINIMIZED && state != KEYG_HIDDEN)
	{
		laststate = state;
	}
	state = KEYG_HIDDEN;
	Game_window *gwin = Game_window::get_instance();
	if (gwin != NULL)
	{
		gwin->set_all_dirty();
		if (autopaint)
			gwin->paint_dirty();
	}
}

void Keyboard_gump::minimize(int placearea)
{
	if (state == KEYG_MINIMIZED)
		return;
	Game_window *gwin = Game_window::get_instance();
	if (state != KEYG_HIDDEN)
	{
		gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
		laststate = state;
	}

	width = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_MINIMIZEDKBD_SHP, 0)->get_width();
	height = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_MINIMIZEDKBD_SHP, 0)->get_height();
	
	locx = (gwin->get_width() / 2) - (width / 2);
	if (placearea == KEYG_LOCBOTTOM)
		locy = gwin->get_height() - height;
	else // if (placearea == KEYG_LOCTOP
		locy = 0;
	state = KEYG_MINIMIZED;
	gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
	if (autopaint)
		gwin->paint_dirty();
}

Keyboard_gump::~Keyboard_gump()
{

}


void Keyboard_gump::paint()
{
	Game_window *gwin = Game_window::get_instance();
	if (state == KEYG_KEYBOARD)
	{
		int caseidx = 0; // Uppercase by default
		if (caseSet == &Lcharrows[0][0])
			caseidx = 1;  // Lowercase
		Shape_manager::get_instance()->paint_shape(locx, locy,
					pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, caseidx), 0);
		if (altDown)
			areaHighlight(OtherButtonsInfo[(KEYG_ALT*OBI_LEN)+0], OtherButtonsInfo[(KEYG_ALT*OBI_LEN)+1], 
							OtherButtonsInfo[(KEYG_ALT*OBI_LEN)+2]+1, OtherButtonsInfo[(KEYG_ALT*OBI_LEN)+3]+1);
		if (ctrlDown)
			areaHighlight(OtherButtonsInfo[(KEYG_CTRL*OBI_LEN)+0], OtherButtonsInfo[(KEYG_CTRL*OBI_LEN)+1], 
							OtherButtonsInfo[(KEYG_CTRL*OBI_LEN)+2]+1, OtherButtonsInfo[(KEYG_CTRL*OBI_LEN)+3]+1);
	}
	else if (state == KEYG_KEYPAD)
	{
		Shape_manager::get_instance()->paint_shape(locx, locy,
					pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYPAD_SHP, 0), 0);
	}
	else if (state == KEYG_HOTPAD)
	{
		Shape_manager::get_instance()->paint_shape(locx, locy,
					pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYPAD_SHP, 1), 0);
	}
	else if (state == KEYG_MINIMIZED)
	{
		Shape_manager::get_instance()->paint_shape(locx, locy,
					pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_MINIMIZEDKBD_SHP, 0), 0);
	}
	if (state == KEYG_KEYPAD || state == KEYG_HOTPAD)
	{
		int Right, Double;
		Touchscreen->getModes(&Right, &Double);
		if (Right)
			areaHighlight(OtherButtonsInfo[(KEYG_HPCLICKRIGHT*OBI_LEN)+0], OtherButtonsInfo[(KEYG_HPCLICKRIGHT*OBI_LEN)+1], 
							OtherButtonsInfo[(KEYG_HPCLICKRIGHT*OBI_LEN)+2]+1, OtherButtonsInfo[(KEYG_HPCLICKRIGHT*OBI_LEN)+3]+1);
		if (Double)
			areaHighlight(OtherButtonsInfo[(KEYG_HPCLICKDOUBLE*OBI_LEN)+0], OtherButtonsInfo[(KEYG_HPCLICKDOUBLE*OBI_LEN)+1], 
							OtherButtonsInfo[(KEYG_HPCLICKDOUBLE*OBI_LEN)+2]+1, OtherButtonsInfo[(KEYG_HPCLICKDOUBLE*OBI_LEN)+3]+1);
	}
	if (state != KEYG_HIDDEN)
	{
		if (buttonDown[0] >= 0 && buttonDown[1] >= 0 && buttonDown[2] >= 0 && buttonDown[3] >= 0)
			areaHighlight(buttonDown[0], buttonDown[1], buttonDown[2]+1, buttonDown[3]+1);
		gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
		gwin->set_painted();
	}
}

void Keyboard_gump::areaHighlight(int l, int t, int r, int b)
{
	int w = r - l;
	int h = b - t;
	Game_window *gwin = Game_window::get_instance();
	char p;
	for (int y = locy+t; y <= locy+b; y++)
	{
		for (int x = locx+l; x <= locx+r; x++)
		{
			p = gwin->get_win()->get_pixel8(x, y);
			p = abs(p - 255);
			gwin->get_win()->put_pixel8(p, x, y);
		}
	}
	gwin->add_dirty(RECTX(l, t, w+4, h+4));
}

int Keyboard_gump::handle_event(SDL_Event *event)
{
	if (state == KEYG_HIDDEN)
		return 0;
	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
	{
		void (Keyboard_gump::*mouseFunc)(int, int) = &Keyboard_gump::mouse_down; // Mouse down by default
		if (event->type == SDL_MOUSEBUTTONUP)
			mouseFunc = &Keyboard_gump::mouse_up;
		Game_window *gwin = Game_window::get_instance();
		int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
		int x = event->button.x/scale, y = event->button.y/scale;
		if (x >= locx && x <= (locx + width)
			&& y >= locy && y <= (locy + height))
		{
			(*this.*mouseFunc)(x - locx, y - locy);
			return 1;
		}
	}
	if (event->type == SDL_MOUSEBUTTONUP &&
		 buttonDown[0] >= 0 && buttonDown[1] >= 0 && buttonDown[2] >= 0 && buttonDown[3] >= 0)
	{// Need to release the button without any action
		buttonDown[0] = -1;
		buttonDown[1] = -1;
		buttonDown[2] = -1;
		buttonDown[3] = -1;
		return 1;
	}
	
	return 0;
}

void Keyboard_gump::ActivateOtherButtonMinimized(int button)
{
	switch(button)
	{
		case KEYG_SHOW:	show(-1, laststate);	break;
		case KEYG_MIN_MOVEUPPERLEFT:
		case KEYG_MIN_MOVELOWERLEFT:
		case KEYG_MIN_MOVEUPPERRIGHT:
		case KEYG_MIN_MOVELOWERRIGHT:
			show(button - KEYG_MIN_MOVEUPPERLEFT, laststate);
			break;
		default:	break;
	}
}

void Keyboard_gump::ActivateOtherButtonKeyboard(int button)
{
	switch(button)
	{
		case KEYG_SHOWPAD:	show(-1, lastpadstate);	break;
		case KEYG_DONE:		minimize();				break;
		case KEYG_CASECHANGE:
			if (caseSet == &Ucharrows[0][0])
				caseSet = &Lcharrows[0][0];
			else
				caseSet = &Ucharrows[0][0];
			break;
		case KEYG_ENTER:	injectKeyEvent('\0', SDLK_RETURN);		break;
		case KEYG_BACKSPACE:injectKeyEvent('\0', SDLK_BACKSPACE);	break;
		case KEYG_LARROW:	injectKeyEvent('\0', SDLK_LEFT);		break;
		case KEYG_RARROW:	injectKeyEvent('\0', SDLK_RIGHT);		break;
		case KEYG_SPACE:	injectKeyEvent(' ');	break;
		case KEYG_ALT:		altDown = !altDown;		break;
		case KEYG_CTRL:		ctrlDown = !ctrlDown;	break;
		case KEYG_KEYBOARD_MOVEUPPERLEFT:
		case KEYG_KEYBOARD_MOVELOWERLEFT:
		case KEYG_KEYBOARD_MOVEUPPERRIGHT:
		case KEYG_KEYBOARD_MOVELOWERRIGHT:
			show(button - KEYG_KEYBOARD_MOVEUPPERLEFT, state);
			break;
		default:	break;
	}
}

void Keyboard_gump::ActivateOtherButtonKeypad(int button)
{
	switch(button)
	{
		case KEYG_KEYPAD_TOKEYBOARD:
			lastpadstate = state;
			show(-1, KEYG_KEYBOARD);
			break;
		case KEYG_DONEWITHKEYPAD:
			lastpadstate = state;
			minimize();
			break;
		case KEYG_KEYPAD_TOHOTPAD:	show(-1, KEYG_HOTPAD);	break;
		case KEYG_KPF5:		injectKeyEvent('\0', SDLK_F5);	break;
		case KEYG_KPF6:		injectKeyEvent('\0', SDLK_F6);	break;
		case KEYG_KPF7:		injectKeyEvent('\0', SDLK_F7);	break;
		case KEYG_KPCLICKDOUBLE:Touchscreen->toggleDouble();break;
		case KEYG_KPCLICKRIGHT:	Touchscreen->toggleRight();	break;

		case KEYG_KEYPAD_MOVEUPPERLEFT:
		case KEYG_KEYPAD_MOVELOWERLEFT:
		case KEYG_KEYPAD_MOVEUPPERRIGHT:
		case KEYG_KEYPAD_MOVELOWERRIGHT:
			show(button - KEYG_KEYPAD_MOVEUPPERLEFT, state);
			break;
		case KEYG_KP7:
		case KEYG_KP8:
		case KEYG_KP9:
		case KEYG_KP4:
		case KEYG_KP5:
		case KEYG_KP6:
		case KEYG_KPMINUS:
		case KEYG_KPPLUS:
		case KEYG_KP1:
		case KEYG_KP2:
		case KEYG_KP3:
		case KEYG_KP0:
		case KEYG_KPPERIOD:
			injectKeyEvent('\0', keypadtable[button - KEYG_KP7]);
			break;
		default:
			break;
	}
}

void Keyboard_gump::ActivateOtherButtonHotpad(int button)
{
	switch(button)
	{
		case KEYG_HOTPAD_TOKEYBOARD:
			lastpadstate = state;
			show(-1, KEYG_KEYBOARD);
			break;
		case KEYG_DONEWITHHOTPAD:
			lastpadstate = state;
			minimize();
			break;
		case KEYG_HOTPAD_TOKEYPAD:	show(-1, KEYG_KEYPAD);	break;
		case KEYG_HPF5:		injectKeyEvent('\0', SDLK_F5);	break;
		case KEYG_HPF6:		injectKeyEvent('\0', SDLK_F6);	break;
		case KEYG_HPF7:		injectKeyEvent('\0', SDLK_F7);	break;
		case KEYG_HPF11:	injectKeyEvent('\0', SDLK_F11);	break;
		case KEYG_HPF12:	injectKeyEvent('\0', SDLK_F12);	break;
		case KEYG_HPCLICKDOUBLE:Touchscreen->toggleDouble();break;
		case KEYG_HPCLICKRIGHT:	Touchscreen->toggleRight();	break;

		case KEYG_HOTPAD_MOVEUPPERLEFT:
		case KEYG_HOTPAD_MOVELOWERLEFT:
		case KEYG_HOTPAD_MOVEUPPERRIGHT:
		case KEYG_HOTPAD_MOVELOWERRIGHT:
			show(button - KEYG_HOTPAD_MOVEUPPERLEFT, state);
			break;
		case KEYG_HPNW:
		case KEYG_HPN:
		case KEYG_HPNE:
		case KEYG_HPW:
		case KEYG_HPPAPERDOLL:
		case KEYG_HPE:
		case KEYG_HPCOMBAT:
		case KEYG_HPMINIMIZEGAME:
		case KEYG_HPSW:
		case KEYG_HPS:
		case KEYG_HPSE:
			int params[1];
			params[0] = hotpadtableparams[button - KEYG_HPNW];
			(*hotpadtable[button - KEYG_HPNW])(params);
			break;
		default:
			break;
	}
}

void Keyboard_gump::ActivateOtherButton(int button)
{
	switch (state)
	{
		case KEYG_MINIMIZED:
			ActivateOtherButtonMinimized(button);	break;
		case KEYG_KEYBOARD:
			ActivateOtherButtonKeyboard(button);	break;
		case KEYG_KEYPAD:
			ActivateOtherButtonKeypad(button);	break;
		case KEYG_HOTPAD:
			ActivateOtherButtonHotpad(button);	break;
		default:
			break;
	}

}

void Keyboard_gump::moveToCorner(int corner)
{	// 0 = upper left, 1 = upper right, 2 = lower left, 3 = lower right
	Game_window *gwin = Game_window::get_instance();
	gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
	int swidth = gwin->get_width();
	int sheight = gwin->get_height();
	if (corner == 0)
	{ // upper left
		locx = 0;
		locy = 0;
	}
	else if (corner == 1)
	{ // upper right
		locx = swidth - width;
		locy = 0;
	}
	else if (corner == 2)
	{ // lower left
		locx = 0;
		locy = sheight - height;
	}
	else // if (corner == 3)
	{ // lower right
		locx = swidth - width;
		locy = sheight - height;
	}
	loccorner = corner;
	if (autopaint)
	{
		gwin->paint_dirty();
		paint();
	}
}

void Keyboard_gump::mouse_down(int mx, int my)
{
	int statewhenstarted = state;
	buttonDown[0] = -1;
	buttonDown[1] = -1;
	buttonDown[2] = -1;
	buttonDown[3] = -1;
	int i,l,t,r,b;
	int start,end;
	start = KEYG_STARTOFKEYBOARD+1;
	end = KEYG_ENDOFKEYBOARD-1;
	if (state == KEYG_MINIMIZED)
	{
		start = KEYG_STARTOFMINIMIZED+1;
		end = KEYG_ENDOFMINIMIZED+1;
	}
	else if (state == KEYG_KEYPAD)
	{
		start = KEYG_STARTOFKEYPAD+1;
		end = KEYG_ENDOFKEYPAD+1;
	}
	else if (state == KEYG_HOTPAD)
	{
		start = KEYG_STARTOFHOTPAD+1;
		end = KEYG_ENDOFHOTPAD+1;
	}
	for (i = start; i <= end; i++)
	{
		l = OtherButtonsInfo[(i*OBI_LEN)+0];
		t = OtherButtonsInfo[(i*OBI_LEN)+1];
		r = OtherButtonsInfo[(i*OBI_LEN)+2];
		b = OtherButtonsInfo[(i*OBI_LEN)+3];
		if (mx >= l && mx <= r && my >= t && my <= b)
		{ // Found a button!
			buttonDown[0] = l;
			buttonDown[1] = t;
			buttonDown[2] = r;
			buttonDown[3] = b;
//			ActivateOtherButton(i);
			return;
		}
	}
	if (statewhenstarted == KEYG_KEYBOARD)
	{
		int row,col;
		for (row = 0; row < numrows; row++)
		{
			if (my >= rowy+(row*rowspace) && my <= rowy+(row*rowspace)+keyheight)
			{
				for (col = 0; col < strlen(caseSet+(row*maxkeysinrow)); col++)
				{
					if (mx >= colx[row]+(col*colspace) && mx <= colx[row]+(col*colspace)+keywidth)
					{// This is the character that was pushed!
						buttonDown[0] = colx[row]+(col*colspace);
						buttonDown[1] = rowy+(row*rowspace);
						buttonDown[2] =	colx[row]+(col*colspace)+keywidth;
						buttonDown[3] = rowy+(row*rowspace)+keyheight;
						return;
					}
				}
			}
		}
	}
}

void Keyboard_gump::mouse_up(int mx, int my)
{
	if (buttonDown[0] < 0 || buttonDown[1] < 0 || buttonDown[2] < 0 || buttonDown[3] < 0)
		return;
	int i,l,t,r,b;
	int start,end;
	int statewhenstarted = state;
	start = KEYG_STARTOFKEYBOARD+1;
	end = KEYG_ENDOFKEYBOARD-1;
	if (state == KEYG_MINIMIZED)
	{
		start = KEYG_STARTOFMINIMIZED+1;
		end = KEYG_ENDOFMINIMIZED+1;
	}
	else if (state == KEYG_KEYPAD)
	{
		start = KEYG_STARTOFKEYPAD+1;
		end = KEYG_ENDOFKEYPAD+1;
	}
	else if (state == KEYG_HOTPAD)
	{
		start = KEYG_STARTOFHOTPAD+1;
		end = KEYG_ENDOFHOTPAD+1;
	}
	for (i = start; i <= end; i++)
	{
		l = OtherButtonsInfo[(i*OBI_LEN)+0];
		t = OtherButtonsInfo[(i*OBI_LEN)+1];
		r = OtherButtonsInfo[(i*OBI_LEN)+2];
		b = OtherButtonsInfo[(i*OBI_LEN)+3];
		if (mx >= l && mx <= r && my >= t && my <= b
			&& mx >= buttonDown[0] && mx <= buttonDown[2]
			&& my >= buttonDown[1] && my <= buttonDown[3])
		{ // Button has been released!
			ActivateOtherButton(i);
		}
	}
	if (statewhenstarted == KEYG_KEYBOARD)
	{
		int row,col;
		for (row = 0; row < numrows; row++)
		{
			if (my >= rowy+(row*rowspace) && my <= rowy+(row*rowspace)+keyheight
				&& my >= buttonDown[1] && my <= buttonDown[3])
			{
				for (col = 0; col < strlen(caseSet+(row*maxkeysinrow)); col++)
				{
					if (mx >= colx[row]+(col*colspace) && mx <= colx[row]+(col*colspace)+keywidth
						&& mx >= buttonDown[0] && mx <= buttonDown[2])
					{// This is the character that was released!
						std::cout << "Key: " << *(caseSet+((row*maxkeysinrow)+col)) << "\n";
						injectKeyEvent(*(caseSet+((row*maxkeysinrow)+col)));
					}
				}
			}
		}
	}
	buttonDown[0] = -1;
	buttonDown[1] = -1;
	buttonDown[2] = -1;
	buttonDown[3] = -1;
}

void Keyboard_gump::injectKeyEvent(char key, SDLKey sdlkey)
{
	SDL_Event event;
	event.type = SDL_KEYDOWN;
	event.key.type = SDL_KEYDOWN;
	event.key.state = SDL_PRESSED;
	event.key.keysym.mod = KMOD_NONE;

	if ( (altDown || ctrlDown) && isupper(key))
	{ // Its uppercase -- need to convert it to lower case so that ctrl and/or alt work
		key = tolower(key);
	}

	if (altDown)
	{
		event.key.keysym.mod = (SDLMod)(event.key.keysym.mod | KMOD_ALT);
		altDown = false;
	}
	if (ctrlDown)
	{
		event.key.keysym.mod = (SDLMod)(event.key.keysym.mod | KMOD_CTRL);
		ctrlDown = false;
	}

	if (key == '\0')
	{
		event.key.keysym.sym = sdlkey;
	}
	else
	{
		event.key.keysym.sym = (SDLKey)key;
		event.key.keysym.unicode = key;
	}

	SDL_PushEvent(&event);
}

#endif
