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
#include "monsters.h"
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
#include "gamemap.h"
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
#include "mappatch.h"
#include "version.h"
#include "drag.h"
#include "u7drag.h"
#ifdef USE_EXULTSTUDIO
#include "server.h"
#include "servemsg.h"
#include "objserial.h"
#endif

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
	int laststate;			// Last state for SFX music tracks, 
							// 1 outside, 2 dungeon, 3 nighttime

public:
	Background_noise(Game_window *gw) : repeats(0), last_sound(-1),
					gwin(gw), laststate(0)
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
	int currentstate = 0;

#ifndef COLOURLESS_REALLY_HATES_THE_BG_SFX

	int bghour = gwin->get_hour();
	if(gwin->is_in_dungeon())
		currentstate = 2;
	else
		if (bghour < 6 || bghour > 20)
			currentstate = 3;	//Night time
		else
			currentstate = 1;

	MyMidiPlayer *player = Audio::get_ptr()->get_midi();
	if (player && player->music_conversion == XMIDI_CONVERT_OGG)
	{
		delay = 1500;	//Quickly get back to this function check
		//We've got OGG so play the background SFX tracks

		int curr_track = player->get_current_track();

		if(curr_track == 7 && currentstate == 3)
		{
			//Play the cricket sounds at night 
			delay = 3000 + rand()%5000;
			Audio::get_ptr()->play_sound_effect(61, MIX_MAX_VOLUME - 30);
		}

		if((curr_track == -1 || laststate != currentstate ) && Audio::get_ptr()->is_music_enabled())
		{
			if(curr_track == -1 || (curr_track >=4 && curr_track <=8) || curr_track == 52)		//Not already playing music
			{
				int tracknum=255;

				//Get the relevant track number. SI tracks are converted back to BG ones later on
				if(gwin->is_in_dungeon())
				{
					//Start the SFX music track then
					if(Game::get_game_type() == BLACK_GATE)
						tracknum = 52;
					else
						tracknum = 42;	
					laststate = 2;
				}
				else				
				{
					if (bghour < 6 || bghour > 20)
					{
						if(Game::get_game_type() == BLACK_GATE)					
							tracknum = 7;
						else
							tracknum = 66;
						laststate = 3;
					}
					else
					{
						//Start the SFX music track then
						if(Game::get_game_type() == BLACK_GATE)
							tracknum = 6;
						else
							tracknum = 67;
						laststate = 1;
					}
				}
				Audio::get_ptr()->start_music(tracknum,1);
			}
		}
	}
	else
	{
		
		//Tests to see if track is playing the SFX tracks, possible 
		//when the game has been restored
		//and the Audio option was changed from OGG to something else
		if(player && player->get_current_track() >=4 && 
		   player->get_current_track() <= 8)
			player->stop_music();


		//Not OGG so play the SFX sounds manually
					// Only if outside.
		if (ava && !gwin->is_main_actor_inside()  &&
			// +++++SI SFX's don't sound right.
			Game::get_game_type() == BLACK_GATE )
		{
			int sound;		// BG SFX #.
			static unsigned char bgnight[] = {61, 61, 255},
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
	}
#endif

	gwin->get_tqueue()->add(curtime + delay, this, udata);
}

//
/*
 *	Create game window.
 */

Game_window::Game_window
	(
	int width, int height, int scale, int scaler		// Window dimensions.
	) : 
	    win(0), map(new Game_map()),
	    usecode(0), combat(false), armageddon(false),
            tqueue(new Time_queue()), clock(tqueue), time_stopped(0),
	    std_delay(c_std_delay),
	    npc_prox(new Npc_proximity_handler(this)),
	    effects(0), gump_man(new Gump_manager),
	    render_seq(0), painted(false), focus(true), 
	    teleported(false), in_dungeon(0), ice_dungeon(false), fonts(0),
	    moving_barge(0), main_actor(0), skip_above_actor(31),
	    npcs(0), bodies(0), mouse3rd(false), fastmouse(false),
            text_bg(false), 
	    palette(-1), brightness(100), user_brightness(100), 
	    faded_out(false), fades_enabled(true),
	    special_light(0), last_restore_hour(6),
	    dragging(0),
	    theft_warnings(0), theft_cx(255), theft_cy(255),
	    background_noise(new Background_noise(this)),
	    double_click_closes_gumps(false),
	    walk_after_teleport(false), removed(new Deleted_objects()), 
	    skip_lift(16), paint_eggs(false), debug(0), camera_actor(0)
#ifdef RED_PLASMA
	    ,load_palette_timer(0), plasma_start_color(0), plasma_cycle_range(0)
#endif
	{
	game_window = this;		// Set static ->.
	shape_man = new Shape_manager();// Create the single instance.

	for (int i=0; i<5; i++)
		extra_fonts[i] = NULL;

	set_window_size(width, height, scale, scaler);
	pal = new Palette();
	string str;
	config->value("config/gameplay/textbackground", text_bg, -1);
	config->value("config/gameplay/mouse3rd", str, "no");
	if (str == "yes")
		mouse3rd = true;
	config->set("config/gameplay/mouse3rd", str, true);
	config->value("config/gameplay/fastmouse", str, "no");
	if (str == "yes")
		fastmouse = true;
	config->set("config/gameplay/fastmouse", str, true);
	config->value("config/gameplay/double_click_closes_gumps", str, "no");
	if (str == "yes")
		double_click_closes_gumps = true;
	config->set("config/gameplay/double_click_closes_gumps", str, true);
	config->value("config/gameplay/walk_after_teleport", str, "no");
	if (str == "yes")
		walk_after_teleport = true;
	config->set("config/gameplay/walk_after_teleport", str, true);
	config->value("config/gameplay/combat/difficulty",
							combat_difficulty, 0);
	config->set("config/gameplay/combat/difficulty",
						combat_difficulty, true);
	config->value("config/audio/disablepause", str, "no");
	config->set("config/audio/disablepause", str, true);
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
//++++Now points into xforms	delete [] invis_xform;
	delete shape_man;
	delete gump_man;
	delete background_noise;
	delete tqueue;
	delete win;
	delete dragging;
	delete pal;
	delete map;
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

#if 0
#define BLEND(alpha, newc, oldc) ((newc*255L) - (oldc*(255L-alpha)))/alpha
void Analyze_xform(unsigned char *xform, int alpha, Palette *pal)
	{
	long br = 0, bg = 0, bb = 0;	// Trying to figure out blend color.
	for (int i = 0; i < 256; i++)
		{
		int xi = xform[i];	// Index of color mapped to.
		br += BLEND(alpha, pal->get_red(xi), pal->get_red(i));
		bg += BLEND(alpha, pal->get_green(xi), pal->get_green(i));
		bb += BLEND(alpha, pal->get_blue(xi), pal->get_blue(i));
		}
	br /= 256;			// Take average.
	bg /= 256;
	bb /= 256;
	cout << "Blend (r,g,b) = " << br << ',' << bg << ',' << bb << endl;
	}
#endif


void Game_window::init_files(bool cycle)
{
	
	// Determine some colors based on the default palette
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

#ifdef RED_PLASMA
	// Display red plasma during load...
	if (cycle)
		setup_load_palette();
#endif

	usecode = Usecode_machine::create(this);

	CYCLE_RED_PLASMA();
	shape_man->load();		// All the .vga files!
	CYCLE_RED_PLASMA();
	if (!fonts)
	{
		fonts = new Fonts_vga_file();
		fonts->init();
	}
	CYCLE_RED_PLASMA();

	ifstream textflx;	
	if (is_system_path_defined("<PATCH>") && U7exists(PATCH_TEXT))
		U7open(textflx, PATCH_TEXT);
	else
  		U7open(textflx, TEXT_FLX);
	Setup_item_names(textflx);	// Set up list of item names.
	std::size_t len, nxforms = sizeof(xforms)/sizeof(xforms[0]);
	if (U7exists(XFORMTBL))
		{			// Read in translucency tables.
		Segment_file xf(XFORMTBL);
		for (int i = 0; i < nxforms; i++)
			{
			xforms[nxforms - 1 - i] = (uint8*)xf.retrieve(i, len);
			CYCLE_RED_PLASMA();
#if 0	/* +++++++Testing */
			pal->load(PALETTES_FLX, 0);	// ++++++TESTING
			cout << "XFORM " << (nxforms - 1 - i) << ":" << endl;
			cout << "Alpha = 64: ";
			Analyze_xform(xforms[nxforms - 1 - i], 64, pal);
			cout << "Alpha = 128: ";
			Analyze_xform(xforms[nxforms - 1 - i], 128, pal);
			cout << "Alpha = 192: ";
			Analyze_xform(xforms[nxforms - 1 - i], 192, pal);
#endif
			}
		}
	else				// Create algorithmically.
		{
		pal->load(PALETTES_FLX, 0);
					// RGB blend colors:
		static unsigned char blends[3*11] = {
			36,10,48, 24,10,4, 25,27,29, 17,33,7, 63,52,12, 
			7,13,63,
			2,17,0, 63,2,2, 65,61,62, 14,10,8, 49,48,46};
		static int alphas[11] = {128, 128, 192, 128, 64, 128,
					128, 128, 128, 128, 128};
		for (int i = 0; i < nxforms; i++)
			{
			xforms[i] = new unsigned char[256];
			pal->create_trans_table(blends[3*i],
				blends[3*i+1], blends[3*i+2],
				alphas[i], xforms[i]);
			}
		}

	invis_xform = xforms[nxforms - 1 - 2];   // ->entry 2.
	unsigned long timer = SDL_GetTicks();
	srand(timer);			// Use time to seed rand. generator.
					// Force clock to start.
	tqueue->add(timer, &clock, reinterpret_cast<long>(this));
					// Go to starting chunk
	scrolltx = game->get_start_tile_x();
	scrollty = game->get_start_tile_y();
		
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

//This is now run from exult.cc
//	Audio::get_ptr()->Init_sfx();
	int fps;			// Init. animation speed.
	config->value("config/video/fps", fps, 5);
	if (fps <= 0)
		fps = 5;
	std_delay = 1000/fps;		// Convert to msecs. between frames.
}

/*
 *	Reload one of the shape files (msg. from ExultStudio).
 */

void Game_window::reload_shapes
	(
	int dragtype			// Type from u7drag.h.
	)
	{
#if 0	/* +++++++Got to rewrite in Shape_manager */
	switch (dragtype)
		{
	case U7_SHAPE_SHAPES:
		shapes.init();		// Reread .vga file.
		shapes.read_info(GAME_BG);	//+++++Needs work.
					// ++++Reread text?
		break;
	case U7_SHAPE_GUMPS:
		gumps.load(GUMPS_VGA);		//++++++Patch?
		break;
	case U7_SHAPE_FONTS:
		fonts->init();			//++++++Patch?
		break;
	case U7_SHAPE_FACES:
		faces.load(FACES_VGA, PATCH_FACES);
		break;
	case U7_SHAPE_SPRITES:
		sprites.load(SPRITES_VGA);	//++++++Patch?
		break;
	default:
		cerr << "Type not supported:  " << dragtype << endl;
		break;
		}
#endif
	}

/*
 *	Get map patch list.
 */
Map_patch_collection *Game_window::get_map_patches()
{
	return map->get_map_patches();
}

/*
 *	Get desired chunk.
 */
Map_chunk *Game_window::get_chunk(int cx, int cy)
{
	return map->get_chunk(cx, cy);
}
Map_chunk *Game_window::get_chunk(Game_object *obj)
{
	return map->get_chunk(obj->get_cx(), obj->get_cy());
}
Map_chunk *Game_window::get_chunk_safely(int cx, int cy)
{
	return map->get_chunk_safely(cx, cy);
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
		long new_expire = Game::get_ticks() + delay;
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
	long delay = time_stopped - Game::get_ticks();
	if (delay > 0)
		return delay;
	time_stopped = 0;		// Done.
	return 0;
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
			if (targ && targ->get_flag(Obj_flags::in_party))
				act->set_target(0);
			}
		}
	}

/*
 *	Add an NPC.
 */

void Game_window::add_npc
	(
	Actor *npc,
	int num				// Number.  Has to match npc->num.
	)
	{
	assert(num == npc->get_npc_num());
	assert(num <= npcs.size());
	if (num == npcs.size())		// Add at end.
		npcs.append(npc);
	else
		{			// Better be unused.
		assert(!npcs[num] || npcs[num]->is_unused());
		delete npcs[num];
		npcs[num] = npc;
		}
	}

/*
 *	Find first unused NPC #.
 */

int Game_window::get_unused_npc
	(
	)
	{
	int cnt = npcs.size();		// Get # in list.
	for (int i = 0; i < cnt; i++)
		{
		if (!npcs[i])
			return i;	// (Don't think this happens.)
		if (npcs[i]->is_unused())
			return i;
		}
	return cnt;			// First free one is past the end.
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
		center_view(get_main_actor()->get_tile());
		paint();
		char msg[80];
		snprintf(msg, 80, "%dx%dx%d", neww, newh, newsc);
		center_text(msg);
	}
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
	Usecode_script::clear();	// Clear out all scheduled usecode.
	map->clear();
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
	remove_all_effects(false);
	}

/*
 *	Look throughout the map for a given shape.  The search starts at
 *	the first currently-selected shape, if possible.
 *
 *	Output:	true if found, else 0.
 */

bool Game_window::locate_shape
	(
	int shapenum,			// Desired shape.
	bool upwards			// If true, search upwards.
	)
	{
					// Get (first) selected object.
	const std::vector<Game_object *>& sel = cheat.get_selected();
	Game_object *start = sel.size() ? sel[0] : 0;
	char msg[80];
	snprintf(msg, sizeof(msg), "Searching for shape %d", shapenum);
	center_text(msg);
	paint();
	show();
	Game_object *obj = map->locate_shape(shapenum, upwards, start);
	if (!obj)
		{
		center_text("Not found");
		return false;		// Not found.
		}
	remove_text_effects();
	cheat.clear_selected();		// Select obj.
	cheat.append_selected(obj);
					//++++++++Got to show it.
	Game_object *owner = obj->get_outermost(); //+++++TESTING
	if (owner != obj)
		cheat.append_selected(owner);
	center_view(owner->get_tile());
	return true;
	}

/*
 *	Set location to ExultStudio.
 */

inline void Send_location
	(
	Game_window *gwin
	)
	{
#ifdef USE_EXULTSTUDIO
	if (client_socket >= 0 &&	// Talking to ExultStudio?
	    cheat.in_map_editor())
		{
		unsigned char data[50];
		unsigned char *ptr = &data[0];
		Write4(ptr, gwin->get_scrolltx());
		Write4(ptr, gwin->get_scrollty());
		Write4(ptr, gwin->get_width()/c_tilesize);
		Write4(ptr, gwin->get_height()/c_tilesize);
		Write4(ptr, gwin->get_win()->get_scale());
		Exult_server::Send_data(client_socket, Exult_server::view_pos,
					&data[0], ptr - data);
		}
#endif
	}

/*
 *	Send current location to ExultStudio.
 */

void Game_window::send_location
	(
	)
	{
	Send_location(this);
	}

/*
 *	Set the scroll position to a given tile.
 */

void Game_window::set_scrolls
	(
	int newscrolltx, int newscrollty
	)
	{
	scrolltx = newscrolltx;
	scrollty = newscrollty;
	set_scroll_bounds();		// Set scroll-control.
	Barge_object *old_active_barge = moving_barge;
	map->read_map_data();		// This pulls in objects.
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
	set_ice_dungeon(nlist->is_ice_dungeon(tx, ty));
	send_location();		// Tell ExultStudio.
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
	set_scrolls(DECR_TILE(cent.tx, tw/2), DECR_TILE(cent.ty, th/2));
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
	if (a == main_actor &&		// Setting back to main actor?
	    camera_actor &&		// Change in chunk?
	    (camera_actor->get_cx() != main_actor->get_cx() ||
	     camera_actor->get_cy() != main_actor->get_cy()))
	    				// Cache out temp. objects.
		emulate_cache(camera_actor->get_cx(), camera_actor->get_cy(),
			main_actor->get_cx(), main_actor->get_cy());
	camera_actor = a;
	Tile_coord t = a->get_tile();
	set_scrolls(t);			// Set scrolling around position,
					//   and read in map there.
	set_all_dirty();
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

/*
 *	Get screen area used by object.
 */

Rectangle Game_window::get_shape_rect(Game_object *obj)
{
	Shape_frame *s = obj->get_shape();
	if(!s)
	{
		// This is probably fatal.
#if DEBUG
		std::cerr << "DEATH! get_shape() returned a NULL pointer: " << __FILE__ << ":" << __LINE__ << std::endl;
		std::cerr << "Betcha it's a little doggie." << std::endl;
#endif
		return Rectangle(0,0,0,0);
	}
	Tile_coord t = obj->get_tile();	// Get tile coords.
	int lftpix = 4*t.tz;
	t.tx += 1 - get_scrolltx();
	t.ty += 1 - get_scrollty();
					// Watch for wrapping.
	if (t.tx < -c_num_tiles/2)
		t.tx += c_num_tiles;
	if (t.ty < -c_num_tiles/2)
		t.ty += c_num_tiles;
	return get_shape_rect(s,
		t.tx*c_tilesize - 1 - lftpix,
		t.ty*c_tilesize - 1 - lftpix);
}

/*
 *	Get screen location of given tile.
 */

inline void Get_shape_location
	(
	Tile_coord t,
	int scrolltx, int scrollty,
	int& x, int& y
	)
	{
	int lft = 4*t.tz;
	t.tx += 1 - scrolltx;
	t.ty += 1 - scrollty;
				// Watch for wrapping.
	if (t.tx < -c_num_tiles/2)
		t.tx += c_num_tiles;
	if (t.ty < -c_num_tiles/2)
		t.ty += c_num_tiles;
	x = t.tx*c_tilesize - 1 - lft;
	y = t.ty*c_tilesize - 1 - lft;
	}

/*
 *	Get screen loc. of object which MUST be on the map (no owner).
 */

void Game_window::get_shape_location(Game_object *obj, int& x, int& y)
{
	Get_shape_location(obj->get_tile(), scrolltx, scrollty, x, y);
}
void Game_window::get_shape_location(Tile_coord t, int&x, int& y)
{
	Get_shape_location(t, scrolltx, scrollty, x, y);
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
		schedule_npcs(2,7,false);
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
		if (is_system_path_defined("<PATCH>") && 
						U7exists(PATCH_INITGAME))
			restore_gamedat(PATCH_INITGAME);
		else
			{
					// Flag that we're reading U7 file.
			Game::set_new_game();
			restore_gamedat(INITGAME);
			}
		ofstream out;
					// Editing, and no IDENTITY?
		if (Game::is_editing() && !U7exists(IDENTITY))
			{
			U7open(out, IDENTITY);
			out << Game::get_gametitle().c_str() << endl;
			out.close();
			}
		U7open(out, GNEWGAMEVER);
		getVersionInfo(out);
		out.close();
		}
					//++++Maybe just test for IDENTITY+++:
	else if (!U7exists(U7NBUF_DAT) && !U7exists(NPC_DAT) &&
							!Game::is_editing())
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
			for(; (*ptr!=0x1a && *ptr!=0x0d &&
							*ptr != 0x0a); ptr++)
				;
			*ptr = 0;
			cout << "Gamedat identity " << gamedat_identity << endl;
			char *static_identity = get_game_identity(INITGAME);
			if(strcmp(static_identity, gamedat_identity))
				{
					delete [] static_identity;
					return false;
				}
			delete [] static_identity;
					//   scroll coords.
		}
	read_save_names();		// Read in saved-game names.	
	return true;
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
	return map->create_ireg_object(info, shnum, frnum, tilex, tiley, lift);
	}

Ireg_game_object *Game_window::create_ireg_object
	(
	int shnum, int frnum
	)
	{
	return map->create_ireg_object(ShapeID::get_info(shnum), shnum, frnum,
						0, 0, 0); 	
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
	// Lets just show a nice message on screen first

	int width = get_width();
	int centre_x  = width/2;
	int height = get_height();
	int centre_y = height/2;
	int text_height = get_text_height(0);
	int text_width = get_text_width(0, "Saving Game");

	win->fill_translucent8(0, width, height, 0, 0, xforms[2]);
	paint_text(0, "Saving Game", centre_x-text_width/2, centre_y-text_height);
	show(true);
	map->write_ireg();		// Write ireg files.
	write_npcs();			// Write out npc.dat.
	usecode->write();		// Usecode.dat (party, global flags).
	write_gwin();			// Write our data.
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
	ofstream gout_stream;
	U7open(gout_stream, GWINDAT);	// Gamewin.dat.
	StreamDataSource gout(&gout_stream);
					// Start with scroll coords (in tiles).
	gout.write2(get_scrolltx());
	gout.write2(get_scrollty());
					// Write clock.
	gout.write2(clock.get_day());
	gout.write2(clock.get_hour());
	gout.write2(clock.get_minute());
	gout.write4(special_light);	// Write spell expiration minute.
	MyMidiPlayer *player = Audio::get_ptr()->get_midi();
	if (player) {
		gout.write4(static_cast<uint32>(player->get_current_track()));
		gout.write4(static_cast<uint32>(player->is_repeating()));
	} else {
		gout.write4(static_cast<uint32>(-1));
		gout.write4(0);
	}
	gout.write1(armageddon ? 1 : 0);
	gout_stream.flush();
	if (!gout_stream.good())
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
	ifstream gin_stream;
	try
	{
		U7open(gin_stream, GWINDAT);	// Gamewin.dat.
	} catch (const file_open_exception&)
	{
		return;
	}
	
	StreamDataSource gin(&gin_stream);

					// Start with scroll coords (in tiles).
	scrolltx = gin.read2();
	scrollty = gin.read2();
					// Read clock.
	clock.set_day(gin.read2());
	clock.set_hour(gin.read2());
	clock.set_minute(gin.read2());
	last_restore_hour = clock.get_total_hours();
	if (!clock.in_queue())		// Be sure clock is running.
		tqueue->add(Game::get_ticks(), &clock, reinterpret_cast<long>(this));
	if (!gin_stream.good())		// Next ones were added recently.
		throw file_read_exception(GWINDAT);
	special_light = gin.read4();
	armageddon = false;		// Old saves may not have this yet.
	
	if (!gin_stream.good())
	{
		special_light = 0;
		return;
	}

	int track_num = gin.read4();
	int repeat = gin.read4();
	if (!gin_stream.good())
	{
		Audio::get_ptr()->stop_music();
		return;
	}

	Audio::get_ptr()->start_music(track_num, repeat != false);
	armageddon = gin.read1() == 1 ? true : false;
	if (!gin_stream.good())
		armageddon = false;

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
	map->write_static();		// Write ifix, map files.
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
	set_palette(PALETTE_RED);		// Palette 8 is the red one.
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
	map->read_map_data();		// Be sure objects are present.
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
		{
		add_nearby_npcs(new_rcx, scrollty/c_tiles_per_chunk, 
			INCR_CHUNK(new_rcx), ((scrollty + 
			(h + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks);
		Send_location(this);
		}
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
	map->read_map_data();		// Be sure objects are present.
	win->copy(0, 0, get_width() - c_tilesize, get_height(), c_tilesize, 0);
	dirty.x += c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
	int h = get_height();
//	add_dirty(Rectangle(0, 0, c_tilesize, h));
	paint(0, 0, c_tilesize, h);
					// Find newly visible NPC's.
	int new_lcx = (scrolltx/c_tiles_per_chunk)%c_num_chunks;
	if (new_lcx != old_lcx)
		{
		add_nearby_npcs(new_lcx, scrollty/c_tiles_per_chunk, 
			INCR_CHUNK(new_lcx), 
	((scrollty + (h + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks);
		Send_location(this);
		}
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
	map->read_map_data();		// Be sure objects are present.
	win->copy(0, c_tilesize, w, h - c_tilesize, 0, 0);
	dirty.y -= c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
//	add_dirty(Rectangle(0, h - c_tilesize, w, c_tilesize));
	paint(0, h - c_tilesize, w, c_tilesize);
					// Find newly visible NPC's.
	int new_bcy = ((scrollty + (h - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks;
	if (new_bcy != old_bcy)
		{
		add_nearby_npcs(scrolltx/c_tiles_per_chunk, new_bcy, 
	    ((scrolltx + (w + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks,
			INCR_CHUNK(new_bcy));
		Send_location(this);
		}
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
	map->read_map_data();		// Be sure objects are present.
	int w = get_width();
	win->copy(0, 0, w, get_height() - c_tilesize, 0, c_tilesize);
	dirty.y += c_tilesize;		// Shift dirty rect.
	dirty = clip_to_win(dirty);
//	add_dirty(Rectangle(0, 0, w, c_tilesize));
	paint(0, 0, w, c_tilesize);
					// Find newly visible NPC's.
	int new_tcy = (scrollty/c_tiles_per_chunk)%c_num_chunks;
	if (new_tcy != old_tcy)
		{
		add_nearby_npcs(scrolltx/c_tiles_per_chunk, new_tcy,
	    ((scrolltx + (w + c_tilesize - 1)/c_tilesize)/c_tiles_per_chunk)%
							c_num_chunks,
							INCR_CHUNK(new_tcy));
		Send_location(this);
		}
	}

/*
 *	Get gump being dragged.
 */

Gump *Game_window::get_dragging_gump
	(
	)
	{
	return dragging ? dragging->gump : 0;
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

	int height = main_actor->get_info().get_3d_height();
	
	Tile_coord start = main_actor->get_tile();
	int dir;
	
	for (dir = 0; dir < 8; dir++)
	{
		Tile_coord dest = start.get_neighbor(dir);
		int cx = dest.tx/c_tiles_per_chunk, cy = dest.ty/c_tiles_per_chunk;
		int tx = dest.tx%c_tiles_per_chunk, ty = dest.ty%c_tiles_per_chunk;

		Map_chunk *clist = get_chunk_safely(cx, cy);
		clist->setup_cache();
		blocked[dir] = clist->is_blocked (height, 
			main_actor->get_lift(), tx, ty, nlift, 
					main_actor->get_type_flags(), 1);
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
				int savetz = start.tz;
				if (!Map_chunk::is_blocked(start, 1, 
						MOVE_WALK, 100) && 
						start.tz < savetz)
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
	if (main_actor_dont_move() || gump_man->gump_mode())
		return;

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
			   btile = moving_barge->get_tile();
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
		if (!gump_man->gump_mode())
			main_actor->get_followers();
		}
	}

/*
 *	Teleport the party.
 */

void Game_window::teleport_party
	(
	Tile_coord t,			// Where to go.
	bool skip_eggs			// Don't activate eggs at dest.
	)
	{
	Tile_coord oldpos = main_actor->get_tile();
	main_actor->set_action(0);	// I think this is right.
	main_actor->move(t.tx, t.ty, t.tz);	// Move Avatar.
	set_all_dirty();

	int cnt = usecode->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		int party_member=usecode->get_party_member(i);
		Actor *person = get_npc(party_member);
		if (person && !person->is_dead() && 
		    person->get_schedule_type() != Schedule::wait)
			{
			person->set_action(0);
			Tile_coord t1 = Map_chunk::find_spot(t, 8,
				person->get_shapenum(), person->get_framenum(),
									1);
			if (t1.tx != -1)
				person->move(t1);
			}
		}
	center_view(t);			// Bring pos. into view.
	main_actor->get_followers();
	if (!skip_eggs)			// Check all eggs around new spot.
		Map_chunk::try_all_eggs(main_actor, t.tx, t.ty, t.tz,
					oldpos.tx, oldpos.ty);
	teleported = 1;
	// generate mousemotion event
	int x, y;
	SDL_GetMouseState(&x, &y);
	SDL_WarpMouse(x, y);
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

 
	Game_object_vector found;
	
	int cnt = 0;
	int actor_lift = main_actor->get_lift();
//	int start = actor_lift > 0 ? -1 : 0;
					// Look downward at most one 'floor'.
	int start = actor_lift >= 5 ?  actor_lift - 5 : 0;
	int not_above = skip_lift;
	if (skip_above_actor < not_above)
		not_above = skip_above_actor;
					// See what was clicked on.
	for (int lift = start; lift < not_above; lift++)
		cnt += find_objects(lift, x, y, found);
	if (!cnt)
		return (0);		// Nothing found.
					// Find 'best' one.
	Game_object *obj = found[cnt - 1];
					// Try to avoid 'transparent' objs.
	int trans = obj->get_info().is_transparent();
	Game_object_vector::iterator it;
	for (it = found.begin(); it != found.end(); ++it)
		if (obj->lt(*(*it)) == 1 || trans)
			{
			int ftrans = (*it)->get_info().is_transparent();
			if (!ftrans || trans)
				{
				obj = *it;
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
	Game_object_vector& list	// Objects found are appended here.
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
			Map_chunk *olist = map->get_chunk(cx, cy);
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
				Shape_frame *s = obj->get_shape();
				int ox, oy;
				get_shape_location(obj, ox, oy);
				if (s->has_point(x - ox, y - oy)) {
					list.push_back(obj);
					++cnt;
				}
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
	int x, int y,			// Coords. in window.
	bool ctrl			// Control key is pressed.
	)
	{
					// Look for obj. in open gump.
	Gump *gump = gump_man->find_gump(x, y);
	Game_object *obj;		// What we find.
	if (gump)
	{
		obj = gump->find_object(x, y);
		if (!obj) obj = gump->get_cont_or_actor(x, y);
	}
	else				// Search rest of world.
		obj = find_object(x, y);
					// Map-editing?
	if (obj && cheat.in_map_editor())
		{
			
		if (ctrl)		// Control?  Toggle.
			cheat.toggle_selected(obj);
		else
			{		// In normal mode, sel. just this one.
			cheat.clear_selected();
			if (cheat.get_edit_mode() == Cheat::move)
				cheat.append_selected(obj);
			}
		}
	else				// All other cases:  unselect.
		cheat.clear_selected();	

					// Do we want the NPC number?
	Actor *npc = obj ? obj->as_actor() : 0;
	if (npc && cheat.number_npcs() &&
	    (npc->get_npc_num() > 0 || npc==main_actor))
	{
		char str[64];
		snprintf (str, 64, "(%i) %s", npc->get_npc_num(), 
				  obj->get_name().c_str());
		add_text(str, obj);
	}
	else if (obj)
					// Show name.
		add_text(obj->get_name().c_str(), obj);
	else if (cheat.in_map_editor() && skip_lift > 0)
		{			// Show flat, but not when editing ter.
		ShapeID id = get_flat(x, y);
		char str[12];
		snprintf(str, 12, "Flat %d:%d", id.get_shapenum(), 
						id.get_framenum());
		add_text(str, x, y);
		}
	// If it's an actor and we want to grab the actor, grab it.
	if (npc && cheat.grabbing_actor() && 
	    (npc->get_npc_num() || npc==main_actor))
		cheat.set_grabbed_actor (npc);

#ifdef DEBUG
	int shnum, frnum;
	if (obj)
		{
		shnum = obj->get_shapenum(), frnum = obj->get_framenum();
		Shape_info& info = obj->get_info();
		cout << "Object " << shnum << ':' << frnum <<
					" has 3d tiles (x, y, z): " <<
			info.get_3d_xtiles(frnum) << ", " <<
			info.get_3d_ytiles(frnum) << ", " <<
			info.get_3d_height();
		Actor *npc = obj->as_actor();
		if (npc)
			cout  << ", sched = " << 
			npc->get_schedule_type() << ", align = " <<
			npc->get_alignment() << ", npcnum = " <<
			npc->get_npc_num();
		cout << endl;
		Tile_coord t = obj->get_tile();
		cout << "tx = " << t.tx << ", ty = " << t.ty << ", tz = " <<
			t.tz << ", quality = " <<
			obj->get_quality() << 
			", okay_to_take = " <<
			static_cast<int>(obj->get_flag(Obj_flags::okay_to_take)) <<
			", flag0x1d = " << static_cast<int>(obj->get_flag(0x1d)) <<
			", hp = " << obj->get_obj_hp()
			<< endl;
		cout << "obj = " << (void *) obj << endl;
		if (obj->get_flag(Obj_flags::asleep))
			cout << "ASLEEP" << endl;
#if 0	/* Want to get rid of dynamic_casts. */
		if (dynamic_cast<Ireg_game_object*> (obj))
			cout << "IREG object" << endl;
		else if (dynamic_cast<Ifix_game_object*> (obj))
			cout << "IFIX object" << endl;
		else
			cout << "TERRAIN object" << endl;
#endif
		if (obj->is_egg())	// Show egg info. here.
			((Egg_object *)obj)->print_debug();
		}
	else				// Obj==0
		{
		ShapeID id = get_flat(x, y);
		shnum = id.get_shapenum();
		cout << "Clicked on flat shape " << 
			shnum << ':' << id.get_framenum() << endl;

#ifdef CHUNK_OBJ_DUMP
		Map_chunk *chunk = get_chunk_safely(x/c_tiles_per_chunk, y/c_tiles_per_chunk);
		Object_iterator it(chunk->get_objects());
		Game_object *each;
		cout << "Chunk Contents: " << endl;
		while ((each = it.get_next()) != 0)
			cout << "    " << each->get_name() << ":" << each->get_shapenum() << ":" << each->get_framenum() << endl;
#endif
		if (id.is_invalid())
			return;
		}
	Shape_info& info = ShapeID::get_info(shnum);
	cout << "TFA[1][0-6]= " << ((static_cast<int>(info.get_tfa(1)))&127) << endl;
	cout << "TFA[0][0-1]= " << ((static_cast<int>(info.get_tfa(0))&3)) << endl;
	cout << "TFA[0][3-4]= " << ((static_cast<int>((info.get_tfa(0)>>3))&3)) << endl;
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
 *	Get the 'flat' that a screen point is in.
 */

ShapeID Game_window::get_flat
	(
	int x, int y			// Window point.
	)
	{
	int tx = (get_scrolltx() + x/c_tilesize)%c_num_tiles;
	int ty = (get_scrollty() + y/c_tilesize)%c_num_tiles;
	int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
	tx = tx%c_tiles_per_chunk;
	ty = ty%c_tiles_per_chunk;
	Map_chunk *chunk = get_chunk(cx, cy);
	ShapeID id = chunk->get_flat(tx, ty);
	return id;
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
 *	Add text over a given item.
 */

void Game_window::add_text
	(
	const char *msg,
	Game_object *item		// Item text ID's, or null.
	)
	{
	if (!msg)			// Happens with edited games.
		return;
					// Don't duplicate for item.
	for (Special_effect *each = effects; each; each = each->next)
		if (each->is_text(item))
			return;		// Already have text on this.
	Text_effect *txt = new Text_effect(msg, item);
//	txt->paint(this);		// Draw it.
//	painted = 1;
	add_effect(txt);
	}

/*
 *	Add a text object at a given spot.
 */

void Game_window::add_text
	(
	const char *msg,
	int x, int y			// Pixel coord. on screen.
	)
	{
	Text_effect *txt = new Text_effect(msg,
		get_scrolltx() + x/c_tilesize, get_scrollty() + y/c_tilesize);
//	txt->paint(this);		// Draw it.
//	painted = 1;
	add_effect(txt);
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
	 bool repaint
	)
	{
	if (!effects)
		return;
	while (effects)
		{
		tqueue->remove(effects);// Remove from time queue if there.
		remove_effect(effects);
		}
	if (repaint)
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
	Tile_coord apos = main_actor ? main_actor->get_tile()
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
 *	A sign or plaque?
 */

static bool Is_sign
	(
	int shnum
	)
	{
	switch(shnum)
		{
	case 820:			// Plaque.
	case 360:
	case 361:
	case 379:
		return true;
	default:
		return false;
		}
	}

/*
 *	Handle a double-click.
 */

void Game_window::double_clicked
	(
	int x, int y			// Coords in window.
	)
	{
#if 0
//++++++++++++TESTING
	static int ncnt = 0;
	cout << "Showing xform for ncnt = " << ncnt << endl;
	std::size_t nxforms = sizeof(xforms)/sizeof(xforms[0]);
	pal->load(PALETTES_FLX, 0, XFORMTBL, nxforms - 1 - ncnt);
	pal->update();
	ncnt = (ncnt + 1)%nxforms;
//^^^^^^^^^^^^TESTING
#endif
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
#ifdef USE_EXULTSTUDIO

		if (cheat.in_map_editor() && cheat.get_edit_mode() == 
				Cheat::combo_pick && client_socket >= 0)
			{		// Add obj/tile to combo in EStudio.
			ShapeID id = obj ? *obj : get_flat(x, y);
			Tile_coord t = obj ? obj->get_tile() :
			    Tile_coord((scrolltx + x/c_tilesize)%c_num_tiles,
			     	       (scrollty + y/c_tilesize)%c_num_tiles, 
									0);
			std::string name = item_names[id.get_shapenum()];
			if (Object_out(client_socket, Exult_server::combo_pick,
					0, t.tx, t.ty, t.tz, id.get_shapenum(),
					 id.get_framenum(), 0, name) == -1)
				cout << "Error sending shape to ExultStudio" 
								<< endl;
			return;
			}
#endif
		// Check path, except if an NPC, sign, or if editing.
	    	if (obj && !obj->as_actor() &&
			!cheat.in_map_editor() &&
			!Is_sign(obj->get_shapenum()) &&
			!Fast_pathfinder_client::is_grabable(
				main_actor->get_tile(),
				obj->get_tile()))
			{
			Mouse::mouse->flash_shape(Mouse::blocked);
			return;
			}
		}
	if (!obj)
		return;			// Nothing found.
	if (combat && !gump &&		// In combat?
	    !gump_man->gump_mode())
		{
		Actor *npc = obj->as_actor();
					// But don't attack party members.
		if ((!npc || !npc->is_in_party()) &&
					// Or bodies.
						!Is_body(obj->get_shapenum()))
			{		// In combat mode.
			// Want everyone to be in combat.
			combat = 0;
			main_actor->set_target(obj);
			toggle_combat();
					// Being a bully?
			bool bully = false;
			if (npc)
				{
				int align = npc->get_alignment();
				bully = npc->get_npc_num() > 0 &&
					(align == Actor::friendly ||
						align == Actor::neutral);
				}
			if (bully && obj->get_info().get_shape_class() ==
							Shape_info::human &&
			   Game::get_game_type() == BLACK_GATE)
				attack_avatar(1 + rand()%3);
			return;
			}
		}
	remove_text_effects();		// Remove text msgs. from screen.
#ifdef DEBUG
	cout << "Object name is " << obj->get_name() << endl;
#endif
	usecode->init_conversation();
	obj->activate(usecode);
	npc_prox->wait(4);		// Delay "barking" for 4 secs.
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
		npc_prox->add(Game::get_ticks(), npc);
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
	unsigned long curtime = Game::get_ticks();
	for (int cy = from_cy; cy != stop_cy; cy = INCR_CHUNK(cy))
		for (int cx = from_cx; cx != stop_cx; cx = INCR_CHUNK(cx))
			for (Npc_actor *npc = get_chunk(cx, cy)->get_npcs();
						npc; npc = npc->get_next())
				{
				if (!npc->is_nearby())
					{
					npc->set_nearby();
					npc_prox->add(curtime, npc);
					}
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
	int backwards,			// Extra periods to look backwards.
	bool repaint
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

	if (repaint)
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
 *	Get guard shape.
 */

int Get_guard_shape
	(
	Tile_coord pos			// Position to use.
	)
	{
	if (!GAME_SI)			// Default (BG).
		return (0x3b2);
					// Moonshade?
	if (pos.tx >= 2054 && pos.ty >= 1698 &&
	    pos.tx < 2590 && pos.ty < 2387)
		return 0x103;		// Ranger.
					// Fawn?
	if (pos.tx >= 895 && pos.ty >= 1604 &&
	    pos.tx < 1173 && pos.ty < 1960)
		return 0x17d;		// Fawn guard.
	return 0xe4;			// Pikeman.
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
		if (npc->is_monster() || npc->is_in_party() ||
		    (npc->get_framenum()&15) == Actor::sleep_frame ||
		    npc->get_npc_num() >= num_npcs1)
			continue;
		int dist = npc->distance(main_actor);
		if (dist < best_dist && Fast_pathfinder_client::is_grabable(
			npc->get_tile(),
			main_actor->get_tile()))
			{
			closest_npc = npc;
			best_dist = dist;
			}
		}
	if (!closest_npc)
		return;			// Didn't get caught.
	int dir = closest_npc->get_direction(main_actor);
					// Face avatar.
	closest_npc->change_frame(closest_npc->get_dir_framenum(dir,
							Actor::standing));
	theft_warnings++;
	if (theft_warnings < 3 + rand()%3)
		{			// Just a warning this time.
		closest_npc->say(first_theft, last_theft);
		return;
		}
	closest_npc->say(first_call_guards, last_call_guards);
					// Show guard running up.
	int gshape = Get_guard_shape(main_actor->get_tile());
					// Create it off-screen.
	Monster_actor *guard = Monster_actor::create(gshape,
		main_actor->get_tile() + Tile_coord(128, 128, 0));
	add_nearby_npc(guard);
	Tile_coord actloc = main_actor->get_tile();
	Tile_coord dest = Map_chunk::find_spot(actloc, 5, 
			guard->get_shapenum(), guard->get_framenum(), 1);
	if (dest.tx != -1)
		{
		int dir = Get_direction(dest.ty - actloc.ty,
						actloc.tx - dest.tx);
					
		char frames[2];		// Use frame for starting attack.
		frames[0] = guard->get_dir_framenum(dir, Actor::standing);
		frames[1] = guard->get_dir_framenum(dir, 3);
		Actor_action *action = new Sequence_actor_action(
				new Frames_actor_action(frames, 2),
				new Usecode_actor_action(0x625, guard,
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
	int gshape = Get_guard_shape(main_actor->get_tile());
	while (create_guards--)
		{
					// Create it off-screen.
		Monster_actor *guard = Monster_actor::create(gshape,
			main_actor->get_tile() + Tile_coord(128, 128, 0));
		add_nearby_npc(guard);
		guard->set_target(main_actor, true);
		guard->approach_another(main_actor);
		}

	Actor_vector npcs;		// See if someone is nearby.
	main_actor->find_nearby_actors(npcs, c_any_shapenum, 20);
	for (Actor_vector::const_iterator it = npcs.begin(); 
							it != npcs.end();++it)
		{
		Actor *npc = (Actor *) *it;
					// No monsters, except guards.
		if ((npc->get_shapenum() == gshape || !npc->is_monster()) && 
		    !npc->is_in_party())
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
	Audio::get_ptr()->resume_audio();
	focus = 1; 
	tqueue->resume(Game::get_ticks());
	}
void Game_window::lose_focus
	(
	)
	{
	cout << "Game paused" << endl;

	string str;
	config->value("config/audio/disablepause", str, "no");
	if (str == "no")
		Audio::get_ptr()->pause_audio();

	focus = false; 
	tqueue->pause(Game::get_ticks());
	}


/*
 *	Prepare for game
 */

void Game_window::setup_game
	(
	)
	{
	map->init();
				// Init. current 'tick'.
	Game::set_ticks(SDL_GetTicks());
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

	// Fade out & clear screen before palette change
	pal->fade_out(c_fade_out_time);
	clear_screen(true);
#ifdef RED_PLASMA
	load_palette_timer = 0;
#endif

	// note: we had to stop the plasma here already, because init_readied
	// and activate_eggs may update the screen through usecode functions
	// (Helm of Light, for example)

	Actor *party[9];
	int cnt = get_party(party, 1);	// Get entire party.
	for (int i = 0; i < cnt; i++)	// Init. rings.
	{
		party[i]->init_readied();
	}
	faded_out = 0;
	time_stopped = 0;
//+++++The below wasn't prev. done by ::read(), so maybe it should be
//+++++controlled by a 'first-time' flag.
				// Want to activate first egg.
	Map_chunk *olist = get_chunk(
			main_actor->get_cx(), main_actor->get_cy());
	olist->setup_cache();

	Tile_coord t = main_actor->get_tile();
	olist->activate_eggs(main_actor, t.tx, t.ty, t.tz, -1, -1);
	
	// Force entire repaint.
	set_all_dirty();
	gump_man->close_all_gumps(true);		// Kill gumps.
	Face_stats::load_config(config);

	// Set palette for time-of-day.
	clock.set_palette();
}

/*
 *	Text-drawing methods:
 */
int Game_window::paint_text_box(int fontnum, const char *text, 
		int x, int y, int w, int h, int vert_lead, int pbreak, int shading)
	{ if(shading>=0)
		win->fill_translucent8(0, w, h, x, y, xforms[shading]);
	  return fonts->paint_text_box(win->get_ib8(),
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
					// Cancel scripts 4 chunks from this.
	Usecode_script::purge(Tile_coord(newx*c_tiles_per_chunk,
			newy*c_tiles_per_chunk, 0), 4*c_tiles_per_chunk);
	int nearby[5][5];		// Chunks within 3.
	// Set to 0
	// No casting _should_ be necessary at this point.
	// Who needs this?
	memset(reinterpret_cast<char*>(nearby), 0, sizeof(nearby));
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
		Tile_coord t = (*it)->get_tile();
		cout << "Culling object: " << (*it)->get_name() << "@" << 
			t.tx << "," << t.ty << "," << t.tz <<endl;
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
	else // Default:  if (Game::get_game_type()==SERPENT_ISLE)
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
