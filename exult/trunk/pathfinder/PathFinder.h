#ifndef	__PathFinder_h_
#define	__PathFinder_h_

#include "tiles.h"


class	PathFinder
	{
public:
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord s, Tile_coord d, 
						int (*tileclassifier)(int,int,int&))=0;

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord& n)=0;

	virtual ~PathFinder();
	};

#endif
