/*
 *  Copyright (C) 2001-2012  The Exult Team
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
#include "font.h"

using std::string;

static const int rowy[] = { 4, 16, 124, 28, 40, 52, 64, 76, 88, 100, 112, 148, 136 };

static const int colx[] = { 35, 50, 120, 195, 192 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

static int framerates[] = { 2, 4, 6, 8, 10, -1 };
// -1 is placeholder for custom framerate
static const int num_default_rates = sizeof(framerates)/sizeof(framerates[0]) - 1;


static string framestring(int fr)
{
	char buf[100];
	sprintf(buf, "%i fps", fr);
	return buf;
}

static const char* pathfind_texts[3] = {"no", "single", "double"};
static int num_pathfind_texts = 3;

class GameplayOptions_button : public Text_button {
public:
	GameplayOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{ }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
};

bool GameplayOptions_button::activate(int button)
{
	if (button != 1) return false;
	if (text == canceltext) {
		((GameplayOptions_gump*)parent)->cancel();
	} else if (text == oktext) {
		((GameplayOptions_gump*)parent)->close();
	}
	return true;
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
	if (btn == buttons[id_facestats])
		facestats = state;
	else if (btn == buttons[id_fastmouse])
		fastmouse = state;
#ifdef UNDER_CE
	else if (btn == buttons[id_dpadopt])
		dpadopt = state;
#else
	else if (btn == buttons[id_mouse3rd])
		mouse3rd = state;
#endif
	else if (btn == buttons[id_doubleclick])
		doubleclick = state;
	else if (btn == buttons[id_cheats])
		cheats = state;
	else if (btn == buttons[id_paperdolls])
		paperdolls = state;
	else if (btn == buttons[id_text_bg])
		text_bg = state;
	else if (btn == buttons[id_frames])
		frames = state;
	else if (btn == buttons[id_rightclick_close])
		rightclick_close = state;
	else if (btn == buttons[id_right_pathfind])
		right_pathfind = state;
	else if (btn == buttons[id_gumps_pause])
		gumps_pause = state;
	else if (btn == buttons[id_smooth_scrolling])
		smooth_scrolling = state;
}

void GameplayOptions_gump::build_buttons()
{
	std::string *stats = new std::string[4];
	stats[0] = "Disabled";
	stats[1] = "Left";
	stats[2] = "Middle";
	stats[3] = "Right";

	std::string *textbgcolor = new std::string[18];
	textbgcolor[0] = "Disabled";
	textbgcolor[1] = "Solid Light Gray";
	textbgcolor[2] = "Dark Purple";
	textbgcolor[3] = "Bright Yellow";
	textbgcolor[4] = "Light Blue";
	textbgcolor[5] = "Dark Green";
	textbgcolor[6] = "Dark Red";
	textbgcolor[7] = "Purple";
	textbgcolor[8] = "Orange";
	textbgcolor[9] = "Light Gray";
	textbgcolor[10] = "Green";
	textbgcolor[11] = "Yellow";
	textbgcolor[12] = "Pale Blue";
	textbgcolor[13] = "Dark Green";
	textbgcolor[14] = "Red";
	textbgcolor[15] = "Bright White";
	textbgcolor[16] = "Dark gray";
	textbgcolor[17] = "White";

#ifdef UNDER_CE
	std::string *dpadtext = new std::string[4];
	dpadtext[0] = "Portrait";
	dpadtext[1] = "Landscape1";
	dpadtext[2] = "Landscape2";
	dpadtext[3] = "Normal";
#endif

	std::string *smooth_text = new std::string[5];
	smooth_text[0] = "Disabled";
	smooth_text[1] = "25%";
	smooth_text[2] = "50%";
	smooth_text[3] = "75%";
	smooth_text[4] = "100%";
	
	std::string *pathfind_text = new std::string[3];
	pathfind_text[0] = "Disabled";
	pathfind_text[1] = "Single";
	pathfind_text[2] = "Double";

	buttons[id_facestats] = new GameplayTextToggle (this, stats, colx[3], rowy[0], 59,
										 facestats, 4);
	buttons[id_text_bg] = new GameplayTextToggle (this, textbgcolor, colx[3]-41, 
										 rowy[1], 100, text_bg, 18);
	if (sman->can_use_paperdolls() && (GAME_BG ||
			Game::get_game_type() == EXULT_DEVEL_GAME))
		buttons[id_paperdolls] = new GameplayEnabledToggle(this, colx[3], rowy[12], 59,
											   paperdolls);
	buttons[id_fastmouse] = new GameplayEnabledToggle(this, colx[3], rowy[3],
										   59, fastmouse);
#ifdef UNDER_CE
	buttons[id_dpadopt] = new GameplayTextToggle(this, dpadtext, colx[3]-21, rowy[4], 
										80, dpadopt, 4);
#else
	buttons[id_mouse3rd] = new GameplayEnabledToggle(this, colx[3], rowy[4],
										   59, mouse3rd);
#endif
	buttons[id_doubleclick] = new GameplayEnabledToggle(this, colx[3], rowy[5],
										   59, doubleclick);
	buttons[id_rightclick_close] = new GameplayEnabledToggle(this, colx[3], rowy[6],
										   59, rightclick_close);
	buttons[id_right_pathfind] = new GameplayTextToggle(this, pathfind_text, colx[3], rowy[7],
										   59, right_pathfind, num_pathfind_texts);
	buttons[id_gumps_pause] = new GameplayEnabledToggle(this, colx[3], rowy[8],
										   59, gumps_pause);
	buttons[id_cheats] = new GameplayEnabledToggle(this, colx[3], rowy[9],
										   59, cheats);
	buttons[id_frames] = new GameplayTextToggle(this, frametext, colx[3], rowy[10], 
										59, frames, num_framerates);
	buttons[id_smooth_scrolling] = new GameplayTextToggle(this, smooth_text, colx[3],
										rowy[2], 59, smooth_scrolling, 5);
}

void GameplayOptions_gump::load_settings()
{
	fastmouse = gwin->get_fastmouse(true);
#ifdef UNDER_CE
	config->value("config/gameplay/dpadopt", dpadopt, 0);
#else
	mouse3rd = gwin->get_mouse3rd();
#endif
	cheats = cheat();
	if (gwin->is_in_exult_menu()) {
		config->value("config/gameplay/facestats", facestats, -1);
		facestats += 1;
	}
	else
		facestats = Face_stats::get_state() + 1;
	doubleclick = 0;
	paperdolls = false;
	string pdolls;
	paperdolls = sman->are_paperdolls_enabled();
	doubleclick = gwin->get_double_click_closes_gumps();
	rightclick_close = gumpman->can_right_click_close();
	right_pathfind = gwin->get_allow_right_pathfind();
	text_bg = gwin->get_text_bg()+1;
	gumps_pause = !gumpman->gumps_dont_pause_game();
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
	smooth_scrolling = gwin->is_lerping_enabled()/25;
}

GameplayOptions_gump::GameplayOptions_gump() : Modal_gump(0, EXULT_FLX_GAMEPLAYOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0, 0, 0, 0), 8, 162);//++++++ ???

	for (int i = id_first; i < id_count; i++)
		buttons[i] = 0;

	load_settings();
	
	build_buttons();

	// Ok
	buttons[id_ok] = new GameplayOptions_button(this, oktext, colx[0], rowy[11]);
	// Cancel
	buttons[id_cancel] = new GameplayOptions_button(this, canceltext, 
											 colx[4], rowy[11]);
}

GameplayOptions_gump::~GameplayOptions_gump()
{
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			delete buttons[i];
}

void GameplayOptions_gump::save_settings()
{
	gwin->set_text_bg(text_bg-1);
	config->set("config/gameplay/textbackground", text_bg-1, false);
	int fps = framerates[frames];
	gwin->set_std_delay(1000/fps);
	config->set("config/video/fps", fps, false);
	gwin->set_fastmouse(fastmouse!=false);
	config->set("config/gameplay/fastmouse", fastmouse ? "yes" : "no", false);
#ifdef UNDER_CE
	config->set("config/gameplay/dpadopt", dpadopt, false);
	keybinder->WINCE_LoadFromDPADOPT(dpadopt);
#else
	gwin->set_mouse3rd(mouse3rd!=false);
	config->set("config/gameplay/mouse3rd", mouse3rd ? "yes" : "no", false);
#endif
	gwin->set_double_click_closes_gumps(doubleclick!=false);
	config->set("config/gameplay/double_click_closes_gumps", 
				doubleclick ? "yes" : "no", false);
	gumpman->set_right_click_close(rightclick_close!=false);
	config->set("config/gameplay/right_click_closes_gumps", 
				rightclick_close ? "yes" : "no" , false);
	cheat.set_enabled(cheats!=false);
	if (gwin->is_in_exult_menu())
		config->set("config/gameplay/facestats", facestats - 1 , false);
	else {
		while (facestats != Face_stats::get_state() + 1)
			Face_stats::AdvanceState();
		Face_stats::save_config(config);
	}
	if (sman->can_use_paperdolls() && (GAME_BG ||
			Game::get_game_type() == EXULT_DEVEL_GAME)) {
		sman->set_paperdoll_status(paperdolls!=false);
		config->set("config/gameplay/bg_paperdolls", 
				paperdolls ? "yes" : "no", false);
	}

	gwin->set_allow_right_pathfind(right_pathfind);
	config->set("config/gameplay/allow_right_pathfind", pathfind_texts[right_pathfind], false);

	gumpman->set_gumps_dont_pause_game(!gumps_pause);
	config->set("config/gameplay/gumps_dont_pause_game", gumps_pause?"no":"yes", false);

	if (smooth_scrolling < 0) smooth_scrolling = 0;
	else if (smooth_scrolling > 4) smooth_scrolling = 4;
	gwin->set_lerping_enabled(smooth_scrolling*25);
	config->set("config/gameplay/smooth_scrolling", smooth_scrolling*25,false);
	config->write_back();
}

void GameplayOptions_gump::paint()
{
	Gump::paint();
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			buttons[i]->paint();

	Font *font = fontManager.get_font("SMALL_BLACK_FONT");
	Image_window8 *iwin = gwin->get_win();

	font->paint_text(iwin->get_ib8(), "Status Bars:", x + colx[0], y + rowy[0] + 1);
	font->paint_text(iwin->get_ib8(), "Text Background:", x + colx[0], y + rowy[1] + 1);
	if (buttons[id_paperdolls])
		font->paint_text(iwin->get_ib8(), "Paperdolls:", x + colx[0], y + rowy[12] + 1);
	font->paint_text(iwin->get_ib8(), "Fullscreen Fast Mouse:", x + colx[0], y + rowy[3] + 1);
#ifdef UNDER_CE
	font->paint_text(iwin->get_ib8(), "D-Pad:", x + colx[0], y + rowy[4] + 1);
#else
	font->paint_text(iwin->get_ib8(), "Use Middle Mouse Button:", x + colx[0], y + rowy[4] + 1);
#endif
	font->paint_text(iwin->get_ib8(), "Doubleclick closes Gumps:", x + colx[0], y + rowy[5] + 1);
	font->paint_text(iwin->get_ib8(), "Right click closes Gumps:", x + colx[0], y + rowy[6] + 1);
	font->paint_text(iwin->get_ib8(), "Right click Pathfinds:", x + colx[0], y + rowy[7] + 1);
	font->paint_text(iwin->get_ib8(), "Gumps pause game:", x + colx[0], y + rowy[8] + 1);
	font->paint_text(iwin->get_ib8(), "Cheats:", x + colx[0], y + rowy[9] + 1);
	font->paint_text(iwin->get_ib8(), "Speed:", x + colx[0], y + rowy[10] + 1);
	font->paint_text(iwin->get_ib8(), "Smooth scrolling:", x + colx[0], y + rowy[2] + 1);
	gwin->set_painted();
}

bool GameplayOptions_gump::mouse_down(int mx, int my, int button)
{
	// Only left and right buttons
	if (button != 1 && button != 3) return false;

	// We'll eat the mouse down if we've already got a button down
	if (pushed) return true;

	// First try checkmark
	pushed = Gump::on_button(mx, my);
					
	// Try buttons at bottom.
	if (!pushed) {
		for (int i = id_first; i < id_count; i++) {
			if (buttons[i] && buttons[i]->on_button(mx, my)) {
				pushed = buttons[i];
				break;
			}
		}
	}

	if (pushed && !pushed->push(button))			// On a button?
		pushed = 0;
		
	return button == 1 || pushed != 0;
}

bool GameplayOptions_gump::mouse_up(int mx, int my, int button)
{
	// Not Pushing a button?
	if (!pushed) return false;

	if (pushed->get_pushed() != button) return button == 1;

	bool res = false;
	pushed->unpush(button);
	if (pushed->on_button(mx, my))
		res = ((Gump_button*)pushed)->activate(button);
	pushed = 0;
	return res;
}
