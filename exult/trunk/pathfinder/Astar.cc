/**	-*-tab-width: 8; -*-
Copyright (C) 2000  Dancer A.L Vesperman, J.S. Freedman

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

#include "Astar.h"

/*
 *	Find path from source to destination.
 *
 *	Output:	1 if successful, else 0.
 */
int Astar::NewPath(Tile_coord s, Tile_coord d,
					int (*tileclassifier)(int,int,int&))
{
	extern Tile_coord *Find_path(Tile_coord, Tile_coord,
					int (*tileclassifier)(int, int, int&));
	src = s;			// Store start, destination.
	dest = d;
	delete [] path;			// Clear out old path, if there.
	path = Find_path(s, d, tileclassifier);
	next_index = 0;
	if (path != 0)			// Failed?  Put in fake.
		{
		path = new Tile_coord(-1, -1, -1);
		return (0);
		}
	return (1);
}

/*
 *	Get next point on path to go to (in tile coords).
 *
 *	Output:	0 if all done.
 */
int Astar::GetNextStep(Tile_coord& n)
{
	if (path[next_index] == Tile_coord(-1, -1, -1))
		return (0);
	n = path[next_index];
	next_index++;
	return 1;
}

/*
 *	Delete.
 */
Astar::~Astar
	(
	)
	{
	delete [] path;
	}

