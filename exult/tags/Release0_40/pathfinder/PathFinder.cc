#include "PathFinder.h"

/*
 *	Given the estimated cost from start to goal, figure the max. cost
 *	before the pathfinder should quit.
 */

int Pathfinder_client::get_max_cost
	(
	int cost_to_goal		// From estimate_cost(start, goal).
	)
	{
	int max_cost = 3*cost_to_goal;
	return (max_cost < 64 ? 64 : max_cost);
	}

/*
 *	Is tile at goal?
 */

int Pathfinder_client::at_goal
	(
	Tile_coord& tile,
	Tile_coord& goal
	)
	{
	return (tile.tx == goal.tx && tile.ty == goal.ty &&
		(goal.tz == -1 || tile.tz == goal.tz));
	}


PathFinder::~PathFinder()
{}
