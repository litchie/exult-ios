/*
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
 
#include "gamewin.h"
#include "imagewin.h"

#ifndef TITLES_H
#define TITLES_H

class Titles
	{
	Game_window *gwin;
	Image_window8 *win;
public:
	Titles();
	~Titles();
	
	void play_intro();
	void clear_screen();
	void refresh_screen();
	void show_menu();
	void play_flic(const char *archive, int index);
	void play_audio(const char *archive, int index);
	void play_midi(int track);
	void end_game(bool success);
	void show_quotes();
	void show_credits();
	bool wait_delay(int ms);
	};

#endif
