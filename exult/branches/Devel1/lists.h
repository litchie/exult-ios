/**
 **	Lists.h - Various list classes.
 **
 **	Written: 5/20/99 - JSF
 **/

#ifndef INCL_LISTS
#define INCL_LISTS 1
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

/*
 *	An entry in a single-link list.
 */
class Slist_entry
	{
	Slist_entry *next;		// ->next in list.
	void *ent;			// Entry.
	friend class Slist;
	friend class Slist_iterator;
	Slist_entry(void *e) : ent(e)
		{  }
	};

/*
 *	A single-link list:
 */
class Slist
	{
	Slist_entry *last;		// ->end of list, or 0 if empty.
public:
	friend class Slist_iterator;
	Slist() : last(0)
		{  }
	Slist(Slist& list2);		// Shallow copy.
	void clear();			// Remove all entries.
	~Slist()
		{ clear(); }
	void insert(void *e)		// Insert at head of list.
		{
		Slist_entry *se = new Slist_entry(e);
		if (!last)		// First one?
			last = se->next = se;
		else
			{
			se->next = last->next;
			last->next = se;
			}
		}
	void append(void *e)		// Add to end of list.
		{
		Slist_entry *se = new Slist_entry(e);
		if (!last)		// First one?
			last = se->next = se;
		else
			{
			se->next = last->next;
			last->next = se;
			last = se;
			}
		}
	void *get_last()
		{ return (last ? last->ent : 0); }
	void *get_first()
		{ return (last ? last->next->ent : 0); }
	void *remove_first()		// Remove 1st and return it.
		{
		if (!last)		// Empty?
			return (0);
		else
			{
			Slist_entry *se = last->next;
			void *e = se->ent;
			if (se == last)
				last = 0;
			else
				last->next = se->next;
			delete se;
			return (e);
			}
		}
	int remove(void *e);		// Remove.
	};

/*
 *	Iterate through a list.
 */
class Slist_iterator
	{
	Slist *list;
	Slist_entry *next;		// What to return next.
public:
	Slist_iterator(Slist *l) : list(l), 
		next(l->last ? l->last->next : 0)
		{  }
	Slist_iterator(Slist& l) : list(&l), 
		next(l.last ? l.last->next : 0)
		{  }
	void reset()			// Set back to start.
		{ next = list->last ? list->last->next : 0; }
	void *operator()()		// Get next.
		{
		void *data;
		if (next)
			{
			data = next->ent;
			next = next == list->last ? 0 : next->next;
			}
		else
			data = 0;
		return (data);
		}
	};

/*
 *	A stack.
 */
class Stack : public Slist
	{
public:
	Stack() : Slist() {  }
	void push(void *e)
		{ insert(e); }
	void *pop()
		{ return remove_first(); }
	void *get_tos()			// Get top-of-stack.
		{ return get_first(); }
	};

#endif
