/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gamewin.cc - X-windows Ultima7 map browser.
 **
 **	Written: 7/22/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <cstring>
#  include <cstdarg>
#  include <cstdio>
#endif
#include "Astar.h"
#include "Audio.h"
#include "Configuration.h"
#include "actions.h"
#include "actors.h"
#include "animate.h"
#include "barge.h"
#include "chunks.h"
#include "dir.h"
#include "effects.h"
#include "egg.h"
#include "files/U7file.h"
#include "flic/playfli.h"
#include "fnames.h"
#include "fontvga.h"
#include "game.h"
#include "gamewin.h"
#include "items.h"
#include "mouse.h"
#include "npcnear.h"
#include "objiter.h"
#include "paths.h"
#include "schedule.h"
#include "segfile.h"
#include "spellbook.h"
#include "ucmachine.h"
#include "utils.h"
#include "virstone.h"

#include "Actor_gump.h"
#include "Paperdoll_gump.h"
#include "Spellbook_gump.h"
#include "Stats_gump.h"

#include "cheat.h"

using std::cerr;
using std::cout;
using std::endl;
using std::exit;
using std::istream;
using std::ifstream;
using std::memcpy;
using std::memset;
using std::ofstream;
using std::rand;
using std::strcmp;
using std::strcpy;
using std::string;
using std::strlen;
using std::srand;
using std::vector;

extern	Configuration *config;
					// THE game window:
Game_window *Game_window::game_window = 0;

/*
 *	Create game window.
 */

Game_window::Game_window
	(
	int width, int height, int scale, int scaler		// Window dimensions.
	) : 
	    win(0), usecode(0), mode(splash), combat(false),
            tqueue(new Time_queue()), clock(tqueue),
	    npc_prox(new Npc_proximity_handler(this)),
	    effects(0), open_gumps(0),
	    render_seq(0), painted(false), focus(true), 
	    teleported(false), in_dungeon(false), fonts(0),
	    moving_barge(0), main_actor(0), skip_above_actor(31),
	    npcs(0),
	    monster_info(0), 
	    palette(-1), brightness(100), user_brightness(100), 
	    faded_out(false), fades_enabled(true),
	    special_light(0), last_restore_hour(6),
	    dragging(0), dragging_save(0),
	    theft_warnings(0), theft_cx(255), theft_cy(255),
	    bg_paperdolls_allowed(false), bg_paperdolls(false),
	    skip_lift(16), paint_eggs(false), debug(0)
	{
	game_window = this;		// Set static ->.

	for (int i=0; i<5; i++)
		extra_fonts[i] = NULL;

	set_window_size(width, height, scale, scaler);

	}

void Game_window::set_window_size(int width, int height, int scale, int scaler)
{
	if(win) {
		delete win;
		delete pal;	
	}
	string	fullscreenstr;		// Check config. for fullscreen mode.
	bool	fullscreen=false;
	config->value("config/video/fullscreen",fullscreenstr,"no");
	if(fullscreenstr=="yes")
		fullscreen=true;
	config->set("config/video/fullscreen",fullscreenstr,true);
	win = new Image_window8(width, height, scale, fullscreen, scaler);
	win->set_title("Exult Ultima7 Engine");
	
}

void Game_window::clear_screen()
{
	win->fill8(0,get_width(),get_height(),0,0);
}



/*
 *	Deleting game window.
 */

Game_window::~Game_window
	(
	)
	{
	clear_world();			// Delete all objects, chunks.
	for (int i = 0; i < sizeof(save_names)/sizeof(save_names[0]); i++)
		delete [] save_names[i];
	int nxforms = sizeof(xforms)/sizeof(xforms[0]);
	for (int i = 0; i < nxforms; i++)
		delete [] xforms[nxforms - 1 - i];
	delete [] invis_xform;
	if(monster_info)
		delete [] monster_info;
	delete tqueue;
	delete win;
	delete dragging_save;
	delete pal;
	delete usecode;
	delete fonts;
	}

/*
 *	Abort.
 */

void Game_window::abort
	(
	const char *msg,
	...
	)
	{
	std::va_list ap;
	va_start(ap, msg);
	char buf[512];
	vsprintf(buf, msg, ap);		// Format the message.
	cerr << "Exult (fatal): " << buf << endl;
	delete this;
	exit(-1);
	}

void Game_window::init_files()
	{
	pal = new Palette();
	clock.set_palette();		// Set palette for correct time.
					// Get a bright green.
	poison_pixel = pal->find_color(4, 63, 4);
					// Get a light gray.
	protect_pixel = pal->find_color(62, 62, 55);
	usecode = new Usecode_machine(this);
					// Get custom usecode functions.
	string gametitle = Game::get_game_type() == BLACK_GATE ?
		"blackgate" : "serpentisle";
	string d = "config/disk/game/" + gametitle + "/userusecode";
	string useruc;
	config->value(d.c_str(), useruc, "");
	if (useruc != "")
		{
		ifstream file;
		const char *nm = useruc.c_str();
		cout << "Reading user usecode:  " << nm << endl;
		U7open(file, nm);
		usecode->read_usecode(file);
		file.close();
		}
	faces.load(FACES_VGA);
	gumps.load(GUMPS_VGA);
	if (!fonts)
		{
		fonts = new Fonts_vga_file();
		fonts->init();
		}
	sprites.load(SPRITES_VGA);
	mainshp.load(MAINSHP_FLX);
	shapes.init();

	U7open(chunks, U7CHUNKS);
	U7open(u7map, U7MAP);
	ifstream textflx;	
  	U7open(textflx, TEXT_FLX);
	Setup_item_names(textflx);	// Set up list of item names.
					// Read in shape dimensions.
	shapes.read_info();
	Segment_file xf(XFORMTBL);	// Read in translucency tables.
	std::size_t len, nxforms = sizeof(xforms)/sizeof(xforms[0]);
	for (int i = 0; i < nxforms; i++)
		xforms[nxforms - 1 - i] = (uint8*)xf.retrieve(i, len);
	invis_xform = (uint8*)xf.retrieve(2, len);
	unsigned long timer = SDL_GetTicks();
	srand(timer);			// Use time to seed rand. generator.
					// Force clock to start.
	tqueue->add(timer, &clock, (long) this);
					// Clear object lists, flags.
#if 1
	for (int i1 = 0; i1 < c_num_chunks; i1++)
		for (int i2 = 0; i2 < c_num_chunks; i2++)
			objects[i1][i2] = 0;
#else	/* Old way +++++++*/
	memset((char *) objects, 0, sizeof(objects));
#endif
	memset((char *) schunk_read, 0, sizeof(schunk_read));

		// Go to starting chunk
	scrolltx = game->get_start_tile_x();
	scrollty = game->get_start_tile_y();
		
	if (Game::get_game_type()==SERPENT_ISLE)
	{
		paperdolls.load(PAPERDOL);
		if (!paperdolls.is_good())
			abort("Can't open 'paperdol.vga' file.");
	}
	else
	{
		try
		{
			paperdolls.load("<SERPENT_STATIC>/paperdol.vga");
			bg_serpgumps.load("<SERPENT_STATIC>/gumps.vga");

			if (paperdolls.is_good() && bg_serpgumps.is_good())
			{
				cout << "Found Serpent Isle 'paperdol.vga' and 'gumps.vga'." << endl << "Support for 'Serpent Isle' Paperdolls in 'Black Gate' ENABLED." << endl;
				bg_paperdolls_allowed = true;
				bg_paperdolls = true;
			}
			else
				cout << "Found Serpent Isle 'paperdol.vga' and 'gumps.vga' but one was bad." << endl << "Support for 'Serpent Isle' Paperdolls in 'Black Gate' DISABLED." << endl;
		}
		catch (...)
		{
			cerr << "Exception attempting to load Serpent Isle 'paperdol.vga' or 'gumps.vga" << endl <<
				"Do you have Serpent Isle and is an absolute path set in the config for Serpent Isle?" << endl <<
				"Support for 'Serpent Isle' Paperdolls in 'Black Gate' DISABLED." << endl;
		}

	}

	}
	

Chunk_object_list *Game_window::get_objects(Game_object *obj)
{
	return get_objects(obj->get_cx(), obj->get_cy());
}

/*
 *	Set/unset barge mode.
 */

void Game_window::set_moving_barge
	(
	Barge_object *b
	)
	{
	if (b && b != moving_barge)
		b->gather();		// Gather up all objects on it.
	moving_barge = b;
	}

/*
 *	Is character moving?
 */

bool Game_window::is_moving
	(
	)
	{
	return moving_barge ? moving_barge->is_moving()
			    : main_actor->is_moving();
	}

/*
 *	Add time for a light spell.
 */

void Game_window::add_special_light
	(
	int minutes
	)
	{
	if (!special_light)		// Nothing in effect now?
		{
		special_light = clock.get_total_minutes();
		clock.set_palette();
		}
	special_light += minutes;	// Figure ending time.
	}

/* 
 *	Get monster info for a given shape.
 */

Monster_info *Game_window::get_monster_info
	(
	int shapenum
	)
	{
	for (int i = 0; i < num_monsters; i++)
		if (shapenum == monster_info[i].get_shapenum())
			return &monster_info[i];
	return (0);
	}

/*
 *	Add a 'path' egg to our list.
 */

void Game_window::add_path_egg
	(
	Egg_object *egg
	)
	{
	int qual = egg->get_quality();
	if (qual >= 0 && qual < 255)
		{
		path_eggs.reserve(qual + 1);
		path_eggs[qual] = egg;
		}
	}

/*
 *	Toggle combat mode.
 */

void Game_window::toggle_combat
	(
	)
	{ 
	combat = !combat;
					// Change party member's schedules.
	int newsched = combat ? Schedule::combat : Schedule::follow_avatar;
	int cnt = usecode->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		int party_member=usecode->get_party_member(i);
		Actor *person=get_npc(party_member);
		if (!person)
			continue;
		int sched = person->get_schedule_type();
		if (sched != newsched && sched != Schedule::wait)
			person->set_schedule_type(newsched);
		}
	if (main_actor->get_schedule_type() != newsched)
		main_actor->set_schedule_type(newsched);
	}

/*
 *	Resize event occurred.
 */

void Game_window::resized
	(
	unsigned int neww, 
	unsigned int newh,
	unsigned int newsc,
	unsigned int newsclr
	)
	{			
	win->resized(neww, newh, newsc, newsclr);
	// Do the following only if in game (not for menus)
	if(usecode) {
		center_view(get_main_actor()->get_abs_tile_coord());
		paint();
		char msg[80];
		snprintf(msg, 80, "%dx%dx%d", neww, newh, newsc);
		center_text(msg);
	}
	}

/*
 *	Create a chunk.
 */

Chunk_object_list *Game_window::create_chunk
	(
	int cx, int cy
	)
	{
	return (objects[cx][cy] = new Chunk_object_list(cx, cy));
	}

/*
 *	Set the scroll boundaries.
 */

void Game_window::set_scroll_bounds
	(
	)
	{
					// Let's try 2x2 tiles.
	scroll_bounds.w = scroll_bounds.h = 2;
	scroll_bounds.x = scrolltx + 
			(get_width()/c_tilesize - scroll_bounds.w)/2;
	scroll_bounds.y = scrollty + 
			(get_height()/c_tilesize - scroll_bounds.h)/2;
	}

/*
 *	Clear out world's contents.  Should be used during a 'restore'.
 */

void Game_window::clear_world
	(
	)
	{
//	delete tqueue;			// Want a fresh queue.
//	tqueue = new Time_queue();
	tqueue->clear();		// Remove all entries.
	clear_dirty();
					// Delete all chunks (& their objs).
	for (int y = 0; y < c_num_chunks; y++)
		for (int x = 0; x < c_num_chunks; x++)
			{
			delete objects[x][y];
			objects[x][y] = 0;
			}
	Monster_actor::delete_all();	// To be safe, del. any still around.
	Dead_body::delete_all();
	main_actor = 0;
	num_npcs = num_npcs1 = 0;
	theft_cx = theft_cy = -1;
	combat = 0;
	delete [] npcs;			// NPC's already deleted above.
	moving_barge = 0;		// Get out of barge mode.
	special_light = 0;		// Clear out light spells.
		//++++++++Clear monsters list when we have it.
					// Clear 'read' flags.
	memset((char *) schunk_read, 0, sizeof(schunk_read));
	}

/*
 *	Center view around a given tile.  This is called during a 'restore'
 *	to init. stuff as well.
 */

void Game_window::center_view
	(
	Tile_coord t
	)
	{
					// Figure in tiles.
	int tw = get_width()/c_tilesize, th = get_height()/c_tilesize;
	scrolltx = t.tx - tw/2;
	scrollty = t.ty - th/2;
	if (scrolltx < 0)
		scrolltx = 0;
	if (scrollty < 0)
		scrollty = 0;
	if (scrolltx + tw > c_num_chunks*c_tiles_per_chunk)
		scrolltx = c_num_chunks*c_tiles_per_chunk - tw - 1;
	if (scrollty + th > c_num_chunks*c_tiles_per_chunk)
		scrollty = c_num_chunks*c_tiles_per_chunk - th - 1;
	set_scroll_bounds();		// Set scroll-control.
	Barge_object *old_active_barge = moving_barge;
	read_map_data();		// This pulls in objects.
					// Found active barge?
	if (!old_active_barge && moving_barge)
		{			// Do it right.
		Barge_object *b = moving_barge;
		moving_barge = 0;
		set_moving_barge(b);
		}
					// Set where to skip rendering.
	int cx = main_actor->get_cx(), cy = main_actor->get_cy();	
	Chunk_object_list *nlist = get_objects(cx, cy);
	nlist->setup_cache();					 
	int tx = main_actor->get_tx(), ty = main_actor->get_ty();
	set_above_main_actor(nlist->is_roof (tx, ty,
						main_actor->get_lift()));
	set_in_dungeon(nlist->has_dungeon() && nlist->in_dungeon(tx, ty));
	paint();
					// See who's nearby.
	add_nearby_npcs(scrolltx/c_tiles_per_chunk, scrollty/c_tiles_per_chunk,
		(scrolltx + get_width()/c_tilesize)/c_tiles_per_chunk,
		(scrollty + get_height()/c_tilesize)/c_tiles_per_chunk);
	}

/*
 *	Scroll if necessary.
 *
 *	Output:	1 if scrolled (screen updated).
 */

int Game_window::scroll_if_needed
	(
	Tile_coord t
	)
	{
	int scrolled = 0;
					// 1 lift = 1/2 tile.
	int tx = t.tx - t.tz/2, ty = t.ty - t.tz/2;
	if (tx <= scroll_bounds.x - 1)
		{
		view_left();
		scrolled = 1;
		}
	else if (tx >= scroll_bounds.x + scroll_bounds.w)
		{
		view_right();
		scrolled = 1;
		}
	if (ty <= scroll_bounds.y - 1)
		{
		view_up();
		scrolled = 1;
		}
	else if (ty >= scroll_bounds.y + scroll_bounds.h)
		{
		view_down();
		scrolled = 1;
		}
	return (scrolled);
	}

/*
 *	Show the absolute game location, where each coordinate is of the
 *	8x8 tiles clicked on.
 */

void Game_window::show_game_location
	(
	int x, int y			// Point on screen.
	)
	{
	x = get_scrolltx() + x/c_tilesize;
	y = get_scrollty() + y/c_tilesize;
	cout << "Game location is (" << x << ", " << y << ")"<<endl;
	}


Shape_info& Game_window::get_info(const Game_object *obj)
{
	return get_info(obj->get_shapenum());
}


/*
 *	Get screen area used by object.
 */

Rectangle Game_window::get_shape_rect(const Game_object *obj)
{
	Shape_frame *s = get_shape(*obj);
	if(!s)
	{
		// This is probably fatal.
#if DEBUG
		std::cerr << "DEATH! get_shape() returned a NULL pointer: " << __FILE__ << ":" << __LINE__ << std::endl;
		std::cerr << "Betcha it's a little doggie." << std::endl;
#endif
		return Rectangle(0,0,0,0);
	}
	int tx, ty, tz;		// Get tile coords.
	obj->get_abs_tile(tx, ty, tz);
	int lftpix = 4*tz;
	return get_shape_rect(s,
		(tx + 1 - get_scrolltx())*c_tilesize - 1 - lftpix,
		(ty + 1 - get_scrollty())*c_tilesize - 1 - lftpix);
}

/*
 *	Get screen area used by a gump.
 */

Rectangle Game_window::get_gump_rect
	(
	Gump *gump
	)
{
	Shape_frame *s = get_gump_shape (gump->get_shapenum(), gump->get_framenum(), gump->is_paperdoll());
		
	return Rectangle(gump->get_x() - s->xleft, 
			gump->get_y() - s->yabove,
					s->get_width(), s->get_height());
}

/*
 *	Get screen loc. of object.
 */

void Game_window::get_shape_location(Game_object *obj, int& x, int& y)
{
	int tx, ty, tz;		// Get tile coords.
	obj->get_abs_tile(tx, ty, tz);
	int lft = 4*tz;
	x = (tx + 1 - scrolltx)*c_tilesize - 1 - lft;
	y = (ty + 1 - scrollty)*c_tilesize - 1 - lft;
}

/*
 *	Get the map objects and scenery for a superchunk.
 */

void Game_window::get_map_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	u7map.seekg(schunk * 16*16*2);	// Get to desired chunk.
	unsigned char buf[16*16*2];
	u7map.read((char*)buf, sizeof(buf));	// Read in the chunk #'s.
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
	unsigned char *mapdata = buf;
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Get chunk #.
			int chunk_num = *mapdata++;
			chunk_num += (*mapdata++) << 8;
			get_chunk_objects(scx + cx, scy + cy, chunk_num);
			}
	}

/*
 *	Read in graphics data into window's image.
 */

void Game_window::get_chunk_objects
	(
	int cx, int cy,			// Chunk index within map.
	int chunk_num			// Desired chunk # within file.
	)
	{
	chunks.seekg(chunk_num * 512);	// Get to desired chunk.
	unsigned char buf[16*16*2];	// Read in 16x16 2-byte shape #'s.
	chunks.read((char*)buf, sizeof(buf));
	unsigned char *data = &buf[0];
					// Get list we'll store into.
	Chunk_object_list *olist = get_objects(cx, cy);
					// A chunk is 16x16 tiles.
	for (int tiley = 0; tiley < c_tiles_per_chunk; tiley++)
		for (int tilex = 0; tilex < c_tiles_per_chunk; tilex++)
			{
			ShapeID id(data[0], (unsigned char) (data[1]&0x7f));
			Shape_frame *shape = get_shape(id);
			if (!shape)
				{
				cout << "Chunk shape is null!"<<endl;
				data += 2;
				continue;
				}
			if (shape->rle)
				{
				int shapenum = id.get_shapenum(),
				    framenum = id.get_framenum();
				Shape_info& info = shapes.get_info(shapenum);
				Game_object *obj = info.is_animated() ?
					new Animated_object(shapenum,
					    	framenum, tilex, tiley)
					: new Game_object(shapenum,
					    	framenum, tilex, tiley);
				olist->add(obj);
				}
			else		// Flat.
				olist->set_flat(tilex, tiley, id);
			data += 2;
			}
	}


/*
 *	Read in the objects for a superchunk from one of the "u7ifix" files.
 */

void Game_window::get_ifix_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	strcpy(fname, U7IFIX);
	int len = strlen(fname);
	fname[len] = '0' + schunk/16;
	int lb = schunk%16;
	fname[len + 1] = lb < 10 ? ('0' + lb) : ('a' + (lb - 10));
	fname[len + 2] = 0;
	ifstream ifix;			// There it is.
	U7open(ifix, fname);
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Get to index entry for chunk.
			int chunk_num = cy*16 + cx;
			ifix.seekg(0x80 + chunk_num*8);
					// Get location, length.
			long shapesoff = Read4(ifix);
			if (!shapesoff) // Nothing there?
				continue;
			unsigned long shapeslen = Read4(ifix);
			get_ifix_chunk_objects(ifix, shapesoff, shapeslen/4,
				scx + cx, scy + cy);
			}
	}

/*
 *	Get the objects from one ifix chunk entry onto the screen.
 */

void Game_window::get_ifix_chunk_objects
	(
	ifstream& ifix,
	long filepos,			// Where chunk's data lies.
	int cnt,			// # entries (objects).
	int cx, int cy			// Absolute chunk #'s.
	)
	{
	ifix.seekg(filepos);		// Get to actual shape.
					// Get buffer to hold entries' indices.
	unsigned char *entries = new unsigned char[4*cnt];
	unsigned char *ent = entries;
	ifix.read((char*)entries, 4*cnt);	// Read them in.
					// Get object list for chunk.
	Chunk_object_list *olist = get_objects(cx, cy);
	for (int i = 0; i < cnt; i++, ent += 4)
		{
		Game_object *obj = new Game_object(ent);
		olist->add(obj);
		}
	delete[] entries;		// Done with buffer.
	olist->setup_dungeon_bits();	// Should have all dungeon pieces now.
	}

/*
 *	Get the name of an ireg file.
 *
 *	Output:	->fname, where name is stored.
 */

char *Game_window::get_ireg_name
	(
	int schunk,			// Superchunk # (0-143).
	char *fname			// Name is stored here.
	)
	{
	strcpy(fname, U7IREG);
	int len = strlen(fname);
	fname[len] = '0' + schunk/16;
	int lb = schunk%16;
	fname[len + 1] = lb < 10 ? ('0' + lb) : ('a' + (lb - 10));
	fname[len + 2] = 0;
	return (fname);
	}

/*
 *	Write out one of the "u7ireg" files.
 *
 *	Output:	0 if error, which is reported.
 */

void Game_window::write_ireg_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ofstream ireg;			// There it is.
	U7open(ireg, get_ireg_name(schunk, fname));
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
			Chunk_object_list *chunk = get_objects(scx + cx,
							       scy + cy);
			Game_object *obj;
					// Restore original order (sort of).
			Object_iterator_backwards next(chunk);
			while ((obj = next.get_next()) != 0)
				obj->write_ireg(ireg);
			Write2(ireg, 0);// End with 2 0's.
			}
	ireg.flush();
	int result = ireg.good();
	if (!result)
		throw file_write_exception(fname);
	}

/*
 *	Read in the objects for a superchunk from one of the "u7ireg" files.
 *	(These are the moveable objects.)
 */

void Game_window::get_ireg_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ifstream ireg;			// There it is.
	try
	{
		U7open(ireg, get_ireg_name(schunk, fname));
	}
	catch(...)
	{
		return;			// Just don't show them.
	}
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
	read_ireg_objects(ireg, scx, scy);
	}

/*
 *	Recognize dead body shapes.  +++++++Hopefully, there's a better way.
 */

int Is_body
	(
	int shapeid
	)
	{
	switch (shapeid)
		{
	case 400:
	case 414:
	case 762:
	case 778:
	case 892:
		return 1;
	default:
		return 0;
		}
	}

/*
 *	Read a list of ireg objects.  They are either placed in the desired
 *	game chunk, or added to their container.
 */

void Game_window::read_ireg_objects
	(
	istream& ireg,			// File to read from.
	int scx, int scy,		// Abs. chunk coords. of superchunk.
	Game_object *container,		// Container, or null.
	unsigned long flags		// Usecode item flags.
	)
	{
	int entlen;			// Gets entry length.
					// Go through entries.
	while (((entlen = Read1(ireg), ireg.good())))
		{
		if (!entlen || entlen == 1)
			if (container)
				return;	// Skip 0's & ends of containers.
			else
				continue;
					// Get copy of flags.
		unsigned long oflags = flags;
		if (entlen != 6 && entlen != 12 && entlen != 18)
			{
			long pos = ireg.tellg();
			cout << "Unknown entlen " << entlen << " at pos. " <<
					pos << endl;
			ireg.seekg(pos + entlen);
			continue;	// Only know these two types.
			}
		unsigned char entry[18];// Get entry.
		ireg.read((char*)entry, entlen);
		int cx = entry[0] >> 4; // Get chunk indices within schunk.
		int cy = entry[1] >> 4;
					// Get coord. #'s where shape goes.
		int tilex = entry[0] & 0xf;
		int tiley = entry[1] & 0xf;
					// Get shape #, frame #.
		int shnum = entry[2]+256*(entry[3]&3);
		int frnum = entry[3] >> 2;
		Shape_info& info = shapes.get_info(shnum);
		unsigned int lift, quality, type;
		Ireg_game_object *obj;
		int is_egg = 0;		// Fields are eggs.
					// An "egg"?
		
		if (shnum == 486 && Game::get_game_type() == SERPENT_ISLE)
		{
			cerr << "Usecode Container. Entlen = " << entlen << endl;
		}
		if (info.get_shape_class() == Shape_info::hatchable)
			{
			int anim = info.is_animated() ||
					// Watch for BG itself.
				(Game::get_game_type() == BLACK_GATE &&
						shnum == 305);
			Egg_object *egg = create_egg(entry, anim);
			get_objects(scx + cx, scy + cy)->add_egg(egg);
			continue;
			}
		else if (entlen == 6)	// Simple entry?
			{
			type = 0;
			lift = entry[4] >> 4;
			quality = entry[5];
			obj = create_ireg_object(info, shnum, frnum,
							tilex, tiley, lift);
			is_egg = obj->is_egg();
			obj->set_low_lift (entry[4] & 0xF);
			obj->set_high_shape (entry[3] >> 7);
			if (!container && // Special case:  food.
			    shnum == 377)
				oflags &= ~(1<<Obj_flags::okay_to_take);
			}
		else if (entlen == 12)	// Container?
			{
			type = entry[4] + 256*entry[5];
			lift = entry[9] >> 4;
			quality = entry[7];
			oflags =	// Override flags (I think).
			    ((entry[11]&1) << Obj_flags::invisible) |
			    (((entry[11]>>3)&1) << Obj_flags::okay_to_take);
			if (shnum == 330)// Virtue stone?
				{
				Virtue_stone_object *v = 
				   new Virtue_stone_object(shnum, frnum, tilex,
						tiley, lift);
				v->set_pos(entry[4], entry[5], entry[6],
								entry[7]);
				obj = v;
				type = 0;
				}
			else if (shnum == 961)
				{
				Barge_object *b = new Barge_object(
				    shnum, frnum, tilex, tiley, lift,
					entry[4], entry[5],
					(quality>>1)&3);
				obj = b;
				if (!moving_barge && (quality&(1<<3)))
					moving_barge = b;
				}
			else if (quality == 1 && entry[8] >= 0x80)
				obj = new Dead_body(
				    shnum, frnum, tilex, tiley, lift,
							entry[8] - 0x80, 1);
			else if (Is_body(shnum))
				{	// Qual==2 for monsters killed.
				int decay = (quality == 2);
				if (decay &&
				    clock.get_total_hours() - last_restore_hour
								> 3)
					continue;// Body decayed.
				obj = new Dead_body(
				    shnum, frnum, tilex, tiley, lift,
								-1, decay);
				}
			else
				obj = new Container_game_object(
				    shnum, frnum, tilex, tiley, lift,
							entry[10]);
					// Read container's objects.
			if (type)	// (0 if empty.)
				{
				read_ireg_objects(ireg, scx, scy, obj, oflags);
				obj->elements_read();
				}
			}
		else			// Length 18 means it's a spellbook.
			{		// Get all 9 spell bytes.
			quality = 0;
			unsigned char circles[9];
			memcpy(&circles[0], &entry[4], 5);
			lift = entry[9] >> 4;
			memcpy(&circles[5], &entry[10], 4);
			uint8 *ptr = &entry[14];
			unsigned long flags = Read4(ptr);
			obj = new Spellbook_object(
				shnum, frnum, tilex, tiley, lift,
				&circles[0], flags);
			}
		obj->set_quality(quality);
		obj->set_flags(oflags);
					// Add, but skip volume check.
		if (!container || !container->add(obj, 1))
			{
			Chunk_object_list *chunk = get_objects(
					scx + cx, scy + cy);
			if (is_egg)
				chunk->add_egg((Egg_object *) obj);
			else
				chunk->add(obj);
			}
		}
	}

/*
 *	Create non-container IREG objects.
 */

Ireg_game_object *Game_window::create_ireg_object
	(
	Shape_info& info,		// Info. about shape.
	int shnum, int frnum,		// Shape, frame.
	int tilex, int tiley,		// Tile within chunk.
	int lift			// Desired lift.
	)
	{
	if (info.is_field())		// (These are all animated.)
		{			// Check shapes.
		if (shnum == 895)	// Fire.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::fire_field);
		else if (shnum == 900)	// Poison.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::poison_field);
		else if (shnum == 902)	// Sleep.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::sleep_field);
		}
	if (info.is_animated())
		return new Animated_ireg_object(
				   shnum, frnum, tilex, tiley, lift);
	if (shnum == 607)		// Path.
		return new Egglike_game_object(
					shnum, frnum, tilex, tiley, lift);
	else if (shnum == 330)		// +++++For fixing pre-alpha savegames.
		{			// +++++Should go away during Alpha.
		Virtue_stone_object *v = new Virtue_stone_object(shnum, frnum,
						tilex, tiley, lift);
		if (frnum >= 0 && frnum < 8)
			v->set_pos(usecode->virtue_stones[frnum]);
		return v;
		}
	else if (shnum == 761)		// Spellbook.
		{
		static unsigned char circles[9] = {0};
		return new Spellbook_object(
				shnum, frnum, tilex, tiley, lift,
				&circles[0], 0L);
		}
	else
		return new Ireg_game_object(shnum, frnum, tilex, tiley, lift);
	}

/*
 *	Create an "egg".
 */

Egg_object *Game_window::create_egg
	(
	unsigned char *entry,		// 1-byte ireg entry.
	bool animated
	)
	{
	int shnum = entry[2]+256*(entry[3]&3);
	int frnum = entry[3] >> 2;
	unsigned short type = entry[4] + 256*entry[5];
	int prob = entry[6];		// Probability (1-100).
	int data1 = entry[7] + 256*entry[8];
	int lift = entry[9] >> 4;
	int data2 = entry[10] + 256*entry[11];
	Egg_object *obj = animated ?
		new Animated_egg_object(shnum, frnum,
			entry[0]&0xf, entry[1]&0xf, lift, type, prob,
						data1, data2)
		: new Egg_object(shnum, frnum,
			entry[0]&0xf, entry[1]&0xf, lift, type, prob,
						data1, data2);
	return (obj);
	}

/*
 *	Read in the objects in a superchunk.
 */

void Game_window::get_superchunk_objects
	(
	int schunk			// Superchunk #.
	)
	{
	get_map_objects(schunk);	// Get map objects/scenery.
	get_ifix_objects(schunk);	// Get objects from ifix.
	get_ireg_objects(schunk);	// Get moveable objects.
	schunk_read[schunk] = 1;	// Done this one now.
	}

/*
 *	Put the actor(s) in the world.
 */

void Game_window::init_actors
	(
	)
	{
	if (main_actor)			// Already done?
	{
		Game::clear_avname();
		Game::clear_avsex();
		Game::clear_avskin();
		return;
	}
	read_npcs();			// Read in all U7 NPC's.

	// Was a name, sex or skincolor set in Game
	// this bascially detects 
	bool	changed = false;

	
	if (Game::get_avsex() == 0 || Game::get_avsex() == 1 || Game::get_avname()
			|| (Game::get_avskin() >= 0 && Game::get_avskin() <= 2))
		changed = true;

	Game::clear_avname();
	Game::clear_avsex();
	Game::clear_avskin();

	// Update gamedat if there was a change
	if (changed)
		{
		if (Game::get_game_type() == SERPENT_ISLE) schedule_npcs(2,7);
		write_npcs();
		}

	}
	
/*
 *	Create initial 'gamedat' directory if needed
 *
 */

bool Game_window::init_gamedat(bool create)
	{
					// Create gamedat files 1st time.
	if (create)
		{
		cout << "Creating 'gamedat' files."<<endl;
		restore_gamedat(INITGAME);

		}
	else if (!U7exists(U7NBUF_DAT) && !U7exists(NPC_DAT))
		{
		return false;
		}
	else
		{
			ifstream identity_file;
			U7open(identity_file, IDENTITY);
			char gamedat_identity[256];
			identity_file.read(gamedat_identity, 256);
			char *ptr = gamedat_identity;
			for(; (*ptr!=0x1a && *ptr!=0x0d); ptr++)
				;
			*ptr = 0;
			cout << "Gamedat identity " << gamedat_identity << endl;
			char *static_identity = Game::get_game_identity(INITGAME);
			if(strcmp(static_identity, gamedat_identity))
				{
					delete [] static_identity;
					return false;
				}
			delete [] static_identity;
			read_gwin();	// Read in 'gamewin.dat' to set clock,
					//   scroll coords.
		}
	read_save_names();		// Read in saved-game names.	
	return true;
	}


/*
 *	Save game by writing out to the 'gamedat' directory.
 *
 *	Output:	0 if error, already reported.
 */

void Game_window::write
	(
	)
	{
					// Write each superchunk to Iregxx.
	for (int schunk = 0; schunk < 12*12 - 1; schunk++)
					// Only write what we've read.
		if (schunk_read[schunk])
			write_ireg_objects(schunk);
	write_npcs();		// Write out npc.dat.
	usecode->write();	// Usecode.dat (party, global flags).
	write_gwin();		// Write our data.
	}

/*
 *	Restore game by reading in 'gamedat'.
 *
 *	Output:	0 if error, already reported.
 */

void Game_window::read
	(
	)
	{
	clear_world();			// Wipe clean.
	read_gwin();		// Read our data.
					// DON'T do anything that might paint()
					//   before calling read_npcs!!
	read_npcs();			// Read in NPC's, monsters, and get
					//   active gump.
	end_gump_mode();		// Kill gumps, and paint new data.
	
	try
	{
		usecode->read();		// Usecode.dat (party, global flags).

		if (Game::get_game_type() == SERPENT_ISLE)
			usecode->set_global_flag(Usecode_machine::did_first_scene, 1);

		if (usecode->get_global_flag(Usecode_machine::did_first_scene))
			main_actor->clear_flag(Obj_flags::dont_render);
		else
			main_actor->set_flag(Obj_flags::dont_render);
	}
	catch(...)
	{
	}
	faded_out = 0;
	clock.set_palette();		// Set palette for time-of-day.
	set_all_dirty();		// Force entire repaint.
	}

/*
 *	Write data for the game.
 *
 *	Output:	0 if error.
 */

void Game_window::write_gwin
	(
	)
	{
	ofstream gout;
	U7open(gout, GWINDAT);	// Gamewin.dat.
					// Start with scroll coords (in tiles).
	Write2(gout, get_scrolltx());
	Write2(gout, get_scrollty());
					// Write clock.
	Write2(gout, clock.get_day());
	Write2(gout, clock.get_hour());
	Write2(gout, clock.get_minute());
	Write4(gout, special_light);	// Write spell expiration minute.
	gout.flush();
	if (!gout.good())
		throw file_write_exception(GWINDAT);
	}

/*
 *	Read data for the game.
 *
 *	Output:	0 if error.
 */

void Game_window::read_gwin
	(
	)
	{
	ifstream gin;
	try
	{
		U7open(gin, GWINDAT);	// Gamewin.dat.
	} catch (const file_open_exception& e)
	{
		return;
	}
	

					// Start with scroll coords (in tiles).
	scrolltx = Read2(gin);
	scrollty = Read2(gin);
					// Read clock.
	clock.set_day(Read2(gin));
	clock.set_hour(Read2(gin));
	clock.set_minute(Read2(gin));
	last_restore_hour = clock.get_total_hours();
	if (!clock.in_queue())		// Be sure clock is running.
		tqueue->add(SDL_GetTicks(), &clock, (long) this);
	if (!gin.good())		// Next ones were added recently.
		throw file_read_exception(GWINDAT);
	special_light = Read4(gin);
	if (!gin.good())
		special_light = 0;
	}

/*
 *	Read in superchunk data to cover the screen.
 */

void Game_window::read_map_data
	(
	)
	{
	int scrolltx = get_scrolltx(), scrollty = get_scrollty();
	int w = get_width(), h = get_height();
					// Start one tile to left.
	int firstsx = (scrolltx - 1)/c_tiles_per_schunk, 
	    firstsy = (scrollty - 1)/c_tiles_per_schunk;
					// End 8 tiles to right.
	int lastsx = (scrolltx + (w + c_tilesize - 2)/c_tilesize + 
					c_tiles_per_chunk/2)/c_tiles_per_schunk;
	int lastsy = (scrollty + (h + c_tilesize - 2)/c_tilesize + 
					c_tiles_per_chunk/2)/c_tiles_per_schunk;
	if (lastsx >= 12)		// Don't go past end.
		lastsx = 11;
	if (lastsy >= 12)
		lastsy = 11;
					// Read in "map", "ifix" objects for
					//  all visible superchunks.
	for (int sy = firstsy; sy <= lastsy; sy++)
		for (int sx = firstsx; sx <= lastsx; sx++)
			{
					// Figure superchunk #.
			int schunk = 12*sy + sx;
					// Read it if necessary.
			if (!schunk_read[schunk])
				get_superchunk_objects(schunk);
			}
	}	

/*
 *	Fade the current palette in or out.
 *	Note:  If pal_num != -1, the current palette is set to it.
 */

void Game_window::fade_palette
	(
	int cycles,			// Length of fade.
	int inout,			// 1 to fade in, 0 to fade to black.
	int pal_num			// 0-11, or -1 for current.
	)
	{
	  if (pal_num == -1) pal_num = palette;
	  pal->load(PALETTES_FLX, pal_num);
	  if(inout)
		  pal->fade_in(cycles);
	  else
		  pal->fade_out(cycles);
	  faded_out = !inout;		// Be sure to set flag.
	}

/*
 *	Flash the current palette red.
 */

void Game_window::flash_palette_red
	(
	)
	{
	int savepal = palette;
	set_palette(8);			// Palette 8 is the red one.
	win->show();
	SDL_Delay(100);
	set_palette(savepal);
	painted = 1;
	}

/*
 *	Read in a palette.
 */

void Game_window::set_palette
	(
	int pal_num,			// 0-11, or -1 to leave unchanged.
	int new_brightness		// New percentage, or -1.
	)
	{
	if (palette == pal_num && brightness == new_brightness)
		return;			// Already set.
	if (pal_num != -1)
		palette = pal_num;	// Store #.
	if (new_brightness > 0)
		brightness = new_brightness;
	if (faded_out)
		return;			// In the black.
	
	pal->load(PALETTES_FLX, palette);	// could throw!
#if 0
	// FIX ME - This code is not any longer needed
	if (!pal->load(PALETTES_FLX, palette))
		abort("Error reading '%s'.", PALETTES_FLX);
#endif
	pal->set_brightness(brightness);
	pal->apply();
	}

/*
 *	Brighten/darken palette for the user.
 */

void Game_window::brighten
	(
	int per				// +- percentage to change.
	)
	{
	int new_brightness = brightness + per;
	if (new_brightness < 20)	// Have a min.
		new_brightness = 20;
	user_brightness = new_brightness;
	set_palette(palette, new_brightness);
	}

/*
 *	Shift view by one tile.
 */

void Game_window::view_right
	(
	)
	{
	if (scrolltx + get_width()/c_tilesize >= c_num_chunks*c_tiles_per_chunk - 1)
		return;
	int w = get_width(), h = get_height();
					// Get current rightmost chunk.
	int old_rcx = (scrolltx + (w - 1)/c_tilesize)/c_tiles_per_chunk;
	scrolltx++;			// Increment offset.
	scroll_bounds.x++;
	if (mode == gump)		// Gump on screen?
		{
		paint();
		return;
		}
	read_map_data();		// Be sure objects are present.
					// Shift image to left.
	win->copy(c_tilesize, 0, w - c_tilesize, h, 0, 0);
	dirty.x -= c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
					// Paint 1 column to right.
//	add_dirty(Rectangle(w - c_tilesize, 0, c_tilesize, h));
	paint(w - c_tilesize, 0, c_tilesize, h);
					// Find newly visible NPC's.
	int new_rcx = (scrolltx + (w - 1)/c_tilesize)/c_tiles_per_chunk;
	if (new_rcx != old_rcx)
		add_nearby_npcs(new_rcx, scrollty/c_tiles_per_chunk, 
			new_rcx + 1, 
		    (scrollty + (h + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk);
	}
void Game_window::view_left
	(
	)
	{
	if (scrolltx <= 0)
		return;
	scrolltx--;
	scroll_bounds.x--;
	if (mode == gump)		// Gump on screen?
		{
		paint();
		return;
		}
	read_map_data();		// Be sure objects are present.
	win->copy(0, 0, get_width() - c_tilesize, get_height(), c_tilesize, 0);
	dirty.x += c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
	int h = get_height();
//	add_dirty(Rectangle(0, 0, c_tilesize, h));
	paint(0, 0, c_tilesize, h);
					// Find newly visible NPC's.
	int new_lcx = scrolltx/c_tiles_per_chunk;
	if (new_lcx != (scrolltx + 1)/c_tiles_per_chunk)
		add_nearby_npcs(new_lcx, scrollty/c_tiles_per_chunk, 
			new_lcx + 1, 
		    (scrollty + (h + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk);
	}
void Game_window::view_down
	(
	)
	{
	if (scrollty + get_height()/c_tilesize >= c_num_chunks*c_tiles_per_chunk - 1)
		return;
	int w = get_width(), h = get_height();
					// Get current bottomost chunk.
	int old_bcy = (scrollty + (h - 1)/c_tilesize)/c_tiles_per_chunk;
	scrollty++;
	scroll_bounds.y++;
	if (mode == gump)		// Gump on screen?
		{
		paint();
		return;
		}
	read_map_data();		// Be sure objects are present.
	win->copy(0, c_tilesize, w, h - c_tilesize, 0, 0);
	dirty.y -= c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
//	add_dirty(Rectangle(0, h - c_tilesize, w, c_tilesize));
	paint(0, h - c_tilesize, w, c_tilesize);
					// Find newly visible NPC's.
	int new_bcy = (scrollty + (h - 1)/c_tilesize)/c_tiles_per_chunk;
	if (new_bcy != old_bcy)
	add_nearby_npcs(scrolltx/c_tiles_per_chunk, new_bcy, 
		    (scrolltx + (w + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk,
			new_bcy + 1);
	}
void Game_window::view_up
	(
	)
	{
	if (scrollty <= 0)
		return;
	scrollty--;
	scroll_bounds.y--;
	if (mode == gump)		// Gump on screen?
		{
		paint();
		return;
		}
	read_map_data();		// Be sure objects are present.
	int w = get_width();
	win->copy(0, 0, w, get_height() - c_tilesize, 0, c_tilesize);
	dirty.y += c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
//	add_dirty(Rectangle(0, 0, w, c_tilesize));
	paint(0, 0, w, c_tilesize);
					// Find newly visible NPC's.
	int new_tcy = scrollty/c_tiles_per_chunk;
	if (new_tcy != (scrollty + 1)/c_tiles_per_chunk)
		add_nearby_npcs(scrolltx/c_tiles_per_chunk, new_tcy,
		    (scrolltx + (w + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk,
								new_tcy + 1);
	}

/*
 *	Alternative start actor function
 *	Placed in an alternative function to prevent breaking barges
 */
void Game_window::start_actor_alt
	(
	int winx, int winy, 		// Mouse position to aim for.
	int speed			// Msecs. between frames.
	)
	{
	int ax, ay;
	int nlift;
	int blocked[8];
	get_shape_location(main_actor, ax, ay);

	int	height = shapes.get_info(main_actor->get_shapenum()).get_3d_height();
	
	Tile_coord start = main_actor->get_abs_tile_coord();
	int dir;
	
	// Check to see if we are allowed to move, if not, stop moving and return
	// Too buggy at the moment, don't do it
	//if (main_actor->get_siflag(Actor::dont_move))
	//{
	//	stop_actor();
	//	return;
	//}

	for (dir = 0; dir < 8; dir++)
	{
		Tile_coord dest = start.get_neighbor(dir);
		int cx = dest.tx/c_tiles_per_chunk, cy = dest.ty/c_tiles_per_chunk;
		int tx = dest.tx%c_tiles_per_chunk, ty = dest.ty%c_tiles_per_chunk;

		Chunk_object_list *clist = get_objects_safely(cx, cy);
		clist->setup_cache();
		blocked[dir] = clist->is_blocked (height, main_actor->get_lift(), tx, ty, nlift, main_actor->get_type_flags(), 1);
	}

	dir = Get_direction (ay - winy, winx - ax);

	if (blocked[dir] && !blocked[(dir+1)%8])
		dir = (dir+1)%8;
	else if (blocked[dir] && !blocked[(dir+7)%8])
		dir = (dir+7)%8;
	else if (blocked[dir])
	{
	   	Game_object *block = main_actor->is_moving() ? 0
			: Game_object::find_blocking(start.get_neighbor(dir));
		if (block == main_actor || !block || !block->move_aside(
						main_actor, dir))
			{
			stop_actor();
			if (main_actor->get_lift()%5)// Up on something?
				{	// See if we're stuck in the air.
				start.tz--;
				if (!Chunk_object_list::is_blocked(start, 1, 
						MOVE_WALK, 100))
					main_actor->move(start.tx, start.ty, 
								start.tz);
				}
			return;
			}
	}

	const int delta = 8*c_tilesize;	// Trying to avoid 'chicken dance'.
	switch (dir)
		{
		case north:
		//cout << "NORTH" << endl;
		ay -= delta;
		break;

		case northeast:
		//cout << "NORTH EAST" << endl;
		ay -= delta;
		ax += delta;
		break;

		case east:
		//cout << "EAST" << endl;
		ax += delta;
		break;

		case southeast:
		//cout << "SOUTH EAST" << endl;
		ay += delta;
		ax += delta;
		break;

		case south:
		//cout << "SOUTH" << endl;
		ay += delta;
		break;

		case southwest:
		//cout << "SOUTH WEST" << endl;
		ay += delta;
		ax -= delta;
		break;

		case west:
		//cout << "WEST" << endl;
		ax -= delta;
		break;

		case northwest:
		//cout << "NORTH WEST" << endl;
		ay -= delta;
		ax -= delta;
		break;
		}

	int lift = main_actor->get_lift();
	int liftpixels = 4*lift;	// Figure abs. tile.
	int tx = get_scrolltx() + (ax + liftpixels)/c_tilesize,
	    ty = get_scrollty() + (ay + liftpixels)/c_tilesize;

	main_actor->walk_to_tile(tx, ty, lift, speed, 0);
	main_actor->get_followers();
	}

/*
 *	Start the actor.
 */

void Game_window::start_actor
	(
	int winx, int winy, 		// Mouse position to aim for.
	int speed			// Msecs. between frames.
	)
	{
	if (main_actor->Actor::get_flag(Obj_flags::asleep) ||
	    main_actor->get_schedule_type() == Schedule::sleep)
		return;			// Zzzzz....
	teleported = 0;
	int lift = main_actor->get_lift();
	int liftpixels = 4*lift;	// Figure abs. tile.
	int tx = get_scrolltx() + (winx + liftpixels)/c_tilesize,
	    ty = get_scrollty() + (winy + liftpixels)/c_tilesize;
	if (moving_barge)
		{			// Want to move center there.
		Tile_coord atile = moving_barge->get_center(),
			   btile = moving_barge->get_abs_tile_coord();
					// Go faster than walking.
		moving_barge->travel_to_tile(
			Tile_coord(tx + btile.tx - atile.tx, 
				   ty + btile.ty - atile.ty, btile.tz), 
					speed/2);
		}
	else
		{
		/*
		main_actor->walk_to_tile(tx, ty, lift, speed, 0);
		main_actor->get_followers();
		*/
					// Set schedule.
		int sched = main_actor->get_schedule_type();
		if (sched != Schedule::follow_avatar &&
						sched != Schedule::combat &&
		    !main_actor->get_flag(Obj_flags::asleep))
			main_actor->set_schedule_type(Schedule::follow_avatar);
		// Going to use the alternative function for this at the moment
		start_actor_alt (winx, winy, speed);
		}
	}

/*
 *	Find path to where user double-right-clicked.
 */

void Game_window::start_actor_along_path
	(
	int winx, int winy, 		// Mouse position to aim for.
	int speed			// Msecs. between frames.
	)
	{
	if (main_actor->Actor::get_flag(Obj_flags::asleep) ||
	    main_actor->get_schedule_type() == Schedule::sleep ||
	    moving_barge)		// For now, don't do barges.
		return;			// Zzzzz....
	teleported = 0;
	int lift = main_actor->get_lift();
	int liftpixels = 4*lift;	// Figure abs. tile.
	Tile_coord dest(get_scrolltx() + (winx + liftpixels)/c_tilesize,
	    get_scrollty() + (winy + liftpixels)/c_tilesize, lift);
	if (!main_actor->walk_path_to_tile(dest, speed))
		cout << "Couldn't find path for Avatar." << endl;
	else
		main_actor->get_followers();
	}

/*
 *	Stop the actor.
 */

void Game_window::stop_actor
	(
	)
	{
	if (moving_barge)
		moving_barge->stop();
	else
		{
		main_actor->stop();	// Stop and set resting state.
		paint();	// ++++++Necessary?
		main_actor->get_followers();
		}
	}

/*
 *	Teleport the party.
 */

void Game_window::teleport_party
	(
	Tile_coord t			// Where to go.
	)
	{
	Tile_coord oldpos = main_actor->get_abs_tile_coord();
	main_actor->set_action(0);	// I think this is right.
	main_actor->move(t.tx, t.ty, t.tz);	// Move Avatar.
	paint();			// Show first.
	show();
	int cnt = usecode->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		int party_member=usecode->get_party_member(i);
		Actor *person = get_npc(party_member);
		if (person && !person->is_dead_npc())
			{
			person->set_action(0);
			Tile_coord t1(-1, -1, -1);
			for (int dist = 1; dist < 8 && t1.tx == -1; dist++)
				t1 = main_actor->find_unblocked_tile(dist, 3);
			if (t1.tx != -1)
				person->move(t1);
			}
		}
	center_view(t);			// Bring pos. into view.
	main_actor->get_followers();
					// Check all eggs around new spot.
	Chunk_object_list::try_all_eggs(main_actor, t.tx, t.ty, t.tz,
					oldpos.tx, oldpos.ty);
	teleported = 1;
	}

/*
 *	Get party members.
 *
 *	Output:	Number returned in 'list'.
 */

int Game_window::get_party
	(
	Actor **list,			// Room for 9.
	int avatar_too			// 1 to include Avatar too.
	)
	{
	int n = 0;
	if (avatar_too && main_actor)
		list[n++] = main_actor;
	int cnt = usecode->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		int party_member = usecode->get_party_member(i);
		Actor *person = get_npc(party_member);
		if (person)
			list[n++] = person;
		}
	return n;			// Return # actually stored.
	}

/*
 *	Find a given shaped item amongst the party, and 'activate' it.  This
 *	is used, for example, by the 'f' command to feed.
 */

void Game_window::activate_item
	(
	int shnum			// Desired shape.
	)
	{
	Actor *party[9];		// Get party.
	int cnt = get_party(party, 1);
	for (int i = 0; i < cnt; i++)
		{
		Actor *person = party[i];
		Game_object *obj = person->find_item(shnum, c_any_qual, c_any_framenum);
		if (obj)
			{
			Game_mode savemode = mode;
			obj->activate(usecode);
			if (mode == conversation)
				mode = savemode;
			return;
			}
		}
	}

/*
 *	Find the highest gump that the mouse cursor is on.
 *
 *	Output:	->gump, or null if none.
 */

Gump *Game_window::find_gump
	(
	int x, int y			// Pos. on screen.
	)
	{
	Gump *gmp;
	Gump *found = 0;		// We want last found in chain.
	for (gmp = open_gumps; gmp; gmp = gmp->get_next())
		{
		Rectangle box = get_gump_rect(gmp);
		if (box.has_point(x, y))
			{		// Check the shape itself.
			Shape_frame *s = get_gump_shape (gmp->get_shapenum(),
						gmp->get_framenum(),
						gmp->is_paperdoll());

			if (s->has_point(x - gmp->get_x(), y - gmp->get_y()))
				found = gmp;
			}
		}
	return (found);
	}

/*
 *	Find gump containing a given object.
 */

Gump *Game_window::find_gump
	(
	Game_object *obj
	)
	{
					// Get container object is in.
	Game_object *owner = obj->get_owner();
	if (!owner)
		return (0);
					// Look for container's gump.
	for (Gump *gmp = open_gumps; gmp; gmp = gmp->get_next())
		if (gmp->get_container() == owner)
			return (gmp);
	return (0);
	}

/*
 *	Find the top object that can be selected, dragged, or activated.
 *	The one returned is the 'highest'.
 *
 *	Output:	->object, or null if none.
 */

Game_object *Game_window::find_object
	(
	int x, int y			// Pos. on screen.
	)
	{
cout << "Clicked at tile (" << get_scrolltx() + x/c_tilesize << ", " <<
		get_scrollty() + y/c_tilesize << ")"<<endl;
	Game_object *found[100];
	int cnt = 0;
	int actor_lift = main_actor->get_lift();
//	int start = actor_lift > 0 ? -1 : 0;
					// Start on floor below actor.
	int start = actor_lift - actor_lift%5;
	int not_above = skip_lift;
	if (skip_above_actor < not_above)
		not_above = skip_above_actor;
					// See what was clicked on.
	for (int lift = start; lift < not_above; lift++)
		cnt += find_objects(lift, x, y, &found[cnt]);
	if (!cnt)
		return (0);		// Nothing found.
					// Find 'best' one.
	Game_object *obj = found[cnt - 1];
					// Try to avoid 'transparent' objs.
	int trans = shapes.get_info(obj->get_shapenum()).is_transparent();
	for (int i = 0; i < cnt - 1; i++)
		if (obj->lt(*found[i]) == 1 || trans)
			{
			int ftrans = shapes.get_info(found[i]->get_shapenum()).
							is_transparent();
			if (!ftrans || trans)
				{
				obj = found[i];
				trans = ftrans;
				}
			}
	return (obj);
	}

/*
 *	Find objects at a given position on the screen with a given lift.
 *
 *	Output: # of objects, stored in list.
 */

int Game_window::find_objects
	(
	int lift,			// Look for objs. with this lift.
	int x, int y,			// Pos. on screen.
	Game_object **list		// Objects found are stored here.
	)
	{
					// Figure chunk #'s.
	int start_cx = (get_scrolltx() + 
				(x + 4*lift)/c_tilesize)/c_tiles_per_chunk;
	int start_cy = (get_scrollty() + 
				(y + 4*lift)/c_tilesize)/c_tiles_per_chunk;
	Game_object *obj;
	int cnt = 0;			// Count # found.
					// Check 1 chunk down & right too.
	for (int ycnt = 0; ycnt < 2; ycnt++)
		{
		int cy = start_cy + ycnt;
		for (int xcnt = 0; xcnt < 2; xcnt++)
			{
			int cx = start_cx + xcnt;
			Chunk_object_list *olist = objects[cx][cy];
			if (!olist)
				continue;
			Object_iterator next(olist->get_objects());
			while ((obj = next.get_next()) != 0)
				{
				if (obj->get_lift() != lift)
					continue;
				Rectangle r = get_shape_rect(obj);
				if (!r.has_point(x, y) || 
					// Don't find invisible eggs.
						!obj->is_findable(this))
					continue;
					// Check the shape itself.
				Shape_frame *s = get_shape(*obj);
				int ox, oy;
				get_shape_location(obj, ox, oy);
				if (s->has_point(x - ox, y - oy))
					list[cnt++] = obj;
				}
			}
		}
	return (cnt);
	}

/*
 *	Show the name of the item the mouse is clicked on.
 */

void Game_window::show_items
	(
	int x, int y			// Coords. in window.
	)
	{
					// Look for obj. in open gump.
	Gump *gump = find_gump(x, y);
	Game_object *obj;		// What we find.
	if (gump)
		obj = gump->find_object(x, y);
	else				// Search rest of world.
		obj = find_object(x, y);

	// Do we want the NPC number?
	if (obj && cheat.number_npcs() && (obj->get_npc_num() > 0 || obj==main_actor))
	{
		char str[8];
		snprintf (str, 8, "%i\n", obj->get_npc_num());
		add_text(str, x, y, obj);
	}
	else if (obj)
					// Show name.
		add_text(obj->get_name().c_str(), x, y, obj);

	// If it's an actor and we want to grab the actor, grab it.
	if (obj && cheat.grabbing_actor() && (obj->get_npc_num() || obj==main_actor))
		cheat.set_grabbed_actor (static_cast<Actor *>(obj));

	//++++++++Testing
#if 1
	int shnum, frnum;
	if (obj)
		{
		shnum = obj->get_shapenum(), frnum = obj->get_framenum();
		Shape_info& info = shapes.get_info(shnum);
		cout << "Object " << shnum << ':' << frnum <<
					" has 3d tiles (x, y, z): " <<
			info.get_3d_xtiles() << ", " <<
			info.get_3d_ytiles() << ", " <<
			info.get_3d_height() << ", sched = " <<
			obj->get_schedule_type() << ", align = " <<
			obj->get_alignment()
			<< endl;
		int tx, ty, tz;
		obj->get_abs_tile(tx, ty, tz);
		cout << "tx = " << tx << ", ty = " << ty << ", tz = " <<
			tz << ", quality = " <<
			obj->get_quality() << ", low lift = " <<
			obj->get_low_lift() << ", high shape = " <<
			obj->get_high_shape () << ", okay_to_take = " <<
			(int) obj->get_flag(Obj_flags::okay_to_take) << endl;
		cout << "Volume = " << info.get_volume() << endl;
		cout << "obj = " << (void *) obj << endl;
		if (obj->get_flag(Obj_flags::asleep))
			cout << "ASLEEP" << endl;
		}
	else				// Obj==0
		{
		int tx = get_scrolltx() + x/c_tilesize;
		int ty = get_scrollty() + y/c_tilesize;
		int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
		tx = tx%c_tiles_per_chunk;
		ty = ty%c_tiles_per_chunk;
		Chunk_object_list *chunk = get_objects(cx, cy);
		ShapeID id = chunk->get_flat(tx, ty);
		shnum = id.get_shapenum();
		cout << "Clicked on flat shape " << 
			shnum << ':' << id.get_framenum() << endl;
		if (id.is_invalid())
			return;
		}
	Shape_info& info = shapes.get_info(shnum);
#if 1
	cout << "TFA[1][0-6]= " << (((int) info.get_tfa(1))&127) << endl;
	cout << "TFA[0][0-1]= " << (((int) info.get_tfa(0)&3)) << endl;
	cout << "TFA[0][3-4]= " << (((int) (info.get_tfa(0)>>3)&3)) << endl;
#endif
	if (info.is_animated())
		cout << "Object is ANIMATED" << endl;
	if (info.has_translucency())
		cout << "Object has TRANSLUCENCY" << endl;
	if (info.is_transparent())
		cout << "Object is TRANSPARENT" << endl;
	if (info.is_light_source())
		cout << "Object is LIGHT_SOURCE" << endl;
	if (info.is_door())
		cout << "Object is a DOOR" << endl;
	if (info.is_solid())
		cout << "Object is SOLID" << endl;
#endif
	}

/*
 *	Add a text object at a given spot.
 */

void Game_window::add_text
	(
	const char *msg,
	int x, int y,			// Pixel coord. on screen.
	Game_object *item		// Item text ID's, or null.
	)
	{
	if (item)			// Don't duplicate for item.
		for (Special_effect *each = effects; each; each = each->next)
			if (each->is_text(item))
				return;	// Already have text on this.

	Text_effect *txt = new Text_effect(msg, item,
		get_scrolltx() + x/c_tilesize, get_scrollty() + y/c_tilesize,
				8 + get_text_width(0, msg),
				8 + get_text_height(0));
	txt->paint(this);		// Draw it.
	painted = 1;
	add_effect(txt);
					// Show for a couple seconds.
	unsigned long curval = SDL_GetTicks();
	tqueue->add(curval + 2000, txt, (long) this);
	}

/*
 *	Add a text object in the center of the screen
 */

void Game_window::center_text
	(
	const char *msg
	)
	{
		remove_text_effects();
		add_text(msg, (get_width()-get_text_width(0,msg))/2,
			 get_height()/2);
	}

/*
 *	Add an effect at the start of the chain.
 */

void Game_window::add_effect
	(
	Special_effect *effect
	)
	{
	effect->next = effects;		// Insert into chain.
	effect->prev = 0;
	if (effect->next)
		effect->next->prev = effect;
	effects = effect;
	}

/*
 *	Remove a text item/sprite from the chain and delete it.
 *	Note:  It better not still be in the time queue.
 */

void Game_window::remove_effect
	(
	Special_effect *effect
	)
	{
	if (effect->next)
		effect->next->prev = effect->prev;
	if (effect->prev)
		effect->prev->next = effect->next;
	else				// Head of chain.
		effects = effect->next;
	delete effect;
	}

/*
 *	Remove all text items.
 */

void Game_window::remove_all_effects
	(
	)
	{
	if (!effects)
		return;
	while (effects)
		{
		tqueue->remove(effects);// Remove from time queue if there.
		remove_effect(effects);
		}
	paint();			// Just paint whole screen.
	}

/*
 *	Remove text effects.
 */

void Game_window::remove_text_effects
	(
	)
	{
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
		if (each->is_text())
			{
			tqueue->remove(each);
			remove_effect(each);
			}
		each = next;
		}
	set_all_dirty();
	}


/*
 *	Remove weather effects.
 */

void Game_window::remove_weather_effects
	(
	)
	{
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
		if (each->is_weather())
			{
			tqueue->remove(each);
			remove_effect(each);
			}
		each = next;
		}
	set_all_dirty();
	}

/*
 *	Find last numbered weather effect added.
 */

int Game_window::get_weather
	(
	)
	{
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
		if (each->is_weather())
			{
			Weather_effect *weather = (Weather_effect *) each;
			if (weather->get_num() >= 0)
				return weather->get_num();
			}
		each = next;
		}
	return 0;
	}

/*
 *	Handle a double-click.
 */

void Game_window::double_clicked
	(
	int x, int y			// Coords in window.
	)
	{
					// Look for obj. in open gump.
	Gump *gump = find_gump(x, y);
	Game_object *obj;
	if (gump)
		{			// Find object in gump.
		obj = gump->find_object(x, y);
		if (!obj)		// Maybe it's a spell.
			{
		 	Gump_button *btn = gump->on_button(this, x, y);
			if (btn)
				btn->double_clicked(this);
			return;
			}
		}
	else				// Search rest of world.
		{
		obj = find_object(x, y);
					// Check path, except if an NPC.
	    	if (obj && obj->get_npc_num() <= 0 &&
			!Fast_pathfinder_client::is_grabable(
					main_actor->get_abs_tile_coord(),
					obj->get_abs_tile_coord()))
			{
			Mouse::mouse->flash_shape(Mouse::blocked);
			return;
			}
		}
	if (obj)
		{
		if (combat && !gump && obj != main_actor &&
					// But don't attack party members.
						obj->get_party_id() < 0 &&
					// Or bodies.
						!Is_body(obj->get_shapenum()))
			{		// In combat mode.
					// Want everyone to be in combat.
			combat = 0;
			toggle_combat();
			main_actor->set_opponent(obj);
			return;
			}
		remove_text_effects();	// Remove text msgs. from screen.
		cout << "Object name is " << obj->get_name() << endl;
		usecode->init_conversation();
		Game_mode savemode = mode;
		obj->activate(usecode);
		npc_prox->wait(4);	// Delay "barking" for 4 secs.
		if (mode == conversation)
			{
			// We had a conversation with an NPC, set the met flag true (BG Only)
			if (Game::get_game_type() == BLACK_GATE && obj->get_npc_num() != -1) obj->set_flag (Obj_flags::met);
			mode = savemode;
			paint();
			}
		}
	}


/*
 *	Show a gump.
 */

void Game_window::show_gump
	(
	Game_object *obj,		// Object gump represents.
	int shapenum			// Shape # in 'gumps.vga'.
	)
	{
	int paperdoll = (shapenum >= ACTOR_FIRST_GUMP && shapenum <= ACTOR_LAST_GUMP);

	// overide for paperdolls
	if (shapenum == 123 && (Game::get_game_type() == SERPENT_ISLE ||
		(can_use_paperdolls() && get_bg_paperdolls())))
		paperdoll=2;
	else if (paperdoll && obj == main_actor)
		shapenum += main_actor->get_type_flag(Actor::tf_sex);
		
	static int cnt = 0;		// For staggering them.
	Gump *gmp;		// See if already open.
	for (gmp = open_gumps; gmp; gmp = gmp->get_next())
		if (gmp->get_owner() == obj &&
		    gmp->get_shapenum() == shapenum)
			break;
	if (gmp)			// Found it?
		{			// Move it to end.
		if (gmp->get_next())
			{
			gmp->remove_from_chain(open_gumps);
			gmp->append_to_chain(open_gumps);
			}
		paint();
		return;
		}
	int x = (1 + cnt)*get_width()/10, 
	    y = (1 + cnt)*get_height()/10;
	
    	Shape_frame *shape = paperdoll == 2 ? get_gump_shape(shapenum, 0, true) :
					get_gump_shape(shapenum, 0, false);
		
	if (x + shape->get_xright() > get_width() ||
	    y + shape->get_ybelow() > get_height())
		{
		cnt = 0;
		x = get_width()/10;
		y = get_width()/10;
		}
	Gump *new_gump = paperdoll == 2 ?
				new Paperdoll_gump(
					(Container_game_object *) obj, x, y, obj->get_npc_num())
			: paperdoll ?
				new Actor_gump(
					(Container_game_object *) obj, x, y, shapenum)
			: shapenum == game->get_shape("gumps/statsdisplay") ?
				new Stats_gump(
					(Container_game_object *) obj, x, y)
			: shapenum == game->get_shape("gumps/spellbook") ?
				new Spellbook_gump((Spellbook_object *) obj)
			: new Gump((Container_game_object *) obj, 
							x, y, shapenum);
					// Paint new one last.
	new_gump->append_to_chain(open_gumps);
	if (++cnt == 8)
		cnt = 0;
	mode = gump;			// Special mode.
	clock.set_palette();		// Gumps get lighter palette.
	paint();			// Show everything.
	}

/*
 *	End gump mode.
 */

void Game_window::end_gump_mode
	(
	)
	{
	int had_gumps = (open_gumps != 0);
	while (open_gumps)		// Remove all gumps.
		{
		Gump *gmp = open_gumps;
		open_gumps = gmp->get_next();
		delete gmp;
		}
	mode = normal;
	clock.set_palette();
	npc_prox->wait(4);		// Delay "barking" for 4 secs.
	if (had_gumps)
		paint();
	}

/*
 *	Remove a gump.
 */

void Game_window::remove_gump
	(
	Gump *gump
	)
	{
	gump->remove_from_chain(open_gumps);
	delete gump;
	if (!open_gumps)		// Last one?  Out of gump mode.
		{
		mode = normal;
		clock.set_palette();
		}
	}

/*
 *	Add an NPC to the 'nearby' list.
 */

void Game_window::add_nearby_npc
	(
	Npc_actor *npc
	)
	{
	if (!npc->is_nearby())
		{
		npc->set_nearby();
		npc_prox->add(SDL_GetTicks(), npc);
		}
	}

/*
 *	Add NPC's in a given range of chunk to the queue for nearby NPC's.
 */

void Game_window::add_nearby_npcs
	(
	int from_cx, int from_cy,	// Starting chunk coord.
	int stop_cx, int stop_cy	// Go up to, but not including, these.
	)
	{
	unsigned long curtime = SDL_GetTicks();
	for (int cy = from_cy; cy != stop_cy; cy++)
		for (int cx = from_cx; cx != stop_cx; cx++)
			for (Npc_actor *npc = get_objects(cx, cy)->get_npcs();
						npc; npc = npc->get_next())
				if (!npc->is_nearby())
					{
					npc->set_nearby();
					npc_prox->add(curtime, npc);
					}
	}

/*
 *	Add all nearby NPC's to the given list.
 */

void Game_window::get_nearby_npcs
	(
	Actor_queue& list
	)
	{
	npc_prox->get_all(list);
	}

/*
 *	Tell all npc's to update their schedules at a new 3-hour period.
 */

void Game_window::schedule_npcs
	(
	int hour3,			// 0=midnight, 1=3am, 2=6am, etc.
	int backwards			// Extra periods to look backwards.
	)
	{
					// Go through npc's.
	for (int i = 1; i < num_npcs; i++)
		{
		Npc_actor *npc = (Npc_actor *) npcs[i];
					// Don't want companions leaving.
		if (npc->get_schedule_type() != Schedule::wait)
			npc->update_schedule(this, hour3, backwards);
		}
	paint();			// Repaint all.
	}

/*
 *	Handle theft.
 */

void Game_window::theft
	(
	)
	{
					// See if in a new location.
	int cx = main_actor->get_cx(), cy = main_actor->get_cy();
	if (cx != theft_cx || cy != theft_cy)
		{
		theft_cx = cx;
		theft_cy = cy;
		theft_warnings = 0;
		}
	Actor_vector npcs;			// See if someone is nearby.
	main_actor->find_nearby_actors(npcs, c_any_shapenum, 12);
	Actor *closest_npc = 0;
	int best_dist = 5000;
	for (Actor_vector::const_iterator it = npcs.begin(); it != npcs.end();++it)
		{
		Actor *npc = *it;
		if (npc->is_monster() || npc == main_actor ||
		    npc->get_party_id() >= 0)
			continue;
		int dist = npc->distance(main_actor);
		if (dist < best_dist && Fast_pathfinder_client::is_grabable(
			npc->get_abs_tile_coord(),
			main_actor->get_abs_tile_coord()))
			{
			closest_npc = npc;
			best_dist = dist;
			}
		}
	if (!closest_npc)
		return;			// Didn't get caught.
	theft_warnings++;
	if (theft_warnings < 3 + rand()%3)
		{			// Just a warning this time.
		closest_npc->say(first_theft, last_theft);
		return;
		}
	closest_npc->say(first_call_guards, last_call_guards);
					// Show guard running up.
	Monster_info *inf = get_monster_info(0x3b2);
	if (inf)
		{			// Create it off-screen.
		Monster_actor *guard = inf->create(
			main_actor->get_cx() + 8, 
			main_actor->get_cy() + 8, 0, 0, 0);
		add_nearby_npc(guard);
		Tile_coord actloc = main_actor->get_abs_tile_coord();
		Tile_coord dest(-1, -1, -1);
		for (int i = 2; i < 6 && dest.tx == -1; i++)
			dest = Game_object::find_unblocked_tile(actloc, i, 3);
		if (dest.tx != -1)
			{
			int dir = Get_direction(dest.ty - actloc.ty,
						actloc.tx - dest.tx);
					
			char frames[2];	// Use frame for starting attack.
			frames[0] = guard->get_dir_framenum(dir,
							Actor::standing);
			frames[1] = guard->get_dir_framenum(dir, 3);
			Actor_action *action = new Sequence_actor_action(
				new Frames_actor_action(frames, 2),
				new Usecode_actor_action(0x625, main_actor,
					Usecode_machine::double_click));
			Schedule::set_action_sequence(guard, dest, action, 1);
			}
		}
	}

/*
 *	Gain/lose focus.
 */

void Game_window::get_focus
	(
	)
	{
	cout << "Game resumed" << endl;
	focus = 1; 
	tqueue->resume(SDL_GetTicks());
	}
void Game_window::lose_focus
	(
	)
	{
	cout << "Game paused" << endl;
	focus = false; 
	tqueue->pause(SDL_GetTicks());
	}


/*
 *	Prepare for game
 */

void Game_window::setup_game
	(
	)
	{
	
	mode = normal;
	set_palette(0);

	init_actors();		// Set up actors if not already done.
				// This also sets up initial 
				// schedules and positions.

	usecode->read();		// Read the usecode flags

	if (Game::get_game_type() == BLACK_GATE)
	{
		string yn;		// Override from config. file.
					// Skip intro. scene?
		config->value("config/gameplay/skip_intro", yn, "no");
		if (yn == "yes")
			usecode->set_global_flag(Usecode_machine::did_first_scene, 1);

					// Have Trinsic password?
		config->value("config/gameplay/have_trinsic_password", yn, "no");
		if (yn == "yes")
			usecode->set_global_flag(
					Usecode_machine::have_trinsic_password, 1);
					// Should Avatar be visible?
		if (usecode->get_global_flag(Usecode_machine::did_first_scene))
			main_actor->clear_flag(Obj_flags::dont_render);
		else
			main_actor->set_flag(Obj_flags::dont_render);
	}
	else
		usecode->set_global_flag(Usecode_machine::did_first_scene, 1);

	faded_out = 0;
	clock.set_palette();		// Set palette for time-of-day.
	set_all_dirty();		// Force entire repaint.

	Audio::get_ptr()->cancel_raw();
	Audio::get_ptr()->cancel_streams();
				// Want to activate first egg.
	Chunk_object_list *olist = get_objects(
			main_actor->get_cx(), main_actor->get_cy());
	olist->setup_cache();
	Tile_coord t = main_actor->get_abs_tile_coord();
	olist->activate_eggs(main_actor, t.tx, t.ty, t.tz, -1, -1);
	paint();
	}

/*
 *	Text-drawing methods:
 */
int Game_window::paint_text_box(int fontnum, const char *text, 
		int x, int y, int w, int h, int vert_lead, int pbreak)
	{ return fonts->paint_text_box(win->get_ib8(),
			fontnum, text, x, y, w, h, vert_lead, pbreak); }
int Game_window::paint_text(int fontnum, const char *text, int xoff, int yoff)
	{ return fonts->paint_text(win->get_ib8(), fontnum, text,
							xoff, yoff); }
int Game_window::paint_text(int fontnum, const char *text, int textlen, 
							int xoff, int yoff)
	{ return fonts->paint_text(win->get_ib8(), fontnum, text, textlen,
							xoff, yoff); }
	
int Game_window::get_text_width(int fontnum, const char *text)
	{ return fonts->get_text_width(fontnum, text); }
int Game_window::get_text_width(int fontnum, const char *text, int textlen)
	{ return fonts->get_text_width(fontnum, text, textlen); }
int Game_window::get_text_height(int fontnum)
	{ return fonts->get_text_height(fontnum); }
int Game_window::get_text_baseline(int fontnum)
	{ return fonts->get_text_baseline(fontnum); }

Font *Game_window::get_font(int fontnum)
	{ return fonts->get_font(fontnum); }



void Game_window::plasma(int w, int h, int x, int y, int startc, int endc)
{
	Image_buffer8 *ibuf = Game_window::get_game_window()->
				get_win()->get_ib8();

	ibuf->fill8(startc, w, h, x, y);

	for (int i=0; i < w*h*8; i++) {
		Uint8 pc = startc + rand()%(endc-startc+1);
		int px = x + rand()%w;
		int py = y + rand()%h;

		for (int j=0; j < 6; j++) {
			int px2 = px + rand()%17 - 8;
			int py2 = py + rand()%17 - 8;
			ibuf->fill8(pc, 3, 1, px2 - 1, py2);
			ibuf->fill8(pc, 1, 3, px2, py2 - 1);
		}
	}
}

