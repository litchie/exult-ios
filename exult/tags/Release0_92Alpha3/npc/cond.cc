/**
 **	Cond.cc - Conditions on FSM edges.
 **
 **	Written: 6/1/99 - JSF
 **/

#include "cond.h"
#include "npc.h"
#include "fsm.h"
#include "convers.h"

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
 *	Default methods:
 */

int Condition::is_true
	(
	Sentence *
	)
	{
	return (0);
	}
int Condition::is_true
	(
	)
	{
	return (0);
	}
int Condition::is_true_at_start
	(
	)
	{
	return (0);
	}
void Condition::sensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	}
void Condition::desensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	}

/*
 *	Is this true when a sentence was spoken?
 */

int Sentence_condition::is_true
	(
	Sentence *spoken
	)
	{
	return (sentence == spoken);
	}

/*
 *	Sensitize an NPC to the owner of this condition.
 */

void Sentence_condition::sensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	if (owner->get_fsm() == npc->get_topic())
		npc->add_sentence(sentence, owner);
	}

/*
 *	Remove from NPC's sensitivity list(s).
 */

void Sentence_condition::desensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	npc->remove_sentence_handler(owner);
	}

/*
 *	Is this true at the start of a conversation?
 */

int Start_condition::is_true_at_start
	(
	)
	{
	return (1);			// Yes, always.
	}

/*
 *	Sensitize an NPC to the owner of this condition.
 */

void Start_condition::sensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	if (owner->get_fsm() == npc->get_topic())
		npc->add_start_handler(owner);
	}

/*
 *	Remove from NPC's sensitivity list(s).
 */

void Start_condition::desensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	npc->remove_start_handler(owner);
	}

/*
 *	Is this true?
 */
int Expr_condition::is_true
	(
	)
	{
	return (expr->eval() != 0);
	}

/*
 *	Sensitize an NPC to the owner of this condition.
 */

void Expr_condition::sensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	npc->add_to_sensitivity_list(owner);
	}

/*
 *	Remove from NPC's sensitivity list(s).
 */

void Expr_condition::desensitize_npc
	(
	Npc *npc,
	Fsm_transition *owner		// Who owns this condition.
	)
	{
	npc->remove_from_sensitivity_list(owner);
	}

/*
 *	Default translator:
 */

Expr *Expr::translate
	(
	)
	{
	return (this);
	}

/*
 *	Evaluate.
 *
 *	Output:	Value of attribute.
 */

int Npc_att_expr::eval
	(
	)
	{
	return (att->get_val());
	}

/*
 *	Evaluate.
 */

int Int_expr::eval
	(
	)
	{
	return (value);
	}

/*
 *	Translate during script compilation.
 */

Expr *Unary_expr::translate
	(
	)
	{
	Expr *new_op = operand->translate();
	if (new_op != operand)
		{
		delete operand;
		operand = new_op;
		}
	return (this);
	}


/*
 *	Evaluate.
 */

int Unary_expr::eval
	(
	)
	{
	int opval = operand->eval();
	switch(op)
		{
	not:
		return (opval ? 0 : 1);
	neg:
		return (-opval);
	default:
		return (opval);
		}
	}

/*
 *	Translate during script compilation.
 */

Expr *Binary_expr::translate
	(
	)
	{
	Expr *new_op1 = operand1->translate();
	Expr *new_op2 = operand2->translate();
	if (new_op1 != operand1)
		{
		delete operand1;
		operand1 = new_op1;
		}
	if (new_op2 != operand2)
		{
		delete operand2;
		operand2 = new_op2;
		}
	return (this);
	}

/*
 *	Evaluate.
 */

int Binary_expr::eval
	(
	)
	{
	int opval1 = operand1->eval();
	int opval2 = operand2->eval();
	switch(op)
		{
	case eq:
		return (opval1 == opval2);
	case neq:
		return (opval1 != opval2);
	case lt:
		return (opval1 < opval2);
	case lte:
		return (opval1 <= opval2);
	case gt:
		return (opval1 > opval2);
	case gte:
		return (opval1 >= opval2);
	case plus:
		return (opval1 + opval2);
	case minus:
		return (opval1 - opval2);
	case mul:
		return (opval1 * opval2);
	case div:
		return (opval2 ? opval1/opval2 : 16000);
	case mod:
		return (opval2 ? opval1 % opval2 : 0);
	case and:
		return (opval1 != 0 && opval2 != 0);
	case or:
		return (opval1 != 0 || opval2 != 0);
	default:
		return (0);
		}
	}

