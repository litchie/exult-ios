/**
 **	Convers.h - Conversation.
 **
 **	Written: 4/6/99 - JSf
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

#include "hash.h"
#include "vec.h"

class Event_handler;

/*
 *	A sentence is the basic unit we deal with.  They're stored in a
 *	global hash table so that, for instance, Sentence("Name?") always
 *	points to the same thing.
 */
class Sentence : public Hash_item
	{
	static Hash_table *table;	// Table they're stored in.
	Sentence(char *nm) : Hash_item(nm)
		{  }
public:
	static void init();		// Create table.
					// Use this to get/create.
	static Sentence *create(char *nm);
	char *get_text()
		{ return get_key(); }
	};

/*
 *	This class indicates the possible responders (fsm transitions) to a
 *	sentence.
 */
class Sentence_response
	{
	Sentence *sentence;		// What's recognized.
	Vector responders;		// Possible event handlers for when
					//  this sentence is spoken.
public:
	Sentence_response(Sentence *s) : sentence(s),
			responders(0, 2)
		{  }
	Sentence *get_sentence()
		{ return sentence; }
	void add_handler(Event_handler *eh)
		{ responders.append(eh); }
	int remove_handler(Event_handler *eh);
	int is_empty();			// No more handlers?
	int respond();			// Respond to sentence.
	};
