/**
 **	aniinf.h - Animation information from 'shape_info.txt'.
 **
 **	Written: 06/01/2008 - Marzo
 **/

#ifndef INCL_ANIINF_H
#define INCL_ANIINF_H	1

/*
Copyright (C) 2008 The Exult Team

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

#include "baseinf.h"
using std::istream;

class Shape_info;

/*
 *	Information about shape animations.
 */
class Animation_info : public Base_info
	{
public:
	enum AniType				// Type of animation
	{
		FA_TIMESYNCHED = 0,		// Frame based on current game ticks.
		FA_HOURLY = 1,			// Frame based on game hour.
		FA_NON_LOOPING = 2,		// Stop at last frame.
		FA_LOOPING = 3,	// Generic loop.
		FA_RANDOM_FRAMES = 4	// Frames completely random.
	};
private:
	AniType	type;
	int		frame_count;	// Frame count of each animation cycle
	int		frame_delay;	// Delay multiplier between frames.
	int		sfx_delay;		// Extra sfx delay, in frames.
	int		freeze_first;	// % chance of advancing first frame of animation.
	int		recycle;		// Where to resume animation when wrapping;
							// this is an offset from cycle start.
public:
	friend class Shape_info;
	static Animation_info *create_from_tfa(int type, int nframes);
	Animation_info()
		: Base_info()
		{  }
	Animation_info(AniType t, int count = -1, int rec = 0, int freeze = 100,
			int delay = 1, int sfxi = 0)
		{ set(t, count, rec, freeze, delay, sfxi); }
		// Read in from file.
	int read(std::istream& in, int index, int version, bool bg);
					// Write out.
	void write(std::ostream& out, int shapenum, bool bg);
	void set(AniType t, int count = -1, int rec = 0, int freeze = 100,
			int delay = 1, int sfxi = 0)
		{
		type = t;
		frame_count = count;
		frame_delay = delay;
		sfx_delay = sfxi;
		freeze_first = freeze;
		recycle = rec;
		}
	AniType get_type() const
		{ return type; }
	void set_type(AniType f)
		{
		if (type != f)
			{
			set_modified(true);
			type = f;
			}
		}
	int get_frame_count() const
		{ return frame_count; }
	void set_frame_count(int f)
		{
		if (frame_count != f)
			{
			set_modified(true);
			frame_count = f;
			}
		}
	int get_frame_delay() const
		{ return frame_delay; }
	void set_frame_delay(int f)
		{
		if (frame_delay != f)
			{
			set_modified(true);
			frame_delay = f;
			}
		}
	int get_sfx_delay() const
		{ return sfx_delay; }
	void set_sfx_delay(int f)
		{
		if (sfx_delay != f)
			{
			set_modified(true);
			sfx_delay = f;
			}
		}
	int get_freeze_first_chance() const
		{ return freeze_first; }
	void set_freeze_first_chance(int f)
		{
		if (freeze_first != f)
			{
			set_modified(true);
			freeze_first = f;
			}
		}
	int get_recycle() const
		{ return recycle; }
	void set_recycle(int f)
		{
		if (recycle != f)
			{
			set_modified(true);
			recycle = f;
			}
		}
	static int get_info_flag()
		{ return 0x40; }
	static const int get_entry_size()
		{ return -1; }
	};

#endif
