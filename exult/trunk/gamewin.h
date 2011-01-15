/*
 *	gamewin.h - X-windows Ultima7 map browser.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2011  The Exult Team
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

#ifndef GAMEWIN_H
#define GAMEWIN_H

#include "flags.h"
#include "iwin8.h"
#include "rect.h"
#include "tiles.h"
#include "vgafile.h"

#include <string>	// STL string
#include <vector>

#define RED_PLASMA	1

#ifdef RED_PLASMA
#define	CYCLE_RED_PLASMA()	cycle_load_palette()
#else
#define	CYCLE_RED_PLASMA()
#endif

class Actor;
class Barge_object;
class Map_chunk;
class Chunk_terrain;
class Egg_object;
class Font;
class Game_object;
class Game_clock;
class Time_sensitive;
class Gump;
class Gump_button;
class Ireg_game_object;
class Dead_body;
class Main_actor;
class Npc_actor;
class Npc_face_info;
class Npc_proximity_handler;
class Palette;
class Time_queue;
class Usecode_machine;
class Deleted_objects;
class Gump_manager;
struct SaveGame_Details;
struct SaveGame_Party;
class Map_patch_collection;
class Dragging_info;
class Game_map;
class Shape_manager;
class Party_manager;
class ShapeID;
class Shape_info;
class Game_render;
class Effects_manager;

using std::vector;

/*
 *	The main game window:
 */
class Game_window
	{
	static Game_window *game_window;// There's just one.
		// Game component classes:
	Dragging_info *dragging;	// Dragging info:
	Effects_manager *effects;	// Manages special effects.
	Game_clock *clock;		// Keeps track of time.
	vector<Game_map*> maps;	// Hold all terrain.
	Game_map *map;			// The current map.
	Game_render *render;		// Helps with rendering.
	Gump_manager *gump_man;		// Open containers on screen.
	Party_manager *party_man;	// Keeps party list.
	Image_window8 *win;		// Window to display into.
	Npc_proximity_handler *npc_prox;// Handles nearby NPC's.
	Palette *pal;
	Shape_manager *shape_man;	// Manages shape file.
	Time_queue *tqueue;		// Time-based queue.
	Time_sensitive *background_noise;
	Usecode_machine *usecode;	// Drives game plot.
		// Game state flags:
	bool combat;			// true if in combat.
	bool focus;			// Do we have focus?
	bool ice_dungeon;		// true if inside ice dungeon
	bool painted;			// true if we updated image buffer.
	bool ambient_light;		// Permanent version of special_light.
		// Game state values:
	int skip_above_actor;		// Level above actor to skip rendering.
	unsigned int in_dungeon;	// true if inside a dungeon.
	int num_npcs1;			// Number of type1 NPC's.
	int std_delay;			// Standard delay between frames.
	long time_stopped;		// For 'stop time' spell.
	unsigned long special_light;	// Game minute when light spell ends.
	int theft_warnings;		// # times warned in current chunk.
	short theft_cx, theft_cy;	// Chunk where warnings occurred.
		// Gameplay objects:
	Barge_object *moving_barge;	// ->cart/ship that's moving, or 0.
	Main_actor *main_actor;		// Main sprite to move around.
	Actor *camera_actor;		// What to center view around.
	vector<Actor*> npcs;		// Array of NPC's + the Avatar.
	vector<Dead_body*> bodies;// Corresponding Dead_body's.
	Deleted_objects *removed;	// List of 'removed' objects.
		// Rendering info:
	int scrolltx, scrollty;		// Top-left tile of screen.
	Rectangle scroll_bounds;	// Walking outside this scrolls.
	Rectangle dirty;		// Dirty rectangle.
		// Savegames:
	char *save_names[10];		// Names of saved games.
		// Options:
	bool mouse3rd;			// use third (middle) mouse button
	bool fastmouse;
	bool double_click_closes_gumps;
	int text_bg;			// draw a dark background behind text
	int	step_tile_delta;	// multiplier for the delta in start_actor_alt
	bool allow_double_right_move;	// If moving with right click is allowed
	bool scroll_with_mouse; 		// scroll game view with mousewheel

		// Private methods:
	void set_scrolls(Tile_coord cent);
	void clear_world();		// Clear out world's contents.
	void read_save_names();		// Read in saved-game names.
	long check_time_stopped();

#ifdef RED_PLASMA
	// Red plasma animation during game load
	uint32 load_palette_timer;
	int plasma_start_color, plasma_cycle_range;
#endif
	
public:
	friend class Game_render;
	/*
	 *	Public flags and gameplay options:
	 */
	int skip_lift;			// Skip objects with lift >= this.  0
					//   means 'terrain-editing' mode.
	bool paint_eggs;
	bool armageddon;		// Spell was cast.
	bool walk_in_formation;		// Use Party_manager for walking.
	int debug;
	uint32 blits;			// For frame-counting.
	/*
	 *	Class maintenance:
	 */
	Game_window(int width, int height, bool fullscreen, int gwidth, int gheight, int scale = 1, 
		int scaler = 0, Image_window::FillMode fillmode=Image_window::AspectCorrectCentre, unsigned int fillsclr=0);
	~Game_window();
					// Get the one game window.
	static Game_window *get_instance()
		{ return game_window; }
	void abort(const char *msg, ...);	// Fatal error.
	/*
	 *	Display:
	 */
	void clear_screen(bool update = false);

	//int get_width() const
	//	{ return win->get_width(); }
	//int get_height() const
	//	{ return win->get_height(); }
	int get_width() const
		{ return win->get_game_width(); }
	int get_height() const
		{ return win->get_game_height(); }
	int get_game_width() const
		{ return win->get_game_width(); }
	int get_game_height() const
		{ return win->get_game_height(); }

	inline int get_scrolltx() const		// Get window offsets in tiles.
		{ return scrolltx; }
	inline int get_scrollty() const
		{ return scrollty; }
	inline Rectangle get_game_rect() const	// Get window's rectangle.
//		{ return Rectangle(win->get_start_x(), win->get_start_y(), win->get_full_width(), win->get_full_height());}
		{ return Rectangle(0, 0, win->get_game_width(), win->get_game_height());}
	inline Rectangle get_full_rect() const	// Get window's rectangle.
		{ return Rectangle(win->get_start_x(), win->get_start_y(), win->get_full_width(), win->get_full_height());}
	Rectangle get_win_tile_rect()	// Get it in tiles, rounding up.
		{ return Rectangle(get_scrolltx(), get_scrollty(),
			(win->get_game_width() + c_tilesize - 1)/c_tilesize,
			(win->get_game_height() + c_tilesize - 1)/c_tilesize); }
					// Clip rectangle to window's.
	Rectangle clip_to_game(Rectangle r)
		{
		Rectangle wr = get_game_rect();
		return (r.intersect(wr));
		}
	Rectangle clip_to_win(Rectangle r)
		{
		Rectangle wr = get_full_rect();
		return (r.intersect(wr));
		}
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int newh, bool newfs,
				unsigned int newgw, unsigned int newgh,
				unsigned int newsc, unsigned int newsclr,
				Image_window::FillMode newfill, unsigned int newfillsclr
				);

	void get_focus();		// Get/lose focus.
	void lose_focus();
	inline bool have_focus() const
		{ return focus; }
	/*
	 *	Game options:
	 */
	bool get_mouse3rd() const
		{ return mouse3rd; }
	void set_mouse3rd(bool m)
		{ mouse3rd = m; }
	bool get_fastmouse(bool ignorefs=false) const
		{ return (ignorefs||get_win()->is_fullscreen()) ? fastmouse : false; }
	void set_fastmouse(bool f)
		{ fastmouse = f; }
	bool get_double_click_closes_gumps() const
		{ return double_click_closes_gumps; }
	void set_double_click_closes_gumps(bool d)
		{ double_click_closes_gumps = d; }
	int get_text_bg() const
		{ return text_bg; }
	void set_text_bg(int t)
		{ text_bg = t; }
	bool can_scroll_with_mouse() const  // scroll game view with mousewheel
		{ return scroll_with_mouse; }
//	void set_mouse_scroll(bool ms)		// not currently used since
//		{ scroll_with_mouse = ms; } // there is no room in the menu
	/*
	 *	Game components:
	 */
	inline Game_map *get_map() const
		{ return map; }
	inline const vector<Game_map*>& get_maps() const
		{ return maps; }
	inline Usecode_machine *get_usecode() const
		{ return usecode; }
	inline Image_window8 *get_win() const
		{ return win; }
	inline Time_queue *get_tqueue() const
		{ return tqueue; }
	Palette *get_pal()
		{ return pal; }
	Effects_manager *get_effects()
		{ return effects; }
	inline Gump_manager *get_gump_man() { return gump_man; }
	inline Party_manager *get_party_man() { return party_man; }
	inline Npc_proximity_handler *get_npc_prox()  { return npc_prox; }
	Game_clock *get_clock () { return clock; }
	Game_map *get_map(int num);	// Read in additional map.
	void set_map(int num);		// Make map #num the current map.
	/*
	 *	ExultStudio support:
	 */
	Map_patch_collection *get_map_patches();
					// Locate shape (for EStudio).
	bool locate_shape(int shapenum, bool upwards, int frnum, int qual);
	void send_location();		// Send our location to EStudio.
	/*
	 *	Gameplay data:
	 */
	inline Barge_object *get_moving_barge() const
		{ return moving_barge; }
	void set_moving_barge(Barge_object *b);
	bool is_moving();		// Is Avatar (or barge) moving?
	inline Main_actor *get_main_actor() const
		{ return main_actor; }
	bool is_main_actor_inside()
		{ return skip_above_actor < 31 ; }
					// Returns if skip_above_actor changed!
	bool set_above_main_actor(int lift)
		{
		if (skip_above_actor == lift) return false;
		skip_above_actor = lift;
		return true;
		}
	int get_render_skip_lift() const	// Skip rendering here.
		{ return skip_above_actor < skip_lift ?
					skip_above_actor : skip_lift; }
	bool main_actor_dont_move();
	bool main_actor_can_act();
	bool main_actor_can_act_charmed();
	inline bool set_in_dungeon(unsigned int lift)
		{ 
		if (in_dungeon == lift)
			return false;
		in_dungeon = lift;
		return true;
		}
	inline void set_ice_dungeon(bool ice) { ice_dungeon = ice; }
	inline unsigned int is_in_dungeon()
		{ return in_dungeon; }
	inline bool is_special_light()	// Light spell in effect?
		{ return ambient_light || special_light != 0; }
					// Light spell.
	void add_special_light(int minutes);
	void toggle_ambient_light(bool state)
		{ ambient_light = state; }
					// Handle 'stop time' spell.
	void set_time_stopped(long ticks);
	long is_time_stopped()
		{ return !time_stopped ? 0 : check_time_stopped(); }
	int get_std_delay() const	// Get/set animation frame delay.
		{ return std_delay; }
	void set_std_delay(int msecs)
		{ std_delay = msecs; }
	inline Actor *get_npc(long npc_num) const
		{ return (npc_num >= 0 && npc_num < (int)npcs.size()) ? 
				npcs[npc_num] : 0; }
	void locate_npc(int npc_num);
	void set_body(int npc_num, Dead_body *body)
		{ if (npc_num >= (int)bodies.size())
			bodies.resize(npc_num + 1);
		  bodies[npc_num] = body;
		}
	Dead_body *get_body(int npc_num)
		{ return bodies[npc_num]; }
	int get_num_npcs()
		{ return npcs.size(); }
	int get_unused_npc();		// Find first unused NPC #.
	void add_npc(Actor *npc, int num);	// Add new one.
	inline int in_combat()		// In combat mode?
		{ return combat; }
	void toggle_combat();
	inline bool get_frame_skipping()	// This needs doing
		{ return true; }
					// Get ->party members.
	int get_party(Actor **list, int avatar_too = 0);
					// Add npc to 'nearby' list.
	void add_nearby_npc(Npc_actor *npc);
	void remove_nearby_npc(Npc_actor *npc);
					// Get all nearby NPC's.
	void get_nearby_npcs(std::vector<Actor*>& list);
					// Update NPCs' schedules.
	void schedule_npcs(int hour, int backwards = 7, bool repaint = true);
	void mend_npcs();		// Restore HP's each hour.
					// Find witness to Avatar's 'crime'.
	Actor *find_witness(Actor *& closest_npc);
	void theft();			// Handle thievery.
	void call_guards(Actor *witness = 0);
	void attack_avatar(int num_guards = 0);
	bool is_hostile_nearby(); // detects if hostiles are nearby for movement speed
	bool failed_copy_protection();
	/*
	 *	Rendering:
	 */
	inline void set_painted()	// Force blit.
		{ painted = 1; }
	inline bool was_painted()
		{ return painted; }
	bool show(bool force = false)	// Returns true if blit occurred.
		{
		if (painted || force)
			{
			win->show();
			++blits;
			painted = false;
			return true;
			}
		return false;
		}
	void clear_dirty()		// Clear dirty rectangle.
		{ dirty.w = 0; }
	bool is_dirty()
		{ return dirty.w > 0; }
					// Paint scene at given tile.
	void paint_map_at_tile(int x, int y, int w, int h,
				int toptx, int topty, int skip_above = 31);
					// Paint area of image.
	void paint(int x, int y, int w, int h);
	void paint(Rectangle& r)
		{ paint(r.x, r.y, r.w, r.h); }
	void paint();			// Paint whole image.
					// Paint 'dirty' rectangle.
	void paint_dirty();
	void set_all_dirty()		// Whole window.
		{ dirty = Rectangle(win->get_start_x(), win->get_start_y(), win->get_full_width(), win->get_full_height()); }
	void add_dirty(Rectangle r)	// Add rectangle to dirty area.
		{ dirty = dirty.w > 0 ? dirty.add(r) : r; }
					// Add dirty rect. for obj. Rets. false
					//   if not on screen.
	bool add_dirty(Game_object *obj)
		{
		Rectangle rect = get_shape_rect(obj);
		rect.enlarge(1+c_tilesize/2);
		rect = clip_to_win(rect);
		if (rect.w > 0 && rect.h > 0)
			{
			add_dirty(rect);
			return true;
			}
		else
			return false;
		}
					// Set view (upper-left).
	void set_scrolls(int newscrolltx, int newscrollty);
	void center_view(Tile_coord t);	// Center view around t.
	void set_camera_actor(Actor *a);
	Actor *get_camera_actor()
		{ return camera_actor; }
					// Scroll if necessary.
	bool scroll_if_needed(Tile_coord t);
	bool scroll_if_needed(Actor *a, Tile_coord t)
		{ if (a == camera_actor) return scroll_if_needed(t); 
							else return false; }
#if 1
					// Show abs. location of mouse.
	void show_game_location(int x, int y);
#endif
					// Get screen area of shape at pt.
	Rectangle get_shape_rect(const Shape_frame *s, int x, int y) const
		{
		return Rectangle(x - s->xleft, y - s->yabove,
				s->get_width(), s->get_height());
		}
					// Get screen area used by object.
	Rectangle get_shape_rect(Game_object *obj);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& x, int& y);
	void get_shape_location(Tile_coord t, int& x, int& y);
	void plasma(int w, int h, int x, int y, int startc, int endc);
	/*
	 *	Save/restore/startup:
	 */
	void write();			// Write out to 'gamedat'.
	void read();			// Read in 'gamedat'.
	void write_gwin();		// Write gamedat/gamewin.dat.
	void read_gwin();		// Read gamedat/gamewin.dat.
	bool was_map_modified();	// Was any map modified?
	void write_map();		// Write map data to <PATCH> dir.
	void read_map();		// Reread initial game map.
	void reload_usecode();		// Reread (patched) usecode.
	void init_actors();		// Place actors in the world.
	void init_files(bool cycle=true);	// Load all files

		// From Gamedat
	void get_saveinfo( Shape_file *&map,
			SaveGame_Details *&details,
			SaveGame_Party *& party);
		// From Savegame
	bool get_saveinfo(int num, char *&name,
			Shape_file *&map,
			SaveGame_Details *&details,
			SaveGame_Party *& party);
	void read_saveinfo(DataSource *in,
			SaveGame_Details *&details,
			SaveGame_Party *& party);
#ifdef HAVE_ZIP_SUPPORT
private:
	bool get_saveinfo_zip(const char *fname, char *&name,
			Shape_file *&map,
			SaveGame_Details *&details,
			SaveGame_Party *& party);
	void restore_flex_files(DataSource &ds, const char *basepath);
public:
#endif
	void write_saveinfo();		// Write the save info to gamedat
	inline char *get_save_name(int i) const	// Get ->saved-game name.
		{ return save_names[i]; }
	void setup_game(bool map_editing);	// Prepare for game
	void read_npcs();		// Read in npc's.
	void write_npcs();		// Write them back.
	void read_schedules();		// Read npc's schedules.
	void write_schedules();		// Write npc's schedules.
	void revert_schedules(Actor *);	// Reset a npc's schedule.
					// Explode a savegame into "gamedat".
	void restore_gamedat(const char *fname);
	void restore_gamedat(int num);
					// Save "gamedat".
	void save_gamedat(const char *fname, const char *savename);
	void save_gamedat(int num, const char *savename);
					// Get IDENTITY string.
	static const char *get_game_identity(const char *savename);
	bool init_gamedat(bool create); // Initialize gamedat directory
#ifdef HAVE_ZIP_SUPPORT
private:
	bool save_gamedat_zip(const char *fname, const char *savename);
	bool Restore_level2 (void *unzipfile, const char *dirname, int dirlen);
	bool restore_gamedat_zip(const char *fname);
	static const char *get_game_identity_zip(const char *savename);
public:
#endif
	/*
	 *	Game control:
	 */
	void view_right();		// Move view 1 chunk to right.
	void view_left();		// Move view left by 1 chunk.
	void view_down();		// Move view down.
	void view_up();			// Move view up.
					// Start moving actor.
	void start_actor_alt (int winx, int winy, int speed);
	void start_actor(int winx, int winy, int speed = 125);
	void start_actor_along_path(int winx, int winy, int speed = 125);
	void stop_actor();		// Stop main actor.
	inline void set_step_tile_delta(int size) { step_tile_delta = size; }
	inline int get_step_tile_delta() { return step_tile_delta; };
	inline void set_allow_double_right_move(bool a) { allow_double_right_move = a; }
	inline bool get_allow_double_right_move() { return allow_double_right_move; }
	void teleport_party(Tile_coord t, bool skip_eggs = false, 
							int new_map = -1, bool no_status_check = true);
	bool activate_item(int shnum, int frnum=c_any_framenum,
			   int qual=c_any_qual); // Activate item in party.
					// Find object (x, y) is in.
	Game_object *find_object(int x, int y);
					// Show names of items clicked on.
	void show_items(int x, int y, bool ctrl = false);
					// Right-click while combat paused.
	void paused_combat_select(int x, int y);
	ShapeID get_flat(int x, int y);	// Return terrain (x, y) is in.
					// Schedule object for deletion.
	void delete_object(Game_object *obj);
					// Handle a double-click in window.
	void double_clicked(int x, int y);
	bool start_dragging(int x, int y);
	bool drag(int x, int y);	// During dragging.
	bool drop_dragged(int x, int y, bool moved);// Done dragging.
	bool is_dragging() const { return dragging != 0; }
	int drop_at_lift(Game_object *to_drop, int x, int y, int at_lift);
	Gump *get_dragging_gump();
	// Create a mini-screenshot (96x60)
	Shape_file* create_mini_screenshot ();
	/*
	 *	Chunk-caching:
	 */
	// Old Style Caching Emulation. Called if player has changed chunks
	void emulate_cache(Map_chunk *olist, Map_chunk *nlist);
	// Is a specific move by a monster or item allowed
	bool emulate_is_move_allowed(int tx, int ty);
	// Swapping a superchunk to disk emulation
	void emulate_swapout (int scx, int scy);


#ifdef RED_PLASMA
	void setup_load_palette();
	void cycle_load_palette();
#endif

private:
	//
	// Interpolated painting stuff, for smooth scrolling
	//

	// These are saved scroll positions
	int scrolltx_l, scrollty_l;		
	int scrolltx_lp, scrollty_lp;
	// These are the pixel offset that needs to be subtracted from shape positions due to smooth scrolling
	int scrolltx_lo, scrollty_lo;	
	// Delta for camera actor position in pixels due to lerping of position
	int avposx_ld, avposy_ld;		
	// Is lerping enabled
	int lerping_enabled;

public:
	// Reset (well update really) saved lerp scroll positions
	void lerp_reset();

	// (Re)paint the entire screen using a lerp factor 0-0x10000
	void paint_lerped(int factor);

	inline int get_scrolltx_lo() const
		{ return scrolltx_lo; }
	inline int get_scrollty_lo() const
		{ return scrollty_lo; }

	int is_lerping_enabled() const { return lerping_enabled; }
	void set_lerping_enabled(int e) { lerping_enabled = e; }
	};
	
void Set_renderer(Image_window8 *win, Palette *pal = 0, bool resize = false);
bool Set_glpalette(Palette *pal = 0, bool rotation = false);

#endif
