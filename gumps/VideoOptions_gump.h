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

#ifndef _VIDEOOPTIONS_GUMP_H
#define _VIDEOOPTIONS_GUMP_H

#include "Modal_gump.h"
#include <string>

class Gump_button;

class VideoOptions_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(VideoOptions_gump,Modal_gump(0,0,0,0));

 private:
	Gump_button* buttons[10];

	int resolution;
	int scaling;
	int scaler;
	int fullscreen;

	int old_resolution;

	std::string *restext;
	int num_resolutions;

 public:
	VideoOptions_gump();
	~VideoOptions_gump();

					// Paint it and its contents.
	virtual void paint();
	virtual void close();

 					// Handle events:
	virtual void mouse_down(int mx, int my);
	virtual void mouse_up(int mx, int my);

	void toggle(Gump_button* btn, int state);
	void build_buttons();

	void load_settings();
	void save_settings();
	void cancel();
};

#endif
