/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gumps.cc - Open containers and their contents.
 **
 **	Written: 3/4/2000 - JSF
 **/
/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include <cctype>
#include <cstdlib>

#include "SDL.h"

#include "exceptions.h"
#include "gamewin.h"
#include "gump_utils.h"
#include "Modal_gump.h"
#include "Slider_gump.h"
#include "Yesno_gump.h"

using std::cout;
using std::endl;

// FIX ME - mouse_update should be in class Mouse
static bool mouse_update = 0;		// Mouse moved/changed.

extern Game_window *gwin;	// from exult.cc - FIX ME!!!
extern unsigned char quitting_time;	// from exult.cc - FIX ME!!!
extern Mouse *mouse;	// from exult.cc - FIX ME!!!
extern int scale;	// from exult.cc - FIX ME!!!
extern unsigned int altkeys;	// from exult.cc - FIX ME!!!


/*
 *	Verify user wants to quit.
 *
 *	Output:	1 to quit.
 */
int Okay_to_quit()
{
	if (Yesno_gump::ask("Do you really want to quit?"))
		quitting_time = 1;
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
	switch (event.type)
	{
	case SDL_MOUSEBUTTONDOWN:
#if DEBUG
cout << "(x,y) rel. to gump is (" << ((event.button.x>>scale) - gump->get_x())
	 << ", " <<	((event.button.y>>scale) - gump->get_y()) << ")"<<endl;
#endif
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
						? std::toupper(chr) : chr);
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

int Do_Modal_gump
	(
	Modal_gump *gump,	// What the user interacts with.
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
  	sprintf(buf, "%d", num);
	gwin->paint_text(font, buf, x - gwin->get_text_width(font, buf), y);
}

char *newstrdup(const char *s)
{
	if(!s)
		throw std::invalid_argument("NULL pointer passed to newstrdup");
	char *ret=new char[std::strlen(s)+1];
	std::strcpy(ret,s);
	return ret;
}
