/**
 **	Exec.cc - Execute actions in the script.
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

#include <stdio.h>
#include "fsm.h"
#include "exec.h"
#include "script.h"

extern void yyerror(char *);

/*
 *	An action to print a response:
 */
class Print_action : public Action
	{
	char *msg;			// What to print (without a NL).
public:
	Print_action(char *m) : msg(m)
		{  }
	virtual void execute(Npc *npc)
		{
		Npc_user *user = npc->get_user();
		user->show_response(msg);
		}
	};

/*
 *	Translate an NPC.
 */

void Npc_spec::translate
	(
	)
	{
	Slist_iterator next(stmts);	// Create attributes, topics.
	Npc_stmt *stmt;
	while ((stmt = (Npc_stmt *) next()) != 0)
		stmt->create(this);
	next.reset();			// Now translate.
	while ((stmt = (Npc_stmt *) next()) != 0)
		stmt->translate(this);
	}

/*
 *	Create an attribute.
 */

void Npc_att_stmt::create
	(
	Npc_spec *npc_spec
	)
	{
	npc_spec->add_attribute(name, min, max, val);
	}

/*
 *	Finish translating attribute.
 */

void Npc_att_stmt::translate
	(
	Npc_spec *npc_spec
	)
	{
	}

/*
 *	Create a topic FSM.
 */

void Npc_topic_stmt::create
	(
	Npc_spec *npc_spec
	)
	{
	topic = npc_spec->add_fsm(name);
	Slist_iterator next(states);	// Go through states.
	State_spec *each;		// Create them.
	while ((each = (State_spec *) next()) != 0)
		each->create(topic);
	}

/*
 *	Finish translating a topic.
 */

void Npc_topic_stmt::translate
	(
	Npc_spec *npc_spec
	)
	{
	npc_spec->set_cur_fsm(topic);	// This is what we're doing.
	Slist_iterator next(states);	// Go through states.
	State_spec *each;		// Translate transitions from them.
	while ((each = (State_spec *) next()) != 0)
		each->translate(npc_spec);
					// Init. to first state.
	each = (State_spec *) states->get_first();
	if (each)
		topic->set_cur_state(each->get_state());
	}

/*
 *	Create state specification.
 */

State_spec::State_spec
	(
	char *nm,			// Name.
	Slist *r			// List of Response_spec's.
	) : responses(r), state(0), name(nm)
	{
	}

/*
 *	Create the state.
 */

void State_spec::create
	(
	Fsm *fsm
	)
	{
					// Create state node.
	state = fsm->add_state(name);
	}

/*
 *	Handle a state specification.
 */

void State_spec::translate
	(
	Npc_spec *npc_spec
	)
	{
	Fsm *fsm = npc_spec->get_cur_fsm();
	Slist_iterator next(responses);	// Create transitions.
	Response_spec *each;
	while ((each = (Response_spec *) next()) != 0)
		each->translate(npc_spec, state);
	}

/*
 *	Create a transition.
 */

void Response_spec::translate
	(
	Npc_spec *npc_spec,
	Fsm_state *from			// State to leave.
	)
	{
					// Get actual condition.
	Condition *cond = cond_spec ? cond_spec->translate() : 0;
	Fsm_state *to = 0;
	Slist_iterator next(actions);	// Go through actions.
	Action_spec *each;
	Slist real_actions;		// Store non-state actions here.
	while ((each = (Action_spec *) next()) != 0)
		{
		Fsm_state *state;	// For state change.
		Action *action;		// For action.
		each->translate(npc_spec, state, action);
		if (state)
			to = state;
		if (action)
			real_actions.append(action);
		}
	if (!to)			// No state trans. specified?
		to = from;
	Fsm_transition *trans = npc_spec->get_cur_fsm()->add_transition(
							cond, from, to);
	Action *act;
	while ((act = (Action *) real_actions.remove_first()) != 0)
		trans->add_action(act);
	}

/*
 *	Translate an expression.
 */

Condition *Expr_cond_spec::translate
	(
	)
	{
	Expr *new_expr = expr->translate();
	if (new_expr != expr)
		{
		delete expr;
		expr = new_expr;
		}
	return (new Expr_condition(expr));
	}

/*
 *	Translate a condition.
 */

Condition *Cond_known_spec::translate
	(
	)
	{
	return (cond);			// Already know it.
	}

/*
 *	Say something.
 */

void Say_action_spec::translate
	(
	Npc_spec *npc_spec,
	Fsm_state *& state,		// New state returned.
	Action *& action		// Actual action returned.
	)
	{
	state = 0;
	action = new Print_action(str);
	}

/*
 *	Switch topics.
 */

void Topic_action_spec::translate
	(
	Npc_spec *npc_spec,
	Fsm_state *& state,		// New state returned.
	Action *& action		// Actual action returned.
	)
	{
	state = 0;
	Fsm *fsm = npc_spec->find_fsm(topic);
	if (!fsm)
		{
		//+++++++++Error.
		action = 0;
		}
	else
		action = new Conversation_topic_action(fsm);
	}

/*
 *	Switch to prev. topic.
 */

void Topic_prev_spec::translate
	(
	Npc_spec *npc_spec,
	Fsm_state *& state,		// New state returned.
	Action *& action		// Actual action returned.
	)
	{
	state = 0;
	action = new Conversation_prev_action;
	}

/*
 *	Switch to a new state.
 */

void State_action_spec::translate
	(
	Npc_spec *npc_spec,
	Fsm_state *& state,		// New state returned.
	Action *& action		// Actual action returned.
	)
	{
	action = 0;
	Fsm *fsm = npc_spec->get_cur_fsm();
	state = fsm->find_state(state_name);
	if (!state)
		{
		char buf[256];
		sprintf(buf, "Can't find state '%s' in FSM '%s'.",
					state_name, fsm->get_name());
		yyerror(buf);
		}
	}

/*
 *	Look up an NPC in our list and print an error if not found.
 */

static Npc_spec *Need_npc
	(
	char *nm
	)
	{
	extern Npc_spec *Find_npc(char *npc);
	Npc_spec *spec = Find_npc(nm);	// Find the NPC.
	char buf[250];			// For errors.
	if (!spec)
		{
		sprintf(buf, "Can't find NPC '%s'.", nm);
		yyerror(buf);
		}
	return (spec);
	}

/*
 *	Find an attribute, and print an error if not found.
 */

static Npc_attribute *Need_att
	(
	Npc_spec *spec,
	char *att			// Name of attribute.
	)
	{
					// Find its attribute.
	Npc_attribute *natt = spec->get_npc()->find_attribute(att);
	if (!natt)
		{
		char buf[250];
		sprintf(buf, "Can't find att. '%s' for '%s'.", att, 
					spec->get_npc()->get_name());
		yyerror(buf);
		}
	return (natt);
	}

/*
 *	Assign to an attribute.
 */

void Att_action_spec::translate
	(
	Npc_spec *npc_spec,
	Fsm_state *& state,		// New state returned.
	Action *& action		// Actual action returned.
	)
	{
	state = 0;			// Not changing state.
	Npc_attribute *att = Need_att(npc_spec, att_name);
					// Translate assigned value.
	Expr *new_expr = expr->translate();
	if (new_expr != expr)
		{
		delete expr;
		expr = new_expr;
		}
	action = new Npc_attribute_action(att, expr);
	}

/*
 *	Create an NPC specification.
 */

Npc_spec *Create_npc
	(
	char *nm,			// Name (already allocated).
	Slist *options,			// Shape, etc.
	Slist *stmts			// Atts., topics.
	)
	{
					// Need to get these.
	long shape = -1, portrait = -1, location = -1;
	Slist_iterator next(options);
	Npc_option *opt;
	while ((opt = (Npc_option *) next()) != 0)
		{
		switch (opt->token)
			{
		case SHAPE:
			shape = opt->value; break;
		case PORTRAIT:
			portrait = opt->value; break;
		case LOCATION:
			location = opt->value; break;
			}
		}
	char buf[256];			// For errors.
	if (shape == -1)
		{
		sprintf(buf, "Need to specify shape for NPC '%s'.", nm);
		yyerror(buf);
		return (0);
		}
	if (location == -1)
		{
		sprintf(buf, "Need to specify location for NPC '%s'.", nm);
		yyerror(buf);
		return (0);
		}
					// Get coords.
	int x = (location >> 16) & 0xffff, y = location & 0xffff;
	int cx = x/16, cy = y/16;
	int sx = x%16, sy = y%16;
	Npc_spec *npc_spec = new Npc_spec(new Npc(nm), stmts, 
		(int) shape, (int) portrait, cx, cy, sx, sy);
	return (npc_spec);
	}

/*
 *	Translate to the actual expression.
 */

Expr *Att_name_expr::translate
	(
	)
	{
	extern Npc_spec *Find_npc(char *npc);
	Npc_spec *spec = Need_npc(npc);	// Find the NPC.
	if (!spec)
		return (new Int_expr(0));
					// Find its attribute.
	Npc_attribute *natt = Need_att(spec, att);
	if (!natt)
		return (new Int_expr(0));
	return (new Npc_att_expr(natt));
	}

/*
 *	Should never get called.
 *
 *	Output:	0
 */

int Att_name_expr::eval
	(
	)
	{
	return (0);
	}

