/**	-*-tab-width: 8; -*- */
#include "Astar.h"

/*
 *	Find path from source to destination.
 *
 *	Output:	1 if successful, else 0.
 */
int Astar::NewPath(int sx,int sy,int dx,int dy,int (*tileclassifier)(int,int))
{
	extern Tile_coord *Find_path(Tile_coord, Tile_coord);

	delete [] path;			// Clear out old path, if there.
					// +++++Want sz, dz too.
	path = Find_path(Tile_coord(sx, sy, 0), Tile_coord(dx, dy, 0));
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
int Astar::GetNextStep(int &nx,int &ny)
{
	if (path[next_index] == Tile_coord(-1, -1, -1))
		return (0);
	nx = path[next_index].tx;
	ny = path[next_index].ty;
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

