/*
 *  Copyright (C) 2001-2013  The Exult Team
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
#include "ShortcutBar_gump.h"
using std::string;

static const int rowy[] = { 4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148, 160, 172 };
static const int colx[] = { 35, 50, 120, 170, 192, 211 };

static const char *oktext = "OK";
static const char *canceltext = "CANCEL";

class MiscOptions_button : public Text_button {
public:
	MiscOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
	{ }
	// What to do when 'clicked':
	virtual bool activate(int button) {
		if (button != 1) return false;

		if (text == canceltext) {
			static_cast<MiscOptions_gump *>(parent)->cancel();
		} else if (text == oktext) {
			static_cast<MiscOptions_gump *>(parent)->close();
		}
		return true;
	}
};

class MiscTextToggle : public Gump_ToggleTextButton {
public:
	MiscTextToggle(Gump *par, std::string *s, int px, int py, int width,
	               int selectionnum, int numsel)
		: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width)
	{ }

	friend class MiscOptions_gump;
	virtual void toggle(int state) {
		static_cast<MiscOptions_gump *>(parent)->toggle(this, state);
	}
};

class MiscEnabledToggle : public Enabled_button {
public:
	MiscEnabledToggle(Gump *par, int px, int py, int width,
	                  int selectionnum)
		: Enabled_button(par, selectionnum, px, py, width)
	{ }

	friend class MiscOptions_gump;
	virtual void toggle(int state) {
		static_cast<MiscOptions_gump *>(parent)->toggle(this, state);
	}
};

void MiscOptions_gump::close() {
	save_settings();
	done = 1;
}

void MiscOptions_gump::cancel() {
	done = 1;
}

void MiscOptions_gump::toggle(Gump_button *btn, int state) {
	if (btn == buttons[id_scroll_mouse])
		scroll_mouse = state;
	else if (btn == buttons[id_menu_intro])
		menu_intro = state;
	else if (btn == buttons[id_usecode_intro])
		usecode_intro = state;
	else if (btn == buttons[id_sc_enabled])
		sc_enabled = state;
	else if (btn == buttons[id_sc_outline])
		sc_outline = state;
	else if (btn == buttons[id_sb_hide_missing])
		sb_hide_missing = state;
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

void MiscOptions_gump::build_buttons() {
	string *yesNo1 = new string[2]; // TODO:need to make this like enabled
	yesNo1[0] = "No";              // if I am going to add much more
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

	string *sc_enabled_txt = new string[3];
	sc_enabled_txt[0] = "No";
	sc_enabled_txt[1] = "transparent";
	sc_enabled_txt[2] = "Yes";

	sc_outline_txt = new string[8]; // keep in order of Pixel_colors
	sc_outline_txt[0] = "green";
	sc_outline_txt[1] = "white";
	sc_outline_txt[2] = "yellow";
	sc_outline_txt[3] = "blue";
	sc_outline_txt[4] = "red";
	sc_outline_txt[5] = "purple";
	sc_outline_txt[6] = "black";
	sc_outline_txt[7] = "No"; // needs to be last

	string *yesNo4 = new string[2];
	yesNo4[0] = "No";
	yesNo4[1] = "Yes";

	int y_index = 0;
	int small_size = 44;
	int large_size = 85;
	buttons[id_scroll_mouse] = new MiscTextToggle(this, yesNo1, colx[5], rowy[y_index],
	        small_size, scroll_mouse, 2);
	buttons[id_menu_intro] = new MiscTextToggle(this, yesNo2, colx[5], rowy[++y_index],
	        small_size, menu_intro, 2);
	buttons[id_usecode_intro] = new MiscTextToggle(this, yesNo3, colx[5], rowy[++y_index],
	        small_size, usecode_intro, 2);
	buttons[id_alternate_drop] = new MiscTextToggle(this, stacks_text, colx[5], rowy[++y_index],
	        small_size, alternate_drop, 2);
	buttons[id_allow_autonotes] = new MiscTextToggle(this, autonotes_text, colx[5], rowy[++y_index],
	        small_size, allow_autonotes, 2);
	buttons[id_sc_enabled] = new MiscTextToggle(this, sc_enabled_txt, colx[3], rowy[++y_index],
	        large_size, sc_enabled, 3);
	buttons[id_sc_outline] = new MiscTextToggle(this, sc_outline_txt, colx[5], rowy[++y_index],
	        small_size, sc_outline, 8);
	buttons[id_sb_hide_missing] = new MiscTextToggle(this, yesNo4, colx[5], rowy[++y_index],
	        small_size, sb_hide_missing, 2);
	// two row gap
	buttons[id_difficulty] = new MiscTextToggle(this, diffs, colx[3], rowy[y_index+=3],
	        large_size, difficulty, 7);
	buttons[id_show_hits] = new MiscEnabledToggle(this, colx[3], rowy[++y_index],
	        large_size, show_hits);

	std::string *modes = new std::string[2];
	modes[0] = "Original";
	modes[1] = "Space pauses";
	buttons[id_mode] = new MiscTextToggle(this, modes, colx[3], rowy[++y_index],
	                                      large_size, mode, 2);
	std::string *charmedDiff = new std::string[2];
	charmedDiff[0] = "Normal";
	charmedDiff[1] = "Hard";
	buttons[id_charmDiff] = new MiscTextToggle(this, charmedDiff, colx[3], rowy[++y_index],
	        large_size, charmDiff, 2);
	// Ok
	buttons[id_ok] = new MiscOptions_button(this, oktext, colx[0], rowy[++y_index]);
	// Cancel
	buttons[id_cancel] = new MiscOptions_button(this, canceltext, colx[4], rowy[y_index]);
}

void MiscOptions_gump::load_settings() {
	string yn;
	scroll_mouse = gwin->can_scroll_with_mouse();
	config->value("config/gameplay/skip_intro", yn, "no");
	usecode_intro = (yn == "yes");
	config->value("config/gameplay/skip_splash", yn, "no");
	menu_intro = (yn == "yes");

	sc_enabled = gwin->get_shortcutbar_type();
	sc_outline = gwin->get_outline_color();
	sb_hide_missing = gwin->sb_hide_missing_items();

	difficulty = Combat::difficulty;
	if (difficulty < -3)
		difficulty = -3;
	else if (difficulty > 3)
		difficulty = 3;
	difficulty += 3;        // Scale to choices (0-6).
	show_hits = Combat::show_hits ? 1 : 0;
	mode = static_cast<int>(Combat::mode);
	if (mode < 0 || mode > 1)
		mode = 0;
	charmDiff = Combat::charmed_more_difficult ? 1 : 0;
	alternate_drop = gwin->get_alternate_drop();
	allow_autonotes = gwin->get_allow_autonotes();
}

MiscOptions_gump::MiscOptions_gump()
	: Modal_gump(0, EXULT_FLX_MISCOPTIONS_SHP, SF_EXULT_FLX) {
	set_object_area(Rectangle(0, 0, 0, 0), 8, 184);//++++++ ???

	load_settings();
	build_buttons();
}

MiscOptions_gump::~MiscOptions_gump() {
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			delete buttons[i];
}

void MiscOptions_gump::save_settings() {
	config->set("config/gameplay/scroll_with_mouse",
	            scroll_mouse ? "yes" : "no", false);
	gwin->set_mouse_with_scroll(scroll_mouse);
	config->set("config/gameplay/skip_intro",
	            usecode_intro ? "yes" : "no", false);
	config->set("config/gameplay/skip_splash",
	            menu_intro ? "yes" : "no", false);

	string str = "no";
	if(sc_enabled == 1)
		str = "transparent";
	else if(sc_enabled == 2)
		str = "yes";
	config->set("config/shortcutbar/use_shortcutbar", str, false);
	config->set("config/shortcutbar/use_outline_color", sc_outline_txt[sc_outline], false);
	config->set("config/shortcutbar/hide_missing_items", sb_hide_missing ? "yes" : "no", false);

	gwin->set_outline_color(static_cast<Pixel_colors>(sc_outline));
	gwin->set_sb_hide_missing_items(sb_hide_missing);
	gwin->set_shortcutbar(static_cast<uint8>(sc_enabled));
	if(g_shortcutBar)
		g_shortcutBar->set_changed();

	Combat::difficulty = difficulty - 3;
	config->set("config/gameplay/combat/difficulty",
	            Combat::difficulty, false);
	Combat::show_hits = (show_hits != 0);
	config->set("config/gameplay/combat/show_hits",
	            show_hits ? "yes" : "no", false);
	Combat::mode = static_cast<Combat::Mode>(mode);
	str = Combat::mode == Combat::keypause ? "keypause"
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

void MiscOptions_gump::paint() {
	Gump::paint();
	for (int i = id_first; i < id_count; i++)
		if (buttons[i])
			buttons[i]->paint();
	Font *font = fontManager.get_font("SMALL_BLACK_FONT");
	Image_window8 *iwin = gwin->get_win();
	int y_index = 0;
	font->paint_text(iwin->get_ib8(), "Scroll game view with mouse:", x + colx[0], y + rowy[y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Skip intro:", x + colx[0], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Skip scripted first scene:", x + colx[0], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Alternate drag'n'drop:", x + colx[0], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Allow Autonotes:", x + colx[0], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Use ShortcutBar :", x + colx[0], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Use outline color :", x + colx[1], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Hide missing items:", x + colx[1], y + rowy[++y_index] + 1);
	// 1 row gap
	font->paint_text(iwin->get_ib8(), "Combat Options:", x + colx[0], y + rowy[y_index+=2] + 1);
	font->paint_text(iwin->get_ib8(), "Difficulty:", x + colx[1], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Show Hits:", x + colx[1], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Mode:", x + colx[1], y + rowy[++y_index] + 1);
	font->paint_text(iwin->get_ib8(), "Charmed Difficulty:", x + colx[1], y + rowy[++y_index] + 1);
	gwin->set_painted();
}

bool MiscOptions_gump::mouse_down(int mx, int my, int button) {
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

bool MiscOptions_gump::mouse_up(int mx, int my, int button) {
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
