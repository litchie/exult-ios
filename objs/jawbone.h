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

#ifndef _JAWBONE_H_
#define _JAWBONE_H_

#include "contain.h"

class Jawbone_object : public Container_game_object {
	friend class Jawbone_gump;

 public:
	Jawbone_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft,
				char res = 0)
		: Container_game_object(shapenum, framenum, tilex, tiley, lft, res)
		{ }
	Jawbone_object() : Container_game_object() {  }

		//virtual ~Jawbone_object() { };

				// Add an object.
	virtual bool add(Game_object *obj, bool dont_check = false,
							bool combine = false);
				// Remove an object.
	virtual void remove(Game_object *obj);

					// Under attack. -> do nothing
	virtual Game_object *attacked(Actor *attacker, int weapon_shape = 0,
								  int ammo_shape = 0) { return this; }

 private:

	Game_object* teeth[19];
	int toothcount;
	void find_teeth();

	void update_frame();

};

#endif
