#ifndef	__Zombie_h_
#define	__Zombie_h_


#include "PathFinder.h"


class	Zombie: public virtual PathFinder
	{
public:
	// Find a path from sx,sy to dx,dy
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord &source,Tile_coord &dest,int (*tileclassifier)(Tile_coord &));

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord &);

	Zombie();
	virtual ~Zombie();

protected:
	Tile_coord	destination;
	Tile_coord	here;
	};

#endif
