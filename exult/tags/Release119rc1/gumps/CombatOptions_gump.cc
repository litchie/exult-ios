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
#include "CombatOptions_gump.h"
#include "exult.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "combat_opts.h"
#include "Text_button.h"
#include "Enabled_button.h"

using std::cerr;
using std::endl;
using std::string;

static const int rowy[] = { 17, 43, 69, 130 };
static const int colx[] = { 35, 50, 120, 170, 192 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

static int framerates[] = { 2, 4, 6, 8, 10, -1 };
 // -1 is placeholder for custom framerate
static int num_default_rates = sizeof(framerates)/sizeof(framerates[0]) - 1;


class CombatOptions_button : public Text_button {
public:
	CombatOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{ }
					// What to do when 'clicked':
	virtual void activate();
};

void CombatOptions_button::activate()
{
	if (text == canceltext) {
		((CombatOptions_gump*)parent)->cancel();
	} else if (text == oktext) {
		((CombatOptions_gump*)parent)->close();
	}
}

class CombatTextToggle : public Gump_ToggleTextButton {
public:
	CombatTextToggle(Gump* par, std::string *s, int px, int py, int width,
					   int selectionnum, int numsel)
	: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width)
	{ }

	friend class CombatOptions_gump;
	virtual void toggle(int state) { 
		((CombatOptions_gump*)parent)->toggle((Gump_button*)this, 
									state);
	}
};

class CombatEnabledToggle : public Enabled_button {
public:
	CombatEnabledToggle(Gump* par, int px, int py, int width, 
						  int selectionnum)
		: Enabled_button(par, selectionnum, px, py, width)
	{ }

	friend class CombatOptions_gump;
	virtual void toggle(int state) {
		((CombatOptions_gump*)parent)->toggle((Gump_button*)this, 
									state);
	}
};	

void CombatOptions_gump::close()
{
	save_settings();
	done = 1;
}

void CombatOptions_gump::cancel()
{
	done = 1;
}

void CombatOptions_gump::toggle(Gump_button* btn, int state)
{
	if (btn == buttons[0])
		difficulty = state;
	else if (btn == buttons[1])
		show_hits = state;
	else if (btn == buttons[2])
		mode = state;
}

void CombatOptions_gump::build_buttons()
{
	std::string *diffs = new std::string[7];
	diffs[0] = "Easiest (-3)";
	diffs[1] = "Easier (-2)";
	diffs[2] = "Easier (-1)";
	diffs[3] = "Normal";
	diffs[4] = "Harder (+1)";
	diffs[5] = "Harder (+2)";
	diffs[6] = "Hardest (+3)";
	buttons[0] = new CombatTextToggle (this, diffs, colx[3], rowy[0], 
									   85, difficulty, 7);
	buttons[1] = new CombatEnabledToggle(this, colx[3], rowy[1],
										 85, show_hits);
	std::string *modes = new std::string[2];
	modes[0] = "Original";
	modes[1] = "Space pauses";
	buttons[2] = new CombatTextToggle (this, modes, colx[3], rowy[2],
									   85, mode, 2);
}

void CombatOptions_gump::load_settings()
{
	difficulty = Combat::difficulty;
	if (difficulty < -3)
		difficulty = -3;
	else if (difficulty > 3)
		difficulty = 3;
	difficulty += 3;		// Scale to choices (0-6).
	show_hits = Combat::show_hits ? 1 : 0;
	mode = (int) Combat::mode;
	if (mode < 0 || mode > 1)
		mode = 0;
}

CombatOptions_gump::CombatOptions_gump() 
	: Modal_gump(0, EXULT_FLX_GAMEPLAYOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0, 0, 0, 0), 8, 162);//++++++ ???
	const int nbuttons = sizeof(buttons)/sizeof(buttons[0]);
	for (int i = 0; i < nbuttons; i++)
		buttons[i] = 0;

	load_settings();
	
	build_buttons();

	// Ok
	buttons[nbuttons - 2] = 
		new CombatOptions_button(this, oktext, colx[0], rowy[3]);
	// Cancel
	buttons[nbuttons - 1] = 
		new CombatOptions_button(this, canceltext, colx[4], rowy[3]);
}

CombatOptions_gump::~CombatOptions_gump()
{
	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i])
			delete buttons[i];
}

void CombatOptions_gump::save_settings()
{
	Combat::difficulty = difficulty - 3;
	config->set("config/gameplay/combat/difficulty",
						Combat::difficulty, true);
	Combat::show_hits = (show_hits != 0);
	config->set("config/gameplay/combat/show_hits", 
					show_hits ? "yes" : "no", true);
	Combat::mode = (Combat::Mode) mode;
	std::string str = Combat::mode == Combat::keypause ? "keypause"
					: "original";
	config->set("config/gameplay/combat/mode", str, true);
}

void CombatOptions_gump::paint()
{
	Gump::paint();
	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		if (buttons[i])
			buttons[i]->paint();

	sman->paint_text(2, "Difficulty:", x + colx[0], y + rowy[0] + 1);
	sman->paint_text(2, "Show Hits:", x + colx[0], y + rowy[1] + 1);
	sman->paint_text(2, "Mode:", x + colx[0], y + rowy[2] + 1);
	gwin->set_painted();
}

void CombatOptions_gump::mouse_down(int mx, int my)
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

void CombatOptions_gump::mouse_up(int mx, int my)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			((Gump_button*)pushed)->activate();
		pushed = 0;
	}
}
