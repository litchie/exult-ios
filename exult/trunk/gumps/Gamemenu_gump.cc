/*
Copyright (C) 2001 The Exult Team

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

#include "SDL_events.h"

#include "Gamemenu_gump.h"
#include "Gump_button.h"
#include "Yesno_gump.h"
#include "gamewin.h"


int Gamemenu_gump::rowy[6] = { 14, 28, 40, 52, 66, 78 };
int Gamemenu_gump::colx = 138;

class Gamemenu_button : public Gump_button {
public:
	Gamemenu_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py, GSF_EXULT_FLX)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
};

void Gamemenu_button::activate(Game_window *gwin)
{
	switch (shapenum) {
	case 37: // load/save
		((Gamemenu_gump*)parent)->loadsave();
		break;
	case 38: // video options
		((Gamemenu_gump*)parent)->video_options();
		break;
	case 39: // audio options
		((Gamemenu_gump*)parent)->audio_options();
		break;
	case 40: // gameplay options
		((Gamemenu_gump*)parent)->gameplay_options();
		break;
	case 41: // quit to menu
		((Gamemenu_gump*)parent)->quit(true);
		break;
	case 42: // quit
		((Gamemenu_gump*)parent)->quit(false);
		break;
	}
}

Gamemenu_gump::Gamemenu_gump() : Modal_gump(0, 36, GSF_EXULT_FLX)
{
	for (int i=0; i<6; i++)
		buttons[i] = new Gamemenu_button(this, colx, rowy[i], 37+i);
}

Gamemenu_gump::~Gamemenu_gump()
{
	for (int i=0; i<6; i++)
		delete buttons[i];
	Game_window::get_game_window()->set_all_dirty();
}

//++++++ IMPLEMENT RETURN_TO_MENU!
void Gamemenu_gump::quit(bool return_to_menu)
{
	extern unsigned char quitting_time;
	if (!Yesno_gump::ask("Do you really want to quit?"))
		return;
	quitting_time = 1;
	done = 1;
}

//+++++ implement actual functionality and option screens
void Gamemenu_gump::loadsave()
{
}

void Gamemenu_gump::video_options()
{
}

void Gamemenu_gump::audio_options()
{
}

void Gamemenu_gump::gameplay_options()
{
}

void Gamemenu_gump::paint(Game_window* gwin)
{
	Gump::paint(gwin);
	for (int i=0; i<6; i++)
		paint_button(gwin, buttons[i]);
	gwin->set_painted();
}

void Gamemenu_gump::mouse_down(int mx, int my)
{
	Game_window *gwin = Game_window::get_game_window();
	pushed = 0;
					// First try checkmark.
	Gump_button *btn = Gump::on_button(gwin, mx, my);
	if (btn)
		pushed = btn;
	else				// Try buttons at bottom.
		for (int i=0; i<6; i++)
			if (buttons[i]->on_button(gwin, mx, my))
			{
				pushed = buttons[i];
				break;
			}
	if (pushed)			// On a button?
	{
		pushed->push(gwin);
		return;
	}
}

void Gamemenu_gump::mouse_up(int mx, int my)
{
	Game_window *gwin = Game_window::get_game_window();
	if (pushed)			// Pushing a button?
	{
		pushed->unpush(gwin);
		if (pushed->on_button(gwin, mx, my))
			((Gamemenu_button*)pushed)->activate(gwin);
		pushed = 0;
	}
}

