/**
 **	Npc.h - Non-player characters.
 **
 **	Written: 4/5/99 - JSF
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

#ifndef INCL_NPC
#define INCL_NPC	1

#include "vec.h"
#include "lists.h"

class Fsm;
class Sentence;
class Event_handler;

/*
 *	An attribute is something that can take a range of integer values.
 */
class Npc_attribute
	{
	char *name;			// Name; i.e., "suspicion".
	int min, max;			// Min., max values allowed.
	int val;			// Current value.
public:
	friend class Npc;
	Npc_attribute(char *nm, int mn = 0, int mx = 100);
	int get_val()
		{ return val; }
	void set_val(int v)
		{ val = v <= min ? min : (v >= max ? max : v); }
	};

/*
 *	An interface for users of NPC's:
 */
class Npc_user
	{
public:
					// Show what the NPC says.
	virtual void show_response(char *msg) = 0;
	};

/*
 *	A non-player character:
 */
class Npc
	{
	char *name;			// Name.
	Npc_user *user;			// User.
	Vector attributes;		// List of Npc_attribute's.
	Vector fsms;			// List of Fsm's.
	Vector sentences;		// List of Sentence_response's we
					//   currently support.
	Slist sensitivity_list;		// List of Fsm_transitions that need
					//   to be checked for truth at any
					//   time.
	Sentence *bye;			// Sentence to quit.
	Vector starts;			// Transitions at start of convers.
	Stack topics;			// Stack of topics (Fsm's).  The cur-
					//   rent topic is the TOS.
	Fsm *base_topic;		// Default for start of conversation.
	int responding_to_sentence;	// Index of sentence we're responding.
	void clear_sentences();		// Clear out list of sentences.
public:
	friend class Npc_sentence_iterator;
	Npc(char *nm);
	~Npc();
	char *get_name()
		{ return name; }
	Fsm *get_topic()		// Get current topic.
		{ return (Fsm *) topics.get_first(); }
	void set_base_topic(Fsm *t)
		{ base_topic = t; }
	void set_user(Npc_user *u)
		{ user = u; }
	Npc_user *get_user()
		{ return user; }
	void set_bye(Sentence *b)
		{ bye = b; }
	Fsm *add_fsm(char *nm);		// Create and add an FSM.
	Fsm *find_fsm(char *nm);	// Find FSM by name.
					// Create and add an attribute.
	Npc_attribute *add_attribute(char *nm, int mi, int ma, int v);
					// Find att. by name.
	Npc_attribute *find_attribute(char *nm);
					// Add sentence and handler.
	void add_sentence(Sentence *s, Event_handler *eh);
					// Remove sentence handler.
	void remove_sentence_handler(Event_handler *eh);
					// Add/remove handler of the start
					//   of a conversation.
	void add_start_handler(Event_handler *eh);
	void remove_start_handler(Event_handler *eh);
					// Add/remove to/from sens. list.
	void add_to_sensitivity_list(Event_handler *eh)
		{ sensitivity_list.append(eh); }
	void remove_from_sensitivity_list(Event_handler *eh)
		{ sensitivity_list.remove(eh); }
	int get_max_sentence_cnt()	// Get max. possible # sentences.
		{ return sentences.get_cnt(); }
	int start(Fsm *topic = 0);	// Start conversation.
	int respond_to_sentence(int i);	// Respond to i'th entry in
					//   'sentences'.
	void check_sensitivity();	// Check sensitivity list.
					// Change to new topic.
	void new_topic(Fsm *topic, int clear_stack = 0);
	void prev_topic();		// Change back to previous topic.
	};

/*
 *	Get the sentences an NPC will respond to.
 */

class Npc_sentence_iterator
	{
	Npc *npc;
	int index;			// Index of next to get.
	int cnt;			// # of sentences in list.
	unsigned char said_bye;		// Already returned "Bye".
public:
	Npc_sentence_iterator(Npc *n) : npc(n), cnt(npc->sentences.get_cnt()),
					index(0), said_bye(0)
		{  }
					// Get next sentence & its index.
	int operator()(Sentence *& sent, int& num);
	};

#endif
