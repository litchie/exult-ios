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
#include "AudioOptions_gump.h"
#include "VideoOptions_gump.h"
#include "Gump_button.h"
#include "Yesno_gump.h"
#include "gamewin.h"
#include "Newfile_gump.h"
#include "File_gump.h"
#include "mouse.h"
#include "gump_utils.h"
#include "exult_flx.h"

static const int rowy[6] = { 14, 28, 40, 52, 66, 78 };
static const int colx = 138;

class Gamemenu_button : public Gump_button {
public:
	Gamemenu_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py, SF_EXULT_FLX)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
};

void Gamemenu_button::activate(Game_window *gwin)
{
	switch (get_shapenum()) {
	case EXULT_FLX_GAM_LOADSAVE_SHP: // load/save
		((Gamemenu_gump*)parent)->loadsave();
		break;
	case EXULT_FLX_GAM_VIDEO_SHP: // video options
		((Gamemenu_gump*)parent)->video_options();
		break;
	case EXULT_FLX_GAM_AUDIO_SHP: // audio options
		((Gamemenu_gump*)parent)->audio_options();
		break;
	case EXULT_FLX_GAM_GAMEPLAY_SHP: // gameplay options
		((Gamemenu_gump*)parent)->gameplay_options();
		break;
	case EXULT_FLX_GAM_QUITMENU_SHP: // quit to menu
		((Gamemenu_gump*)parent)->quit(true);
		break;
	case EXULT_FLX_GAM_QUIT_SHP: // quit
		((Gamemenu_gump*)parent)->quit(false);
		break;
	}
}

Gamemenu_gump::Gamemenu_gump() : Modal_gump(0, EXULT_FLX_GAMEMENU_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 82); //+++++ ???

	buttons[0] = new Gamemenu_button(this, colx, rowy[0], EXULT_FLX_GAM_LOADSAVE_SHP);
	buttons[1] = new Gamemenu_button(this, colx, rowy[1], EXULT_FLX_GAM_VIDEO_SHP);
	buttons[2] = new Gamemenu_button(this, colx, rowy[2], EXULT_FLX_GAM_AUDIO_SHP);
	buttons[3] = new Gamemenu_button(this, colx, rowy[3], EXULT_FLX_GAM_GAMEPLAY_SHP);
	buttons[4] = new Gamemenu_button(this, colx, rowy[4], EXULT_FLX_GAM_QUITMENU_SHP);
	buttons[5] = new Gamemenu_button(this, colx, rowy[5], EXULT_FLX_GAM_QUIT_SHP);
}

Gamemenu_gump::~Gamemenu_gump()
{
	for (int i=0; i<6; i++)
		delete buttons[i];
}

//++++++ IMPLEMENT RETURN_TO_MENU!
void Gamemenu_gump::quit(bool return_to_menu)
{
	if (!Yesno_gump::ask("Do you really want to quit?"))
		return;
	quitting_time = QUIT_TIME_YES;
	done = 1;
}

//+++++ implement actual functionality and option screens
void Gamemenu_gump::loadsave()
{
	//File_gump *fileio = new File_gump();
	Newfile_gump *fileio = new Newfile_gump();
	Do_Modal_gump(fileio, Mouse::hand);
	if (fileio->restored_game()) done = 1;
	delete fileio;
}

void Gamemenu_gump::video_options()
{
	VideoOptions_gump *vid_opts = new VideoOptions_gump();
	Do_Modal_gump(vid_opts, Mouse::hand);
	delete vid_opts;
	Game_window::get_game_window()->set_palette();
}

void Gamemenu_gump::audio_options()
{
	AudioOptions_gump *aud_opts = new AudioOptions_gump();
	Do_Modal_gump(aud_opts, Mouse::hand);
	delete aud_opts;
}

void Gamemenu_gump::gameplay_options()
{
}

void Gamemenu_gump::paint(Game_window* gwin)
{
	Gump::paint(gwin);
	for (int i=0; i<6; i++)
		buttons[i]->paint(gwin);
	gwin->set_painted();
}

void Gamemenu_gump::mouse_down(int mx, int my)
{
	Game_window *gwin = Game_window::get_game_window();
	pushed = Gump::on_button(gwin, mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i=0; i<6; i++)
			if (buttons[i]->on_button(gwin, mx, my)) {
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

