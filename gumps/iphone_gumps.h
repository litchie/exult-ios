/*
Copyright (C) 2011-2013 The Exult Team

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

#ifndef IPHONE_GUMPS_H
#define IPHONE_GUMPS_H

#ifdef __IPHONEOS__

#include "gamewin.h"
#include "../objs/objs.h"

#include "Modal_gump.h"
#include <string>

class Gump_button;
using Gump_button_vector = std::vector<Gump_button *>;
using Game_object_map_xy = std::map<Game_object *, int *>;

enum ITEMMENU_ACTIONS { ITEMMENU_ACTION_NONE, ITEMMENU_ACTION_MENU, ITEMMENU_ACTION_USE, ITEMMENU_ACTION_PICKUP, ITEMMENU_ACTION_MOVE, ITEMMENU_ACTION_COUNT };
class Itemmenu_gump : public Modal_gump {
	UNREPLICATABLE_CLASS(Itemmenu_gump)
public:
	Gump_button_vector buttons;
	Game_object_map_xy objects;
	Game_object *objectSelected;
	int objectSelectedClickXY[2];
	int objectAction;

	Itemmenu_gump(Game_object_map_xy *mobjxy, int cx, int cy);
	Itemmenu_gump(Game_object *obj, int ox, int oy, int cx, int cy);
	~Itemmenu_gump() override;

	// Paint it and its contents.
	void paint() override;
	void close() override {
		done = 1;
	}
	// Handle events:
	bool mouse_down(int mx, int my, int button) override;
	bool mouse_up(int mx, int my, int button) override;

	void postCloseActions();

};
#endif
#endif
