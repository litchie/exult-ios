#ifndef	__Astar_h_
#define	__Astar_h_

#include "PathFinder.h"
#include "objs.h"


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
	virtual	int	NewPath(int sx,int sy,int sz,int dx,int dy, int dz,
					int (*tileclassifier)(int,int,int&));

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(int &nx,int &ny, int &nz);

	virtual ~Astar();
	};

#endif
