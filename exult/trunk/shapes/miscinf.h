/*
 *  miscinf.h - Information about several previously-hardcoded shape data.
 *
 *  Copyright (C) 2006  The Exult Team
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

#ifndef MISCINF_H
#define MISCINF_H 1

/*
 *	A class to get the extra information for a given shape.
 */
class Shapeinfo_lookup
	{
	static void setup();
public:
	struct Animation_info
	{
		int		shapenum;		// The shape number
		int		type;			// Type of animation; one of FA_LOOPING,
								// FA_SUNDIAL or FA_ENERGY_FIELD
		int		first_frame;	// First frame of the animation cycle
		int		frame_count;	// Frame count of the animation cycle
		int		offset;			// Overall phase shift of the animation
		int		offset_type;	// If 1, set offset = init_frame % frame_count
								// If -1, set offset = init_frame
	};
	static int get_explosion_sprite (int shapenum);
	static int get_shape_sfx (int shapenum);
	static Animation_info *get_animation_cycle_info (int shapenum, int init_frame);
	static bool get_usecode_events (int shapenum);
	static bool get_mountain_top (int shapenum);
	};
#endif
