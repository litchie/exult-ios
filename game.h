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
 
#ifndef GAME_H
#define GAME_H

class Game_window;
class Image_window8;

class Game
	{
public:
	Game_window *gwin;
	Image_window8 *win;

	Game();
	virtual ~Game();
	
	static Game *get_game(const char *identity);
	
	virtual void play_intro() =0;
	virtual void end_game(bool success) =0;
	virtual void show_menu() =0;
	virtual void show_quotes() =0;
	virtual void show_credits() =0;
	virtual int  get_start_tile_x() =0;
	virtual int  get_start_tile_y() =0;
	
	void clear_screen();
	void refresh_screen();
	void play_flic(const char *archive, int index);
	void play_audio(const char *archive, int index);
	void play_midi(int track);
	bool wait_delay(int ms);
	};

class BG_Game: public Game
	{
public:
	BG_Game();
	~BG_Game();
	
	virtual void play_intro();
	virtual void end_game(bool success);
	virtual void show_menu();
	virtual void show_quotes();
	virtual void show_credits();
	virtual int  get_start_tile_x()
		{ return (64*tiles_per_chunk); }
	virtual int  get_start_tile_y()
		{ return (136*tiles_per_chunk); }
	};

class SI_Game: public Game
	{
public:
	SI_Game();
	~SI_Game();
	
	virtual void play_intro();
	virtual void end_game(bool success);
	virtual void show_menu();
	virtual void show_quotes();
	virtual void show_credits();
	virtual int  get_start_tile_x()
		{ return (64*tiles_per_chunk); }
	virtual int  get_start_tile_y()
		{ return (136*tiles_per_chunk); }
	};

#endif
