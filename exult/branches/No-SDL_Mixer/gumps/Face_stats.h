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

#ifndef PARTY_PORTRAIT_H
#define PARTY_PORTRAIT_H

#include "Configuration.h"
#include "Gump.h"

class Actor;
class Portrait_button;

class Face_stats : public Gump
{
	// Only allow for one to be made
	static Face_stats	*self;
	static int		mode;
	Face_stats();

	int		party_size;
	Portrait_button	*party[8];
	int		npc_nums[8];

	int		resx;
	int		resy;

	bool		has_changed();
	void		create_buttons();
	void		delete_buttons();

	Rectangle	region;

public:
	virtual ~Face_stats();
						// Is a given point on a button?
	virtual Gump_button *on_button(int mx, int my);
	virtual void paint();
					// Don't close on end_gump_mode
	virtual bool is_persistent() const { return true; }
					// Can't be dragged with mouse
	virtual bool is_draggable() const { return false; }
					// Show the hand cursor
	virtual bool no_handcursor() const { return true; }

	virtual Rectangle get_rect() { return region; }
	virtual bool has_point(int x, int y);

					// add dirty region, if dirty
	virtual void update_gump();

	virtual int add(Game_object *obj, int mx = -1, int my = -1,
			int sx = -1, int sy = -1, bool dont_check = false,
						bool combine = false);

	virtual Container_game_object *find_actor(int mx, int my);

	static int get_state() {
		return self ? mode : -1;
	}
	static void CreateGump();
	static void RemoveGump();
	static void AdvanceState();
	static void save_config(Configuration *config);
	static void load_config(Configuration *config);
};


#endif //PARTY_PORTRAIT
