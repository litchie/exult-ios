/**
 **	Cond.h - Conditions on FSM edges.
 **
 **	Written: 6/1/99 - JSF
 **/

#ifndef INCL_COND
#define INCL_COND	1

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
class Npc;
class Npc_attribute;
class Fsm_transition;
class Expr;

/*
 *	A condition.
 */
class Condition
	{
public:
					// Is this true when a given sentence
					//   has just been spoken?
	virtual int is_true(Sentence *spoken);
	virtual int is_true();		// Is this just true?
	virtual int is_true_at_start();	// True at start of conversation?
					// Add to NPC's lists.
	virtual void sensitize_npc(Npc *npc, Fsm_transition *owner);
					// Remove.
	virtual void desensitize_npc(Npc *npc, Fsm_transition *owner);
	};

/*
 *	Waiting for a given sentence to be spoken.
 */
class Sentence_condition : public Condition
	{
	Sentence *sentence;
public:
	Sentence_condition(Sentence *s) : sentence(s)
		{  }
					// Is this true when a given sentence
					//   has just been spoken?
	virtual int is_true(Sentence *spoken);
					// Add to NPC's lists.
	virtual void sensitize_npc(Npc *npc, Fsm_transition *owner);
					// Remove.
	virtual void desensitize_npc(Npc *npc, Fsm_transition *owner);
	};

/*
 *	This condition is true at the start of a conversation topic.
 */
class Start_condition : public Condition
	{
public:
	virtual int is_true_at_start();	// True at start of conversation?
					// Add to NPC's lists.
	virtual void sensitize_npc(Npc *npc, Fsm_transition *owner);
					// Remove.
	virtual void desensitize_npc(Npc *npc, Fsm_transition *owner);
	};
#if 0
/*
 *	Compare and Npc_attribute against a constant.
 */
class Npc_attribute_condition : public Condition
	{
	Npc_attribute *att;
	Op op;				// Operator: = ! < > .
	int value;			// What to compare against.
public:
	Npc_attribute_condition(Npc_attribute *a, Op o, int v)
		: att(a), op(o), value(v)
		{  }
	virtual int is_true();
					// Add to NPC's lists.
	virtual void sensitize_npc(Npc *npc, Fsm_transition *owner);
					// Remove.
	virtual void desensitize_npc(Npc *npc, Fsm_transition *owner);
	};
#endif

/*
 *	An true/false expression:
 */
class Expr_condition : public Condition
	{
	Expr *expr;			// What to evaluate.
public:
	Expr_condition(Expr *e) : expr(e)
		{  }
	virtual int is_true();
					// Add to NPC's lists.
	virtual void sensitize_npc(Npc *npc, Fsm_transition *owner);
					// Remove.
	virtual void desensitize_npc(Npc *npc, Fsm_transition *owner);
	};

/*
 *	Expressions:
 */
class Expr
	{
public:
	enum Op				// Operators.
		{
		eq = 0,			// Binary.
		neq = 1,
		lt = 2,
		lte = 3,
		gt = 4,
		gte = 5,
		plus = 6,
		minus = 7,
		mul = 8,
		div = 9,
		mod = 10,
		and = 50,		// Logical.
		or = 51,
		not = 100,		// Unary.
		neg = 101
		};
	virtual Expr *translate();	// Translate when compiling.
	virtual int eval() = 0;		// Evaluate (to an integer).
	};

/*
 *	An NPC's attribute value:
 */
class Npc_att_expr : public Expr
	{
	Npc_attribute *att;		// The attribute.
public:
	Npc_att_expr(Npc_attribute *a) : att(a)
		{  }
	virtual int eval();		// Evaluate (to an integer).
	};

/*
 *	An integer constant:
 */
class Int_expr : public Expr
	{
	int value;
public:
	Int_expr(int v) : value(v)
		{  }
	virtual int eval();		// Evaluate (to an integer).
	};

/*
 *	Unary expressions:
 */
class Unary_expr : public Expr
	{
	Op op;				// Operator.
	Expr *operand;			// Operand.
public:
	Unary_expr(Op o, Expr *ond) : op(o), operand(ond)
		{  }
	virtual Expr *translate();	// Translate when compiling.
	virtual int eval();		// Evaluate (to an integer).
	};

/*
 *	Binary expressions:
 */
class Binary_expr : public Expr
	{
	Op op;				// Operator.
	Expr *operand1, *operand2;	// Operands.
public:
	Binary_expr(Op o, Expr *ond1, Expr *ond2) : op(o), 
			operand1(ond1), operand2(ond2)
		{  }
	virtual Expr *translate();	// Translate when compiling.
	virtual int eval();		// Evaluate (to an integer).
	};




#endif
