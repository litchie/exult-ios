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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>

#include "hash_utils.h"
#include "exult_constants.h"
#include "palette.h"
#include "vgafile.h"
#include "singles.h"

class Game_window;
class Image_window8;
class Image_buffer8;
class Mouse;

struct str_int_pair
{
	const char *str;
	int  num;
};

enum Exult_Game {
	NONE,
	BLACK_GATE,
	SERPENT_ISLE,
	EXULT_DEVEL_GAME		// One that we develop.
};

#define GAME_BG (Game::get_game_type() == BLACK_GATE)
#define GAME_SI (Game::get_game_type() == SERPENT_ISLE)
#define GAME_FOV (Game::get_game_type()==BLACK_GATE && Game::has_expansion())
#define GAME_SS (Game::get_game_type()==SERPENT_ISLE && Game::has_expansion())

class Game : public Game_singletons {
private:
	static bool new_game_flag;
	static Exult_Game game_type;
	static bool expansion;
#ifndef DONT_HAVE_HASH_MAP
	hash_map<const char*, int, hashstr, eqstr> shapes;
	hash_map<const char*, str_int_pair, hashstr, eqstr> resources;
#else /* !HAVE_HASH_MAP */
	std::map<const char*, int, ltstr> shapes;
	std::map<const char*, str_int_pair, ltstr> resources;
#endif
	Mouse *menu_mouse;
	static std::string gametitle;
	static unsigned int ticks;
protected:
	static bool editing_flag;
	int topx, topy, centerx, centery;
	Vga_file menushapes;
	bool	jive;

public:
	Game_window *gwin;
	Image_window8 *win;
	Image_buffer8 *ibuf;
	//	Palette pal;

	Game();
	virtual ~Game();

	static void set_new_game() { new_game_flag = true; }
	static bool is_new_game() { return new_game_flag; }
	static bool is_editing() { return editing_flag; }
	static Game *create_game(Exult_Game mygame, const char *title = 0);
	static Exult_Game get_game_type() { return game_type; }
	static bool has_expansion() { return expansion; }

	static const char *get_avname ();
	static int get_avsex ();
	static int get_avskin ();
	static void set_avname (char *name);
	static void set_avsex (int sex);
	static void set_avskin (int skin);
	static void clear_avname ();
	static void clear_avsex ();
	static void clear_avskin ();

	static std::string get_gametitle() { return gametitle; }
	
	virtual void play_intro() =0;
	virtual void end_game(bool success) =0;
	virtual void top_menu() =0;
	virtual void show_quotes() =0;
	virtual void show_credits() =0;
	virtual bool new_game(Vga_file &shapes) =0;
	virtual int  get_start_tile_x() =0;
	virtual int  get_start_tile_y() =0;
	virtual void show_journey_failed() = 0;

	void play_flic(const char *archive, int index);
	void play_audio(const char *archive, int index);
	void play_midi(int track, bool repeat = false);
	
	void add_shape(const char *name, int shapenum);
	int get_shape(const char *name);
	void add_resource(const char *name, const char *str, int num);
	str_int_pair get_resource(const char *name);
	
	bool show_menu(bool skip = false);
	void journey_failed_text();
	void set_jive () {jive = true;}
	void clear_jive () {jive = false;}

	inline static unsigned int get_ticks() { return ticks; }
	inline static void set_ticks(unsigned int t) { ticks = t; }
};

extern Game *game;
extern int wait_delay(int ms, int startcol = 0, int ncol = 0);
extern Exult_Game exult_menu(Game_window *gwin);
	
#endif
