/*
Copyright (C) 2000 The Exult Team

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

#ifndef GAMEWIN_H
#define GAMEWIN_H

#include "exult_constants.h"
#include "flags.h"
#include "iwin8.h"
#include "lists.h"
#include "rect.h"
#include "tiles.h"
#include "shapeid.h"
#include "shapevga.h"
#include "gameclk.h"

#include <string>	// STL string
#include <vector>	// STL container

class Actor;
class Barge_object;
class Chunk_object_list;
class Egg_object;
class Font;
class Fonts_vga_file;
class Game_object;
class Gump;
class Gump_button;
class Ireg_game_object;
class Main_actor;
class Monster_info;
class Npc_actor;
class Npc_face_info;
class Npc_proximity_handler;
class Palette;
class Special_effect;
class Time_queue;
class Usecode_machine;

/*
 *	The main game window:
 */
class Game_window
	{
	static Game_window *game_window;// There's just one.
	Image_window8 *win;		// Window to display into.
	Palette *pal;
public:
	enum Game_mode {		// Can be in different modes.
		splash,			// Splash screen.
		menu,			// Menu screen
		quotes,			// Quotes scroller
		normal,			// Normal game-play.
		conversation,		// Talking.
		gump			// Showing open container(s).
		};
private:
	Usecode_machine *usecode;	// Drives game plot.
	Game_mode mode;			// Mode we're in.
	bool combat;			// true if in combat.
	Time_queue *tqueue;		// Time-based queue.
	Game_clock clock;		// Keeps track of time.
	Npc_proximity_handler *npc_prox;// Handles nearby NPC's.
	Special_effect *effects;	// Text snippets, sprite effects.
	Gump *open_gumps;	// Open containers on screen.
	unsigned long render_seq;	// For marking rendered objects.
	bool painted;			// true if we updated image buffer.
	bool focus;			// Do we have focus?
	unsigned char poison_pixel;	// For rendering poisoned actors.
	unsigned char protect_pixel;	// For rendering protected actors.
	bool teleported;		// true if just teleported.
	bool in_dungeon;		// true if inside a dungeon.
	std::ifstream chunks;		// "u7chunks" file.
	Shapes_vga_file shapes;		// "shapes.vga" file.
	Vga_file gumps;			// "gumps.vga" - open chests, bags.
	Vga_file paperdolls;		// "paperdoll.vga" - paperdolls in SI
	Vga_file faces;			// "faces.vga" - faces for conv.
	Fonts_vga_file *fonts;		// "fonts.vga" file.
	Shape_file *extra_fonts[5];	// extra font shapes
	Vga_file sprites;		// "sprites.vga" file.
	Vga_file mainshp;
	std::ifstream u7map;			// "u7map" file.
	Xform_palette xforms[11];	// Transforms translucent colors
					//   0xf4 through 0xfe.
	Xform_palette invis_xform;	// For showing invisible NPC's.
	Barge_object *moving_barge;	// ->cart/ship that's moving, or 0.
	Main_actor *main_actor;		// Main sprite to move around.
	int skip_above_actor;		// Level above actor to skip rendering.
	int num_npcs, num_npcs1;	// Numbers of NPC's, type1 NPC's.
	Actor **npcs;			// List of NPC's + the Avatar.
	int num_monsters;		// Number of monster types.
	Monster_info *monster_info;	// Array from 'monsters.dat'.
	std::vector<Egg_object *> path_eggs;	// Path eggs, indexed by 'quality'.
					// A list of objects in each chunk.
	Chunk_object_list *objects[c_num_chunks][c_num_chunks];
	bool schunk_read[144]; // Flag for reading in each "ifix".
	int scrolltx, scrollty;
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
					// Dragging info:
	Game_object *dragging;		// What's being dragged.
	Gump *dragging_gump;
	Gump_button *dragging_button;
					// Last mouse, paint positions:
	int dragging_mousex, dragging_mousey, dragging_paintx, dragging_painty;
	Rectangle dragging_rect;	// Rectangle to repaint.
	Image_buffer *dragging_save;	// Image below dragged object.
					// Theft info:
	int theft_warnings;		// # times warned in current chunk.
	short theft_cx, theft_cy;	// Chunk where warnings occurred.
					// Open a U7 file, throw an
					//    exception if failed
	Chunk_object_list *create_chunk(int cx, int cy);
	void set_scroll_bounds();	// Set scroll-controller.
	void clear_world();		// Clear out world's contents.
	void read_save_names();		// Read in saved-game names.
	void read_map_data();		// Read in 'ifix', 'ireg', etc.
					// Render the map & objects.
	int paint_map(int x, int y, int w, int h);
public:
	int skip_lift;			// Skip objects with lift > 0.
	bool paint_eggs;
	int debug;
	Game_window(int width = 0, int height = 0, int scale = 1);
	~Game_window();
					// Get the one game window.
	static Game_window *get_game_window()
		{ return game_window; }
	void clear_screen();
		
	void set_window_size(int w, int h, int s);
	void abort(const char *msg, ...);	// Fatal error.
	int get_width()
		{ return win->get_width(); }
	int get_height()
		{ return win->get_height(); }
	inline int get_scrolltx() const		// Get window offsets in tiles.
		{ return scrolltx; }
	inline int get_scrollty() const
		{ return scrollty; }
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
	void set_fades_enabled(bool f) { fades_enabled = f; }		
	bool get_fades_enabled() const { return fades_enabled; }
	void set_palette()		// Set for time, flags, lighting.
		{ clock.set_palette(); }
	bool is_chunk_read(int cx, int cy)
		{ return schunk_read[12*(cy/c_chunks_per_schunk) +
						cx/c_chunks_per_schunk]; }
					// Get/create objs. list for a chunk.
	Chunk_object_list *get_objects(int cx, int cy)
		{
		Chunk_object_list *list = objects[cx][cy];
		if (!list)
			list = create_chunk(cx, cy);
		return (list);
		}
	Chunk_object_list *get_objects(Game_object *obj);
	Chunk_object_list *get_objects_safely(int cx, int cy)
		{
		return (cx >= 0 && cx < c_num_chunks && 
		        cy >= 0 && cy < c_num_chunks ? get_objects(cx, cy) : 0);
		}
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
	inline bool set_in_dungeon(int tf)
		{ 
		if (in_dungeon == tf)
			return false;
		in_dungeon = tf;
		return true;
		}
	inline bool is_in_dungeon()
		{ return in_dungeon; }
	inline bool is_special_light()	// Light spell in effect?
		{ return special_light != 0; }
					// Light spell.
	void add_special_light(int minutes);
	inline Actor *get_npc(long npc_num) const
		{ return (npc_num >= 0 && npc_num < num_npcs) ? 
				npcs[npc_num] : 0; }
	inline bool was_teleported()
		{ return teleported; }
					// Find monster info. for shape.
	Monster_info *get_monster_info(int shapenum);
	Egg_object *get_path_egg(int q)	// Get path egg by quality.
		{ return path_eggs[q]; }
	void add_path_egg(Egg_object *egg);
	int get_num_npcs()
		{ return num_npcs; }
	int get_num_shapes()
		{ return shapes.get_num_shapes(); }
 	int get_num_faces() 
		{ return faces.get_num_shapes(); }
	int get_num_gumps()
		{ return gumps.get_num_shapes(); }
	int get_num_sprites()
		{ return sprites.get_num_shapes(); }
	inline void set_mode(Game_mode md)
		{ mode = md; }
	inline Game_mode get_mode() const
		{ return mode; }
	inline int in_combat()		// In combat mode?
		{ return combat; }
	void toggle_combat();
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int newh, unsigned int newsc);
	inline void set_painted()		// Force blit.
		{ painted = 1; }
	inline bool was_painted()
		{ return painted; }
	bool show()			// Returns true if blit occurred.
		{
		if (painted)
			{
			win->show();
			painted = false;
			return true;
			}
		return false;
		}
	bool show(int)
		{
		win->show();
		painted = false;
		return true;
		}
	void center_view(Tile_coord t);	// Center view around t.
					// Scroll if necessary.
	int scroll_if_needed(Tile_coord t);
#if 1
					// Show abs. location of mouse.
	void show_game_location(int x, int y);
#endif
	inline Shapes_vga_file& get_shapes()	// Get 'shapes.vga' file.
		{ return shapes; }
	Shape_info& get_info(int shnum)	// Get shape info.
		{ return shapes.get_info(shnum); }
	Shape_info& get_info(const Game_object *obj);
					// Get shape from shapes.vga.
	Shape_frame *get_shape(int shapenum, int framenum)
		{ return shapes.get_shape(shapenum, framenum); }
	Shape_frame *get_shape(const ShapeID& id)
		{ return get_shape(id.get_shapenum(), id.get_framenum()); }
					// Get # frames in a shape.
	int get_shape_num_frames(int shapenum)
		{ return shapes.get_num_frames(shapenum); }
	int get_sprite_num_frames(int shapenum)
		{ return sprites.get_num_frames(shapenum); }
					// Get screen area of shape at pt.
	Rectangle get_shape_rect(const Shape_frame *s, int x, int y)
		{
		return Rectangle(x - s->xleft, y - s->yabove,
				s->get_width(), s->get_height());
		}
					// Get screen area used by object.
	Rectangle get_shape_rect(const Game_object *obj);
	Shape_frame *get_gump_shape(int shapenum, int framenum, bool paperdoll = false)
		{ return paperdoll ? paperdolls.get_shape(shapenum, framenum) : gumps.get_shape(shapenum, framenum); }
					// Get screen area of a gump.
					//   for painting it.
	Rectangle get_gump_rect(Gump *gump);
					// Get sprites shape.
	Shape_frame *get_sprite_shape(int shapenum, int framenum)
		{ return sprites.get_shape(shapenum, framenum); }
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& x, int& y);
					// Paint shape in window.
	void paint_shape(int xoff, int yoff, Shape_frame *shape,
						int translucent = 0)
		{
		if (!shape)
			{
				std::cout << "NULL SHAPE!!!" << std::endl;
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
	void paint_shape(int xoff, int yoff, int shapenum, int framenum)
		{
		paint_shape(xoff, yoff, get_shape(shapenum, framenum),
				shapes.get_info(shapenum).has_translucency());
		}
	void paint_invisible(int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = get_shape(shapenum, framenum);
		if (shape)
			shape->paint_rle_transformed(win->get_ib8(),
						xoff, yoff, invis_xform);
		}
					// Paint outline around a shape.
	void paint_outline(int xoff, int yoff, int shapenum, int framenum,
								int pix)
		{
		Shape_frame *shape = get_shape(shapenum, framenum);
		if (shape)
			shape->paint_rle_outline(win->get_ib8(), 
					xoff, yoff, pix);
		}
	void paint_poison_outline(int xoff, int yoff, int shnum, int frnum)
		{ paint_outline(xoff, yoff, shnum, frnum, poison_pixel); }
	void paint_protect_outline(int xoff, int yoff, int shnum, int frnum)
		{ paint_outline(xoff, yoff, shnum, frnum, protect_pixel); }
					// A "gump" is an open container.
	void paint_gump(int xoff, int yoff, int shapenum, int framenum, bool paperdoll = false)
		{
		Shape_frame *shape = paperdoll ? paperdolls.get_shape(shapenum & 0xFFF, framenum) :
				gumps.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(xoff, yoff, shape);
		}
	Shape_frame *get_face(int shapenum, int framenum) {
		return faces.get_shape(shapenum, framenum);
	}
	void paint_face(int xoff, int yoff, int shapenum, int framenum) {
		Shape_frame *shape = faces.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(xoff, yoff, shape);
	}

	void paint_sprite(int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = sprites.get_shape(shapenum, framenum);
		if (shape)		// They have translucency.
			paint_shape(xoff, yoff, shape, 1);
		}
					// Get "map" superchunk objs/scenery.
	void get_map_objects(int schunk);
					// Get "chunk" objects/scenery.
	void get_chunk_objects(int cx, int cy, int chunk_num);
					// Get "ifix" objects for a superchunk.
	void get_ifix_objects(int schunk);
					// Get "ifix" objs. for given chunk.
	void get_ifix_chunk_objects(std::ifstream& ifix, long filepos, int cnt,
							int cx, int cy);
					// Get iregxx name.
	static char *get_ireg_name(int schunk, char *fname);
					// Write moveable objects to file.
	void write_ireg_objects(int schunk);
					// Get moveable objects.
	void get_ireg_objects(int schunk);
	void read_ireg_objects(std::istream& ireg, int scx, int scy,
					Game_object *container = 0,
//			unsigned long flags = (1<<11));
			unsigned long flags = (1<<Obj_flags::okay_to_take));
	Ireg_game_object *create_ireg_object(Shape_info& info, int shnum, 
			int frnum, int tilex, int tiley, int lift);
					// Create special objects.
	Egg_object *create_egg(unsigned char *entry, bool animated);
					// Get all superchunk objects.
	void get_superchunk_objects(int schunk);
	void write();			// Write out to 'gamedat'.
	void read();			// Read in 'gamedat'.
	void write_gwin();		// Write gamedat/gamewin.dat.
	void read_gwin();		// Read gamedat/gamewin.dat.
	void init_actors();		// Place actors in the world.
	void init_files();		// Load all files
	void clear_dirty()		// Clear dirty rectangle.
		{ dirty.w = 0; }
					// Paint scene at given tile.
	void paint_map_at_tile(int x, int y, int w, int h,
				int toptx, int topty, int skip_above = 31);
					// Paint area of image.
	void paint(int x, int y, int w, int h);
	void paint(Rectangle& r)
		{ paint(r.x, r.y, r.w, r.h); }
	void paint()			// Paint whole image.
		{
		read_map_data();	// Gather in all objs., etc.
		paint(0, 0, get_width(), get_height());
		clear_dirty();
		}
	void paint_dirty()		// Paint 'dirty' rectangle.
		{
		if (dirty.w > 0)
			{
			Rectangle box = dirty;
			clear_dirty();
			paint(box);	// (Could create new dirty rects.)
			}
		}
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
	void paint_tile(Chunk_object_list *olist, int tilex, int tiley,
							int xoff, int yoff);
	void paint_chunk_flats(int cx, int cy);
	void paint_dungeon_chunk_flats(int cx, int cy);
					// Paint objects in given chunk at
					//   given lift.
	int paint_chunk_objects(int cx, int cy);
					// Paint an obj. after dependencies.
	void paint_object(Game_object *obj);
	void paint_dungeon_object(Chunk_object_list *olist, Game_object *obj);
					// Fade palette in/out.
	void fade_palette(int cycles, int inout, int pal_num = -1);
	bool is_palette_faded_out()
		{ return faded_out; }
	void flash_palette_red();	// Flash red for a moment.
					// Set desired palette.
	void set_palette(int pal_num, int new_brightness = -1);
	int get_brightness()		// Percentage:  100 = normal.
		{ return brightness; }
	void brighten(int per);		// Brighten/darken by percentage for
					//   the user.
	void restore_users_brightness();// Restore to user's setting.
	void view_right();		// Move view 1 chunk to right.
	void view_left();		// Move view left by 1 chunk.
	void view_down();		// Move view down.
	void view_up();			// Move view up.
					// Start moving actor.
	void start_actor(int winx, int winy, int speed = 125);
	void stop_actor();		// Stop main actor.
	void teleport_party(Tile_coord t);
					// Get ->party members.
	int get_party(Actor **list, int avatar_too = 0);
	void activate_item(int shnum);	// Activate item in party.
					// Find gump (x, y) is in.
	Gump *find_gump(int x, int y);
					// Find gump object is in.
	Gump *find_gump(Game_object *obj);
					// Find top object that (x,y) is in.
	Game_object *find_object(int x, int y);
	int find_objects(int lift, int x, int y, Game_object **list);
	void show_items(int x, int y);	// Show names of items clicked on.
					// Add text item.
	void add_text(const char *msg, int x, int y, Game_object *item = 0);
	void center_text(const char *msg);
	void add_effect(Special_effect *effect);
					// Remove text item & delete it.
	void remove_effect(Special_effect *txt);
	void remove_all_effects();
	void remove_text_effects();
	void remove_weather_effects();	// Remove just the weather.
	int get_weather();		// Get # of last weather added.
					// Handle a double-click in window.
	void double_clicked(int x, int y);
	void show_gump(Game_object *obj, int shapenum);
	void end_gump_mode();		// Remove gumps from screen.
					// Remove a gump from screen.
	void remove_gump(Gump *gump);
					// Add npc to 'nearby' list.
	void add_nearby_npc(Npc_actor *npc);
					// Track npcs in range of chunks.
	void add_nearby_npcs(int from_cx, int from_cy,
						int stop_cx, int stop_cy);
					// Get all nearby NPC's.
	void get_nearby_npcs(Actor_queue& list);
					// Update NPCs' schedules.
	void schedule_npcs(int hour3, int backwards = 0);
	void theft();			// Handle thievery.
	void get_focus();		// Get/lose focus.
	inline void lose_focus()
		{ focus = false; }
	inline bool have_focus() const
		{ return focus; }
	void setup_game();		// Prepare for game
	void read_npcs();		// Read in npc's.
	void write_npcs();		// Write them back.
	void read_schedules();		// Read npc's schedules.
					// Start dragging.
	bool start_dragging(int x, int y);
	bool drag(int x, int y);	// During dragging.
	bool drop_dragged(int x, int y, bool moved);// Done dragging.
					// Paint text using "fonts.vga".
	int paint_text_box(int fontnum, const char *text, int x, int y, int w, 
		int h, int vert_lead = 0, int pbreak = 0);
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
private:
	void drop(int x, int y);
public:
	bool drop_at_lift(Game_object *to_drop, int x, int y, int at_lift);
	bool init_gamedat(bool create); // Initialize gamedat directory
					// Explode a savegame into "gamedat".
	void restore_gamedat(const char *fname);
	void restore_gamedat(int num);
					// Save "gamedat".
	void save_gamedat(const char *fname, const char *savename);
	void save_gamedat(int num, const char *savename);
	int find_roof(int cx, int cy);

	void plasma(int w, int h, int x, int y, int startc, int endc);
	
	bool get_frame_skipping()	// This needs doing
	{ return true; }
	
private:
	void start_actor_alt (int winx, int winy, int speed);

	};

#endif
