#include "Zombie.h"

#include "../gamewin.h"

Zombie::Zombie() {}

Zombie::~Zombie() {}


int     Zombie::NewPath(Tile_coord &start,Tile_coord &dest,int (*tileclassifier)(Tile_coord &))
{
	here=start;
	classify=tileclassifier;
	destination=dest;
	return 1;	// Always claim to find the path
}



int     Zombie::GetNextStep(Tile_coord &next)
{
	next=here;
	if(here.tx<destination.tx)
		next.tx++;
	else
	if(here.tx>destination.tx)
		next.tx--;

	if(here.ty<destination.ty)
		next.ty++;
	else
	if(here.ty>destination.ty)
		next.ty--;

        int cx = next.tx/tiles_per_chunk, cy = next.ty/tiles_per_chunk;
        int tx = next.tx%tiles_per_chunk, ty = next.ty%tiles_per_chunk;
	Game_window *gwin = Game_window::get_game_window();
        Chunk_object_list *olist = gwin->get_objects(cx, cy);
	int	new_lift;

	if (olist->is_blocked(next.tz, tx, ty, new_lift))
		{
		// Zombies don't do doors or anything.
		// They just look unhappy and keep calling for their lawyers
		return -1;
		}
	next.tz=new_lift;
	return 0;
}
