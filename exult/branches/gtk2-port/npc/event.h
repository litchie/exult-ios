/**
 **	Event.h - Events.
 **
 **	Written: 4/7/99 - JSF
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

class Sentence;

/*
 *	Here's an interface to things that can handle events, like sentences
 *	used in conversation.  +++++Move to its own header?
 */
class Event_handler
	{
public:
					// Handle sentence that was spoken.
	virtual int handle_sentence(Sentence *spoken);
	virtual void handle_start();	// Handle start of conversation.
	virtual void handle();		// Check for true cond. at any time.
	};


