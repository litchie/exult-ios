/*
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


#include "Zombie.h"

/*
 *	Find path from source to destination.
 *
 *	Output:	1 if successful, else 0.
 */
int Zombie::NewPath(Tile_coord s, Tile_coord d, Pathfinder_client *)
{
	src = s;			// Store start, destination.
	dest = d;
	cur = s;			// Get current coords.
	sum = 0;			// Clear accumulator.
	long deltax = Tile_coord::delta(cur.tx, dest.tx);
	long deltay = Tile_coord::delta(cur.ty, dest.ty);
	if (!deltax && !deltay)		// Going nowhere?
		{
		major_distance = 0;
		return (0);
		}		
	unsigned int abs_deltax, abs_deltay;
	int x_dir, y_dir;
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
		major_coord = &cur.ty;
		minor_coord = &cur.tx;
		major_dir = y_dir;
		minor_dir = x_dir;
		major_delta = abs_deltay;
		minor_delta = abs_deltax;
		}
	else				// Moving faster along x?
		{
		major_coord = &cur.tx;
		minor_coord = &cur.ty;
		major_dir = x_dir;
		minor_dir = y_dir;
		major_delta = abs_deltax;
		minor_delta = abs_deltay;
		}
	major_distance = major_delta;	// How far to go.
	return (1);
}

/*
 *	Get next point on path to go to (in tile coords).
 *
 *	Output:	0 if all done.
 */
int Zombie::GetNextStep(Tile_coord& n, bool& done)
{
	if (major_distance <= 0)
		{
		done = true;
		return (0);
		}
					// Subtract from distance to go.
	major_distance -= major_frame_incr;
					// Accumulate change.
	sum += major_frame_incr * minor_delta;
					// Figure change in slower axis.
	int minor_frame_incr = sum/major_delta;
	sum = sum % major_delta;	// Remove what we used.
					// Update coords. within world.
	*major_coord += major_dir*major_frame_incr;
	*minor_coord += minor_dir*minor_frame_incr;
					// Watch for wrapping.
	*major_coord = (*major_coord + c_num_tiles)%c_num_tiles;
	*minor_coord = (*minor_coord + c_num_tiles)%c_num_tiles;
	n = cur;			// Return new tile.
					// ++++++For now, ignore tz.
	done = (major_distance <= 0);	// Indicate if this is the last one.
	return (1);
}

/*
 *	Delete.
 */
Zombie::~Zombie
	(
	)
	{
	}
