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
#include <string>
#include <cstring>

#include "SDL_events.h"

#include "gump_utils.h"
#include "Configuration.h"
#include "Gump_button.h"
#include "Gump_ToggleButton.h"
#include "VideoOptions_gump.h"
#include "exult.h"
#include "exult_flx.h"
#include "gamewin.h"
#include "mouse.h"
#include "Text_button.h"

using std::cerr;
using std::endl;
using std::string;

static const int rowy[] = { 5, 20, 35, 50, 80 };
static const int colx[] = { 35, 50, 115, 127, 130 };

static const char* oktext = "OK";
static const char* canceltext = "CANCEL";

static int resolutions[] = { 320, 200,
							 320, 240,
							 400, 300,
							 512, 384,
							 640, 480,
							 800, 600,
							 -1, -1 }; 
// These -1's are placeholders for a custom resolution

static int num_default_res = sizeof(resolutions)/(2*sizeof(resolutions[0])) -1;

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
	virtual void activate(Game_window *gwin);
};

void VideoOptions_button::activate(Game_window *gwin)
{
	if (text == canceltext) {
		((VideoOptions_gump*)parent)->cancel();
	} else if (text == oktext) {
		((VideoOptions_gump*)parent)->close(gwin);
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
void VideoOptions_gump::close(Game_window* gwin)
{
	if (resolution != old_resolution) {
		restore_background = false;
	}

	save_settings();

	// have to repaint everything in case resolution changed
	if (!want_restore_background())
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
		scaler = state;
	else if(btn==buttons[3])
		fullscreen = state;
}

void VideoOptions_gump::build_buttons()
{
	// the text arrays are freed by the destructors of the buttons

	buttons[0] = new VideoTextToggle (this, restext, colx[4], rowy[0], 59,
									  resolution, num_resolutions);

	std::string *scalingtext = new std::string[2];
	scalingtext[0] = "x1";
	scalingtext[1] = "x2";
	buttons[1] = new VideoTextToggle (this, scalingtext, colx[4], rowy[1], 59,
									  scaling, 2);

	std::string *enabledtext = new std::string[2];
	enabledtext[0] = "Disabled";
	enabledtext[1] = "Enabled";
	buttons[3] = new VideoTextToggle (this, enabledtext, colx[4], rowy[3], 59,
									  fullscreen, 2);

	std::string *scalers = new std::string[Image_window::NumScalers];
	for (int i = 0; i < Image_window::NumScalers; i++)
		scalers[i] = Image_window::get_name_for_scaler(i);

	buttons[2] = new VideoTextToggle (this, scalers, colx[2], rowy[2], 74,
									  scaler, Image_window::NumScalers);
}

void VideoOptions_gump::load_settings()
{
	Game_window *gwin = Game_window::get_instance();
	int w = gwin->get_width();
	int h = gwin->get_height();

	resolutions[2*num_default_res] = w;
	resolutions[2*num_default_res+1] = h;

	num_resolutions = num_default_res;
	
	resolution = -1;
	int i;
	for (i = 0; i < num_default_res; i++) {
		if (resolutions[2*i] == w && resolutions[2*i+1] == h) {
			resolution = i;
			break;
		}
	}
	
	if (resolution == -1) {
		num_resolutions++;
		resolution = num_default_res;
	}

	restext = new std::string[num_resolutions];
	for (i = 0; i < num_resolutions; i++) {
		restext[i] = resolutionstring(resolutions[2*i], resolutions[2*i+1]);
	}

	old_resolution = resolution;
	scaling = gwin->get_win()->get_scale()-1;
	scaler = gwin->get_win()->get_scaler();
	fullscreen = gwin->get_win()->is_fullscreen()?1:0;
	gwin->set_palette();
	
}

VideoOptions_gump::VideoOptions_gump() : Modal_gump(0, EXULT_FLX_VIDEOOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 95);//++++++ ???

	for (int i=0; i<10; i++) buttons[i] = 0;

	load_settings();
	
	build_buttons();

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
	Game_window *gwin = Game_window::get_instance();
	
	int resx = resolutions[2*resolution];
	int resy = resolutions[2*resolution+1];
	config->set("config/video/width", resx, true);
	config->set("config/video/height", resy, true);
	config->set("config/video/scale", scaling+1, true);
	if (scaler > Image_window::NoScaler && scaler < Image_window::NumScalers)
		config->set("config/video/scale_method",Image_window::get_name_for_scaler(scaler),true);
	config->set("config/video/fullscreen", fullscreen ? "yes" : "no", true);
	
	gwin->resized(resx,resy,scaling+1,scaler);
	if(((fullscreen==0)&&(gwin->get_win()->is_fullscreen()))||
	   ((fullscreen==1)&&(!gwin->get_win()->is_fullscreen())))
		{
		gwin->get_win()->toggle_fullscreen();
		gwin->paint();
		}
	gwin->set_painted();
}

void VideoOptions_gump::paint(Game_window* gwin)
{
	Gump::paint(gwin);
	for (int i=0; i<10; i++)
		if (buttons[i])
			buttons[i]->paint(gwin);

	gwin->paint_text(2, "Resolution:", x + colx[0], y + rowy[0] + 1);
	gwin->paint_text(2, "Scaling:", x + colx[0], y + rowy[1] + 1);
	gwin->paint_text(2, "Scaler:", x + colx[0], y + rowy[2] + 1);
	gwin->paint_text(2, "Full Screen:", x + colx[0], y + rowy[3] + 1);
	gwin->set_painted();
}

void VideoOptions_gump::mouse_down(int mx, int my)
{
	Game_window *gwin = Game_window::get_instance();
	pushed = Gump::on_button(gwin, mx, my);
					// First try checkmark.
	// Try buttons at bottom.
	if (!pushed)
		for (int i=0; i<10; i++)
			if (buttons[i] && buttons[i]->on_button(gwin, mx, my)) {
				pushed = buttons[i];
				break;
			}

	if (pushed)			// On a button?
	{
		pushed->push(gwin);
		return;
	}
}

void VideoOptions_gump::mouse_up(int mx, int my)
{
	Game_window *gwin = Game_window::get_instance();
	if (pushed)			// Pushing a button?
	{
		pushed->unpush(gwin);
		if (pushed->on_button(gwin, mx, my))
			((Gump_button*)pushed)->activate(gwin);
		pushed = 0;
	}
}
