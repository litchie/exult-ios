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

#include <sys/time.h>
#include <unistd.h>

#include "SDL.h"
#include "SDL_syswm.h"

#include "gamewin.h"
#include "fnames.h"
#include "Audio.h"
#include "Configuration.h"
#include "mouse.h"
#include "gumps.h"
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
bool	usecode_trace=false;	// Do we trace Usecode-intrinsics?

/*
 *	Local functions:
 */
static void Init();
static int Play();
static void Handle_keystroke(SDLKey ch, int shift, int alt, int ctrl);


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

	cout << "Exult V0." << RELNUM << ".\n" <<
	    "Copyright (C) 2000 J. S. Freedman, Dancer Vesperman, \n" <<
	    "                   Willem Jan Palenstijn, Tristan Tarrant\n";
	cout << "Low level graphics use the 'SDL' library.\n";

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

	Init();				// Create main window.
	
#if 0	/* Make this an option. */
	if (argc > 2)			// Specify chunkx, chunky on cmndline.
		gwin->set_chunk_offsets(atoi(argv[1]), atoi(argv[2]));
	else				// Else start in Trinsic.
#endif
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
	timer.tv_usec = 20000;		// Try 1/50 second.
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
		cerr << "Unable to initialize SDL: " << SDL_GetError() << '\n';
		exit(-1);
		}
	atexit(SDL_Quit);
	audio->Init(9615*2,2);
	SDL_SysWMinfo info;		// Get system info.
	SDL_VERSION(&info.version);
					// Ignore clicks until splash done.
	SDL_SetEventFilter(Filter_splash_events);
#ifdef XWIN
	SDL_GetWMInfo(&info);
	display = info.info.x11.display;
	xfd = ConnectionNumber(display);
	int screen_num = DefaultScreen(display);
	unsigned int display_width = DisplayWidth(display, screen_num);
	unsigned int display_height = DisplayHeight(display, screen_num);
					// Make window 1/2 size of screen.
	int w = display_width/2, h = display_height/2;
	if (w < 500)
		w = 500;
	if (h < 400)
		h = 400;
	gwin = new Game_window(w, h);
#else
	gwin = new Game_window(640, 480);
#endif
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

/*
 *	Handle events until a flag is set.
 */

static void Handle_events
	(
	unsigned char *stop
	)
	{
	long last_repaint = 0;		// For insuring animation repaints.
	/*
	 *	Main event loop.
	 */
	while (!*stop)
		{
		int rotate = 0;		// 1 to rotate colors.
		Delay();		// Wait a fraction of a second.
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
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
						// Show animation every 1/10 sec.
			if (ticks > last_repaint + 100)
				{
				gwin->paint_dirty();
				last_repaint = ticks;
				rotate = 1;
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
		gwin->show();		// Blit to screen if necessary.
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
	unsigned int mask;
	char keybuf[10];
//cout << "Event " << (int) event.type << " received\n";
	switch (event.type)
		{
	case SDL_MOUSEBUTTONDOWN:
		if (gwin->get_mode() != Game_window::normal &&
		    gwin->get_mode() != Game_window::gump)
			break;
		if (event.button.button == 1)
			{
#if DEBUG
cout << "Mouse down at (" << event.button.x << ", " <<
	event.button.y << ")\n";
#endif
			dragging = gwin->start_dragging(event.button.x,
							event.button.y);
			dragged = 0;
			}
					// Move sprite toward mouse
					//  when right button pressed.
		if (event.button.button == 3 && 
		    gwin->get_mode() == Game_window::normal)
			gwin->start_actor(event.button.x, event.button.y);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 3)
			{
			if (gwin->get_mode() != Game_window::normal)
				break;
			gwin->stop_actor();
			}
		else if (event.button.button == 1)
			{
			unsigned long curtime = SDL_GetTicks();
			if (dragging)
				gwin->drop_dragged(event.button.x, 
						event.button.y, dragged);
					// Last click within .5 secs?
			if (curtime - last_b1_click < 500)
				{
				dragging = 0;
				gwin->double_clicked(event.button.x, 
							event.button.y);
				if (gwin->get_mode() == Game_window::gump)
					mouse->set_shape(Mouse::hand);
				break;
				}
			last_b1_click = curtime;
			if (!dragged)
					// Identify item(s) clicked on.
				gwin->show_items(event.button.x, 
							event.button.y);
			dragging = 0;
			}
		break;
	case SDL_MOUSEMOTION:
		{
#ifdef MOUSE
		mouse->move(event.motion.x, event.motion.y);
		if (gwin->get_mode() == Game_window::normal)
			{
			int ax, ay;	// Get Avatar screen location.
			gwin->get_shape_location(gwin->get_main_actor(), 
								ax, ay);
			int dy = ay - event.motion.y, dx = event.motion.x - ax;
			Direction dir = Get_direction(dy, dx);
			int dist = dy*dy + dx*dx;
			if (dist < 48*48)
				mouse->set_short_arrow(dir);
			else if (dist < 180*180)
				mouse->set_medium_arrow(dir);
			else
				mouse->set_long_arrow(dir);
			}
		gwin->set_painted();	// We'll need to blit.
#endif
		if (gwin->get_mode() != Game_window::normal &&
		    gwin->get_mode() != Game_window::gump)
			break;
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			{
			gwin->drag(event.motion.x, event.motion.y);
			dragged = 1;
			}
					// Dragging with right?
		if (event.motion.state & SDL_BUTTON(3))
			gwin->start_actor(event.motion.x, event.motion.y);
		break;
		}
	case SDL_ACTIVEEVENT:
#ifdef MOUSE
		if (event.active.state & SDL_APPMOUSEFOCUS)
			{
			if (event.active.gain)
				{
				int x, y;
				SDL_GetMouseState(&x, &y);
				mouse->set_location(x, y);
				}
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
	case SDL_KEYDOWN:			// Keystroke.
		Handle_keystroke(event.key.keysym.sym,
			event.key.keysym.mod & KMOD_SHIFT,
			event.key.keysym.mod & KMOD_ALT,
			event.key.keysym.mod & KMOD_CTRL);
		break;
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
		if(inventory_page<gwin->get_usecode()->get_party_count()) {
			++inventory_page;
			int npc_num = 0; // Default to Avatar
			if(inventory_page>0)
				npc_num = gwin->get_usecode()->get_party_member(inventory_page-1);
			Actor *actor = gwin->get_npc(npc_num);
			actor->activate(gwin->get_usecode());
		}
		break;
	case SDLK_c:
		// Go into combat mode
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
			"t - Fake time period change\n"
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
			quitting_time = 1;
		break;
	case SDLK_m:			// Show next mouse cursor.
		{
#ifdef MOUSE
		static int mcur = 0;
		mouse->set_shape(++mcur);
		gwin->set_painted();
#if DEBUG
		cout << "Mouse cursor:  " << mcur << '\n';
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
		cout << "Skip_lift = " << gwin->skip_lift << '\n';
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
	case SDLK_s:
		if (ctrl)		// Save to 'gamedat'.
			{
			if (gwin->write())
				cout << "Save to 'gamedat' successful\n";
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
	case SDLK_t:			// Fake out time period change.
		gwin->fake_next_period();
		break;
	case SDLK_RIGHT:
		gwin->view_right();
		break;
	case SDLK_LEFT:
		gwin->view_left();
		break;
	case SDLK_DOWN:
		gwin->view_down();
		break;
	case SDLK_UP:
		gwin->view_up();
		break;
	case SDLK_F4:
		gwin->get_win()->toggle_fullscreen();
		gwin->paint();
		break;
	}
}

/*
 *	Wait for a click.
 *
 *	Output:	0 if user hit ESC.
 */
static int Get_click
	(
	int& x, int& y
	)
	{
	while (1)
		{
		SDL_Event event;
		Delay();		// Wait a fraction of a second.
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
#endif
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == 1)
					{
					x = event.button.x;
					y = event.button.y;
					return (1);
					}
				break;
			case SDL_MOUSEMOTION:
#ifdef MOUSE
				mouse->move(event.motion.x, event.motion.y);
				gwin->set_painted();
#endif
				break;
			case SDL_QUIT:
				quitting_time = 1;
				return (0);
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					return (0);
				break;
				}
#ifdef MOUSE
		mouse->show();		// Turn on mouse.
#endif
		gwin->show();		// Blit to screen if necessary.
		}
	return (0);			// Shouldn't get here.
	}

/*
 *	Get a click.
 *
 *	Output:	0 if user hit ESC.
 */

int Get_click
	(
	int& x, int& y,			// Location returned (if not ESC).
	Mouse::Mouse_shapes shape	// Mouse shape to use.
	)
	{
	Mouse::Mouse_shapes saveshape = mouse->get_shape();
	if (shape != Mouse::dontchange)
		mouse->set_shape(shape);
	int ret = Get_click(x, y);
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
	long last_repaint = 0;		// For insuring animation repaints.
	while (actor->is_moving())
		{
		Delay();		// Wait a fraction of a second.
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
#endif
		SDL_Event event;
		while (SDL_PollEvent(&event))
			switch (event.type)
				{
			case SDL_MOUSEMOTION:
#ifdef MOUSE
				mouse->move(event.motion.x, event.motion.y);
				gwin->set_painted();
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
		mouse->show();		// Re-display mouse.
		gwin->show();		// Blit to screen if necessary.
		}
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
		if (event.button.button == 1)
			gump->mouse_down(event.button.x, event.button.y);
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 1)
			gump->mouse_up(event.button.x, 	event.button.y);
		break;
	case SDL_MOUSEMOTION:
#ifdef MOUSE
		mouse->move(event.motion.x, event.motion.y);
		gwin->set_painted();
#endif
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			gump->mouse_drag(event.motion.x, event.motion.y);
		break;
	case SDL_QUIT:
		quitting_time = 1;
		return (0);
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == SDLK_ESCAPE)
			return (0);
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
	int escaped = 0;
					// Get area to repaint when done.
	Rectangle box = gwin->get_gump_rect(gump);
	box.enlarge(6);
#ifdef MOUSE
	mouse->hide();			// Turn off mouse.
#endif
	gump->paint(gwin);		// Paint gump.
	gwin->show();
	do
		{
		Delay();		// Wait a fraction of a second.
#ifdef MOUSE
		mouse->hide();		// Turn off mouse.
#endif
		SDL_Event event;
		while (!escaped && SDL_PollEvent(&event))
			escaped = !Handle_gump_event(gump, event);
#ifdef MOUSE
		mouse->show();		// Re-display mouse.
#endif
		gwin->show();		// Blit to screen if necessary.
		}
	while (!gump->is_done() && !escaped);
	mouse->hide();
	gwin->paint(box);		// Paint over gump.
	mouse->set_shape(saveshape);
	mouse->show();
	return (!escaped);
	}


#if 0
// The old win32 code.
//#ifdef __WIN32

UINT g_nCmdShow;
HINSTANCE g_hInstance;

LONG APIENTRY Handle_event (HWND, UINT, UINT, LONG);

void gXXXXettimeofday(timeval* tv, int x); //in objs.cpp

UINT TimerID;

//main function
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow ) {
	OSVERSIONINFO osVer;
  osVer.dwOSVersionInfoSize = sizeof(osVer);
  if (!GetVersionEx(&osVer)) return false;
  if (osVer.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
    MessageBox(NULL, "This version of Exult requires Window 95 or higher.",
	"Exult", MB_OK );
    return false;
  }

  g_nCmdShow = nCmdShow;
  g_hInstance = hInstance;

  //TO DO: Parse commandline

  Init(); //init window class and create game window

  //init location (Trinsic)
  gwin->set_chunk_offsets(64, 135);

  return (Play());
}

int Play() {
	MSG msg;

	gwin->paint();			// Get backgrounds loaded.
	gwin->init_actors();		// Place actors in world.
	gwin->paint();
	gwin->show();

  HWND hWnd = gwin->get_win()->win;

  TimerID = SetTimer(hWnd, 1, 20, NULL);

  while (GetMessage(&msg, NULL, 0, 0)) { //message loop
	    TranslateMessage(&msg); //necessary?
	    DispatchMessage(&msg);
  }

  KillTimer(hWnd, 1);

  delete gwin;

  return msg.wParam; //end program
}

//create image window instance
HWND InitInstance (int cwidth, int cheight) {
  HWND hWnd;
  RECT ClientRect;
  int width, height;

  ClientRect.left = 50;
  ClientRect.top = 50;
  ClientRect.right = 50 + cwidth;
  ClientRect.bottom = 50 + cheight;

  AdjustWindowRect(&ClientRect, WS_OVERLAPPEDWINDOW, false);
  width = ClientRect.right - ClientRect.left;
  height = ClientRect.bottom - ClientRect.top;

  //TO DO: adjust to screen size

  hWnd = CreateWindow("ExultImgWClass", "Exult", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, g_hInstance, NULL);

  if (!hWnd) return NULL;

  ShowWindow(hWnd, g_nCmdShow);
//  UpdateWindow(hWnd);
  return hWnd;
}

BOOL ShiftDown ( ) {
  return (GetKeyState(VK_SHIFT) & 0x8000000);
}

BOOL CtrlDown () {
  return (GetKeyState(VK_CONTROL) & 0x8000000);
}

void TranslateCoord(HWND hWnd, int &xPos, int &yPos) {
  RECT ClientRect;
  GetClientRect(hWnd, &ClientRect);
  int cwidth = ClientRect.right;
  int cheight = ClientRect.bottom;

  if (cwidth*cheight) {
    xPos = xPos*320/cwidth;  // horizontal position of cursor
    yPos = yPos*200/cheight;  // vertical position of cursor
  }
}

//handle message
LONG APIENTRY Handle_event (HWND hWnd, UINT Message, UINT wParam, LONG lParam) {
  static BOOL doubleclicked = false;
  PAINTSTRUCT PaintInfo;

  int xPos = LOWORD(lParam);
  int yPos = HIWORD(lParam);

  UINT q;
  switch (Message) {
	case WM_DESTROY: //quit
      PostQuitMessage(0);
      break;
    case WM_LBUTTONDOWN: //left mouse pressed
		  gwin->end_intro();
			if (gwin->get_mode() != Game_window::normal)
			break; //not in normal game mode

      if (!CtrlDown()) break;

    case WM_MBUTTONDOWN: //middle mouse or Ctrl-Left mouse (fall-through)
      TranslateCoord(hWnd, xPos, yPos);
			if (ShiftDown()) // Show abs. game loc. (for scripts).
				gwin->show_game_location(xPos, yPos);
			else
				gwin->debug_shape(hWnd, xPos, yPos);
      break;

    case WM_LBUTTONDBLCLK: //left mouse double clicked
      TranslateCoord(hWnd, xPos, yPos);
			if (gwin->get_mode() != Game_window::normal)
			break; //not in normal game mode
			gwin->double_clicked(hWnd, xPos, yPos);
      doubleclicked = true;
      break;

    case WM_RBUTTONDOWN: //right mouse pressed
		  gwin->end_intro();
			gwin->stop_showing_item();
			if (gwin->get_mode() != Game_window::normal)
			break; //not in normal game mode

      TranslateCoord(hWnd, xPos, yPos);

      gwin->start_actor(xPos, yPos);

      SetCapture(hWnd); //capture mouse while holding right button
      break;

    case WM_LBUTTONUP: //left mouse released
      if (doubleclicked) {
	doubleclicked = false;
	break;
      }
      TranslateCoord(hWnd, xPos, yPos);
			if (gwin->get_mode() == Game_window::conversation) {	// Conversation.
				gwin->converse(xPos, yPos);
	break;
      }
      gwin->show_items(xPos, yPos);
	break;

    case WM_RBUTTONUP: //right mouse released
      ReleaseCapture(); //release mouse capture
      gwin->stop_actor();
      break;

    case WM_MOUSEMOVE: //moving mouse
      if (wParam & MK_RBUTTON) //right mouse down?
	gwin->start_actor(xPos, yPos);
      break;

    case WM_TIMER: //timer event
		  struct timeval TVAL;
		  gXXXXettimeofday(&TVAL,0);
		gwin->get_tqueue()->activate(timer);
      gwin->show();
      break;

    case WM_KEYDOWN: //key pressed, ok?
      if (wParam >= 'A' && wParam <= 'Z' && !ShiftDown())
	wParam += 0x20;	 //convert to lowercase
      if ((char)wParam == 'q')
	PostQuitMessage(0);
      else {
/*	  char s[10];
	sprintf(s, "%x", (char)wParam);
	MessageBox(NULL,s,"Key pressed",MB_OK); */

	Handle_keystroke((char)wParam);
      }
      break;

    case WM_PAINT: //paint window
      if (gwin) {
	gwin->get_win(hWnd)->ScreenDC = BeginPaint(hWnd, &PaintInfo);
	gwin->show(hWnd);
	EndPaint(hWnd, &PaintInfo);
	gwin->get_win(hWnd)->ScreenDC = 0;
      }
      break;

    case WM_SIZING: case WM_SIZE:
      if (gwin)
	gwin->show(hWnd);
      break;

    default:
	return (DefWindowProc(hWnd, Message, wParam, lParam));
  }
  return 0;
}

void Init() {
  WNDCLASS wc;

  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = (WNDPROC)Handle_event;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = g_hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = "ExultImgWClass";

  if (!RegisterClass(&wc)) {
    MessageBox(NULL,"Error registering Window class","Exult",MB_OK);
    exit(1);
  }

  gwin = new Game_window(320, 200);
}

#endif //__WIN32
