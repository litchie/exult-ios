/*
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
#  include <cctype>
#  include <cstdlib>
#endif

#include "SDL_events.h"
#include "SDL_keyboard.h"

#include "exceptions.h"
#include "exult.h"
#include "gamewin.h"
#include "gump_utils.h"
#include "Modal_gump.h"
#include "Slider_gump.h"
#include "Yesno_gump.h"
#include "Gump_manager.h"

using std::cout;
using std::endl;
using std::toupper;

#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
namespace std {
using ::snprintf;
}
#else
using std::snprintf;
#endif

/*
 *	Verify user wants to quit.
 *
 *	Output:	1 to quit.
 */
int Okay_to_quit()
{
	if (Yesno_gump::ask("Do you really want to quit?"))
		quitting_time = QUIT_TIME_YES;
	return quitting_time;
}

/*
 *	Wait for a click.
 *
 *	Output:	0 if user hit ESC.
 */
static int Handle_gump_event
	(
	Modal_gump *gump,
	SDL_Event& event
	)
{
	int scale_factor = Game_window::get_game_window()->get_fastmouse() ? 1 : Game_window::get_game_window()->get_win()->get_scale();
	static bool rightclick;

	switch (event.type)
	{
	case SDL_MOUSEBUTTONDOWN:
#ifdef DEBUG
cout << "(x,y) rel. to gump is (" << ((event.button.x / scale_factor) - gump->get_x())
	 << ", " <<	((event.button.y / scale_factor) - gump->get_y()) << ")"<<endl;
#endif
		if (event.button.button == 1)
			gump->mouse_down(event.button.x / scale_factor, 
						event.button.y / scale_factor);
		else if (event.button.button == 2 && Game_window::get_game_window()->get_mouse3rd()) 
        {
			gump->key_down(SDLK_RETURN);
            gump->text_input(SDLK_RETURN, SDLK_RETURN);
        }
		else if (event.button.button == 3)
			rightclick = true;
		else if (event.button.button == 4) // mousewheel up
			gump->mousewheel_up();
		else if (event.button.button == 5) // mousewheel down
			gump->mousewheel_down();
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == 1)
			gump->mouse_up(event.button.x / scale_factor,
						event.button.y / scale_factor);
		else if (event.button.button == 3 && Game_window::get_game_window()->get_mouse3rd() && rightclick) {
			rightclick = false;
			if (Game_window::get_game_window()->get_gump_man()->can_right_click_close()) return 0;
		}
		break;
	case SDL_MOUSEMOTION:
		Mouse::mouse->move(event.motion.x / scale_factor, event.motion.y / scale_factor);
		Mouse::mouse_update = true;
					// Dragging with left button?
		if (event.motion.state & SDL_BUTTON(1))
			gump->mouse_drag(event.motion.x / scale_factor,
						event.motion.y / scale_factor);
		break;
	case SDL_QUIT:
		if (Okay_to_quit())
			return (0);
	case SDL_KEYDOWN:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
				return (0);
			if ((event.key.keysym.sym == SDLK_s) &&
				(event.key.keysym.mod & KMOD_ALT) &&
				(event.key.keysym.mod & KMOD_CTRL)) {
					make_screenshot(true);
					return 1;
			}

			gump->key_down(event.key.keysym.sym);
			gump->text_input(event.key.keysym.sym, event.key.keysym.unicode);
			break;
		}
	}
	return (1);
}

/*
 *	Handle a modal gump, like the range slider or the save box, until
 *	the gump self-destructs.
 *
 *	Output:	0 if user hit ESC.
 */

int Do_Modal_gump
	(
	Modal_gump *gump,	// What the user interacts with.
	Mouse::Mouse_shapes shape	// Mouse shape to use.
	)
{
    SDL_EnableUNICODE(1);

	// maybe make this selective? it's nice for menus, but annoying for sliders
	//	gwin->end_gump_mode();

	Game_window *gwin = Game_window::get_game_window();
	Mouse::Mouse_shapes saveshape = Mouse::mouse->get_shape();
	if (shape != Mouse::dontchange)
		Mouse::mouse->set_shape(shape);
	gwin->show(true);
	int escaped = 0;
					// Get area to repaint when done.
	Rectangle box = gump->get_rect();
	box.enlarge(2);
	box = gwin->clip_to_win(box);
					// Create buffer to backup background.
	Image_buffer *back = gwin->get_win()->create_buffer(box.w, box.h);
	Mouse::mouse->hide();			// Turn off mouse.
					// Save background.
	gwin->get_win()->get(back, box.x, box.y);
	gump->paint(gwin);		// Paint gump.
	Mouse::mouse->show();
	gwin->show();
	do
	{
		Delay();		// Wait a fraction of a second.
		Mouse::mouse->hide();		// Turn off mouse.
		Mouse::mouse_update = false;
		SDL_Event event;
		while (!escaped && !gump->is_done() && SDL_PollEvent(&event))
			escaped = !Handle_gump_event(gump, event);
		Mouse::mouse->show();		// Re-display mouse.
		if (!gwin->show() &&	// Blit to screen if necessary.
		    Mouse::mouse_update)	// If not, did mouse change?
			Mouse::mouse->blit_dirty();
	}
	while (!gump->is_done() && !escaped);
	Mouse::mouse->hide();
					// Restore background, if wanted.
	if (gump->want_restore_background())
		gwin->get_win()->put(back, box.x, box.y);
	delete back;
	Mouse::mouse->set_shape(saveshape);
					// Leave mouse off.
	gwin->show(true);

    SDL_EnableUNICODE(0);

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
	Slider_gump *slider = new Slider_gump(minval, maxval,
							step, defval);
	int ok = Do_Modal_gump(slider, Mouse::hand);
	int ret = !ok ? 0 : slider->get_val();
	delete slider;
	return (ret);
}


/*
 *	Show a number.
 */

void Paint_num
	(
	Game_window *gwin,
	int num,
	int x,				// Coord. of right edge of #.
	int y				// Coord. of top of #.
	)
{
	const int font = 2;
	char buf[20];
  	snprintf(buf, 20, "%d", num);
	gwin->paint_text(font, buf, x - gwin->get_text_width(font, buf), y);
}
