#ifndef	__Zombie_h_
#define	__Zombie_h_


#include "PathFinder.h"


class	Zombie: public virtual PathFinder
	{
public:
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord s, Tile_coord d,
						int (*tileclassifier)(int,int,int&));

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord& n);
	virtual ~Zombie();
	};

#endif
