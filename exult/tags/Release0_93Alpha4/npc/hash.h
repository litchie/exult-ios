/**
 **	Hash.h - A hash table.
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

class Vector;

/*
 *	Here's an item in the table:
 */
class Hash_item
	{
	char *key;			// ->key.
public:
	friend class Hash_table;
	Hash_item(char *k);
	~Hash_item()
		{ delete key; }
	char *get_key()
		{ return key; }
	};

/*
 *	A hash table:
 */
class Hash_table
	{
	Vector *bins;			// Each bin is a vector of items.
	int num_bins;
public:
	Hash_table(int nbins = 137);
	~Hash_table();
	Hash_item *search(char *k);	// Search for a key.
	int add(Hash_item *newitem);	// Add an item.
	};
