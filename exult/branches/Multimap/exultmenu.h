/*
 *  Copyright (C) 2001  The Exult Team
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

#ifndef EXULTMENU_H
#define EXULTMENU_H

#include "palette.h"
#include "game.h"

class Mouse;
class Image_buffer8;
class Game_window;

class ExultMenu {
private:
	Game_window *gwin;
	Image_buffer8 *ibuf;
	Vga_file exult_flx;
	//	Palette pal;
	int topx, topy, centerx, centery, menuy;
	void calc_win();
	Mouse *menu_mouse;
public:
	ExultMenu(Game_window *gw);
	~ExultMenu();
	Exult_Game run();
	void setup();
};

#endif //EXULTMENU_H
