/*
 *  Copyright (C) 2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>
#include <cstring>

#include "SDL_events.h"

#include "Gump_manager.h"
#include "Configuration.h"
#include "Gump_button.h"
#include "Gump_ToggleButton.h"
#include "GameplayOptions_gump.h"
#include "exult.h"
#include "exult_flx.h"
#include "game.h"
#include "gamewin.h"
#include "mouse.h"
#include "cheat.h"
#include "Face_stats.h"
#include "Text_button.h"
#include "Enabled_button.h"

using std::cerr;
using std::endl;
using std::string;

static const int rowy[] = { 5, 18, 31, 44, 57, 70, 83, 96, 109, 122, 146 };
static const int colx[] = { 35, 50, 120, 195, 192 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

static int framerates[] = { 2, 4, 6, 8, 10, -1 };
 // -1 is placeholder for custom framerate
static int num_default_rates = sizeof(framerates)/sizeof(framerates[0]) - 1;


static string framestring(int fr)
{
	char buf[100];
	sprintf(buf, "%i fps", fr);
	return buf;
}

class GameplayOptions_button : public Text_button {
public:
	GameplayOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{ }
					// What to do when 'clicked':
	virtual void activate();
};

void GameplayOptions_button::activate()
{
	if (text == canceltext) {
		((GameplayOptions_gump*)parent)->cancel();
	} else if (text == oktext) {
		((GameplayOptions_gump*)parent)->close();
	}
}

class GameplayTextToggle : public Gump_ToggleTextButton {
public:
	GameplayTextToggle(Gump* par, std::string *s, int px, int py, int width,
					   int selectionnum, int numsel)
		: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width)
	{ }

	friend class GameplayOptions_gump;
	virtual void toggle(int state) { 
		((GameplayOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};

class GameplayEnabledToggle : public Enabled_button {
public:
	GameplayEnabledToggle(Gump* par, int px, int py, int width, 
						  int selectionnum)
		: Enabled_button(par, selectionnum, px, py, width)
	{ }

	friend class GameplayOptions_gump;
	virtual void toggle(int state) {
		((GameplayOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};	

void GameplayOptions_gump::close()
{
	save_settings();
	done = 1;
}

void GameplayOptions_gump::cancel()
{
	done = 1;
}

void GameplayOptions_gump::toggle(Gump_button* btn, int state)
{
	if (btn == buttons[0])
		facestats = state;
	else if (btn == buttons[1])
		fastmouse = state;
	else if (btn == buttons[2])
		mouse3rd = state;
	else if (btn == buttons[3])
		doubleclick = state;
	else if (btn == buttons[4])
		cheats = state;
	else if (btn == buttons[5])
		paperdolls = state;
	else if (btn == buttons[6])
		text_bg = state;
#if 0	/* ++++No longer needed */
	else if (btn == buttons[7])
		walk_after_teleport = state;
#endif
	else if (btn == buttons[8])
		frames = state;
	else if (btn == buttons[11])
		rightclick_close = state;
}

void GameplayOptions_gump::build_buttons()
{
#if 0
	std::string *enabledtext1 = new std::string[2];
	enabledtext1[0] = "Disabled";
	enabledtext1[1] = "Enabled";

	std::string *enabledtext2 = new std::string[2];
	enabledtext2[0] = "Disabled";
	enabledtext2[1] = "Enabled";

	std::string *enabledtext3 = new std::string[2];
	enabledtext3[0] = "Disabled";
	enabledtext3[1] = "Enabled";

	std::string *enabledtext4 = new std::string[2];
	enabledtext4[0] = "Disabled";
	enabledtext4[1] = "Enabled";

	std::string *enabledtext5 = new std::string[2];
	enabledtext5[0] = "Disabled";
	enabledtext5[1] = "Enabled";
#endif

	std::string *stats = new std::string[4];
	stats[0] = "Disabled";
	stats[1] = "Left";
	stats[2] = "Middle";
	stats[3] = "Right";

	std::string *textbgcolor = new std::string[12];
	textbgcolor[0] = "Disabled";
	textbgcolor[1] = "Purple";
	textbgcolor[2] = "Orange";
	textbgcolor[3] = "Light Gray";
	textbgcolor[4] = "Green";
	textbgcolor[5] = "Yellow";
	textbgcolor[6] = "Pale Blue";
	textbgcolor[7] = "Dark Green";
	textbgcolor[8] = "Red";
	textbgcolor[9] = "Bright White";
	textbgcolor[10] = "Dark gray";
	textbgcolor[11] = "White";

	buttons[0] = new GameplayTextToggle (this, stats, colx[3], rowy[0], 59,
										 facestats, 4);
	buttons[6] = new GameplayTextToggle (this, textbgcolor, colx[3]-21, 
										 rowy[1], 80, text_bg, 12);
	if (GAME_BG)
		buttons[5] = new GameplayEnabledToggle(this, colx[3], rowy[2], 59,
											   paperdolls);
#if 0	/* ++++++Option no longer needed. */
	else if (GAME_SI)
		buttons[7] = new GameplayEnabledToggle(this, colx[3], rowy[2], 59, 
											   walk_after_teleport);
#endif
	buttons[1] = new GameplayEnabledToggle(this, colx[3], rowy[3],
										   59, fastmouse);
	buttons[2] = new GameplayEnabledToggle(this, colx[3], rowy[4],
										   59, mouse3rd);
	buttons[3] = new GameplayEnabledToggle(this, colx[3], rowy[5],
										   59, doubleclick);
	buttons[11] = new GameplayEnabledToggle(this, colx[3], rowy[6],
										   59, rightclick_close);
	buttons[4] = new GameplayEnabledToggle(this, colx[3], rowy[7],
										   59, cheats);
	buttons[8] = new GameplayTextToggle(this, frametext, colx[3], rowy[8], 
										59, frames, num_framerates);
}

void GameplayOptions_gump::load_settings()
{
	fastmouse = gwin->get_fastmouse();
	mouse3rd = gwin->get_mouse3rd();
	cheats = cheat();
	facestats = Face_stats::get_state() + 1;
	doubleclick = 0;
	paperdolls = false;
	string pdolls;
	paperdolls = sman->get_bg_paperdolls();
	doubleclick = gwin->get_double_click_closes_gumps();
	rightclick_close = gumpman->can_right_click_close();
	text_bg = gwin->get_text_bg()+1;
	int realframes = 1000/gwin->get_std_delay();
	int i;

    frames = -1;
	framerates[num_default_rates] = realframes;
	for (i=0; i < num_default_rates; i++) {
		if (realframes == framerates[i]) {
			frames = i;
			break;
		}
	}

	num_framerates = num_default_rates;
	if (frames == -1) {
		num_framerates++;
		frames = num_default_rates;
	}
	frametext = new string[num_framerates];
	for (i=0; i < num_framerates; i++) {
		frametext[i] = framestring(framerates[i]);
	}
}

GameplayOptions_gump::GameplayOptions_gump() : Modal_gump(0, EXULT_FLX_GAMEPLAYOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0, 0, 0, 0), 8, 162);//++++++ ???

	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		buttons[i] = 0;

	load_settings();
	
	build_buttons();

	// Ok
	buttons[9] = new GameplayOptions_button(this, oktext, colx[0], rowy[10]);
	// Cancel
	buttons[10] = new GameplayOptions_button(this, canceltext, 
											 colx[4], rowy[10]);
}

GameplayOptions_gump::~GameplayOptions_gump()
{
	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i])
			delete buttons[i];
}

void GameplayOptions_gump::save_settings()
{
	gwin->set_text_bg(text_bg-1);
	config->set("config/gameplay/textbackground", text_bg-1, true);
	int fps = framerates[frames];
	gwin->set_std_delay(1000/fps);
	config->set("config/video/fps", fps, true);
	gwin->set_fastmouse(fastmouse!=false);
	config->set("config/gameplay/fastmouse", fastmouse ? "yes" : "no", true);
	gwin->set_mouse3rd(mouse3rd!=false);
	config->set("config/gameplay/mouse3rd", mouse3rd ? "yes" : "no", true);
	gwin->set_double_click_closes_gumps(doubleclick!=false);
	config->set("config/gameplay/double_click_closes_gumps", 
				doubleclick ? "yes" : "no", true);
	gumpman->set_right_click_close(rightclick_close!=false);
	config->set("config/gameplay/right_click_closes_gumps", 
				rightclick_close ? "yes" : "no" , true);
	cheat.set_enabled(cheats!=false);
	while (facestats != Face_stats::get_state() + 1)
		Face_stats::AdvanceState();
	Face_stats::save_config(config);
	if (GAME_BG && sman->can_use_paperdolls())
		sman->set_bg_paperdolls(paperdolls!=false);
	config->set("config/gameplay/bg_paperdolls", 
				paperdolls ? "yes" : "no", true);
}

void GameplayOptions_gump::paint()
{
	Gump::paint();
	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i])
			buttons[i]->paint();

	sman->paint_text(2, "Status Bars:", x + colx[0], y + rowy[0] + 1);
	sman->paint_text(2, "Text Background:", x + colx[0], y + rowy[1] + 1);
	if (GAME_BG)
		sman->paint_text(2, "Paperdolls:", x + colx[0], y + rowy[2] + 1);
	else if (GAME_SI)
		sman->paint_text(2, "Walk after Teleport:", x + colx[0], y + rowy[2] + 1);
	sman->paint_text(2, "Fast Mouse:", x + colx[0], y + rowy[3] + 1);
	sman->paint_text(2, "Use Middle Mouse Button:", x + colx[0], y + rowy[4] + 1);
	sman->paint_text(2, "Doubleclick closes Gumps:", x + colx[0], y + rowy[5] + 1);
	sman->paint_text(2, "Right click closes Gumps:", x + colx[0], y + rowy[6] + 1);
	sman->paint_text(2, "Cheats:", x + colx[0], y + rowy[7] + 1);
	sman->paint_text(2, "Speed:", x + colx[0], y + rowy[8] + 1);
	gwin->set_painted();
}

void GameplayOptions_gump::mouse_down(int mx, int my)
{
	pushed = Gump::on_button(mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
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

void GameplayOptions_gump::mouse_up(int mx, int my)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			((Gump_button*)pushed)->activate();
		pushed = 0;
	}
}
