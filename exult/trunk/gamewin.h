/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Gamewin.h - X-windows Ultima7 map browser.
 **
 **	Written: 7/22/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

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

#include "imagewin.h"
#include "actors.h"
#include "vgafile.h"
#include "gameclk.h"
#include "fnames.h"

#include <string>	// STL string
#include <vector>	// STL container

class Gump_object;
class Gump_button;
class Slist;
class Usecode_machine;
class Actor;
class Time_queue;
class Npc_proximity_handler;
class Npc_face_info;

/*
 *	The main game window:
 */
class Game_window
	{
	static Game_window *game_window;// There's just one.
	Image_window *win;		// Window to display into.
public:
	enum Game_mode {		// Can be in different modes.
		splash,			// Splash screen.
		normal,			// Normal game-play.
		conversation,		// Talking.
		gump			// Showing open container(s).
		};
private:
	Usecode_machine *usecode;	// Drives game plot.
	Game_mode mode;			// Mode we're in.
	Time_queue *tqueue;		// Time-based queue.
	Game_clock clock;		// Keeps track of time.
	Npc_proximity_handler *npc_prox;// Handles nearby NPC's.
	Text_object *texts;		// Text snippets shown on screen.
	Gump_object *open_gumps;	// Open containers on screen.
	Npc_face_info *face_info[3];	// NPC's on-screen faces in convers.
	int num_faces;			// # of faces.
	int last_face_shown;		// Index of last npc face shown.
	Rectangle *conv_choices;	// Choices during a conversation.
	unsigned char painted;		// 1 if we updated image buffer.
	unsigned char focus;		// Do we have focus?
	ifstream chunks;		// "u7chunks" file.
	Shapes_vga_file shapes;		// "shapes.vga" file.
	Vga_file faces;			// "faces.vga" file.
	Vga_file gumps;			// "gumps.vga" - open chests, bags.
	Vga_file fonts;			// "fonts.vga" file.
	Vga_file sprites;		// "sprites.vga" file.
	Vga_file mainshp;
	Vga_file endshape;		
	ifstream u7map;			// "u7map" file.
	Xform_palette xforms[11];	// Transforms translucent colors
					//   0xf4 through 0xfe.
	Main_actor *main_actor;		// Main sprite to move around.
	int skip_above_actor;		// Level above actor to skip rendering.
	int num_npcs, num_npcs1;	// Numbers of NPC's, type1 NPC's.
	Actor **npcs;			// List of NPC's + the Avatar.
	int num_monsters;		// Number of monster types.
	Monster_info *monster_info;	// Array from 'monsters.dat'.
					// A list of objects in each chunk.
	Chunk_object_list *objects[num_chunks][num_chunks];
	unsigned char schunk_read[144]; // Flag for reading in each "ifix".
		// +++++Want to replace these with scrolltx, scrollty:
	int chunkx, chunky;		// Chunk coord. of window within world.
	int palette;			// Palette #.
	int brightness;			// Palette brightness.
	Rectangle dirty;		// Dirty rectangle.
	char *save_names[10];		// Names of saved games.
					// Dragging info:
	Game_object *dragging;		// What's being dragged.
	Gump_object *dragging_gump;
	Gump_button *dragging_gump_button;
					// Last mouse, paint positions:
	int dragging_mousex, dragging_mousey, dragging_paintx, dragging_painty;
	Rectangle dragging_rect;	// Rectangle to repaint.
	Image_buffer *dragging_save;	// Image below dragged object.
					// Open a U7 file.
	int u7open(ifstream& in, char *fname, int dont_abort = 0);
	void clear_world();		// Clear out world's contents.
	void read_save_names();		// Read in saved-game names.
public:
	int skip_lift;			// Skip objects with lift > 0.
	int paint_eggs;
	int debug;
	Game_window(int width = 0, int height = 0);
	~Game_window();
					// Get the one game window.
	static Game_window *get_game_window()
		{ return game_window; }
	void abort(char *msg, ...);	// Fatal error.
	int get_width()
		{ return win->get_width(); }
	int get_height()
		{ return win->get_height(); }
	int get_scrolltx()		// Get window offsets in tiles.
		{ return chunkx*tiles_per_chunk; }
	int get_scrollty()
		{ return chunky*tiles_per_chunk; }
	Usecode_machine *get_usecode()
		{ return usecode; }
	Rectangle get_win_rect()	// Get window's rectangle.
		{ return Rectangle(0, 0, win->get_width(), win->get_height());}
	Rectangle get_win_tile_rect()	// Get it in tiles, rounding up.
		{ return Rectangle(get_scrolltx(), get_scrollty(),
			(get_width() + tilesize - 1)/tilesize,
			(get_height() + tilesize - 1)/tilesize); }
					// Clip rectangle to window's.
	Rectangle clip_to_win(Rectangle r)
		{
		Rectangle wr = get_win_rect();
		return (r.intersect(wr));
		}
	Image_window *get_win()
		{ return win; }
	Time_queue *get_tqueue()
		{ return tqueue; }
	int get_hour()			// Get current time.
		{ return clock.get_hour(); }
	int get_minute()
		{ return clock.get_minute(); }
	void increment_clock(int num_minutes)
		{ clock.increment(num_minutes); }
	void fake_next_period()		// For debugging.
		{ clock.fake_next_period(); }
					// Get/create objs. list for a chunk.
	Chunk_object_list *get_objects(int cx, int cy)
		{
		Chunk_object_list *list = objects[cx][cy];
		if (!list)
			list = objects[cx][cy] = new Chunk_object_list(cx, cy);
		return (list);
		}
	Chunk_object_list *get_objects(Game_object *obj)
		{ return get_objects(obj->get_cx(), obj->get_cy()); }
	Actor *get_main_actor()
		{ return main_actor; }
	int set_above_main_actor(int inside, int lift)
		{			// Use ht=4, round up to nearest 5.
		int new_skip = !inside ? 31 : ((lift + 4 + 4)/5)*5;
		return (new_skip == skip_above_actor ? 0
				: ((skip_above_actor = new_skip), 1));
		}
	int set_above_main_actor(int lift)// Use this if chunk didn't change.
		{
		return !is_main_actor_inside() ? 0 : 
			((skip_above_actor = ((lift + 4 + 4)/5)*5), 1);
		}
	int is_main_actor_inside()
		{ return skip_above_actor < 31 ; }
	Actor *get_npc(long npc_num)
		{ return (npc_num >= 0 && npc_num < num_npcs) ? npcs[npc_num] 
									: 0; }
					// Find monster info. for shape.
	Monster_info *get_monster_info(int shapenum);
	int get_num_npcs()
		{ return num_npcs; }
	int get_num_shapes()
		{ return shapes.get_num_shapes(); }
	int get_num_faces()
		{ return faces.get_num_shapes(); }
	int get_num_gumps()
		{ return gumps.get_num_shapes(); }
	int get_num_fonts()
		{ return fonts.get_num_shapes(); }
	int get_num_sprites()
		{ return sprites.get_num_shapes(); }
	void set_mode(Game_mode md)
		{ mode = md; }
	Game_mode get_mode()
		{ return mode; }
					// Resize event occurred.
	void resized(unsigned int neww, unsigned int newh);
	void set_painted()		// Force blit.
		{ painted = 1; }
	void show()
		{
		if (painted)
			{
			win->show();
			painted = 0;
			}
		}
	void show(int)
		{
		win->show();
		}
	void set_chunk_offsets(int cx, int cy)
		{
		if (cx >= 0 && cx < num_chunks)
			chunkx = cx;
		if (cy >= 0 && cy < num_chunks)
			chunky = cy;
		}
#if 1
					// Show abs. location of mouse.
	void show_game_location(int x, int y);
#endif
	Shapes_vga_file& get_shapes()	// Get 'shapes.vga' file.
		{ return shapes; }
	Shape_info& get_info(int shnum)	// Get shape info.
		{ return shapes.get_info(shnum); }
	Shape_info& get_info(Game_object *obj)
		{ return get_info(obj->get_shapenum()); }
					// Get shape from shapes.vga.
	Shape_frame *get_shape(int shapenum, int framenum)
		{ return shapes.get_shape(shapenum, framenum); }
	Shape_frame *get_shape(ShapeID& id)
		{ return get_shape(id.get_shapenum(), id.get_framenum()); }
					// Get # frames in a shape.
	int get_shape_num_frames(int shapenum)
		{ return shapes.get_num_frames(shapenum); }
	int get_sprite_num_frames(int shapenum)
		{ return sprites.get_num_frames(shapenum); }
					// Get screen area used by object.
	Rectangle get_shape_rect(Game_object *obj)
		{
		Shape_frame *s = get_shape(*obj);
		if(!s)
			{
			// This is probably fatal.
#if DEBUG
			cerr << "DEATH! get_shape() returned a NULL pointer: " << __FILE__ << ":" << __LINE__ << endl;
			cerr << "Betcha it's a little doggie." << endl;
#endif
			return Rectangle(0,0,0,0);
			}
		int cx = obj->get_cx(), cy = obj->get_cy();
		int lft = 4*obj->get_lift();
		return Rectangle(
			(cx - chunkx)*chunksize +
				obj->get_tx()*tilesize + 
						tilesize - 1 - s->xleft - lft,
			(cy - chunky)*chunksize +
				obj->get_ty()*tilesize + 
						tilesize - 1 - s->yabove - lft,
			s->get_width(),
			s->get_height()
			);
		}
	Shape_frame *get_gump_shape(int shapenum, int framenum)
		{ return gumps.get_shape(shapenum, framenum); }
					// Get screen area of a gump.
					//   for painting it.
	Rectangle get_gump_rect(Gump_object *gump);
					// Get screen loc. of object.
	void get_shape_location(Game_object *obj, int& x, int& y)
		{
		x = (obj->get_cx() - chunkx)*chunksize +
				(1 + obj->get_tx())*tilesize - 1
						- 4*obj->get_lift();
		y = (obj->get_cy() - chunky)*chunksize +
				(1 + obj->get_ty())*tilesize - 1
						- 4*obj->get_lift();
		}
					// Paint shape in window.
	void paint_shape(int xoff, int yoff, Shape_frame *shape,
						int translucent = 0)
		{
		if (!shape)
			{
				cout << "NULL SHAPE!!!" << endl;
				return;
			}
		if (!shape->rle)	// Not RLE?
			win->copy8(shape->data, 8, 8, xoff - tilesize, 
						yoff - tilesize);
		else if (!translucent)
			paint_rle_shape(*shape, xoff, yoff);
		else
			paint_rle_shape_translucent(*shape, xoff, yoff);
		}
	void paint_shape(int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = get_shape(shapenum, framenum);
		if (shape)
			paint_shape(xoff, yoff, shape, 
				shapes.get_info(shapenum).has_translucency());
		}
					// A "face" is used in conversations.
	void paint_face(int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = faces.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(xoff, yoff, shape);
		}
					// A "gump" is an open container.
	void paint_gump(int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = gumps.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(xoff, yoff, shape);
		}
	void paint_sprite(int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = sprites.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(xoff, yoff, shape);
		}
					// Read encoded show into window.
	void paint_rle_shape(Shape_frame& shape, int xoff, int yoff);
	void paint_rle_shape_translucent(Shape_frame& shape, 
							int xoff, int yoff);
					// Get "map" superchunk objs/scenery.
	void get_map_objects(int schunk);
					// Get "chunk" objects/scenery.
	void get_chunk_objects(int cx, int cy, int chunk_num);
					// Get "ifix" objects for a superchunk.
	void get_ifix_objects(int schunk);
					// Get "ifix" objs. for given chunk.
	void get_ifix_chunk_objects(ifstream& ifix, long filepos, int cnt,
							int cx, int cy);
					// Get iregxx name.
	static char *get_ireg_name(int schunk, char *fname);
					// Write moveable objects to file.
	int write_ireg_objects(int schunk);
					// Get moveable objects.
	void get_ireg_objects(int schunk);
	void read_ireg_objects(istream& ireg, int scx, int scy,
					Game_object *container = 0);
					// Create special objects.
	Egg_object *create_egg(unsigned char *entry);
					// Get all superchunk objects.
	void get_superchunk_objects(int schunk);
	int write();			// Write out to 'gamedat'.
	int read();			// Read in 'gamedat'.
	int write_gwin();		// Write gamedat/gamewin.dat.
	int read_gwin();		// Read gamedat/gamewin.dat.
	void init_actors();		// Place actors in the world.
					// Paint area of image.
	void paint(int x, int y, int w, int h);
	void paint(Rectangle& r)
		{ paint(r.x, r.y, r.w, r.h); }
	void paint()			// Paint whole image.
		{
		paint(0, 0, get_width(), get_height());
		clear_dirty();
		}
	void clear_dirty()		// Clear dirty rectangle.
		{ dirty.w = 0; }
	void paint_dirty()		// Paint 'dirty' rectangle.
		{
		if (dirty.w > 0)
			paint(dirty);
		clear_dirty();
		}
	void add_dirty(Rectangle r)	// Add rectangle to dirty area.
		{ dirty = dirty.w > 0 ? dirty.add(r) : r; }
					// Add dirty rect. for obj.  Rets. 0
					//   if not on screen.
	int add_dirty(Game_object *obj)
		{
		Rectangle rect = get_shape_rect(obj);
		rect.enlarge(5);
		rect = clip_to_win(rect);
		if (rect.w > 0 && rect.h > 0)
			{
			add_dirty(rect);
			return 1;
			}
		else
			return 0;
		}
	char *get_save_name(int i)	// Get ->saved-game name.
		{ return save_names[i]; }
					// Paint a bit of text.
	void paint_text_object(Text_object *txt);
					// Paint "flat" scenery in a chunk.
	void paint_chunk_flats(int cx, int cy);
					// Paint objects in given chunk at
					//   given lift.
	void paint_chunk_objects(int at_lift, int cx, int cy, int flat_only);
					// Paint an obj. after dependencies.
	void paint_object(Game_object *obj, int at_lift, int flat_only);
	void set_palette(int pal_num);	// Set desired palette.
	void set_palette(char *fname, int res, int fade=0);	// Set palette from Flex
	void brighten(int per);		// Brighten/darken by percentage.
	void view_right();		// Move view 1 chunk to right.
	void view_left();		// Move view left by 1 chunk.
	void view_down();		// Move view down.
	void view_up();			// Move view up.
					// Start moving actor.
	void start_actor(int winx, int winy, int speed = 125)
		{
		main_actor->walk_to_point(chunkx*chunksize + winx, 
				chunky*chunksize + winy, speed);
		}
	void stop_actor();		// Stop main actor.
					// Find gump (x, y) is in.
	Gump_object *find_gump(int x, int y);
					// Find gump object is in.
	Gump_object *find_gump(Game_object *obj);
					// Find top object that (x,y) is in.
	Game_object *find_object(int x, int y);
	int find_objects(int lift, int x, int y, Game_object **list);
	void show_items(int x, int y);	// Show names of items clicked on.
					// Add text item.
	void add_text(const char *msg, int x, int y);
					// Remove text item & delete it.
	void remove_text(Text_object *txt);
	void remove_all_text();
					// Handle a double-click in window.
	void double_clicked(int x, int y);
	void init_faces();		// Clear out face list.
					// Show a "face" on the screen.
	void show_face(int shape, int frame);
	void remove_face(int shape);	// Remove "face" from screen.
	int get_num_faces_on_screen()	// # of faces on screen.
		{ return num_faces; }
					// Show what NPC said.
	void show_npc_message(char *msg);
	int is_npc_text_pending();	// Need to prompt user?
	void clear_text_pending();	// Don't need to prompt.
					// Show what Avatar can say.
	void show_avatar_choices(int num_choices, char **choices);
	void show_avatar_choices(vector<string> &choices);
					// User clicked on a choice.
	int conversation_choice(int x, int y);
	void show_gump(Container_game_object *obj, int shapenum);
	void end_gump_mode();		// Remove gumps from screen.
					// Remove a gump from screen.
	void remove_gump(Gump_object *gump);
					// Queue up npcs in range of chunks.
	void add_nearby_npcs(int from_cx, int from_cy,
						int stop_cx, int stop_cy);
	void schedule_npcs(int hour3);	// Update NPCs' schedules.
	void get_focus();		// Get/lose focus.
	void lose_focus()
		{ focus = 0; }
	int have_focus()
		{ return focus; }
	void end_splash();		// End splash screen.
	void read_npcs();		// Read in npc's.
	int write_npcs();		// Write them back.
	void read_schedules();		// Read npc's schedules.
					// Start dragging.
	int start_dragging(int x, int y);
	void drag(int x, int y);	// During dragging.
	void drop_dragged(int x, int y, int moved);// Done dragging.
					// Paint text using "fonts.vga".
	int paint_text_box(int fontnum, char *text, int x, int y, int w, 
				int h, int vert_lead = 0);
	int paint_text(int fontnum, const char *text, int xoff, int yoff);
	int paint_text(int fontnum, const char *text, int textlen, 
							int xoff, int yoff);
					// Get text width.
	int get_text_width(int fontnum, const char *text);
	int get_text_width(int fontnum, const char *text, int textlen);
					// Get text height, baseline.
	int get_text_height(int fontnum);
	int get_text_baseline(int fontnum);
private:
	void drop(int x, int y);
	int drop_at_lift(int at_lift);
public:
	void restore_gamedat(char *fname);// Explode a savegame into "gamedat".
	void restore_gamedat(int num);
					// Save "gamedat".
	int save_gamedat(char *fname, char *savename);
	int save_gamedat(int num, char *savename);
	char *get_game_identity(char *fname);
	char *get_shape_file_name(int n);
	Vga_file *get_shape_file_data(int n);
	int get_shape_file_count();
	void play_flic(char *archive, int index);
	void paint_splash();
	void end_game();
	int find_roof(int cx, int cy);
	};

