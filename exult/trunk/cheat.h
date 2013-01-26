/*
 *  Copyright (C) 2000-2013  The Exult Team
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

#ifndef CHEAT_H
#define CHEAT_H

#include <vector>
#include "singles.h"

class Game_window;
class ShapeBrowser;
class SoundTester;
class CheatScreen;
class Actor;
class Game_object;
class Tile_coord;
class Effects_manager;
class Map_chunk;

using std::vector;

class Cheat : public Game_singletons {
public:
	Cheat();
	~Cheat();

	void init(void);
	void finish_init(void);
	enum Map_editor_mode {
	    move = 0,           // Normal dragging.
	    paint = 1,          // Left-mouse dragging paints shapes.
	    paint_chunks = 2,       // Left-dragging paints whole chunks.
	    combo_pick = 3,         // Left-click adds item to combo.
	    select_chunks = 4       // Select whole chunks.
	};
private:
	ShapeBrowser *browser;
	SoundTester *tester;
	CheatScreen *cscreen;

	bool god_mode;
	bool wizard_mode;
	bool map_editor;
	bool tile_grid;
	Map_editor_mode edit_mode;
	int  edit_lift;
	int  edit_shape, edit_frame;      // What to 'paint' with.
	int  edit_chunknum;           // For painting with chunks.
	vector<Game_object *> selected;   // Selected objects (map-editing).
	vector<Game_object *> clipboard;  // Cut/copy/paste objects.
	bool infravision;
	bool pickpocket;
	bool grab_actor;
	bool npc_numbers;
	bool hack_mover;

	bool enabled;
	// Rectangle containing selected chunks:
	int chunksel_left, chunksel_right, chunksel_top, chunksel_bottom;

	void send_select_status();
public:
	bool operator()(void) const {
		return enabled;
	}
	void set_enabled(bool en);

	bool in_god_mode(void) const {
		return god_mode;
	}
	bool in_wizard_mode(void) const {
		return wizard_mode;
	}
	bool in_map_editor(void) const {
		return map_editor;
	}
	bool show_tile_grid(void) const {
		return map_editor && tile_grid;
	}
	Map_editor_mode get_edit_mode(void) const {
		return edit_mode;
	}
	int  get_edit_lift(void) const {
		return edit_lift;
	}
	int  get_edit_shape(void) const {
		return edit_shape;
	}
	int  get_edit_frame(void) const {
		return edit_frame;
	}
	int  get_edit_chunknum(void) const {
		return edit_chunknum;
	}
	bool in_infravision(void) const {
		return infravision;
	}
	bool in_pickpocket(void) const {
		return pickpocket;
	}
	bool in_hack_mover(void) const {
		return (hack_mover || map_editor);
	}

	void toggle_god(void);
	void set_god(bool god) {
		god_mode = god;
	}
	void toggle_wizard(void);
	void set_wizard(bool wizard) {
		wizard_mode = wizard;
	}
	void toggle_map_editor(void);
	void toggle_tile_grid(void);
	void set_edit_mode(Map_editor_mode md);
	void clear_chunksel();
	void add_chunksel(Map_chunk *chunk, bool extend = false);
	void move_chunk(Map_chunk *chunk, int dx, int dy);
	void move_selected_chunks(int dx, int dy);
	void set_edit_lift(int lift);
	void set_edit_shape(int sh, int fr);
	void set_edit_chunknum(int chnum) {
		edit_chunknum = chnum;
	}
	void set_map_editor(bool map) {
		if (map_editor != map)
			toggle_map_editor();
	}
	void toggle_infravision(void);
	void set_infravision(bool infra) {
		infravision = infra;
	}
	void toggle_pickpocket(void);
	void set_pickpocket(bool pick) {
		pickpocket = pick;
	}
	void toggle_hack_mover(void);
	void set_hack_mover(bool hm) {
		hack_mover = hm;
	}

	void toggle_eggs(void) const;
	void change_gender(void) const;

	void toggle_Petra(void) const;
	void toggle_naked(void) const;
	void change_skin(void) const;

	void levelup_party(void) const;
	void heal_party(void) const;

	void fake_time_period(void) const;
	void dec_skip_lift(void) const;
	void set_skip_lift(int skip) const;

	void append_selected(Game_object *obj);
	void toggle_selected(Game_object *obj);
	void clear_selected();
	void delete_selected();
	void move_selected_objs(int dx, int dy, int dz);
	void move_selected(int dx, int dy, int dz);
	const vector<Game_object *> &get_selected() const {
		return selected;
	}
	bool is_selected(Game_object *o);

	void cut(bool copy = false);
	void paste(int mx, int my);
	void paste();
	const vector<Game_object *> &get_clipboard() const {
		return clipboard;
	}

	void map_teleport(void) const;
	void cursor_teleport(void) const;
	void next_map_teleport(void) const;

	void create_coins(void) const;
	void create_last_shape(void) const;
	void delete_object(void);
	void shape_browser(void) const;
	bool get_browser_shape(int &shape, int &frame) const;
	void sound_tester(void) const;

	void cheat_screen(void) const;

	bool grabbing_actor(void) const {
		return grab_actor;
	}
	void toggle_grab_actor(void);
	void set_grab_actor(bool grab) {
		grab_actor = grab;
	}
	void set_grabbed_actor(Actor *actor) const;
	void clear_this_grabbed_actor(Actor *actor) const;

	bool number_npcs(void) const {
		return npc_numbers;
	}
	void toggle_number_npcs(void);
	void set_number_npcs(bool num) {
		npc_numbers = num;
	}
};

extern Cheat cheat;

#endif
