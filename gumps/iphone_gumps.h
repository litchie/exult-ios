/*
Copyright (C) 2001 The Exult Team

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

#ifndef _IPHONE_GUMPS_H
#define _IPHONE_GUMPS_H

#ifdef __IPHONEOS__

#include "SDL.h"
#include "gamewin.h"

extern "C" int SDLCALL SDL_iPhoneKeyboardShow(SDL_Window * window);
extern "C" int SDLCALL SDL_iPhoneKeyboardHide(SDL_Window * window);
extern "C" SDL_bool SDLCALL SDL_iPhoneKeyboardIsShown(SDL_Window * window);
extern "C" int SDLCALL SDL_iPhoneKeyboardToggle(SDL_Window * window);

class KeyboardButton_gump
{
 public:
	KeyboardButton_gump(int placex = 0, int placey = 0);
	~KeyboardButton_gump();
	int handle_event(SDL_Event *event);
	void paint();
	bool autopaint;

private:
	Vga_file iphone_vga;
	void mouse_down(int mx, int my);
	void mouse_up(int mx, int my);
	
 	int locx;
	int locy;
	int width;
	int height;
};

#endif
#endif
