#ifndef	__Astar_h_
#define	__Astar_h_

#include "PathFinder.h"


class	Astar: public virtual PathFinder
	{
	Tile_coord *path;		// Coords. to goal, ending with -1's.
	int next_index;			// Index of next tile to return.
public:
	Astar() : path(0), next_index(0)
		{  }
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord s, Tile_coord d,
						int (*tileclassifier)(int,int,int&));

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord& n);

	virtual ~Astar();
	};

#endif
