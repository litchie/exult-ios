/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Path.cc - Pathfinding algorithm.
 **
 **	Written: 4/7/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <hash_set>
#include "objs.h"
#include "gamewin.h"

/*
 *	Iterate through neighbors of a tile (in 2 dimensions).
 */
class Neighbor_iterator
	{
	Tile_coord tile;		// Original tile.
	static int coords[2][8];	// Coords to go through.
	int index;			// 0-7.
public:
	Neighbor_iterator(Tile_coord t) : tile(t), index(0)
		{  }
					// Get next neighbor.
	int operator()(Tile_coord& newt)
		{
		while (index < 8)
			{
			newt = Tile_coord(tile.tx + coords[index][0],
					tile.ty + coords[index][1], tile.tz);
			index++;
			if (newt.tx >= 0 && newt.tx < num_tiles &&
			    newt.ty >= 0 && newt.ty < num_tiles)
				return (1);
			}
		return (0);
		}
	};

/*
 *	Statics:
 */
int Neighbor_iterator::coords[2][8] = {
	-1, -1, 0, -1, 1, -1,
	0, -1,           0, 1,
	1, -1,  1, 0,  1, 1
	};

/*
 *	Estimate cost from one point to another.
 */

inline int Cost_to_goal
	(
	Tile_coord& from,
	Tile_coord& to
	)
	{
	return from.distance(to);
	}

/*
 *	Figure cost going from one tile to an adjacent tile.
 *
 *	Output:	Cost, or -1 if blocked.
 *		The 'tz' field in tile may be modified.
 */

static int Get_cost
	(
	Game_window *gwin,
	Tile_coord& tile		// The tile we're going to.  The 'tz'
					//   field may be modified.
	)
	{
	int cx = tile.tx/tiles_per_chunk, cy = tile.ty/tiles_per_chunk;
	int tx = tile.tx%tiles_per_chunk, ty = tile.ty%tiles_per_chunk;
	Chunk_object_list *olist = gwin->get_objects(cx, cy);
	olist->setup_cache();		// Make sure cache is valid.
	int new_lift;			// Might climb/descend.
	if (olist->is_blocked(tile.tz, tx, ty, new_lift))
		{			//+++++++Check for door.
					//+++++++Need method to get shape.
		return -1;
		}
	int cost = 1;
	if (new_lift != tile.tz)
		{
		cost++;
		tile.tz = new_lift;
		}
					// Maybe check types of ground?
	return (cost);
	}

/*
 *	A node for our search:
 */
class Search_node
	{
	Tile_coord tile;		// The coords (x, y, z) in tiles.
	short start_cost;		// Actual cost from start.
	short goal_cost;		// Estimated cost to goal.
	short total_cost;		// Sum of the two above.
	Search_node *parent;		// Prev. in path.
	Search_node *priority_next;	// ->next with same total_cost, or
					//   NULL if not in 'open' set.
public:
	Search_node(Tile_coord& t, short scost, short gcost, Search_node *p)
		: tile(t), start_cost(scost), goal_cost(gcost),
		  parent(p), priority_next(0)
		{
		total_cost = gcost + scost;
		}
					// For creating a key to search for.
	Search_node(Tile_coord& t) : tile(t)
		{  }
	Tile_coord get_tile() const
		{ return tile; }
	int get_start_cost()
		{ return start_cost; }
	int get_goal_cost()
		{ return goal_cost; }
	int get_total_cost()
		{ return total_cost; }
	int is_open()			// In 'open' priority queue?
		{ return priority_next != 0; }
	void update(short scost, short gcost, Search_node *p)
		{
		start_cost = scost;
		goal_cost = gcost;
		total_cost = gcost + scost;
		parent = p;
		}
	Tile_coord *create_path()	// Create path back to start, ending
					//   with Tile_coord(-1, -1, -1).
		{
		int cnt = 2;		// This, and fake after it.
					// Count back to start.
		Search_node *each = this;
		while ((each = each->parent) != 0)
			cnt++;
		Tile_coord *result = new Tile_coord[cnt];
		result[cnt - 1] = Tile_coord(-1, -1, -1);
		each = this;
		for (int i = cnt - 2; i >= 0; i--)
			{
			result[i] = each->tile;
			each = each->parent;
			}
		return result;
		}
					// Add to chain of same priorities.
	void add_to_chain(Search_node *&last)
		{
		if (last)
			{
			priority_next = last->priority_next;
			last->priority_next = this;
			}
		else
			{
			last = this;
			priority_next = this;
			}
		}
					// Remove this from its chain.
	void remove_from_chain(Search_node *&last)
		{
		if (priority_next == this)
					// Only one in chain?
			last = 0;
		else
			{		// Got to find prev. to this.
			Search_node *prev = last;
			do
				{
				Search_node *next = prev->priority_next;
				if (next == this)
					break;
				prev = next;
				}
			while (prev != last);
			if (prev)
				{
				prev->priority_next = priority_next;
				if (last == this)
					last = priority_next;
				}
			}
		priority_next = 0;	// No longer in 'open'.
		}
					// Remove 1st from a priority chain.
	static Search_node *remove_first_from_chain(Search_node *&last)
		{
		Search_node *first = last->priority_next;
		if (first == last)	// Last entry?
			last = 0;
		else
			last->priority_next = first->priority_next;
		first->priority_next = 0;
		return first;
		}
	};

/*
 *	Hash function for nodes:
 */
class Hash_node
	{
public:
	size_t operator() (const Search_node *a) const
		{
		const Tile_coord t = a->get_tile();
		return ((t.tz << 16) + (t.ty << 8) + t.tx);
		}
	};

/*
 *	For testing if two nodes match.
 */
class Equal_nodes
	{
public:
     	bool operator() (const Search_node *a, const Search_node *b) const
     		{
		Tile_coord ta = a->get_tile(), tb = b->get_tile();
		return ta == tb;
		}
	};

/*
 *	The priority queue for the A* algorithm:
 */
class A_star_queue
	{
	Vector open;			// Nodes to be done, by priority. Each
					//   is a ->last node in chain.
	int best;			// Index of 1st non-null ent. in open.
					// For finding each tile's node:
	hash_set<Search_node *, Hash_node, Equal_nodes> lookup;
public:
	A_star_queue() : open(256), lookup(1000)
		{  
		best = open.get_cnt();	// Best is past end.
		}
	~A_star_queue()
		{
		lookup.clear();		// Remove all nodes.
		}
	void add_back(Search_node *nd)	// Add an existing node back to 'open'.
		{
		int total_cost = nd->get_total_cost();
		Search_node *last = (Search_node *) open.get(total_cost);
		nd->add_to_chain(last);	// Add node to this chain.
		open.put(total_cost, last);
		if (total_cost < best)
			best = total_cost;
		}
	void add(Search_node *nd)	// Add new node to 'open' set.
		{
		lookup.insert(nd);
		add_back(nd);
		}
					// Remove node from 'open' set.
	void remove_from_open(Search_node *nd)
		{
		if (!nd->is_open())
			return;		// Nothing to do.
		int total_cost = nd->get_total_cost();
		Search_node *last = (Search_node *) open.get(total_cost);
		nd->remove_from_chain(last);
		if (!last)		// Last in chain?
			{
			open.put(total_cost, (Search_node *) 0);
			if (total_cost == best)
				{
				int cnt = open.get_cnt();
				for (best++; best < cnt; best++)
					if (open.get(best) != 0)
						break;
				}
			}
		}
	Search_node *pop()		// Pop best from priority queue.
		{
		Search_node *last = (Search_node *) open.get(best);
		if (!last)
			return (0);
					// Return 1st in list.
		Search_node *node = Search_node::remove_first_from_chain(last);
		if (!last)		// List now empty?
			{
			open.put(best, (Search_node *) 0);
			int cnt = open.get_cnt();
			for (best++; best < cnt; best++)
				if (open.get(best) != 0)
					break;
			}
		return node;
		}
					// Find node for given tile.
	Search_node *find(Tile_coord tile)
		{
		Search_node key(tile);
		hash_set<Search_node *, Hash_node, Equal_nodes>::iterator it =
							lookup.find(&key);
		return *it;
		}
	};

/*
 *	First cut at using the A* pathfinding algorithm.
 *
 *	Output:	->(allocated) array of Tile_coords to follow, or 0 if failed.
 */

Tile_coord *Find_path
	(
	Tile_coord start,		// Where to start from.
	Tile_coord goal			// Where to end up.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	A_star_queue nodes;		// The priority queue & hash table.
	int max_cost = Cost_to_goal(start, goal);
					// Create start node.
	nodes.add(new Search_node(start, 0, max_cost, 0));
	max_cost *= 3;			// Don't try forever.
	if (max_cost < 64)
		max_cost = 64;
	Search_node *node;		// Try 'best' node each iteration.
	while ((node = nodes.pop()) != 0)
		{
		if (node->get_tile() == goal)
					// Success.
			return node->create_path();
					// Go through surrounding tiles.
		Neighbor_iterator get_next(node->get_tile());
		Tile_coord ntile(0, 0, 0);
		while (get_next(ntile))
			{		// Get cost to next tile.
			int step_cost = Get_cost(gwin, ntile);
					// Blocked?
			if (step_cost == -1)
				continue;
					// Get cost from start to ntile.
			int new_cost = node->get_start_cost() + step_cost;
					// See if next tile already seen.
			Search_node *next = nodes.find(ntile);
					// Already there, and cheaper?
			if (next && next->get_start_cost() < new_cost)
				continue;
			int new_goal_cost = Cost_to_goal(ntile, goal);
					// Skip nodes too far away.
			if (new_goal_cost >= max_cost)
				continue;
			if (!next)	// Create if necessary.
				{
				next = new Search_node(ntile, new_cost,
						new_goal_cost, node);
				nodes.add(next);
				}
			else
				{	// It's going to move.
				nodes.remove_from_open(next);
				next->update(new_cost, new_goal_cost, node);
				nodes.add_back(next);
				}
			}
		}
	return 0;			// Failed if here.
	}
