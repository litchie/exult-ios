/*
 *  Copyright (C) 2000-2004  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
#include "U7file.h"
#include "exceptions.h"

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
Party_manager *Game_singletons::partyman = 0;
class FileSystem *Game_singletons::pent_filesys = 0;

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
	partyman = g->get_party_man();
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
 *	Read in shape-file info.
 */

void Shape_manager::read_shape_info
	(
	)
	{
	// Want space for extra shapes if BG multiracial enabled.
	shapes.init(bg_multiracial_allowed ? 1036 : -1);
	shapes.read_info(Game::get_game_type());// Read in shape dimensions.
	if (GAME_SI ||			// Fixup Avatar shapes 1024-1035.
	    bg_multiracial_allowed)
		{
		// Odd are female, even are male.
		Shape_info& male = shapes.get_info(721);
		Shape_info& female = shapes.get_info(989);
		for (int i = 1024; i <= 1035; ++i)
			{
			Shape_info& info = shapes.get_info(i);
			info.copy((i%2) ? female : male);
			}
		}
	}

/*
 *	Load files.
 */

void Shape_manager::load
	(
	)
	{
	// Determine some colors based on the default palette
	Palette pal;
	pal.load(PALETTES_FLX, 0);	// could throw!
					// Get a bright green.
	special_pixels[POISON_PIXEL] = pal.find_color(4, 63, 4);
					// Get a light gray.
	special_pixels[PROTECT_PIXEL] = pal.find_color(62, 62, 55);
					// Yellow for cursed.
	special_pixels[CURSED_PIXEL] = pal.find_color(62, 62, 5);
					// Light blue for charmed.
	special_pixels[CHARMED_PIXEL] = pal.find_color(30, 40, 63);
					// Red for hit in battle.
	special_pixels[HIT_PIXEL] = pal.find_color(63, 4, 4);

	files[SF_GUMPS_VGA].load(GUMPS_VGA, PATCH_GUMPS);

	if (Game::get_game_type()==SERPENT_ISLE)
		{
		files[SF_PAPERDOL_VGA].load(PAPERDOL, PATCH_PAPERDOL);
		if (!files[SF_PAPERDOL_VGA].is_good())
			gwin->abort("Can't open 'paperdol.vga' file.");
		}
	else if (Game::get_game_type()==BLACK_GATE) // NOT for devel. games.
		{
		try
			{
			// TODO: For now, I am placing a 'paperdol.vga' file in the exult/data folder.
			// The objective would be to place the aforementioned file inside 'exult_bg.flx',
			// but I am not sure how it would be loaded here.
			files[SF_PAPERDOL_VGA].load(
					"<SERPENTISLE_STATIC>/paperdol.vga",
					U7exists(PATCH_PAPERDOL) ? PATCH_PAPERDOL : "<DATA>/bg_paperdol.vga");
			files[SF_BG_SIGUMP_FLX].load(
					"<SERPENTISLE_STATIC>/gumps.vga");
			files[SF_BG_SISHAPES_VGA].load(
					"<SERPENTISLE_STATIC>/shapes.vga");

			if (files[SF_PAPERDOL_VGA].is_good() && 
			    files[SF_BG_SIGUMP_FLX].is_good() && 
			    files[SF_BG_SISHAPES_VGA].is_good())
				{
					std::cout << "Support for SI Paperdolls and Multiracial Avatars in BG is enabled." << std::endl;
				bg_paperdolls_allowed = true;
				bg_multiracial_allowed = true;
				}
			else
				std::cout << "Bad SI 'paperdol.vga', 'gumps.vga' or 'shapes.vga'." << std::endl
						  << "Support for SI Paperdolls and Multiracial Avatars in BG is disabled." << std::endl;
			}
		catch (const exult_exception &e)	
			{
				std::cerr << "Couldn't open SI 'paperdol.vga', 'gumps.vga' or 'shapes.vga'." << std::endl
						  << "Support for SI Paperdolls and Multiracial Avatars in BG is disabled." << std::endl;
			}

		}
	files[SF_SPRITES_VGA].load(SPRITES_VGA, PATCH_SPRITES);
	files[SF_FACES_VGA].load(FACES_VGA, PATCH_FACES);
	files[SF_EXULT_FLX].load(EXULT_FLX);
	read_shape_info();
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
			std::memcpy(xforms[nxforms - 1 - i].colors, data,
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

	invis_xform = &xforms[nxforms - 1 - 0];   // ->entry 0.
	}

/*
 *	Reload one of the shape files (msg. from ExultStudio).
 */

void Shape_manager::reload_shapes
	(
	int dragtype			// Type from u7drag.h.
	)
	{
	U7FileManager::get_ptr()->reset();	// Cache no longer valid.
	switch (dragtype)
		{
	case U7_SHAPE_SHAPES:
		read_shape_info();
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
 *	Just reload info. files.
 */

void Shape_manager::reload_shape_info
	(
	)
	{
	shapes.reload_info(Game::get_game_type(),
					bg_multiracial_allowed ? 1036 : -1);
	}

/*
 *	Clean up.
 */
Shape_manager::~Shape_manager()
	{
	delete fonts;
	assert(this == instance);
	instance = 0;
	}

/*
 *	Text-drawing methods:
 */
int Shape_manager::paint_text_box(int fontnum, const char *text, 
		int x, int y, int w, int h, int vert_lead, bool pbreak, 
		bool center, int shading, Cursor_info *cursor)
	{
	if(shading>=0)
		gwin->get_win()->fill_translucent8(
				0, w, h, x, y, xforms[shading]);
	return fonts->paint_text_box(gwin->get_win()->get_ib8(),
		fontnum, text, x, y, w, h, vert_lead, pbreak, center, cursor); 
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

int Shape_manager::find_cursor(int fontnum, const char *text, int x, int y, 
				int w, int h, int cx, int cy, int vert_lead)
	{ return fonts->find_cursor(fontnum, text, x, y, w, h, cx, cy,
							vert_lead); }

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



