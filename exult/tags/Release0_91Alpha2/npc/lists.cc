/**
 **	Lists.cc - Various list classes.
 **
 **	Written: 5/20/99 - JSF
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

#include "lists.h"

/*
 *	Copy another list.
 */

Slist::Slist
	(
	Slist& list2
	)
	{
	Slist_entry *each = list2.last;
	if (!each)
		{
		last = 0;
		return;
		}
	each = each->next;		// Create first.
	Slist_entry *first = new Slist_entry(each->ent);
	last = first;
	while (each != list2.last)
		{
		each = each->next;
		last->next = new Slist_entry(each->ent);
		last = last->next;
		}
	last->next = first;
	}

/*
 *	Remove all entries.
 */

void Slist::clear
	(
	)
	{
	while (last)
		remove_first();
	}

/*
 *	Remove an entry.
 *
 *	Output:	0 if not found.
 */

int Slist::remove
	(
	void *e				// Entry to find.
	)
	{
	if (!last)
		return (0);		// Empty.
	Slist_entry *prev = last;
	do
		{
		Slist_entry *cur = prev->next;
		if (cur->ent == e)	// The one?
			{		// Might a list of 1.
			if (prev->next == prev)
				last = 0;
			else
				{
				prev->next = cur->next;
				if (cur == last)
					last = prev;
				}
			delete cur;
			return (1);
			}
		prev = cur;
		}
	while (prev != last);
	return (0);
	}
