/**
 **	Gamewin.cc - X-windows Ultima7 map browser.
 **
 **	Written: 7/22/98 - JSF
 **/

/*
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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

#ifndef ALPHA_LINUX_CXX
#  include <cstdlib>
#  include <cstring>
#  include <cstdarg>
#  include <cstdio>
#endif

#include "Astar.h"
#include "Audio.h"
#include "Configuration.h"
#include "Face_stats.h"
#include "Flex.h"
#include "Gump.h"
#include "Gump_manager.h"
#include "actions.h"
#include "actors.h"
#include "animate.h"
#include "barge.h"
#include "bodies.h"
#include "cheat.h"
#include "chunks.h"
#include "chunkter.h"
#include "delobjs.h"
#include "dir.h"
#include "effects.h"
#include "egg.h"
#include "exult.h"
#include "files/U7file.h"
#include "flic/playfli.h"
#include "fnames.h"
#include "fontvga.h"
#include "game.h"
#include "gamewin.h"
#include "items.h"
#include "jawbone.h"
#include "keys.h"
#include "mouse.h"
#include "npcnear.h"
#include "objiter.h"
#include "paths.h"
#include "schedule.h"
#include "segfile.h"
#include "spellbook.h"
#include "ucmachine.h"
#include "ucsched.h"			/* Only used to flush objects. */
#include "utils.h"
#include "virstone.h"

using std::cerr;
using std::cout;
using std::endl;
using std::istream;
using std::ifstream;
using std::ios;
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
using std::snprintf;

					// THE game window:
Game_window *Game_window::game_window = 0;

/*
 *	Provide chirping birds.
 */
class Background_noise : public Time_sensitive
	{
	int repeats;			// Repeats in quick succession.
	int last_sound;			// # of last sound played.
	Game_window *gwin;
public:
	Background_noise(Game_window *gw) : repeats(0), last_sound(-1),
					gwin(gw)
		{ gwin->get_tqueue()->add(5000, this, 0L); }
	virtual ~Background_noise()
		{ gwin->get_tqueue()->remove(this); }
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Play background sound.
 */

void Background_noise::handle_event
	(
	unsigned long curtime,
	long udata
	)
	{
	Main_actor *ava = gwin->get_main_actor();
	unsigned long delay = 8000;
					// Only if outside.
	if (ava && !gwin->is_main_actor_inside() &&
					// +++++SI SFX's don't sound right.
	    Game::get_game_type() == BLACK_GATE)
		{
		int sound;		// BG SFX #.
		static unsigned char bgnight[] = {61, 103, 110},
				     bgday[] = {82, 85, 85};
		if (repeats > 0)	// Repeating?
			sound = last_sound;
		else
			{
			int hour = gwin->get_hour();
			if (hour < 6 || hour > 20)
				sound = bgnight[rand()%sizeof(bgnight)];
			else
				sound = bgday[rand()%sizeof(bgday)];
					// Translate BG to SI #'s.
			sound = Audio::game_sfx(sound);
			last_sound = sound;
			}
		Audio::get_ptr()->play_sound_effect(sound);
		repeats++;		// Count it.
		if (rand()%(repeats + 1) == 0)
					// Repeat.
			delay = 500 + rand()%1000;
		else
			{
			delay = 4000 + rand()%3000;
			repeats = 0;
			}
		}
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Create game window.
 */

Game_window::Game_window
	(
	int width, int height, int scale, int scaler		// Window dimensions.
	) : 
	    win(0), usecode(0), combat(false),
            tqueue(new Time_queue()), clock(tqueue), time_stopped(0),
	    npc_prox(new Npc_proximity_handler(this)),
	    effects(0), gump_man(new Gump_manager),
	    render_seq(0), painted(false), focus(true), 
	    teleported(false), in_dungeon(0), fonts(0),
	    moving_barge(0), main_actor(0), skip_above_actor(31),
	    npcs(0), bodies(0),
	    chunk_terrains(0), num_chunk_terrains(0),
	    palette(-1), brightness(100), user_brightness(100), 
	    faded_out(false), fades_enabled(true),
	    special_light(0), last_restore_hour(6),
	    dragging(0), dragging_save(0),
	    theft_warnings(0), theft_cx(255), theft_cy(255),
	    background_noise(new Background_noise(this)),
	    bg_paperdolls_allowed(false), bg_paperdolls(false),
	    removed(new Deleted_objects()), 
	    skip_lift(16), paint_eggs(false), debug(0), camera_actor(0)
#ifdef RED_PLASMA
	    ,load_palette_timer(0), plasma_start_color(0), plasma_cycle_range(0)
#endif
	{
	game_window = this;		// Set static ->.

	for (int i=0; i<5; i++)
		extra_fonts[i] = NULL;

	set_window_size(width, height, scale, scaler);
	pal = new Palette();
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

void Game_window::clear_screen(bool update)
{
	win->fill8(0,get_width(),get_height(),0,0);

	// update screen
	if (update)
		show(1);
}



/*
 *	Deleting game window.
 */

Game_window::~Game_window
	(
	)
	{
    gump_man->close_all_gumps(true);
	clear_world();			// Delete all objects, chunks.
	int i;	// Blame MSVC
	for (i = 0; i < sizeof(save_names)/sizeof(save_names[0]); i++)
		delete [] save_names[i];
	int nxforms = sizeof(xforms)/sizeof(xforms[0]);
	for (i = 0; i < nxforms; i++)
		delete [] xforms[nxforms - 1 - i];
	delete [] invis_xform;
	delete gump_man;
	delete background_noise;
	delete tqueue;
	delete win;
	delete dragging_save;
	delete pal;
	delete usecode;
	delete removed;
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
	throw quit_exception(-1);
	}

void Game_window::init_files(bool cycle)
{
	
	// Determine some colors based on the default palette
	set_palette(0);
					// Get a bright green.
	poison_pixel = pal->find_color(4, 63, 4);
					// Get a light gray.
	protect_pixel = pal->find_color(62, 62, 55);
					// Red for hit in battle.
	hit_pixel = pal->find_color(63, 4, 4);
	// What about charmed/cursed/paralyzed?

#ifdef RED_PLASMA
	// Display red plasma during load...
	if (cycle)
		setup_load_palette();
#endif

	usecode = Usecode_machine::create(this);
	cout << "Loading exult.flx..." << endl;
	exult_flx.load("<DATA>/exult.flx");

	const char* gamedata = game->get_resource("files/gameflx").str;
	cout << "Loading " << gamedata << "..." << endl;
	gameflx.load(gamedata);
	CYCLE_RED_PLASMA();
	faces.load(FACES_VGA);
	CYCLE_RED_PLASMA();
	gumps.load(GUMPS_VGA);
	CYCLE_RED_PLASMA();
	if (!fonts)
	{
		fonts = new Fonts_vga_file();
		fonts->init();
	}
	sprites.load(SPRITES_VGA);
	CYCLE_RED_PLASMA();
	mainshp.load(MAINSHP_FLX);
	CYCLE_RED_PLASMA();
	shapes.init();

	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_U7CHUNKS))
		U7open(chunks, PATCH_U7CHUNKS);
	else
		U7open(chunks, U7CHUNKS);
	chunks.seekg(0, ios::end);	// Get to end so we can get length.
					// 2 bytes/tile.
	num_chunk_terrains = chunks.tellg()/(c_tiles_per_chunk*2);
	std::ifstream u7map;		// Read in map.
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_U7MAP))
		U7open(u7map, PATCH_U7MAP);
	else
		U7open(u7map, U7MAP);
	for (int schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
	{			// Read in the chunk #'s.
		unsigned char buf[16*16*2];
		u7map.read((char*)buf, sizeof(buf));
		int scy = 16*(schunk/12);// Get abs. chunk coords.
		int scx = 16*(schunk%12);
		uint8 *mapdata = buf;
					// Go through chunks.
		for (int cy = 0; cy < 16; cy++)
			for (int cx = 0; cx < 16; cx++)
				terrain_map[scx+cx][scy+cy] = Read2(mapdata);
		CYCLE_RED_PLASMA();
	}
	u7map.close();
	ifstream textflx;	
  	U7open(textflx, TEXT_FLX);
	Setup_item_names(textflx);	// Set up list of item names.
					// Read in shape dimensions.
	shapes.read_info();
	Segment_file xf(XFORMTBL);	// Read in translucency tables.
	std::size_t len, nxforms = sizeof(xforms)/sizeof(xforms[0]);
	for (int i = 0; i < nxforms; i++)
	{
		xforms[nxforms - 1 - i] = (uint8*)xf.retrieve(i, len);
		CYCLE_RED_PLASMA();
	}
	invis_xform = (uint8*)xf.retrieve(2, len);
	unsigned long timer = SDL_GetTicks();
	srand(timer);			// Use time to seed rand. generator.
					// Force clock to start.
	tqueue->add(timer, &clock, (long) this);

					// Clear object lists, flags.
	memset((char *) objects, 0, sizeof(objects));
	memset((char *) schunk_read, 0, sizeof(schunk_read));
	memset((char *) schunk_modified, 0, sizeof(schunk_modified));

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
		catch (const exult_exception &e)
		{
			cerr << "Exception attempting to load Serpent Isle 'paperdol.vga' or 'gumps.vga" << endl <<
				"Do you have Serpent Isle and is the correcct path set in the config for Serpent Isle?" << endl <<
				"Support for 'Serpent Isle' Paperdolls in 'Black Gate' DISABLED." << endl;
		}

	}


	// initialize keybinder
	if (keybinder)
		delete keybinder;
	keybinder = new KeyBinder();

	std::string d, keyfilename;
	d = "config/disk/game/"+Game::get_gametitle()+"/keys";
	config->value(d.c_str(),keyfilename,"(default)");
	if (keyfilename == "(default)") {
	  config->set(d.c_str(), keyfilename, true);
	  keybinder->LoadDefaults();
	} else {
	  keybinder->LoadFromFile(keyfilename.c_str());
	}

	CYCLE_RED_PLASMA();
	// initialize .wav SFX pack
	Audio::get_ptr()->Init_sfx();
}
	

Map_chunk *Game_window::get_chunk(Game_object *obj)
{
	return get_chunk(obj->get_cx(), obj->get_cy());
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
	else if (!b && moving_barge)
		moving_barge->done();	// No longer 'barging'.
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
 *  Are we in dont_move mode?
 */

bool Game_window::main_actor_dont_move()
    { 
    return main_actor->get_siflag(Actor::dont_move)
            || main_actor->get_flag(Obj_flags::dont_render);
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
 *	Set 'stop time' value.
 */

void Game_window::set_time_stopped
	(
	long delay			// Delay in ticks (1/1000 secs.),
					//   -1 to stop indefinitely, or 0
					//   to end.
	)
	{
	if (delay == -1)
		time_stopped = -1;
	else if (!delay)
		time_stopped = 0;
	else
		{
		long new_expire = SDL_GetTicks() + delay;
		if (new_expire > time_stopped)	// Set expiration time.
			time_stopped = new_expire;
		}
	}

/*
 *	Return delay to expiration (or 3000 if indefinite).
 */

long Game_window::check_time_stopped
	(
	)
	{
	if (time_stopped == -1)
		return 3000;
	long delay = time_stopped - SDL_GetTicks();
	if (delay > 0)
		return delay;
	time_stopped = 0;		// Done.
	return 0;
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
		path_eggs.put(qual, egg);
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
		if (sched != newsched && sched != Schedule::wait &&
		    sched != Schedule::loiter)
			person->set_schedule_type(newsched);
		}
	if (main_actor->get_schedule_type() != newsched)
		main_actor->set_schedule_type(newsched);
	if (combat)			// Get rid of flee modes.
		{
		Actor *all[9];
		int cnt = get_party(all, 1);
		for (int i = 0; i < cnt; i++)
			{		// Did Usecode set to flee?
			Actor *act = all[i];
			if (act->get_attack_mode() == Actor::flee &&
			    !act->did_user_set_attack())
				act->set_attack_mode(Actor::nearest);
					// And avoid attacking party members,
					//  in case of Usecode bug.
			Game_object *targ = act->get_target();
			if (targ &&
			    (targ == main_actor || targ->get_party_id() >= 0))
				act->set_target(0);
			}
		}
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

Map_chunk *Game_window::create_chunk
	(
	int cx, int cy
	)
	{
	return (objects[cx][cy] = new Map_chunk(cx, cy));
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
	tqueue->clear();		// Remove all entries.
	clear_dirty();
	removed->flush();		// Delete.
//+++++Enable this when saving/restoring of sched. usecode is written:
//	Schedule_usecode::clear();	// Clear out all scheduled usecode.
					// Delete all chunks (& their objs).
	for (int y = 0; y < c_num_chunks; y++)
		for (int x = 0; x < c_num_chunks; x++)
			{
			delete objects[x][y];
			objects[x][y] = 0;
			}
	int cnt = chunk_terrains.size();
	for (int i = 0; i < cnt; i++)
		delete chunk_terrains[i];
	chunk_terrains.resize(0);
	Monster_actor::delete_all();	// To be safe, del. any still around.
	main_actor = 0;
	camera_actor = 0;
	num_npcs1 = 0;
	theft_cx = theft_cy = -1;
	combat = 0;
	npcs.resize(0);			// NPC's already deleted above.
	bodies.resize(0);
	moving_barge = 0;		// Get out of barge mode.
	special_light = 0;		// Clear out light spells.
					// Clear 'read' flags.
	memset((char *) schunk_read, 0, sizeof(schunk_read));
	memset((char *) schunk_modified, 0, sizeof(schunk_modified));
	}

/*
 *	Set the scroll position so that a given tile is centered.  (Used by
 *	center_view.)
 */

void Game_window::set_scrolls
	(
	Tile_coord cent			// Want center here.
	)
	{
					// Figure in tiles.
	int tw = get_width()/c_tilesize, th = get_height()/c_tilesize;
	scrolltx = cent.tx - tw/2;
	scrollty = cent.ty - th/2;
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
	int cx = camera_actor->get_cx(), cy = camera_actor->get_cy();	
	Map_chunk *nlist = get_chunk(cx, cy);
	nlist->setup_cache();					 
	int tx = camera_actor->get_tx(), ty = camera_actor->get_ty();
	set_above_main_actor(nlist->is_roof (tx, ty,
						camera_actor->get_lift()));
	set_in_dungeon(nlist->has_dungeon()?nlist->is_dungeon(tx, ty):0);
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
	set_scrolls(t);
	set_all_dirty();
					// See who's nearby.
	add_nearby_npcs(scrolltx/c_tiles_per_chunk, scrollty/c_tiles_per_chunk,
		(scrolltx + get_width()/c_tilesize)/c_tiles_per_chunk,
		(scrollty + get_height()/c_tilesize)/c_tiles_per_chunk);
	}

/*
 *	Set actor to center view around.
 */

void Game_window::set_camera_actor
	(
	Actor *a
	)
	{
	camera_actor = a;
	Tile_coord t = a->get_abs_tile_coord();
	set_scrolls(t);			// Set scrolling around position,
					//   and read in map there.
	}

/*
 *	Scroll if necessary.
 *
 *	Output:	1 if scrolled (screen updated).
 */

bool Game_window::scroll_if_needed
	(
	Tile_coord t
	)
	{
	bool scrolled = false;
					// 1 lift = 1/2 tile.
	int tx = t.tx - t.tz/2, ty = t.ty - t.tz/2;
	if (Tile_coord::gte(DECR_TILE(scroll_bounds.x), tx))
		{
		view_left();
		scrolled = true;
		}
	else if (Tile_coord::gte(tx, 
			(scroll_bounds.x + scroll_bounds.w)%c_num_tiles))
		{
		view_right();
		scrolled = true;
		}
	if (Tile_coord::gte(DECR_TILE(scroll_bounds.y), ty))
		{
		view_up();
		scrolled = true;
		}
	else if (Tile_coord::gte(ty, 
			(scroll_bounds.y + scroll_bounds.h)%c_num_tiles))
		{
		view_down();
		scrolled = true;
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
	tx += 1 - get_scrolltx();
	ty += 1 - get_scrollty();
				// Watch for wrapping.
	if (tx < -c_num_tiles/2)
		tx += c_num_tiles;
	if (ty < -c_num_tiles/2)
		ty += c_num_tiles;
	return get_shape_rect(s,
		tx*c_tilesize - 1 - lftpix,
		ty*c_tilesize - 1 - lftpix);
}


/*
 *	Get screen loc. of object.
 */

void Game_window::get_shape_location(Game_object *obj, int& x, int& y)
{
	int tx, ty, tz;		// Get tile coords.
	obj->get_abs_tile(tx, ty, tz);
	int lft = 4*tz;
	tx += 1 - get_scrolltx();
	ty += 1 - get_scrollty();
				// Watch for wrapping.
	if (tx < -c_num_tiles/2)
		tx += c_num_tiles;
	if (ty < -c_num_tiles/2)
		ty += c_num_tiles;
	x = tx*c_tilesize - 1 - lft;
	y = ty*c_tilesize - 1 - lft;
}

/*
 *	Get the map objects and scenery for a superchunk.
 */

void Game_window::get_map_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			get_chunk_objects(scx + cx, scy + cy);
	}

/*
 *	Read in terrain graphics data into window's image.  (May also be
 *	called during map-editing if the chunknum changes.)
 */

void Game_window::get_chunk_objects
	(
	int cx, int cy			// Chunk index within map.
	)
	{
					// Get list we'll store into.
	Map_chunk *chunk = get_chunk(cx, cy);
	int chunk_num = terrain_map[cx][cy];
					// Already have this one?
	Chunk_terrain *ter = chunk_num < chunk_terrains.size() ?
					chunk_terrains[chunk_num] : 0;
	if (!ter)			// No?  Got to read it.
		{			// Read in 16x16 2-byte shape #'s.
		chunks.seekg(chunk_num * 512);
		unsigned char buf[16*16*2];	
		chunks.read((char*)buf, sizeof(buf));
		ter = new Chunk_terrain(&buf[0]);
		chunk_terrains.put(chunk_num, ter);
		}
	chunk->set_terrain(ter);
	}

/*
 *	Set a chunk to a new terrain (during map-editing).
 */

void Game_window::set_chunk_terrain
	(
	int cx, int cy,			// Coords. of chunk to change.
	int chunknum			// New chunk #.
	)
	{
	terrain_map[cx][cy] = chunknum;	// Set map.
	get_chunk_objects(cx, cy);	// Set chunk to it.
	}

/*
 *	Get the name of an ireg or ifix file.
 *
 *	Output:	->fname, where name is stored.
 */

char *Game_window::get_schunk_file_name
	(
	char *prefix,			// "ireg" or "ifix".
	int schunk,			// Superchunk # (0-143).
	char *fname			// Name is stored here.
	)
	{
	strcpy(fname, prefix);
	int len = strlen(fname);
	fname[len] = '0' + schunk/16;
	int lb = schunk%16;
	fname[len + 1] = lb < 10 ? ('0' + lb) : ('a' + (lb - 10));
	fname[len + 2] = 0;
	return (fname);
	}

/*
 *	Write out one of the "u7ifix" files.
 *
 *	Output:	Errors reported.
 */

void Game_window::write_ifix_objects
	(
	int schunk			// Superchunk # (0-143).
	)
	{
	char fname[128];		// Set up name.
	ofstream ifix;			// There it is.
	U7open(ifix, get_schunk_file_name(PATCH_U7IFIX, schunk, fname));
					// +++++Use game title.
	const int count = c_chunks_per_schunk*c_chunks_per_schunk;
	Flex::write_header(ifix, "Exult",  count);
	uint8 table[2*count*4];
	uint8 *tptr = &table[0];
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
					// Store file position in table.
			long start = ifix.tellp();
			Write4(tptr, start);
			Map_chunk *chunk = get_chunk(scx + cx,
							       scy + cy);
					// Restore original order (sort of).
			Object_iterator_backwards next(chunk);
			Game_object *obj;
			while ((obj = next.get_next()) != 0)
				obj->write_ifix(ifix);
					// Store IFIX data length.
			Write4(tptr, ifix.tellp() - start);
			}
	ifix.seekp(0x80, std::ios::beg);	// Write table.
	ifix.write((char*) &table[0], sizeof(table));
	ifix.flush();
	int result = ifix.good();
	if (!result)
		throw file_write_exception(fname);
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
	ifstream ifix;			// There it is.
	if (is_system_path_defined("<PATCH>") &&
					// First check for patch.
	    U7exists(get_schunk_file_name(PATCH_U7IFIX, schunk, fname)))
		U7open(ifix, fname);
	else
		U7open(ifix, get_schunk_file_name(U7IFIX, schunk, fname));
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
	Map_chunk *olist = get_chunk(cx, cy);
	for (int i = 0; i < cnt; i++, ent += 4)
		{
		Ifix_game_object *obj = new Ifix_game_object(ent);
		olist->add(obj);
		}
	delete[] entries;		// Done with buffer.
	olist->setup_dungeon_levels();	// Should have all dungeon pieces now.
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
	U7open(ireg, get_schunk_file_name(U7IREG, schunk, fname));
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
					// Go through chunks.
	for (int cy = 0; cy < 16; cy++)
		for (int cx = 0; cx < 16; cx++)
			{
			Map_chunk *chunk = get_chunk(scx + cx,
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
		U7open(ireg, get_schunk_file_name(U7IREG, schunk, fname));
	}
	catch(const file_exception & f)
	{
		return;			// Just don't show them.
	}
	int scy = 16*(schunk/12);	// Get abs. chunk coords.
	int scx = 16*(schunk%12);
	read_ireg_objects(ireg, scx, scy);
					// A fixup:
	if (schunk == 10*12 + 11 && Game::get_game_type() == SERPENT_ISLE)
		{			// Lever in SilverSeed:
		Game_object_vector vec;
		if (Game_object::find_nearby(vec, Tile_coord(2936, 2726, 0),
					787, 0, 0, c_any_qual, 5))
			vec[0]->move(2937, 2727, 2);
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
	sint8 index_id = -1;
					// Go through entries.
	while (((entlen = Read1(ireg), ireg.good())))
		{

		// Skip 0's & ends of containers.

		if (!entlen || entlen == 1)
		{
			if (container)
				return;	// Skip 0's & ends of containers.
			else
				continue;
		}
		// Detect the 2 byte index id
		else if (entlen == 2)
		{
			index_id = (sint8) Read2 (ireg);
			continue;
		}

					// Get copy of flags.
		unsigned long oflags = flags & ~(1<<Obj_flags::is_temporary);
		if (entlen != 6 && entlen != 10 && entlen != 12 && entlen != 18)
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

		// Has flag byte(s)
		if (entlen == 10)
		{
			// Temporary
			if (entry[6] & 1) oflags |= 1<<Obj_flags::is_temporary;
		}
		
		if (info.get_shape_class() == Shape_info::hatchable)
			{
			bool anim = info.is_animated() ||
					// Watch for BG itself.
				(Game::get_game_type() == BLACK_GATE &&
						shnum == 305);
			Egg_object *egg = create_egg(entry, anim);
			get_chunk(scx + cx, scy + cy)->add_egg(egg);
			continue;
			}
		else if (entlen == 6 || entlen == 10)	// Simple entry?
			{
			type = 0;
			lift = entry[4] >> 4;
			quality = entry[5];
			obj = create_ireg_object(info, shnum, frnum,
							tilex, tiley, lift);
			is_egg = obj->is_egg();
			obj->set_low_lift (entry[4] & 0xF);
			obj->set_high_shape (entry[3] >> 7);

#if 0	/* Causes too much trouble. */
			if (!container && // Special case:  food.
			    shnum == 377)
				oflags &= ~(1<<Obj_flags::okay_to_take);
#endif
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
			else if (Game::get_game_type() == SERPENT_ISLE &&
				shnum == 555) // serpent jawbone
				{
				obj = new Jawbone_object(shnum, frnum,
					tilex, tiley, lift, entry[10]);
				}
			else if (Game::get_game_type() == SERPENT_ISLE && 
					 shnum == 400 && frnum == 8 && quality == 1)
				// Gwenno. Ugly hack to fix bug without having to start
				// a new game. Remove someday... (added 20010820)
				{
					obj = new Dead_body(400, 8, tilex, tiley, lift, 149);
					if (bodies.size() <= 149) bodies.resize(150);
					bodies[149] = obj;
				}
			else if (quality == 1 && 
					 (entry[8] >= 0x80 || 
					  Game::get_game_type() == SERPENT_ISLE)) 
				{		// NPC's body.
					int npc_num = (entry[8] - 0x80) & 0xFF;
					obj = new Dead_body(shnum, frnum, tilex, tiley, lift,
										npc_num);
					if (bodies.size() <= npc_num) bodies.resize(npc_num+1);
					bodies[npc_num] = obj;
				}
			else if (Is_body(shnum)) {
				obj = new Dead_body(
				    shnum, frnum, tilex, tiley, lift, -1);
			}
			else
				obj = new Container_game_object(
				    shnum, frnum, tilex, tiley, lift,
							entry[10]);
					// Read container's objects.
			if (type)	// (0 if empty.)
				{	// Don't pass along invisibility!
				read_ireg_objects(ireg, scx, scy, obj, 
					oflags & ~(1<<Obj_flags::invisible));
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

		if (container)
		{
			if (index_id != -1 && container->add_readied(obj, index_id, 1, 1))
				continue;
			else if (container->add(obj, 1))
				continue;
		}

		Map_chunk *chunk = get_chunk(
				scx + cx, scy + cy);
		if (is_egg)
			chunk->add_egg((Egg_object *) obj);
		else
			chunk->add(obj);
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
		if (shnum == 895 ||	// Fire.
		    shnum == 561)	// SI - blue flame.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::fire_field);
		else if (shnum == 900)	// Poison.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::poison_field);
		else if (shnum == 902)	// Sleep.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::sleep_field);
		else if (shnum == 756)	// Caltrops.
			return new Field_object(shnum, frnum, tilex, tiley,
					lift, Egg_object::caltrops_field);
		}
	if (info.is_animated())
		return new Animated_ireg_object(
				   shnum, frnum, tilex, tiley, lift);
	if (shnum == 607)		// Path.
		return new Egglike_game_object(
					shnum, frnum, tilex, tiley, lift);
#if 0
	else if (shnum == 330)		// +++++For fixing pre-alpha savegames.
		{			// +++++Should go away during Alpha.
		Virtue_stone_object *v = new Virtue_stone_object(shnum, frnum,
						tilex, tiley, lift);
		if (frnum >= 0 && frnum < 8)
			v->set_pos(usecode->virtue_stones[frnum]);
		return v;
		}
#endif
	else if (shnum == 761)		// Spellbook.
		{
		static unsigned char circles[9] = {0};
		return new Spellbook_object(
				shnum, frnum, tilex, tiley, lift,
				&circles[0], 0L);
		}
	else if (info.get_shape_class() == Shape_info::container)
		return new Container_game_object(shnum, frnum, tilex, tiley,
									lift);
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
	CYCLE_RED_PLASMA();
	get_map_objects(schunk);	// Get map objects/scenery.
	CYCLE_RED_PLASMA();
	get_ifix_objects(schunk);	// Get objects from ifix.
	CYCLE_RED_PLASMA();
	get_ireg_objects(schunk);	// Get moveable objects.
	CYCLE_RED_PLASMA();
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
		schedule_npcs(2,7);
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
		Game::set_new_game();
		if (is_system_path_defined("<PATCH>") && 
						U7exists(PATCH_INITGAME))
			restore_gamedat(PATCH_INITGAME);
		else
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
	write_saveinfo();
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
	Audio::get_ptr()->cancel_streams();
#ifdef RED_PLASMA
	// Display red plasma during load...
	setup_load_palette();
#endif

	clear_world();			// Wipe clean.
	read_gwin();			// Read our data.
					// DON'T do anything that might paint()
					//   before calling read_npcs!!
	setup_game();			// Read NPC's, usecode.
	gump_man->close_all_gumps(true);		// Kill gumps.
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
 *	Write out map data (IFIXxx, U7CHUNKS, U7MAP) to static, and also
 *	save 'gamedat' to <PATCH>/initgame.dat.
 *
 *	Note:  This is for map-editing.
 *
 *	Output:	Errors are reported.
 */

void Game_window::write_map
	(
	)
	{
	U7mkdir(PATCHDAT, 0755);	// Create dir if not already there.
	int schunk;			// Write each superchunk to 'static'.
	for (schunk = 0; schunk < 12*12 - 1; schunk++)
					// Only write what we've modified.
		if (schunk_modified[schunk])
			write_ifix_objects(schunk);
#if 0	/* +++++Finish later. Don't delete this! */
	ofstream ochunks;		// Open file for chunks data.
	U7open(ochunks, PATCH_U7CHUNKS);
	int cnt = chunk_terrains.size();
	for (int i = 0; i < cnt; i++)
			// +++Won't work.  Write out as a FLEX file??
		{			// Write modified ones.
		Chunk_terrain *ter = chunk_terrains[i];
		if (ter && ter->is_modified())
			{
			ochunks.seekp(i*512);
			unsigned char data[512];
			ter->write_flats(data);
			ochunks.write((char*)data, 512);
			}
		}
	if (!ochunks.good())
		throw file_write_exception(U7CHUNKS);
	ochunks.close();
#endif
	std::ofstream u7map;		// Write out map.
	U7open(u7map, PATCH_U7MAP);
	for (schunk = 0; schunk < c_num_schunks*c_num_schunks; schunk++)
		{
		int scy = 16*(schunk/12);// Get abs. chunk coords.
		int scx = 16*(schunk%12);
		uint8 buf[16*16*2];
		uint8 *mapdata = buf;
					// Go through chunks.
		for (int cy = 0; cy < 16; cy++)
			for (int cx = 0; cx < 16; cx++)
				Write2(mapdata, terrain_map[scx+cx][scy+cy]);
		u7map.write((char*)buf, sizeof(buf));
		}
	if (!u7map.good())
		throw file_write_exception(U7MAP);
	u7map.close();
	write();			// Write out to 'gamedat' too.
	save_gamedat(PATCH_INITGAME, "Saved map");
	}

/*
 *	Reinitialize game from map.
 */

void Game_window::read_map
	(
	)
	{
	init_gamedat(true);		// Unpack 'initgame.dat'.
	read();				// This does the whole restore.
	}

/*
 *	Reload (patched) usecode.
 */

void Game_window::reload_usecode
	(
	)
	{
					// Get custom usecode functions.
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_USECODE))
		{
		ifstream file;
		U7open(file, PATCH_USECODE);
		usecode->read_usecode(file);
		file.close();
		}
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
	int new_brightness,		// New percentage, or -1.
	bool repaint
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
	pal->apply(repaint);
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
	int w = get_width(), h = get_height();
					// Get current rightmost chunk.
	int old_rcx = ((scrolltx + (w - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks;
	scrolltx = INCR_TILE(scrolltx);
	scroll_bounds.x = INCR_TILE(scroll_bounds.x);
	if (gump_man->showing_gumps())		// Gump on screen?
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
	int new_rcx = ((scrolltx + (w - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks;
	if (new_rcx != old_rcx)
		add_nearby_npcs(new_rcx, scrollty/c_tiles_per_chunk, 
			INCR_CHUNK(new_rcx),
	((scrollty + (h + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks);
	}
void Game_window::view_left
	(
	)
	{
	int old_lcx = (scrolltx/c_tiles_per_chunk)%c_num_chunks;
					// Want to wrap.
	scrolltx = DECR_TILE(scrolltx);
	scroll_bounds.x = DECR_TILE(scroll_bounds.x);
	if (gump_man->showing_gumps())			// Gump on screen?
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
	int new_lcx = (scrolltx/c_tiles_per_chunk)%c_num_chunks;
	if (new_lcx != old_lcx)
		add_nearby_npcs(new_lcx, scrollty/c_tiles_per_chunk, 
			INCR_CHUNK(new_lcx), 
	((scrollty + (h + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks);
	}
void Game_window::view_down
	(
	)
	{
	int w = get_width(), h = get_height();
					// Get current bottomost chunk.
	int old_bcy = ((scrollty + (h - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks;
	scrollty = INCR_TILE(scrollty);
	scroll_bounds.y = INCR_TILE(scroll_bounds.y);
	if (gump_man->showing_gumps())			// Gump on screen?
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
	int new_bcy = ((scrollty + (h - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks;
	if (new_bcy != old_bcy)
	add_nearby_npcs(scrolltx/c_tiles_per_chunk, new_bcy, 
	    ((scrolltx + (w + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks,
			INCR_CHUNK(new_bcy));
	}
void Game_window::view_up
	(
	)
	{
	int old_tcy = (scrollty/c_tiles_per_chunk)%c_num_chunks;
					// Want to wrap.
	scrollty = DECR_TILE(scrollty);
	scroll_bounds.y = DECR_TILE(scroll_bounds.y);
	if (gump_man->showing_gumps())		// Gump on screen?
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
	int new_tcy = (scrollty/c_tiles_per_chunk)%c_num_chunks;
	if (new_tcy != old_tcy)
		add_nearby_npcs(scrolltx/c_tiles_per_chunk, new_tcy,
	    ((scrolltx + (w + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks,
							INCR_CHUNK(new_tcy));
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
	
	for (dir = 0; dir < 8; dir++)
	{
		Tile_coord dest = start.get_neighbor(dir);
		int cx = dest.tx/c_tiles_per_chunk, cy = dest.ty/c_tiles_per_chunk;
		int tx = dest.tx%c_tiles_per_chunk, ty = dest.ty%c_tiles_per_chunk;

		Map_chunk *clist = get_chunk_safely(cx, cy);
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
				if (!Map_chunk::is_blocked(start, 1, 
						MOVE_WALK, 100))
					main_actor->move(start.tx, start.ty, 
								start.tz);
				}
			return;
			}
	}

	const int delta = 8*c_tilesize;// Bigger # here avoids jerkiness,
					//   but causes probs. with followers.
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
					// Wrap:
	tx = (tx + c_num_tiles)%c_num_tiles;
	ty = (ty + c_num_tiles)%c_num_tiles;

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
	if (main_actor_dont_move())
		{
//		stop_actor();  Causes problems in animations.
		return;
		}

	teleported = 0;
	if (moving_barge)
		{			// Want to move center there.
		int lift = main_actor->get_lift();
		int liftpixels = 4*lift;	// Figure abs. tile.
		int tx = get_scrolltx() + (winx + liftpixels)/c_tilesize,
		    ty = get_scrollty() + (winy + liftpixels)/c_tilesize;
					// Wrap:
		tx = (tx + c_num_tiles)%c_num_tiles;
		ty = (ty + c_num_tiles)%c_num_tiles;
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
					// Animation in progress?
	if (main_actor_dont_move())
		return;
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
		if (person && !person->is_dead())
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
	Map_chunk::try_all_eggs(main_actor, t.tx, t.ty, t.tz,
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
	int shnum,			// Desired shape.
	int frnum,			// Desired frame
	int qual			// Desired quality
	)
	{
	Actor *party[9];		// Get party.
	int cnt = get_party(party, 1);
	for (int i = 0; i < cnt; i++)
		{
		Actor *person = party[i];
		Game_object *obj = person->find_item(shnum, qual, frnum);
		if (obj)
			{
			obj->activate(usecode);
			return;
			}
		}
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
#ifdef DEBUG
cout << "Clicked at tile (" << get_scrolltx() + x/c_tilesize << ", " <<
		get_scrollty() + y/c_tilesize << ")"<<endl;
#endif
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
	int start_cx = ((get_scrolltx() + 
		(x + 4*lift)/c_tilesize)/c_tiles_per_chunk)%c_num_chunks;
	int start_cy = ((get_scrollty() + 
		(y + 4*lift)/c_tilesize)/c_tiles_per_chunk)%c_num_chunks;
	Game_object *obj;
	int cnt = 0;			// Count # found.
					// Check 1 chunk down & right too.
	for (int ycnt = 0; ycnt < 2; ycnt++)
		{
		int cy = (start_cy + ycnt)%c_num_chunks;
		for (int xcnt = 0; xcnt < 2; xcnt++)
			{
			int cx = (start_cx + xcnt)%c_num_chunks;
			Map_chunk *olist = objects[cx][cy];
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
	Gump *gump = gump_man->find_gump(x, y);
	Game_object *obj;		// What we find.
	if (gump)
	{
		obj = gump->find_object(x, y);
		if (!obj) obj = gump->find_actor(x, y);
		if (!obj) obj = gump->get_container();
	}
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
#ifdef DEBUG
	int shnum, frnum;
	if (obj)
		{
		shnum = obj->get_shapenum(), frnum = obj->get_framenum();
		Shape_info& info = shapes.get_info(shnum);
		cout << "Object " << shnum << ':' << frnum <<
					" has 3d tiles (x, y, z): " <<
			info.get_3d_xtiles(frnum) << ", " <<
			info.get_3d_ytiles(frnum) << ", " <<
			info.get_3d_height() << ", sched = " <<
			obj->get_schedule_type() << ", align = " <<
			obj->get_alignment() << ", npcnum = " <<
			obj->get_npc_num()
			<< endl;
		int tx, ty, tz;
		obj->get_abs_tile(tx, ty, tz);
		cout << "tx = " << tx << ", ty = " << ty << ", tz = " <<
			tz << ", quality = " <<
			obj->get_quality() << ", low lift = " <<
			obj->get_low_lift() << ", high shape = " <<
			obj->get_high_shape () << ", okay_to_take = " <<
			(int) obj->get_flag(Obj_flags::okay_to_take) <<
			", flag0x1d = " << (int) obj->get_flag(0x1d)
			<< endl;
		cout << "obj = " << (void *) obj << endl;
		if (obj->get_flag(Obj_flags::asleep))
			cout << "ASLEEP" << endl;
		if (dynamic_cast<Ireg_game_object*> (obj))
			cout << "IREG object" << endl;
		else if (dynamic_cast<Ifix_game_object*> (obj))
			cout << "IFIX object" << endl;
		else
			cout << "TERRAIN object" << endl;
		if (obj->is_egg())	// Show egg info. here.
			((Egg_object *)obj)->print_debug();
		}
	else				// Obj==0
		{
		int tx = (get_scrolltx() + x/c_tilesize)%c_num_tiles;
		int ty = (get_scrollty() + y/c_tilesize)%c_num_tiles;
		int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
		tx = tx%c_tiles_per_chunk;
		ty = ty%c_tiles_per_chunk;
		Map_chunk *chunk = get_chunk(cx, cy);
		ShapeID id = chunk->get_flat(tx, ty);
		shnum = id.get_shapenum();
		cout << "Clicked on flat shape " << 
			shnum << ':' << id.get_framenum() << endl;

#ifdef CHUNK_OBJ_DUMP
		Object_iterator it(chunk->get_objects());
		Game_object *each;
		cout << "Chunk Contents: " << endl;
		while ((each = it.get_next()) != 0)
			cout << "    " << each->get_name() << ":" << each->get_shapenum() << ":" << each->get_framenum() << endl;
#endif
		if (id.is_invalid())
			return;
		}
	Shape_info& info = shapes.get_info(shnum);
	cout << "TFA[1][0-6]= " << (((int) info.get_tfa(1))&127) << endl;
	cout << "TFA[0][0-1]= " << (((int) info.get_tfa(0)&3)) << endl;
	cout << "TFA[0][3-4]= " << (((int) (info.get_tfa(0)>>3)&3)) << endl;
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
 *	Remove an item from the world and set it for later deletion.
 */

void Game_window::delete_object
	(
	Game_object *obj
	)
	{
#if 0
		// don't do this, since this function is only called from
		// Game_object::remove_this (and inherited's)

	obj->remove_this(1);		// Remove from world or container, but
								//   don't delete.
#endif

	obj->set_invalid();		// Set to invalid chunk.
	if (!obj->is_monster())		// Don't delete these!
		removed->insert(obj);	// Add to pool instead.
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
 *	Remove a given object's text effect.
 */

void Game_window::remove_text_effect
	(
	Game_object *item		// Item text was added for.
	)
	{
	for (Special_effect *each = effects; each; each = each->next)
		if (each->is_text(item))
			{		// Found it.
			tqueue->remove(each);
			remove_effect(each);
			paint();
			return;
			}
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
	int dist			// Only remove those from eggs at
					//   least this far away.
	)
	{
	Tile_coord apos = main_actor ? main_actor->get_abs_tile_coord()
				: Tile_coord(-1, -1, -1);
	Special_effect *each = effects;
	while (each)
		{
		Special_effect *next = each->next;
					// See if we're far enough away.
		if (each->is_weather() && (!dist ||
		    ((Weather_effect *) each)->out_of_range(apos, dist)))
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
					// Animation in progress?
	if (main_actor_dont_move())
		return;
					// Nothing going on?
	if (!Usecode_script::get_count())
		removed->flush();	// Flush removed objects.
					// Look for obj. in open gump.
	Game_object *obj = 0;
	bool gump = gump_man->double_clicked(x, y, obj);

	// If gump manager didn't handle it, we search the world for an object
	if (!gump)
	{
		obj = find_object(x, y);

		// Check path, except if an NPC.
	    	if (obj && obj->get_npc_num() <= 0 && !obj->is_monster() &&
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
		{			// In combat mode.

			// Want everyone to be in combat.
			combat = 0;
			main_actor->set_target(obj);
			toggle_combat();
					// Being a bully?
			if (obj->get_npc_num() > 0 && obj->get_alignment() ==
							Actor::friendly &&
			   Game::get_game_type() == BLACK_GATE)
				attack_avatar(1 + rand()%3);
			return;
		}
		remove_text_effects();	// Remove text msgs. from screen.
#ifdef DEBUG
		cout << "Object name is " << obj->get_name() << endl;
#endif
		usecode->init_conversation();
		obj->activate(usecode);
		npc_prox->wait(4);	// Delay "barking" for 4 secs.
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
 *	Remove an NPC from the nearby list.
 */

void Game_window::remove_nearby_npc
	(
	Npc_actor *npc
	)
	{
	if (npc->is_nearby())
		npc_prox->remove(npc);
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
	stop_cx %= c_num_chunks;	// Watch out for end.
	stop_cy %= c_num_chunks;
	unsigned long curtime = SDL_GetTicks();
	for (int cy = from_cy; cy != stop_cy; cy = INCR_CHUNK(cy))
		for (int cx = from_cx; cx != stop_cx; cx = INCR_CHUNK(cx))
			for (Npc_actor *npc = get_chunk(cx, cy)->get_npcs();
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
					// Go through npc's, skipping Avatar.
	for (Actor_vector::iterator it = npcs.begin() + 1; 
						it != npcs.end(); it++)
		{
		Npc_actor *npc = (Npc_actor *) *it;
					// Don't want companions leaving.
		if (npc && npc->get_schedule_type() != Schedule::wait)
			npc->update_schedule(this, hour3, backwards);
		}
	paint();			// Repaint all.
	}

/*
 *	Tell all npc's to restore some of their HP's and/or mana on the hour.
 */

void Game_window::mend_npcs
	(
	)
	{
					// Go through npc's.
	for (Actor_vector::iterator it = npcs.begin(); 
						it != npcs.end(); it++)
		{
		Npc_actor *npc = (Npc_actor *) *it;
		if (npc)
			npc->mend_hourly();
		}
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
	for (Actor_vector::const_iterator it = npcs.begin(); 
							it != npcs.end();++it)
		{
		Actor *npc = *it;
		if (npc->is_monster() || npc == main_actor ||
		    npc->get_party_id() >= 0 ||
		    npc->get_npc_num() >= num_npcs1)
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
					// Create it off-screen.
	Monster_actor *guard = Monster_actor::create(0x3b2,
		main_actor->get_cx() + 8, main_actor->get_cy() + 8, 0, 0, 0);
	add_nearby_npc(guard);
	Tile_coord actloc = main_actor->get_abs_tile_coord();
	Tile_coord dest(-1, -1, -1);
	for (int i = 2; i < 6 && dest.tx == -1; i++)
		dest = Game_object::find_unblocked_tile(actloc, i, 3);
	if (dest.tx != -1)
		{
		int dir = Get_direction(dest.ty - actloc.ty,
						actloc.tx - dest.tx);
					
		char frames[2];		// Use frame for starting attack.
		frames[0] = guard->get_dir_framenum(dir, Actor::standing);
		frames[1] = guard->get_dir_framenum(dir, 3);
		Actor_action *action = new Sequence_actor_action(
				new Frames_actor_action(frames, 2),
				new Usecode_actor_action(0x625, main_actor,
					Usecode_machine::double_click));
		Schedule::set_action_sequence(guard, dest, action, true);
		}
	}

/*
 *	Have nearby residents attack the Avatar.
 */

void Game_window::attack_avatar
	(
	int create_guards		// # of extra guards to create.
	)
	{
	while (create_guards--)
		{
					// Create it off-screen.
		Monster_actor *guard = Monster_actor::create(0x3b2,
			main_actor->get_cx() + 8, 
			main_actor->get_cy() + 8, 0, 0, 0);
		add_nearby_npc(guard);
		guard->set_target(main_actor, true);
		guard->approach_another(main_actor);
		}

	Actor_vector npcs;		// See if someone is nearby.
	main_actor->find_nearby_actors(npcs, c_any_shapenum, 12);
	for (Actor_vector::const_iterator it = npcs.begin(); 
							it != npcs.end();++it)
		{
		Actor *npc = (Actor *) *it;
					// No monsters, except guards.
		if ((npc->get_shapenum() == 0x3b2 || !npc->is_monster()) && 
		    npc != main_actor &&
		    npc->get_party_id() < 0)
			npc->set_target(main_actor, true);
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
	init_actors();		// Set up actors if not already done.
				// This also sets up initial 
				// schedules and positions.

	CYCLE_RED_PLASMA();

	usecode->read();		// Read the usecode flags
	CYCLE_RED_PLASMA();

	if (Game::get_game_type() == BLACK_GATE)
	{
		string yn;		// Override from config. file.
					// Skip intro. scene?
		config->value("config/gameplay/skip_intro", yn, "no");
		if (yn == "yes")
			usecode->set_global_flag(
				Usecode_machine::did_first_scene, 1);

					// Should Avatar be visible?
		if (usecode->get_global_flag(Usecode_machine::did_first_scene))
			main_actor->clear_flag(Obj_flags::dont_render);
		else
			main_actor->set_flag(Obj_flags::dont_render);
	}

	CYCLE_RED_PLASMA();

	Actor *party[9];
	int cnt = get_party(party, 1);	// Get entire party.

	for (int i = 0; i < cnt; i++)	// Init. rings.
	{
		party[i]->init_readied();
		CYCLE_RED_PLASMA();
	}
	faded_out = 0;
	time_stopped = 0;
	Audio::get_ptr()->cancel_streams();
//+++++The below wasn't prev. done by ::read(), so maybe it should be
//+++++controlled by a 'first-time' flag.
				// Want to activate first egg.
	Map_chunk *olist = get_chunk(
			main_actor->get_cx(), main_actor->get_cy());
	olist->setup_cache();
	CYCLE_RED_PLASMA();
	Tile_coord t = main_actor->get_abs_tile_coord();
	olist->activate_eggs(main_actor, t.tx, t.ty, t.tz, -1, -1);
	
	// Force entire repaint.
	set_all_dirty();
	Face_stats::load_config(config);

	// Fade out & clear screen before palette change
	pal->fade_out(c_fade_out_time);
	clear_screen(true);
#ifdef RED_PLASMA
	load_palette_timer = 0;
#endif

	// Set palette for time-of-day.
	clock.set_palette();
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
	Image_buffer8 *ibuf = get_win()->get_ib8();

	ibuf->fill8(startc, w, h, x, y);

	for (int i=0; i < w*h; i++) {
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
	painted = true;
}

/*
 *	Chunk caching emulation:  swap out chunks which are now at least
 *	3 chunks away.
 */
void Game_window::emulate_cache(int oldx, int oldy, int newx, int newy)
{
	if (oldx == -1 || oldy == -1)
		return;			// Seems like there's nothing to do.
	remove_weather_effects(120);	// Cancel weather from eggs that are
					//   far away.
	int nearby[5][5];		// Chunks within 3.
	// Set to 0
	memset(nearby, 0, sizeof(nearby));
					// Figure old range.
	int old_minx = c_num_chunks + oldx - 2, 
	    old_maxx = c_num_chunks + oldx + 2;
	int old_miny = c_num_chunks + oldy - 2, 
	    old_maxy = c_num_chunks + oldy + 2;
					// Figure new range.
	int new_minx = c_num_chunks + newx - 2, 
	    new_maxx = c_num_chunks + newx + 2;
	int new_miny = c_num_chunks + newy - 2, 
	    new_maxy = c_num_chunks + newy + 2;
	// Now we write what we are now near
	int x, y;
	for (y = new_miny; y <= new_maxy; y++) 
		{
		if (y > old_maxy)
			break;		// Beyond the end.
		int dy = y - old_miny;
		if (dy < 0)
			continue;
		assert(dy < 5);
		for (x = new_minx; x <= new_maxx; x++)
			{
			if (x > old_maxx)
				break;
			int dx = x - old_minx;
			if (dx >= 0)
				{
				assert(dx < 5);
				nearby[dx][dy] = 1;
				}
			}
		}
	// Swap out chunks no longer nearby (0).
	Game_object_vector removes;
	for (y = 0; y < 5; y++)
		for (x = 0; x < 5; x++)
			{
			if (nearby[x][y] != 0)
				continue;
			Map_chunk *list = get_chunk_safely(
				(old_minx + x)%c_num_chunks,
				(old_miny + y)%c_num_chunks);
			if (!list) continue;
			Object_iterator it(list->get_objects());
			Game_object *each;
			while ((each = it.get_next()) != 0)
				{
				if (each->is_egg())
					((Egg_object *) each)->reset();
				else if (each->get_flag(Obj_flags::is_temporary))
					removes.push_back(each);
				}
			}
	for (Game_object_vector::const_iterator it=removes.begin(); 
						it!=removes.end(); ++it)
		{
#ifdef DEBUG
		cout << "Culling object: " << (*it)->get_name() << "@" << ((Game_object *)(*it))->get_worldx() << "," << ((Game_object *)(*it))->get_worldy() << "," << ((Game_object *)(*it))->get_lift() <<endl;
#endif
		(*it)->delete_contents();  // first delete item's contents
		(*it)->remove_this(0);
		}
	}

// Tests to see if a move goes out of range of the actors superchunk
bool Game_window::emulate_is_move_allowed(int tx, int ty)
{
	int ax = camera_actor->get_cx() / c_chunks_per_schunk;
	int ay = camera_actor->get_cy() / c_chunks_per_schunk;
	tx /= c_tiles_per_schunk;
	ty /= c_tiles_per_schunk;

	int difx = ax - tx;
	int dify = ay - ty;
	
	if (difx < 0) difx = -difx;
	if (dify < 0) dify = -dify;

	// Is it within 1 superchunk range?
	if ((!difx || difx == 1 || difx == c_num_schunks || difx == c_num_schunks-1) && 
		(!dify || dify == 1 || dify == c_num_schunks || dify == c_num_schunks-1))
		return true;

	return false;
}

//create mini-screenshot (96x60) for use in savegames
Shape_file* Game_window::create_mini_screenshot()
{
	Shape_file* sh = 0;
	Shape_frame* fr = 0;
	unsigned char* img = 0;

	set_all_dirty();
	paint_map(0, 0, get_width(), get_height());

	img = win->mini_screenshot();
	
	if (img) {
		fr = new Shape_frame();
		fr->xleft = 0;
		fr->yabove = 0;
		fr->xright = 95;
		fr->ybelow = 59;
		fr->create_rle(img, 96, 60);
		fr->rle = 1;
		delete [] img;

		sh = new Shape_file(fr);
	}

	set_all_dirty();
	paint();
	return sh;
}

#ifdef RED_PLASMA

#define	BG_PLASMA_START_COLOR	128
#define	BG_PLASMA_CYCLE_RANGE	80

#define	SI_PLASMA_START_COLOR	16
#define	SI_PLASMA_CYCLE_RANGE	96

void Game_window::setup_load_palette()
{
	if (load_palette_timer != 0)
		return;

	if (Game::get_game_type()==BLACK_GATE)
    {
        plasma_start_color = BG_PLASMA_START_COLOR;
        plasma_cycle_range = BG_PLASMA_CYCLE_RANGE;
    }
	else if (Game::get_game_type()==SERPENT_ISLE)
    {
        plasma_start_color = SI_PLASMA_START_COLOR;
        plasma_cycle_range = SI_PLASMA_CYCLE_RANGE;
    }
    
    // Put up the plasma to the screen
    plasma(get_width(), get_height(), 0, 0, plasma_start_color, plasma_start_color+plasma_cycle_range-1);

     // Load the palette
	if (Game::get_game_type()==BLACK_GATE)
		pal->load("<STATIC>/intropal.dat",2);
	else if (Game::get_game_type()==SERPENT_ISLE)
		pal->load(MAINSHP_FLX,1);

	pal->apply();
	load_palette_timer = SDL_GetTicks();
}

void Game_window::cycle_load_palette()
{
	if (load_palette_timer == 0)
		return;
	uint32 ticks = SDL_GetTicks();
	if(ticks > load_palette_timer+75)
	{
		for(int i = 0; i < 4; ++i)
			get_win()->rotate_colors(plasma_start_color, plasma_cycle_range, 1);
		show(true);

		// We query the timer here again, as the blit can take easily 50 ms and more
		// depending on the chosen scaler and the overall system speed
		load_palette_timer = SDL_GetTicks();
	}
}
#endif
