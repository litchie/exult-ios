/**
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

class Font_face;
class Slist;
class Usecode_machine;
class Actor;
class Time_queue;
class Npc_proximity_handler;

/*
 *	The main game window:
 */
class Game_window
	{
	Image_window *win;		// Window to display into.
	Image_window *shapewin;		// Debug win. for showing shapes.
	Game_object shapewin_objs[20];	// List of shapes to show.
	int shapewin_cur;		// Index of current shape being shown
					//   in shapewin.
	Font_face *font12;		// 12-point "avatar.ttf" font.
public:
	enum Game_mode {		// Can be in different modes.
		intro,			// Splash screen.
		normal,			// Normal game-play.
		conversation		// Talking.
		};
private:
	Usecode_machine *usecode;	// Drives game plot.
	Game_mode mode;			// Mode we'er in.
	Time_queue *tqueue;		// Time-based queue.
	Game_clock clock;		// Keeps track of time.
	Npc_proximity_handler *npc_prox;// Handles nearby NPC's.
	Text_object *texts;		// Text snippets shown on screen.
	Rectangle npc_text_rect;	// Rectangle NPC statement is shown in.
	Rectangle *conv_choices;	// Choices during a conversation.
	unsigned char painted;		// 1 if we updated image buffer.
	unsigned char focus;		// Do we have focus?
	ifstream chunks;		// "u7chunks" file.
	Vga_file shapes;		// "shapes.vga" file.
	Vga_file faces;			// "faces.vga" file.
	Vga_file gumps;			// "gumps.vga" - open chests, bags.
	ifstream u7map;			// "u7map" file.
	Actor *main_actor;		// Main sprite to move around.
	unsigned char main_actor_inside;// 1 if actor is in a building.
	int num_npcs;			// Number of NPC's.
	Actor **npcs;			// List of NPC's + the Avatar.
					// A list of objects in each chunk.
	Chunk_object_list *objects[num_chunks][num_chunks];
	unsigned char schunk_read[144]; // Flag for reading in each "ifix".
	int chunkx, chunky;		// Chunk coord. of window within world.
	int brightness;			// Palette brightness.
					// Open a U7 file.
	int u7open(ifstream& in, char *fname, int dont_abort = 0);
public:
	int skip_lift;			// Skip objects with lift > 0.
	int debug;
	Game_window(int width = 0, int height = 0);
	~Game_window();
	void open_shape_window();	// Open window to show shapes.
	void abort(char *msg, ...);	// Fatal error.
	int get_width()
		{ return win->get_width(); }
	int get_height()
		{ return win->get_height(); }
	int get_chunkx()		// Get window offsets.
		{ return chunkx; }
	int get_chunky()
		{ return chunky; }
	Usecode_machine *get_usecode()
		{ return usecode; }
	Rectangle get_win_rect()	// Get window's rectangle.
		{ return Rectangle(0, 0, win->get_width(), win->get_height());}
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
					// Get/create objs. list for a chunk.
	Chunk_object_list *get_objects(int cx, int cy)
		{
		Chunk_object_list *list = objects[cx][cy];
		if (!list)
			list = objects[cx][cy] = new Chunk_object_list;
		return (list);
		}
	Image_window *get_shapewin()
    		{ return shapewin; }
  	Image_window *get_win(Window xwin) {
		if (shapewin && xwin == shapewin->get_win())
			return shapewin;
		else
			return win;
  		}
	Actor *get_main_actor()
		{ return main_actor; }
	int check_main_actor_inside()	// See if main actor moved in/out-side.
		{
		if (main_actor_inside != find_roof(main_actor->get_cx(),
						main_actor->get_cy()))
			{
			main_actor_inside = !main_actor_inside;
			return 1;
			}
		return 0;
		}
	Actor *get_npc(int npc_num)
		{ return npc_num < num_npcs ? npcs[npc_num] : 0; }
	int get_num_npcs()
		{ return num_npcs; }
	int get_num_shapes()
		{ return shapes.get_num_shapes(); }
	int get_num_faces()
		{ return faces.get_num_shapes(); }
	int get_num_gumps()
		{ return gumps.get_num_shapes(); }
	void set_mode(Game_mode md)
		{ mode = md; }
	Game_mode get_mode()
		{ return mode; }
					// Resize event occurred.
	void resized(Window xwin, unsigned int neww, unsigned int newh);
	void show()
		{
		if (painted)
			{
			win->show();
			painted = 0;
			}
		}
	void show(Window xwin)
		{
		if (shapewin && xwin == shapewin->get_win())
			shapewin->show();
		else
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
					// Show shape(s) clicked on.
	void debug_shape(Window xwin, int x, int y);	// DEBUGGING.
	void find_debug_shapes(int lift, int x, int y);
	void shapewin_paint(int num);
#endif
					// Get shape from shapes.vga.
	Shape_frame *get_shape(int shapenum, int framenum)
		{ return shapes.get_shape(shapenum, framenum); }
	Shape_frame *get_shape(ShapeID& id)
		{ return get_shape(id.get_shapenum(), id.get_framenum()); }
					// Get screen area used by object in
					//   given chunk.
	Rectangle get_shape_rect(Game_object *obj, int cx, int cy)
		{
		Shape_frame *s = get_shape(*obj);
		int lft = 4*obj->get_lift();
		return Rectangle(
			(cx - chunkx)*chunksize +
				obj->get_shape_pos_x()*tilesize + 
						tilesize - s->xleft - lft,
			(cy - chunky)*chunksize +
				obj->get_shape_pos_y()*tilesize + 
						tilesize - s->yabove - lft,
			s->get_width(),
			s->get_height()
			);
		}

					// Get screen area used by sprite.
	Rectangle get_shape_rect(Sprite *sprite)
		{ return get_shape_rect(sprite, sprite->get_cx(),
							sprite->get_cy()); }
					// Paint shape in window.
	void paint_shape(Image_window *iwin, int xoff, int yoff, 
							Shape_frame *shape);
	void paint_shape(Image_window *iwin,
			int xoff, int yoff, int shapenum, int framenum)
		{
		Shape_frame *shape = get_shape(shapenum, framenum);
		if (shape)
			paint_shape(iwin, xoff, yoff, shape);
		}
					// A "face" is used in conversations.
	void paint_face(Image_window *iwin, int xoff, int yoff,
						int shapenum, int framenum)
		{
		Shape_frame *shape = faces.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(iwin, xoff, yoff, shape);
		}
					// A "gump" is an open container.
	void paint_gump(Image_window *iwin, int xoff, int yoff,
						int shapenum, int framenum)
		{
		Shape_frame *shape = gumps.get_shape(shapenum, framenum);
		if (shape)
			paint_shape(iwin, xoff, yoff, shape);
		}
					// Read encoded show into window.
					// Read encoded show into window.
	void get_rle_shape(Image_window *iwin,
				Shape_frame& shape, int xoff, int yoff);
					// Get "map" superchunk objs/scenery.
	void get_map_objects(int schunk);
					// Get "chunk" objects/scenery.
	void get_chunk_objects(int cx, int cy, int chunk_num);
					// Get "ifix" objects for a superchunk.
	void get_ifix_objects(int schunk);
					// Get "ifix" objs. for given chunk.
	void get_ifix_chunk_objects(ifstream& ifix, long filepos, int cnt,
							int cx, int cy);
					// Get moveable objects.
	void get_ireg_objects(int schunk);
	void read_ireg_objects(ifstream& ireg, int scx, int scy,
					Container_game_object *container = 0);
					// Create special objects.
	Egg_object *create_egg(unsigned char *entry);
					// Get all superchunk objects.
	void get_superchunk_objects(int schunk);
	void init_actors();		// Place actors in the world.
					// Paint area of image.
	void paint(int x, int y, int w, int h);
	void paint(Rectangle& r)
		{ paint(r.x, r.y, r.w, r.h); }
	void paint()			// Paint whole image.
		{ paint(0, 0, get_width(), get_height()); }
					// Paint a bit of text.
	void paint_text(Text_object *txt);
					// Paint "flat" scenery in a chunk.
	void paint_chunk_flats(int cx, int cy, int xoff, int yoff);
					// Paint objects in given chunk at
					//   given lift.
	void paint_chunk_objects(int at_lift,
				int cx, int cy, int xoff, int yoff);
					// Get desired palette.
	void get_palette(int pal_num, int brightness = 100);
	void brighten(int per);		// Brighten/darken by percentage.
	void view_right();		// Move view 1 chunk to right.
	void view_left();		// Move view left by 1 chunk.
	void view_down();		// Move view down.
	void view_up();			// Move view up.
					// Repaint sprite after moving it.
	void repaint_sprite(Sprite *sprite, Rectangle& oldrect);
#if 0
	void animate(timeval& time);	// Do animation at given time.
#endif
					// Start moving actor.
	void start_actor(int winx, int winy);
	void stop_actor();		// Stop moving the actor.
	int find_roof(int cx, int cy);	// Find a "roof" in given chunk.
					// Find objects that (x,y) is in.
	int find_objects(int lift, int x, int y, Game_object **list);
	void show_items(int x, int y);	// Show names of items clicked on.
					// Remove text item & delete it.
	void remove_text(Text_object *txt);
					// Handle a double-click in window.
	void double_clicked(Window xwin, int x, int y);
					// Show a "face" on the screen.
	void show_face(int shape, int frame);
					// Show what NPC said.
	void show_npc_message(char *msg);
					// Show what Avatar can say.
	void show_avatar_choices(int num_choices, char **choices);
					// User clicked on a choice.
	void conversation_choice(int x, int y);
					// Queue up npcs in range of chunks.
	void add_nearby_npcs(int from_cx, int from_cy,
						int stop_cx, int stop_cy);
	void get_focus(Window xwin)	// Get/lose focus.
		{
		if (xwin == win->get_win())
			focus = 1;
		}
	void lose_focus(Window xwin)
		{
		if (xwin == win->get_win())
			focus = 0;
		}
	int have_focus()
		{ return focus; }
	void end_intro();		// End splash screen.
	void read_npcs();		// Read in npc's & schedules.
	void read_schedules();
	void write_gamedat(char *fname);// Explode a savegame into "gamedat".
	};

