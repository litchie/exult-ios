/**
 **	Exec.h - Execute actions in the script.
 **
 **	Written: 5/27/99 - JSF
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

#include "npc.h"
#include "cond.h"

class Slist;
class Npc;
class Cond_spec;
class Condition;
class Action;
class Fsm;
class Fsm_state;

/*
 *	An NPC specification:
 */
class Npc_spec
	{
	Npc *npc;			// The NPC "personality".
	Slist *stmts;			// Topics, attributes to add.
	Fsm *cur_fsm;			// Current FSM being translated.
	int shape, portrait;		// Shape, portrait ID's.
	int chunkx, chunky;		// Chunk coord. #'s.
	int shapex, shapey;		// Shape coords. (0-15) within chunk.
public:
	friend class Script_compiler;
	Npc_spec(Npc *n, Slist *st, int shapeid, int portraitid, 
					int cx, int cy, int sx, int sy) 
		: npc(n), stmts(st), shape(shapeid), portrait(portraitid),
		  chunkx(cx), chunky(cy), shapex(sx), shapey(sy),
				cur_fsm(0)
		{  }
	void translate();		// Process the statements.
	Npc *get_npc()
		{ return npc; }
	Fsm *get_cur_fsm()		// Get/set current fsm (topic).
		{ return cur_fsm; }
	void set_cur_fsm(Fsm *f)
		{ cur_fsm = f; }
	Fsm *find_fsm(char *nm)		// Find FSM by name.
		{ return npc->find_fsm(nm); }
					// Add attribute.
	void add_attribute(char *nm, int min, int max, int val)
		{ npc->add_attribute(nm, min, max, val); }
	Fsm *add_fsm(char *nm)
		{ return (npc->add_fsm(nm)); }
	};

/*
 *	An option:
 */
class Npc_option
	{
public:
	int token;			// SHAPE, etc.
	long value;			// What to set it to.
	Npc_option(int t, long v) : token(t), value(v)
		{  }
	};

/*
 *	An NPC statement to be translated:
 */
class Npc_stmt
	{
public:
	virtual void create(Npc_spec *npc_spec) = 0;
	virtual void translate(Npc_spec *npc_spec) = 0;
	};

/*
 *	An attribute declaration:
 */
class Npc_att_stmt : public Npc_stmt
	{
	char *name;
	int min, max;			// Value range.
	int val;			// Initial value.
public:
	Npc_att_stmt(char *nm, int mi, int ma, int v)
		: name(nm), min(mi), max(ma), val(v)
		{  }
					// Create attribute.
	virtual void create(Npc_spec *npc_spec);
					// Finish.
	virtual void translate(Npc_spec *npc_spec);
	};

/*
 *	A topic:
 */
class Npc_topic_stmt : public Npc_stmt
	{
	char *name;
	Fsm *topic;			// Created.
	Slist *states;			// List of state_spec's.
public:
	Npc_topic_stmt(char *nm, Slist *st) : name(nm), states(st), topic(0)
		{  }
					// Create attribute.
	virtual void create(Npc_spec *npc_spec);
					// Finish.
	virtual void translate(Npc_spec *npc_spec);
	};

/*
 *	Specify a state:
 */
class State_spec
	{
	char *name;			// Name.
	Fsm_state *state;		// State created.
	Slist *responses;		// List of what to respond to.
public:
	State_spec(char *nm, Slist *r);
	void create(Fsm *fsm);		// Create state.
	void translate(Npc_spec *npc_spec);
	Fsm_state *get_state()
		{ return state; }
	};

/*
 *	A response to a condition:
 */
class Response_spec
	{
	Cond_spec *cond_spec;
	Slist *actions;
public:
	Response_spec(Cond_spec *c, Slist *a) : cond_spec(c), actions(a)
		{  }
	void translate(Npc_spec *npc_state, Fsm_state *state);
	};

/*
 *	Specify a condition for following an FSM transition.
 */
class Cond_spec
	{
public:
	virtual Condition *translate() = 0;
	};

/*
 *	An expression.
 */
class Expr_cond_spec : public Cond_spec
	{
	Expr *expr;			// Needs to be translated.
public:
	Expr_cond_spec(Expr *e) : expr(e)
		{  }
	virtual Condition *translate();
	};

/*
 *	Condition known during the first pass.
 */
class Cond_known_spec : public Cond_spec
	{
	Condition *cond;
public:
	Cond_known_spec(Condition *c) : cond(c)
		{  }
	virtual Condition *translate();
	};

/*
 *	Specify an action:
 */
class Action_spec
	{
public:
	virtual void translate(Npc_spec *npc_spec,
				Fsm_state *& state, Action *& action) = 0;
	};

/*
 *	Say something.
 */
class Say_action_spec : public Action_spec
	{
	char *str;			// What to say.
public:
	Say_action_spec(char *s) : str(s)
		{  }
	virtual void translate(Npc_spec *npc_spec,
				Fsm_state *& state, Action *& action);
	};

/*
 *	Switch topics.
 */
class Topic_action_spec : public Action_spec
	{
	char *topic;			// Topic name.
public:
	Topic_action_spec(char *t) : topic(t)
		{  }
	virtual void translate(Npc_spec *npc_spec,
				Fsm_state *& state, Action *& action);
	};

/*
 *	Switch to prev. topic.
 */
class Topic_prev_spec : public Action_spec
	{
public:
	virtual void translate(Npc_spec *npc_spec,
				Fsm_state *& state, Action *& action);
	};

/*
 *	Switch to a new state.
 */
class State_action_spec : public Action_spec
	{
	char *state_name;		// Name of state.
public:
	State_action_spec(char *st) : state_name(st)
		{  }
	virtual void translate(Npc_spec *npc_spec,
				Fsm_state *& state, Action *& action);
	};

/*
 *	Assign to an attribute.
 */
class Att_action_spec : public Action_spec
	{
	char *att_name;			// Name of attribute.
	Expr *expr;			// What to assign.
public:
	Att_action_spec(char *nm, Expr *e) : att_name(nm), expr(e)
		{  }
	virtual void translate(Npc_spec *npc_spec,
				Fsm_state *& state, Action *& action);
	};

/*
 *	Routines:
 */
Npc_spec *Create_npc
	(
	char *nm,			// Name (already allocated).
	Slist *options,			// Shape, etc.
	Slist *stmts			// Atts., topics.
	);

/*
 *	An attribute to be resolved:
 */
class Att_name_expr : public Expr
	{
	char *npc;			// Npc name.
	char *att;			// Attribute name.
public:
	Att_name_expr(char *n, char *a) : npc(n), att(a)
		{  }
	virtual Expr *translate();	// Translate when compiling.
	virtual int eval();		// Evaluate (to an integer).
	};
