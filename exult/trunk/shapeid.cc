/*
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gamewin.h"
#include "shapeid.h"
#include "vgafile.h"
#include "fnames.h"
#include "game.h"
#include "Configuration.h"

Shape_manager *Shape_manager::instance = 0;

/*
 *	Create shape manager.
 */
Shape_manager::Shape_manager
	(
	) : bg_paperdolls_allowed(false), bg_multiracial_allowed(false),
	    bg_paperdolls(false)
	{
	assert(instance == 0);
	instance = this;
	string str;
	config->value("config/gameplay/bg_paperdolls", str, "yes");
	if (str == "yes")
		bg_paperdolls = true;
	config->set("config/gameplay/bg_paperdolls", str, true);
	}

/*
 *	Load files.
 */

void Shape_manager::load
	(
	)
	{
	shapes.init();
	shapes.read_info(GAME_BG);	// Read in shape dimensions.
	files[SF_GUMPS_VGA].load(GUMPS_VGA);

	if (Game::get_game_type()==SERPENT_ISLE)
		{
		files[SF_PAPERDOL_VGA].load(PAPERDOL);
		if (!files[SF_PAPERDOL_VGA].is_good())
			Game_window::get_instance()->abort(
					"Can't open 'paperdol.vga' file.");
		}
	else
		{
		try
			{
			files[SF_PAPERDOL_VGA].load(
					"<SERPENT_STATIC>/paperdol.vga");
			files[SF_BG_SIGUMP_FLX].load(
					"<SERPENT_STATIC>/gumps.vga");
			files[SF_BG_SISHAPES_VGA].load(
					"<SERPENT_STATIC>/shapes.vga");

			if (files[SF_PAPERDOL_VGA].is_good() && 
			    files[SF_BG_SIGUMP_FLX].is_good() && 
			    files[SF_BG_SISHAPES_VGA].is_good())
				{
				cout << "Found Serpent Isle 'paperdol.vga', 'gumps.vga' and 'shapes.vga'." << endl << "Support for 'Serpent Isle' Paperdolls and Multiracial Avatars in 'Black Gate' ENABLED." << endl;
				bg_paperdolls_allowed = true;
				bg_multiracial_allowed = true;
				}
			else
				cout << "Found Serpent Isle 'paperdol.vga', 'gumps.vga' and 'shapes.vga' but one or more as bad." << endl << "Support for 'Serpent Isle' Paperdolls and Multiracial Avatars in 'Black Gate' DISABLED." << endl;
			}
		catch (const exult_exception &e)	
			{
			cerr << "Exception attempting to load Serpent Isle 'paperdol.vga', 'gumps.vga' or 'shapes.vga'"<< endl <<
				"Do you have Serpent Isle and is the correct path set in the config for Serpent Isle?" << endl <<
				"Support for 'Serpent Isle' Paperdolls and Multiracial Avatars in 'Black Gate' DISABLED." << endl;
			}

		}
	files[SF_SPRITES_VGA].load(SPRITES_VGA);
	files[SF_FACES_VGA].load(FACES_VGA, PATCH_FACES);
	files[SF_EXULT_FLX].load("<DATA>/exult.flx");
	const char* gamedata = game->get_resource("files/gameflx").str;
	cout << "Loading " << gamedata << "..." << endl;
	files[SF_GAME_FLX].load(gamedata);
	}

/*
 *	Clean up.
 */
Shape_manager::~Shape_manager()
	{
	assert(this == instance);
	instance = 0;
	}

/*
 *	Read in shape.
 */
Shape_frame *ShapeID::cache_shape()
{
	if (framenum == -1) return 0;

	Shape_manager *sman = Shape_manager::get_instance();
	if (has_trans != 2) has_trans = 0;
	if (!shapefile)
		{			// Special case.
		shape = sman->shapes.get_shape(shapenum, framenum);
		if (has_trans != 2) 
			has_trans = 
			    sman->shapes.get_info(shapenum).has_translucency();
		}
	else if (shapefile < SF_OTHER)
		shape = sman->files[(int) shapefile].get_shape(
							shapenum, framenum);
	else
		{
		std::cerr << "Error! Wrong ShapeFile!" << std::endl;
		return 0;
		} 
	return shape;

}

int ShapeID::get_num_frames() const
{
	Shape_manager *sman = Shape_manager::get_instance();

	if (!shapefile)
		return sman->shapes.get_num_frames(shapenum);
	else if (shapefile < SF_OTHER)
		{
		if (!sman->files[(int) shapefile].is_good())
			return 0;
		return sman->files[(int) shapefile].get_num_frames(shapenum);
		}
	std::cerr << "Error! Wrong ShapeFile!" << std::endl;
	return 0;
}



