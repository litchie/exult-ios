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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"

#include "Gamemenu_gump.h"
#include "AudioOptions_gump.h"
#include "VideoOptions_gump.h"
#include "GameplayOptions_gump.h"
#include "CombatOptions_gump.h"
#include "Gump_button.h"
#include "Yesno_gump.h"
#include "gamewin.h"
#include "Newfile_gump.h"
#include "File_gump.h"
#include "mouse.h"
#include "exult.h"
#include "exult_flx.h"
#include "Text_button.h"
#include "gameclk.h"
#include "Gump_manager.h"
#include <string>

using std::string;

static const int rowy[6] = { 4, 18, 30, 42, 56, 68 };
static const int colx = 31;

static const char* loadsavetext = "Load/Save Game";
static const char* videoopttext = "Video Options";
static const char* audioopttext = "Audio Options";
static const char* gameopttext = "Gameplay Options";
static const char *combattext = "Combat Options";
static const char* quitmenutext = "Quit to Menu";
static const char* quittext = "Quit";

class Gamemenu_button : public Text_button {
public:
	Gamemenu_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 108, 11)
	{  }
					// What to do when 'clicked':
	virtual void activate();
};

void Gamemenu_button::activate()
{
	if (text == loadsavetext) {
		((Gamemenu_gump*)parent)->loadsave();
	} else if (text == videoopttext) {
		((Gamemenu_gump*)parent)->video_options();
	} else if (text == audioopttext) {
		((Gamemenu_gump*)parent)->audio_options();
	} else if (text == gameopttext) {
		((Gamemenu_gump*)parent)->gameplay_options();
	} else if (text == combattext) {
		((Gamemenu_gump*)parent)->combat_options();
	} else if (text == quitmenutext) {
		((Gamemenu_gump*)parent)->quit(true);
	} else if (text == quittext) {
		((Gamemenu_gump*)parent)->quit(false);
	}
}

Gamemenu_gump::Gamemenu_gump() : Modal_gump(0, EXULT_FLX_GAMEMENU_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 82); //+++++ ???

	buttons[0] = new Gamemenu_button(this, loadsavetext, colx, rowy[0]);
	buttons[1] = new Gamemenu_button(this, videoopttext, colx, rowy[1]);
	buttons[2] = new Gamemenu_button(this, audioopttext, colx, rowy[2]);
	buttons[3] = new Gamemenu_button(this, gameopttext, colx, rowy[3]);
	buttons[4] = new Gamemenu_button(this, combattext, colx, rowy[4]);
	buttons[5] = new Gamemenu_button(this, quittext, colx, rowy[5]);
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
	gumpman->do_modal_gump(fileio, Mouse::hand);
	if (fileio->restored_game())
		done = true;
	delete fileio;
}

void Gamemenu_gump::video_options()
{
	VideoOptions_gump *vid_opts = new VideoOptions_gump();
	gumpman->do_modal_gump(vid_opts, Mouse::hand);

		// resolution could have changed, so recenter & repaint menu.
	set_pos();
	gwin->paint();
	gwin->show();
	delete vid_opts;

	gclock->set_palette();
}

void Gamemenu_gump::audio_options()
{
	AudioOptions_gump *aud_opts = new AudioOptions_gump();
	gumpman->do_modal_gump(aud_opts, Mouse::hand);
	delete aud_opts;
}

void Gamemenu_gump::gameplay_options()
{
	GameplayOptions_gump *gp_opts = new GameplayOptions_gump();
	gumpman->do_modal_gump(gp_opts, Mouse::hand);
	delete gp_opts;
}

void Gamemenu_gump::combat_options()
{
	CombatOptions_gump *cbt_opts = new CombatOptions_gump();
	gumpman->do_modal_gump(cbt_opts, Mouse::hand);
	delete cbt_opts;
}

void Gamemenu_gump::paint()
{
	Gump::paint();
	for (int i=0; i<6; i++)
		if (buttons[i])
			buttons[i]->paint();
	gwin->set_painted();
}

void Gamemenu_gump::mouse_down(int mx, int my)
{
	pushed = Gump::on_button(mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i=0; i<6; i++)
			if (buttons[i] && buttons[i]->on_button(mx, my)) {
				pushed = buttons[i];
				break;
			}

	if (pushed)			// On a button?
	{
		pushed->push();
		return;
	}
}

void Gamemenu_gump::mouse_up(int mx, int my)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			((Gamemenu_button*)pushed)->activate();
		pushed = 0;
	}
}

