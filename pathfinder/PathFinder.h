#ifndef	__PathFinder_h_
#define	__PathFinder_h_




class	PathFinder
	{
	// Find a path from sx,sy,sz to dx,dy,dz
	// Return 0 if no path can be traced.
	// Return !0 if path found
	virtual	int	NewPath(int sx,int sy,int sz,int dx,int dy,int dz,int (*tileclassifier)(int,int,int&))=0;

	// Retrieve the coordinates of the next step on the path
	virtual	int	GetNextStep(int &nx,int &ny,int &nz)=0;

public:
	virtual ~PathFinder();
	};

#endif
