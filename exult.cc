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

#ifdef MACOS
#include <Events.h>
#endif


#include "gamewin.h"
#include "actors.h"
#include "usecode.h"
#include "fnames.h"
#include "Audio.h"
#include "Configuration.h"
#include "mouse.h"
#include "gumps.h"
#include "effects.h"
#include "args.h"
#include "game.h"
#include "browser.h"
#include "barge.h"

Audio *audio;
Configuration *config;

/*
 *	Globals:
 */
Game_window *gwin = 0;
unsigned char quitting_time = 0;	// Time to quit.
Mouse *mouse = 0;
ShapeBrowser *browser;
int scale = 0;				// 1 if scaling X2.
bool    cheat=true;			// Enable cheating keys
bool	god_mode = false;
bool    wizard_mode = false;
bool	usecode_trace=false;		// Do we trace Usecode-intrinsics?
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
static void Init();
static int Play();
static void Handle_keystroke(SDLKey ch, int shift, int alt, int ctrl);
int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);
int Modal_gump(Modal_gump_object *, Mouse::Mouse_shapes);

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

	// Declare everything from the commandline that we're interested in.
        parameters.declare("-h",&needhelp,true);
        parameters.declare("--help",&needhelp,true);
        parameters.declare("/?",&needhelp,true);
        parameters.declare("/h",&needhelp,true);
        parameters.declare("-game",&gamename,"default");

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

	cout << "Exult V" << VERSION << "." << endl <<
	    "Copyright (C) 2000 J. S. Freedman, Dancer Vesperman, " << endl <<
	    "                   Willem Jan Palenstijn, Tristan Tarrant, " << endl <<
	    "                   Max Horn, Luke Dunstan, Ryan Nunn" << endl;
	cout << "Low level graphics use the 'SDL' library."<< endl;

        config = new Configuration;	// Create configuration object
	config->read_config_file(USER_CONFIGURATION_FILE);
	audio = new Audio;

	{
	// Select the data directory
	string	data_directory;
	vector<string> vs=config->listkeys("config/disk/game",false);
	if(vs.size()==0)
		{
#if DEBUG
		cerr << "No game keys. Converting..." << endl;
#endif
		// Convert from the older format
		config->value("config/disk/u7path",data_directory,".");
		config->set("config/disk/game/blackgate/path",data_directory,true);
		string	s("blackgate");
		config->set("config/disk/game/blackgate/title",s,true);
		vs.push_back(s);
		}
	if(gamename=="default")
		{
		// If the user didn't specify, start up the first game we can find.
		gamename=vs[0];
#if DEBUG
		cerr << "Setting default game to " << gamename << endl;
#endif
		}
	string d("config/disk/game/"),gametitle;
	d+=gamename;
	d+="/path";
	config->value(d.c_str(),data_directory,".");
	if(data_directory==".")
		config->set("config/disk/game/blackgate/path",data_directory,true);
	cout << "chdir to " << data_directory << endl;
	chdir(data_directory.c_str());
	d="config/disk/game/";
	d+=gamename;
	d+="/title";
	config->value(d.c_str(),data_directory,"(unnamed)");
	cout << "Loading game: " << data_directory << endl;
	}

	string	tracing;
	config->value("config/debug/trace/intrinsics",tracing,"no");
	if(tracing=="yes")
		usecode_trace=true;	// Enable tracing of intrinsics

	string  cheating;
	config->value("config/gameplay/cheat",cheating,"yes");
	if(cheating=="no")
		cheat = false;

#if USECODE_DEBUGGER
	string	u_debugging;
	config->value("config/debug/debugger/enable",u_debugging,"no");
	if(u_debugging=="yes")
		usecode_debugging=true;	// Enable usecode debugger
	initialise_usecode_debugger();
#endif

	Init();				// Create main window.
	
	mouse = new Mouse(gwin);
	mouse->set_shape(Mouse::hand);
	
	int result = Play();		// start game
//	delete config;			// free configuration object
	return result;
	}

static int Filter_intro_events(const SDL_Event *event);
static int Filter_splash_events(const SDL_Event *event);
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
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|
						SDL_INIT_AUDIO) < 0)
		{
		cerr << "Unable to initialize SDL: " << SDL_GetError() << endl;
		exit(-1);
		}
	atexit(SDL_Quit);
	
	SDL_SysWMinfo info;		// Get system info.
	SDL_ShowCursor(0);
	SDL_VERSION(&info.version);
					// Ignore clicks until splash done.

#ifdef XWIN
        SDL_GetWMInfo(&info);
        display = info.info.x11.display;
        xfd = ConnectionNumber(display);
#endif
	
	int w, h, sc;

	w = 320;
	h = 200;
	sc = 2;

	int sw, sh, scaleval;
	config->value("config/video/width", sw, w);
	config->value("config/video/height", sh, h);
	config->value("config/video/scale", scaleval, sc);
	gwin = new Game_window(sw, sh, scaleval);

	if (Game::get_game_type() == BLACK_GATE)
		audio->Init(9615*2,2);
	else if (Game::get_game_type() == SERPENT_ISLE)
		audio->Init(11111*2,2);
	else
		audio->Init(11025*2,2);

#ifdef WIN32
	//enable unknown (to SDL) window messages, including MM_MCINOTIFY
	//(for MIDI repeats)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif //WIN32

	string yn;
					// Skip splash screen?
	config->value("config/gameplay/skip_splash", yn, "no");
	if(yn == "no") {
		gwin->set_mode(Game_window::splash);
		Game::get_game()->play_intro();
	}
	Game::get_game()->show_menu();
	gwin->set_mode(Game_window::normal);
	SDL_SetEventFilter(Filter_intro_events);
	browser = new ShapeBrowser();
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
	Handle_events(&quitting_time);
	delete browser;
	delete gwin;
	delete mouse;
	delete audio;
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
 *	Filter out events during the splash screen.
 */
static int Filter_splash_events
	(
	const SDL_Event *event
	)
	{
	switch (event->type)
		{
	case SDL_MOUSEBUTTONUP:
	case SDL_KEYUP:
		return 0;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_KEYDOWN:
					// Now handle intro. scene.
		SDL_SetEventFilter(Filter_intro_events);
		gwin->setup_game();		// This will start the scene.
		return 0;
		}
	return (1);
	}

/*
 *	Statics used below:
 */
static int show_mouse = 0;		// 1 to display mouse in main loop.
static int dragging = 0;		// Object or gump being moved.
static int dragged = 0;			// Flag for when obj. moved.
static int mouse_update = 0;		// Mouse moved/changed.
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
		show_mouse = 1;
		return 1;
		}
	if (gwin->get_usecode()->get_global_flag(
					Usecode_machine::did_first_scene))
		{
		SDL_SetEventFilter(0);	// Intro. is done.
		show_mouse = 1;
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
		mouse_update = 0;

		SDL_Event event;
		while (!*stop && SDL_PollEvent(&event))
			Handle_event(event);
					// Get current time.
		unsigned long ticks = SDL_GetTicks();
					// Animate unless dormant.
		if(gwin->get_mode()!=Game_window::splash)
			{
			if (gwin->have_focus() && !dragging)
				gwin->get_tqueue()->activate(ticks);
					// Show animation every 1/20 sec.
			if (ticks > last_repaint + 50 ||
					// This avoids jumpy walking:
			    gwin->was_painted())
				{
				gwin->paint_dirty();
				last_repaint = ticks;
				rotate = 1;
				int x, y;// Check for 'stuck' Avatar.
				if (!gwin->is_moving() &&
				    !gwin->was_teleported())
#if 0	/* Seems to work fine as above. */
				if ((gwin->get_moving_barge() && 
					!gwin->is_moving()) || 
						!gwin->get_moving_barge())
#endif
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
			dragged = 0;
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
			if (dragging)
				gwin->drop_dragged(event.button.x >> scale, 
					event.button.y >> scale, dragged);
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500)
				{
				dragging = 0;
				gwin->double_clicked(event.button.x >> scale, 
						event.button.y >> scale);
				if (gwin->get_mode() == Game_window::gump)
					mouse->set_shape(Mouse::hand);
				break;
				}
			last_b1_click = curtime;
			if (!dragged)
					// Identify item(s) clicked on.
				gwin->show_items(event.button.x >> scale, 
						event.button.y >> scale);
			dragging = 0;
			}
		break;
	case SDL_MOUSEMOTION:
		{
		mouse->move(event.motion.x >> scale, event.motion.y >> scale);
		if (gwin->get_mode() == Game_window::normal)
			Set_mouse_and_speed(event.motion.x, event.motion.y);
		mouse_update = 1;	// Need to blit mouse.
		if (gwin->get_mode() != Game_window::normal &&
		    gwin->get_mode() != Game_window::gump)
			break;
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			{
			gwin->drag(event.motion.x >> scale, 
						event.motion.y >> scale);
			dragged = 1;
			}
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
			event.key.keysym.mod & KMOD_CTRL);
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
			((Windows_MCI*)(audio->get_midi()))->callback(event.syswm.msg->wParam, 
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
	int ctrl
	)
	{
	static int inventory_page = -1, stats_page = -1;

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
	case SDLK_PLUS:			// Brighten.
	case SDLK_KP_PLUS:
		if(cheat&&alt) {
			current_res++;
			if(current_res>=num_res)
				current_res = 0;
			gwin->resized(res_list[current_res].x,
					res_list[current_res].y,
					res_list[current_res].scale);
		} else
			gwin->brighten(20);
		break;
	case SDLK_MINUS:		// Darken.
	case SDLK_KP_MINUS:
		if(cheat&&alt) {
			current_res--;
			if(current_res<0)
				current_res = num_res-1;
			gwin->resized(res_list[current_res].x,
					res_list[current_res].y,
					res_list[current_res].scale);
		} else
			gwin->brighten(-20);
		break;
	case SDLK_b:
		if(cheat&&ctrl) {
			browser->browse_shapes();
			gwin->paint();
			gwin->set_palette(-1,-1);
		} else			// Open spellbook.
			gwin->activate_item(761);
		break;
	case SDLK_f:			// Feed food.
		gwin->activate_item(377);	// +++++Black gate.
		break;
	case SDLK_i:
		{
		// Show Inventory
		if (gwin->get_mode() != Game_window::gump)
			inventory_page = stats_page = -1;
		if(inventory_page<gwin->get_usecode()->get_party_count())
			++inventory_page;
		else
			inventory_page = 0;
		Actor *actor = Get_party_member(inventory_page);
		if (actor)
			actor->activate(gwin->get_usecode());
		if (gwin->get_mode() == Game_window::gump)
			mouse->set_shape(Mouse::hand);
		break;
		}
	case SDLK_z:			// Show stats.
		{
		if (gwin->get_mode() != Game_window::gump)
			stats_page = -1;
		if (stats_page < gwin->get_usecode()->get_party_count())
			++stats_page;
		else
			stats_page = 0;
		Actor *actor = Get_party_member(stats_page);
		if (actor)
			gwin->show_gump(actor, Game::get_game()->get_shape("gumps/statsdisplay"));
		if (gwin->get_mode() == Game_window::gump)
			mouse->set_shape(Mouse::hand);
		break;
		}
	case SDLK_c:
		{
		if (ctrl)		// Create last shape viewed.
			{
			int current_shape = 0;
			int current_frame = 0;
			if(browser->get_shape(current_shape, current_frame))
				gwin->get_main_actor()->add(
					new Ireg_game_object(current_shape,
					current_frame, 0, 0), 1);
			else
				gwin->center_text("Can only create from 'shapes.vga'");
			break;
			}
		gwin->toggle_combat();	// Go into combat mode
		gwin->paint();
		int mx, my;		// Update mouse.
		SDL_GetMouseState(&mx, &my);
		Set_mouse_and_speed(mx, my);
		break;
		}
	case SDLK_d:			// ctrl-d:  delete what mouse is on.
		{
		if (ctrl&&cheat)
			{
			int x, y;
			SDL_GetMouseState(&x, &y);
			x = x>>scale;
			y = y>>scale;
			Game_object *obj = gwin->find_object(x, y);
			if (obj)
				{
				obj->remove_this();
				gwin->paint();
				}
			}
		break;
		}
	case SDLK_h:	// Help keys, ctrl = cheat keys
		{
		char buf[1024];

		if (!ctrl)
			{
			sprintf(buf, "Keyboard commands\n"
				"  +/- - Higher/Lower brightness\n"
				"  c - Combat mode\n"
				"  i - Show inventory\n"
				"  m - Turn on/off music\n"
				"  p - Repaint screen\n"
				"  ctrl-s/r - Quick Save/Restore\n"
				"  s - Show save box\n"
				"  z - Show stats\n"
				"  F4 - Toggle fullscreen\n"
				"  alt-x, F10, ESC - Exit\n"
				"  ctrl-h - Cheat Commands\n"
			);
			}
		else
			{
			sprintf(buf, "Cheat commands\n"
				"  Arrow keys - scroll map\n"
				"  alt-+/- - Switch resolution\n"
				"  ctrl-b - Shape Browser\n"
				"  ctrl-c - Create Object\n"
				"  ctrl-d - Delete Object\n"
				"  alt-g - Toggle God Mode\n"
				"  g - Change Avatar gender\n"
				"  ctrl-m - Get 100 gold coins\n"
				"  alt-n  - Toggle Naked flag (SI)\n"
				"  alt-p  - Toggle Petra mode (SI)\n"
				"  alt-s - Change skin color (SI)\n"
				"  ctrl-t - Fake time period change\n"
				"  alt-t  - Teleport\n"
				"  ctrl-alt-t - Map Teleport\n"
				"  alt-w - Toggle Archwizard Mode\n"
				"  ctrl-w - Test weather\n"
			);
			}

		gwin->paint_text_box(MAINSHP_FONT1, buf, 
			5, 5, 300, 400);
		gwin->get_win()->show();
		break;
		}
	case SDLK_ESCAPE:		// ESC key.
		inventory_page = stats_page = -1;
		if (gwin->get_mode() == Game_window::gump)
			gwin->end_gump_mode();
		else			// For now, quit.
			Okay_to_quit();
		break;
	case SDLK_m:
		if (ctrl&&cheat) {	// CTRL-m:  get 100 gold coins!
			gwin->get_main_actor()->add_quantity(100, 644);
			gwin->center_text("Added 100 gold coins");
			break;
		} else if (alt) {
			static int mnum = 0;
			if (shift && mnum > 0)
				audio->start_music(--mnum, 0);
			else
				audio->start_music(mnum++, 0);
		} else			// Show map.
			gwin->activate_item(178);	//++++Black gate.
		break;
	case SDLK_l:			// Decrement skip_lift.
		if(!cheat)
			break;
		if (gwin->skip_lift == 16)
			gwin->skip_lift = 11;
		else
			gwin->skip_lift--;
		if (gwin->skip_lift <= 0)
			gwin->skip_lift = 16;
#if DEBUG
		cout << "Skip_lift = " << gwin->skip_lift << endl;
#endif
					// FALL THROUGH.
	case SDLK_p:			// Rerender image.
		// Toggle petra mode
		if(alt && cheat && (Game::get_game_type() != BLACK_GATE))
		{
			if (gwin->get_main_actor()->get_siflag(Actor::petra))
				gwin->get_main_actor()->clear_siflag(Actor::petra);
			else
				gwin->get_main_actor()->set_siflag(Actor::petra);
			gwin->set_all_dirty();
			break;
		}

		gwin->paint();
		break;
	case SDLK_e:
		if(!cheat)
			break;
		gwin->paint_eggs = !gwin->paint_eggs;
		if(gwin->paint_eggs)
			gwin->center_text("Eggs display enabled");
		else
			gwin->center_text("Eggs display disabled");
		gwin->paint();
		break;
	case SDLK_g:
		if (alt) {	// toggle god-mode
			if (cheat) {
				god_mode = !god_mode;
				if (god_mode)
					gwin->center_text("God Mode Enabled");
				else
					gwin->center_text("God Mode Disabled");
			}
		} else {	// Change Avatars gender
			if(!cheat)
				break;
			if (gwin->get_main_actor()->get_type_flag(Actor::tf_sex))
				gwin->get_main_actor()->clear_type_flag(Actor::tf_sex);
			else
				gwin->get_main_actor()->set_type_flag(Actor::tf_sex);
			gwin->set_all_dirty();
		}
		break;
	case SDLK_n:		// Toggle Naked flag
		if(!cheat || (Game::get_game_type() == BLACK_GATE))
			break;
		if (gwin->get_main_actor()->get_siflag(Actor::naked))
			gwin->get_main_actor()->clear_siflag(Actor::naked);
		else
			gwin->get_main_actor()->set_siflag(Actor::naked);
		gwin->set_all_dirty();
		break;
	case SDLK_r:
		if (ctrl)		// Restore from 'gamedat'.
			{
			if (gwin->read())
				gwin->center_text("Game restored");
			gwin->paint();
			break;
			}
		break;
	case SDLK_s:
		if (ctrl)		// Save to 'gamedat'.
			{
			if (gwin->write())
				gwin->center_text("Game saved");
			}
		else if (alt)		// Change skin color
			{
			int color = gwin->get_main_actor()->get_skin_color();

			if (color < 0 || color > 2)
				break;
			color = (color + 4) %3;
			gwin->get_main_actor()->set_skin_color(color);
			gwin->set_all_dirty();
			}
		else
			{
			File_gump_object *fileio = new File_gump_object();
			Modal_gump(fileio, Mouse::hand);
			delete fileio;
			}
		break;
	case SDLK_t:			// 'Target' mode.
		{
		if (ctrl && alt && cheat) {	// map teleport
			// Display map.
			Shape_frame *map = gwin->get_sprite_shape(22, 0);
					// Get coords. for centered view.
			int x = (gwin->get_width() - map->get_width())/2 + map->get_xleft();
			int y = (gwin->get_height() - map->get_height())/2 + map->get_yabove();
			gwin->paint_shape(x, y, map, 1);

			// mark current location
			int tx, ty, z, xx, yy;
			gwin->get_main_actor()->get_abs_tile(tx, ty, z);
	
			//the 5 and 10 below are the map-borders, 3072 dimensions of the world
			//the +1 _seems_ to improve location, maybe something to do with "/ 3072"?
			xx = ((tx * (map->get_width() - 10)) / 3072) + (5 + x - map->get_xleft()) + 1;
			yy = ((ty * (map->get_height() - 10)) / 3072) + (5 + y - map->get_yabove()) + 1;
			gwin->get_win()->fill8(0, 1, 5, xx, yy - 2); // black isn't the correct colour,
			gwin->get_win()->fill8(0, 5, 1, xx - 2, yy); // ++++should be yellow

			gwin->show(1);
			if (!Get_click(xx, yy, Mouse::greenselect)) {
				gwin->paint();
				break;
			}
			
			//the 5 and 10 below are the map-borders, 3072 dimensions of the world
			tx = ((xx - (5 + x - map->get_xleft()))*3072) / (map->get_width() - 10);
			ty = ((yy - (5 + y - map->get_yabove()))*3072) / (map->get_height() - 10);
			cout << "Teleporting to " << tx << "," << ty << "!" << endl;
			Tile_coord t(tx,ty,0);
			gwin->teleport_party(t);
			gwin->center_text("Teleport!!!");
			break;
		}
		if (ctrl&&cheat)		// 'T':  Fake next time change.
			{
			gwin->fake_next_period();
			gwin->center_text("Game clock incremented");
			break;
			}
		int x, y;
		if (alt&&cheat)		// Teleport.
			{
			SDL_GetMouseState(&x, &y);
			x = x>>scale;
			y = y>>scale;
			Tile_coord t(gwin->get_scrolltx() + x/tilesize,
				gwin->get_scrollty() + y/tilesize, 0);
			gwin->teleport_party(t);
			gwin->center_text("Teleport!!!");
			break;
			}
		if (!Get_click(x, y, Mouse::greenselect))
			break;
		gwin->double_clicked(x, y);
		if (gwin->get_mode() == Game_window::gump)
			mouse->set_shape(Mouse::hand);
		break;
		}
	case SDLK_w:			// Test weather.
		if (ctrl&&cheat)		// Duration is 4*number secs.
			{
			static int wcnt = 0, wmax = 1;
			if (wcnt == 0)
				gwin->add_effect(new Clouds_effect(15));
			else if (wcnt == 1)
				gwin->add_effect(
						new Lightning_effect(2));
			wcnt = (wcnt + 1)%wmax;
			}
		else if (alt && cheat) {  // toggle archwizard mode
			wizard_mode = !wizard_mode;
			if (wizard_mode)
				gwin->center_text("Archwizard Mode Enabled");
			else
				gwin->center_text("Archwizard Mode Disabled");
		 } else			// Activate watch.
			gwin->activate_item(159);	// ++++Blackgate.
		break;
	case SDLK_x:			// Alt-x means quit.
		if (alt)
			Okay_to_quit();
		break;
	case SDLK_RIGHT:
		if(!cheat)
			break;
		for (int i = 16; i; i--)
			gwin->view_right();
		break;
	case SDLK_LEFT:
		if(!cheat)
			break;
		for (int i = 16; i; i--)
			gwin->view_left();
		break;
	case SDLK_DOWN:
		if(!cheat)
			break;
		for (int i = 16; i; i--)
			gwin->view_down();
		break;
	case SDLK_UP:
		if(!cheat)
			break;
		for (int i = 16; i; i--)
			gwin->view_up();
		break;
	case SDLK_HOME:
		if(!cheat)
			break;
		gwin->center_view(gwin->get_main_actor()->get_abs_tile_coord());
		gwin->paint();
		break;
	case SDLK_F4:
		gwin->get_win()->toggle_fullscreen();
		gwin->paint();
		break;
	case SDLK_F10:
		if(!cheat)
			break;
		{
			Game::get_game()->end_game(shift==0);
			gwin->set_palette(0);
			gwin->paint();
			gwin->fade_palette (50, 1, 0);
		}
		break;
	case SDLK_F11:
		if(!cheat && Game::get_game_type() != SERPENT_ISLE)
			break;
		{
			Game::get_game()->set_jive();
			Game::get_game()->play_intro();
			Game::get_game()->clear_jive();
			gwin->set_palette(0);
			gwin->paint();
			gwin->fade_palette (50, 1, 0);
		}
		break;
	default:
		break;
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
		mouse_update = 0;

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
				mouse_update = 1;
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
		mouse_update = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEMOTION:
				mouse->move(event.motion.x >> scale,
						 event.motion.y >> scale);
				mouse_update = 1;
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
		mouse_update = 1;
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
		mouse_update = 0;
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

