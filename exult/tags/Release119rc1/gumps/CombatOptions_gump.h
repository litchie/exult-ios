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

#ifndef _COMBATOPTIONS_GUMP_H
#define _COMBATOPTIONS_GUMP_H

#include "Modal_gump.h"
#include <string>

class Gump_button;

class CombatOptions_gump : public Modal_gump
{
	UNREPLICATABLE_CLASS_I(CombatOptions_gump,Modal_gump(0,0,0,0));

 private:
	Gump_button* buttons[5];	// Below settings, + ok, cancel.
	int difficulty;			// Setting for the buttons.
	int show_hits;
	int mode;
 public:
	CombatOptions_gump();
	~CombatOptions_gump();

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
