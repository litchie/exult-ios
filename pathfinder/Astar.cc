/**	-*-tab-width: 8; -*- */
#include "Astar.h"

/*
 *	Find path from source to destination.
 *
 *	Output:	1 if successful, else 0.
 */
int Astar::NewPath(int sx,int sy,int sz,int dx,int dy,int dz,
					int (*tileclassifier)(int,int,int))
{
	extern Tile_coord *Find_path(Tile_coord, Tile_coord,
					int (*tileclassifier)(int, int, int));

	delete [] path;			// Clear out old path, if there.
	path = Find_path(Tile_coord(sx, sy, sz), Tile_coord(dx, dy, dz),
						tileclassifier);
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
int Astar::GetNextStep(int &nx,int &ny, int& nz)
{
	if (path[next_index] == Tile_coord(-1, -1, -1))
		return (0);
	nx = path[next_index].tx;
	ny = path[next_index].ty;
	nz = path[next_index].tz;
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

