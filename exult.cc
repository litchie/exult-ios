/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Exult.cc - X-windows Ultima7 map browser.
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

#include <stdlib.h>
#include <ctype.h>

// #ifdef HAVE_SYS_TIME_H
#ifdef XWIN  /* Only needed in XWIN. */
#include <sys/time.h>
#endif
#include <unistd.h>

#include "SDL.h"
#include "SDL_syswm.h"

#ifdef WIN32
#include <mmsystem.h>   //for MM_MCINOTIFY message
#include "audio/midi_drivers/win_MCI.h"
#endif


#include "gamewin.h"
#include "actors.h"
#include "ucmachine.h"
#include "fnames.h"
#include "Audio.h"
#include "Configuration.h"
#include "mouse.h"
#include "gumps.h"
#include "effects.h"
#include "args.h"
#include "game.h"
#include "barge.h"
#include "cheat.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;

Configuration *config;
Cheat cheat;

/*
 *	Globals:
 */
Game_window *gwin = 0;
static string data_path;
unsigned char quitting_time = 0;	// 1 = Time to quit, 2 = Restart.
Mouse *mouse = 0;
int scale = 0;				// 1 if scaling X2.

bool intrinsic_trace = false;		// Do we trace Usecode-intrinsics?
bool usecode_trace = false;		// Do we trace Usecode-instruction?

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
	{ 320, 200, 2 },
	{ 320, 240, 2 },
	{ 512, 384, 1 },
	{ 640, 480, 1 },
	{ 800, 600, 1 }
};
int num_res = sizeof(res_list)/sizeof(struct resolution);
int current_res = 0;

/*
 *	Local functions:
 */
int exult_main(void);
static void Init();
static int Play();
static void Handle_keystroke(SDLKey ch, int shift, int alt, int ctrl, Uint16 unicode);
int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);
int Modal_gump(Modal_gump_object *, Mouse::Mouse_shapes);
static void Try_key(Game_window *);
void increase_resolution (void);
void decrease_resolution (void);
int find_resolution(int w, int h, int s);
bool get_play_intro (void);
void set_play_intro (bool);
void toggle_fullscreen (void);
void quick_save (void);
void quick_restore (void);
void toggle_combat (void);
void target_mode (void);
void gump_next_inventory (void);
void gump_next_stats (void);
void gump_file (void);
void show_about (void);
void show_help (void);
void show_cheat_help (void);

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
	bool	needhelp=false;
	string	gamename("default");
	Args    parameters;
	int		result;

	// Declare everything from the commandline that we're interested in.
	parameters.declare("-h",&needhelp,true);
	parameters.declare("--help",&needhelp,true);
	parameters.declare("/?",&needhelp,true);
	parameters.declare("/h",&needhelp,true);
//	parameters.declare("-game",&gamename,"default");

	// Process the args
	parameters.process(argc,argv);

	if(needhelp)
	{
		cerr << "Usage: exult [--help|-h|/?|/h] [-game GAMENAME] " << endl <<
			"--help\t\tShow this information" << endl <<
			"-game GAMENAME\tSet the game data name to play" << endl <<
			"\t(refer to the documentation)" << endl;
		exit(1);
	}
	
	try
	{
		result = exult_main();
	}
	catch( const exult_exception & e )
	{
		cerr << "An exception occured: " << e.what() << " (errno = " << e.get_errno() << endl;
		if( e.get_errno() != 0)
			perror("Error Description");
	}
	catch(...)
	{
	}
	
	return result;
}


/*
 *	Main program.
 */

int exult_main(void)
{
	cout << "Exult V" << VERSION << "." << endl;

	// Read in configuration file
	config = new Configuration;
	config->read_config_file(USER_CONFIGURATION_FILE);

	// Setup virtual directories
	config->value("config/disk/data_path",data_path,EXULT_DATADIR);
	cout << "Data path = " << data_path << endl;
	add_system_path("<DATA>", data_path.c_str());
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
				exit(-1);
			}
		}
	}
	add_system_path("<STATIC>", "static");
	add_system_path("<GAMEDAT>", "gamedat");
	add_system_path("<SAVEGAME>", "savegame");

	// Convert from old format if needed
	vector<string> vs=config->listkeys("config/disk/game",false);
	if(vs.size()==0) {
		// Convert from the older format
		string data_directory;
		config->value("config/disk/u7path",data_directory,".");
		config->set("config/disk/game/blackgate/path",data_directory,true);
		string	s("blackgate");
		config->set("config/disk/game/blackgate/title",s,true);
		vs.push_back(s);
	}

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

	mouse = new Mouse(gwin);
	mouse->set_shape(Mouse::hand);

	int result = Play();		// start game
//	delete config;			// free configuration object
	return result;
}


static int Filter_intro_events(const SDL_Event *event);
static void Handle_events(unsigned char *stop);
static void Handle_event(SDL_Event& event);

#ifdef XWIN
static Display *display = 0;
static int xfd = -1;			// X connection #.

/*
 *	Here's a somewhat better way to delay in X:
 */
inline void X_Delay
	(
	)
	{
	fd_set rfds;
	struct timeval timer;
	timer.tv_sec = 0;
	timer.tv_usec = 50000;		// Try 1/50 second.
	FD_ZERO(&rfds);
	FD_SET(xfd, &rfds);
					// Wait for timeout or event.
	select(xfd + 1, &rfds, 0, 0, &timer);
	}
#endif

/*
 *	Initialize and create main window.
 */
static void Init
	(
	)
	{
	Uint32 init_flags = SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO;
#ifdef NO_SDL_PARACHUTE
	init_flags |= SDL_INIT_NOPARACHUTE;
#endif

	if (SDL_Init(init_flags) < 0)
		{
		cerr << "Unable to initialize SDL: " << SDL_GetError() << endl;
		exit(-1);
		}
	atexit(SDL_Quit);
	
	SDL_SysWMinfo info;		// Get system info.
	SDL_ShowCursor(0);
	SDL_VERSION(&info.version);

	SDL_EnableUNICODE(1);	// Activate unicode translation for keypresses
	
#ifdef XWIN
        SDL_GetWMInfo(&info);
        display = info.info.x11.display;
        xfd = ConnectionNumber(display);
#endif
	
	int w, h, sc;

	// Default resolution is 320x200 with 2x scaling
	w = 320;
	h = 200;
	sc = 2;

	int sw, sh, scaleval;
	config->value("config/video/width", sw, w);
	config->value("config/video/height", sh, h);
	config->value("config/video/scale", scaleval, sc);
	gwin = new Game_window(sw, sh, scaleval);
	current_res = find_resolution(sw, sh, scaleval);
	Audio::get_ptr();

	string disable_fades;
	config->value("config/video/disable_fades", disable_fades, "no");
	if (disable_fades == "yes")
		gwin->set_fades_enabled(false);

#ifdef WIN32
	//enable unknown (to SDL) window messages, including MM_MCINOTIFY
	//(for MIDI repeats)
	//SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif //WIN32
	SDL_SetEventFilter(0);
	// Show the banner
	ExultMenu exult_menu(gwin);
	Exult_Game mygame = exult_menu.run();
	Game::create_game(mygame);

	string yn;

	gwin->init_files();
					// Skip splash screen?
	config->value("config/gameplay/skip_splash", yn, "no");
	if(yn == "no") 
		game->play_intro();
	game->show_menu();
	gwin->set_mode(Game_window::normal);
	SDL_SetEventFilter(Filter_intro_events);
	gwin->setup_game();		// This will start the scene.
					// Get scale factor for mouse.
	if (gwin->get_win())
		scale = gwin->get_win()->get_scale() == 2 ? 1 : 0;
	}

/*
 *	Play game.
 */

static int Play()
{
	do
	{
		quitting_time = 0;
		Handle_events(&quitting_time);
		if( quitting_time == 2 )
			gwin->read();	// Restart
	}
	while (quitting_time == 2);
	delete gwin;
	delete mouse;
	delete Audio::get_ptr();	// Follow not this pointer, now, for
					// that way lies madness.
	delete config;
	return (0);
}

/*
 *	Delay between animations.
 */

inline void Delay
	(
	)
	{
#ifdef XWIN
	X_Delay();
#else					/* May use this for Linux too. */
	SDL_Delay(20);			// Try 1/50 second.
#endif
	}

/*
 *	Verify user wants to quit.
 *
 *	Output:	1 to quit.
 */
static int Okay_to_quit
	(
	)
	{
	if (Yesno_gump_object::ask("Do you really want to quit?"))
		quitting_time = 1;
	return quitting_time;
	}

/*
 *	Statics used below:
 */
static bool show_mouse = false;		// display mouse in main loop?
static bool dragging = false;		// Object or gump being moved.
static bool dragged = false;		// Flag for when obj. moved.
static bool mouse_update = 0;		// Mouse moved/changed.
static unsigned int altkeys = 0;	// SDL doesn't seem to handle ALT
					//   right, so we'll keep track.
					// 1/6, 1/10, 1/20 frame rates.
const int slow_speed = 166, medium_speed = 100, fast_speed = 50;
static int avatar_speed = slow_speed;	// Avatar speed (frame delay in
					//    1/1000 secs.)

/*
 *	Filter out events during the intro. sequence.
 */
static int Filter_intro_events
	(
	const SDL_Event *event
	)
	{
	
	if (gwin->get_mode() == Game_window::conversation)
		{
		SDL_SetEventFilter(0);	// Intro. conversation started.
		show_mouse = true;
		return 1;
		}
	if (gwin->get_usecode()->get_global_flag(
					Usecode_machine::did_first_scene))
		{
		SDL_SetEventFilter(0);	// Intro. is done.
		show_mouse = true;
		return 0;
		}
	switch (event->type)
		{
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
	case SDL_KEYDOWN:
	case SDL_MOUSEMOTION:
	case SDL_KEYUP:
		return 0;		// The intro. is running.
		}
	return (1);
	}

/*
 *	Handle events until a flag is set.
 */

static void Handle_events
	(
	unsigned char *stop
	)
	{
	unsigned long last_repaint = 0;		// For insuring animation repaints.
	/*
	 *	Main event loop.
	 */
	while (!*stop)
		{
		int rotate = 0;		// 1 to rotate colors.
		Delay();		// Wait a fraction of a second.

		mouse->hide();		// Turn off mouse.
		mouse_update = false;

		SDL_Event event;
		while (!*stop && SDL_PollEvent(&event))
			Handle_event(event);
					// Get current time.
		unsigned long ticks = SDL_GetTicks();
					// Animate unless dormant.
		if (gwin->have_focus() && !dragging)
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
		if (ticks > last_repaint + 50 || gwin->was_painted())
					// This avoids jumpy walking:
			{
			gwin->paint_dirty();
			last_repaint = ticks;
			rotate = 1;
			int x, y;// Check for 'stuck' Avatar.
			if (!gwin->is_moving() &&
			    !gwin->was_teleported())
				{
				int ms = SDL_GetMouseState(&x, &y);
				if ((SDL_BUTTON(3) & ms) &&
				    gwin->get_usecode()->get_global_flag(
				    Usecode_machine::did_first_scene))
					gwin->start_actor(x >> scale, 
							  y >> scale, 
							  avatar_speed);
				}
			}

		if (show_mouse)
			mouse->show();	// Re-display mouse.

		if (rotate)
			{		// (Blits in simulated 8-bit mode.)
			gwin->get_win()->rotate_colors(0xf0, 4, 0);
			gwin->get_win()->rotate_colors(0xe8, 8, 0);
			gwin->get_win()->rotate_colors(0xe0, 8, 1);
			}
		if (!gwin->show() &&	// Blit to screen if necessary.
		    mouse_update)	// If not, did mouse change?
			mouse->blit_dirty();
		}
	}

/*
 *	Set mouse and speed.
 */

inline void Set_mouse_and_speed
	(
	int mousex, int mousey		// Physical mouse location.
	)
	{
	int ax, ay;			// Get Avatar/barge screen location.
	Barge_object *barge = gwin->get_moving_barge();
	if (barge)
		{			// Use center of barge.
		gwin->get_shape_location(barge, ax, ay);
		ax -= barge->get_xtiles()*(tilesize/2);
		ay -= barge->get_ytiles()*(tilesize/2);
		}
	else				
		gwin->get_shape_location(gwin->get_main_actor(), ax, ay);
	int dy = ay - (mousey >> scale), dx = (mousex >> scale) - ax;
	Direction dir = Get_direction(dy, dx);
	int dist = dy*dy + dx*dx;
	if (dist < 40*40)
		{
		if(gwin->in_combat())
			mouse->set_short_combat_arrow(dir);
		else
			mouse->set_short_arrow(dir);
		avatar_speed = slow_speed;
		}
	else if (dist < 75*75)
		{
		if(gwin->in_combat())
			mouse->set_medium_combat_arrow(dir);
		else
			mouse->set_medium_arrow(dir);
		avatar_speed = medium_speed;
		}
	else
		{		// No long arrow in combat: use medium
		if(gwin->in_combat())
			mouse->set_medium_combat_arrow(dir);
		else
			mouse->set_long_arrow(dir);
		avatar_speed = fast_speed;
		}
	}

/*
 *	Handle an event.  This should work for all platforms.
 */

static void Handle_event
	(
	SDL_Event& event
	)
	{
					// For detecting double-clicks.
	static unsigned long last_b1_click = 0;
	//cout << "Event " << (int) event.type << " received"<<endl;
	switch (event.type)
		{
	case SDL_MOUSEBUTTONDOWN:
		if (gwin->get_mode() != Game_window::normal &&
		    gwin->get_mode() != Game_window::gump)
			break;
		if (event.button.button == 1)
			{
			dragging = gwin->start_dragging(
					event.button.x >> scale,
					event.button.y >> scale);
			dragged = false;
			}
					// Move sprite toward mouse
					//  when right button pressed.
		if (event.button.button == 3 && 
		    gwin->get_mode() == Game_window::normal)
			{		// Try removing old queue entry.
			gwin->get_tqueue()->remove(gwin->get_main_actor());
			gwin->start_actor(event.button.x >> scale, 
				event.button.y >> scale, avatar_speed);
			}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 3)
			{
#if 0
			if (gwin->get_mode() != Game_window::normal &&
			    gwin->get_mode() != Game_window::gump)
				break;
#endif
			gwin->stop_actor();
			}
		else if (event.button.button == 1)
			{
			unsigned long curtime = SDL_GetTicks();
			bool click_handled = false;
			if (dragging) {
				click_handled = gwin->drop_dragged(event.button.x >> scale, 
					event.button.y >> scale, dragged);
			}
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500)
				{
				dragging = false;
				gwin->double_clicked(event.button.x >> scale, 
						event.button.y >> scale);
				if (gwin->get_mode() == Game_window::gump)
					mouse->set_shape(Mouse::hand);
				break;
				}
			last_b1_click = curtime;
			if (!click_handled)
					// Identify item(s) clicked on.
				gwin->show_items(event.button.x >> scale, 
						event.button.y >> scale);
			dragging = false;
			}
		break;
	case SDL_MOUSEMOTION:
		{
		mouse->move(event.motion.x >> scale, event.motion.y >> scale);
		if (gwin->get_mode() == Game_window::normal)
			Set_mouse_and_speed(event.motion.x, event.motion.y);
		mouse_update = true;	// Need to blit mouse.
		if (gwin->get_mode() != Game_window::normal &&
		    gwin->get_mode() != Game_window::gump)
			break;
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			dragged = gwin->drag(event.motion.x >> scale, 
						event.motion.y >> scale);
					// Dragging with right?
		if ((event.motion.state & SDL_BUTTON(3)) &&
					// But not right after teleport.
		    !gwin->was_teleported())
			gwin->start_actor(event.motion.x >> scale, 
					event.motion.y >> scale, avatar_speed);
		break;
		}
	case SDL_ACTIVEEVENT:
					// Get scale factor for mouse.
		scale = gwin->get_win()->get_scale() == 2 ? 1 : 0;

		if (event.active.state & SDL_APPMOUSEFOCUS)
			{
			if (event.active.gain)
				{
				int x, y;
				SDL_GetMouseState(&x, &y);
				mouse->set_location(x >> scale, y >> scale);
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
		if (event.active.state & SDL_APPACTIVE)
					// Became active.
			if (event.active.gain)
				gwin->init_actors();
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
		Handle_keystroke(event.key.keysym.sym,
			event.key.keysym.mod & KMOD_SHIFT,
			(event.key.keysym.mod & KMOD_ALT) || altkeys,
			event.key.keysym.mod & KMOD_CTRL,
			event.key.keysym.unicode);
		break;
	case SDL_KEYUP:			// Key released.
		switch (event.key.keysym.sym)
			{
		case SDLK_RALT:		// Right alt.
		case SDLK_RMETA:
			altkeys &= ~1;	// Clear flag.
			break;
		case SDLK_LALT:
		case SDLK_LMETA:
			altkeys &= ~2;
			break;
		default: break;
			}
		break;
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
 *	Get the i'th party member, with the 0'th being the Avatar.
 */

static Actor *Get_party_member
	(
	int num				// 0=avatar.
	)
	{
	int npc_num = 0;	 	// Default to Avatar
	if (num > 0)
		npc_num = gwin->get_usecode()->get_party_member(num - 1);
	return gwin->get_npc(npc_num);
	}

/*
 *	Handle a keystroke.
 */

static void Handle_keystroke
	(
	SDLKey sym,
	int shift,
        int alt,
	int ctrl,
	Uint16 unicode
	)
	{
	SDLMod mods = SDL_GetModState();

	switch (sym)
		{
	case SDLK_RALT:			// Right alt.
	case SDLK_RMETA:
		altkeys |= 1;		// Set flag.
		break;
	case SDLK_LALT:
	case SDLK_LMETA:
		altkeys |= 2;
		break;
	case SDLK_PLUS:
	case SDLK_KP_PLUS:
		if(alt && !ctrl) {		// Alt-+ : Increase resolution
			increase_resolution();

		} else if (!alt && !ctrl) {	// + : Brighten
			gwin->brighten(20);
		}
		break;
	case SDLK_MINUS:
	case SDLK_KP_MINUS:
		if(alt && !ctrl) {		// Alt-- : Decrease resolution
			decrease_resolution();

		} else if (!alt && !ctrl) {	// - : Darken
			gwin->brighten(-20);
		}
		break;
	case SDLK_ESCAPE:
		if (!alt && !ctrl) {		// ESC : close gumps or quit
			if (gwin->get_mode() == Game_window::gump)
				gwin->end_gump_mode();
			else			// For now, quit.
				Okay_to_quit();
		}
		break;
	case SDLK_RIGHT:
		if (cheat() && !alt && !ctrl) {
			for (int i = 16; i; i--)
				gwin->view_right();
		}
		break;
	case SDLK_LEFT:
		if (cheat() && !alt && !ctrl) {
			for (int i = 16; i; i--)
				gwin->view_left();
		}
		break;
	case SDLK_DOWN:
		if (cheat() && !alt && !ctrl) {
			for (int i = 16; i; i--)
				gwin->view_down();
		}
		break;
	case SDLK_UP:
		if (cheat() && !alt && !ctrl) {
			for (int i = 16; i; i--)
				gwin->view_up();
		}
		break;
	case SDLK_HOME:
		if (cheat() && !alt && !ctrl) {	// Home : center screen on avatar
			gwin->center_view(gwin->get_main_actor()->get_abs_tile_coord());
			gwin->paint();
		}
		break;
	case SDLK_F4:
		if (!alt && !ctrl) {		// F4 : Toggle fullscreen mode
			toggle_fullscreen();
		}
		break;
	case SDLK_F10:
		if (cheat() && !alt && !ctrl) {	// F10 : show endgame
			game->end_game(shift==0);
			gwin->set_palette(0);
			gwin->paint();
			gwin->fade_palette (50, 1, 0);
		}
		break;
	case SDLK_F11:				// F11 : show SI intro 
		if (cheat() && !alt && !ctrl && Game::get_game_type() == SERPENT_ISLE) {
			game->set_jive();
			game->play_intro();
			game->clear_jive();
			gwin->set_palette(0);
			gwin->paint();
			gwin->fade_palette (50, 1, 0);
		}
		break;
	default:
		if ((unicode & 0xFF80) == 0)
		{
		int chr = (unicode & 0x7f);
		if (chr < 0x20)		// Control char?  Branch on orig.
			chr = (int) sym;
		switch (chr)
			{
			case 'b':
				if(ctrl && !alt) {		// Ctrl-b : Shape browser
					cheat.shape_browser();

				} else if (!alt && !ctrl) {	// b : Open spellbook.
					gwin->activate_item(761);
				}
				break;
			case 'c':
				if (ctrl && !alt) {		// Ctrl-c : Create last shape viewed.
					cheat.create_last_shape();

				} else if (!ctrl && !alt) {	// c : Combat mode
					toggle_combat();
				}
				break;
			case 'd':
				if (ctrl && !alt) {		// Ctrl-d : delete what mouse is on.
					cheat.delete_object();
				}
				break;
			case 'e':
				if (!alt && !ctrl) {		// e : toggle eggs display
					cheat.toggle_eggs();
				}
				break;
			case 'f':
				if (!ctrl && !alt) {		// f : Feed food.
					gwin->activate_item(377);	// +++++Black gate.
				}
				break;
			case 'g':
				if (alt && !ctrl) {		// Alt-g : toggle god-mode
					cheat.toggle_god();

				} else if (!ctrl && !alt) {	// g :  Change Avatars gender
					cheat.change_gender();
				}
				break;
			case 'h':
				if (!alt && !ctrl) {		// h : help
					show_help();
				
				} else if (ctrl && !alt) {	// Ctrl-h : cheat help
					show_cheat_help();
				}
				break;
			case 'i':
				if (alt && !ctrl) {    		// Alt-i : infravision
					cheat.toggle_infravision();

				} else if (!alt && !ctrl) {	// i : show inventory
					gump_next_inventory();
				}
				break;
			case 'k':
				if (!alt && !ctrl) {		// k : find key
					Try_key(gwin);
				}
				break;
			case 'l':
				if(!alt && !ctrl) {		// l : decrement skip_lift
					cheat.dec_skip_lift();
				}
				break;
			case 'm':
				if (ctrl && alt) {  		// Ctrl-Alt-m : hack mover
					cheat.toggle_hack_mover();

				} else if (ctrl && !alt) {	// Ctrl-m : 100 gold coins
					cheat.create_coins();

				} else if (alt && !ctrl) {	// Alt-m : next song
								// Shift-Alt-m : previous song
					static int mnum = 0;
					if (shift && mnum > 0)
						Audio::get_ptr()->start_music(--mnum, 0);
					else
						Audio::get_ptr()->start_music(mnum++, 0);

				} else if (!alt && !ctrl) {	// m : Show map.
					gwin->activate_item(178);	//++++Black gate.
				}
				break;
			case 'n':
				if (alt && !ctrl) {		// Alt-n : Toggle Naked flag
					cheat.toggle_naked();
				}
				break;
			case 'p':
				if (alt && !ctrl) {		// Alt-p : Toggle Petra mode
					cheat.toggle_Petra();

				} else if (!alt && !ctrl) {	// p : Rerender screen
					gwin->paint();
				}
				break;
		#ifdef MACOS
			case 'q':
				if (mods & KMOD_META) {		// Mac only: Alt-q/Cmd-Q : Quit
					Okay_to_quit();
				}
				break;
		#endif
			case 'r':
				if (ctrl && !alt) {		// Ctrl-r : Restore from 'gamedat'
					quick_restore();
				}
				break;
			case 's':
				if (ctrl && !alt) {		// Ctrl-s : Save to 'gamedat'
					quick_save();

				} else if (alt && !ctrl) {	// Alt-s : Change skin color
					cheat.change_skin();

				} else if (!alt && !ctrl) { 	// s : save/restore gump
					gump_file();
				}
				break;
			case 't':
				if (ctrl && alt) {		// Ctrl-Alt-t : map teleport
					cheat.map_teleport();

				} else if (ctrl && !alt) {	// Ctrl-t :  Fake next time change.
					cheat.fake_time_period();

				} else if (alt && !ctrl) { 	// Alt-t : Teleport to cursor
					cheat.cursor_teleport();

				} else if (!alt && !ctrl) {	// t : Target mode.
					target_mode();
				}
				break;
			case 'v':
				if(!ctrl && !alt) {
					show_about();
				}
				break;
			case 'w':
				if (alt && !ctrl) {  		// Alt-w : toggle archwizard mode
					cheat.toggle_wizard();

				} else if (!alt && !ctrl) {	// w : Activate watch.
					gwin->activate_item(159);	// ++++Blackgate.
				}
				break;
			case 'x':
				if (alt && !ctrl) {		// Alt-x : quit
					Okay_to_quit();
				}
				break;
			case 'z':
				if (!alt && !ctrl) { 		// z : Show stats
					gump_next_stats();
				}
				break;
			}
		}
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

		mouse->hide();		// Turn off mouse.
		mouse_update = false;

		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == 1)
					{
					x = event.button.x >> scale;
					y = event.button.y >> scale;
					return (1);
					}
				break;
			case SDL_MOUSEMOTION:
				mouse->move(event.motion.x >> scale, 
						event.motion.y >> scale);
				mouse_update = true;
				break;
			case SDL_KEYDOWN:
				{
				int c = event.key.keysym.sym;
				if (c == SDLK_ESCAPE)
					return (0);
				if (chr)// Looking for a character?
					{
					*chr = (event.key.keysym.mod & 
							KMOD_SHIFT)
						? toupper(c) : c;
					return (1);
					}
				break;
				}
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
					{
				case SDLK_RALT:		// Right alt.
				case SDLK_RMETA:
					altkeys &= ~1;	// Clear flag.
					break;
				case SDLK_LALT:
				case SDLK_LMETA:
					altkeys &= ~2;
					break;
				default:
					break;
					}
				break;
				}
		mouse->show();		// Turn on mouse.

		if (!gwin->show() &&	// Blit to screen if necessary.
		    mouse_update)
			mouse->blit_dirty();
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
	Mouse::Mouse_shapes saveshape = mouse->get_shape();
	if (shape != Mouse::dontchange)
		mouse->set_shape(shape);
	mouse->show();
	gwin->show(1);			// Want to see new mouse.
	int ret = Get_click(x, y, chr);
	mouse->set_shape(saveshape);
	return (ret);
	}

/*
 *	Wait for someone to stop walking.
 */

void Wait_for_arrival
	(
	Actor *actor			// Whom to wait for.
	)
	{
	unsigned char os = mouse->is_onscreen();
	unsigned long last_repaint = 0;		// For insuring animation repaints.
	while (actor->is_moving())
		{
		Delay();		// Wait a fraction of a second.

		mouse->hide();		// Turn off mouse.
		mouse_update = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEMOTION:
				mouse->move(event.motion.x >> scale,
						 event.motion.y >> scale);
				mouse_update = true;
				break;
				}
					// Get current time, & animate.
		unsigned long ticks = SDL_GetTicks();
		if (gwin->have_focus() && !dragging)
			gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/10 sec.
		if (ticks > last_repaint + 100)
			{
			gwin->paint_dirty();
			last_repaint = ticks;
			}

		mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    mouse_update)	// If not, did mouse change?
			mouse->blit_dirty();
		}

	if (!os)
		mouse->hide();

	}

/*
 *	Wait for a click.
 *
 *	Output:	0 if user hit ESC.
 */
static int Handle_gump_event
	(
	Modal_gump_object *gump,
	SDL_Event& event
	)
	{
	switch (event.type)
		{
	case SDL_MOUSEBUTTONDOWN:
cout << "(x,y) rel. to gump is (" << ((event.button.x>>scale) - gump->get_x())
	 << ", " <<	((event.button.y>>scale) - gump->get_y()) << ")"<<endl;
		if (event.button.button == 1)
			gump->mouse_down(event.button.x >> scale, 
						event.button.y >> scale);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 1)
			gump->mouse_up(event.button.x >> scale,
						event.button.y >> scale);
		break;
	case SDL_MOUSEMOTION:
		mouse->move(event.motion.x >> scale, event.motion.y >> scale);
		mouse_update = true;
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			gump->mouse_drag(event.motion.x >> scale,
						event.motion.y >> scale);
		break;
	case SDL_QUIT:
		if (Okay_to_quit())
			return (0);
	case SDL_KEYDOWN:
		{
		if (event.key.keysym.sym == SDLK_ESCAPE)
			return (0);
		int chr = event.key.keysym.sym;
		gump->key_down((event.key.keysym.mod & KMOD_SHIFT)
					? toupper(chr) : chr);
		break;
		}
	case SDL_KEYUP:			// Watch for ALT keys released.
		switch (event.key.keysym.sym)
			{
		case SDLK_RALT:		// Right alt.
		case SDLK_RMETA:
			altkeys &= ~1;	// Clear flag.
			break;
		case SDLK_LALT:
		case SDLK_LMETA:
			altkeys &= ~2;
			break;
		default: break;
			}
		break;
		}
	return (1);
	}

/*
 *	Handle a modal gump, like the range slider or the save box, until
 *	the gump self-destructs.
 *
 *	Output:	0 if user hit ESC.
 */

int Modal_gump
	(
	Modal_gump_object *gump,	// What the user interacts with.
	Mouse::Mouse_shapes shape	// Mouse shape to use.
	)
	{
	Mouse::Mouse_shapes saveshape = mouse->get_shape();
	if (shape != Mouse::dontchange)
		mouse->set_shape(shape);
	gwin->show(1);
	int escaped = 0;
					// Get area to repaint when done.
	Rectangle box = gwin->get_gump_rect(gump);
	box.enlarge(2);
	box = gwin->clip_to_win(box);
					// Create buffer to backup background.
	Image_buffer *back = gwin->get_win()->create_buffer(box.w, box.h);
	mouse->hide();			// Turn off mouse.
					// Save background.
	gwin->get_win()->get(back, box.x, box.y);
	gump->paint(gwin);		// Paint gump.
	mouse->show();
	gwin->show();
	do
		{
		Delay();		// Wait a fraction of a second.
		mouse->hide();		// Turn off mouse.
		mouse_update = false;
		SDL_Event event;
		while (!escaped && !gump->is_done() && SDL_PollEvent(&event))
			escaped = !Handle_gump_event(gump, event);
		mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    mouse_update)	// If not, did mouse change?
			mouse->blit_dirty();
		}
	while (!gump->is_done() && !escaped);
	mouse->hide();
					// Restore background.
	gwin->get_win()->put(back, box.x, box.y);
	delete back;
	mouse->set_shape(saveshape);
					// Leave mouse off.
	gwin->show(1);
	return (!escaped);
	}

/*
 *	Prompt for a numeric value using a slider.
 *
 *	Output:	Value, or 0 if user hit ESC.
 */

int Prompt_for_number
	(
	int minval, int maxval,		// Range.
	int step,
	int defval			// Default to start with.
	)
	{
	Slider_gump_object *slider = new Slider_gump_object(minval, maxval,
							step, defval);
	int ok = Modal_gump(slider, Mouse::hand);
	int ret = !ok ? 0 : slider->get_val();
	delete slider;
	return (ret);
	}

/*
 *	look for a key to unlock a door or chest.
 */

static void Try_key
	(
	Game_window *gwin
	)
	{
	int x, y;
	if (!Get_click(x, y, Mouse::greenselect))
		return;
					// Look for obj. in open gump.
	Gump_object *gump = gwin->find_gump(x, y);
	Game_object *obj;
	if (gump)
		obj = gump->find_object(x, y);
	else				// Search rest of world.
		obj = gwin->find_object(x, y);
	if (!obj)
		return;
	int qual = obj->get_quality();	// Key quality should match.
	Actor *party[10];		// Get ->party members.
	int party_cnt = gwin->get_party(&party[0], 1);
	for (int i = 0; i < party_cnt; i++)
		{
		Actor *act = party[i];
		GOVector keys;		// Get keys.
		if (act->get_objects(keys, 641, qual, -359))
			{
			keys[0]->activate(gwin->get_usecode());
			return;
			}
		}
	mouse->flash_shape(Mouse::redx);	// Nothing matched.
	}
	
int get_resolution (void)
{
	return current_res;
}

void set_resolution (int new_res, bool save)
{
	if(new_res>=0 && new_res<num_res) {
		current_res = new_res;
		gwin->resized(res_list[current_res].x,
			res_list[current_res].y,
			res_list[current_res].scale);
		scale = gwin->get_win()->get_scale() == 2 ? 1 : 0;
		if(save) {
			char val[20];
			sprintf(val, "%d", res_list[current_res].x);
			config->set("config/video/width",val,true);
			sprintf(val, "%d", res_list[current_res].y);
			config->set("config/video/height",val,true);
			sprintf(val, "%d", res_list[current_res].scale);
			config->set("config/video/scale",val,true);
		}
	}
}

void decrease_resolution (void) {
	if (!cheat()) return;

	current_res--;
	if(current_res<0)
		current_res = num_res-1;
	set_resolution(current_res,false);
}

void increase_resolution (void) {
	if (!cheat()) return;

	current_res++;
	if(current_res>=num_res)
		current_res = 0;
	set_resolution(current_res,false);
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

bool get_play_intro (void)
{
	std::string yn;
	config->value("config/gameplay/skip_splash", yn, "no");
	return(yn=="no");
}

void set_play_intro (bool play)
{
	config->set("config/gameplay/skip_splash", play?"no":"yes", true);
}

bool get_play_1st_scene (void)
{
	std::string yn;
	config->value("config/gameplay/skip_intro", yn, "no");
	return(yn=="no");
}

void set_play_1st_scene (bool play)
{
	config->set("config/gameplay/skip_intro", play?"no":"yes", true);
}

void toggle_fullscreen (void)
{
	gwin->get_win()->toggle_fullscreen();
	gwin->paint();
}

void quick_restore (void)
{
	try
	{
		gwin->read();
	}
	catch(...)
	{
		gwin->center_text("Restoring game failed!");
		return;
	}
	gwin->center_text("Game restored");
	gwin->paint();
}

void quick_save (void)
{
	try
	{
		gwin->write();
	}
	catch(...)
	{
		gwin->center_text("Saving game failed!");
		return;
	}
	gwin->center_text("Game saved");
}

void toggle_combat (void)
{
	gwin->toggle_combat();
	gwin->paint();
	int mx, my;			// Update mouse.
	SDL_GetMouseState(&mx, &my);
	Set_mouse_and_speed(mx, my);
}

void target_mode (void)
{
	int x, y;
	if (!Get_click(x, y, Mouse::greenselect))
		return;
	gwin->double_clicked(x, y);
	if (gwin->get_mode() == Game_window::gump)
		mouse->set_shape(Mouse::hand);
}

void gump_next_inventory (void) {
	static int inventory_page = -1;

	if (gwin->get_mode() != Game_window::gump)
		inventory_page = -1;
	if(inventory_page<gwin->get_usecode()->get_party_count())
		++inventory_page;
	else
		inventory_page = 0;
	Actor *actor = Get_party_member(inventory_page);
	if (actor)
		actor->activate(gwin->get_usecode());
	if (gwin->get_mode() == Game_window::gump)
		mouse->set_shape(Mouse::hand);
}

void gump_next_stats (void) {
	static int stats_page = -1;

	if (gwin->get_mode() != Game_window::gump)
		stats_page = -1;
	if (stats_page < gwin->get_usecode()->get_party_count())
		++stats_page;
	else
		stats_page = 0;
	Actor *actor = Get_party_member(stats_page);
	if (actor)
		gwin->show_gump(actor, game->get_shape("gumps/statsdisplay"));
	if (gwin->get_mode() == Game_window::gump)
		mouse->set_shape(Mouse::hand);
}

void gump_file (void) {
	File_gump_object *fileio = new File_gump_object();
	Modal_gump(fileio, Mouse::hand);
	delete fileio;
}

void show_about (void) {
	Scroll_gump *scroll;
	scroll = new Scroll_gump();

	scroll->add_text("Exult V"VERSION"\n");
	scroll->add_text("(C) 1999-2000 Exult Team\n\n");
	scroll->add_text("Available under the terms of the ");
	scroll->add_text("GNU General Public License\n\n");
	scroll->add_text("http://exult.sourceforge.net\n");

	scroll->paint(gwin);
	do {
		int x, y;
		Get_click(x,y, Mouse::hand);
	} while (scroll->show_next_page(gwin));
	gwin->paint();
	delete scroll;
}

void show_help (void) {
	Scroll_gump *scroll;
	scroll = new Scroll_gump();

	scroll->add_text("Keyboard commands~");
	scroll->add_text("+/- - Change brightness\n");
	scroll->add_text("c - Combat mode\n");
	scroll->add_text("f - Use food\n");
	scroll->add_text("i - Show inventory\n");
	scroll->add_text("k - Try keys\n");
	scroll->add_text("m - Show map\n");
	scroll->add_text("p - Repaint screen\n");
	scroll->add_text("ctrl-s - Quick Save\n");
	scroll->add_text("ctrl-r - Restore\n");
	scroll->add_text("s - Show save box\n");
	scroll->add_text("v - About box\n");
	scroll->add_text("w - Watch\n");
	scroll->add_text("F4 - Toggle fullscreen\n");
	scroll->add_text("ctrl-h - Cheat Commands\n");

	scroll->paint(gwin);
	do {
		int x, y;
		Get_click(x,y, Mouse::hand);
	} while (scroll->show_next_page(gwin));
	gwin->paint();
	delete scroll;
}

void show_cheat_help (void) {
	Scroll_gump *scroll;
	scroll = new Scroll_gump();

	scroll->add_text("Cheat commands~");
	scroll->add_text("Arrow keys - scroll map\n");
	scroll->add_text("Home - recenter map\n");
	scroll->add_text("alt-+/- - Switch resolution\n");
	scroll->add_text("ctrl-b - Shape Browser\n");
	scroll->add_text("ctrl-c - Create Object\n");
	scroll->add_text("ctrl-d - Delete Object\n");
	scroll->add_text("e - Toggle Egg display\n");
	scroll->add_text("alt-g - Toggle God Mode\n");
	scroll->add_text("g - Change Avatar gender\n");
	scroll->add_text("alt-i - Toggle infravision\n");
	scroll->add_text("ctrl-m - Get 100 gold coins\n");
	scroll->add_text("ctrl-alt-m - Hack-Mover\n");
	scroll->add_text("ctrl-t - Next time period\n");
	scroll->add_text("alt-t  - Teleport\n");
	scroll->add_text("ctrl-alt-t - Map Teleport\n");
	scroll->add_text("alt-w - Archwizard mode~");
	if(Game::get_game_type() == SERPENT_ISLE)
		{
		scroll->add_text("SI-only keys~");
		scroll->add_text("alt-n - Toggle Naked flag\n");
		scroll->add_text("alt-p - Toggle Petra mode\n");
		scroll->add_text("alt-s - Change skin color\n");
		}

	scroll->paint(gwin);
	do {
		int x, y;
		Get_click(x,y, Mouse::hand);
	} while (scroll->show_next_page(gwin));
	gwin->paint();
	delete scroll;
}
