/**
 **	Convers.cc - Conversation.
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

#include "convers.h"
#include "event.h"

Hash_table *Sentence::table = 0;

/*
 *	Initialize the class.
 */

void Sentence::init
	(
	)
	{
	if (!table)
		table = new Hash_table(317);
	}

/*
 *	Create a unique sentence.
 */

Sentence *Sentence::create
	(
	char *nm
	)
	{
	Sentence *s = (Sentence *) table->search(nm);
	if (s)				// Already exists?
		return (s);
	s = new Sentence(nm);
	table->add(s);
	return (s);
	}

/*
 *	Remove a handler.
 *
 *	Output:	0 if responders list is now empty.
 */

int Sentence_response::remove_handler
	(
	Event_handler *eh
	)
	{
	int cnt = responders.get_cnt();
	int handlers = 0;		// Count non-null handlers.
	for (int i = 0; i < cnt; i++)
		{			// Tell each handler.
		Event_handler *each = (Event_handler *) responders.get(i);
		if (each == eh)
			responders.put(i, 0);
		else if (each)
			handlers++;
		}
	return (handlers);
	}

/*
 *	See if this is empty.
 */

int Sentence_response::is_empty
	(
	)
	{
	int cnt = responders.get_cnt();
	for (int i = 0; i < cnt; i++)
		if (responders.get(i))
			return (0);
	return (1);
	}

/*
 *	Respond to a sentence.
 *
 *	Output:	1 if handled, else 0.
 */

int Sentence_response::respond
	(
	)
	{
	int cnt = responders.get_cnt();
	int handled = 0;		// Want to see if it was handled.
	for (int i = 0; i < cnt; i++)
		{			// Tell each handler.
		Event_handler *eh = (Event_handler *) responders.get(i);
		if (eh)
			handled |= eh->handle_sentence(sentence);
		}
	return (handled);
	}

