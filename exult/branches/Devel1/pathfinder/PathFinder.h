#ifndef	__PathFinder_h_
#define	__PathFinder_h_


#include "../objs.h"


class	PathFinder
	{
public:
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(Tile_coord &start,Tile_coord &dest,int (*tileclassifier)(Tile_coord &))=0;

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(Tile_coord &)=0;
	// Returns -1 if no path available right now

	PathFinder();
	virtual ~PathFinder();

protected:
	int (*classify)(Tile_coord &);
	};

#endif
