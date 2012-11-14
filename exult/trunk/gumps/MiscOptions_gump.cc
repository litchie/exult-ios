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
#include "MiscOptions_gump.h"
#include "exult.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "combat_opts.h"
#include "Text_button.h"
#include "Enabled_button.h"
#include "font.h"
#include "gamewin.h"
#include "Notebook_gump.h"
using std::string;

static const int rowy[] = { 4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148 };
static const int colx[] = { 35, 50, 120, 170, 192, 215 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

class MiscOptions_button : public Text_button {
public:
	MiscOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{ }
					// What to do when 'clicked':
	virtual bool activate(int button)
	{
		if (button != 1) return false;

		if (text == canceltext) {
			((MiscOptions_gump*)parent)->cancel();
		} else if (text == oktext) {
			((MiscOptions_gump*)parent)->close();
		}
		return true;
	}
};

class MiscTextToggle : public Gump_ToggleTextButton {
public:
	MiscTextToggle(Gump* par, std::string *s, int px, int py, int width,
					   int selectionnum, int numsel)
	: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width)
	{ }

	friend class MiscOptions_gump;
	virtual void toggle(int state) { 
		((MiscOptions_gump*)parent)->toggle((Gump_button*)this, 
									state);
	}
};

class MiscEnabledToggle : public Enabled_button {
public:
	MiscEnabledToggle(Gump* par, int px, int py, int width, 
						  int selectionnum)
		: Enabled_button(par, selectionnum, px, py, width)
	{ }

	friend class MiscOptions_gump;
	virtual void toggle(int state) {
		((MiscOptions_gump*)parent)->toggle((Gump_button*)this, 
									state);
	}
};	

void MiscOptions_gump::close()
{
	save_settings();
	done = 1;
}

void MiscOptions_gump::cancel()
{
	done = 1;
}

void MiscOptions_gump::toggle(Gump_button* btn, int state)
{
	if (btn == buttons[id_scroll_mouse])
		scroll_mouse = state;
	else if (btn == buttons[id_menu_intro])
		menu_intro = state;
	else if (btn == buttons[id_usecode_intro])
		usecode_intro = state;
	else if (btn == buttons[id_difficulty])
		difficulty = state;
	else if (btn == buttons[id_show_hits])
		show_hits = state;
	else if (btn == buttons[id_mode])
		mode = state;
	else if (btn == buttons[id_charmDiff])
		charmDiff = state;
	else if (btn == buttons[id_alternate_drop])
		alternate_drop = state;
	else if (btn == buttons[id_allow_autonotes])
		allow_autonotes = state;
}

void MiscOptions_gump::build_buttons()
{
	string *yesNo1 = new string[2]; // TODO:need to make this like enabled
	yesNo1[0] = "No";			   // if I am going to add much more
	yesNo1[1] = "Yes";
	string *yesNo2 = new string[2];
	yesNo2[0] = "No";
	yesNo2[1] = "Yes";
	string *yesNo3 = new string[2];
	yesNo3[0] = "No";
	yesNo3[1] = "Yes";
	std::string *diffs = new std::string[7];
	diffs[0] = "Easiest (-3)";
	diffs[1] = "Easier (-2)";
	diffs[2] = "Easier (-1)";
	diffs[3] = "Normal";
	diffs[4] = "Harder (+1)";
	diffs[5] = "Harder (+2)";
	diffs[6] = "Hardest (+3)";
	std::string *stacks_text = new std::string[2];
        stacks_text[0] = "No";
        stacks_text[1] = "Yes";
	std::string *autonotes_text = new std::string[2];
        autonotes_text[0] = "No";
	    autonotes_text[1] = "Yes";

	buttons[id_scroll_mouse] = new MiscTextToggle (this, yesNo1, colx[5], rowy[0], 
							   40, scroll_mouse, 2);
	buttons[id_menu_intro] = new MiscTextToggle (this, yesNo2, colx[5], rowy[1], 
							   40, menu_intro, 2);
	buttons[id_usecode_intro] = new MiscTextToggle (this, yesNo3, colx[5], rowy[2], 
							   40, usecode_intro, 2);
	buttons[id_alternate_drop] = new MiscTextToggle(this, stacks_text, colx[5], rowy[3], 
							   40, alternate_drop, 2);
	buttons[id_allow_autonotes] = new MiscTextToggle(this, autonotes_text, colx[5], rowy[4],
							   40, allow_autonotes, 2);
	buttons[id_difficulty] = new MiscTextToggle (this, diffs, colx[3], rowy[8], 
							   85, difficulty, 7);
	buttons[id_show_hits] = new MiscEnabledToggle(this, colx[3], rowy[9],
							   85, show_hits);

	std::string *modes = new std::string[2];
	modes[0] = "Original";
	modes[1] = "Space pauses";
	buttons[id_mode] = new MiscTextToggle (this, modes, colx[3], rowy[10],
							   85, mode, 2);
	std::string *charmedDiff = new std::string[2]; 
	charmedDiff[0] = "Normal";
	charmedDiff[1] = "Hard";
	buttons[id_charmDiff] = new MiscTextToggle (this, charmedDiff, colx[3], rowy[11],
							   85, charmDiff, 2);
	// Ok
	buttons[id_ok] = new MiscOptions_button(this, oktext, colx[0], rowy[12]);
	// Cancel
	buttons[id_cancel] = new MiscOptions_button(this, canceltext, colx[4], rowy[12]);
}

void MiscOptions_gump::load_settings()
{
	string yn;
	scroll_mouse = gwin->can_scroll_with_mouse();
	config->value("config/gameplay/skip_intro", yn, "no");
	usecode_intro = (yn == "yes");
	config->value("config/gameplay/skip_splash", yn, "no");
	menu_intro = (yn == "yes");
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
	charmDiff = Combat::charmed_more_difficult ? 1: 0;
	alternate_drop = gwin->get_alternate_drop();
	allow_autonotes = gwin->get_allow_autonotes();
}

MiscOptions_gump::MiscOptions_gump() 
	: Modal_gump(0, EXULT_FLX_GAMEPLAYOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0, 0, 0, 0), 8, 162);//++++++ ???

	load_settings();
	build_buttons();
}

MiscOptions_gump::~MiscOptions_gump()
{
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			delete buttons[i];
}

void MiscOptions_gump::save_settings()
{
	config->set("config/gameplay/scroll_with_mouse", 
			scroll_mouse ? "yes" : "no", false);
	gwin->set_mouse_with_scroll(scroll_mouse);
	config->set("config/gameplay/skip_intro", 
			usecode_intro ? "yes" : "no", false);
	config->set("config/gameplay/skip_splash", 
			menu_intro ? "yes" : "no", false);
	Combat::difficulty = difficulty - 3;
	config->set("config/gameplay/combat/difficulty",
						Combat::difficulty, false);
	Combat::show_hits = (show_hits != 0);
	config->set("config/gameplay/combat/show_hits", 
					show_hits ? "yes" : "no", false);
	Combat::mode = (Combat::Mode) mode;
	std::string str = Combat::mode == Combat::keypause ? "keypause"
					: "original";
	config->set("config/gameplay/combat/mode", str, false);
	Combat::charmed_more_difficult = (charmDiff != 0);
	config->set("config/gameplay/combat/charmDifficulty",
					charmDiff ? "hard" : "normal", false);
	gwin->set_alternate_drop(alternate_drop);
	config->set("config/gameplay/alternate_drop", 
						alternate_drop ? "yes" : "no", false);
	gwin->set_allow_autonotes(allow_autonotes);
	config->set("config/gameplay/allow_autonotes", 
						allow_autonotes ? "yes" : "no", false);

	config->write_back();
}

void MiscOptions_gump::paint()
{
	Gump::paint();
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			buttons[i]->paint();
	Font *font = fontManager.get_font("SMALL_BLACK_FONT");
	Image_window8 *iwin = gwin->get_win();
	font->paint_text(iwin->get_ib8(), "Scroll game view with mouse:", x + colx[0], y + rowy[0] + 1);
	font->paint_text(iwin->get_ib8(), "Skip intro:", x + colx[0], y + rowy[1] + 1);
	font->paint_text(iwin->get_ib8(), "Skip scripted first scene:", x + colx[0], y + rowy[2] + 1);
	font->paint_text(iwin->get_ib8(), "Alternate drag'n'drop:", x+colx[0], y+ rowy[3] + 1);
	font->paint_text(iwin->get_ib8(), "Allow Autonotes:", x+colx[0], y+ rowy[4] + 1);

	font->paint_text(iwin->get_ib8(), "Combat Options:", x + colx[0], y + rowy[7] + 1);
	font->paint_text(iwin->get_ib8(), "Difficulty:", x + colx[1], y + rowy[8] + 1);
	font->paint_text(iwin->get_ib8(), "Show Hits:", x + colx[1], y + rowy[9] + 1);
	font->paint_text(iwin->get_ib8(), "Mode:", x + colx[1], y + rowy[10] + 1);
	font->paint_text(iwin->get_ib8(), "Charmed Difficulty:", x + colx[1], y + rowy[11] + 1);
	gwin->set_painted();
}

bool MiscOptions_gump::mouse_down(int mx, int my, int button)
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
				pushed = (Gump_button *)buttons[i];
				break;
			}
		}
	}

	if (pushed && !pushed->push(button))			// On a button?
		pushed = 0;
		
	return button == 1 || pushed != 0;
}

bool MiscOptions_gump::mouse_up(int mx, int my, int button)
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
