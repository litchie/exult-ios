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
#include "fontvga.h"
#include "fnames.h"
#include "game.h"
#include "Configuration.h"
#include "utils.h"
#include "segfile.h"
#include "u7drag.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;

Shape_manager *Shape_manager::instance = 0;

/*
 *	Singletons:
 */
Game_window *Game_singletons::gwin = 0;
Game_map *Game_singletons::gmap = 0;
Effects_manager *Game_singletons::eman = 0;
Shape_manager *Game_singletons::sman = 0;
Usecode_machine *Game_singletons::ucmachine = 0;
Game_clock *Game_singletons::gclock = 0;
Palette *Game_singletons::pal = 0;
Gump_manager *Game_singletons::gumpman = 0;

void Game_singletons::init
	(
	Game_window *g
	)
	{
	gwin = g;
	gmap = g->get_map();
	eman = g->get_effects();
	sman = Shape_manager::get_instance();
	ucmachine = g->get_usecode();
	gclock = g->get_clock();
	pal = g->get_pal();
	gumpman = g->get_gump_man();
	}

/*
 *	Create shape manager.
 */
Shape_manager::Shape_manager
	(
	) : bg_paperdolls_allowed(false), bg_multiracial_allowed(false),
	    bg_paperdolls(false), fonts(0)
	{
	assert(instance == 0);
	instance = this;
	std::string str;
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
	// Determine some colors based on the default palette
	Palette *pal = gwin->get_pal();
	pal->load(PALETTES_FLX, 0);	// could throw!
					// Get a bright green.
	special_pixels[POISON_PIXEL] = pal->find_color(4, 63, 4);
					// Get a light gray.
	special_pixels[PROTECT_PIXEL] = pal->find_color(62, 62, 55);
					// Yellow for cursed.
	special_pixels[CURSED_PIXEL] = pal->find_color(62, 62, 5);
					// Red for hit in battle.
	special_pixels[HIT_PIXEL] = pal->find_color(63, 4, 4);
	// What about charmed/cursed/paralyzed?

	shapes.init();
	shapes.read_info(GAME_BG);	// Read in shape dimensions.
	files[SF_GUMPS_VGA].load(GUMPS_VGA, PATCH_GUMPS);

	if (Game::get_game_type()==SERPENT_ISLE)
		{
		files[SF_PAPERDOL_VGA].load(PAPERDOL, PATCH_PAPERDOL);
		if (!files[SF_PAPERDOL_VGA].is_good())
			gwin->abort("Can't open 'paperdol.vga' file.");
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
				std::cout << "Found Serpent Isle 'paperdol.vga', 'gumps.vga' and 'shapes.vga'." << std::endl << "Support for 'Serpent Isle' Paperdolls and Multiracial Avatars in 'Black Gate' ENABLED." << std::endl;
				bg_paperdolls_allowed = true;
				bg_multiracial_allowed = true;
				}
			else
				std::cout << "Found Serpent Isle 'paperdol.vga', 'gumps.vga' and 'shapes.vga' but one or more as bad." << std::endl << "Support for 'Serpent Isle' Paperdolls and Multiracial Avatars in 'Black Gate' DISABLED." << std::endl;
			}
		catch (const exult_exception &e)	
			{
			std::cerr << "Exception attempting to load Serpent Isle 'paperdol.vga', 'gumps.vga' or 'shapes.vga'"<< std::endl <<
				"Do you have Serpent Isle and is the correct path set in the config for Serpent Isle?" << std::endl <<
				"Support for 'Serpent Isle' Paperdolls and Multiracial Avatars in 'Black Gate' DISABLED." << std::endl;
			}

		}
	files[SF_SPRITES_VGA].load(SPRITES_VGA, PATCH_SPRITES);
	files[SF_FACES_VGA].load(FACES_VGA, PATCH_FACES);
	files[SF_EXULT_FLX].load("<DATA>/exult.flx");
	const char* gamedata = game->get_resource("files/gameflx").str;
	std::cout << "Loading " << gamedata << "..." << std::endl;
	files[SF_GAME_FLX].load(gamedata);
	if (!fonts)
		{
		fonts = new Fonts_vga_file();
		fonts->init();
		}
					// Get translucency tables.
	std::size_t len, nxforms = sizeof(xforms)/sizeof(xforms[0]);
					// RGBA blend colors:
	static unsigned char blends[4*11] = {
			144,40,192,128, 96,40,16,128, 100,108,116,192, 
			68,132,28,128, 255,208,48,64, 28,52,255,128,
			8,68,0,128, 255,8,8,118, 255,244,248,128, 
			56,40,32,128, 228,224,214,82};
	for (int i = 0; i < nxforms; i++)
		xforms[i].set_color(blends[4*i], blends[4*i+1],
					blends[4*i+2], blends[4*i+3]);
	if (U7exists(XFORMTBL))
		{			// Read in translucency tables.
		Segment_file xf(XFORMTBL);
		for (int i = 0; i < nxforms; i++)
			{
			uint8 *data = (uint8*)xf.retrieve(i, len);
			memcpy(xforms[nxforms - 1 - i].colors, data,
						sizeof(xforms[0].colors));
			delete[] data;
			}
		}
	else				// Create algorithmically.
		{
		gwin->get_pal()->load(PALETTES_FLX, 0);
		for (int i = 0; i < nxforms; i++)
			{
			gwin->get_pal()->create_trans_table(xforms[i].r/4,
				xforms[i].g/4, xforms[i].b/4,
				xforms[i].a, xforms[i].colors);
			}
		}

	invis_xform = &xforms[nxforms - 1 - 2];   // ->entry 2.
	}

/*
 *	Reload one of the shape files (msg. from ExultStudio).
 */

void Shape_manager::reload_shapes
	(
	int dragtype			// Type from u7drag.h.
	)
	{
	switch (dragtype)
		{
	case U7_SHAPE_SHAPES:
		shapes.init();		// Reread .vga file.
		shapes.read_info(GAME_BG);	//+++++Needs work.
					// ++++Reread text?
		break;
	case U7_SHAPE_GUMPS:
		files[SF_GUMPS_VGA].load(GUMPS_VGA, PATCH_GUMPS);
		break;
	case U7_SHAPE_FONTS:
		fonts->init();
		break;
	case U7_SHAPE_FACES:
		files[SF_FACES_VGA].load(FACES_VGA, PATCH_FACES);
		break;
	case U7_SHAPE_SPRITES:
		files[SF_SPRITES_VGA].load(SPRITES_VGA, PATCH_SPRITES);
		break;
	case U7_SHAPE_PAPERDOL:
		files[SF_PAPERDOL_VGA].load(PAPERDOL, PATCH_PAPERDOL);
		break;
	default:
		cerr << "Type not supported:  " << dragtype << endl;
		break;
		}
	}

/*
 *	Clean up.
 */
Shape_manager::~Shape_manager()
	{
	delete fonts;
#if 0	/* +++No longer needed. */
	int nxforms = sizeof(xforms)/sizeof(xforms[0]);
	for (int i = 0; i < nxforms; i++)
		delete [] xforms[nxforms - 1 - i];
#endif
	assert(this == instance);
	instance = 0;
	}

/*
 *	Text-drawing methods:
 */
int Shape_manager::paint_text_box(int fontnum, const char *text, 
		int x, int y, int w, int h, int vert_lead, int pbreak, 
								int shading)
	{
	if(shading>=0)
		gwin->get_win()->fill_translucent8(
				0, w, h, x, y, xforms[shading]);
	return fonts->paint_text_box(gwin->get_win()->get_ib8(),
			fontnum, text, x, y, w, h, vert_lead, pbreak); 
	}
int Shape_manager::paint_text(int fontnum, const char *text, 
							int xoff, int yoff)
	{
	return fonts->paint_text(gwin->get_win()->get_ib8(), fontnum, text,
							xoff, yoff); 
	}
int Shape_manager::paint_text(int fontnum, const char *text, int textlen, 
							int xoff, int yoff)
	{
	return fonts->paint_text(gwin->get_win()->get_ib8(), fontnum, 
						text, textlen, xoff, yoff);
	}

int Shape_manager::get_text_width(int fontnum, const char *text)
	{ return fonts->get_text_width(fontnum, text); }
int Shape_manager::get_text_width(int fontnum, const char *text, int textlen)
	{ return fonts->get_text_width(fontnum, text, textlen); }
int Shape_manager::get_text_height(int fontnum)
	{ return fonts->get_text_height(fontnum); }
int Shape_manager::get_text_baseline(int fontnum)
	{ return fonts->get_text_baseline(fontnum); }

Font *Shape_manager::get_font(int fontnum)
	{ return fonts->get_font(fontnum); }


/*
 *	Read in shape.
 */
Shape_frame *ShapeID::cache_shape()
{
	if (framenum == -1) return 0;

	if (has_trans != 2) has_trans = 0;
	if (!shapefile)
		{			// Special case.
		shape = sman->shapes.get_shape(shapenum, framenum);
		if (has_trans != 2) 
			has_trans = 
			    sman->shapes.get_info(shapenum).has_translucency();
		}
	else if (shapefile < SF_OTHER)
		{
		shape = sman->files[(int) shapefile].get_shape(
							shapenum, framenum);
		if (shapefile == SF_SPRITES_VGA)
			has_trans = 1;
		}
	else
		{
		std::cerr << "Error! Wrong ShapeFile!" << std::endl;
		return 0;
		} 
	return shape;

}

int ShapeID::get_num_frames() const
{
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



