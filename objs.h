/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objs.h - Game objects.
 **
 **	Written: 10/1/98 - JSF
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

#ifndef INCL_OBJS
#define INCL_OBJS	1

#if (defined(DOS) || defined(XWIN))

#else

#define Rectangle RECTX

#endif

#include "vec.h"
#include "usecode.h"
#include "tqueue.h"

class Vga_file;
class Game_window;
class Npc_actor;
class Rectangle;
class Container_game_object;

/*
 *	Sizes:
 */
const int tilesize = 8;			// A tile (shape) is 8x8 pixels.
const int tiles_per_chunk = 16;		// A chunk is 16x16 tiles.
const int chunksize = 16 * 8;		// A chunk has 16 8x8 shapes.
const int num_chunks = 12*16;		// Total # of chunks in each dir.
					// Total # tiles in each dir.:
const int num_tiles = tiles_per_chunk*num_chunks;

/*
 *	A 3D tile coordinate:
 */
class Tile_coord
	{
public:
	short tx, ty, tz;		// Coords. within world. tz=lift.
	Tile_coord(int x, int y, int z) : tx(x), ty(y), tz(z)
		{  }
	};
					// Add two coords.
inline Tile_coord operator+(Tile_coord a, Tile_coord b)
	{ return Tile_coord(a.tx + b.tx, a.ty + b.ty, a.tz + b.tz); }

/*
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */
class ShapeID
	{
	unsigned char low, high;
public:
	ShapeID(unsigned char l, unsigned char h) : low(l), high(h)
		{  }
	ShapeID(unsigned char *& data)	// Read from buffer & incr. ptr.
		{
		low = *data++;
		high = *data++;
		}
					// Create "end-of-list"/invalid entry.
	ShapeID() : low(0xff), high(0xff)
		{  }
	int is_invalid()		// End-of-list or invalid?
		{ return (low == 0xff && high == 0xff); }
	int is_eol()
		{ return is_invalid(); }
	int get_shapenum()
		{ return low + 256*(high&0x3); }
	int get_framenum()
		{ return (high >> 2) & 0x1f; }
					// Set to given shape.
	void set_shape(int shapenum, int framenum)
		{
		low = shapenum & 0xff;
		high = ((shapenum >> 8) & 3) | (framenum << 2);
		}
	ShapeID(int shapenum, int framenum)
		{ set_shape(shapenum, framenum); }
	void set_shape(int shapenum)	// Set shape, but keep old frame #.
		{
		low = shapenum & 0xff;
		high = ((shapenum >> 8) & 3) | (high & ~3);
		}
	void set_frame(int framenum)	// Set to new frame.
		{ high = (high & 3) | (framenum << 2); }
	};

/*
 *	A game object is a shape from shapes.vga along with info. about its
 *	position within its chunk.
 */
class Game_object : public ShapeID
	{
	unsigned char shape_pos;	// (X,Y) of shape within chunk.
	unsigned char lift;		// Raise by 4* this number.
	short quality;			// Some sort of game attribute.
	Game_object *next;		// ->next in chunk list or container.
	Vector dependencies;		// Objects which must be painted before
					//   this can be rendered.
public:	//++++++Testing.  Maybe replace with a seq. number.
	unsigned char rendered;		// 1 when rendered.
protected:
	unsigned char cx, cy;		// (Absolute) chunk coords., or if this
					//   is in a container, coords. within
					//   gump's rectangle.
public:
	friend class Chunk_object_list;
	friend class Barge_object;
	friend class Gump_object;
	friend class Actor_gump_object;
					// Create from ifix record.
	Game_object(unsigned char *ifix)
			: ShapeID(ifix[2], ifix[3]), shape_pos(ifix[0]),
			  lift(ifix[1] & 0xf), quality(0), cx(255), cy(255)
		{  }
					// Create from ireg. data.
	Game_object(unsigned char l, unsigned char h, unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: ShapeID(l, h), shape_pos((shapex << 4) + shapey), 
					lift(lft), quality(0), cx(255), cy(255)
		{  }
	Game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: ShapeID(shapenum, framenum),
				shape_pos((tilex << 4) + tiley),
				lift(lft), quality(0), cx(255), cy(255)
		{  }
					// Copy constructor.
	Game_object(Game_object& obj2)
		: ShapeID(obj2), shape_pos(obj2.shape_pos), lift(obj2.lift),
		  quality(obj2.quality), cx(obj2.cx), cy(obj2.cy)
		{  }
	Game_object() : ShapeID()	// Create fake entry.
		{  }
	virtual ~Game_object()
		{  }
	int get_tx()			// Get tile (0-15) within chunk.
		{ return (shape_pos >> 4) & 0xf; }
	int get_ty()
		{ return shape_pos & 0xf; }
	int get_lift()
		{ return lift; }
	int get_worldx()		// Get x-coord. within world.
		{ return cx*chunksize + get_tx()*tilesize; }
	int get_worldy()		// Get y-coord. within world.
		{ return cy*chunksize + get_ty()*tilesize; }
					// Get location in abs. tiles.
	void get_abs_tile(int& atx, int& aty, int& atz)
		{
		atz = get_lift();
		atx = cx*tiles_per_chunk + get_tx();
		aty = cy*tiles_per_chunk + get_ty();
		}
	Tile_coord get_abs_tile_coord()	// Same thing.
		{
		int x, y, z;
		get_abs_tile(x, y, z);
		return Tile_coord(x, y, z);
		}
	int get_quality()
		{ return quality; }
	void set_quality(int q)
		{ quality = q; }
	int get_quantity();		// Like # of coins.
	int get_volume();		// Get space taken.
					// Add/remove to/from quantity.
	int modify_quantity(int delta);
					// Set shape coord. within chunk.
	void set_shape_pos(unsigned int shapex, unsigned int shapey)
		{ shape_pos = (shapex << 4) + shapey; }
	void set_lift(int l)
		{ lift = l; }
	Game_object *get_next()
		{ return next; }
	void set_next(Game_object *obj) // Set next in list.
		{ next = obj; }
	int lt(Game_object& obj2);	// Is this less than another in pos.?
					// Return chunk coords.
	int get_cx()
		{ return cx; }
	int get_cy()
		{ return cy; }
					// Move to new abs. location.
	void move(int newtx, int newty, int newlift);
	int get_dependency_count()	// Get objs. to paint first.
		{ return dependencies.get_cnt(); }
	Game_object *get_dependency(int i)
		{ return (Game_object *) dependencies.get(i); }
					// Remove a dependency.
	void remove_dependency(Game_object *obj2)
		{
		int i = dependencies.find(obj2);
		if (i >= 0)
			dependencies.put(i, 0);
		}
	void clear_dependencies();	// Remove all dependencies.
					// Find nearby objects.
	int find_nearby(Vector& vec, int shapenum, int quality, int mask);
					// Render.
	virtual void paint(Game_window *gwin);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
					// Set new NPC schedule.
	virtual void set_schedule_type(int new_schedule_type)
		{  }
	virtual int get_schedule_type()	// Return NPC schedule.
		{ return 11; }		// Loiter.
	virtual char *get_name();
	virtual Game_object *clone()	// Create a copy.
		{ return new Game_object(*this); }
	virtual void remove();		// Remove/delete this object.
	virtual void set_property(int prop, int val)
		{  }
	virtual int get_property(int prop)
		{ return 0; }
	virtual Container_game_object *get_owner()
		{ return 0; }
	virtual void set_owner(Container_game_object *o)
		{  }
	virtual int is_dragable();	// Can this be dragged?
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Set/clear/get actor flag.
	virtual void set_flag(int flag) { }
	virtual void clear_flag(int flag) { }
	virtual int get_flag(int flag) { return 0; }
	virtual int get_npc_num()	// Get its ID (1-num_npcs).
		{ return 0; }
	virtual int is_egg()		// An egg?
		{ return 0; }
					// Count contained objs.
	virtual int count_objects(int shapenum, int framenum = -359)
		{ return 0; }
					// Get contained objs.
	virtual int get_objects(Vector& vec, int shapenum, 
						int framenum = -359)
		{ return 0; }
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0)
		{ return 0; }
	virtual int add_quantity(int delta, int shapenum, int qual,
					int framenum, int dontcreate = 0)
		{ return delta; }
	virtual int create_quantity(int delta, int shapenum, int qual,
								int framenum)
		{ return delta; }
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum)
		{ return delta; }
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord()
		{ return get_abs_tile_coord(); }
	};

/*
 *	A moveable game object (from 'ireg' files):
 */
class Ireg_game_object : public Game_object
	{
	Container_game_object *owner;	// Container this is in, or 0.
public:
					// Create from ireg. data.
	Ireg_game_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Game_object(l, h, shapex, shapey, lft), owner(0)
		{  }
	Ireg_game_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft),
						owner(0)
		{  }
					// Copy constructor.
	Ireg_game_object(Ireg_game_object& obj2)
		: Game_object(obj2), owner(0)
		{  }
	Ireg_game_object() : owner(0)	// Create fake entry.
		{  }
	virtual Game_object *clone()	// Create a copy.
		{ return new Ireg_game_object(*this); }
	virtual void remove();		// Remove/delete this object.
	virtual Container_game_object *get_owner()
		{ return owner; }
	virtual void set_owner(Container_game_object *o)
		{ owner = o; }
	virtual int is_dragable();	// Can this be dragged?
	};

/*
 *	A container object:
 */
class Container_game_object : public Ireg_game_object
	{
	int volume_used;		// Amount of volume occupied.
protected:
	Game_object *last_object;	// ->last obj., which pts. to first.
public:
	Container_game_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Ireg_game_object(l, h, shapex, shapey, lft),
		  last_object(0), volume_used(0)
		{  }
	Container_game_object() : last_object(0), volume_used(0) {  }
	void remove(Game_object *obj);
	Game_object *get_last_object()
		{ return last_object; }
	Game_object *get_first_object()	// Get first inside.
		{ return last_object ? last_object->get_next() : 0; }
					// For when an obj's quantity changes:
	void modify_volume_used(int delta)
		{ volume_used += delta; }
	int has_room(Game_object *obj)	// Room for this object?
		{ return obj->get_volume() + volume_used <= get_volume(); }
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Add/remove quantities of objs.
	virtual int add_quantity(int delta, int shapenum, int qual,
					int framenum, int dontcreate = 0);
	virtual int create_quantity(int delta, int shapenum, int qual,
							int framenum);
	virtual int remove_quantity(int delta, int shapenum, int qual,
								int framenum);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Count contained objs.
	virtual int count_objects(int shapenum, int framenum = -359);
					// Get contained objs.
	virtual int get_objects(Vector& vec, int shapenum, 
							int framenum = -359);
	};

/*
 *	A 'barge', such as a ship or horse-and-cart.  +++++For now, the
 *	objects aren't really added or kept track of, since they have to
 *	be rendered in the outside world.+++++
 */
class Barge_object : public Ireg_game_object
	{
public:
	Barge_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Ireg_game_object(l, h, shapex, shapey, lft)
		{  }
#if 0
	Barge_object() : Ireg_game_object()
		{  }
#endif
					// Add an object.
	virtual int add(Game_object *obj, int dont_check = 0);
					// Render.
	virtual void paint(Game_window *gwin);
	};

/*
 *	An object that cycles through its frames, or wiggles if just one
 *	frame.
 */
class Animated_object : public Game_object, public Time_sensitive
	{
	unsigned char frames;		// # of frames.
	unsigned char animating;	// 1 if animation turned on.
	unsigned char deltax, deltay;	// If wiggling, deltas from
					//   original position.
public:
	Animated_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0);
	Animated_object(int shapenum, int framenum, unsigned int tilex, 
				unsigned int tiley, unsigned int lft = 0);
	virtual ~Animated_object();
					// Render.
	virtual void paint(Game_window *gwin);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
					// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord()
		{ return get_abs_tile_coord() + 
					Tile_coord(-deltax, -deltay, 0); }
	};

/*
 *	An "egg" is a special object that activates under certain
 *	circumstances.
 */
class Egg_object : public Game_object
	{
	unsigned char type;		// One of the below types.
	unsigned char probability;	// 1-100, chance of egg activating.
	unsigned char criteria;		// Don't know this one.
	unsigned char distance;		// Distance for activation (0-31).
	unsigned char flags;		// Formed from below flags.
	unsigned short data1, data2;	// More data, depending on type.
public:
	enum Egg_types {		// Types of eggs:
		monster = 1,
		jukebox = 2,
		soundsfx = 3,
		voice = 4,
		usecode = 5,
		missile = 6,
		teleport = 7,
		weather = 8,
		path = 9,
		button = 10
		};
	enum Egg_flag_shifts {
		nocturnal = 0,
		once = 1,
		hatched = 2,
		auto_reset = 3
		};
					// Create from ireg. data.
	Egg_object(unsigned char l, unsigned char h, unsigned int shapex,
		unsigned int shapey, unsigned int lft, 
		unsigned short itype,
		unsigned char prob, short d1, short d2);
	int get_distance()
		{ return distance; }
	int is_active()			// Can it be activated?
		{ return !(flags & (1 << (int) hatched)); }
	int within_distance(int abs_tx, int abs_ty);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	virtual int is_egg()		// An egg?
		{ return 1; }
	};

/*
 *	Data cached for a chunk to speed up processing, but which doesn't need
 *	to be saved to disk:
 */
class Chunk_cache
	{
	unsigned char setup_done;	// Already setup.
	unsigned short blocked[256];	// For each tile, a bit for each lift
					//   level if it's blocked by an obj.
					// In the process of implementing:+++++
	Vector egg_objects;		// ->eggs which influence this chunk.
	unsigned short eggs[256];	// Bit #i (0-14) set means that the
					//   tile is within egg_object[i]'s
					//   influence.  Bit 15 means it's 1 or
					//   more of 
					//   egg_objects[15-(num_eggs-1)].
	friend class Chunk_object_list;
	Chunk_cache();			// Create empty one.
	~Chunk_cache();
	int get_num_eggs()
		{ return egg_objects.get_cnt(); }
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, int set);
					// Add/remove object.
	void update_object(Chunk_object_list *chunk,
						Game_object *obj, int add);
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles);
					// Add egg.
	void add_egg(Chunk_object_list *chunk, Egg_object *egg);
					// Set up with chunk's data.
	void setup(Chunk_object_list *chunk);
					// Set blocked tile's bits.
	void set_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] |= 
						((1 << ztiles) - 1) << lift;
		}
					// Clear blocked tile's bits.
	void clear_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] &= 
					~(((1 << ztiles) - 1) << lift);
		}
					// Is a spot occupied?
	int is_blocked(int lift, int tx, int ty, int& new_lift);
					// Activate eggs nearby.
	void activate_eggs(Chunk_object_list *chunk,
				int tx, int ty, unsigned short eggbits);
	void activate_eggs(Chunk_object_list *chunk, int tx, int ty)
		{
		unsigned short eggbits = eggs[ty*tiles_per_chunk + tx];
		if (eggbits)
			activate_eggs(chunk, tx, ty, eggbits);
		}
	};

/*
 *	Game objects are stored in a list for each chunk, sorted from top-to-
 *	bottom, left-to-right.
 */
class Chunk_object_list
	{
	ShapeID flats[256];		// Flat (non-RLE) shapes.  The unused
					//   are "invalid" entries.
	Game_object *objects;		// ->first in ordered list.
	Npc_actor *npcs;		// List of NPC's in this chunk.
					//   (Managed by Npc_actor class.)
	Chunk_cache *cache;		// Data for chunks near player.
	unsigned char roof;		// 1 if a roof present.
	unsigned char cx, cy;		// Absolute chunk coords. of this.
public:
	friend class Npc_actor;
	Chunk_object_list(int chunkx, int chunky);
	void add(Game_object *obj);	// Add an object.
	void add_egg(Egg_object *egg);	// Add an egg.
	void remove(Game_object *obj);	// Remove an object.
	int is_roof()			// Is there a roof?
		{ return roof; }
	int get_cx()
		{ return cx; }
	int get_cy()
		{ return cy; }
	Npc_actor *get_npcs()		// Get ->first npc in chunk.
		{ return npcs; }
					// Set/get flat shape.
	void set_flat(int shapex, int shapey, ShapeID id)
		{ flats[16*shapey + shapex] = id; }
	ShapeID get_flat(int shapex, int shapey)
		{ return flats[16*shapey + shapex]; }
	Game_object *get_first()	// Return first object.
		{ return objects; }
					// Return next object.
	Game_object *get_next(Game_object *obj)
		{ return obj->next; }
					// Get/create cache.
	Chunk_cache *need_cache()
		{ 
		return cache ? cache 
				: (cache = new Chunk_cache()); 
		}
	void setup_cache()
		{ 
		if (!cache || !cache->setup_done)
			need_cache()->setup(this);
		}
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, int set)
		{ need_cache()->set_blocked(startx, starty, endx, endy,
							lift, ztiles, set); }
					// Is a spot occupied?
	int is_blocked(int lift, int tx, int ty, int& new_lift)
		{ return cache->is_blocked(lift, tx, ty, new_lift); }
					// Set area within egg's influence.
	void set_egged(Egg_object *egg, Rectangle& tiles)
		{ need_cache()->set_egged(egg, tiles); }
	void activate_eggs(int tx, int ty)
		{ need_cache()->activate_eggs(this, tx, ty); }
	};

/*
 *	Directions:
 */
enum Direction
	{
	east = 0,
	northeast = 1,
	north = 2,
	northwest = 3,
	west = 4,
	southwest = 5,
	south = 6,
	southeast = 7
	};

Direction Get_direction
	(
	int deltay,
	int deltax
	);

/*
 *	A sequence of frames.  Frame 0 is the resting state.
 */
class Frames_sequence
	{
	unsigned char *frames;		// Last one is 0xff.
public:
	Frames_sequence(int cnt, unsigned char *f);
	~Frames_sequence()
		{ delete frames; }
	unsigned char get_resting()	// Get resting frame.
		{ return frames[0]; }
					// Get next frame.  Call
					//   with index = 0 for first one.
	unsigned char get_next(int& index)
		{
		unsigned char frame = frames[++index];
		return frame == 0xff ? frames[index = 1] : frame;
		}
	};

/*
 *	A sprite is a game object which can change shape and move around.
 *	++++++So far, this base class is never invoked, so we might not need
 *	the Time_sensitive part.+++++++
 */
class Sprite : public Container_game_object, public Time_sensitive
	{
	Chunk_object_list *chunk;	// The chunk list we're on.
	int major_frame_incr;		// # of pixels to move
					//   along major axis for each frame.
	Frames_sequence *frames[8];	// A frame sequence for each dir.
	/*
	 *	Motion info.:
	 */
	unsigned long curx, cury;	// Current pos. within world.
					// ->'s to curx, cury.
	unsigned long *major_coord, *minor_coord;
	int major_dir, minor_dir;	// 1 or -1 for dir. along each axis.
	int major_delta, minor_delta;	// For each pixel we move along major
					//   axis, we add 'minor_delta'.  When
					//   the sum >= 'major_delta', we move
					//   1 pixel along minor axis, and
					//   subtract 'major_delta' from sum.
	int sum;			// Sum of 'minor_delta''s.
	Frames_sequence *frames_seq;	// ->sequence of frames to display.
	int frame_index;		// Index into frames_seq.
	int major_distance;		// Distance in pixels to go.
protected:
	int frame_time;			// Time between frames in msecs.
public:
	Sprite(int shapenum);
	int in_world()			// Do we really exist?
		{ return chunk != 0; }
					// Set a frame seq. for a direction.
	void set_frame_sequence(Direction dir, int cnt, unsigned char *seq)
		{
		delete frames[(int) dir];
		frames[(int) dir] = new Frames_sequence(cnt, seq);
		}
					// Get resting frame for given dir.
	int get_resting_frame(Direction dir)
		{
		Frames_sequence *seq = frames[(int) dir];
		return seq != 0 ? seq->get_resting() : -1;
		}
					// Move to new chunk, shape coords.
	void move(int new_cx, int new_cy, Chunk_object_list *new_chunk, 
			int new_sx, int new_sy, int new_frame, 
			int new_lift = -1)
		{
		if (chunk)		// Remove from current chunk.
			chunk->remove(this);
		chunk = new_chunk;	// Add to new one.
		set_shape_pos(new_sx, new_sy);
		if (new_frame >= 0)
			set_frame(new_frame);
		if (new_lift >= 0)
			set_lift(new_lift);
		chunk->add(this);
		}
	int is_moving()
		{ return major_dir != 0; }
	void stop();			// Stop motion.
					// Start moving.
	void start(unsigned long destx, unsigned long desty, int speed);
	int at_destination()		// Reached/passed dest. in start()?
		{ return major_distance <= 0; }
	virtual int is_dragable();	// Can this be dragged?
					// Figure next frame location.
	int next_frame(unsigned long time,
		int& new_cx, int& new_cy, int& new_tx, int& new_ty,
		int& new_frame);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};
/*
 *	A rectangle:
 */
class Rectangle
	{
public:					// Let's make it all public.
	int x, y;			// Position.
	int w, h;			// Dimensions.
	Rectangle(int xin, int yin, int win, int hin)
			: x(xin), y(yin), w(win), h(hin)
		{  }
	Rectangle() { }			// An uninitialized one.
	int has_point(int px, int py)	// Is this point in it?
		{ return (px >= x && px < x + w && py >= y && py < y + h); }
	Rectangle add(Rectangle& r2)	// Add another to this one to get
		{			//  a rect. that encloses both.
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x < r2.x ? x : r2.x;
		r.y = y < r2.y ? y : r2.y;
		r.w = (xend > xend2 ? xend : xend2) - r.x;
		r.h = (yend > yend2 ? yend : yend2) - r.y;
		return (r);
		}
					// Intersect another with this.
	Rectangle intersect(Rectangle& r2)
		{
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x >= r2.x ? x : r2.x;
		r.y = y >= r2.y ? y : r2.y;
		r.w = (xend <= xend2 ? xend : xend2) - r.x;
		r.h = (yend <= yend2 ? yend : yend2) - r.y;
		return (r);
		}
	int intersects(Rectangle r2)	// Does it intersect another?
		{
		return (x >= r2.x + r2.w ? 0 : r2.x >= x + w ? 0 :
			y >= r2.y + r2.h ? 0 : r2.y >= y + h ? 0 : 1);
		}
	void shift(int deltax, int deltay)
		{
		x += deltax;
		y += deltay;
		}		
	void enlarge(int delta)		// Add delta in each dir.
		{ x -= delta; y -= delta; w += 2*delta; h += 2*delta; }
	};

/*
 *	A text object is a message that stays on the screen for just a couple
 *	of seconds.  These are all kept in a single list, and managed by
 *	Game_window.
 */
class Text_object : public Time_sensitive
	{
	Text_object *next, *prev;	// All of them are chained together.
	char *msg;			// What to print.
	int cx, cy;			// Chunk coords of upper-left corner.
	unsigned char sx, sy;		// Tile coords. within chunk.
	short width, height;		// Dimensions of rectangle.
public:
	friend class Game_window;
	Text_object(const char *m, int c_x, int c_y, 
					int s_x, int s_y, int w, int h);
	virtual ~Text_object()
		{ delete msg; }
					// At timeout, remove from screen.
	virtual void handle_event(unsigned long curtime, long udata);
	};

#endif
