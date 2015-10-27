/*
Copyright (C) 2001-2015 The Exult Team

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

#ifndef _IPHONEOPTIONS_GUMP_H
#define _IPHONEOPTIONS_GUMP_H

#ifdef __IPHONEOS__

#include "Modal_gump.h"
#include <string>

class Gump_button;

class iphoneOptions_gump : public Modal_gump {
	UNREPLICATABLE_CLASS_I(iphoneOptions_gump, Modal_gump(0, 0, 0, 0));

private:
	bool item_menu;
	int dpad_location;
	bool trlucent_bar;
	bool missing_button_bar;

	enum button_ids {
	    id_first = 0,
	    id_ok = id_first,
	    id_cancel,
	    id_item_menu,
	    id_dpad_location,
		id_trlucent_bar,
		id_missing_button_bar,
	    id_count
	};
	Gump_button *buttons[id_count];

public:
	iphoneOptions_gump();
	virtual ~iphoneOptions_gump();

	// Paint it and its contents.
	virtual void paint();
	virtual void close();

	// Handle events:
	virtual bool mouse_down(int mx, int my, int button);
	virtual bool mouse_up(int mx, int my, int button);

	void toggle(Gump_button *btn, int state);
	void build_buttons();

	void load_settings();
	void save_settings();
	void cancel();
};

#endif
#endif
