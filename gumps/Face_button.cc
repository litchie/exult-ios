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
#include "gamewin.h"

Face_button::Face_button(Gump *par, int px, int py, Actor *a)
	: Gump_button(par, 0, px, py), actor(a)
{
	Paperdoll_gump::Paperdoll_npc *npcinfo =
		Paperdoll_gump::GetCharacterInfo(a->get_shapenum());

	if (!npcinfo) npcinfo = Paperdoll_gump::GetCharacterInfoSafe(a->get_sexed_coloured_shape());
	if (!npcinfo) npcinfo = Paperdoll_gump::GetCharacterInfoSafe(a->get_shape_real());

	set_shape(npcinfo->head_shape);
	set_frame(npcinfo->head_frame);
	set_file(npcinfo->file);
}


void Face_button::double_clicked(int x, int y)
{
	actor->show_inventory();
}

void Face_button::update_widget()
{
	Paperdoll_gump::Paperdoll_npc *npcinfo =
		Paperdoll_gump::GetCharacterInfo(actor->get_shapenum());

	if (!npcinfo) npcinfo = Paperdoll_gump::GetCharacterInfoSafe(actor->get_sexed_coloured_shape());
	if (!npcinfo) npcinfo = Paperdoll_gump::GetCharacterInfoSafe(actor->get_shape_real());

	if (get_shapenum() != npcinfo->head_shape ||
		get_framenum() != npcinfo->head_frame ||
		get_shapefile() != npcinfo->file)
	{
		gwin->add_dirty(get_rect());
		set_shape(npcinfo->head_shape);
		set_frame(npcinfo->head_frame);
		set_file(npcinfo->file);
		gwin->add_dirty(get_rect());
	}

}
