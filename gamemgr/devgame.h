/*
 *  Copyright (C) 2001-2013  The Exult Team
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

#ifndef DEVGAME_H
#define DEVGAME_H

#include "sigame.h"

/*
 *  Inheriting from SI_Game is a temporary kludge.+++++++++
 */
class DEV_Game: public SI_Game {
public:
#if 0
	DEV_Game() = default;
	~DEV_Game() override;
	void play_intro() override;
	void end_game(bool success) override;
	void top_menu() override;
	void show_quotes() override;
	void show_credits() override;
	bool new_game(Vga_file &shapes) override;
	int  get_start_tile_x() override {
		return 25 * c_tiles_per_chunk;
	}
	int  get_start_tile_y() override {
		return 155 * c_tiles_per_chunk;
	}
	void show_journey_failed() override;
#endif
};


#endif
