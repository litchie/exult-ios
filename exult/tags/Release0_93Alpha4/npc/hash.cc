/**
 **	Hash.cc - A hash table.
 **
 **	Written: 4/6/99 - JSF
 **/

/*
Copyright (C) 1999  Jeffrey S. Freedman

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

#include <string.h>
#include "hash.h"
#include "vec.h"

/*
 *	Create a hash item.
 */

Hash_item::Hash_item
	(
	char *k				// ->key.  A copy is made.
	) : key(strdup(k))
	{
	}

/*
 *	Create a hash table.
 */

Hash_table::Hash_table
	(
	int nbins			// # of bins desired.
	) : num_bins(nbins)
	{
	bins = new Vector[num_bins];
	}

/*
 *	Delete a hash table.  Note:  The items are not deleted.
 */

Hash_table::~Hash_table
	(
	)
	{
	delete [] bins;
	}

/*
 *	Compute hash value.
 */

static int Hash
	(
	char *key,
	int num_bins
	)
	{
	unsigned int v = 0;
	int i;
	for (i = 1; *key; i++, key++)
		v += *key * i;
	return (v % num_bins);
	}

/*
 *	Search for an entry.
 */

Hash_item *Hash_table::search
	(
	char *k
	)
	{
	int hval = Hash(k, num_bins);	// Figure bin #.
	Vector& bin = bins[hval];
	int bcnt = bin.get_cnt();	// Get # entries in bin.
	for (int i = 0; i < bcnt; i++)
		{
		Hash_item *item = (Hash_item *) bin.get(i);
		if (item && strcmp(item->key, k) == 0)
			return (item);	// Found it.
		}
	return (0);
	}

/*
 *	Add an item to the table.
 *
 *	Output:	0 if already there.
 */

int Hash_table::add
	(
	Hash_item *newitem
	)
	{
					// Figure bin #.
	int hval = Hash(newitem->key, num_bins);
	Vector& bin = bins[hval];
	int bcnt = bin.get_cnt();	// Get # entries in bin.
	int empty = -1;			// Gets index of empty space.
	for (int i = 0; i < bcnt; i++)
		{
		Hash_item *item = (Hash_item *) bin.get(i);
		if (!item)		// Empty?
			empty = i;
		else if (strcmp(item->key, newitem->key) == 0)
			return (0);	// Already there.
		}
	if (empty >= 0)			// An empty slot?
		bin.put(empty, newitem);
	else
		bin.append(newitem);
	return (1);
	}

