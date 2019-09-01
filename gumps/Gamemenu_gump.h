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

#ifndef GAMEMENU_GUMP_H
#define GAMEMENU_GUMP_H

#include "Modal_gump.h"

class Gump_button;

class Gamemenu_gump : public Modal_gump {
	UNREPLICATABLE_CLASS(Gamemenu_gump)

private:
	Gump_button *buttons[6];

public:
	Gamemenu_gump();
	~Gamemenu_gump() override;

	// Paint it and its contents.
	void paint() override;
	void close() override {
		done = 1;
	}
	// Handle events:
	bool mouse_down(int mx, int my, int button) override;
	bool mouse_up(int mx, int my, int button) override;

	void quit(bool return_to_menu = false);
	void loadsave();
	void video_options();
	void audio_options();
	void gameplay_options();
	void misc_options();

	static void do_exult_menu();
};

#endif
