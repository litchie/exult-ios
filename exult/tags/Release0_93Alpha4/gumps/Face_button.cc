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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "Face_button.h"
#include "Paperdoll_gump.h"
#include "actors.h"

Face_button::Face_button(Gump *par, int px, int py, Actor *a)
	: Gump_button(par, 0, px, py), actor(a)
{
	Paperdoll_gump::Paperdoll_npc *npcinfo;

	npcinfo = Paperdoll_gump::GetCharacterInfo(a->get_shape_real());

	shapenum = npcinfo->head_shape;
	framenum = npcinfo->head_frame;

	Paperdoll_gump::Paperdoll_file npcinfofile = npcinfo->file;

	switch (npcinfofile) {
	case Paperdoll_gump::paperdoll:
		shapefile = GSF_PAPERDOL_VGA; break;
	case Paperdoll_gump::exult_flx:
		shapefile = GSF_EXULT_FLX; break;
	case Paperdoll_gump::gameflx:
		//+++++ NOT YET
		break;
	case Paperdoll_gump::shapes:
		//+++++ NOT YET
		break;
	}
}

void Face_button::double_clicked(Game_window *gwin)
{
	actor->show_inventory();
}
