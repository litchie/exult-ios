/*
Copyright (C) 2001-2013 The Exult Team

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

#ifndef _VIDEOOPTIONS_GUMP_H
#define _VIDEOOPTIONS_GUMP_H

#include "Modal_gump.h"
#include <string>
#include "imagewin/imagewin.h"

class Gump_button;

class VideoOptions_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(VideoOptions_gump,Modal_gump(0,0,0,0));
	static VideoOptions_gump *video_options_gump;
 private:

	uint32 resolution;
	int scaling;
	int scaler;
	int fullscreen;
	uint32 game_resolution;
	int fill_scaler;
	Image_window::FillMode fill_mode;
	bool has_ac;
	bool share_settings;

	bool o_share_settings;
	uint32 o_resolution;
	int o_scaling;
	int o_scaler;
	uint32 o_game_resolution;
	int o_fill_scaler;
	Image_window::FillMode o_fill_mode;

	static uint32 *resolutions;
	static int num_resolutions;

	static uint32 *win_resolutions;
	static int num_win_resolutions;

	static uint32 game_resolutions[3];
	static int num_game_resolutions;

	static Image_window::FillMode startup_fill_mode;

	enum button_ids {
		id_first = 0,
		id_apply = id_first,
		id_fullscreen,
		id_share_settings,
		id_resolution,	// id_resolution and all past it
		id_scaler,		// are deleted by rebuild_buttons
		id_scaling,
		id_game_resolution,
		id_fill_scaler,
		id_fill_mode,
		id_has_ac,
		id_count
	};
	Gump_button* buttons[id_count];

 public:
	VideoOptions_gump();
	virtual ~VideoOptions_gump();
	static VideoOptions_gump *get_instance()
		{ return video_options_gump; }

					// Paint it and its contents.
	virtual void paint();
	virtual void close();

 					// Handle events:
	virtual bool mouse_down(int mx, int my, int button);
	virtual bool mouse_up(int mx, int my, int button);

	void toggle(Gump_button* btn, int state);
	void rebuild_buttons();
	void rebuild_dynamic_buttons();

	void load_settings(bool Fullscreen);
	void save_settings();
	void cancel();

	void set_scaling(int scaleVal)
	{ scaling = scaleVal; }
	void set_scaler(int scalerNum)
	{ scaler = scalerNum; }
	void set_resolution(uint32 Res)
	{ resolution = Res; }
	void set_game_resolution(uint32 gRes)
	{ game_resolution = gRes; }
	void set_fill_scaler(int f_scaler)
	{ fill_scaler = f_scaler; }
	void set_fill_mode(Image_window::FillMode f_mode)
	{ fill_mode = f_mode; }
};

#endif
