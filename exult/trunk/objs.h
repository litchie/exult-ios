/**
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

#include <sys/time.h>

#else

#include <winsock.h>
#define Rectangle RECTX

#endif

class Vga_file;

/*
 *	Sizes:
 */
const int chunksize = 16 * 8;		// A chunk has 16 8x8 shapes.
const int num_chunks = 12*16;		// Total # of chunks in each dir.

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
	Game_object *next;		// ->next in chunk list.
public:
	friend class Chunk_object_list;
	Game_object(unsigned char *ifix)// Create from ifix record.
			: ShapeID(ifix[2], ifix[3]), shape_pos(ifix[0]),
			  lift(ifix[1] & 0xf)
		{  }
					// Create from map chunk.
	Game_object(unsigned char l, unsigned char h, unsigned int shapex,
				unsigned int shapey)
		: ShapeID(l, h), shape_pos((shapex << 4) + shapey), lift(0)
		{  }
					// Create from ireg. data.
	Game_object(unsigned char l, unsigned char h, unsigned int shapex,
				unsigned int shapey, unsigned int lft)
		: ShapeID(l, h), shape_pos((shapex << 4) + shapey), 
					lift(lft)
		{  }
	Game_object() : ShapeID()	// Create fake entry.
		{  }
	int get_shape_pos_x()
		{ return (shape_pos >> 4) & 0xf; }
	int get_shape_pos_y()
		{ return shape_pos & 0xf; }
	int get_lift()
		{ return lift; }
	void set_lift(int l)
		{ lift = l; }
					// Set shape coord. within chunk.
	void set_shape_pos(unsigned int shapex, unsigned int shapey)
		{ shape_pos = (shapex << 4) + shapey; }
	Game_object *get_next()
		{ return next; }
	void set_next(Game_object *obj) // Set next in list.
		{ next = obj; }
	int lt(Game_object& obj2)	// Is this less than another in pos.?
		{
		int y = get_shape_pos_y(), y2 = obj2.get_shape_pos_y();
		int l = lift, l2 = obj2.lift;
		return (l < l2 || (l == l2 &&
			(y < y2 || (y == y2 && 
				get_shape_pos_x() < obj2.get_shape_pos_x()))));
		} 
	virtual int get_usecode();	// Get usecode function to run.
	virtual char *get_name();
	virtual void set_property(int prop, int val)
		{  }
	virtual int get_property(int prop)
		{ return 0; }
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
	unsigned char roof;		// 1 if a roof present.
public:
	Chunk_object_list() : objects(0), roof(0)
		{  }
	void add(Game_object *obj);	// Add an object.
	void remove(Game_object *obj);	// Remove an object.
	void set(Game_object *objs)	// Set to (already sorted) list.
		{
		objects = objs;
		}
	int is_roof()			// Is there a roof?
		{ return roof; }
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
					// Is a spot occupied?
	int is_occupied(Vga_file& shapes, Game_object *dont_count,
					int at_lift, int at_sx, int at_sy);
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

/*
 *	Axes:
 */
enum Axis
	{
	xaxis = 0,
	yaxis = 1
	};

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
 *	Figure time passed in microseconds (so only call this for short
 *	differences).
 */
inline long Time_passed
	(
	timeval &to,
	timeval &from
	)
	{
	return to.tv_sec >= from.tv_sec ? (to.tv_sec - from.tv_sec)*1000000
						+ (to.tv_usec - from.tv_usec)
					// Watch for midnight.
			: ((24*60*60 - from.tv_sec) + to.tv_sec)*1000000
						+ (to.tv_usec - from.tv_usec);
	}

/*
 *	A sprite is a game object which can change shape and move around.
 */
class Sprite : public Game_object
	{
	int cx, cy;			// (Absolute) chunk coords.
	Chunk_object_list *chunk;	// The chunk list we're on.
	int major_frame_incr;		// # of pixels to move
					//   along major axis for each frame.
	Frames_sequence *frames[8];	// A frame sequence for each dir.
	/*
	 *	Motion info.:
	 */
	unsigned long curx, cury;	// Current pos. within world.
	Axis major_axis;		// Axis along which motion is greater.
	int major_delta, minor_delta;	// For each pixel we move along major
					//   axis, we add 'minor_delta'.  When
					//   the sum >= 'major_delta', we move
					//   1 pixel along minor axis, and
					//   subtract 'major_delta' from sum.
	int sum;			// Sum of 'minor_delta''s.
	int x_dir, y_dir;		// 1 or -1 for dir. along each axis.
	Frames_sequence *frames_seq;	// ->sequence of frames to display.
	int frame_index;		// Index into frames_seq.
	timeval last_frame_time;	// When last frame was computed.
	int frame_time;			// Time between frames in microsecs.
public:
	Sprite(int shapenum);
	int in_world()			// Do we really exist?
		{ return chunk != 0; }
	int get_cx()			// Get chunk coords.
		{ return cx; }
	int get_cy()
		{ return cy; }
	int get_worldx()		// Get x-coord. within world.
		{ return cx*chunksize + get_shape_pos_x()*8; }
	int get_worldy()		// Get y-coord. within world.
		{ return cy*chunksize + get_shape_pos_y()*8; }
					// Move to new chunk, shape coords.
	void move(int new_cx, int new_cy, Chunk_object_list *new_chunk, 
			int new_sx, int new_sy, int new_frame, 
			int new_lift = -1)
		{
		if (chunk)		// Remove from current chunk.
			chunk->remove(this);
		chunk = new_chunk;	// Add to new one.
		cx = new_cx;
		cy = new_cy;
		set_shape_pos(new_sx, new_sy);
		if (new_frame >= 0)
			set_frame(new_frame);
		if (new_lift >= 0)
			set_lift(new_lift);
		chunk->add(this);
		}
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
	int is_moving()
		{ return x_dir != 0; }
	void stop();			// Stop motion.
					// Start moving.
	void start(unsigned long destx, unsigned long desty, int speed);
					// Figure next frame location.
	virtual int next_frame(timeval& time,
		int& new_cx, int& new_cy, int& new_sx, int& new_sy,
		int& new_frame);
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
		
	};

#endif
