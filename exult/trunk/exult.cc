/**
 **	Exult.cc - Multiplatform Ultima 7 game engine
 **
 **	Written: 7/22/98 - JSF
 **/

/*
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
#  include <cctype>
#endif

#include "SDL.h"

#define Font _XFont_
#include "SDL_syswm.h"
#undef Font

#ifdef USE_EXULTSTUDIO  /* Only needed for communication with exult studio */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include "xdrag.h"
#include "server.h"
#endif

#include "Audio.h"
#include "Configuration.h"
#include "Gump_manager.h"
#include "Scroll_gump.h"
#include "actors.h"
#include "args.h"
#include "cheat.h"
#include "effects.h"
#include "exult.h"
#include "exultmenu.h"
#include "fnames.h"
#include "font.h"
#include "game.h"
#include "gamewin.h"
#include "gump_utils.h"
#include "keys.h"
#include "mouse.h"
#include "ucmachine.h"
#include "utils.h"

using std::atof;
using std::cerr;
using std::cout;
using std::endl;
using std::atexit;
using std::exit;
using std::toupper;
using std::string;
using std::vector;
using std::snprintf;

Configuration *config = 0;
KeyBinder *keybinder = 0;

/*
 *	Globals:
 */
Game_window *gwin = 0;
quitting_time_enum quitting_time = QUIT_TIME_NO;

bool intrinsic_trace = false;		// Do we trace Usecode-intrinsics?
bool usecode_trace = false;		// Do we trace Usecode-instruction?

const std::string c_empty_string;

#if USECODE_DEBUGGER
bool	usecode_debugging=false;	// Do we enable the usecode debugger?
#endif



struct resolution {
	int x;
	int y;
	int scale;
} res_list[] = { 
	{ 320, 200, 1 },
	{ 320, 240, 1 },
	{ 400, 300, 1 },
	{ 320, 200, 2 },
	{ 320, 240, 2 },
	{ 400, 300, 2 },
	{ 512, 384, 1 },
	{ 640, 480, 1 },
	{ 800, 600, 1 }
};
int num_res = sizeof(res_list)/sizeof(struct resolution);
int current_res = 0;

#ifdef XWIN
int xfd = 0;			// X connection #.
static class Xdnd *xdnd = 0;
#endif


/*
 *	Local functions:
 */
static int exult_main(void);
static void Init();
static int Play();
static int Get_click(int& x, int& y, char *chr);
static int find_resolution(int w, int h, int s);
static void set_resolution (int new_res, bool save);
static void Drop_dragged_shape(int shape, int frame, int x, int y);
static void Drop_dragged_chunk(int chunknum, int x, int y);
static void BuildGameMap();
static void Handle_events();
static void Handle_event(SDL_Event& event);


/*
 *	Statics:
 */
static bool show_mouse = true;		// display mouse in main loop?
static bool dragging = false;		// Object or gump being moved.
static bool dragged = false;		// Flag for when obj. moved.

static string arg_gamename = "default";	// cmdline arguments
static int arg_buildmap = -1;



/*
 *	A handy breakpoint.
 */

static void Breakpoint
	(
	)
	{
	return;
	}

/*
 *	Main program.
 */

int main
	(
	int argc,
	char *argv[]
	)
{

#ifdef BEOS
	// get exult path
	int counti;
	char datapath[256];
	for (counti=strlen(argv[0]) ; argv[0][counti]!='/' ; counti--);
	strncpy(datapath, argv[0], counti);
	chdir(datapath);
#endif


	bool	needhelp=false;
	int		result;
	Args    parameters;

	// Declare everything from the commandline that we're interested in.
	parameters.declare("-h",&needhelp,true);
	parameters.declare("--help",&needhelp,true);
	parameters.declare("/?",&needhelp,true);
	parameters.declare("/h",&needhelp,true);
	parameters.declare("-game",&arg_gamename,"default");
	parameters.declare("-buildmap",&arg_buildmap,-1);

	// Process the args
	parameters.process(argc,argv);

	if(needhelp)
	{
		cerr << "Usage: exult [--help|-h|/?|/h] [-game GAMENAME] [-buildmap 0|1|2]" << endl <<
			"--help\t\tShow this information" << endl <<
			"-game GAMENAME\tSet the game data name to play" << endl <<
			"\t(refer to the documentation)" << endl <<
			"-buildmap\tCreate a fullsize map of the game world in u7map??.pcx" << endl <<
			"\t(0 = all roofs, 1 = no level 2 roofs, 2 = no roofs)" << endl <<
			"\t(WARNING: requires big amounts of RAM, HD space and time!)" << endl;
			
		exit(1);
	}
	
	try
	{
		result = exult_main();
	}
	catch( const exult_exception & e )
	{
		cerr << "============================" << endl <<
			"An exception occured: " << endl << 
			e.what() << endl << 
			"errno: " << e.get_errno() << endl;
		if( e.get_errno() != 0)
			perror("Error Description");
		cerr << "============================" << endl;
	}
	
	return result;
}


/*
 *	Main program.
 */

int exult_main(void)
{
	string data_path;

	cout << "Exult V" << VERSION << "." << endl;

	// Read in configuration file
	config = new Configuration;
	config->read_config_file(USER_CONFIGURATION_FILE);

	// Setup virtual directories
	config->value("config/disk/data_path",data_path,EXULT_DATADIR);
	cout << "Data path = " << data_path << endl;
	add_system_path("<DATA>", data_path);
	if (!U7exists("<DATA>/exult.flx"))
	{
		add_system_path("<DATA>", EXULT_DATADIR);
		if (!U7exists("<DATA>/exult.flx"))
		{
			add_system_path("<DATA>", "data");
			if(!U7exists("<DATA>/exult.flx"))
			{
				// We've tried them all...
				cerr << "Could not find 'exult.flx' anywhere." << endl;	
				cerr << "Please make sure Exult is correctly installed," << endl;
				cerr << "and the Exult data path is specified in the configuration file." << endl;
				cerr << "(See the README file for more information)" << endl;
				exit(-1);
			}
		}
	}
	add_system_path("<STATIC>", "static");
	add_system_path("<GAMEDAT>", "gamedat");
//	add_system_path("<SAVEGAME>", "savegame");
	add_system_path("<SAVEGAME>", ".");

	// Convert from old format if needed
	vector<string> vs=config->listkeys("config/disk/game",false);
	if(vs.size()==0)
	{
		// Convert from the older format
		string data_directory;
		config->value("config/disk/u7path",data_directory,".");
		config->set("config/disk/game/blackgate/path",data_directory,true);
		const string	s("blackgate");
		config->set("config/disk/game/blackgate/title",s,true);
		vs.push_back(s);
	}

	// This is for Serpent Isle Paperdolls in Black Gate
	string	serp_static;
	config->value("config/disk/game/serpentisle/path",serp_static,".");
	serp_static += "/static";
	add_system_path("<SERPENT_STATIC>", serp_static.c_str());

	string	tracing;
	config->value("config/debug/trace/intrinsics",tracing,"no");
	if(tracing=="yes")
		intrinsic_trace=true;	// Enable tracing of intrinsics

	string uctrace;
	config->value("config/debug/trace/usecode", uctrace,"no");
	if (uctrace=="yes")
		usecode_trace=true;	// Enable tracing of UC-instructions
		

#if USECODE_DEBUGGER
	string	u_debugging;
	config->value("config/debug/debugger/enable",u_debugging,"no");
	if(u_debugging=="yes")
		usecode_debugging=true;	// Enable usecode debugger
	initialise_usecode_debugger();
#endif
	cheat.init();

	Init();				// Create main window.

	cheat.finish_init();

	Mouse::mouse = new Mouse(gwin);
	Mouse::mouse->set_shape(Mouse::hand);

	int result = Play();		// start game
	return result;
}


/*
 *	Initialize and create main window.
 */
static void Init
	(
	)
{
	Uint32 init_flags = SDL_INIT_VIDEO|SDL_INIT_TIMER;
#ifdef NO_SDL_PARACHUTE
	init_flags |= SDL_INIT_NOPARACHUTE;
#endif
	if (SDL_Init(init_flags) < 0)
	{
		cerr << "Unable to initialize SDL: " << SDL_GetError() << endl;
		exit(-1);
	}
	std::atexit(SDL_Quit);
	
	SDL_SysWMinfo info;		// Get system info.
        SDL_GetWMInfo(&info);
#ifdef USE_EXULTSTUDIO
					// Want drag-and-drop events.
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif
	SDL_ShowCursor(0);
	SDL_VERSION(&info.version);

	int w, h, sc, sclr;

	// Default resolution is 320x200 with 2x scaling
	w = 320;
	h = 200;
	sc = 2;
	sclr = Image_window::SaI;

	int sw, sh, scaleval;
	string gr, gg, gb, scaler;
	config->value("config/video/width", sw, w);
	config->value("config/video/height", sh, h);
	config->value("config/video/scale_method", scaler, "---");
	config->value("config/video/gamma/red", gr, "1.0");
	config->value("config/video/gamma/green", gg, "1.0");
	config->value("config/video/gamma/blue", gb, "1.0");

	config->value("config/video/scale", scaleval, sc);
	if (scaler == "bilinear") sclr = Image_window::bilinear;
	else if (scaler == "interlaced") sclr = Image_window::interlaced;
	else if (scaler == "point") sclr = Image_window::point;
	else if (scaler == "SuperEagle") sclr = Image_window::SuperEagle;
	else config->set("config/video/scale_method","2xSaI",true);

	Image_window8::set_gamma(atof(gr.c_str()), atof(gg.c_str()), atof(gb.c_str()));	
	gwin = new Game_window(sw, sh, scaleval, sclr);
	current_res = find_resolution(sw, sh, scaleval);
	Audio::Init();

	string disable_fades;
	config->value("config/video/disable_fades", disable_fades, "no");
	if (disable_fades == "yes")
		gwin->set_fades_enabled(false);


	if (arg_buildmap >= 0)
		BuildGameMap();

	SDL_SetEventFilter(0);
	// Show the banner
	Exult_Game mygame;
	game = 0;

	store_system_paths();

	do {
		reset_system_paths();
		fontManager.reset();
		U7FileManager::get_ptr()->reset();

		if(game)
			delete game;
		ExultMenu exult_menu(gwin);
		mygame = exult_menu.run();
		Game::create_game(mygame);

		string yn;
		
					// Skip splash screen?
		config->value("config/gameplay/skip_splash", yn, "no");
		if(yn == "no") 
		game->play_intro();
	} while(!game->show_menu());
	gwin->init_files();
	gwin->setup_game();		// This will start the scene.
					// Get scale factor for mouse.
#ifdef USE_EXULTSTUDIO
        SDL_GetWMInfo(&info);
        xfd = ConnectionNumber(info.info.x11.display);
	Server_init();			// Initialize server (for map-editor).
	xdnd = new Xdnd(info.info.x11.display, info.info.x11.wmwindow,
		info.info.x11.window, Drop_dragged_shape, Drop_dragged_chunk);
#endif
}

/*
 *	Play game.
 */

static int Play()
{
	do
	{
		quitting_time = QUIT_TIME_NO;
		Handle_events();
		if( quitting_time == QUIT_TIME_RESTART )
		{
			Mouse::mouse->hide();	// Turn off mouse.
			gwin->read();	// Restart
			gwin->setup_game();
		}
	}
	while (quitting_time == QUIT_TIME_RESTART);
	delete gwin;
	delete Mouse::mouse;

	Audio::Destroy();	// Deinit the sound system.

	delete config;
	return (0);
}



/*
 *	Handle events until a flag is set.
 */

static void Handle_events
	(
	)
	{
	uint32 last_repaint = 0;	// For insuring animation repaints.
	uint32 last_rotate = 0;
	/*
	 *	Main event loop.
	 */
	while (!quitting_time)
		{
#ifdef USE_EXULTSTUDIO
		Server_delay();		// Handle requests.
#else
		Delay();		// Wait a fraction of a second.
#endif
		// Mouse scale factor
		int scale = gwin->get_win()->get_scale();

		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;

		SDL_Event event;
		while (!quitting_time && SDL_PollEvent(&event))
			Handle_event(event);
					// Get current time.
		uint32 ticks = SDL_GetTicks();
					// Animate unless dormant.
		if (gwin->have_focus() && !dragging)
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
		if (ticks > last_repaint + 50 || gwin->was_painted())
					// This avoids jumpy walking:
			{
			gwin->paint_dirty();
			last_repaint = ticks;
			int x, y;// Check for 'stuck' Avatar.
			if (!gwin->is_moving() &&
			    !gwin->was_teleported())
				{
				int ms = SDL_GetMouseState(&x, &y);
				if ((SDL_BUTTON(3) & ms) &&
						(Game::get_game_type() == SERPENT_ISLE ||
						gwin->get_usecode()->get_global_flag(
							Usecode_machine::did_first_scene)))
					gwin->start_actor(x / scale, 
							  y / scale, 
							  Mouse::mouse->avatar_speed);
				}
			}

		if (show_mouse)
			Mouse::mouse->show();	// Re-display mouse.

					// Rotate less often if scaling.
		if (ticks > last_rotate + (100 << (scale==1?0:1)))
			{		// (Blits in simulated 8-bit mode.)
			gwin->get_win()->rotate_colors(0xf8, 4, 0);
			gwin->get_win()->rotate_colors(0xf4, 4, 0);
			gwin->get_win()->rotate_colors(0xf0, 4, 0);
			gwin->get_win()->rotate_colors(0xe8, 8, 0);
			gwin->get_win()->rotate_colors(0xe0, 8, 1);
			last_rotate = ticks;
			if (gwin->get_win()->is_palettized())	// Non palettized requires explicit blit.
				gwin->set_painted();
			}
		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)	// If not, did mouse change?
			Mouse::mouse->blit_dirty();
		}
	}

/*
 *	Handle an event.  This should work for all platforms, and should only
 *	be called in 'normal' and 'gump' modes.
 */

static void Handle_event
	(
	SDL_Event& event
	)
	{
	// Mouse scale factor
	int scale = gwin->get_win()->get_scale();
	Gump_manager *gump_man = gwin->get_gump_man();

					// For detecting double-clicks.
	static uint32 last_b1_click = 0, last_b3_click = 0;
	//cout << "Event " << (int) event.type << " received"<<endl;
	switch (event.type)
		{
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == 1)
			{
			dragging = gwin->start_dragging(
					event.button.x / scale,
					event.button.y / scale);
			//Mouse::mouse->set_shape(Mouse::hand);
			dragged = false;
			}
					// Move sprite toward mouse
					//  when right button pressed.
		if (event.button.button == 3)
			{		// Try removing old queue entry.
			gwin->get_tqueue()->remove(gwin->get_main_actor());
			gwin->start_actor(event.button.x / scale, 
				event.button.y / scale, Mouse::mouse->avatar_speed);
			}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 3)
			{
			uint32 curtime = SDL_GetTicks();
					// Last click within .5 secs?
			if (curtime - last_b3_click < 500)
				gwin->start_actor_along_path(
					event.button.x / scale, 
					event.button.y / scale, Mouse::mouse->avatar_speed);
			else
				gwin->stop_actor();
			last_b3_click = curtime;
			}
		else if (event.button.button == 1)
			{
			uint32 curtime = SDL_GetTicks();
			bool click_handled = false;
			if (dragging) {
				click_handled = gwin->drop_dragged(
					event.button.x / scale, 
					event.button.y / scale, dragged);
			}
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500)
				{
				dragging = false;
				gwin->double_clicked(event.button.x / scale, 
						event.button.y / scale);
				Mouse::mouse->set_speed_cursor();
				break;
				}
			last_b1_click = curtime;
			if (!click_handled)
					// Identify item(s) clicked on.
				gwin->show_items(event.button.x / scale, 
						event.button.y / scale);
			dragging = false;
			}
		break;
	case SDL_MOUSEMOTION:
		{
		Gump *gmp = 0;

		Mouse::mouse->move(event.motion.x / scale, event.motion.y / scale);
		Mouse::mouse->set_speed_cursor();
		Mouse::mouse_update = true;	// Need to blit mouse.

					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			dragged = gwin->drag(event.motion.x / scale, 
						event.motion.y / scale);
					// Dragging with right?
		if ((event.motion.state & SDL_BUTTON(3)) &&
					// But not right after teleport.
		    !gwin->was_teleported())
			gwin->start_actor(event.motion.x / scale, 
			event.motion.y / scale, Mouse::mouse->avatar_speed);
		break;
		}
	case SDL_ACTIVEEVENT:

		if (event.active.state & SDL_APPMOUSEFOCUS)
			{
			if (event.active.gain)
				{
				int x, y;
				SDL_GetMouseState(&x, &y);
				Mouse::mouse->set_location(x/scale, y/scale);
				}
			gwin->set_painted();
			}

		if (event.active.state & SDL_APPINPUTFOCUS)
			{
			if (event.active.gain)
				gwin->get_focus();
			else
				gwin->lose_focus();
			}
#if 0
		if (event.active.state & SDL_APPACTIVE)
					// Became active.
			if (event.active.gain)
				gwin->init_actors();
#endif
		break;
#if 0
	case ConfigureNotify:		// Resize.
		gwin->resized(event.xconfigure.window,
			event.xconfigure.width, event.xconfigure.height);
		break;
#endif
	case SDL_QUIT:
		Okay_to_quit();
		break;
	case SDL_KEYDOWN:		// Keystroke.
			//+++++++++TESTING:
//		Audio::get_ptr()->playwave("/home/jeff/exult/test.wav", false);
		keybinder->HandleEvent(event);
		break;
#ifdef USE_EXULTSTUDIO
	case SDL_SYSWMEVENT:
		{
		XEvent& ev = event.syswm.msg->event.xevent;
		if (ev.type == ClientMessage)
			xdnd->client_msg((XClientMessageEvent&) ev);
		else if (ev.type == SelectionNotify)
			xdnd->select_msg((XSelectionEvent&) ev);
		break;
		}
#endif
#if 0
//#ifdef WIN32
	case SDL_SYSWMEVENT:
//		printf("SYSWMEVENT received, %x\n", event.syswm.msg->msg);
		if (event.syswm.msg->msg == MM_MCINOTIFY) {
#if DEBUG
			cerr << "MM_MCINOTIFY message received"<<endl;
#endif
			((Windows_MCI*)(Audio::get_ptr()->get_midi()))->callback(event.syswm.msg->wParam, 
							event.syswm.msg->hwnd);
		}
		break;
#endif
		}
	}


/*
 *	Wait for a click, or optionally, a kbd. chr.
 *
 *	Output:	0 if user hit ESC.
 */
static int Get_click
	(
	int& x, int& y,
	char *chr			// Char. returned if not null.
	)
	{
	while (1)
		{
		SDL_Event event;
		Delay();		// Wait a fraction of a second.

		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;

		// Mouse scale factor
		int scale = gwin->get_win()->get_scale();

		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == 1)
					{
					x = event.button.x / scale;
					y = event.button.y / scale;
					if (chr) *chr = 0;
					return (1);
					}
					// May have been moving before.
				else if (event.button.button == 3)
					// Just stop.  Don't get followers!
					gwin->get_main_actor()->stop();
				break;
			case SDL_MOUSEMOTION:
				Mouse::mouse->move(event.motion.x / scale, 
						event.motion.y / scale);
				Mouse::mouse_update = true;
				break;
			case SDL_KEYDOWN:
				{
				//+++++ convert to unicode first?
				int c = event.key.keysym.sym;
				switch(c) {
				case SDLK_ESCAPE:
					return 0;
				case SDLK_RSHIFT: case SDLK_LSHIFT:
				case SDLK_RCTRL: case SDLK_LCTRL:
				case SDLK_RALT: case SDLK_LALT:
				case SDLK_RMETA: case SDLK_LMETA:
				case SDLK_RSUPER: case SDLK_LSUPER:
				case SDLK_NUMLOCK: case SDLK_CAPSLOCK:
				case SDLK_SCROLLOCK:
					break;
				default:
					if ((c == 's') && 
 					   (event.key.keysym.mod & KMOD_ALT) &&
					   (event.key.keysym.mod & KMOD_CTRL)){
						make_screenshot(true);
						break;
					}
					if (chr)// Looking for a character?
					{
						*chr = (event.key.keysym.mod & 
							KMOD_SHIFT)
							? toupper(c) : c;
						return (1);
					}
					break;
				}
				break;
				}
			case SDL_ACTIVEEVENT:
				if (event.active.state & SDL_APPINPUTFOCUS)
					{
					if (event.active.gain)
						gwin->get_focus();
					else
						gwin->lose_focus();
					}
				}
		Mouse::mouse->show();		// Turn on mouse.

		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)
			Mouse::mouse->blit_dirty();
		}
	return (0);			// Shouldn't get here.
	}

/*
 *	Get a click, or, optionally, a keyboard char.
 *
 *	Output:	0 if user hit ESC.
 *		Chr gets keyboard char., or 0 if it's was a mouse click.
 */

int Get_click
	(
	int& x, int& y,			// Location returned (if not ESC).
	Mouse::Mouse_shapes shape,	// Mouse shape to use.
	char *chr			// Char. returned if not null.
	)
	{
	if (chr)
		*chr = 0;		// Init.
	Mouse::Mouse_shapes saveshape = Mouse::mouse->get_shape();
	if (shape != Mouse::dontchange)
		Mouse::mouse->set_shape(shape);
	Mouse::mouse->show();
	gwin->show(1);			// Want to see new mouse.
	int ret = Get_click(x, y, chr);
	Mouse::mouse->set_shape(saveshape);
	return (ret);
	}

int Put_click
	(
	int x, int y, int button
	)
	{
	SDL_Event e;
	e.type = SDL_MOUSEBUTTONUP;
	e.button.button = button;
	int scale = gwin->get_win()->get_scale();
	e.button.x = x * scale;
	e.button.y = y * scale;
	return !SDL_PushEvent(&e);
	}

/*
 *	Wait for someone to stop walking.  If a timeout is given, at least
 *	one animation cycle will still always occur.
 */

void Wait_for_arrival
	(
	Actor *actor,			// Whom to wait for.
	Tile_coord dest,		// Where he's going.
	long maxticks			// Max. # msecs. to wait, or 0.
	)
	{
	// Mouse scale factor
	int scale = gwin->get_win()->get_scale();

	unsigned char os = Mouse::mouse->is_onscreen();
	uint32 last_repaint = 0;	// For insuring animation repaints.
	Actor_action *orig_action = actor->get_action();
	uint32 stop_time = SDL_GetTicks() + maxticks;
	bool timeout = false;
	while (actor->is_moving() && actor->get_action() == orig_action &&
	       actor->get_abs_tile_coord() != dest && !timeout)
		{
		Delay();		// Wait a fraction of a second.

		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEMOTION:
				Mouse::mouse->move(event.motion.x / scale,
						 event.motion.y / scale);
				Mouse::mouse_update = true;
				break;
				}
					// Get current time, & animate.
		uint32 ticks = SDL_GetTicks();
		if (maxticks && ticks > stop_time)
			timeout = true;
		if (gwin->have_focus())
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
		if (ticks > last_repaint + 50 || gwin->was_painted())
			{
			gwin->paint_dirty();
			last_repaint = ticks;
			}

		Mouse::mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)	// If not, did mouse change?
			Mouse::mouse->blit_dirty();
		}

	if (!os)
		Mouse::mouse->hide();

	}

int find_resolution(int w, int h, int s)
{
	int res = 0;
	for(int i=0; i<num_res; i++) {
		if(res_list[i].x==w && res_list[i].y==h && res_list[i].scale==s)
			res = i;
	}
	return res;
}


void set_resolution (int new_res, bool save)
{
	if(new_res>=0 && new_res<num_res) {
		int scaler = gwin->get_win()->get_scaler();
		current_res = new_res;
		gwin->resized(res_list[current_res].x,
			res_list[current_res].y,
			res_list[current_res].scale, scaler);
		if(save) {
			char val[20];
			snprintf(val, 20, "%d", res_list[current_res].x);
			config->set("config/video/width",val,true);
			snprintf(val, 20, "%d", res_list[current_res].y);
			config->set("config/video/height",val,true);
			snprintf(val, 20, "%d", res_list[current_res].scale);
			config->set("config/video/scale",val,true);

			// Scaler
			if (scaler == Image_window::bilinear)
				config->set("config/video/scale_method","bilinear",true);
			else if (scaler == Image_window::interlaced)
				config->set("config/video/scale_method","interlaced",true);
			else if (scaler == Image_window::SuperEagle)
				config->set("config/video/scale_method","SuperEagle",true);
			else if (scaler == Image_window::point)
				config->set("config/video/scale_method","point",true);
			else
				config->set("config/video/scale_method","2xSaI",true);
		}
	}
}

void increase_resolution()
{
	if (!cheat()) return;

	current_res++;
	if(current_res>=num_res)
		current_res = 0;
	set_resolution(current_res,false);
}

void decrease_resolution()
{
	if (!cheat()) return;

	current_res--;
	if(current_res<0)
		current_res = num_res-1;
	set_resolution(current_res,false);
}

void make_screenshot (bool silent)
{
	char fn[15];
	int i;
	FILE *f;
	bool namefound = false;

	// look for the next available exult???.pcx file
	for (i = 0; i < 1000 && !namefound; i++) {
		snprintf(fn, 15, "exult%03i.pcx", i);
		f = fopen(fn, "rb");
		if (f) {
			fclose(f);
		} else {
			namefound = true;
		}
	}

	if (!namefound) {
		if (!silent) gwin->center_text("Too many screenshots");
	} else {
		SDL_RWops *dst = SDL_RWFromFile(fn, "wb");

		if (gwin->get_win()->screenshot(dst)) {
			cout << "Screenshot saved in " << fn << endl;
			if (!silent) gwin->center_text("Screenshot");
		} else {
			if (!silent) gwin->center_text("Screenshot failed");
		}
	}	
}

void change_gamma (bool down)
{
	float r,g,b;
	char text[256];
	float delta = down?0.05:-0.05;
	Image_window8::get_gamma(r, g, b);	
	Image_window8::set_gamma(r+delta, g+delta, b+delta);	
	gwin->set_palette (-1, -1);

	// Message
	Image_window8::get_gamma(r, g, b);	
	snprintf (text, 256, "Gamma Set to R: %01.2f G: %01.2f B: %01.2f", r, g, b);
	gwin->center_text(text);	

	snprintf (text, 256, "%.2f", r);
	config->set("config/video/gamma/red", text, true);
	snprintf (text, 256, "%.2f", g);
	config->set("config/video/gamma/green", text, true);
	snprintf (text, 256, "%.2f", b);
	config->set("config/video/gamma/blue", text, true);

}

void BuildGameMap()
{
	int w, h, sc, sclr;

	// create 2048x2048 screenshots of the full Ultima 7 map.
	// WARNING!! Takes up lots of memory and diskspace!
	if (arg_buildmap >= 0) {
		int maplift = 16;
		Exult_Game gametype = BLACK_GATE;
		switch(arg_buildmap) {
			case 0: maplift = 16; break;
			case 1: maplift = 10; break;
			case 2: maplift = 5; break;
		}
		if (arg_gamename == "serpentisle")
			gametype = SERPENT_ISLE;
		h = w = c_tilesize * c_tiles_per_schunk; sc = 1, sclr = Image_window::point;
		Image_window8::set_gamma(1, 1, 1);
		gwin = new Game_window(w, h, sc, sclr);
		current_res = find_resolution(w, h, sc);
		Game::create_game(gametype);
		gwin->init_files();	
		for (int x = 0; x < c_num_chunks / c_chunks_per_schunk; x++) {
			for (int y = 0; y < c_num_chunks / c_chunks_per_schunk; y++) {
				gwin->paint_map_at_tile(0,0,w,h,x * c_tiles_per_schunk, y * c_tiles_per_schunk, maplift);
				char fn[15];
				snprintf(fn, 15, "u7map%x%x.pcx", x, y);
				SDL_RWops *dst = SDL_RWFromFile(fn, "wb");
				cerr << x << "," << y << ": ";
				gwin->get_win()->screenshot(dst);
			}
		}
		exit(0);
	}
}


#ifdef USE_EXULTSTUDIO
/*
 *	Drop a shape dragged from a shape-chooser via drag-and-drop.  Dnd is
 *	only supported under X for now.
 */

static void Drop_dragged_shape
	(
	int shape, int frame,		// What to create.
	int x, int y			// Mouse coords. within window.
	)
	{
	int scale = gwin->get_win()->get_scale();
	if (!cheat.in_map_editor())	// Get into editing mode.
		cheat.toggle_map_editor();
	x /= scale;			// Watch for scaled window.
	y /= scale;
	cout << "Last drag pos: (" << x << ", " << y << ')' << endl;
	cout << "Create shape (" << shape << '/' << frame << ')' <<
								endl;
					// Create object.
	Shape_info& info = gwin->get_info(shape);
	int sclass = info.get_shape_class();
					// Is it an ireg (changeable) obj?
	bool ireg = (sclass != Shape_info::unusable &&
		     sclass != Shape_info::building);
	Game_object *newobj;
	if (ireg)
		newobj = gwin->create_ireg_object(info, shape, frame, 0, 0, 0);
	else
		newobj = new Ifix_game_object(shape, frame, 0, 0, 0);
					// First see if it's a gump.
	Gump *on_gump = ireg ? gwin->get_gump_man()->find_gump(x, y) : 0;
	if (on_gump)
		{
		if (!on_gump->add(newobj, x, y, x, y))
			delete newobj;
		else
			on_gump->paint(gwin);
		}
	else				// Try to drop at increasing hts.
		{
		int edit_lift = cheat.get_edit_lift();
		for (int lift = edit_lift; lift <= 11; lift++)
			if (gwin->drop_at_lift(newobj, x, y, lift))
				{	// Success.
				gwin->set_ifix_modified(newobj->get_cx(),
							newobj->get_cy());
				return;
				}
		delete newobj;	// Failed.
		}
	}

/*
 *	Drop a chunk dragged from a chunk-chooser via drag-and-drop.  Dnd is
 *	only supported under X for now.
 */

static void Drop_dragged_chunk
	(
	int chunknum,			// Index in 'u7chunks'.
	int x, int y			// Mouse coords. within window.
	)
	{
	int scale = gwin->get_win()->get_scale();
	if (!cheat.in_map_editor())	// Get into editing mode.
		cheat.toggle_map_editor();
	x /= scale;			// Watch for scaled window.
	y /= scale;
	cout << "Last drag pos: (" << x << ", " << y << ')' << endl;
	cout << "Set chunk (" << chunknum << ')' << endl;
					// Need chunk-coordinates.
	int tx = gwin->get_scrolltx() + x/c_tilesize,
	    ty = gwin->get_scrollty() + y/c_tilesize;
	int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
	gwin->set_chunk_terrain(cx, cy, chunknum);
	gwin->paint();
	}
#endif
