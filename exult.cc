/**
 **	Exult.cc - Multiplatform Ultima 7 game engine
 **
 **	Written: 7/22/98 - JSF
 **/

/*
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2002  The Exult Team
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

#include <SDL.h>

#define Font _XFont_
#include <SDL_syswm.h>
#undef Font

#ifdef USE_EXULTSTUDIO  /* Only needed for communication with exult studio */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef WIN32
#include "windrag.h"
#elif defined(XWIN)
#include "xdrag.h"
#endif
#include "server.h"
#include "chunks.h"
#include "chunkter.h"
#endif

#if (defined(USECODE_DEBUGGER) && defined(XWIN))
#include <csignal>
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
#include "gamemap.h"
#include "gump_utils.h"
#include "keyactions.h"
#include "keys.h"
#include "mouse.h"
#include "ucmachine.h"
#include "utils.h"
#include "version.h"
#include "u7drag.h"

#include "exult_flx.h"
#include "exult_bg_flx.h"
#include "exult_si_flx.h"
#include "crc.h"

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
int usecode_trace = 0;		// Do we trace Usecode-instructions?
							// 0 = no, 1 = short, 2 = long

// Save game compression level
int save_compression = 1;
bool ignore_crc = false;

const std::string c_empty_string;

#if 0 && USECODE_DEBUGGER
bool	usecode_debugging=false;	// Do we enable the usecode debugger?
extern void initialise_usecode_debugger(void);
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
#elif defined(WIN32)
static HWND hgwin;
static class Windnd *windnd = 0;
#endif


/*
 *	Local functions:
 */
static int exult_main(const char *);
static void Init();
static int Play();
static int Get_click(int& x, int& y, char *chr, bool drag_ok);
static int find_resolution(int w, int h, int s);
static void set_resolution (int new_res, bool save);
#ifdef USE_EXULTSTUDIO
static void Move_dragged_shape(int shape, int frame, int x, int y,
				int prevx, int prevy, bool show);
static void Move_dragged_combo(int xtiles, int ytiles, int tiles_right,
	int tiles_below, int x, int y, int prevx, int prevy, bool show);
static void Drop_dragged_shape(int shape, int frame, int x, int y, void *d);
static void Drop_dragged_chunk(int chunknum, int x, int y, void *d);
static void Drop_dragged_combo(int cnt, U7_combo_data *combo, 
							int x, int y, void *d);
#endif
static void BuildGameMap();
static void Handle_events();
static void Handle_event(SDL_Event& event);


/*
 *	Statics:
 */
static bool show_mouse = true;		// display mouse in main loop?
static bool dragging = false;		// Object or gump being moved.
static bool dragged = false;		// Flag for when obj. moved.
static bool run_bg = false;		// skip menu and run bg
static bool run_si = false;		// skip menu and run si

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
	bool	showversion=false;
	int		result;
	Args    parameters;

	// Declare everything from the commandline that we're interested in.
	parameters.declare("-h",&needhelp,true);
	parameters.declare("--help",&needhelp,true);
	parameters.declare("/?",&needhelp,true);
	parameters.declare("/h",&needhelp,true);
	parameters.declare("--bg",&run_bg,true);
	parameters.declare("--si",&run_si,true);
	parameters.declare("-v",&showversion,true);
	parameters.declare("--version",&showversion,true);
	parameters.declare("--game",&arg_gamename,"default");
	parameters.declare("--buildmap",&arg_buildmap,-1);
	parameters.declare("--nocrc",&ignore_crc,true);

	// Process the args
	parameters.process(argc,argv);

	if(needhelp)
	{
		cerr << "Usage: exult [--help|-h] [-v|--version] [--bg|--si] [--buildmap 0|1|2] [--nocrc]" << endl <<
			"--help\t\tShow this information" << endl <<
			"--bg\t\tSkip menu and run Black Gate" << endl <<
			"--si\t\tSkip menu and run Serpent Isle" << endl <<
			"--version\tShow version info" << endl <<
			"--buildmap\tCreate a fullsize map of the game world in u7map??.pcx" << endl <<
			"\t\t(0 = all roofs, 1 = no level 2 roofs, 2 = no roofs)" << endl <<
			"\t\tonly valid when used together with -bg or -si" << endl <<
			"\t\t(WARNING: requires big amounts of RAM, HD space and time!)" << endl <<
			"--nocrc\t\tDon't check crc's of .flx files" << endl;
			
		exit(1);
	}
	if (run_bg && run_si) {
		cerr << "Error: You may only specify either -bg or -si!" << 
									endl;
		exit(1);
	}

	if(showversion) {
		getVersionInfo(cerr);
		return 0;
	}
	
	try
	{
		result = exult_main(argv[0]);
	}
	catch( const quit_exception & e )
    {
        result = 0;
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

int exult_main(const char *runpath)
{
	string data_path;

	//cout << "Exult V" << VERSION << "." << endl;
	getVersionInfo(cout);

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
				char *sep = std::strrchr(runpath,'/');
				if (!sep) sep = std::strrchr(runpath,'\\');
				int plen = sep-runpath;
				char *dpath = new char[plen+10];
				std::strncpy(dpath, runpath, plen+1);
				dpath[plen+1] = 0;
				std::strcat(dpath,"data");
				cerr << "dpath = " << dpath << endl;
				add_system_path("<DATA>",dpath);
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
	}
	add_system_path("<STATIC>", "static");
	add_system_path("<GAMEDAT>", "gamedat");
//	add_system_path("<SAVEGAME>", "savegame");
	add_system_path("<SAVEGAME>", ".");


	// Check CRCs of our .flx files
	bool crc_ok = true;
	uint32 crc = crc32_syspath("<DATA>/exult.flx");
	if (crc != EXULT_FLX_CRC32) {
		crc_ok = false;
		cerr << "exult.flx has a wrong checksum!" << endl;
	}
	if (U7exists("<DATA>/exult_bg.flx")) {
		if (crc32_syspath("<DATA>/exult_bg.flx") != EXULT_BG_FLX_CRC32) {
			crc_ok = false;
			cerr << "exult_bg.flx has a wrong checksum!" << endl;
		}
	}
	if (U7exists("<DATA>/exult_si.flx")) {
		if (crc32_syspath("<DATA>/exult_si.flx") != EXULT_SI_FLX_CRC32) {
			crc_ok = false;
			cerr << "exult_si.flx has a wrong checksum!" << endl;
		}
	}

	bool config_ignore_crc;
	config->value("config/disk/no_crc",config_ignore_crc);
	ignore_crc |= config_ignore_crc;

	if (!ignore_crc && !crc_ok) {
		cerr << "This usually means the file(s) mentioned above are "
			 << "from a different version" << endl
			 << "of Exult than this one. Please re-install Exult" << endl
			 << endl
			 << "(Note: if you modified the .flx files yourself, "
			 << "you can skip this check" << endl
			 << "by passing the --nocrc parameter.)" << endl;
		
		return 1;
	}


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

	// Enable tracing of intrinsics?
	config->value("config/debug/trace/intrinsics",intrinsic_trace);

	// Enable tracing of UC-instructions?
	string uctrace;
	config->value("config/debug/trace/usecode", uctrace, "no");
	to_uppercase(uctrace);
	if (uctrace == "YES")
		usecode_trace = 1;
	else if (uctrace == "VERBOSE")
		usecode_trace = 2;
	else
		usecode_trace = 0;

	// Save game compression level
	config->value("config/disk/save_compression_level", save_compression, 1);
	if (save_compression < 0 || save_compression > 2) save_compression = 1;
	config->set("config/disk/save_compression_level", save_compression, true);

#if 0 && USECODE_DEBUGGER
	// Enable usecode debugger
	config->value("config/debug/debugger/enable",usecode_debugging);
	initialise_usecode_debugger();
#endif

#if (defined(USECODE_DEBUGGER) && defined(XWIN))
	signal(SIGUSR1, SIG_IGN);
#endif

	cheat.init();

	Init();				// Create main window.

	cheat.finish_init();

	Mouse::mouse = new Mouse(gwin);
	Mouse::mouse->set_shape(Mouse::hand);

	int result = Play();		// start game

#if defined(WIN32) && defined(USE_EXULTSTUDIO)
	// Currently, leaving the game results in destruction of the window.
	//  Maybe sometime in the future, there is an option like "return to
	//  main menu and select another scenario". Becaule DnD isn't registered until
	//  you really enter the game, we remove it here to prevent possible bugs
	//  invilved with registering DnD a second time over an old variable.
    RevokeDragDrop(hgwin);
	delete windnd;
#endif

	return result;
}

/*
 *	Calculate paths for the given game, using the config file and
 *	falling back to defaults if necessary.  These are stored in
 *	per-game system_path entries, which are then used later once the
 *	game is selected.
 */
void get_game_paths(const string &gametitle)
{
	std::string data_directory, static_dir, gamedat_dir, savegame_dir,
		default_dir, system_path_tag(to_uppercase(gametitle)),
		config_path("config/disk/game/" + gametitle + "/path");

	config->value(config_path.c_str(), data_directory, ".");
	if (data_directory == ".")
		config->set(config_path.c_str(), data_directory, true);
#if 0
	cout << "setting " << gametitle
		<< " game directories to: " << data_directory << endl;
#endif

	config_path = "config/disk/game/" + gametitle + "/static_path";
	default_dir = data_directory + "/static";
	config->value(config_path.c_str(), static_dir, default_dir.c_str());
	add_system_path("<" + system_path_tag + "_STATIC>", static_dir);
#if 0
	cout << "setting " << gametitle
		<< " static directory to: " << static_dir << endl;
#endif

	const char *home = 0;		// Will get $HOME.
	string home_game("");		// Gets $HOME/.exult/gametitle.
	config_path = "config/disk/game/" + gametitle + "/gamedat_path";
	default_dir = data_directory + "/gamedat";
	config->value(config_path.c_str(), gamedat_dir, "");
#if (!defined(WIN32) && !defined(MACOS))
	if (gamedat_dir == "" &&	// Not set?
					// And default doesn't exist?
	    !U7exists(default_dir.c_str()) && (home = getenv("HOME")) != 0)
		{
		home_game = home;
		home_game += "/.exult";
					// Create $HOME/.exult/gametitle.
		U7mkdir(home_game.c_str(), 0755);
		home_game = home_game + '/' + gametitle;
		U7mkdir(home_game.c_str(), 0755);
					// Successfully created dir?
		if (U7exists(home_game.c_str()))
			{		// Use $HOME/.exult/gametitle/gamedat.
			gamedat_dir = home_game + "/gamedat";
			config->set(config_path.c_str(), gamedat_dir.c_str(),
								true);
			}
		else
			home_game = "";	// Failed.
		}
#endif
	if (gamedat_dir == "")		// Didn't create it in $HOME/.exult?
		gamedat_dir = default_dir;
	add_system_path("<" + system_path_tag + "_GAMEDAT>", gamedat_dir);
#if 0
	cout << "setting " << gametitle
		<< " gamedat directory to: " << gamedat_dir << endl;
#endif

	config_path = "config/disk/game/" + gametitle + "/savegame_path";
	if (home_game == "")
		config->value(config_path.c_str(), savegame_dir, 
						data_directory.c_str());
	else
		{			// Store saves under $HOME/....
		config->value(config_path.c_str(), savegame_dir,
						home_game.c_str());
		config->set(config_path.c_str(), savegame_dir.c_str(), true);
		}
	add_system_path("<" + system_path_tag + "_SAVEGAME>", savegame_dir);
#if 0
	cout << "setting " << gametitle
		<< " savegame directory to: " << savegame_dir << endl;
#endif

	// A patch directory is optional.
	config_path = "config/disk/game/" + gametitle + "/patch";
	string patch_directory;
	config->value(config_path.c_str(), patch_directory, "");
	if (patch_directory != "")
		add_system_path("<" + system_path_tag + "_PATCH>", patch_directory.c_str());
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
					// KBD repeat should be nice.
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, 
						SDL_DEFAULT_REPEAT_INTERVAL);
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
	sclr = Image_window::get_scaler_for_name(scaler);
	if (sclr == Image_window::NoScaler) config->set("config/video/scale_method","2xSaI",true);

	Image_window8::set_gamma(atof(gr.c_str()), atof(gg.c_str()), atof(gb.c_str()));	
	gwin = new Game_window(sw, sh, scaleval, sclr);
	current_res = find_resolution(sw, sh, scaleval);
	Audio::Init();

	bool disable_fades;
	config->value("config/video/disable_fades", disable_fades, false);
	gwin->set_fades_enabled(!disable_fades);


	if (arg_buildmap >= 0)
		BuildGameMap();

	SDL_SetEventFilter(0);
	// Show the banner
	Exult_Game mygame;
	game = 0;

	// Figure out all the games' paths, before we store them.  That
	// way, we don't have to recalculate them if we come back to the
	// main menu and make another selection.
	if (arg_gamename == "default") {
		// We're going to the menu, so just load the two default
		// games.
		get_game_paths("blackgate");
		get_game_paths("serpentisle");
	} else
		// The user gave us a game title, so load that one's
		// paths.
		get_game_paths(arg_gamename);

	store_system_paths();

	do {
		reset_system_paths();
		fontManager.reset();
		U7FileManager::get_ptr()->reset();
		const char *title = 0;

		if(game)
			delete game;
		
		if (run_bg) {
			mygame = BLACK_GATE;
			run_bg = false;
		} else if (run_si) {
			mygame = SERPENT_ISLE;
			run_si = false;
		} else if (arg_buildmap < 0 && arg_gamename != "default") {
			mygame = EXULT_DEVEL_GAME;
			title = arg_gamename.c_str();
		} else {
			ExultMenu exult_menu(gwin);
			mygame = exult_menu.run();
		}
		Game::create_game(mygame, title);
		
					// Skip splash screen?
		bool skip_splash;
		config->value("config/gameplay/skip_splash", skip_splash);
		if(!skip_splash) 
			game->play_intro();
	} while(!game->show_menu());
	gwin->init_files();
	gwin->read_gwin();
	gwin->setup_game();		// This will start the scene.
					// Get scale factor for mouse.
#ifdef USE_EXULTSTUDIO
#ifndef WIN32
        SDL_GetWMInfo(&info);
        xfd = ConnectionNumber(info.info.x11.display);
	Server_init();			// Initialize server (for map-editor).
	xdnd = new Xdnd(info.info.x11.display, info.info.x11.wmwindow,
		info.info.x11.window, Move_dragged_shape, Move_dragged_combo,
				Drop_dragged_shape, Drop_dragged_chunk, 
							Drop_dragged_combo);
#else
	SDL_GetWMInfo(&info);
	Server_init();			// Initialize server (for map-editor).
	hgwin = info.window;
        OleInitialize(NULL);
	windnd = new Windnd(hgwin, Move_dragged_shape, Move_dragged_combo,
				Drop_dragged_shape, Drop_dragged_chunk,
							Drop_dragged_combo);
	if (FAILED(RegisterDragDrop(hgwin, windnd))) {
	     cout << "Something's wrong with OLE2 ..." << endl;
	};
#endif
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
			/////gwin->setup_game();
			//// setup_game is already being called from inside
			//// of gwin->read(), so no need to call it here, I hope...
		}
	}
	while (quitting_time == QUIT_TIME_RESTART);
	delete gwin;
	delete Mouse::mouse;

	Audio::Destroy();	// Deinit the sound system.

	delete config;
	return (0);
}

#ifdef USE_EXULTSTUDIO			// Shift-click means 'paint'.
/*
 *	Add a shape while map-editing.
 */

static void Drop_in_map_editor
	(
	SDL_Event& event,
	bool dragging			// Painting terrain.
	)
	{
	static int lasttx = -1, lastty = -1;
	int scale = gwin->get_win()->get_scale();
	int x = event.button.x/scale, y = event.button.y/scale;
	int tx = (gwin->get_scrolltx() + x/c_tilesize);
	int ty = (gwin->get_scrollty() + y/c_tilesize);
	if (dragging)			// See if moving to a new tile.
		{
		if (tx == lasttx && ty == lastty)
			return;
		}
	lasttx = tx; lastty = ty;
	int shnum = cheat.get_edit_shape();
	int frnum;
	SDLMod mod = SDL_GetModState();
	if (mod & KMOD_ALT)		// ALT?  Pick random frame.
		{
		ShapeID id(shnum, 0);
		frnum = std::rand()%id.get_num_frames();
		}
	else if (mod & KMOD_CTRL)	// Cycle through frames.
		{
		frnum = cheat.get_edit_frame();
		ShapeID id(shnum, 0);
		int nextframe = (frnum + 1)%id.get_num_frames();
		cheat.set_edit_shape(shnum, nextframe);
		}
	else
		frnum = cheat.get_edit_frame();
	Drop_dragged_shape(shnum, frnum, event.button.x, event.button.y, 0);
	}
#endif

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
		int scale = gwin->get_fastmouse() ? 1 : 
						gwin->get_win()->get_scale();

		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;

					// Get current time.
		uint32 ticks = SDL_GetTicks();
		Game::set_ticks(ticks);

		SDL_Event event;
		while (!quitting_time && SDL_PollEvent(&event))
			Handle_event(event);

					// Animate unless dormant.
		if (gwin->have_focus() && !dragging)
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
		if (ticks > last_repaint + 50 || gwin->was_painted())
					// This avoids jumpy walking:
			{
			gwin->paint_dirty();
			while (ticks > last_repaint+50)last_repaint += 50;

			int x, y;// Check for 'stuck' Avatar.
			if (!gwin->is_moving() &&
			    !((gwin->get_walk_after_teleport() && GAME_SI) ? false : gwin->was_teleported()))
				{
				int ms = SDL_GetMouseState(&x, &y);
				if (SDL_BUTTON(3) & ms)
					gwin->start_actor(x/scale, y/scale, 
						Mouse::mouse->avatar_speed);
				}
			}

		if (show_mouse)
			Mouse::mouse->show();	// Re-display mouse.

					// Rotate less often if scaling and 
					//   not paletized.
		int rot_speed = 100 << (gwin->get_win()->is_palettized() ||
								scale==1?0:1);
		if (ticks > last_rotate + rot_speed)
			{		// (Blits in simulated 8-bit mode.)
			gwin->get_win()->rotate_colors(0xf8, 4, 0);
			gwin->get_win()->rotate_colors(0xf4, 4, 0);
			gwin->get_win()->rotate_colors(0xf0, 4, 0);
			gwin->get_win()->rotate_colors(0xe8, 8, 0);
			gwin->get_win()->rotate_colors(0xe0, 8, 1);
			while (ticks > last_rotate + rot_speed) 
				last_rotate += rot_speed;
					// Non palettized needs explicit blit.
			if (!gwin->get_win()->is_palettized())
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
	int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
	bool dont_move_mode = gwin->main_actor_dont_move();

	// We want this
	Gump_manager *gump_man = gwin->get_gump_man();
	static bool right_on_gump = false;
	Gump *gump = 0;

					// For detecting double-clicks.
	static uint32 last_b1_click = 0, last_b3_click = 0;
	//cout << "Event " << (int) event.type << " received"<<endl;
	switch (event.type)
		{
	case SDL_MOUSEBUTTONDOWN:
		{
        	if (dont_move_mode)
        		break;
		int x = event.button.x/scale, y = event.button.y/scale;
		if (event.button.button == 1)
			{
#ifdef USE_EXULTSTUDIO
			if (cheat.in_map_editor())
				{	// Paint if shift-click.
				if (cheat.get_edit_shape() >= 0 &&
					// But always if painting.
			    	    (cheat.get_edit_mode() == Cheat::paint ||
					(SDL_GetModState() & KMOD_SHIFT)))
					{
					Drop_in_map_editor(event, false);
					break;
					}
					// Don't drag if not in 'move' mode.
				else if (cheat.get_edit_mode() != Cheat::move)
					break;
				}
#endif
			dragging = gwin->start_dragging(x, y);
			//Mouse::mouse->set_shape(Mouse::hand);
			dragged = false;
			}
					// Move sprite toward mouse
					//  when right button pressed.
		if (gwin->get_mouse3rd())
			if (event.button.button == 2)
				{
					ActionTarget(0);
				}
		if (event.button.button == 3) {
			
			// Try removing old queue entry.
			gwin->get_tqueue()->remove(gwin->get_main_actor());

			if (!dragging &&	// Causes crash if dragging.
				gump_man->can_right_click_close() &&
					gump_man->gump_mode() && 
					gump_man->find_gump(x, y, false)) {
				gump = 0;
				right_on_gump = true;
			}
			else 
				gwin->start_actor(x, y, 
						Mouse::mouse->avatar_speed);

		}
		if (event.button.button == 4 || event.button.button == 5) 
			{
			if (!cheat()) break;
			SDLMod mod = SDL_GetModState();
			if (event.button.button == 4)
				if (mod & KMOD_ALT)
					ActionScrollLeft(0);
				else
					ActionScrollUp(0);
			else
				if (mod & KMOD_ALT)
					ActionScrollRight(0);
				else
					ActionScrollDown(0);
			}
		break;
		}
	case SDL_MOUSEBUTTONUP:
		{
	        if (dont_move_mode)
        	    break;
		int x = event.button.x/scale, y = event.button.y/scale;
		if (event.button.button == 3)
			{
			uint32 curtime = SDL_GetTicks();
					// Last click within .5 secs?
			if (curtime - last_b3_click < 500)
				gwin->start_actor_along_path(x, y,
						Mouse::mouse->avatar_speed);
			else if (right_on_gump && 
				(gump = gump_man->find_gump(x, y, false))) {
				Rectangle dirty = gump->get_dirty();
				gwin->add_dirty(dirty);
				gump_man->close_gump(gump);
				gump = 0;
				right_on_gump = false;
			}
			else
				gwin->stop_actor();
			last_b3_click = curtime;
			}
		else if (event.button.button == 1)
			{
			uint32 curtime = SDL_GetTicks();
			bool click_handled = false;
			if (dragging) {
				click_handled = gwin->drop_dragged(x, y,
								dragged);
			}
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500)
				{
				dragging = false;
				gwin->double_clicked(x, y);
				Mouse::mouse->set_speed_cursor();
				break;
				}
			last_b1_click = curtime;
			if (!click_handled)
					// Identify item(s) clicked on.
				gwin->show_items(x, y, 
					(SDL_GetModState() & KMOD_CTRL) != 0);
			dragging = false;
			}
		break;
		}
	case SDL_MOUSEMOTION:
		{
		Mouse::mouse->move(event.motion.x / scale, 
						event.motion.y / scale);
		Mouse::mouse->set_speed_cursor();
		Mouse::mouse_update = true;	// Need to blit mouse.
		right_on_gump = false;

					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			{
#ifdef USE_EXULTSTUDIO			// Painting?
			if (cheat.in_map_editor() && 
			    cheat.get_edit_shape() >= 0 &&
			    (cheat.get_edit_mode() == Cheat::paint ||
					(SDL_GetModState() & KMOD_SHIFT)))
				{
				Drop_in_map_editor(event, true);
				break;
				}
#endif
			dragged = gwin->drag(event.motion.x / scale, 
						event.motion.y / scale);
			}
					// Dragging with right?
		else if ((event.motion.state & SDL_BUTTON(3)) &&
					// But not right after teleport (if disabled).
		    !((gwin->get_walk_after_teleport() && GAME_SI) ? false : gwin->was_teleported()))
			gwin->start_actor(event.motion.x / scale, 
			event.motion.y / scale, Mouse::mouse->avatar_speed);
#ifdef USE_EXULTSTUDIO			// Painting?
		else if (cheat.in_map_editor() && 
			 cheat.get_edit_shape() >= 0 &&
			 (cheat.get_edit_mode() == Cheat::paint ||
					(SDL_GetModState() & KMOD_SHIFT)))
			{
			static int prevx = -1, prevy = -1;
			Move_dragged_shape(cheat.get_edit_shape(),
				cheat.get_edit_frame(), 
				event.motion.x, event.motion.y, 
						prevx, prevy, false);
			prevx = event.motion.x; prevy = event.motion.y;
			}
#endif
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
		if (!dragging)		// ESC while dragging causes crashes.
			keybinder->HandleEvent(event);
		break;
#ifdef USE_EXULTSTUDIO
#ifndef WIN32
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
	char *chr,			// Char. returned if not null.
	bool drag_ok			// Okay to drag/close while here.
	)
	{
	dragging = false;		// Init.
	while (1)
		{
		SDL_Event event;
		Delay();		// Wait a fraction of a second.

		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;

		// Mouse scale factor
		int scale = gwin->get_fastmouse() ? 1 
				: gwin->get_win()->get_scale();

		static bool rightclick;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == 3)
					rightclick = true;
				else if (drag_ok && event.button.button == 1)
					{
					x = event.button.x / scale;
					y = event.button.y / scale;
					dragging = gwin->start_dragging(x, y);
					dragged = false;
					}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == 1)
					{
					x = event.button.x / scale;
					y = event.button.y / scale;
					bool drg = dragging;
					dragging = false;
					if (!drg ||
					    !gwin->drop_dragged(x, y, dragged))
						{
						if (chr) *chr = 0;
						return (1);
						}
					}
				else if (event.button.button == 3) {
					// Just stop.  Don't get followers!
					gwin->get_main_actor()->stop();
					if (gwin->get_mouse3rd() && rightclick)						{
						rightclick = false;
						return 0;
					}
				}
				break;
			case SDL_MOUSEMOTION:
				{
				int mx = event.motion.x / scale,
				    my = event.motion.y / scale;
				Mouse::mouse->move(mx, my);
				Mouse::mouse_update = true;
				if (drag_ok &&
				    (event.motion.state & SDL_BUTTON(1)))
					dragged = gwin->drag(mx, my);
				break;
				}
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
	char *chr,			// Char. returned if not null.
	bool drag_ok			// Okay to drag/close while here.
	)
	{
	if (chr)
		*chr = 0;		// Init.
	Mouse::Mouse_shapes saveshape = Mouse::mouse->get_shape();
	if (shape != Mouse::dontchange)
		Mouse::mouse->set_shape(shape);
	Mouse::mouse->show();
	gwin->show(1);			// Want to see new mouse.
	int ret = Get_click(x, y, chr, drag_ok);
	Mouse::mouse->set_shape(saveshape);
	return (ret);
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
	int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();

	unsigned char os = Mouse::mouse->is_onscreen();
	uint32 last_repaint = 0;	// For insuring animation repaints.
	Actor_action *orig_action = actor->get_action();
	uint32 stop_time = SDL_GetTicks() + maxticks;
	bool timeout = false;
	while (actor->is_moving() && actor->get_action() == orig_action &&
	       actor->get_tile() != dest && !timeout)
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
		Game::set_ticks(ticks);
		if (maxticks && ticks > stop_time)
			timeout = true;
		if (gwin->have_focus())
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
		if (ticks > last_repaint + 50 || gwin->was_painted())
			{
			gwin->paint_dirty();
			while (ticks > last_repaint+50)last_repaint += 50;
			}

		Mouse::mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)	// If not, did mouse change?
			Mouse::mouse->blit_dirty();
		}

	if (!os)
		Mouse::mouse->hide();

	}

/*
 *	Shift 'wizard's view' according to mouse position.
 */

static void Shift_wizards_eye
	(
	int mx, int my
	)
	{
					// Figure dir. from center.
	int cx = gwin->get_width()/2, cy = gwin->get_height()/2;
        int dy = cy - my, dx = mx - cx;
        Direction dir = Get_direction(dy, dx);
	static int deltas[16] = {0,-1, 1,-1, 1,0, 1,1, 0,1, 
						-1,1, -1,0, -1,-1};
	int dirx = deltas[2*dir], diry = deltas[2*dir + 1];
	if (dirx == 1)
		gwin->view_right();
	else if (dirx == -1)
		gwin->view_left();
	if (diry == 1)
		gwin->view_down();
	else if (diry == -1)
		gwin->view_up();
	}

/*
 *	Do the 'wizard's eye' spell by letting the user browse around.
 */

void Wizard_eye
	(
	long msecs			// Length of time in milliseconds.
	)
	{
	// Mouse scale factor
	int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale();
					// Center of screen.
	int cx = gwin->get_width()/2, cy = gwin->get_height()/2;

	unsigned char os = Mouse::mouse->is_onscreen();
	uint32 last_repaint = 0;	// For insuring animation repaints.
	uint32 stop_time = SDL_GetTicks() + msecs;
	bool timeout = false;
	while (!timeout)
		{
		Delay();		// Wait a fraction of a second.

		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEMOTION:
				{
				int mx = event.motion.x/scale,
				    my = event.motion.y/scale;
				Mouse::mouse->move(mx, my);
				Mouse::mouse->set_shape(
					Mouse::mouse->get_short_arrow(
					Get_direction(cy - my, mx - cx)));
				Mouse::mouse_update = true;
				break;
				}
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					timeout = true;
				}
					// Get current time, & animate.
		uint32 ticks = SDL_GetTicks();
		Game::set_ticks(ticks);
		if (ticks > stop_time)
			timeout = true;
		if (gwin->have_focus())
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
		if (ticks > last_repaint + 50 || gwin->was_painted())
			{		// Right mouse button down?
			int x, y;
			int ms = SDL_GetMouseState(&x, &y);
			if (SDL_BUTTON(3) & ms)
				Shift_wizards_eye(x/scale, y/scale);
			gwin->set_all_dirty();
			gwin->paint_dirty();
					// Paint sprite over view.
			ShapeID eye(10, 0, SF_SPRITES_VGA);
			Shape_frame *spr = eye.get_shape();
					// Center it.
			int w = gwin->get_width(), h = gwin->get_height();
			int sw = spr->get_width(), sh = spr->get_height();
			int topx = (w - sw)/2,
			    topy = (h - sh)/2;
			gwin->paint_shape(topx + spr->get_xleft(),
					topy + spr->get_yabove(), eye);
			if (topy > 0)	// Black-fill area around sprite.
				{
				gwin->get_win()->fill8(0, w, topy, 0, 0);
				gwin->get_win()->fill8(0, w, h - topy - sh,
								0, topy + sh);
				}
			if (topx > 0)
				{
				gwin->get_win()->fill8(0, topx, sh, 0, topy);
				gwin->get_win()->fill8(0, w - topx - sw, sh,
							topx + sw, topy);
				}
			while (ticks > last_repaint+50)last_repaint += 50;
			}

		Mouse::mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)	// If not, did mouse change?
			Mouse::mouse->blit_dirty();
		}

	if (!os)
		Mouse::mouse->hide();
	gwin->center_view(gwin->get_main_actor()->get_tile());
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
			if (scaler > Image_window::NoScaler && scaler < Image_window::NumScalers)
				config->set("config/video/scale_method",Image_window::get_name_for_scaler(scaler),true);
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
#ifdef HAVE_SNPRINTF
	Image_window8::get_gamma(r, g, b);	
	snprintf (text, 256, "Gamma Set to R: %01.2f G: %01.2f B: %01.2f", r, g, b);
#else
	strncpy (text, "Gamma Changed", 256);
#endif
	gwin->center_text(text);	

	int igam = (int) ((r*10000)+0.5);
	snprintf (text, 256, "%d.%04d", igam/10000, igam%10000);
	config->set("config/video/gamma/red", text, true);

	igam = (int) ((b*10000)+0.5);
	snprintf (text, 256, "%d.%04d", igam/10000, igam%10000);
	config->set("config/video/gamma/green", text, true);

	igam = (int) ((g*10000)+0.5);
	snprintf (text, 256, "%d.%04d", igam/10000, igam%10000);
	config->set("config/video/gamma/blue", text, true);
}

void BuildGameMap()
{
	int w, h, sc, sclr;

	// create 2048x2048 screenshots of the full Ultima 7 map.
	// WARNING!! Takes up lots of memory and diskspace!
	if (arg_buildmap >= 0) {
		int maplift = 16;
		Exult_Game gametype;
		switch(arg_buildmap) {
			case 0: maplift = 16; break;
			case 1: maplift = 10; break;
			case 2: maplift = 5; break;
		}
		if (run_bg)
			gametype = BLACK_GATE;
		else if (run_si)
			gametype = SERPENT_ISLE;
		else {
			cerr << "You have to specify --bg or --si when using -buildmap" << endl;
			exit(1);
		}

		h = w = c_tilesize * c_tiles_per_schunk; sc = 1, sclr = Image_window::point;
		Image_window8::set_gamma(1, 1, 1);
		gwin = new Game_window(w, h, sc, sclr);
		current_res = find_resolution(w, h, sc);
		Game::create_game(gametype);
		gwin->init_files(false); //init, but don't show plasma	
		gwin->get_map()->init();// +++++Got to clean this up.
		gwin->set_palette(0);
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
 *	Show a grid being dragged.
 */

static void Move_grid
	(
	int x, int y,			// Mouse coords. within window.
	int prevx, int prevy,		// Prev. coords, or -1.
	bool ireg,			// A single IREG object?
	int xtiles, int ytiles,		// Dimension of grid to show.
	int tiles_right, int tiles_below// # tiles to show to right of and
					//   below (x, y).
	)
	{
	int scale = gwin->get_win()->get_scale();
	x /= scale;			// Watch for scaled window.
	y /= scale;
	int lift = cheat.get_edit_lift();
	x += lift*4 - 1;		// Take lift into account, round.
	y += lift*4 - 1;
	int tx = x/c_tilesize;		// Figure tile on ground.
	int ty = y/c_tilesize;
	tx += tiles_right;
	ty += tiles_below;
	if (prevx != -1)		// See if moved to a new tile.
		{
		prevx /= scale;
		prevy /= scale;
		prevx += lift*4 - 1;	// Take lift into account, round.
		prevy += lift*4 - 1;
		int ptx = prevx/c_tilesize, pty = prevy/c_tilesize;
		if (tx == ptx && ty == pty)
			return;		// Will be in same tile.
					// Repaint over old area.
		const int pad = 8;
		Rectangle r((ptx - xtiles + 1)*c_tilesize - pad,
			    (pty - ytiles + 1)*c_tilesize - pad,
			    xtiles*c_tilesize + 2*pad,
			    ytiles*c_tilesize + 2*pad);
		r = gwin->clip_to_win(r);
		gwin->add_dirty(r);
		gwin->paint_dirty();
		}
					// First see if it's a gump.
	if (ireg && gwin->get_gump_man()->find_gump(x, y))
		return;			// Skip if so.
	tx -= xtiles - 1;		// Get top-left of footprint.
	ty -= ytiles - 1;
					// Let's try a green outline.
	int pix = gwin->get_poison_pixel();
	Image_window8 *win = gwin->get_win();
	win->set_clip(0, 0, win->get_width(), win->get_height());
	for (int Y = 0; Y <= ytiles; Y++)
		win->fill8(pix, xtiles*c_tilesize, 1, 
					tx*c_tilesize, (ty + Y)*c_tilesize);
	for (int X = 0; X <= xtiles; X++)
		win->fill8(pix, 1, ytiles*c_tilesize,
				(tx + X)*c_tilesize, ty*c_tilesize);
	win->clear_clip();
	gwin->set_painted();
	}

/*
 *	Show where a shape dragged from a shape-chooser will go.
 *	ALSO, this is called with shape==-1 to just force a repaint.
 */

static void Move_dragged_shape
	(
	int shape, int frame,		// What to create, OR -1 to just
					//   repaint window.
	int x, int y,			// Mouse coords. within window.
	int prevx, int prevy,		// Prev. coords, or -1.
	bool show			// Blit window.
	)
	{
	if (shape == -1)
		{
		gwin->set_all_dirty();
		return;
		}
	Shape_info& info = gwin->get_info(shape);
					// Get footprint in tiles.
	int xtiles = info.get_3d_xtiles(frame),
	    ytiles = info.get_3d_ytiles(frame);
	int sclass = info.get_shape_class();
					// Is it an ireg (changeable) obj?
	bool ireg = (sclass != Shape_info::unusable &&
		     sclass != Shape_info::building);
	Move_grid(x, y, prevx, prevy, ireg, xtiles, ytiles, 0, 0);
	if (show)
		gwin->show();
	}

/*
 *	Show where a shape dragged from a shape-chooser will go.
 */

static void Move_dragged_combo
	(
	int xtiles, int ytiles,		// Dimensions in tiles.
	int tiles_right,		// Tiles right of & below hot-spot.
	int tiles_below,
	int x, int y,			// Mouse coords. within window.
	int prevx, int prevy,		// Prev. coords, or -1.
	bool show			// Blit window.
	)
	{
	Move_grid(x, y, prevx, prevy, false, xtiles, ytiles, tiles_right,
							tiles_below);
	if (show)
		gwin->show();
	}

/*
 *	Create an object as moveable (IREG) or fixed.
 */

static Game_object *Create_object
	(
	int shape, int frame,		// What to create.
	bool& ireg			// Rets. TRUE if ireg (moveable).
	)
	{
	Shape_info& info = gwin->get_info(shape);
	int sclass = info.get_shape_class();
					// Is it an ireg (changeable) obj?
	ireg = (sclass != Shape_info::unusable &&
		sclass != Shape_info::building);
	Game_object *newobj;
	if (ireg)
		newobj = gwin->get_map()->create_ireg_object(
						info, shape, frame, 0, 0, 0);
	else
		newobj = new Ifix_game_object(shape, frame, 0, 0, 0);
	return newobj;
	}

/*
 *	Drop a shape dragged from a shape-chooser via drag-and-drop.  Dnd is
 *	only supported under X for now.
 */

static void Drop_dragged_shape
	(
	int shape, int frame,		// What to create.
	int x, int y,			// Mouse coords. within window.
	void *data			// Passed data, unused by exult
	)
	{
	int scale = gwin->get_win()->get_scale();
	if (!cheat.in_map_editor())	// Get into editing mode.
		cheat.toggle_map_editor();
	cheat.clear_selected();		// Remove old selected.
	gwin->get_map()->set_map_modified();
	x /= scale;			// Watch for scaled window.
	y /= scale;
	ShapeID sid(shape, frame);
	if (gwin->skip_lift == 0)	// Editing terrain?
		{
		int tx = (gwin->get_scrolltx() + x/c_tilesize)%c_num_tiles;
		int ty = (gwin->get_scrollty() + y/c_tilesize)%c_num_tiles;
		int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
		Map_chunk *chunk = gwin->get_chunk(cx, cy);
		Chunk_terrain *ter = chunk->get_terrain();
		tx %= c_tiles_per_chunk; ty %= c_tiles_per_chunk;
		ShapeID curid = ter->get_flat(tx, ty);
		if (sid.get_shapenum() != curid.get_shapenum() ||
		    sid.get_framenum() != curid.get_framenum())
			{
			ter->set_flat(tx, ty, sid);
			gwin->set_all_dirty();	// ++++++++For now.++++++++++
			}
		return;
		}
	Shape_frame *sh = sid.get_shape();
	if (!sh || !sh->is_rle())	// Flats are only for terrain.
		return;			// Shouldn't happen.
	cout << "Last drag pos: (" << x << ", " << y << ')' << endl;
	cout << "Create shape (" << shape << '/' << frame << ')' <<
								endl;
	bool ireg;			// Create object.
	Game_object *newobj = Create_object(shape, frame, ireg);
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
				cheat.append_selected(newobj);
				gwin->set_all_dirty();	// For now, until we
					//    clear out grid we're painting.
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
	int x, int y,			// Mouse coords. within window.
	void *data			// Passed data, unused by exult
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
	int tx = (gwin->get_scrolltx() + x/c_tilesize)%c_num_tiles,
	    ty = (gwin->get_scrollty() + y/c_tilesize)%c_num_tiles;
	int cx = tx/c_tiles_per_chunk, cy = ty/c_tiles_per_chunk;
	gwin->get_map()->set_chunk_terrain(cx, cy, chunknum);
	gwin->paint();
	}

/*
 *	Drop a combination object dragged from ExultStudio.
 */

void Drop_dragged_combo
	(
	int cnt,			// # shapes.
	U7_combo_data *combo,		// The shapes.
	int x, int y,			// Mouse coords. within window.
	void *data			// Passed data, unused by exult
	)
	{
	int scale = gwin->get_win()->get_scale();
	if (!cheat.in_map_editor())	// Get into editing mode.
		cheat.toggle_map_editor();
	cheat.clear_selected();		// Remove old selected.
	x /= scale;			// Watch for scaled window.
	y /= scale;
	int at_lift = cheat.get_edit_lift();
	x += at_lift*4 - 1;		// Take lift into account, round.
	y += at_lift*4 - 1;
					// Figure tile at mouse pos.
	int tx = (gwin->get_scrolltx() + x/c_tilesize)%c_num_tiles,
	    ty = (gwin->get_scrollty() + y/c_tilesize)%c_num_tiles;
	for (int i = 0; i < cnt; i++)
		{			// Drop each shape.
		U7_combo_data& elem = combo[i];
					// Figure new tile coord.
		int ntx = (tx + elem.tx)%c_num_tiles, 
		    nty = (ty + elem.ty)%c_num_tiles, 
		    ntz = at_lift + elem.tz;
		if (ntz < 0)
			ntz = 0;
		ShapeID sid(elem.shape, elem.frame);
		if (gwin->skip_lift == 0)// Editing terrain?
			{
			int cx = ntx/c_tiles_per_chunk, 
			    cy = nty/c_tiles_per_chunk;
			Map_chunk *chunk = gwin->get_chunk(cx, cy);
			Chunk_terrain *ter = chunk->get_terrain();
			ntx %= c_tiles_per_chunk; nty %= c_tiles_per_chunk;
			ter->set_flat(ntx, nty, sid);
			continue;
			}
		bool ireg;		// Create object.
		Game_object *newobj = Create_object(elem.shape, 
							elem.frame, ireg);
		newobj->set_invalid();	// Not in world.
		newobj->move(ntx, nty, ntz);
					// Add to selection.
		cheat.append_selected(newobj);
		}
	gwin->set_all_dirty();		// For now, until we clear out grid.
	}

#endif
