/*
Copyright (C) 2001-2011 The Exult Team

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

#ifdef __IPHONEOS__

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"

#include "fnames.h"
#include "iphone_gumps.h"
#include "exult.h"
#include "exult_iphone_flx.h"
#include "keyactions.h"
#include "gamewin.h"
#include "shapeid.h"
#include <string>

using std::string;
using std::cout;
using std::endl;

KeyboardButton_gump::KeyboardButton_gump(int placex, int placey)
{
	autopaint = true;
	iphone_vga.load(IPHONE_FLX);
	width = iphone_vga.get_shape(EXULT_IPHONE_FLX_KEYBOARDBUTTON_SHP, 0)->get_width();
	height = iphone_vga.get_shape(EXULT_IPHONE_FLX_KEYBOARDBUTTON_SHP, 0)->get_height();
	locx = placex;
	locy = placey;
}

KeyboardButton_gump::~KeyboardButton_gump()
{

}


void KeyboardButton_gump::paint()
{
	Game_window *gwin = Game_window::get_instance();
	Shape_manager *sman = Shape_manager::get_instance();
	sman->paint_shape(locx, locy,
 		iphone_vga.get_shape(EXULT_IPHONE_FLX_KEYBOARDBUTTON_SHP, 0), 0);
	gwin->add_dirty(Rectangle(locx, locy, width+4, height+4));
	gwin->set_painted();
}

int KeyboardButton_gump::handle_event(SDL_Event *event)
{
	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
	{
		void (KeyboardButton_gump::*mouseFunc)(int, int) = &KeyboardButton_gump::mouse_down; // Mouse down by default
		if (event->type == SDL_MOUSEBUTTONUP)
			mouseFunc = &KeyboardButton_gump::mouse_up;
		Game_window *gwin = Game_window::get_instance();
		int scale = gwin->get_fastmouse() ? 1 : gwin->get_win()->get_scale_factor();
		int x = event->button.x/scale, y = event->button.y/scale;
		//std::cout << "x,y: " << x << "," << y << " locx,locy: " << locx << "," << locy << " widthXheight: " << width << "X" << height << std::endl;
                if (x >= locx && x <= (locx + width)
                        && y >= locy && y <= (locy + height))
                {
                        (*this.*mouseFunc)(x - locx, y - locy);
                        return 1;
                }
	}
	
	return 0;
}

void KeyboardButton_gump::mouse_down(int mx, int my)
{
	// Find the SDL window...
        SDL_Window *window = NULL;
        unsigned int idWin = -1;
        while (window == NULL)
        {
                idWin++;
                window = SDL_GetWindowFromID(idWin);
        }
        SDL_iPhoneKeyboardToggle(window);
}

void KeyboardButton_gump::mouse_up(int mx, int my)
{

}


#endif
