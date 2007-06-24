/*
Copyright (C) 2001-2002 The Exult Team

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
#include "exult.h"
#include "exult_pocketpc_flx.h"
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
static const char Ucharrows[numrows][maxkeysinrow] = { "~!@#$%^&*()_+", "QWERTYUIOP{}|", "ASDFGHJKL:\"", "ZXCVBNM<>?" };
static const char Lcharrows[numrows][maxkeysinrow] = { "`1234567890-=", "qwertyuiop[]\\", "asdfghjkl;'", "zxcvbnm,./" };

static const enum OtherButtons {	KEYG_STARTOFSHOWN,
									KEYG_CASECHANGE,
									KEYG_ENTER,
									KEYG_ALT,
									KEYG_CTRL,
									KEYG_SPACE,
									KEYG_BACKSPACE,
									KEYG_LARROW,
									KEYG_RARROW,
									KEYG_DONE,
									KEYG_SHOWN_MOVEUPPERLEFT,
									KEYG_SHOWN_MOVEUPPERRIGHT,
									KEYG_SHOWN_MOVELOWERLEFT,
									KEYG_SHOWN_MOVELOWERRIGHT,
									KEYG_ENDOFSHOWN,
									KEYG_STARTOFMINIMIZED,
									KEYG_SHOW,
									KEYG_MIN_MOVEUPPERLEFT,
									KEYG_MIN_MOVEUPPERRIGHT,
									KEYG_MIN_MOVELOWERLEFT,
									KEYG_MIN_MOVELOWERRIGHT,
									KEYG_ENDOFMINIMIZED,
									NUMOTHERBUTTONS,
								};
static const int OBI_LEN = 4;
static const int OtherButtonsInfo[NUMOTHERBUTTONS * OBI_LEN] = {
						 -1,  -1,  -1,  -1, // KEYG_STARTOFSHOWN
						  3,  64,  41,  76, // KEYG_CASECHANGE
					    160,  49, 174,  76, // KEYG_ENTER
						108,  64, 127,  76, // KEYG_ALT
						 44,  64,  69,  76, // KEYG_CTRL
						 72,  64, 105,  76, // KEYG_SPACE
						183,  34, 202,  46, // KEYG_BACKSPACE
						130,  64, 142,  76, // KEYG_LARROW
						145,  64, 157,  76, // KEYG_RARROW
						184,  61, 198,  69, // KEYG_DONE
 						179,  53, 190,  60, // KEYG_SHOWN_MOVEUPPERLEFT
						191,  53, 202,  60, // KEYG_SHOWN_MOVEUPPERRIGHT
						179,  70, 190,  77, // KEYG_SHOWN_MOVELOWERLEFT
						191,  70, 202,  77, // KEYG_SHOWN_MOVELOWERRIGHT
					 	 -1,  -1,  -1,  -1, // KEYG_ENDOFSHOWN
					 	 -1,  -1,  -1,  -1, // KEYG_STARTOFMINIMIZED
  						  5,   8,  19,  16, // KEYG_SHOW
						  0,   0,  11,   7, // KEYG_MIN_MOVEUPPERLEFT
						 12,   0,  24,   7, // KEYG_MIN_MOVEUPPERRIGHT
						  0,  17,  11,  24, // KEYG_MIN_MOVELOWERLEFT
						 12,  17,  24,  24, // KEYG_MIN_MOVELOWERRIGHT
						 -1,  -1,  -1,  -1, // KEYG_ENDOFMINIMIZED
};

Keyboard_gump::Keyboard_gump(int placex, int placey, bool upperCase)
{
	autopaint = true;
	buttonDown[0] = -1;
	buttonDown[1] = -1;
	buttonDown[2] = -1;
	buttonDown[3] = -1;
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
	lastlocx = locx;
	lastlocy = locy;
	hide();
}

void Keyboard_gump::show(int placex, int placey)
{
	if (state == KEYG_SHOWN)
		return;
	Game_window *gwin = Game_window::get_instance();
	if (state == KEYG_MINIMIZED)
		gwin->add_dirty(RECTX(locx, locy, width+4, height+4));

	width = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, 0)->get_width();
	height = pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_KEYBOARD_SHP, 0)->get_height();
	locx = placex;
	locy = placey;
	if (locx == -1)
		locx = lastlocx;
	if (locy == -1)
		locy = lastlocy;
	state = KEYG_SHOWN;
	gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
	if (autopaint)
		gwin->paint_dirty();
}

void Keyboard_gump::hide()
{
	if (state == KEYG_SHOWN)
	{
		lastlocx = locx;
		lastlocy = locy;
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
	if (state == KEYG_SHOWN)
		gwin->add_dirty(RECTX(locx, locy, width+4, height+4));
	
	lastlocx = locx;
	lastlocy = locy;
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
	if (state == KEYG_SHOWN)
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
	else if (state == KEYG_MINIMIZED)
	{
		Shape_manager::get_instance()->paint_shape(locx, locy,
					pocketpc_vga.get_shape(EXULT_POCKETPC_FLX_MINIMIZEDKBD_SHP, 0), 0);
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

void Keyboard_gump::ActivateOtherButton(int button)
{
	Game_window *gwin = Game_window::get_instance();
	if (button == KEYG_SHOW)
	{
		show();
	}
	else if (button == KEYG_DONE)
	{
		minimize();
	}
	else if (button == KEYG_CASECHANGE)
	{
		if (caseSet == &Ucharrows[0][0])
			caseSet = &Lcharrows[0][0];
		else
			caseSet = &Ucharrows[0][0];
		//paint();
		//gwin->show();
		//gwin->paint();
	}
	else if (button == KEYG_ENTER)
	{
		injectKeyEvent('\0', SDLK_RETURN);
	}
	else if (button == KEYG_BACKSPACE)
	{
		injectKeyEvent('\0', SDLK_BACKSPACE);
	}
	else if (button == KEYG_LARROW)
	{
		injectKeyEvent('\0', SDLK_LEFT);
	}
	else if (button == KEYG_RARROW)
	{
		injectKeyEvent('\0', SDLK_RIGHT);
	}
	else if (button == KEYG_SPACE)
	{
		injectKeyEvent(' ');
	}
	else if (button == KEYG_ALT)
	{
		altDown = !altDown;
	}
	else if (button == KEYG_CTRL)
	{
		ctrlDown = !ctrlDown;
	}
	else if (button >= KEYG_SHOWN_MOVEUPPERLEFT && button <= KEYG_SHOWN_MOVELOWERRIGHT)
	{
		moveToCorner(button - KEYG_SHOWN_MOVEUPPERLEFT);
	}
	else if (button >= KEYG_MIN_MOVEUPPERLEFT && button <= KEYG_MIN_MOVELOWERRIGHT)
	{
		show();
		moveToCorner(button - KEYG_MIN_MOVEUPPERLEFT);
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
	start = KEYG_STARTOFSHOWN+1;
	end = KEYG_ENDOFSHOWN-1;
	if (state == KEYG_MINIMIZED)
	{
		start = KEYG_STARTOFMINIMIZED+1;
		end = KEYG_ENDOFMINIMIZED+1;
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
	if (statewhenstarted == KEYG_SHOWN)
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
	start = KEYG_STARTOFSHOWN+1;
	end = KEYG_ENDOFSHOWN-1;
	if (state == KEYG_MINIMIZED)
	{
		start = KEYG_STARTOFMINIMIZED+1;
		end = KEYG_ENDOFMINIMIZED+1;
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
	if (statewhenstarted == KEYG_SHOWN)
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
	event.key.keysym.mod = SDL_GetModState();
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
		event.key.keysym.sym = sdlkey;
	else
		event.key.keysym.unicode = key;
	SDL_PushEvent(&event);
}

#endif