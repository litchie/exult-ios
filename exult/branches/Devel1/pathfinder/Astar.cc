#include "Astar.h"

#include "gamewin.h"

Astar::Astar() {}

Astar::~Astar() {}

extern Tile_coord *Find_path ( Game_window *gwin, Tile_coord start, Tile_coord goal );


int     Astar::NewPath(Tile_coord &source,Tile_coord &dest,int (*tileclassifier)(Tile_coord &))
{
	start=source;
	goal=dest;
	calculated_path.clear();
	Game_window *gwin = Game_window::get_game_window();

	
	Tile_coord *t=Find_path(gwin,start,goal);
	if(!t)
		return 0;

	// Push the path into our stack
	int	i=0;
	while(t[i]&&!(t[i]==goal))
		{
		calculated_path.push_back(t[i]);
		++i;
		}
	delete [] t;
	return 1;
}


int     Astar::GetNextStep(Tile_coord &next)
{
	if(calculated_path.size()==0)
		return -1;
	next=calculated_path.front();
	calculated_path.pop_front();
	return 1;
}
