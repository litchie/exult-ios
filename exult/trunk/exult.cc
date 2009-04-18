/**
 **	Exult.cc - Multiplatform Ultima 7 game engine
 **
 **	Written: 7/22/98 - JSF
 **/
/*
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2008  The Exult Team
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
#include "servemsg.h"
#include "objserial.h"
#include "server.h"
#include "chunks.h"
#include "chunkter.h"
#endif

#if (defined(USECODE_DEBUGGER) && defined(XWIN))
#include <csignal>
#endif

#ifdef UNDER_CE
#include <gx.h>
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
#include "drag.h"
#include "palette.h"
#include "glshape.h"
#include "combat_opts.h"
#include "U7file.h"
#include "U7fileman.h"

#include "exult_flx.h"
#include "exult_bg_flx.h"
#include "exult_si_flx.h"
#include "crc.h"
#include "items.h"
#include "gamemgr/modmgr.h"

#ifdef UNDER_CE
  #include "Keyboard_gump.h"
  #include "touchscreen.h"
#endif

#ifndef UNDER_EMBEDDED_CE
using std::atof;
using std::cerr;
using std::cout;
using std::endl;
using std::atexit;
using std::exit;
using std::toupper;
using std::string;
using std::vector;
#endif

Configuration *config = 0;
KeyBinder *keybinder = 0;
GameManager *gamemanager = 0;

/*
 *	Globals:
 */
Game_window *gwin = 0;
quitting_time_enum quitting_time = QUIT_TIME_NO;

bool intrinsic_trace = false;		// Do we trace Usecode-intrinsics?
int usecode_trace = 0;		// Do we trace Usecode-instructions?
							// 0 = no, 1 = short, 2 = long
bool combat_trace = false; // show combat messages?

// Save game compression level
int save_compression = 1;
bool ignore_crc = false;

const std::string c_empty_string;

#ifdef UNDER_CE
string WINCE_exepath;
bool minimized;
Keyboard_gump *gkeyboard;
clsTouchscreen *Touchscreen;
#endif

#if 0 && USECODE_DEBUGGER
bool	usecode_debugging=false;	// Do we enable the usecode debugger?
extern void initialise_usecode_debugger(void);
#endif

struct resolution {
	int x;
	int y;
	int scale;
} res_list[] = { 
#ifdef UNDER_CE
	{ 240, 160, 1 },
	{ 160, 240, 1 },
	{ 240, 320, 1 },
	{ 240, 160, 2 },
	{ 160, 240, 2 },
	{ 240, 320, 2 },
#endif
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
static int Get_click(int& x, int& y, char *chr, bool drag_ok, Paintable *p, bool rotate_colors = false);
static int find_resolution(int w, int h, int s);
static void set_resolution (int new_res, bool save);
#ifdef USE_EXULTSTUDIO
static void Move_dragged_shape(int shape, int frame, int x, int y,
				int prevx, int prevy, bool show);
static void Move_dragged_combo(int xtiles, int ytiles, int tiles_right,
	int tiles_below, int x, int y, int prevx, int prevy, bool show);
static void Drop_dragged_shape(int shape, int frame, int x, int y, void *d);
static void Drop_dragged_chunk(int chunknum, int x, int y, void *d);
static void Drop_dragged_npc(int npcnum, int x, int y, void *d);
static void Drop_dragged_combo(int cnt, U7_combo_data *combo, 
							int x, int y, void *d);
#endif
static void BuildGameMap(BaseGameInfo *game);
static void Handle_events();
static void Handle_event(SDL_Event& event);



/*
 *	Statics:
 */
static bool run_bg = false;		// skip menu and run bg
static bool run_si = false;		// skip menu and run si
static bool run_fov = false;		// skip menu and run fov
static bool run_ss = false;		// skip menu and run ss
static string arg_gamename = "default";	// cmdline arguments
static string arg_modname = "default";	// cmdline arguments
static string arg_configfile = "";
static int arg_buildmap = -1;
static bool arg_nomenu = false;
static bool arg_edit_mode = false;	// Start up ExultStudio.
static bool arg_write_xml = false;	// Write out game's config. as XML.

static bool dragging = false;		// Object or gump being moved.
static bool dragged = false;		// Flag for when obj. moved.
static bool right_on_gump = false;	// Right clicked on gump?
static int show_items_x = 0, show_items_y = 0;
static int show_items_time = 0;
static bool show_items_clicked = false;
static int left_down_x = 0, left_down_y = 0;

/*
 *	A handy breakpoint.
 */

static void Breakpoint
	(
	)
	{
	return;
	}

#if (defined(XWIN) && HAVE_SIGNAL_H && HAVE_SYS_WAIT_H)

// a SIGCHLD handler to properly clean up forked playmidi processes (if any)

#include <sys/wait.h>
#include <signal.h>
void sigchld_handler(int sig)
{
	waitpid(-1, 0, WNOHANG);
}

#endif


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

#if (defined(XWIN) && HAVE_SIGNAL_H && HAVE_SYS_WAIT_H)
	signal(SIGCHLD, sigchld_handler);
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
	parameters.declare("--fov",&run_fov,true);
	parameters.declare("--ss",&run_ss,true);
	parameters.declare("--nomenu", &arg_nomenu, true);
	parameters.declare("-v",&showversion,true);
	parameters.declare("--version",&showversion,true);
	parameters.declare("--game",&arg_gamename,"default");
	parameters.declare("--mod",&arg_modname,"default");
	parameters.declare("--buildmap",&arg_buildmap,-1);
	parameters.declare("--nocrc",&ignore_crc,true);
	parameters.declare("-c",&arg_configfile,"");
	parameters.declare("--edit",&arg_edit_mode,true);
	parameters.declare("--write-xml",&arg_write_xml,true);

	// Process the args
	parameters.process(argc,argv);

	if(needhelp)
	{
		cerr << "Usage: exult [--help|-h] [-v|--version] [-c configfile]"<<endl
			 << "             [--bg|--si] [--buildmap 0|1|2] [--nocrc]" << endl
			 << "--help\t\tShow this information" << endl
			 << "--version\tShow version info" << endl
			 << " -c configfile\tSpecify alternate config file" << endl
			 << "--bg\t\tSkip menu and run Black Gate (prefers original game)" << endl
			 << "--fov\t\tSkip menu and run Black Gate with Forge of Virtue expansion" << endl
			 << "--si\t\tSkip menu and run Serpent Isle (prefers original game)" << endl
			 << "--ss\t\tSkip menu and run Serpent Isle with Silver Seed expansion" << endl
			 << "--nomenu\tSkip BG/SI game menu" << endl
			 << "--game <game>\tRun original game" << endl
			 << "--mod <mod>\tMust be used together with '--bg', '--si' or '--game <game>'; runs the specified game using the mod with title equal to '<mod>'" << endl
			 << "--buildmap <N>\tCreate a fullsize map of the game world in u7map??.pcx" << endl
			 << "\t\t(N=0: all roofs, 1: no level 2 roofs, 2: no roofs)" << endl
			 << "\t\tonly valid when used together with --bg or --si" << endl
			 << "\t\t(WARNING: requires big amounts of RAM, HD space and time!)" << endl
			 << "--nocrc\t\tDon't check crc's of .flx files" << endl
			 << "--edit\t\tStart in map-edit mode" << endl
			 << "--write-xml\tWrite 'patch/exultgame.xml'" << endl;
			
		exit(1);
	}
	if ((unsigned)run_bg +(unsigned)run_si
		+ (unsigned)run_fov + (unsigned)run_ss > 1) {
		cerr << "Error: You may only specify one of --bg, --fov, --si or --ss!" << 
									endl;
		exit(1);
	}

	if (arg_modname != "default" &&
		!(run_bg || run_si || run_fov || run_ss || (arg_gamename != "default")))
	{
		cerr << "Error: You must also specify the game to be used!" << endl;
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
		Audio::Destroy();	// Deinit the sound system.
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
#ifdef UNDER_CE
	HWND RunningHandle = FindWindow(NULL, _T("Exult Ultima7 Engine"));
	if (RunningHandle != NULL)
	{// There's a minimized (probably) instance of Exult ... let's switch to it
		//cerr << "ERROR: Already running.  Switching to running instance..." << std::endl;
		HWND OldHandle = GetForegroundWindow();
		ShowWindow(RunningHandle, SW_RESTORE);
		ShowWindow(OldHandle, SW_MINIMIZE);
		return -1;
	}
	minimized = false;
#endif

	string data_path;
	string music_path;
	// output version info
	getVersionInfo(cout);

#ifdef UNDER_CE
	WINCE_exepath = string(runpath).substr(0, string(runpath).find_last_of("\\")+1);
#endif

	// Read in configuration file
	config = new Configuration;

	if (arg_configfile != "") {
		config->read_abs_config_file(arg_configfile);
	} else {
#ifdef EASY_USER_CONFIGURATION_FILE
		if (U7exists(EASY_USER_CONFIGURATION_FILE))
			config->read_config_file(EASY_USER_CONFIGURATION_FILE);
		else
			config->read_config_file(USER_CONFIGURATION_FILE);
#else
		config->read_config_file(USER_CONFIGURATION_FILE);
#endif
	}

	// Setup virtual directories
	config->value("config/disk/data_path",data_path,EXULT_DATADIR);
	add_system_path("<DATA>", data_path);
	if (!U7exists(EXULT_FLX))
	{
		add_system_path("<DATA>", EXULT_DATADIR);
		if (!U7exists(EXULT_FLX))
		{
			add_system_path("<DATA>", "data");
			if(!U7exists(EXULT_FLX))
			{
				const char *sep = std::strrchr(runpath,'/');
				if (!sep) sep = std::strrchr(runpath,'\\');
				int plen = sep-runpath;
				char *dpath = new char[plen+10];
				std::strncpy(dpath, runpath, plen+1);
				dpath[plen+1] = 0;
				std::strcat(dpath,"data");
				cerr << "dpath = " << dpath << endl;
				add_system_path("<DATA>",dpath);
				if(!U7exists(EXULT_FLX))
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
	std::string default_music = get_system_path("<DATA>/music");
	config->value("config/disk/music_path",music_path,default_music.c_str());

	add_system_path("<MUSIC>", music_path);
	add_system_path("<STATIC>", "static");
	add_system_path("<GAMEDAT>", "gamedat");
	add_system_path("<PATCH>", "patch");
//	add_system_path("<SAVEGAME>", "savegame");
	add_system_path("<SAVEGAME>", ".");
	add_system_path("<MODS>", "mods");

	std::cout << "Exult path settings:" << std::endl;
	std::cout << "Data          : " << get_system_path("<DATA>") << std::endl;
	std::cout << "Digital music : " << get_system_path("<MUSIC>") << std::endl;
	std::cout << std::endl;

	// Check CRCs of our .flx files
	bool crc_ok = true;
	uint32 crc = crc32_syspath(EXULT_FLX);
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
	vector<string> vs = config->listkeys("config/disk/game",false);
	if (vs.size() == 0 && config->key_exists("config/disk/u7path"))
		{
		// Convert from the older format
		string data_directory;
		config->value("config/disk/u7path",data_directory,"./blackgate");
		config->set("config/disk/game/blackgate/path",data_directory,true);
		}

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

	config->value("config/debug/trace/combat",combat_trace);

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

#ifdef UNDER_CE
	GXOpenInput();

	GXKeyList keys = GXGetDefaultKeys(GX_LANDSCAPEKEYS);

	std::cout << "Up " << keys.vkUp << std::endl;
	std::cout << "Down " << keys.vkDown << std::endl;
	std::cout << "Left " << keys.vkLeft << std::endl;
	std::cout << "Right " << keys.vkRight << std::endl;
	std::cout << "A " << keys.vkA << std::endl;
	std::cout << "B " << keys.vkB << std::endl;
	std::cout << "C " << keys.vkC << std::endl;
	std::cout << "Start " << keys.vkStart << std::endl;

	gkeyboard = new Keyboard_gump();
	Touchscreen = new clsTouchscreen();
#endif

	Init();				// Create main window.

	cheat.finish_init();
	cheat.set_map_editor(arg_edit_mode);	// Start in map-edit mode?
	if (arg_write_xml)
		game->write_game_xml();

	Mouse::mouse = new Mouse(gwin);
	Mouse::mouse->set_shape(Mouse::hand);

#ifdef UNDER_CE
	gkeyboard->autopaint = false;
	gkeyboard->minimize();
	gkeyboard->autopaint = true;
#endif

	int result = Play();		// start game

#ifdef UNDER_CE
	GXCloseInput();
#endif

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

namespace ExultIcon {
	#include "exulticon.h"
}

static void SetIcon()
{
	SDL_Surface* iconsurface = SDL_CreateRGBSurfaceFrom(ExultIcon::header_data,
														ExultIcon::width,
														ExultIcon::height,
														8,
														ExultIcon::width,
														0, 0, 0, 0);
	SDL_Color iconpal[256];
	for (int i = 0; i < 256; ++i) {
		iconpal[i].r = ExultIcon::header_data_cmap[i][0];
		iconpal[i].g = ExultIcon::header_data_cmap[i][1];
		iconpal[i].b = ExultIcon::header_data_cmap[i][2];
	}
	SDL_SetPalette(iconsurface, SDL_LOGPAL, iconpal, 0, 256);

	SDL_SetColorKey(iconsurface, SDL_SRCCOLORKEY, 0);

	SDL_WM_SetIcon(iconsurface, 0);

	SDL_FreeSurface(iconsurface);
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
#ifdef WIN32
	// Due to the new menu size, the window can sometimes
	// be partially offscreen in Windows. SDL currently
	// offers no better way of doing this, so...
	// I don't know if this will be problem in other OSes,
	// so I leave it Windows-specific for now.
	// Maybe it would be best to use SDL_putenv instead?
	SDL_putenv("SDL_VIDEO_CENTERED=center");
	// Yes, SDL_putenv is GOOD.  putenv does NOT work with wince. -PTG 2007/06/11
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

#ifndef MACOSX		// Don't set icon on OS X; the external icon is *much* nicer
	SetIcon();
#endif

	int w, h, sc, sclr;

#ifdef UNDER_CE
	// WinCE default resolution is 320x240 with no scaling
	w = 320;
	h = 240;
	sc = 1;
	sclr = Image_window::SaI;
#else
	// Default resolution is 320x200 with 2x scaling
	w = 320;
	h = 200;
	sc = 2;
	sclr = Image_window::SaI;
#endif
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
		// Ensure proper values for scaleval based on sclr.
	if (sclr == Image_window::NoScaler)
		{
		config->set("config/video/scale_method","2xSaI",true);
		sclr = Image_window::SaI;
		scaleval = 2;
		}
	else if (sclr == Image_window::Hq3x)
		scaleval = 3;
	else if (sclr != Image_window::point || sclr != Image_window::interlaced ||
			sclr != Image_window::OpenGL)
		scaleval = 2;
	config->set("config/video/scale", scaleval, true);

	// Load games and mods; also stores system paths:
	gamemanager = new GameManager();

	Image_window8::set_gamma(atof(gr.c_str()), atof(gg.c_str()), atof(gb.c_str()));

	if (arg_buildmap < 0)
		{
#if defined(__zaurus__) || defined(UNDER_CE)
		gwin = new Game_window(sw, sh, scaleval, sclr);
#else
		if (arg_gamename != "default" || run_bg || run_si || run_fov || run_ss)
			gwin = new Game_window(sw, sh, scaleval, sclr);
		else
			gwin = new Game_window(400, 300, scaleval, sclr);
#endif

		current_res = find_resolution(sw, sh, scaleval);
		Audio::Init();

		bool disable_fades;
		config->value("config/video/disable_fades", disable_fades, false);
		gwin->get_pal()->set_fades_enabled(!disable_fades);
		}

	SDL_SetEventFilter(0);
	// Show the banner
	game = 0;

	do
		{
		reset_system_paths();
		fontManager.reset();
		U7FileManager::get_ptr()->reset();

		if (game)
			{
			delete game;
			game = 0;
			}
		
		ModManager *basegame = 0;
		if (run_bg)
			{
			basegame = gamemanager->get_bg();
			arg_gamename = CFG_BG_NAME;
			run_bg = false;
			}
		else if (run_fov)
			{
			basegame = gamemanager->get_fov();
			arg_gamename = CFG_FOV_NAME;
			run_fov = false;
			}
		else if (run_si)
			{
			basegame = gamemanager->get_si();
			arg_gamename = CFG_SI_NAME;
			run_si = false;
			}
		else if (run_ss)
			{
			basegame = gamemanager->get_ss();
			arg_gamename = CFG_SS_NAME;
			run_ss = false;
			}
		BaseGameInfo *newgame = 0;
		if (basegame || arg_gamename != "default")
			{
			if (!basegame)
				basegame = gamemanager->find_game(arg_gamename);
			if (basegame)
				{
				if (arg_modname != "default")
					// Prints error messages:
					newgame = basegame->get_mod(arg_modname);
				else
					newgame = basegame;
				arg_modname = "default";
				}
			else
				{
				cerr << "Game '" << arg_gamename << "' not found." << endl;
				newgame = 0;
				}
			// Prevent game from being reloaded in case the player
			// tries to return to the main menu:
			arg_gamename = "default";
			}
		else
			{
#if !(defined(__zaurus__))
#ifdef UNDER_CE
			gwin->resized(sw, sh, scaleval, sclr);
			// Ensure proper clipping:
			gwin->get_win()->set_clip(0, 0, sw, sh);
#else
			gwin->resized(400, 300, scaleval, sclr);
			// Ensure proper clipping:
			gwin->get_win()->set_clip(0, 0, 400, 300);
#endif
#endif
			ExultMenu exult_menu(gwin);
			newgame = exult_menu.run();
			}
		if (!newgame)
			{
			// Should never happen... maybe display the exult menu instead?
			cerr << "Could not find any games to run; leaving." << endl;
			exit(1);
			}

		if (arg_buildmap >= 0)
			{
			BuildGameMap(newgame);
			exit(0);
			}

		gwin->resized(sw, sh, scaleval, sclr);
		// Ensure proper clipping:
		gwin->get_win()->set_clip(0, 0, sw, sh);
		Game::create_game(newgame);

		Audio *audio = Audio::get_ptr();
		MyMidiPlayer *midi = 0;

		if (audio)
			{
			Audio::get_ptr()->Init_sfx();
			midi = audio->get_midi();
			}

		Setup_text(GAME_SI, Game::has_expansion());

					// Skip splash screen?
		bool skip_splash;
		config->value("config/gameplay/skip_splash", skip_splash);
		
		// Make sure we have a proper palette before playing the intro.
		gwin->get_pal()->load(EXULT_FLX, EXULT_FLX_EXULT0_PAL);
		gwin->get_pal()->apply();
		if(!skip_splash && (Game::get_game_type() != EXULT_DEVEL_GAME
				|| U7exists(INTRO_DAT)))
			{
			if (midi) midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_INTRO);
			game->play_intro();
			}

		if (midi)
			{
			if (arg_nomenu)
				midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_INTRO);
			else
				midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_MAINMENU);
			}
		}
	while(!game->show_menu(arg_nomenu));

	// Should not be needed anymore:
	delete gamemanager;

	Audio *audio = Audio::get_ptr();
	if (audio) {
		MyMidiPlayer *midi = audio->get_midi();
		if (midi) midi->set_timbre_lib(MyMidiPlayer::TIMBRE_LIB_GAME);
	}
	gwin->init_files();
	gwin->read_gwin();
	gwin->setup_game(arg_edit_mode);	// This will start the scene.
	// Get scale factor for mouse.
#ifdef USE_EXULTSTUDIO
#ifndef WIN32
	SDL_GetWMInfo(&info);
	xfd = ConnectionNumber(info.info.x11.display);
	Server_init();			// Initialize server (for map-editor).
	xdnd = new Xdnd(info.info.x11.display, info.info.x11.wmwindow,
		info.info.x11.window, Move_dragged_shape, Move_dragged_combo,
				Drop_dragged_shape, Drop_dragged_chunk, 
				Drop_dragged_npc, Drop_dragged_combo);
#elif !defined(UNDER_CE)
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

static void Paint_with_shape
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

/*
 *	Set a complete chunk while map-editing.
 */

static void Paint_with_chunk
	(
	SDL_Event& event,
	bool dragging			// Painting terrain.
	)
	{
	static int lastcx = -1, lastcy = -1;
	int scale = gwin->get_win()->get_scale();
	int x = event.button.x/scale, y = event.button.y/scale;
	int cx = (gwin->get_scrolltx() + x/c_tilesize)/c_tiles_per_chunk;
	int cy = (gwin->get_scrollty() + y/c_tilesize)/c_tiles_per_chunk;
	if (dragging)			// See if moving to a new chunk.
		{
		if (cx == lastcx && cy == lastcy)
			return;
		}
	lastcx = cx; lastcy = cy;
	int chnum = cheat.get_edit_chunknum();
	Drop_dragged_chunk(chnum, event.button.x, event.button.y, 0);
	}

/*
 *	Select chunks.
 */

static void Select_chunks
	(
	SDL_Event& event,
	bool dragging,			// Painting terrain.
	bool toggle
	)
	{
	static int lastcx = -1, lastcy = -1;
	int scale = gwin->get_win()->get_scale();
	int x = event.button.x/scale, y = event.button.y/scale;
	int cx = (gwin->get_scrolltx() + x/c_tilesize)/c_tiles_per_chunk;
	int cy = (gwin->get_scrollty() + y/c_tilesize)/c_tiles_per_chunk;
	if (dragging)			// See if moving to a new chunk.
		{
		if (cx == lastcx && cy == lastcy)
			return;
		}
	lastcx = cx; lastcy = cy;
	Map_chunk *chunk = gwin->get_map()->get_chunk(cx, cy);
	if (toggle)
		{
		if (!chunk->is_selected())
			cheat.add_chunksel(chunk);
		else
			chunk->set_selected(false);
		}
	else
		cheat.add_chunksel(chunk);
	gwin->set_all_dirty();
	}

/*
 *	Select for combo.
 */
static void Select_for_combo
	(
	SDL_Event& event,
	bool dragging,			// Dragging to select.
	bool toggle
	)
	{
	static Game_object *last_obj = 0;
	int scale = gwin->get_win()->get_scale();
	int x = event.button.x/scale, y = event.button.y/scale;
	int tx = (gwin->get_scrolltx() + x/c_tilesize)%c_num_tiles;
	int ty = (gwin->get_scrollty() + y/c_tilesize)%c_num_tiles;
	Game_object *obj = gwin->find_object(x, y);
	if (obj) {
		if (dragging && obj == last_obj)
			return;
		last_obj = obj;
	} else
		return;
	ShapeID id = *obj;
	Tile_coord t = obj->get_tile();
	std::string name = item_names[id.get_shapenum()];
	if (toggle)
		cheat.toggle_selected(obj);
	else if (!cheat.is_selected(obj)) {
		cheat.append_selected(obj);
		gwin->add_dirty(obj);
	}
	if (Object_out(client_socket, 
		toggle ? Exult_server::combo_toggle : Exult_server::combo_pick,
			0, t.tx, t.ty, t.tz, id.get_shapenum(),
					 id.get_framenum(), 0, name) == -1)
		cout << "Error sending shape to ExultStudio" << endl;
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
	uint32 last_rest = 0;
	uint32 last_fps = 0;
	/*
	 *	Main event loop.
	 */
	int last_x=-1, last_y=-1;
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
#if defined(WIN32) && defined(USE_EXULTSTUDIO)
		if (ticks - Game::get_ticks() < 10)
			{
			// Reducing processor usage with a slight delay.
			SDL_Delay(10 - (ticks - Game::get_ticks()));
			continue;
			}
#endif
		Game::set_ticks(ticks);
#ifdef DEBUG
		if (last_fps == 0 || ticks >= last_fps + 10000) {
			float fps = (float)gwin->blits*1000.0/
							(ticks - last_fps);
			cerr << "***#ticks = " << ticks - last_fps <<
				", blits = " << gwin->blits << ", ";
			cerr << "FPS:  " << fps << endl;
			last_fps = ticks;
			gwin->blits = 0;
		}
#endif

		SDL_Event event;
		while (!quitting_time && SDL_PollEvent(&event))
			Handle_event(event);

					// Animate unless dormant.
		if (gwin->have_focus() && !dragging)
			gwin->get_tqueue()->activate(ticks);

		// Moved this out of the animation loop, since we want movement to be
		// more responsive. Also, if the step delta is only 1 tile,
		// always check every loop
		if (!gwin->is_moving() || gwin->get_step_tile_delta() == 1)
			{
			int x, y;// Check for 'stuck' Avatar.
			int ms = SDL_GetMouseState(&x, &y);
			if ((SDL_BUTTON(3) & ms) && !right_on_gump)
				gwin->start_actor(x/scale, y/scale, 
					Mouse::mouse->avatar_speed);
			else if (ticks > last_rest)
				{
				gwin->get_main_actor()->resting(ticks - last_rest);
				last_rest = ticks;
				}
			}

		// handle delayed showing of clicked items (wait for possible dblclick)
		if (show_items_clicked && ticks > show_items_time)
		{
			gwin->show_items(show_items_x, show_items_y, false);
			show_items_clicked = false;
		}

		// Lerping stuff...
		int lerp = gwin->is_lerping_enabled();
		if (lerp)
		{
			// Always repaint,
			Actor *act = gwin->get_camera_actor();
			int mswait = (act->get_frame_time()*lerp)/100;
			if (mswait <= 0) mswait = (gwin->get_std_delay()*lerp)/100;

			// Force a reset if position changed
			if (last_x != gwin->get_scrolltx() || last_y != gwin->get_scrollty())
			{
				//printf ("%i: %i -> %i, %i -> %i\n", ticks, last_x, gwin->get_scrolltx(), last_y, gwin->get_scrollty());
				gwin->lerp_reset();
				last_repaint = ticks;
			}
			last_x = gwin->get_scrolltx();
			last_y = gwin->get_scrollty();

			// Is lerping (smooth scrolling) enabled
			if (mswait && ticks < (last_repaint+mswait*2))
			{
				gwin->paint_lerped(((ticks-last_repaint)*0x10000)/mswait);
			}
		}
		else // No lerping
		{
#if HAVE_OPENGL
					// OpenGL?  Repaint all each time.
			if (GL_manager::get_instance())
				{
					// Show animation every 1/20 sec.
				if (ticks > last_repaint + 50 || gwin->was_painted())
					gwin->paint();
				}
			else
#endif
			if (gwin->is_dirty())	// Note the ending else in the above #if!
				gwin->paint_dirty();
				// Reset it for lerping.
			last_x = last_y = -1;
		}
		Mouse::mouse->show();	// Re-display mouse.
					// Rotate less often if scaling and 
					//   not paletized.
		int rot_speed = 100 << (gwin->get_win()->is_palettized() ||
								scale==1?0:1);
		if (ticks > last_rotate + rot_speed &&
		    !GL_manager::get_instance())	//++++Disable in OGL.
			{		// (Blits in simulated 8-bit mode.)
			gwin->get_win()->rotate_colors(0xfc, 3, 0);
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
#ifdef UNDER_CE
			if (gkeyboard->handle_event(&event))
				break;
			Touchscreen->handle_event(&event);
#endif
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
					Paint_with_shape(event, false);
					break;
					}
				else if (cheat.get_edit_chunknum() >= 0 &&
				  cheat.get_edit_mode() == Cheat::paint_chunks)
					{
					Paint_with_chunk(event, false);
					break;
					}
				else if (cheat.get_edit_mode() ==
							Cheat::select_chunks)
					{
					Select_chunks(event, false,
						SDL_GetModState()&KMOD_CTRL);
					break;
					}
				else if (cheat.get_edit_mode() ==
							Cheat::combo_pick)
					{
					Select_for_combo(event, false,
						SDL_GetModState()&KMOD_CTRL);
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
			left_down_x = x; left_down_y = y;
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
#ifdef UNDER_CE
			if (gkeyboard->handle_event(&event))
				break;
			Touchscreen->handle_event(&event);
#endif
		int x = event.button.x/scale, y = event.button.y/scale;
		if (event.button.button == 3)
			{
			uint32 curtime = SDL_GetTicks();
					// Last click within .5 secs?
			if (gwin->get_allow_double_right_move() && curtime - last_b3_click < 500)
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
				{
				gwin->stop_actor();
				if (Combat::is_paused() && gwin->in_combat())
					gwin->paused_combat_select(x, y);
				}
			last_b3_click = curtime;
			}
		else if (event.button.button == 1)
			{
			uint32 curtime = SDL_GetTicks();
			bool click_handled = false;
			if (dragging) {
				click_handled = gwin->drop_dragged(x, y,
								dragged);
				Mouse::mouse->set_speed_cursor();
			}
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500 &&
				left_down_x - 1 <= x && x <= left_down_x + 1 &&
				left_down_y - 1 <= y && y <= left_down_y + 1)
			{
				dragging = dragged = false;
				gwin->double_clicked(x, y);
				Mouse::mouse->set_speed_cursor();
				show_items_clicked = false;
				break;
				}
			if (!dragging || !dragged)
				last_b1_click = curtime;

			if (!click_handled && 
				left_down_x - 1 <= x && x <= left_down_x + 1 &&
				left_down_y - 1 <= y && y <= left_down_y + 1)
			{
				show_items_x = x; show_items_y = y;
				// Identify item(s) clicked on.
				if (cheat.in_map_editor()) {
					gwin->show_items(x,y,(SDL_GetModState() & KMOD_CTRL) != 0);
				} else {
					show_items_time = curtime + 500;
					show_items_clicked = true;
				}
			}
			dragging = dragged = false;
			}
		break;
		}
	case SDL_MOUSEMOTION:
		{
		Mouse::mouse->move(event.motion.x / scale, 
						event.motion.y / scale);
		if (!dragging)
			Mouse::mouse->set_speed_cursor();
		Mouse::mouse_update = true;	// Need to blit mouse.
		if (right_on_gump &&
			!(gump_man->can_right_click_close() &&
			  gump_man->gump_mode() && 
			  gump_man->find_gump(event.motion.x / scale,
								  event.motion.y / scale, false))) {
			right_on_gump = false;
		}

					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			{
#ifdef USE_EXULTSTUDIO			// Painting?
			if (cheat.in_map_editor())
				{ 
				if (cheat.get_edit_shape() >= 0 &&
				    (cheat.get_edit_mode() == Cheat::paint ||
					(SDL_GetModState() & KMOD_SHIFT)))
					{
					Paint_with_shape(event, true);
					break;
					}
				else if (cheat.get_edit_chunknum() >= 0 &&
				  cheat.get_edit_mode() == Cheat::paint_chunks)
					{
					Paint_with_chunk(event, true);
					break;
					}
				else if (cheat.get_edit_mode() ==
							Cheat::select_chunks)
					{
					Select_chunks(event, true,
						SDL_GetModState()&KMOD_CTRL);
					break;
					}
				else if (cheat.get_edit_mode() ==
							Cheat::combo_pick)
					{
					Select_for_combo(event, true,
						SDL_GetModState()&KMOD_CTRL);
					break;
					}
				}
#endif
			dragged = gwin->drag(event.motion.x / scale, 
						event.motion.y / scale);
			}
					// Dragging with right?
		else if ((event.motion.state & SDL_BUTTON(3)) && !right_on_gump)
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
		gwin->get_gump_man()->okay_to_quit();
		break;
	case SDL_KEYDOWN:		// Keystroke.
	case SDL_KEYUP:
		if (!dragging &&	// ESC while dragging causes crashes.
		    !gwin->get_gump_man()->handle_kbd_event(&event))
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
	bool drag_ok,			// Okay to drag/close while here.
	Paintable *paint,		// Paint this each cycle if OpenGL.
	bool rotate_colors		// If the palette colors should rotate.
	)
	{
	dragging = false;		// Init.
	uint32 last_rotate = 0;

	while (1)
		{
		SDL_Event event;
		Delay();		// Wait a fraction of a second.

		uint32 ticks = SDL_GetTicks();
		Game::set_ticks(ticks);
		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;
		
		if (rotate_colors)
			{
			int scale = gwin->get_fastmouse() ? 1 : 
							gwin->get_win()->get_scale();
			int rot_speed = 100 << (gwin->get_win()->is_palettized() ||
									scale==1?0:1);
			if (ticks > last_rotate + rot_speed &&
				!GL_manager::get_instance())	//++++Disable in OGL.
				{		// (Blits in simulated 8-bit mode.)
				gwin->get_win()->rotate_colors(0xfc, 3, 0);
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
			}

		// Mouse scale factor
		int scale = gwin->get_fastmouse() ? 1 
				: gwin->get_win()->get_scale();
		static bool rightclick;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEBUTTONDOWN:
#ifdef UNDER_CE
				if (gkeyboard->handle_event(&event))
					break;
				Touchscreen->handle_event(&event);
#endif
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
#ifdef UNDER_CE
				if (gkeyboard->handle_event(&event))
					break;
				Touchscreen->handle_event(&event);
#endif
				if (event.button.button == 1)
					{
					x = event.button.x / scale;
					y = event.button.y / scale;
					bool drg = dragging, drged = dragged;
					dragging = dragged = false;
					if (!drg ||
					    !gwin->drop_dragged(x, y, drged))
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
		if (GL_manager::get_instance())
			{
			gwin->paint();
			if (paint)
				paint->paint();
			}
		else if (dragging)
			gwin->paint_dirty();
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
	bool drag_ok,			// Okay to drag/close while here.
	Paintable *paint,		// Paint this over everything else.
	bool rotate_colors		// If the palette colors should rotate.
	)
	{
	if (chr)
		*chr = 0;		// Init.
	Mouse::Mouse_shapes saveshape = Mouse::mouse->get_shape();
	if (shape != Mouse::dontchange)
		Mouse::mouse->set_shape(shape);
	if (paint)
		paint->paint();
	Mouse::mouse->show();
	gwin->show(1);			// Want to see new mouse.
	gwin->get_tqueue()->pause(Game::get_ticks());
	int ret = Get_click(x, y, chr, drag_ok, paint, rotate_colors);
	gwin->get_tqueue()->resume(Game::get_ticks());
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
			eye.paint_shape(topx + spr->get_xleft(),
					topy + spr->get_yabove());
			int sizex = (w - 320)/2, sizey = (h - 200)/2;
			if (sizey)	// Black-fill area outside original resolution.
				{
				gwin->get_win()->fill8(0, w, sizey, 0, 0);
				gwin->get_win()->fill8(0, w, sizey, 0, h - sizey);
				}
			if (sizex)
				{
				gwin->get_win()->fill8(0, sizex, 200, 0, sizey);
				gwin->get_win()->fill8(0, sizex, 200, w - sizex, sizey);
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
	Effects_manager *eman = gwin->get_effects();

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
		if (!silent) eman->center_text("Too many screenshots");
	} else {
		SDL_RWops *dst = SDL_RWFromFile(fn, "wb");

		if (gwin->get_win()->screenshot(dst)) {
			cout << "Screenshot saved in " << fn << endl;
			if (!silent) eman->center_text("Screenshot");
		} else {
			if (!silent) eman->center_text("Screenshot failed");
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
	gwin->get_pal()->apply(true);	// So new brightness applies.

	// Message
#ifdef HAVE_SNPRINTF
	Image_window8::get_gamma(r, g, b);	
	snprintf (text, 256, "Gamma Set to R: %01.2f G: %01.2f B: %01.2f", r, g, b);
#else
	strncpy (text, "Gamma Changed", 256);
#endif
	gwin->get_effects()->center_text(text);	

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

void BuildGameMap(BaseGameInfo *game)
{
	int w, h, sc, sclr;

	// create 2048x2048 screenshots of the full Ultima 7 map.
	// WARNING!! Takes up lots of memory and diskspace!
	if (arg_buildmap >= 0) {
		int maplift = 16;
		switch(arg_buildmap) {
			case 0: maplift = 16; break;
			case 1: maplift = 10; break;
			case 2: maplift = 5; break;
		}
		
		h = w = c_tilesize * c_tiles_per_schunk; sc = 1, sclr = Image_window::point;
		Image_window8::set_gamma(1, 1, 1);

		string	fullscreenstr;		// Check config. for fullscreen mode.
		config->value("config/video/fullscreen",fullscreenstr,"no");
		// set windowed mode
		config->set("config/video/fullscreen","no",false);
		gwin = new Game_window(w, h, sc, sclr);
		// restore original fullscreen setting
		config->set("config/video/fullscreen",fullscreenstr,true);
		Audio::Init();
		current_res = find_resolution(w, h, sc);
		Game::create_game(game);
		gwin->init_files(false); //init, but don't show plasma	
		gwin->get_map()->init();// +++++Got to clean this up.
		gwin->get_pal()->set(0);
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
		Audio::Destroy();
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
		ptx += tiles_right;
		pty += tiles_below;
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
	int pix = Shape_manager::get_instance()->get_special_pixel(
								POISON_PIXEL);
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
	Shape_info& info = ShapeID::get_info(shape);
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
	Shape_info& info = ShapeID::get_info(shape);
	int sclass = info.get_shape_class();
					// Is it an ireg (changeable) obj?
	ireg = (sclass != Shape_info::unusable &&
		sclass != Shape_info::building);
	Game_object *newobj;
	if (ireg)
		newobj = gwin->get_map()->create_ireg_object(
						info, shape, frame, 0, 0, 0);
	else
		newobj = gwin->get_map()->create_ifix_object(shape, frame);
	return newobj;
	}

/*
 *	Drop a shape dragged from a shape-chooser via drag-and-drop.
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
		Map_chunk *chunk = gwin->get_map()->get_chunk(cx, cy);
		Chunk_terrain *ter = chunk->get_terrain();
		tx %= c_tiles_per_chunk; ty %= c_tiles_per_chunk;
		ShapeID curid = ter->get_flat(tx, ty);
		if (sid.get_shapenum() != curid.get_shapenum() ||
		    sid.get_framenum() != curid.get_framenum())
			{
			ter->set_flat(tx, ty, sid);
			gwin->get_map()->set_chunk_terrains_modified();
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
	Dragging_info drag(newobj);
	drag.drop(x, y, true);		// (Dels if it fails.)
	}

/*
 *	Drop a chunk dragged from a chunk-chooser via drag-and-drop.
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
 *	Drop a npc dragged from a npc-chooser via drag-and-drop.
 */

static void Drop_dragged_npc
	(
	int npcnum,
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
	cout << "Set npc (" << npcnum << ')' << endl;
	Actor *npc = gwin->get_npc(npcnum);
	if (!npc)
		return;
	if (npc->is_pos_invalid())	// Brand new?
		npc->clear_flag(Obj_flags::dead);
	else
		npc->remove_this(true);	// Remove if already on map.
	Dragging_info drag(npc);
	if (drag.drop(x, y))
		npc->set_unused(false);
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
			Map_chunk *chunk = gwin->get_map()->get_chunk(cx, cy);
			Chunk_terrain *ter = chunk->get_terrain();
			ntx %= c_tiles_per_chunk; nty %= c_tiles_per_chunk;
			ter->set_flat(ntx, nty, sid);
			gwin->get_map()->set_chunk_terrains_modified();
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
