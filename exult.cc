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

#include "gamewin.h"
#include "usecode.h"
#include "fnames.h"
#include "Audio.h"
#include "Configuration.h"
#include "mouse.h"
#include "gumps.h"
#include "effects.h"
#include "args.h"


Audio *audio;
Configuration *config;

#define MOUSE 1

/*
 *	Globals:
 */
Game_window *gwin = 0;
unsigned char quitting_time = 0;	// Time to quit.
Mouse *mouse = 0;
int scale = 0;				// 1 if scaling X2.
bool	usecode_trace=false;		// Do we trace Usecode-intrinsics?
#if USECODE_DEBUGGER
bool	usecode_debugging=false;	// Do we enable the usecode debugger?
#endif

/*
 *	Local functions:
 */
static void Init();
static int Play();
static void Handle_keystroke(SDLKey ch, int shift, int alt, int ctrl);
int Get_click(int& x, int& y, Mouse::Mouse_shapes shape, char *chr = 0);

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

	cout << "Exult V0." << RELNUM << "." << endl <<
	    "Copyright (C) 2000 J. S. Freedman, Dancer Vesperman, " << endl <<
	    "                   Willem Jan Palenstijn, Tristan Tarrant"<< endl;
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

#if USECODE_DEBUGGER
	string	u_debugging;
	config->value("config/debug/debugger/enable",u_debugging,"no");
	if(u_debugging=="yes")
		usecode_debugging=true;	// Enable usecode debugger
#endif

	Init();				// Create main window.
	
	mouse = new Mouse(gwin);
	mouse->set_shape(Mouse::hand);
#ifdef MOUSE
	SDL_ShowCursor(0);
#endif
	
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
	audio->Init(9615*2,2);
	SDL_SysWMinfo info;		// Get system info.
	SDL_VERSION(&info.version);
					// Ignore clicks until splash done.
	SDL_SetEventFilter(Filter_splash_events);
	int w, h;
#ifdef XWIN
	SDL_GetWMInfo(&info);
	display = info.info.x11.display;
	xfd = ConnectionNumber(display);
	int screen_num = DefaultScreen(display);
	unsigned int display_width = DisplayWidth(display, screen_num);
	unsigned int display_height = DisplayHeight(display, screen_num);
					// Make window 1/2 size of screen.
	w = display_width/2, h = display_height/2;
	if (w < 500)
		w = 500;
	if (h < 400)
		h = 400;
#else
	w = 640;
	h = 480;
#endif //not XWIN

	int sw, sh;			// Get screen size.
	config->value("config/video/width", sw, w);
	config->value("config/video/height", sh, h);
	gwin = new Game_window(sw, sh);

#ifdef WIN32
	//enable unknown (to SDL) window messages, including MM_MCINOTIFY
	//(for MIDI repeats)
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
#endif //WIN32

	string yn;			// Skip intro. scene?
	config->value("config/gameplay/skip_intro", yn, "no");
	if (yn == "yes")
		gwin->get_usecode()->set_global_flag(
			Usecode_machine::did_first_scene, 1);
					// Have Trinsic password?
	config->value("config/gameplay/have_trinsic_password", yn, "no");
	if (yn == "yes")
		gwin->get_usecode()->set_global_flag(
			Usecode_machine::have_trinsic_password, 1);
	}

/*
 *	Play game.
 */

static int Play()
	{
	Handle_events(&quitting_time);
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
		return 0;
		}
	return (1);
	}

/*
 *	Filter out events during the intro. sequence.
 */
static int Filter_intro_events
	(
	const SDL_Event *event
	)
	{
	gwin->end_splash();		// This will start the scene.
	if (gwin->get_mode() == Game_window::conversation)
		{
		SDL_SetEventFilter(0);	// Intro. conversation started.
		return 1;
		}
	if (gwin->get_usecode()->get_global_flag(
					Usecode_machine::did_first_scene))
		{
		SDL_SetEventFilter(0);	// Intro. is done.
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
 *	Statics used below:
 */
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
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
		mouse_update = 0;
#endif
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
			if (ticks > last_repaint + 50)
				{
				gwin->paint_dirty();
				last_repaint = ticks;
				rotate = 1;
				int x, y;// Check for 'stuck' Avatar.
				if (!gwin->get_main_actor()->is_moving())
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
#ifdef MOUSE
		mouse->show();		// Re-display mouse.
#endif
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
			gwin->start_actor(event.button.x >> scale, 
				event.button.y >> scale, avatar_speed);
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
#ifdef MOUSE
		mouse->move(event.motion.x >> scale, event.motion.y >> scale);
		if (gwin->get_mode() == Game_window::normal)
			{
			int ax, ay;	// Get Avatar screen location.
			gwin->get_shape_location(gwin->get_main_actor(), 
								ax, ay);
			int dy = ay - (event.motion.y >> scale), 
			    dx = (event.motion.x >> scale) - ax;
			Direction dir = Get_direction(dy, dx);
			int dist = dy*dy + dx*dx;
			if (dist < 48*48)
				{
				if(gwin->in_combat())
					mouse->set_short_combat_arrow(dir);
				else
					mouse->set_short_arrow(dir);
				avatar_speed = slow_speed;
				}
			else if (dist < 180*180)
				{
				if(gwin->in_combat())
					mouse->set_medium_combat_arrow(dir);
				else
					mouse->set_medium_arrow(dir);
				avatar_speed = medium_speed;
				}
			else
				{
				// No long arrow in combat: use medium
				if(gwin->in_combat())
					mouse->set_medium_combat_arrow(dir);
				else
					mouse->set_long_arrow(dir);
				avatar_speed = fast_speed;
				}
			}
		mouse_update = 1;	// Need to blit mouse.
#endif
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
		if (event.motion.state & SDL_BUTTON(3))
			gwin->start_actor(event.motion.x >> scale, 
					event.motion.y >> scale, avatar_speed);
		break;
		}
	case SDL_ACTIVEEVENT:
					// Get scale factor for mouse.
		scale = gwin->get_win()->get_scale() == 2 ? 1 : 0;
#ifdef MOUSE
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
#endif
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
		quitting_time = 1;
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
#ifdef WIN32
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
 *	Display a shape with info
 *
 */
static void shape_showcase(
	int current_file, 
	int current_shape,
	int current_frame
	)
	{
	char buf[256];
	gwin->paint();
	// First of all draw the shape
	Vga_file *shape_file = gwin->get_shape_file_data(current_file);
	Shape_frame *frame = shape_file->get_shape(
						current_shape, current_frame);
	if (frame)
		gwin->paint_shape(
			gwin->get_width()/2, gwin->get_height()/2, frame);
	// Then show some info about it
	sprintf(buf, "Shape file: \"%s\"", gwin->get_shape_file_name(current_file));
	gwin->paint_text_box(2, buf, 
		100, 100,  gwin->get_width(), gwin->get_height());
	sprintf(buf, "Shape %d/%d - Frame %d/%d", 
		current_shape, shape_file->get_num_shapes()-1,
		current_frame, shape_file->get_num_frames(current_shape)-1);
	gwin->paint_text_box(2, buf, 
		100, 300,  gwin->get_width(), gwin->get_height());
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
	static int current_shape = 0, current_frame = 0, current_file = 0;
	Vga_file *vga_file;
	
	static int inventory_page = -1;
	int stepping = 1;
	
	if(alt)
		stepping = 10;	// If the user is pressing alt, use a
				// larger stepping when cycling shapes
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
		gwin->brighten(20);
		break;
	case SDLK_MINUS:		// Darken.
	case SDLK_KP_MINUS:
		gwin->brighten(-20);
		break;
	case SDLK_b:
		Breakpoint();
		break;
	case SDLK_i:
		// Show Inventory
		if (gwin->get_mode() != Game_window::gump)
			inventory_page = -1;
		if(inventory_page<gwin->get_usecode()->get_party_count()) {
			++inventory_page;
			int npc_num = 0; // Default to Avatar
			if(inventory_page>0)
				npc_num = gwin->get_usecode()->
					get_party_member(inventory_page-1);
			Actor *actor = gwin->get_npc(npc_num);
			actor->activate(gwin->get_usecode());
			if (gwin->get_mode() == Game_window::gump)
				mouse->set_shape(Mouse::hand);
		}
		break;
	case SDLK_c:
		gwin->toggle_combat();	// Go into combat mode
		gwin->paint();
		break;
	case SDLK_h:
		{
		char buf[512];
		sprintf(buf, "EXULT - Keyboard commands\n\n"
			"Arrow keys - scroll map\n"
			"Plus-Minus - Increment-decrement brightness\n"
			"e - Toggle eggs visibility\n"
			"PgUp/PgDn - Show next-previous shape file\n"
			"sS - Show next-previous shape\n"
			"fF - Show next-previous frame\n"
			"i - Show inventory\n"
			"l - Decrement lift\n"
			"m - Change mouse shape\n"
			"p - Repaint screen\n"
			"q - Exit\n"
			"ctrl-t - Fake time period change\n"
		);
			
		gwin->paint_text_box(2, buf, 
			15, 15, 300, 400);
		break;
		}
	case SDLK_q:
		quitting_time = 1;
		break;
	case SDLK_ESCAPE:		// ESC key.
		inventory_page = -1;
		if (gwin->get_mode() == Game_window::gump)
			gwin->end_gump_mode();
		else			// For now, quit.
			if (Yesno_gump_object::ask(
					"Do you really want to quit?"))
				quitting_time = 1;
		break;
	case SDLK_m:			// Show next mouse cursor.
		{
#ifdef MOUSE
		static int mcur = 0;
		mouse->set_shape(++mcur);
		gwin->set_painted();
#if DEBUG
		cout << "Mouse cursor:  " << mcur << endl;
#endif
#endif
		break;
		}
	case SDLK_l:			// Decrement skip_lift.
		if (gwin->skip_lift == 16)
			gwin->skip_lift = 11;
		else
			gwin->skip_lift--;
		if (gwin->skip_lift == 0)
			gwin->skip_lift = 16;
#if DEBUG
		cout << "Skip_lift = " << gwin->skip_lift << endl;
#endif
					// FALL THROUGH.
	case SDLK_p:			// Rerender image.
		gwin->paint();
		break;
	case SDLK_e:
		gwin->paint_eggs = 1-gwin->paint_eggs;
		gwin->paint();
		break;
	case SDLK_f:		// Show next frame
		vga_file = gwin->get_shape_file_data(current_file);
		if (!shift) {
			current_frame += stepping;
			if(current_frame>=vga_file->get_num_frames(current_shape))
				current_frame = 0;
		} else {
			current_frame -= stepping;
			if(current_frame<0)
				current_frame = vga_file->get_num_frames(current_shape);
		}
		shape_showcase(current_file, current_shape, current_frame);
		break;
	case SDLK_r:
		if (ctrl)		// Restore from 'gamedat'.
			{
			if (gwin->read())
				cout << "Restore from 'gamedat' successful"<<endl;
			gwin->paint();
			break;
			}
		break;
	case SDLK_s:
		if (ctrl)		// Save to 'gamedat'.
			{
			if (gwin->write())
				cout << "Save to 'gamedat' successful"<<endl;
			break;
			}
					// Show next shape.
		current_frame = 0;
		vga_file = gwin->get_shape_file_data(current_file);
		if (!shift) {
			current_shape += alt ? 20 : 1;
			if(current_shape>=vga_file->get_num_shapes())
				current_shape = 0;
		} else {
			current_shape -= alt ? 20 : 1;
			if(current_shape<0)
				current_shape = vga_file->get_num_shapes()-1;
		}
		shape_showcase(current_file, current_shape, current_frame);
		break;
	case SDLK_t:			// 'Target' mode.
		{
		if (ctrl)		// 'T':  Fake next time change.
			{
			gwin->fake_next_period();
			break;
			}
		int x, y;
		if (!Get_click(x, y, Mouse::greenselect))
			break;
		gwin->double_clicked(x, y);
		if (gwin->get_mode() == Game_window::gump)
			mouse->set_shape(Mouse::hand);
		break;
		}
	case SDLK_w:			// Test weather.
		if (ctrl)		// Duration is 4*number secs.
			{
			static int wcnt = 0, wmax = 1;
			if (wcnt == 0)
				gwin->add_effect(new Clouds_effect(15));
			else if (wcnt == 1)
				gwin->add_effect(
						new Lightning_effect(2));
			wcnt = (wcnt + 1)%wmax;
			}
		break;
	case SDLK_x:			// Alt-x means quit.
		if (alt && Yesno_gump_object::ask(
						"Do you really want to quit?"))
			quitting_time = 1;
		break;
	case SDLK_PAGEUP:
		current_shape=0;
		current_frame=0;
		--current_file;
		if(current_file<0)
			current_file = gwin->get_shape_file_count()-1;
		shape_showcase(current_file, current_shape, current_frame);
		break;
	case SDLK_PAGEDOWN:
		current_shape=0;
		current_frame=0;
		++current_file;
		if(current_file==gwin->get_shape_file_count())
			current_file = 0;
		shape_showcase(current_file, current_shape, current_frame);
		break;
	case SDLK_RIGHT:
		for (int i = 16; i; i--)
			gwin->view_right();
		break;
	case SDLK_LEFT:
		for (int i = 16; i; i--)
			gwin->view_left();
		break;
	case SDLK_DOWN:
		for (int i = 16; i; i--)
			gwin->view_down();
		break;
	case SDLK_UP:
		for (int i = 16; i; i--)
			gwin->view_up();
		break;
	case SDLK_F4:
		gwin->get_win()->toggle_fullscreen();
		gwin->paint();
		break;
	case SDLK_F10:
		gwin->end_game();
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
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
		mouse_update = 0;
#endif
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
#ifdef MOUSE
				mouse->move(event.motion.x >> scale, 
						event.motion.y >> scale);
				mouse_update = 1;
#endif
				break;
			case SDL_QUIT:
				quitting_time = 1;
				return (0);
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
#ifdef MOUSE
		mouse->show();		// Turn on mouse.
#endif
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
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
		mouse_update = 0;
#endif
		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEMOTION:
#ifdef MOUSE
				mouse->move(event.motion.x >> scale,
						 event.motion.y >> scale);
				mouse_update = 1;
#endif
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
#ifdef MOUSE
		mouse->show();		// Re-display mouse.
#endif
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
#ifdef MOUSE
		mouse->move(event.motion.x >> scale, event.motion.y >> scale);
		mouse_update = 1;
#endif
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			gump->mouse_drag(event.motion.x >> scale,
						event.motion.y >> scale);
		break;
	case SDL_QUIT:
		quitting_time = 1;
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
#ifdef MOUSE
	mouse->hide();			// Turn off mouse.
#endif
					// Save background.
	gwin->get_win()->get(back, box.x, box.y);
	gump->paint(gwin);		// Paint gump.
	mouse->show();
	gwin->show();
	do
		{
		Delay();		// Wait a fraction of a second.
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
		mouse_update = 0;
#endif
		SDL_Event event;
		while (!escaped && !gump->is_done() && SDL_PollEvent(&event))
			escaped = !Handle_gump_event(gump, event);
#ifdef MOUSE
		mouse->show();		// Re-display mouse.
#endif
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

