/*
 *  Copyright (C) 2001-2011  The Exult Team
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
#include <string>
#include <cstring>

#include "SDL_events.h"

#include "Configuration.h"
#include "Gump_button.h"
#include "Gump_ToggleButton.h"
#include "VideoOptions_gump.h"
#include "exult.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "gameclk.h"
#include "mouse.h"
#include "Text_button.h"
#include "palette.h"
#include "Yesno_gump.h"
#include "font.h"

using std::cerr;
using std::endl;
using std::string;

static const int rowy[] = { 20, 45, 60, 5, 155, 85, 100, 115, 130 };
static const int colx[] = { 35, 50, 115, 127, 130 };

static const char* applytext = "APPLY";

uint32 *VideoOptions_gump::resolutions = 0;
int VideoOptions_gump::num_resolutions = 0;

uint32 *VideoOptions_gump::win_resolutions = 0;
int VideoOptions_gump::num_win_resolutions = 0;

uint32 VideoOptions_gump::game_resolutions[3] = {0,0,0};
int VideoOptions_gump::num_game_resolutions = 0;

Image_window::FillMode VideoOptions_gump::startup_fill_mode = (Image_window::FillMode)0;

static string resolutionstring(int w, int h)
{
	char buf[100];
	sprintf(buf, "%ix%i", w, h);
	return buf;
}


class VideoOptions_button : public Text_button {
public:
	VideoOptions_button(Gump *par, string text, int px, int py)
		: Text_button(par, text, px, py, 59, 11)
		{  }
					// What to do when 'clicked':
	virtual bool activate(int button=1);
};

bool VideoOptions_button::activate(int button)
{
	if (button != 1) return false;
	if (text == applytext) {
		((VideoOptions_gump*)parent)->save_settings();
	}
	return true;
}

class VideoTextToggle : public Gump_ToggleTextButton {
public:
	VideoTextToggle(Gump* par, std::string *s, int px, int py, int width, 
				int selectionnum, int numsel)
		: Gump_ToggleTextButton(par, s, selectionnum, numsel, px, py, width) {}

	friend class VideoOptions_gump;
	virtual void toggle(int state) { 
		((VideoOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};
void VideoOptions_gump::close()
{
	//save_settings();

	// have to repaint everything in case resolution changed
	gwin->set_all_dirty();
	done = 1;
}

void VideoOptions_gump::cancel()
{
	done = 1;
}

void VideoOptions_gump::toggle(Gump_button* btn, int state)
{
	if(btn==buttons[0])
	{
		if (fullscreen) resolution = resolutions[state];
		else resolution = win_resolutions[state];
	}
	else if(btn==buttons[1])
		scaling = state;
	else if(btn==buttons[2])
	{
		scaler = state;
		rebuild_dynamic_buttons();
	}
	else if(btn==buttons[3])
	{
		fullscreen = state;
		rebuild_dynamic_buttons();
	}
	else if(btn==buttons[4])
		game_resolution = state;
	else if(btn==buttons[5])
		fill_scaler = state;
	else if(btn==buttons[6]) {
		if (state == 0) fill_mode = Image_window::Fill;
		else if (state == 3) fill_mode = startup_fill_mode;
		else fill_mode = (Image_window::FillMode) ((state << 1) | (has_ac?1:0));
		rebuild_dynamic_buttons();
	}
	else if(btn==buttons[7]) {
		has_ac = state!=0;
		fill_mode = (Image_window::FillMode) ((fill_mode&~1) | (has_ac?1:0));
	}

	paint();
}

void VideoOptions_gump::rebuild_buttons()
{
	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		{
		delete buttons[i];
		buttons[i] = 0;
		}

	// the text arrays are freed by the destructors of the buttons

	std::string *enabledtext = new std::string[2];
	enabledtext[0] = "Disabled";
	enabledtext[1] = "Enabled";
	buttons[3] = new VideoTextToggle (this, enabledtext, colx[2], rowy[3], 74,
									  fullscreen, 2);

	std::string *scalers = new std::string[Image_window::NumScalers];
	for (int i = 0; i < Image_window::NumScalers; i++)
		scalers[i] = Image_window::get_name_for_scaler(i);

	buttons[2] = new VideoTextToggle (this, scalers, colx[2], rowy[1], 74,
									  scaler, Image_window::NumScalers);

	std::string *game_restext = new std::string[3];
	game_restext[0] = "Auto";
	game_restext[1] = "320x200";
	game_restext[2] = resolutionstring(game_resolutions[2]>>16, game_resolutions[2]&0xFFFF);

	buttons[4] = new VideoTextToggle (this, game_restext, colx[2], rowy[5], 74,
									  game_resolution, num_game_resolutions);

	std::string *fill_scaler_text = new std::string[2];
	fill_scaler_text[0] = "Point";
	fill_scaler_text[1] = "Bilinear";
	buttons[5] = new VideoTextToggle (this, fill_scaler_text , colx[2], rowy[6], 74,
									  fill_scaler, 2);

	int sel_fill_mode;
	has_ac = false;

	if (fill_mode == Image_window::Fill)
	{
		sel_fill_mode = 0;
	}
	else if (fill_mode == Image_window::Fit)
	{
		sel_fill_mode = 1;
	}
	else if (fill_mode == Image_window::AspectCorrectFit )
	{
		sel_fill_mode = 1;
		has_ac = true;
	}
	else if (fill_mode == Image_window::Centre)
	{
		sel_fill_mode = 2;
	}
	else if (fill_mode == Image_window::AspectCorrectCentre)
	{
		sel_fill_mode = 2;
		has_ac = true;
	}
	else
	{
		sel_fill_mode = 3;
	}

	int num_fill_modes = 3;
	if (startup_fill_mode > Image_window::AspectCorrectCentre) num_fill_modes = 4;

	std::string *fill_mode_text = new std::string[4];
	fill_mode_text[0] = "Fill";
	fill_mode_text[1] = "Fit";
	fill_mode_text[2] = "Centre";
	fill_mode_text[3] = "Custom";

	buttons[6] = new VideoTextToggle (this, fill_mode_text, colx[2], rowy[7], 74, sel_fill_mode, num_fill_modes);

	rebuild_dynamic_buttons();
}

void VideoOptions_gump::rebuild_dynamic_buttons()
{
	delete buttons[0];
	buttons[0] = 0;
	delete buttons[1];
	buttons[1] = 0;
	delete buttons[7];
	buttons[7] = 0;

	int num_resolutions;
	uint32 *resolutions;
	uint32 current_res = 0;

	if (fullscreen)
	{
		num_resolutions = VideoOptions_gump::num_resolutions;
		resolutions = VideoOptions_gump::resolutions;
		if (!gwin->get_win()->is_fullscreen())
			current_res = (gwin->get_win()->get_display_width()<<16)|(gwin->get_win()->get_display_height());
	}
	else
	{
		num_resolutions = VideoOptions_gump::num_win_resolutions;
		resolutions = VideoOptions_gump::win_resolutions;
		current_res = (gwin->get_win()->get_display_width()<<16)|(gwin->get_win()->get_display_height());
	}
		
	int selected_res = 0;
	std::string *restext = new std::string[num_resolutions+1];
	for (int i = 0; i < num_resolutions; i++)
	{
		int rw = resolutions[i]>>16;
		int rh = resolutions[i]&0xFFFF;
		restext[i] = resolutionstring(rw, rh);
		if (resolutions[i] <= resolution && resolutions[selected_res] < resolutions[i]) 
			selected_res = i;
		if (resolutions[i] == current_res) current_res = 0;
	}
	if (current_res)
	{
		restext[num_resolutions] = resolutionstring(current_res>>16, current_res&0xFFFF);

		if (resolutions[num_resolutions] <= resolution && resolutions[selected_res] < resolutions[num_resolutions]) 
		{
			selected_res = num_resolutions;
			resolutions[num_resolutions] = current_res;
		}

		num_resolutions++;
	}

	resolution = resolutions[selected_res];

	buttons[0] = new VideoTextToggle (this, restext, colx[2], rowy[0], 74,
									  selected_res, num_resolutions);

	buttons[1] = 0;
	const int max_scales = scaling > 8 && scaling <= 16 ? scaling : 8;
	const int num_scales = (scaler == Image_window::point ||
	                  scaler == Image_window::interlaced ||
					  scaler == Image_window::bilinear ||
					  scaler == Image_window::OpenGL) ? max_scales : 1;
	if (num_scales > 1)
	{
		// the text arrays is freed by the destructor of the button
		std::string *scalingtext = new std::string[num_scales];
		for (int i = 0; i < num_scales; i++)
			{
			char buf[10];
			snprintf(buf, sizeof(buf), "x%d", i+1);
			scalingtext[i] = buf;
			}
		buttons[1] = new VideoTextToggle (this, scalingtext, colx[2], rowy[2], 
				74, scaling, num_scales);
	}
	else if (scaler == Image_window::Hq3x)
		scaling = 2;
	else
		scaling = 1;

	if (fill_mode == Image_window::Fit ||fill_mode == Image_window::AspectCorrectFit ||fill_mode == Image_window::Centre ||fill_mode == Image_window::AspectCorrectCentre)
	{
		std::string *ac_text = new std::string[2];
		ac_text[0] = "Disabled";
		ac_text[1] = "Enabled";
		buttons[7] = new VideoTextToggle (this, ac_text, colx[3], rowy[8], 62, has_ac?1:0, 2);
	}
}

void VideoOptions_gump::load_settings()
{
	int i;
	int w = gwin->get_win()->get_display_width();
	int h = gwin->get_win()->get_display_height();

	if (resolutions == 0)
	{
		std::map<uint32, Image_window::Resolution> Resolutions = gwin->get_win()->Resolutions;
		if (gwin->get_win()->is_fullscreen()) Resolutions[(w<<16)|h] = Image_window::Resolution();

		num_resolutions = Resolutions.size();
		resolutions = new uint32[num_resolutions+1];

		i = 0;
		for (std::map<uint32, Image_window::Resolution>::const_iterator it = Resolutions.begin(); it != Resolutions.end(); ++it)
			resolutions[i++] = it->first;

		// Add in useful window resolutions
		Resolutions[(320<<16)|200] = Image_window::Resolution();
		Resolutions[(320<<16)|240] = Image_window::Resolution();
		Resolutions[(400<<16)|300] = Image_window::Resolution();
		Resolutions[(512<<16)|384] = Image_window::Resolution();
		Resolutions[(640<<16)|400] = Image_window::Resolution();
		Resolutions[(640<<16)|480] = Image_window::Resolution();
		Resolutions[(960<<16)|600] = Image_window::Resolution();
		Resolutions[(960<<16)|720] = Image_window::Resolution();
		if (!gwin->get_win()->is_fullscreen()) Resolutions[(w<<16)|h] = Image_window::Resolution();

		num_win_resolutions = Resolutions.size();
		win_resolutions = new uint32[num_win_resolutions+1];

		i = 0;
		for (std::map<uint32, Image_window::Resolution>::const_iterator it = Resolutions.begin(); it != Resolutions.end(); ++it)
			win_resolutions[i++] = it->first;
	}

	resolution = (w<<16)|h;

	if (startup_fill_mode == 0)
		startup_fill_mode = gwin->get_win()->get_fill_mode();

	has_ac = 0;

	int gw, gh; 
	config->value("config/video/game/width", gw, w);
	config->value("config/video/game/height", gh, h);

	if (gw == 0 && gh == 0)
		game_resolution = 0;
	else if (gw == 320 && gh == 200)
		game_resolution = 1;
	else {
		game_resolution = 2;
	}

	if (num_game_resolutions == 0)
	{
		game_resolutions[0] = 0;
		game_resolutions[1] = (320<<16)|200;
		game_resolutions[2] = (gw<<16)|gh;
		num_game_resolutions = (game_resolutions[0] != game_resolutions[2] && game_resolutions[1] != game_resolutions[2])?3:2;
	}

	scaling = gwin->get_win()->get_scale_factor()-1;
	scaler = gwin->get_win()->get_scaler();
	fullscreen = gwin->get_win()->is_fullscreen()?1:0;
	fill_scaler = gwin->get_win()->get_fill_scaler()==Image_window::bilinear?1:0;
	fill_mode = gwin->get_win()->get_fill_mode();

	gclock->set_palette();

	o_resolution = resolution;
	o_scaling = scaling;
	o_scaler = scaler;
	o_fullscreen = fullscreen;
	o_fill_scaler = fill_scaler;
	o_fill_mode = fill_mode;

	o_game_resolution = game_resolution;
}

VideoOptions_gump::VideoOptions_gump() : Modal_gump(0, EXULT_FLX_VIDEOOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 170);//++++++ ???

	for (int i=0; i<10; i++) buttons[i] = 0;

	load_settings();
	
	rebuild_buttons();

	// Ok
	//buttons[8] = new VideoOptions_button(this, oktext, colx[0], rowy[4]);
	// Cancel
	buttons[9] = new VideoOptions_button(this, applytext, colx[4], rowy[4]);
}

VideoOptions_gump::~VideoOptions_gump()
{
	for (int i=0; i<10; i++)
		if (buttons[i]) delete buttons[i];
}

void VideoOptions_gump::save_settings()
{
	int resx = resolution>>16;
	int resy = resolution&0xFFFF;
	int gw = game_resolutions[game_resolution]>>16;
	int gh = game_resolutions[game_resolution]&0xFFFF; 

	int tgw = gw, tgh = gh, tw, th;
	Image_window::get_draw_dims(resx,resy,scaling+1,fill_mode, tgw,tgh,tw,th);
	if (tw/(scaling+1) < 320 || th/(scaling+1) < 200)
	{
		if (!Yesno_gump::ask("Scaled size less than 320x200.\nExult may be unusable.\nApply anyway?", "TINY_BLACK_FONT")) 
			return;
	}

	config->set("config/video/display/width", resx, false);
	config->set("config/video/display/height", resy, false);
	config->set("config/video/game/width", gw, false);
	config->set("config/video/game/height", gh, false);
	config->set("config/video/scale", scaling+1, false);
	config->set("config/video/scale_method",Image_window::get_name_for_scaler(scaler),false);
	config->set("config/video/fullscreen", fullscreen ? "yes" : "no", false);
	
	std::string fmode_string;
	Image_window::fillmode_to_string(fill_mode,fmode_string);
	config->set("config/video/fill_mode",fmode_string,false);

	config->set("config/video/fill_scaler",fill_scaler?"Bilinear":"Point",false);

	gwin->resized(resx, resy, fullscreen!=0, gw, gh, scaling+1, scaler, fill_mode, fill_scaler?Image_window::bilinear:Image_window::point);
	gclock->set_palette();
	set_pos();
	gwin->set_all_dirty();

	if (!Countdown_gump::ask("Settings applied.\nKeep? %i...",20))
	{
		resx = o_resolution>>16;
		resy = o_resolution&0xFFFF;
		gw = game_resolutions[o_game_resolution]>>16;
		gh = game_resolutions[o_game_resolution]&0xFFFF; 

		config->set("config/video/display/width", resx, false);
		config->set("config/video/display/height", resy, false);
		config->set("config/video/game/width", gw, false);
		config->set("config/video/game/height", gh, false);
		config->set("config/video/scale", o_scaling +1, false);
		config->set("config/video/scale_method",Image_window::get_name_for_scaler(o_scaler),false);
		config->set("config/video/fullscreen", o_fullscreen ? "yes" : "no", false);
		
		Image_window::fillmode_to_string(o_fill_mode,fmode_string);
		config->set("config/video/fill_mode",fmode_string,false);

		config->set("config/video/fill_scaler",o_fill_scaler?"Bilinear":"Point",false);

		gwin->resized(resx, resy, o_fullscreen!=0, gw, gh, o_scaling+1, o_scaler, o_fill_mode, o_fill_scaler?Image_window::bilinear:Image_window::point);
		gclock->set_palette();
		set_pos();
		gwin->set_all_dirty();
	}
	else
	{
		o_resolution = resolution;
		o_scaling = scaling;
		o_scaler = scaler;
		o_fullscreen = fullscreen;
		o_game_resolution = game_resolution;
		o_fill_mode = fill_mode;
		o_fill_scaler = fill_scaler;
	}

	config->write_back();
}

void VideoOptions_gump::paint()
{
	Gump::paint();
	for (int i=0; i<10; i++)
		if (buttons[i])
			buttons[i]->paint();

	Font *font = fontManager.get_font("SMALL_BLACK_FONT");
	Image_window8 *iwin = gwin->get_win();

	if (fullscreen) font->paint_text(iwin->get_ib8(), "Display Mode:", x + colx[0], y + rowy[0] + 1);
	else font->paint_text(iwin->get_ib8(), "Window Size:", x + colx[0], y + rowy[0] + 1);
	if (buttons[1]) font->paint_text(iwin->get_ib8(), "Scaling:", x + colx[0], y + rowy[2] + 1);
	font->paint_text(iwin->get_ib8(), "Scaler:", x + colx[0], y + rowy[1] + 1);
	font->paint_text(iwin->get_ib8(), "Full Screen:", x + colx[0], y + rowy[3] + 1);
	font->paint_text(iwin->get_ib8(), "Game Area:", x + colx[0], y + rowy[5] + 1);
	font->paint_text(iwin->get_ib8(), "Fill Quality:", x + colx[0], y + rowy[6] + 1);
	font->paint_text(iwin->get_ib8(), "Fill Mode:", x + colx[0], y + rowy[7] + 1);
	if (buttons[7]) font->paint_text(iwin->get_ib8(), "AR Correction:", x + colx[0], y + rowy[8] + 1);
	gwin->set_painted();
}

bool VideoOptions_gump::mouse_down(int mx, int my, int button)
{
	// Only left and right buttons
	if (button != 1 && button != 3) return false;

	// We'll eat the mouse down if we've already got a button down
	if (pushed) return true;

	// First try checkmark
	pushed = Gump::on_button(mx, my);
					
	// Try buttons at bottom.
	if (!pushed) {
		for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++) {
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

bool VideoOptions_gump::mouse_up(int mx, int my, int button)
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
