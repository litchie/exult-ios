/*
Copyright (C) 2001 The Exult Team

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

#include <iostream>

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

using std::cerr;
using std::endl;
using std::string;

static const int rowy[] = { 5, 20, 35, 50, 65, 80, 95, 146 };
static const int colx[] = { 35, 50, 120, 127, 130 };

class VideoOptions_button : public Gump_button {
public:
	VideoOptions_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py, SF_EXULT_FLX)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
};

void VideoOptions_button::activate(Game_window *gwin)
{
	switch (get_shapenum()) {
	case EXULT_FLX_AUD_CANCEL_SHP:
		((VideoOptions_gump*)parent)->cancel();
		break;
	case EXULT_FLX_AUD_OK_SHP:
		((VideoOptions_gump*)parent)->close(gwin);
		break;
	}
}

class VideoToggle : public Gump_ToggleButton {
public:
	VideoToggle(Gump* par, int px, int py, int shapenum, 
				int selectionnum, int numsel)
		: Gump_ToggleButton(par, px, py, shapenum, selectionnum, numsel) {}

	friend class VideoOptions_gump;
	virtual void toggle(int state) { 
		((VideoOptions_gump*)parent)->toggle((Gump_button*)this, state);
	}
};


void VideoOptions_gump::close(Game_window* gwin)
{
	save_settings();
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
	// resolution
	buttons[0] = new VideoToggle(this, colx[3], rowy[0], EXULT_FLX_VID_RESOLUTION_SHP, resolution, 5);
	buttons[1] = new VideoToggle(this, colx[3], rowy[1], EXULT_FLX_VID_SCALING_SHP, scaling, 2);
	buttons[2] = new VideoToggle(this, colx[2], rowy[2], EXULT_FLX_VID_SCALER_SHP, scaler, 5);
	buttons[3] = new VideoToggle(this, colx[3], rowy[3], EXULT_FLX_AUD_ENABLED_SHP, fullscreen, 2);
}

void VideoOptions_gump::load_settings()
{
	Game_window *gwin = Game_window::get_game_window();
	long res_array[] = { 320*200, 320*240, 400*300, 512*384, 800*600, -1 };
	long pixels = gwin->get_width()*gwin->get_height();
	for(resolution=0; res_array[resolution]>0 && res_array[resolution]!=pixels; resolution++)
	if(res_array[resolution]<0)
		resolution = 0;
	scaling = gwin->get_win()->get_scale()-1;
	scaler = gwin->get_win()->get_scaler();
	fullscreen = gwin->get_win()->is_fullscreen()?1:0;
	gwin->set_palette();
	
}

VideoOptions_gump::VideoOptions_gump() : Modal_gump(0, EXULT_FLX_VIDEOOPTIONS_SHP, SF_EXULT_FLX)
{
	set_object_area(Rectangle(0,0,0,0), 8, 162);//++++++ ???

	for (int i=0; i<10; i++) buttons[i] = 0;

	load_settings();
	
	build_buttons();

	// Ok
	buttons[8] = new VideoOptions_button(this, colx[0], rowy[7], EXULT_FLX_AUD_OK_SHP);
	// Cancel
	buttons[9] = new VideoOptions_button(this, colx[4], rowy[7], EXULT_FLX_AUD_CANCEL_SHP);
}

VideoOptions_gump::~VideoOptions_gump()
{
	for (int i=0; i<10; i++)
		if (buttons[i]) delete buttons[i];
}

void VideoOptions_gump::save_settings()
{
	Game_window *gwin = Game_window::get_game_window();
	
	int resx, resy;
	switch(resolution) {
	case 0:
		resx= 320;
		resy= 200;
		break;
	case 1:
		resx= 320;
		resy= 240;
		break;
	case 2:
		resx= 400;
		resy= 300;
		break;
	case 3:
		resx= 512;	
		resy= 384;
		break;
	case 4:
		resx= 800;
		resy= 600;
		break;
	}
	config->set("config/video/width", resx, true);
	config->set("config/video/height", resy, true);
	config->set("config/video/scale", scaling+1, true);
	if (scaler == Image_window::bilinear)
		config->set("config/video/scale_method","bilinear",true);
	else if (scaler == Image_window::interlaced)
		config->set("config/video/scale_method","interlaced",true);
	else if (scaler == Image_window::SuperEagle)
		config->set("config/video/scale_method","SuperEagle",true);
	else if (scaler == Image_window::point)
		config->set("config/video/scale_method","point",true);
	else
		config->set("config/video/scale_method","2xSaI",true);
	config->set("config/video/fullscreen", fullscreen ? "yes" : "no", true);
	
	gwin->resized(resx,resy,scaling+1,scaler);
	if(((fullscreen==0)&&(gwin->get_win()->is_fullscreen()))||
	   ((fullscreen==1)&&(!gwin->get_win()->is_fullscreen())))
		gwin->get_win()->toggle_fullscreen();
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
	gwin->paint_text(2, "Gamma R:", x + colx[0], y + rowy[4] + 1);
	gwin->paint_text(2, "Gamma G:", x + colx[0], y + rowy[5] + 1);
	gwin->paint_text(2, "Gamma B:", x + colx[0], y + rowy[6] + 1);
	gwin->set_painted();
}

void VideoOptions_gump::mouse_down(int mx, int my)
{
	Game_window *gwin = Game_window::get_game_window();
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
	Game_window *gwin = Game_window::get_game_window();
	if (pushed)			// Pushing a button?
	{
		pushed->unpush(gwin);
		if (pushed->on_button(gwin, mx, my))
			((Gump_button*)pushed)->activate(gwin);
		pushed = 0;
	}
}
