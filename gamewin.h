/*
 *	gamewin.h - X-windows Ultima7 map browser.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
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

#ifndef GAMEWIN_H
#define GAMEWIN_H

#include "exult_constants.h"
#include "flags.h"
#include "iwin8.h"
#include "lists.h"
#include "rect.h"
#include "tiles.h"
#include "gameclk.h"
#include "shapeid.h"

#include <string>	// STL string
#include "vec.h"

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
class Fonts_vga_file;
class Game_object;
class Gump;
class Gump_button;
class Ireg_game_object;
class Dead_body;
class Main_actor;
class Npc_actor;
class Npc_face_info;
class Npc_proximity_handler;
class Palette;
class Special_effect;
class Time_queue;
class Usecode_machine;
class Deleted_objects;
class Gump_manager;
struct SaveGame_Details;
struct SaveGame_Party;
class Map_patch_collection;
class Dragging_info;
class Game_map;

					// Special pixels.
enum Pixel_colors {POISON_PIXEL = 0, PROTECT_PIXEL, CURSED_PIXEL, HIT_PIXEL,
			NPIXCOLORS};

/*
 *	The main game window:
 */
class Game_window
	{
	static Game_window *game_window;// There's just one.
	Image_window8 *win;		// Window to display into.
	Shape_manager *shape_man;	// Manages shape file.
	Palette *pal;
	Game_map *map;			// Holds all terrain.
	Usecode_machine *usecode;	// Drives game plot.
	bool combat;			// true if in combat.
	Time_queue *tqueue;		// Time-based queue.
	Game_clock clock;		// Keeps track of time.
	long time_stopped;		// For 'stop time' spell.
	int std_delay;			// Standard delay between frames.
	Npc_proximity_handler *npc_prox;// Handles nearby NPC's.
	Special_effect *effects;	// Text snippets, sprite effects.
	Gump_manager *gump_man;		// Open containers on screen.
	unsigned long render_seq;	// For marking rendered objects.
	bool painted;			// true if we updated image buffer.
	bool focus;			// Do we have focus?
	unsigned char special_pixels[NPIXCOLORS];	// Special colors.
	bool teleported;		// true if just teleported.
	unsigned int in_dungeon;	// true if inside a dungeon.
	bool ice_dungeon;		// true if inside ice dungeon
	Fonts_vga_file *fonts;		// "fonts.vga" file.
	Shape_file *extra_fonts[5];	// extra font shapes
	Xform_palette xforms[11];	// Transforms translucent colors
					//   0xf4 through 0xfe.
	Xform_palette invis_xform;	// For showing invisible NPC's.
	Barge_object *moving_barge;	// ->cart/ship that's moving, or 0.
	Main_actor *main_actor;		// Main sprite to move around.
	int skip_above_actor;		// Level above actor to skip rendering.
	int num_npcs1;			// Number of type1 NPC's.
	Actor_vector npcs;		// Array of NPC's + the Avatar.
	Exult_vector<Dead_body*> bodies;// Corresponding Dead_body's.
	Deleted_objects *removed;	// List of 'removed' objects.
	int scrolltx, scrollty;		// Top-left tile of screen.
	Actor *camera_actor;		// What to center view around.
	Rectangle scroll_bounds;	// Walking outside this scrolls.
	int palette;			// Palette #.
	int brightness;			// Palette brightness.
	int user_brightness;		// User's setting for brightness.
	bool faded_out;			// true if faded palette to black.
	bool fades_enabled;
	unsigned long special_light;	// Game minute when light spell ends.
	Rectangle dirty;		// Dirty rectangle.
	char *save_names[10];		// Names of saved games.
	long last_restore_hour;		// Hour in game of last restore.
	Dragging_info *dragging;	// Dragging info:
					// Theft info:
	int theft_warnings;		// # times warned in current chunk.
	short theft_cx, theft_cy;	// Chunk where warnings occurred.
	Time_sensitive *background_noise;

	void set_scrolls(Tile_coord cent);
	void set_scroll_bounds();	// Set scroll-controller.
	void clear_world();		// Clear out world's contents.
	void read_save_names();		// Read in saved-game names.
	void paint_terrain_only(int start_chunkx, int start_chunky,
				int stop_chunkx, int stop_chunky);
					// Render the map & objects.
	int paint_map(int x, int y, int w, int h);
					// Render dungeon blackness
	void paint_blackness(int cx, int cy, int stop_chunkx, int stop_chunky, int index=0);

	bool mouse3rd;			// use third (middle) mouse button
	bool fastmouse;
	bool double_click_closes_gumps;
	bool walk_after_teleport;
	int text_bg;			// draw a dark background behind text

#ifdef RED_PLASMA
	// Red plasma animation during game load
	uint32 load_palette_timer;
    int plasma_start_color, plasma_cycle_range;
#endif
	
public:
	int skip_lift;			// Skip objects with lift >= this.  0
					//   means 'terrain-editing' mode.
	bool paint_eggs;
	bool armageddon;		// Spell was cast.
	int combat_difficulty;		// 0=normal, >0 harder, <0 easier.
	int debug;
	Game_window(int width = 0, int height = 0, int scale = 1, 
							int scaler = 0);
	~Game_window();
					// Get the one game window.
	static Game_window *get_instance()
		{ return game_window; }
	void clear_screen(bool update = false);
		
	void set_window_size(int w, int h, int s, int sclr);
	void abort(const char *msg, ...);	// Fatal error.
	bool get_mouse3rd() const
		{ return mouse3rd; }
	void set_mouse3rd(bool m)
		{ mouse3rd = m; }
	bool get_fastmouse() const
		{ return get_win()->is_fullscreen() ? fastmouse : false; }
	void set_fastmouse(bool f)
		{ fastmouse = f; }
	bool get_double_click_closes_gumps() const
		{ return double_click_closes_gumps; }
	void set_double_click_closes_gumps(bool d)
		{ double_click_closes_gumps = d; }
	bool get_walk_after_teleport() const
		{ return walk_after_teleport; }
	void set_walk_after_teleport(bool w)
		{ walk_after_teleport = w; }
	int get_text_bg() const
		{ return text_bg; }
	void set_text_bg(int t)
		{ text_bg = t; }
	int get_width() const
		{ return win->get_width(); }
	int get_height() const
		{ return win->get_height(); }
	inline int get_scrolltx() const		// Get window offsets in tiles.
		{ return scrolltx; }
	inline int get_scrollty() const
		{ return scrollty; }
	inline Game_map *get_map() const
		{ return map; }
	inline Usecode_machine *get_usecode() const
		{ return usecode; }
	inline Rectangle get_win_rect() const	// Get window's rectangle.
		{ return Rectangle(0, 0, win->get_width(), win->get_height());}
	Rectangle get_win_tile_rect()	// Get it in tiles, rounding up.
		{ return Rectangle(get_scrolltx(), get_scrollty(),
			(get_width() + c_tilesize - 1)/c_tilesize,
			(get_height() + c_tilesize - 1)/c_tilesize); }
					// Clip rectangle to window's.
	Rectangle clip_to_win(Rectangle r)
		{
		Rectangle wr = get_win_rect();
		return (r.intersect(wr));
		}
	inline Image_window8 *get_win() const
		{ return win; }
	inline Time_queue *get_tqueue() const
		{ return tqueue; }
	inline Xform_palette get_xform(int i) const
		{ return xforms[i]; }
	int get_hour()			// Get current time.
		{ return clock.get_hour(); }
	int get_minute()
		{ return clock.get_minute(); }
	unsigned long get_total_hours()
		{ return clock.get_total_hours(); }
	void increment_clock(int num_minutes)
		{ clock.increment(num_minutes); }
	void fake_next_period()		// For debugging.
		{ clock.fake_next_period(); }
	Game_clock *get_clock () { return &clock; }
	void set_fades_enabled(bool f) { fades_enabled = f; }		
	bool get_fades_enabled() const { return fades_enabled; }
	void set_palette()		// Set for time, flags, lighting.
		{ clock.set_palette(); }
	void reload_shapes(int dragtype);	// Reload a shape file.
	Map_patch_collection *get_map_patches();
					// Get/create objs. list for a chunk.
	Map_chunk *get_chunk(int cx, int cy);
	Map_chunk *get_chunk(Game_object *obj);
	Map_chunk *get_chunk_safely(int cx, int cy);
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
	bool main_actor_dont_move();
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
		{ return special_light != 0; }
					// Light spell.
	void add_special_light(int minutes);
					// Handle 'stop time' spell.
	void set_time_stopped(long ticks);
protected:
	long check_time_stopped();
public:
	long is_time_stopped()
		{ return !time_stopped ? 0 : check_time_stopped(); }
	int get_std_delay() const	// Get/set animation frame delay.
		{ return std_delay; }
	void set_std_delay(int msecs)
		{ std_delay = msecs; }
	inline Actor *get_npc(long npc_num) const
		{ return (npc_num >= 0 && npc_num < npcs.size()) ? 
				npcs[npc_num] : 0; }
	void set_body(int npc_num, Dead_body *body)
		{ bodies.put(npc_num, body); }
	Dead_body *get_body(int npc_num)
		{ return bodies[npc_num]; }
	inline bool was_teleported()
		{ return teleported; }
	int get_num_npcs()
		{ return npcs.size(); }
	int get_unused_npc();		// Find first unused NPC #.
	void add_npc(Actor *npc, int num);	// Add new one.
	inline int in_combat()		// In combat mode?
		{ return combat; }
	void toggle_combat();
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int newh, unsigned int newsc, unsigned int newsclr);
	inline void set_painted()		// Force blit.
		{ painted = 1; }
	inline bool was_painted()
		{ return painted; }
	bool show(bool force = false)			// Returns true if blit occurred.
		{
		if (painted || force)
			{
			win->show();
			painted = false;
			return true;
			}
		return false;
		}
					// Locate shape (for EStudio).
	bool locate_shape(int shapenum, bool upwards);
	void send_location();		// Send our location to EStudio.
					// Set view (upper-left).
	void set_scrolls(int newscrolltx, int newscrollty);
	void center_view(Tile_coord t);	// Center view around t.
	void set_camera_actor(Actor *a);
	Actor *get_camera_actor()
		{ return camera_actor; }
					// Scroll if necessary.
	bool scroll_if_needed(Tile_coord t);
	bool scroll_if_needed(Actor *a, Tile_coord t)
		{ if (a == camera_actor) return scroll_if_needed(t); else return false; }
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
					// Paint shape in window.
	void paint_shape(int xoff, int yoff, Shape_frame *shape,
						int translucent = 0)
		{
		if (!shape || !shape->data)
			{
				CERR("NULL SHAPE!!!");
				return;
			}
		if (!shape->rle)	// Not RLE?
			win->copy8(shape->data, 8, 8, xoff - c_tilesize, 
						yoff - c_tilesize);
		else if (!translucent)
			shape->paint_rle(win->get_ib8(), xoff, yoff);
		else
			shape->paint_rle_translucent(win->get_ib8(), 
					xoff, yoff, xforms, 
					sizeof(xforms)/sizeof(xforms[0]));
		}

	inline void paint_shape(int xoff, int yoff, ShapeID &shape, bool force_trans = false)
		{ paint_shape(xoff, yoff, shape.get_shape(), force_trans||shape.is_translucent()); }
	inline void paint_invisible(int xoff, int yoff, Shape_frame *shape)
		{
		if (shape) shape->paint_rle_transformed(win->get_ib8(),
						xoff, yoff, invis_xform);
		}

					// Paint outline around a shape.
	inline void paint_outline(int xoff, int yoff, Shape_frame *shape, 
							Pixel_colors pix)
		{
		if (shape) shape->paint_rle_outline(win->get_ib8(), 
					xoff, yoff, special_pixels[(int) pix]);
		}
	unsigned char get_special_pixel(Pixel_colors pix)
		{ return special_pixels[(int) pix]; }
	Ireg_game_object *create_ireg_object(Shape_info& info, int shnum, 
			int frnum, int tilex, int tiley, int lift);
	Ireg_game_object *create_ireg_object(int shnum, int frnum);
	void write();// Write out to 'gamedat'.
	void read();			// Read in 'gamedat'.
	void write_gwin();		// Write gamedat/gamewin.dat.
	void read_gwin();		// Read gamedat/gamewin.dat.
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
public:
#endif

	void write_saveinfo();		// Write the save info to gamedat

	void clear_dirty()		// Clear dirty rectangle.
		{ dirty.w = 0; }
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
		{ dirty = Rectangle(0, 0, get_width(), get_height()); }
	void add_dirty(Rectangle r)	// Add rectangle to dirty area.
		{ dirty = dirty.w > 0 ? dirty.add(r) : r; }
					// Add dirty rect. for obj.  Rets. false
					//   if not on screen.
	bool add_dirty(Game_object *obj)
		{
		Rectangle rect = get_shape_rect(obj);
		rect.enlarge(5);
		rect = clip_to_win(rect);
		if (rect.w > 0 && rect.h > 0)
			{
			add_dirty(rect);
			return true;
			}
		else
			return false;
		}
	inline char *get_save_name(int i) const	// Get ->saved-game name.
		{ return save_names[i]; }
					// Paint "flat" scenery in a chunk.
	void paint_chunk_flats(int cx, int cy, int xoff, int yoff);
	//				// Paint blackness in a dungeon
	//void paint_dungeon_black(int cx, int cy, int xoff, int yoff, int index=0);
					// Paint objects in given chunk at
					//   given lift.
	int paint_chunk_objects(int cx, int cy);
					// Paint an obj. after dependencies.
	void paint_object(Game_object *obj);
					// Fade palette in/out.
	void fade_palette(int cycles, int inout, int pal_num = -1);
	bool is_palette_faded_out()
		{ return faded_out; }
	void flash_palette_red();	// Flash red for a moment.
					// Set desired palette.
	void set_palette(int pal_num, int new_brightness = -1, bool repaint=true);
	int get_brightness()		// Percentage:  100 = normal.
		{ return brightness; }
	void brighten(int per);		// Brighten/darken by percentage for
					//   the user.
	int get_users_brightness()	// What user set it at.
		{ return user_brightness; }
	void view_right();		// Move view 1 chunk to right.
	void view_left();		// Move view left by 1 chunk.
	void view_down();		// Move view down.
	void view_up();			// Move view up.
					// Start moving actor.
	void start_actor(int winx, int winy, int speed = 125);
	void start_actor_along_path(int winx, int winy, int speed = 125);
	void stop_actor();		// Stop main actor.
	void teleport_party(Tile_coord t, bool skip_eggs = false);
					// Get ->party members.
	int get_party(Actor **list, int avatar_too = 0);
	void activate_item(int shnum, int frnum=c_any_framenum,
			   int qual=c_any_qual); // Activate item in party.
					// Find object (x, y) is in.
	Game_object *find_object(int x, int y);
	int find_objects(int lift, int x, int y, Game_object_vector& list);
					// Show names of items clicked on.
	void show_items(int x, int y, bool ctrl = false);
	ShapeID get_flat(int x, int y);	// Return terrain (x, y) is in.
					// Schedule object for deletion.
	void delete_object(Game_object *obj);
					// Add text item.
	void add_text(const char *msg, Game_object *item);
	void add_text(const char *msg, int x, int y);
	void center_text(const char *msg);
	void add_effect(Special_effect *effect);
	void remove_text_effect(Game_object *item);
					// Remove text item & delete it.
	void remove_effect(Special_effect *txt);
	void remove_all_effects(bool repaint=false);
	void remove_text_effects();
					// Remove just the weather.
	void remove_weather_effects(int dist = 0);
	int get_weather();		// Get # of last weather added.
					// Handle a double-click in window.
	void double_clicked(int x, int y);
					// Add npc to 'nearby' list.
	void add_nearby_npc(Npc_actor *npc);
	void remove_nearby_npc(Npc_actor *npc);
					// Track npcs in range of chunks.
	void add_nearby_npcs(int from_cx, int from_cy,
						int stop_cx, int stop_cy);
					// Get all nearby NPC's.
	void get_nearby_npcs(Actor_queue& list);
					// Update NPCs' schedules.
	void schedule_npcs(int hour3, int backwards = 0, bool repaint = true);
	void mend_npcs();		// Restore HP's each hour.
	void theft();			// Handle thievery.
	void attack_avatar(int num_guards = 0);
	void get_focus();		// Get/lose focus.
	void lose_focus();
	inline bool have_focus() const
		{ return focus; }
	void setup_game();		// Prepare for game
	void read_npcs();		// Read in npc's.
	void write_npcs();		// Write them back.
	void read_schedules();		// Read npc's schedules.
	void write_schedules();		// Write npc's schedules.
	void revert_schedules(Actor *);	// Reset a npc's schedule.
					// Start dragging.
	bool start_dragging(int x, int y);
	bool drag(int x, int y);	// During dragging.
	bool drop_dragged(int x, int y, bool moved);// Done dragging.
	bool is_dragging() const { return dragging != 0; }
					// Paint text using "fonts.vga".
	int paint_text_box(int fontnum, const char *text, int x, int y, int w, 
		int h, int vert_lead = 0, int pbreak = 0, int shading = -1);
	int paint_text(int fontnum, const char *text, int xoff, int yoff);
	int paint_text(int fontnum, const char *text, int textlen, 
							int xoff, int yoff);
					// Get text width.
	int get_text_width(int fontnum, const char *text);
	int get_text_width(int fontnum, const char *text, int textlen);
					// Get text height, baseline.
	int get_text_height(int fontnum);
	int get_text_baseline(int fontnum);
	Font *get_font(int fontnum);
	bool drop_at_lift(Game_object *to_drop, int x, int y, int at_lift);
	bool init_gamedat(bool create); // Initialize gamedat directory
					// Explode a savegame into "gamedat".
	void restore_gamedat(const char *fname);
	void restore_gamedat(int num);
					// Save "gamedat".
	void save_gamedat(const char *fname, const char *savename);
	void save_gamedat(int num, const char *savename);
					// Get IDENTITY string.
	static char *get_game_identity(const char *savename);

#ifdef HAVE_ZIP_SUPPORT
private:
	bool save_gamedat_zip(const char *fname, const char *savename);
	bool Restore_level2 (void *unzipfile);
	bool restore_gamedat_zip(const char *fname);
	static char *get_game_identity_zip(const char *savename);
public:
#endif

	void plasma(int w, int h, int x, int y, int startc, int endc);
	
	// Create a mini-screenshot (96x60)
	Shape_file* create_mini_screenshot ();

	inline bool get_frame_skipping()	// This needs doing
	{ return true; }
	// Old Style Caching Emulation. Called if player has changed chunks
	void emulate_cache(int oldx, int oldy, int newx, int newy);
	// Is a specific move by a monster or item allowed
	bool emulate_is_move_allowed(int tx, int ty);
	// Swapping a superchunk to disk emulation
	void emulate_swapout (int scx, int scy);
	inline Gump_manager *get_gump_man() { return gump_man; }
	Gump *get_dragging_gump();
	inline Npc_proximity_handler *get_npc_prox()  { return npc_prox; }

protected:
	void start_actor_alt (int winx, int winy, int speed);

#ifdef RED_PLASMA
	void setup_load_palette();
	void cycle_load_palette();
#endif
	};
#endif
