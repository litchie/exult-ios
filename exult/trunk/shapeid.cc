/*
Copyright (C) 2000-2001 The Exult Team

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

#include "gamewin.h"
#include "shapeid.h"
#include "vgafile.h"

Shape_frame *ShapeID::cache_shape()
{
	Game_window *gwin = Game_window::get_game_window();
	has_trans = 0;

	switch (shapefile) {
	case SF_SHAPES_VGA:
		shape = gwin->get_shape(shapenum, framenum);
		has_trans = gwin->get_shapes().get_info(shapenum).has_translucency();
		break;
	case SF_GUMPS_VGA:
		shape = gwin->get_gump_shape (shapenum, framenum);
		break;
	case SF_PAPERDOL_VGA:
		shape = gwin->get_paperdoll_shape (shapenum, framenum);
		break;
	case SF_SPRITES_VGA:
		shape = gwin->get_sprite_shape (shapenum, framenum);
		break;
	case SF_FACES_VGA:
		shape = gwin->get_face (shapenum, framenum);
		break;
	case SF_EXULT_FLX:
		shape = gwin->get_exult_shape(shapenum, framenum);
		break;
	case SF_GAME_FLX:
		shape = gwin->get_gameflx_shape (shapenum, framenum);
		break;
	case SF_BG_SIGUMP_FLX:
		shape = gwin->get_bg_sigump_shape(shapenum, framenum);
		break;
	default:
		std::cerr << "Error! Wrong ShapeFile!" << std::endl;
		return 0;
	} 

	return shape;

}
