/**
 **	Fsm.h - A finite-state-machine.
 **
 **	Written: 4/2/99 - JSF
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

#include "vec.h"
#include "event.h"

class Fsm_transition;
class Condition;
class Action;
class Npc;
class Npc_attribute;
class Expr;

/*
 *	Here's a state:
 */
class Fsm_state
	{
	char *name;
	Vector out;			// Lines to next states.
	Vector in;			// Lines from previous states.
	void *data;			// Application data.
					// Add a transition.
	void add_transition(Fsm_transition *trans, Vector& tlist);
	void add_transition_in(Fsm_transition *trans)
		{ add_transition(trans, in); }
	void add_transition_out(Fsm_transition *trans)
		{ add_transition(trans, out); }
					// Remove transitions.
	void remove_transition(Fsm_transition *trans, Vector& tlist);
	void remove_transition_in(Fsm_transition *trans)
		{ remove_transition(trans, in); }
	void remove_transition_out(Fsm_transition *trans)
		{ remove_transition(trans, out); }
	Fsm_state(char *nm);		// A copy of name is made.
public:
	friend class Fsm;
	~Fsm_state();
	char *get_name()
		{ return name; }
	};

/*
 *	A state transition:
 */
class Fsm_transition : public Event_handler
	{
	Fsm *fsm;			// Our owner.
	Fsm_state *from, *to;		// What it connects.
	Condition *cond;		// Transition condition.
	Vector actions;			// List of Action's.
	void *data;			// Application data.
	Fsm_transition(Fsm *owner, Condition *c, Fsm_state *f, Fsm_state *t) 
		: fsm(owner), cond(c), from(f), to(t), data(0)
		{  }
	void execute_actions();		// Do transition actions.
public:
	friend class Fsm;
	~Fsm_transition();
	Fsm_state *get_from()
		{ return from; }
	Fsm_state *get_to()
		{ return to; }
	Fsm *get_fsm()
		{ return fsm; }
	virtual int handle_sentence(Sentence *spoken);
	virtual void handle_start();	// Handle start of conversation.
	virtual void handle();		// Check/handle true condition.
	void add_action(Action *action)
		{ actions.append(action); }
	};

/*
 *	Here's a finite-state-machine:
 */
class Fsm
	{
	Npc *npc;			// Npc this belongs to.
	char *name;			// A descriptive name.
	Vector states;			// List of states.
	Fsm_state *cur_state;		// ->current state.
					// Sensitize/desensitize NPC.
	void sensitize_npc(Fsm_state *s);
	void desensitize_npc(Fsm_state *s);
public:
	friend class Fsm_transition;
	Fsm(Npc *n, char *nm);
	~Fsm();
	char *get_name()
		{ return name; }
	Fsm_state *add_state(char *nm)	// Add a state.
		{
		Fsm_state *st = new Fsm_state(nm);
		states.append(st);
		return (st);
		}
					// Add a transition.
	Fsm_transition *add_transition(Condition *c, Fsm_state *f,
						Fsm_state *t)
		{
		Fsm_transition *tr = new Fsm_transition(this, c, f, t);
		f->add_transition_out(tr);
		t->add_transition_in(tr);
		return (tr);
		}
	Fsm_state *find_state(char *nm);// Find state by name.
					// Change state.
	void set_cur_state(Fsm_state *s);
	void activate(Fsm_state *s = 0);// For start/end of converse. topics.
	void deactivate();
	};

/*
 *	An action, executed during a transition between states:
 */
class Action
	{
public:
	virtual void execute(Npc *npc) = 0;
	};

/*
 *	An action to change to a new topic.
 */
class Conversation_topic_action : public Action
	{
	Fsm *topic;
public:
	Conversation_topic_action(Fsm *t) : topic(t)
		{  }
	virtual void execute(Npc *npc);
	};

/*
 *	An action to return to the previous topic.
 */
class Conversation_prev_action : public Action
	{
public:
	Conversation_prev_action() { }
	virtual void execute(Npc *npc);
	};

/*
 *	An action to modify an NPC attribute.
 */
class Npc_attribute_action : public Action
	{
	Npc_attribute *att;
	Expr *val;			// Value to assign.
public:
	Npc_attribute_action(Npc_attribute *a, Expr *v) : att(a), val(v)
		{  }
	virtual void execute(Npc *npc);
	};

