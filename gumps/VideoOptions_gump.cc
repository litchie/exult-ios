/*
 *  Copyright (C) 2001-2002  The Exult Team
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

using std::cerr;
using std::endl;
using std::string;

static const int rowy[] = { 5, 20, 35, 50, 80, 65 };
static const int colx[] = { 35, 50, 115, 127, 130 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

uint32 *VideoOptions_gump::resolutions = 0;
int VideoOptions_gump::num_resolutions = 0;

uint32 VideoOptions_gump::game_resolutions[3] = {0,0,0};
int VideoOptions_gump::num_game_resolutions = 0;

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
	virtual void activate();
};

void VideoOptions_button::activate()
{
	if (text == canceltext) {
		((VideoOptions_gump*)parent)->cancel();
	} else if (text == oktext) {
		((VideoOptions_gump*)parent)->close();
	}
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
	save_settings();

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
		resolution = state;
	else if(btn==buttons[1])
		scaling = state;
	else if(btn==buttons[2])
		{
		scaler = state;
		rebuild_scale_button();
		paint();
		}
	else if(btn==buttons[3])
		fullscreen = state;
	else if(btn==buttons[4])
		game_resolution = state;
}

void VideoOptions_gump::rebuild_buttons()
{
	for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++)
		{
		delete buttons[i];
		buttons[i] = 0;
		}

	// the text arrays are freed by the destructors of the buttons

	buttons[0] = new VideoTextToggle (this, restext, colx[2], rowy[0], 74,
									  resolution, num_resolutions);

	rebuild_scale_button();

	std::string *enabledtext = new std::string[2];
	enabledtext[0] = "Disabled";
	enabledtext[1] = "Enabled";
	buttons[3] = new VideoTextToggle (this, enabledtext, colx[2], rowy[3], 74,
									  fullscreen, 2);

	std::string *scalers = new std::string[Image_window::NumScalers];
	for (int i = 0; i < Image_window::NumScalers; i++)
		scalers[i] = Image_window::get_name_for_scaler(i);

	buttons[2] = new VideoTextToggle (this, scalers, colx[2], rowy[2], 74,
									  scaler, Image_window::NumScalers);

	buttons[4] = new VideoTextToggle (this, game_restext, colx[2], rowy[5], 74,
									  game_resolution, num_game_resolutions);

}

void VideoOptions_gump::rebuild_scale_button()
{
	delete buttons[1];
	buttons[1] = 0;
	const int max_scales = scaling > 8 && scaling <= 16 ? scaling : 8;
	const int num_scales = (scaler == Image_window::point ||
	                  scaler == Image_window::interlaced ||
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
		buttons[1] = new VideoTextToggle (this, scalingtext, colx[2], rowy[1], 
				74, scaling, num_scales);
	}
	else if (scaler == Image_window::Hq3x)
		scaling = 2;
	else
		scaling = 1;
}

void VideoOptions_gump::load_settings()
{
	int w = gwin->get_win()->get_display_width();
	int h = gwin->get_win()->get_display_height();

	if (resolutions == 0)
	{
		const std::map<uint32, Image_window::Resolution> &Resolutions = gwin->get_win()->Resolutions;

		num_resolutions = Resolutions.size();

		if (Resolutions.find((w<<16)|h) == Resolutions.end()) {
			resolution = num_resolutions++;
		}

		int i = 0;
		resolutions = new uint32[num_resolutions];

		for (std::map<uint32, Image_window::Resolution>::const_iterator it = Resolutions.begin(); it != Resolutions.end(); ++it)
			resolutions[i++] = it->first;

		if (num_resolutions != i)
			resolutions[i] = (w<<16)|h;
	}

	resolution = 0;
	restext = new std::string[num_resolutions];
	for (int i = 0; i < num_resolutions; i++)
	{
		int rw = resolutions[i]>>16;
		int rh = resolutions[i]&0xFFFF;
		restext[i] = resolutionstring(rw, rh);
		if (rw==w && rh==h) resolution = i;
	}

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

	game_restext = new std::string[3];

	game_restext[0] = "Auto";
	game_restext[1] = "320x200";
	game_restext[2] = resolutionstring(game_resolutions[2]>>16, game_resolutions[2]&0xFFFF);

	scaling = gwin->get_win()->get_scale_factor()-1;
	scaler = gwin->get_win()->get_scaler();
	fullscreen = gwin->get_win()->is_fullscreen()?1:0;
	gclock->set_palette();
}

VideoOptions_gump::VideoOptions_gump() : Modal_gump(0, EXULT_FLX_VIDEOOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 95);//++++++ ???

	for (int i=0; i<10; i++) buttons[i] = 0;

	load_settings();
	
	rebuild_buttons();

	// Ok
	buttons[8] = new VideoOptions_button(this, oktext, colx[0], rowy[4]);
	// Cancel
	buttons[9] = new VideoOptions_button(this, canceltext, colx[4], rowy[4]);
}

VideoOptions_gump::~VideoOptions_gump()
{
	for (int i=0; i<10; i++)
		if (buttons[i]) delete buttons[i];
}

void VideoOptions_gump::save_settings()
{
	int resx = resolutions[resolution]>>16;
	int resy = resolutions[resolution]&0xFFFF;
	int gw = game_resolutions[game_resolution]>>16;
	int gh = game_resolutions[game_resolution]&0xFFFF; 

	config->set("config/video/display/width", resx, false);
	config->set("config/video/display/height", resy, false);
	config->set("config/video/game/width", gw, false);
	config->set("config/video/game/height", gh, false);
	config->set("config/video/scale", scaling+1, false);
	if (scaler > Image_window::NoScaler && scaler < Image_window::NumScalers)
		config->set("config/video/scale_method",Image_window::get_name_for_scaler(scaler),false);
	config->set("config/video/fullscreen", fullscreen ? "yes" : "no", false);
	
	config->write_back();

	gwin->resized(resx, resy, fullscreen!=0, gw, gh, scaling+1, scaler);

	gwin->set_painted();
}

void VideoOptions_gump::paint()
{
	Gump::paint();
	for (int i=0; i<10; i++)
		if (buttons[i])
			buttons[i]->paint();

	sman->paint_text(2, "Resolution:", x + colx[0], y + rowy[0] + 1);
	if (scaler == Image_window::point || scaler == Image_window::interlaced ||
			scaler == Image_window::OpenGL)
		sman->paint_text(2, "Scaling:", x + colx[0], y + rowy[1] + 1);
	sman->paint_text(2, "Scaler:", x + colx[0], y + rowy[2] + 1);
	sman->paint_text(2, "Full Screen:", x + colx[0], y + rowy[3] + 1);
	sman->paint_text(2, "Game Res:", x + colx[0], y + rowy[5] + 1);
	gwin->set_painted();
}

void VideoOptions_gump::mouse_down(int mx, int my)
{
	pushed = Gump::on_button(mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i=0; i<10; i++)
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

void VideoOptions_gump::mouse_up(int mx, int my)
{
	if (pushed)			// Pushing a button?
	{
		pushed->unpush();
		if (pushed->on_button(mx, my))
			((Gump_button*)pushed)->activate();
		pushed = 0;
	}
}
