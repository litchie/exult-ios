/*
 *  Copyright (C) 2000-2001  The Exult Team
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

#ifndef BGGAME_H
#define BGGAME_H

#include "game.h"


class BG_Game: public Game {
public:
	BG_Game();
	~BG_Game();
	
	virtual void play_intro();
	virtual void end_game(bool success);
	virtual void top_menu();
	virtual void show_quotes();
	virtual void show_credits();
	virtual bool new_game(Vga_file &shapes);
	virtual int  get_start_tile_x()
		{ return (64*c_tiles_per_chunk); }
	virtual int  get_start_tile_y()
		{ return (136*c_tiles_per_chunk); }
	virtual void show_journey_failed();
	static bool is_installed();

private:
	Vga_file shapes;

	void scene_lord_british();
	void scene_butterfly();
	void scene_guardian();
	void scene_desk();
	void scene_moongate();
};

	
#endif
