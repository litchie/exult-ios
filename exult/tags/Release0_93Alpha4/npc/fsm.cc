/**
 **	Fsm.cc - A finite-state-machine.
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

#include <string.h>
#include "fsm.h"
#include "npc.h"
#include "cond.h"

/*
 *	Add a transtition.
 */

void Fsm_state::add_transition
	(
	Fsm_transition *trans,
	Vector& tlist			// In or out.
	)
	{
	tlist.append(trans);		// Forget empty slots for now.  We
					//   may use a linked list instead
					//   anyway.
	}

/*
 *	Remove a transition.
 */

void Fsm_state::remove_transition
	(
	Fsm_transition *trans,
	Vector& tlist			// In or out.
	)
	{
	int cnt = tlist.get_cnt();	// Got to look for it.
	for (int i = 0; i < cnt; i++)
		if (trans == tlist.get(i))
			{
			tlist.put(i, 0);
			return;
			}
	}

/*
 *	Create a state.
 */

Fsm_state::Fsm_state
	(
	char *nm
	) : name(strdup(nm)), data(0)
	{
	}

/*
 *	Delete a state, along with any transitions in or out.
 */

Fsm_state::~Fsm_state
	(
	)
	{
	delete name;
	int tcnt = out.get_cnt();	// Get transitions going out.
	int i;
	for (i = 0; i < tcnt; i++)
		{
		Fsm_transition *trans = (Fsm_transition *) out.get(i);
		if (trans)
			{
			trans->get_to()->remove_transition_in(trans);
			delete trans;
			}
		}
	tcnt = in.get_cnt();		// Get transitions going in.
	for (i = 0; i < tcnt; i++)
		{
		Fsm_transition *trans = (Fsm_transition *) in.get(i);
		if (trans)
			{
			trans->get_from()->remove_transition_out(trans);
			delete trans;
			}
		}
	}

/*
 *	Execute actions when following transition.
 */

void Fsm_transition::execute_actions
	(
	)
	{
	int cnt = actions.get_cnt();
	for (int i = 0; i < cnt; i++)
		{
		Action *act = (Action *) actions.get(i);
		act->execute(fsm->npc);
		}
	}

/*
 *	Delete a transition.  We assume it's already been removed.
 */

Fsm_transition::~Fsm_transition
	(
	)
	{
	delete cond;
	// ++++++++++Delete actions?
	}

/*
 *	Handle a spoken sentence.
 *
 *	Output:	1 if handled, else 0.
 */

int Fsm_transition::handle_sentence
	(
	Sentence *spoken		// Sentence that was spoken.
	)
	{
	if (!cond->is_true(spoken))
		return (0);
	execute_actions();		// Do any actions.
	fsm->set_cur_state(to);		// Go to next state.
	return (1);
	}

/*
 *	Handle the start of a conversation.
 */

void Fsm_transition::handle_start
	(
	)
	{
	if (!cond->is_true_at_start())
		return;
	execute_actions();		// Do any actions.
	fsm->set_cur_state(to);		// Go to next state.
	}

/*
 *	Follow if condition is true.
 */

void Fsm_transition::handle
	(
	)
	{
	if (!cond->is_true())
		return;
	execute_actions();		// Do any actions.
	fsm->set_cur_state(to);		// Go to next state.
	}

/*
 *	Create an FSM.
 */

Fsm::Fsm
	(
	Npc *n,				// Npc this will belong to.
	char *nm
	) : npc(n), states(0, 3), name(strdup(nm)), cur_state(0)
	{
	}

/*
 *	Delete an FSM and its states.
 */

Fsm::~Fsm
	(
	)
	{
	delete name;
	int cnt = states.get_cnt();
	for (int i = 0; i < cnt; i++)
		{
		Fsm_state *state = (Fsm_state *) states.get(i);
		delete state;
		}
	}

/*
 *	Sensitize the NPC to a given state.
 */

void Fsm::sensitize_npc
	(
	Fsm_state *s
	)
	{
	int cnt = s->out.get_cnt();	// Go through outward transitions.
	for (int i = 0; i < cnt; i++)
		{
		Fsm_transition *tr = (Fsm_transition *) s->out.get(i);
					// Add to NPC's lists.
		tr->cond->sensitize_npc(npc, tr);
		}
	}

/*
 *	Desensitize the NPC to a given state.
 */

void Fsm::desensitize_npc
	(
	Fsm_state *s
	)
	{
	int cnt = s->out.get_cnt();
	for (int i = 0; i < cnt; i++)
		{
		Fsm_transition *tr = (Fsm_transition *) 
					s->out.get(i);
		tr->cond->desensitize_npc(npc, tr);
		}
	}

/*
 *	Find a state by name.
 */

Fsm_state *Fsm::find_state
	(
	char *nm
	)
	{
	int cnt = states.get_cnt();
	for (int i = 0; i < cnt; i++)
		{
		Fsm_state *state = (Fsm_state *) states.get(i);
		if (state && strcmp(nm, state->name) == 0)
			return (state);
		}
	return (0);
	}

/*
 *	Set current state, remove sentences from the NPC that were there
 *	for the previous state, and add new sentence sensitivities 
 *	to the NPC that owns us.
 */

void Fsm::set_cur_state
	(
	Fsm_state *s
	)
	{
	int cnt;
	if (cur_state)			// Remove sent. for prev. state.
		desensitize_npc(cur_state);
	sensitize_npc(s);		// Add sentences for new state.
	cur_state = s;
	npc->check_sensitivity();	// Check for transitions' truth.
	}

/*
 *	Activate an FSM for, i.e., starting a conversation topic.
 */

void Fsm::activate
	(
	Fsm_state *s			// State to set to, or 0 for current.
	)
	{
	if (s)
		cur_state = s;
	sensitize_npc(cur_state);	// Add sentences.
	npc->check_sensitivity();	// Check for transitions' truth.
	}

/*
 *	Deactivate an FSM (for ending a conversation topic).
 */

void Fsm::deactivate
	(
	)
	{
	desensitize_npc(cur_state);
	}

/*
 *	Change the conversation topic.
 */

void Conversation_topic_action::execute
	(
	Npc *npc
	)
	{
	npc->new_topic(topic);
	}

/*
 *	Return to previous conversation topic.
 */

void Conversation_prev_action::execute
	(
	Npc *npc
	)
	{
	npc->prev_topic();
	}

/*
 *	Modify an attribute.
 */

void Npc_attribute_action::execute
	(
	Npc *
	)
	{
	int ival = val->eval();		// Evaluate expr.
	att->set_val(ival);
	}
