/*
 *	path.cc - Pathfinding algorithms.
 *
 *  Copyright (C) 2000-2001  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <iostream>

#include "hash_utils.h"
#include "PathFinder.h"
#include "exult_constants.h"
#include "vec.h"

using std::cout;
using std::endl;
using std::size_t;

Tile_coord *Find_path
	(
	Tile_coord start,		// Where to start from.
	Tile_coord goal,		// Where to end up.
	Pathfinder_client *client,	// Provides costs.
	int& pathlen			// Length of path returned.
	);


/*
 *	Iterate through neighbors of a tile (in 2 dimensions).
 */
class Neighbor_iterator
	{
	Tile_coord tile;		// Original tile.
	static int coords[16];		// Coords to go through ((x,y) pairs)
	int index;			// 0-7.
public:
	Neighbor_iterator(Tile_coord t) : tile(t), index(0)
		{  }
					// Get next neighbor.
	int operator()(Tile_coord& newt)
		{
		if (index < 8)
			{
			newt = Tile_coord(tile.tx + coords[2*index],
				tile.ty + coords[2*index + 1], tile.tz);
			index++;
					// Handle world-wrapping.
			newt.tx = (newt.tx + c_num_tiles)%c_num_tiles;
			newt.ty = (newt.ty + c_num_tiles)%c_num_tiles;
			return (1);
			}
		return (0);
		}
	};

/*
 *	Statics:
 */
int Neighbor_iterator::coords[16] = {
	-1, -1, 0, -1, 1, -1,
	-1,  0,        1,  0,
	-1,  1, 0,  1, 1,  1
	};

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
					// Create path back to start.
	Tile_coord *create_path(int& pathlen)
		{
		int cnt = 1;		// This.
					// Count back to start.
		Search_node *each = this;
		while ((each = each->parent) != 0)
			cnt++;
		pathlen = cnt - 1;	// Don't want starting tile.
		Tile_coord *result = new Tile_coord[pathlen];
		each = this;
		for (int i = pathlen - 1; i >= 0; i--)
			{
			result[i] = each->tile;
			each = each->parent;
			}
		return result;
		}
#if VERIFYCHAIN
					// Returns 0 if bad chain.
	int verify_chain(Search_node *last, int removed = 0)
		{
		if (!last)
			return (1);
		int found = 0;
		Search_node *prev = last;
		int cnt = 0;
		do
			{
			Search_node *next = prev->priority_next;
			if (next == this)
				found = 1;
			prev = next;
			if (cnt > 10000)
				break;
			}
		while (prev != last);
		if (!found && !removed)
			return (0);
		if (cnt == 10000)
			return (0);
		return (1);
		}
#endif
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
#if VERIFYCHAIN
		if (!verify_chain(last))
			cout << "Bad chain after adding." << endl;
#endif
		}
					// Remove this from its chain.
	void remove_from_chain(Search_node *&last)
		{
#if VERIFYCHAIN
		if (!verify_chain(last))
			cout << "Bad chain before removing." << endl;
#endif
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
#if VERIFYCHAIN
		if (!verify_chain(last, 1))
			cout << "Bad chain after removing." << endl;
#endif
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

#ifndef DONT_HAVE_HASH_SET

/*
 *	Hash function for nodes:
 */
class Hash_node
	{
public:
	size_t operator() (const Search_node *a) const
		{
		const Tile_coord t = a->get_tile();
		return ((t.tz << 24) + (t.ty << 12) + t.tx);
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

#else

/*
 *	"Less than" relation for nodes
 */
class Less_nodes
	{
public:
     	bool operator() (const Search_node *a, const Search_node *b) const
     		{
			Tile_coord ta = a->get_tile(), tb = b->get_tile();
			uint32 apos = ta.tx << 16, bpos = tb.tx << 16;
			apos |= ta.ty << 4;
			bpos |= tb.ty << 4;
			apos |= ta.tz;
			bpos |= tb.tz;
			/* Because #(short x short) is <= #int, we can define an injective projection,
			** which is all we need. */
			return apos < bpos;
		}
	};

#endif


/*
 *	The priority queue for the A* algorithm:
 */
class A_star_queue
	{
	Exult_vector<Search_node*> open;// Nodes to be done, by priority. Each
					//   is a ->last node in chain.
	int best;			// Index of 1st non-null ent. in open.
					// For finding each tile's node:
#ifndef DONT_HAVE_HASH_SET
	typedef	hash_set<Search_node *, Hash_node, Equal_nodes> Lookup_set;
#else
	typedef	std::set<Search_node *, Less_nodes> Lookup_set;
#endif
	Lookup_set lookup;
public:
#ifndef DONT_HAVE_HASH_SET
	A_star_queue() : open(256), lookup(1000)
#else
	A_star_queue() : open(256), lookup()
#endif
		{
		open.insert(open.begin(), 256, (Search_node *) 0);
		best = open.size();	// Best is past end.
		}
	~A_star_queue()
		{
#if 1	
	/* 
	This _should_ work, but might hang some hash_set implementations.
	The problem is that on deleting the Search_node, the hash_set can
	no longer properly evaluate the hash value (since the hash function
	dereferences the Search_node* stored). This might cause an endless
	loop.
	*/
		for(Lookup_set::iterator X = lookup.begin(); X != lookup.end();) {
			Search_node *sn = *X;
			X++;
			delete sn; // only delete this _after_ iterating
}
#endif
		lookup.clear();		// Remove all nodes.
		}
	void add_back(Search_node *nd)	// Add an existing node back to 'open'.
		{
		int total_cost = nd->get_total_cost();
		Search_node *last = total_cost < open.size() ?
						open[total_cost] : 0;
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
		Search_node *last = total_cost < open.size() ?
						open[total_cost] : 0;
		nd->remove_from_chain(last);
					// Store updated 'last'.
		open.put(total_cost, last);
		if (!last)		// Last in chain?
			{
			if (total_cost == best)
				{
				int cnt = open.size();
				for (best++; best < cnt; best++)
					if (open[best] != 0)
						break;
				}
			}
		}
	Search_node *pop()		// Pop best from priority queue.
		{
		Search_node *last = best < open.size() ? open[best] : 0;
		if (!last)
			return (0);
					// Return 1st in list.
		Search_node *node = Search_node::remove_first_from_chain(last);
					// Store updated 'last'.
		open.put(best, last);
		if (!last)		// List now empty?
			{
			int cnt = open.size();
			for (best++; best < cnt; best++)
				if (open[best] != 0)
					break;
			}
		return node;
		}
					// Find node for given tile.
	Search_node *find(Tile_coord tile)
		{
		Search_node key(tile);
#ifndef DONT_HAVE_HASH_SET
		hash_set<Search_node *, Hash_node, Equal_nodes>::iterator it =
							lookup.find(&key);
#else
		std::set<Search_node *, Less_nodes>::iterator it =
			lookup.find(&key);
#endif
		if (it != lookup.end())
			return *it;
		else
			return 0;
		}
	};

static int tracing = 0;

/*
 *	First cut at using the A* pathfinding algorithm.
 *
 *	Output:	->(allocated) array of Tile_coords to follow, or 0 if failed.
 */

Tile_coord *Find_path
	(
	Tile_coord start,		// Where to start from.
	Tile_coord goal,		// Where to end up.
	Pathfinder_client *client,	// Provides costs.
	int& pathlen			// Length of path returned.
	)
	{
	A_star_queue nodes;		// The priority queue & hash table.
	int max_cost = client->estimate_cost(start, goal);
					// Create start node.
	nodes.add(new Search_node(start, 0, max_cost, 0));
					// Figure when to give up.
	max_cost = client->get_max_cost(max_cost);
	Search_node *node;		// Try 'best' node each iteration.
	while ((node = nodes.pop()) != 0)
		{
		if (tracing)
			cout << "Goal: (" << goal.tx << ", " << goal.ty <<
			"), Node: (" << node->get_tile().tx << ", " <<
			node->get_tile().ty << ")" << endl;
		Tile_coord curtile = node->get_tile();
		if (client->at_goal(curtile, goal))
					// Success.
			return node->create_path(pathlen);
					// Go through surrounding tiles.
		Neighbor_iterator get_next(curtile);
		Tile_coord ntile(0, 0, 0);
		while (get_next(ntile))
			{		// Get cost to next tile.
			int step_cost = client->get_step_cost(curtile, ntile);
					// Blocked?
			if (step_cost == -1)
				continue;
					// Get cost from start to ntile.
			int new_cost = node->get_start_cost() + step_cost;
					// See if next tile already seen.
			Search_node *next = nodes.find(ntile);
					// Already there, and cheaper?
			if (next && next->get_start_cost() <= new_cost)
				continue;
			int new_goal_cost = client->estimate_cost(ntile, goal);
					// Skip nodes too far away.
			if (new_cost + new_goal_cost >= max_cost)
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
	pathlen = 0;			// Failed if here.
	return 0;	
	}
