/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objiter.h - Game objects iterator.
 **
 **	Written: 7/29/2000 - JSF
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

#ifndef INCL_OBJITER
#define INCL_OBJITER	1

/*
 *	Want to make all object iterators modification-safe.  For now, just
 *	trying to detect modification during iteration.
 */
class Safe_object_iterator
	{
	Object_list& list;
public:
	Safe_object_iterator(Object_list& l) : list(l)
		{ list.add_iterator(); }
	~Safe_object_iterator()
		{ list.remove_iterator(); }
	};

/*
 *	Iterate through list of objects.
 */
class Object_iterator : public Safe_object_iterator
	{
protected:
	Game_object *first;
	Game_object *stop;
	Game_object *cur;		// Next to return.
public:
	void reset()
		{ cur = first; stop = 0; }
	Object_iterator(Object_list& objects) 
		: Safe_object_iterator(objects), first(objects.first)
		{ reset(); }
#if defined(INCL_CHUNKS)
	Object_iterator(Chunk_object_list *chunk) 
		: Safe_object_iterator(chunk->objects),
		  first(chunk->objects.first)
		{ reset(); }
#endif
	Game_object *get_next()
		{
		if (cur == stop)
			return 0;
		Game_object *ret = cur;
		cur = cur->next;
		stop = first;
		return ret;
		}
	};

#if defined(INCL_CHUNKS)
/*
 *	Iterate through a chunk's nonflat objects.
 */
class Nonflat_object_iterator : public Object_iterator
	{
	Game_object *nonflats;
public:
	void reset()
		{ cur = nonflats; stop = 0; }
	Nonflat_object_iterator(Chunk_object_list *chunk)
		: Object_iterator(chunk), nonflats(chunk->first_nonflat)
		{ reset(); }
	};

/*
 *	Iterate through a chunk's flat objects.
 */
class Flat_object_iterator : public Safe_object_iterator
	{
	Game_object *first;
	Game_object *stop;
	Game_object *cur;		// Next to return.
	Game_object *stop_at;
public:
	void reset()
		{ cur = first; stop = 0; }
	Flat_object_iterator(Chunk_object_list *chunk)
		: Safe_object_iterator(chunk->objects)
		{
		first = chunk->objects.first == chunk->first_nonflat ? 0 :
							chunk->objects.first;
		stop_at = chunk->first_nonflat ? chunk->first_nonflat
						: chunk->objects.first;
		reset();
		}
	Game_object *get_next()
		{
		if (cur == stop)
			return 0;
		Game_object *ret = cur;
		cur = cur->get_next();
		stop = stop_at;
		return ret;
		}
	};

/*
 *	Iterate backwards through list of objects.
 */
class Object_iterator_backwards : public Safe_object_iterator
	{
	Game_object *first;
	Game_object *stop;
	Game_object *cur;		// Return prev. to this.
public:
	void reset()
		{ cur = first; stop = 0; }
	Object_iterator_backwards(Chunk_object_list *chunk) 
		: Safe_object_iterator(chunk->objects),
		  first(chunk->objects.first)
		{ reset(); }
	Game_object *get_next()
		{
		if (cur == stop)
			return 0;
		cur = cur->prev;
		stop = first;
		return cur;
		}
	};
#endif

#endif
