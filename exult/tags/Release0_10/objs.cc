/**
 **	Objs.cc - Game objects.
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

#include "objs.h"
#include "vgafile.h"

#if !(defined(XWIN) || defined(DOS)) 	// WIN32
void gettimeofday(timeval* tv, int x) {
  _SYSTEMTIME ST;

  GetSystemTime(&ST);
  tv->tv_sec = ST.wSecond + 60* (ST.wMinute + 60 * ST.wHour);
  tv->tv_usec = ST.wMilliseconds * 1000; //can't get Microseconds (yet)
}
#endif

/*
 *	The default usecode function for a shape is the shape # (I think).
 */

int Game_object::get_usecode
	(
	)
	{
	return (get_shapenum());
	}

/*
 *	Get name.
 */

char *Game_object::get_name
	(
	)
	{
	extern char *item_names[];
	return item_names[get_shapenum()];
	}

/*
 *	Add a game object to a chunk's list.
 */

void Chunk_object_list::add
	(
	Game_object *newobj		// Object to add.
	)
	{
					// Get x,y of shape within chunk.
	int x = newobj->get_shape_pos_x(), y = newobj->get_shape_pos_y();
	int num_entries = 0;		// Need to count as we sort.
	Game_object *obj;
	Game_object *prev = 0;
	for (obj = objects; obj && !newobj->lt(*obj); obj = obj->next)
		prev = obj;
	if (!prev)			// Goes in front?
		{
		newobj->next = objects;
		objects = newobj;
		}
	else
		{
		newobj->next = prev->next;
		prev->next = newobj;
		}
	if (newobj->get_lift() >= 5)	// Looks like a roof?
		roof = 1;
	}

/*
 *	Remove a game object from this list.
 */

void Chunk_object_list::remove
	(
	Game_object *remove
	)
	{
	if (remove == objects)		// First one?
		{
		objects = remove->next;
		return;
		}
	Game_object *obj;		// Got to find it.
	for (obj = objects; obj && obj->next != remove; obj = obj->next)
		;
	if (obj)			// This is before it.
		obj->next = remove->next;
	}

/*
 *	Is a given square occupied at a given lift?
 *
 *	Output: 1 if so, else 0.
 */

int Chunk_object_list::is_occupied
	(
	Vga_file& shapes,		// For getting dims.  (May go away).
	Game_object *dont_count,	// But don't count this object.
	int at_lift,			// Given lift.
	int at_sx, int at_sy		// Square to test.
	)
	{
#if 1			/* ++++++Until we get the right footprint dims. */
	return (0);
#else
	Game_object *obj;		// Just run through them all.
					// (Could be faster!!!)
	for (obj = objects; obj; obj = obj->next)
		{
		if (obj == dont_count || obj->lift != at_lift)
			continue;
		int shnum = obj->get_shapenum();
		if (!shapes.is_obstacle(shnum))
			continue;	// Can walk over it.
		int y = obj->get_shape_pos_y();
		if (y < at_sy)
			continue;
		int x = obj->get_shape_pos_x();
		if (x < at_sx)
			continue;
		int w, h;		// Get dims.
		shapes.get_dim(shnum, w, h);
		if (y - h < at_sy && x - w < at_sx)
			return (1);
		}
	return (0);
#endif
	}

/*
 *	Create a sequence of frames.
 */

Frames_sequence::Frames_sequence
	(
	int cnt,			// # of frames.
	unsigned char *f		// List of frames.
	)
	{
	frames = new unsigned char[cnt + 1];
	memcpy(frames, f, cnt);		// Copy in the list.
	frames[cnt] = 0xff;		// Terminate it.
	}

/*
 *	Create a moveable sprite.
 */

Sprite::Sprite
	(
	int shapenum
	)  : Game_object(), cx(-1), cy(-1), chunk(0),
		x_dir(0), major_frame_incr(8), frames_seq(0)
	{
	set_shape(shapenum, 0); 
	for (int i = 0; i < 8; i++)
		frames[i] = 0;
	}

/*
 *	Stop moving.
 */

void Sprite::stop
	(
	)
	{
	x_dir = 0;
	if (frames_seq)			// Set to "resting" frame.
		set_frame(frames_seq->get_resting());
	}

/*
 *	Start moving.
 */

void Sprite::start
	(
	unsigned long destx,		// Move towards pt. within world.
	unsigned long desty,
	int speed			// # microsecs. between frames.
	)
	{
	if (!in_world())
		return;			// We can't start moving.
	frame_time = speed;
	Direction dir;			// Gets compass direction.++++++Get
					//  northeast, etc. too.
	if (!is_moving())		// Not already moving?
					// Store current time.
		gettimeofday(&last_frame_time, 0);
	curx = get_worldx();		// Get current coords.
	cury = get_worldy();
	sum = 0;			// Clear accumulator.
	long deltax = destx - curx;	// Get changes.
	long deltay = desty - cury;
	unsigned long abs_deltax, abs_deltay;
	if (deltay >= 0)		// Figure directions.
		{
		y_dir = 1;
		abs_deltay = deltay;
		}
	else
		{
		y_dir = -1;
		abs_deltay = -deltay;
		}
	if (deltax >= 0)
		{
		x_dir = 1;
		abs_deltax = deltax;
		}
	else
		{
		x_dir = -1;
		abs_deltax = -deltax;
		}
	if (abs_deltay >= abs_deltax)	// Moving faster along y?
		{
		dir = y_dir > 0 ? south : north;
		major_axis = yaxis;
		major_delta = abs_deltay;
		minor_delta = abs_deltax;
		}
	else				// Moving faster along x?
		{
		dir = x_dir > 0 ? east : west;
		major_axis = xaxis;
		major_delta = abs_deltax;
		minor_delta = abs_deltay;
		}
					// Different dir. than before?
	if (frames[(int) dir] != frames_seq)
		{			// Set frames sequence.
		frames_seq = frames[(int) dir];
		frame_index = -1;
		}
	}

/*
 *	Figure where the sprite will be in the next frame.
 *
 *	Output: 0 if don't need to move.
 */

int Sprite::next_frame
	(
	timeval& time,			// Current time.
	int& new_cx, int& new_cy,	// New chunk coords. returned.
	int& new_sx, int& new_sy,	// New shape coords. returned.
	int& next_frame			// Next frame # returned.
	)
	{
	if (!is_moving())
		return (0);
	long since = Time_passed(time, last_frame_time);
	if (since < frame_time)		// Not time yet?
		return (0);
	last_frame_time = time;		// Store this time.
					// Figure change in faster axis.
	int new_major = major_frame_incr;
					// Accumulate change.
	sum += major_frame_incr * minor_delta;
					// Figure change in slower axis.
	int new_minor = sum/major_delta;
	sum = sum % major_delta;	// Remove what we used.
	if (major_axis == xaxis)	// Which axis?
		{
		curx += x_dir*new_major;
		cury += y_dir*new_minor;
		}
	else
		{
		cury += y_dir*new_major;
		curx += x_dir*new_minor;
		}
	new_cx = curx/chunksize;	// Return new chunk pos.
	new_cy = cury/chunksize;
	new_sx = (curx%chunksize)/8;
	new_sy = (cury%chunksize)/8;
	if (frames_seq)			// Got a sequence of frames?
		next_frame = frames_seq->get_next(frame_index);
	else
		next_frame = -1;
	return (1);
	}

#if 0
/*
 *	Lookup arctangent in a table for degrees 5-85.
 */

static unsigned Lookup_atan
	(
	unsigned dydx
	)
	{
					// 1024*tan(x), where x ranges from
					//   5 deg to 85.
	static unsigned tans[17] = {90, 181, 274, 373, 477, 591, 717, 859,
			1024, 1220, 1462, 1774, 2196, 2813, 3822, 5807, 11704};
++++++++++++++++++++++++++++++++++++

/*
 *	Return the arctangent, in 5-degree increments as an angle counter-
 *	clockwise from the east.
 *
 *	Output: Arctangent/5 degrees.
 */

unsigned Arctangent
	(
	int deltay,
	int deltax
	)
	{
	if (!deltax)			// Vertical?
		return (deltax > 0 ? 90/5 : 270/5);
	if (deltay >= 0)
		if (deltax >= 0)	// Top-right quadrant?
			{
#endif
