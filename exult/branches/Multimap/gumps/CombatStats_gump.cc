/*
Copyright (C) 2001-2002 The Exult Team

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

#include "actors.h"
#include "game.h"
#include "gamewin.h"
#include "misc_buttons.h"
#include "CombatStats_gump.h"
#include "Paperdoll_gump.h"
#include "Gump_manager.h"


/*
 *	Statics:
 */

static const int colx = 110;
static const int coldx = 29;
static const int rowy[7] = {15, 29, 42, 73, 87, 93, 106};

/*
 *	Create stats display.
 */
CombatStats_gump::CombatStats_gump(int initx, int inity) : 
	Gump(0, initx, inity, game->get_shape("gumps/cstats/1"))
{
	set_object_area(Rectangle(0,0,0,0), 7, 95);


	party_size = gwin->get_party(party, 1);

	Gump::shapenum = game->get_shape("gumps/cstats/1") + party_size - 1;
	ShapeID::set_shape(Gump::shapenum, 0);

	int i;	// Blame MSVC
	for (i = 0; i < party_size; i++) {
		halo_btn[i] = new Halo_button(this, colx + i*coldx, rowy[4], party[i]);
		cmb_btn[i] = new Combat_mode_button(this, colx + i*coldx + 1, rowy[3],
											party[i]);
		face_btn[i] = new Face_button(this, colx + i*coldx - 13, rowy[0],
									  party[i]);
	}
	for (i = party_size; i < 9; i++) {
		halo_btn[i] = 0;
		cmb_btn[i] = 0;
		face_btn[i] = 0;
	}
}

CombatStats_gump::~CombatStats_gump()
{
	for (int i = 0; i < 9; i++) {
		delete halo_btn[i];
		delete cmb_btn[i];
		delete face_btn[i];
	}
}

/*
 *	Paint on screen.
 */

void CombatStats_gump::paint()
{
	Gump_manager* gman = gumpman;

	Gump::paint();

	// stats for all party members
	for (int i = 0; i < party_size; i++) {
		face_btn[i]->paint();

		gman->paint_num(party[i]->get_effective_prop(Actor::combat),
				  x + colx + i*coldx, y + rowy[1]);		
		gman->paint_num(party[i]->get_property(Actor::health),
				  x + colx + i*coldx, y + rowy[2]);

		halo_btn[i]->paint();
		cmb_btn[i]->paint();
	}

	// magic stats only for Avatar
  	gman->paint_num(party[0]->get_property(Actor::magic),
						x + colx, y + rowy[5]);
  	gman->paint_num(party[0]->get_property(Actor::mana),
						x + colx, y + rowy[6]);	
}

Gump_button* CombatStats_gump::on_button(int mx, int my)
{
	Gump_button *btn = Gump::on_button(mx, my);
	if (btn)
		return btn;
	for (int i = 0; i < party_size; i++) {
		if (halo_btn[i]->on_button(mx, my))
			return halo_btn[i];
		if (cmb_btn[i]->on_button(mx, my))
			return cmb_btn[i];
		if (face_btn[i]->on_button(mx, my))
			return face_btn[i];
	}
	return 0;
}
