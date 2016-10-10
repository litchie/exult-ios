/*
 *  Copyright (C) 2001-2015  The Exult Team
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

#ifdef __IPHONEOS__

#include <iostream>
#include <cstring>

#include "SDL_events.h"

#include "Gump_manager.h"
#include "Configuration.h"
#include "Gump_button.h"
#include "Gump_ToggleButton.h"
#include "iphoneOptions_gump.h"
#include "exult.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "Text_button.h"
#include "Enabled_button.h"
#include "font.h"
#include "gamewin.h"

using std::string;

static const int rowy[] = { 4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148 };
static const int colx[] = { 35, 50, 120, 170, 192, 215 };

static const char *oktext = "OK";
static const char *canceltext = "CANCEL";

static const char *dpad_texts[3] = {"no", "left", "right"};
static int num_dpad_texts = 3;

class iphoneOptions_button : public Text_button {
public:
	iphoneOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
	{ }
	// What to do when 'clicked':
	virtual bool activate(int button) {
		if (button != 1) return false;

		if (text == canceltext) {
			reinterpret_cast<iphoneOptions_gump *>(parent)->cancel();
		} else if (text == oktext) {
			reinterpret_cast<iphoneOptions_gump *>(parent)->close();
		}
		return true;
	}
};

class iphoneTextToggle : public Gump_ToggleTextButton {
public:
	iphoneTextToggle(Gump *par, std::string *s, int px, int py, int width,
	               int selectionnum, int numsel)
		: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width)
	{ }

	friend class iphoneOptions_gump;
	virtual void toggle(int state) {
		reinterpret_cast<iphoneOptions_gump *>(parent)->toggle(this, state);
	}
};

class iphoneEnabledToggle : public Enabled_button {
public:
	iphoneEnabledToggle(Gump *par, int px, int py, int width,
	                  int selectionnum)
		: Enabled_button(par, selectionnum, px, py, width)
	{ }

	friend class iphoneOptions_gump;
	virtual void toggle(int state) {
		reinterpret_cast<iphoneOptions_gump *>(parent)->toggle(this, state);
	}
};

void iphoneOptions_gump::close() {
	save_settings();
	done = 1;
}

void iphoneOptions_gump::cancel() {
	done = 1;
}

void iphoneOptions_gump::toggle(Gump_button *btn, int state) {
	if (btn == buttons[id_item_menu])
		item_menu = state;
	else if (btn == buttons[id_dpad_location])
		dpad_location = state;
	else if (btn == buttons[id_touch_pathfind])
		touch_pathfind = state;
}

void iphoneOptions_gump::build_buttons() {
	string *yesNo1 = new string[2]; // TODO:need to make this like enabled
	yesNo1[0] = "No";              // if I am going to add much more
	yesNo1[1] = "Yes";
	
	string *yesNo2 = new string[2];
	yesNo2[0] = "No";
	yesNo2[1] = "Yes";
	
	string *yesNo3 = new string[2];
	yesNo3[0] = "No";
	yesNo3[1] = "Yes";
	
	string *dpad_text = new string[3];
	dpad_text[0] = "Disabled";
	dpad_text[1] = "Left";
	dpad_text[2] = "Right";

	buttons[id_item_menu] = new iphoneEnabledToggle(this, colx[4], rowy[0],
	        59, item_menu);

	buttons[id_dpad_location] = new iphoneTextToggle(this, dpad_text, colx[4], rowy[1],
		59, dpad_location, num_dpad_texts);

	buttons[id_touch_pathfind] = new iphoneEnabledToggle(this, colx[4], rowy[2],
		59, touch_pathfind);
	// Ok
	buttons[id_ok] = new iphoneOptions_button(this, oktext, colx[0], rowy[12]);
	// Cancel
	buttons[id_cancel] = new iphoneOptions_button(this, canceltext, colx[4], rowy[12]);
}

void iphoneOptions_gump::load_settings() {
	//string yn;
	item_menu = gwin->get_item_menu();
	dpad_location = gwin->get_dpad_location();
	touch_pathfind = gwin->get_touch_pathfind();
}

iphoneOptions_gump::iphoneOptions_gump()
	: Modal_gump(0, EXULT_FLX_GAMEPLAYOPTIONS_SHP, SF_EXULT_FLX) {
	set_object_area(Rectangle(0, 0, 0, 0), 8, 162);//++++++ ???

	for (int i = id_first; i < id_count; i++)
		buttons[i] = 0;

	load_settings();
	build_buttons();
}

iphoneOptions_gump::~iphoneOptions_gump() {
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			delete buttons[i];
}

void iphoneOptions_gump::save_settings() {
	gwin->set_item_menu(item_menu != 0);
	config->set("config/iphoneos/item_menu",
	            item_menu ? "yes" : "no", false);

	gwin->set_dpad_location(dpad_location);
	config->set("config/iphoneos/dpad_location", dpad_texts[dpad_location], false);

	gwin->set_touch_pathfind(touch_pathfind != 0);
	config->set("config/iphoneos/touch_pathfind",
	            touch_pathfind ? "yes" : "no", false);

	config->write_back();

	touchui->onDpadLocationChanged();
}

void iphoneOptions_gump::paint() {
	Gump::paint();
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			buttons[i]->paint();
	Font *font = fontManager.get_font("SMALL_BLACK_FONT");
	Image_window8 *iwin = gwin->get_win();
	font->paint_text(iwin->get_ib8(), "Item helper menu:", x + colx[0], y + rowy[0] + 1);
	font->paint_text(iwin->get_ib8(), "D-Pad screen location:", x + colx[0], y + rowy[1] + 1);
	font->paint_text(iwin->get_ib8(), "Long touch pathfind:", x + colx[0], y + rowy[2] + 1);

	gwin->set_painted();
}

bool iphoneOptions_gump::mouse_down(int mx, int my, int button) {
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

	if (pushed && !pushed->push(button))            // On a button?
		pushed = 0;

	return button == 1 || pushed != 0;
}

bool iphoneOptions_gump::mouse_up(int mx, int my, int button) {
	// Not Pushing a button?
	if (!pushed) return false;

	if (pushed->get_pushed() != button) return button == 1;

	bool res = false;
	pushed->unpush(button);
	if (pushed->on_button(mx, my))
		res = pushed->activate(button);
	pushed = 0;
	return res;
}
#endif