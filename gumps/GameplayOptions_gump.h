/*
Copyright (C) 2001-2012 The Exult Team

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

#ifndef _GAMEPLAYOPTIONS_GUMP_H
#define _GAMEPLAYOPTIONS_GUMP_H

#include "Modal_gump.h"
#include <string>

class Gump_button;

class GameplayOptions_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(GameplayOptions_gump,Modal_gump(0,0,0,0));

 private:
	int facestats;
	int fastmouse;
#ifdef UNDER_CE
	int dpadopt; 
#endif
	int mouse3rd;
	int doubleclick;
	int rightclick_close;
	int cheats;
	int paperdolls;
	int text_bg;
	int frames;
	int right_pathfind;
	int	gumps_pause;

	std::string* frametext;
	int num_framerates;
	int smooth_scrolling;

	enum button_ids {
		id_first = 0,
		id_ok = id_first,
		id_cancel,
		id_facestats,
		id_text_bg,
		id_fastmouse,
#ifdef UNDER_CE
		id_dpadopt,
#else
		id_mouse3rd,
#endif
		id_doubleclick,
		id_rightclick_close,
		id_right_pathfind,
		id_gumps_pause,
		id_cheats,
		id_frames,
		id_smooth_scrolling,
		id_paperdolls,
		id_count,
	};
	Gump_button* buttons[id_count];

 public:
	GameplayOptions_gump();
	virtual ~GameplayOptions_gump();

					// Paint it and its contents.
	virtual void paint();
	virtual void close();

 					// Handle events:
	virtual bool mouse_down(int mx, int my, int button);
	virtual bool mouse_up(int mx, int my, int button);

	void toggle(Gump_button* btn, int state);
	void build_buttons();

	void load_settings();
	void save_settings();
	void cancel();
};

#endif
