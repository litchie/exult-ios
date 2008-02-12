/**
 **	basic_block.cc - Basic block analysis for usecode compiler.
 **
 **	Written: 2/7/08
 **/

/*
Copyright (C) 2008 The Exult Team

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

#ifndef INCL_BASICBLOCK

#include <vector>
#include <set>
#include <iostream>
#include <iomanip>
#include "ucexpr.h"
using std::vector;
using std::set;
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::setw;
using std::setfill;

class Basic_block;

/*
 *	Class representing a single usecode instruction and its
 *	parameters, if any; for jump instructions, this does NOT
 *	include the instruction offset.
 */

class Opcode
	{
protected:
	char opcode;
	vector<char> params;
	bool is_jump;
public:
	Opcode(char op) : opcode(op)
		{
		switch (opcode)
			{
			case UC_LOOPTOP:	case UC_LOOPTOP32:
				is_jump = true; params.reserve(8); break;
			case UC_CMPS:		case UC_CMPS32:
				is_jump = true; params.reserve(2); break;
			case UC_CONVERSE:	case UC_CONVERSE32:	case UC_JNE:
			case UC_JNE32:		case UC_JMP:		case UC_JMP32:
			case UC_LOOPTOPS:	case UC_LOOPTOPS32:
				is_jump = true; break;
			case UC_PUSHB:
				is_jump = false; params.reserve(1); break;
			case UC_PUSHI:		case UC_PUSHS:		case UC_POP:
			case UC_PUSH:		case UC_POPSTATIC:	case UC_PUSHSTATIC:
			case UC_POPCLSVAR:	case UC_PUSHCLSVAR:	case UC_ADDSI:
			case UC_ADDSV:		case UC_CALLO:		case UC_CALLM:
			case UC_CALLE:		case UC_CALL:		case UC_ARRC:
			case UC_AIDX:		case UC_AIDXS:		case UC_POPARR:
			case UC_POPARRS:	case UC_PUSHF:		case UC_POPF:
			case UC_CLSCREATE:
				is_jump = false; params.reserve(2); break;
			case UC_CALLIS:		case UC_CALLI:
				is_jump = false; params.reserve(3); break;
			case UC_CALLMS:		case UC_PUSHI32:	case UC_PUSHS32:
			case UC_ADDSI32:	case UC_CALL32:		case UC_CALLE32:
				is_jump = false; params.reserve(4); break;
			default:
				is_jump = false; break;
			}
		}
	int get_opcode() const
		{ return opcode; }
	void WriteParam1(unsigned short val)
		{ Write1(params, val); }
	void WriteParam2(unsigned short val)
		{ Write2(params, val); }
	void WriteParam4(unsigned int val)
		{ Write4(params, val); }
	void write(vector<char>& out)
		{
		Write1(out, opcode);
		out.insert(out.end(), params.begin(), params.end());
		}
#if 0	// For debugging.
	void write_text() const
		{
		cout << setw(2) << setfill('0') << (int)((unsigned char)opcode) << ' ';
		for (vector<char>::const_iterator it = params.begin();
				it != params.end(); ++it)
			cout << setw(2) << setfill('0') << (int)((unsigned char)*it) << ' ';
		if (is_jump)
			cout << "<offset>";
		}
#endif
	int get_size() const	// Total size, including offset.
		{
		int size = 1 + params.size();
		if (is_32bit())
			return size + 4;	// +4 for 32-bit offset.
		else if (is_jump)
			return size + 2;	// +2 for 16-bit offset.
		else
			return size;
		}
	bool is_32bit() const	// Only matters for jumps.
		{ return is_jump && (opcode & 0x80) != 0; }
	void set_32bit()		// Only matters for jumps.
		{ if (is_jump) opcode |= 0x80; }
	};

/*
 *	A class representing a basic block. A basic block is a group of
 *	statements/instructions with the following properties:
 *		(1) There is only one entrance to the block;
 *		(2) There is only one exit from the block.
 *	Thus, when a basic block is being executed, whenever the first
 *	instruction of the block is executed, all the instructions of
 *	of that block will also be executed.
 *
 *	A basic block must be created when
 *		(1) The given instruction is a jump target;
 *		(2) the previous block reached its end by means of a
 *		    branching instruction or a return.
 */

class Basic_block
	{
	friend void PopOpcode(Basic_block *dest);
	friend void WriteOp(Basic_block *dest, unsigned short val);
	friend void WriteOpParam1(Basic_block *dest, unsigned short val);
	friend void WriteOpParam2(Basic_block *dest, unsigned short val);
	friend void WriteOpParam4(Basic_block *dest, unsigned int val);
	friend void WriteJumpParam2(Basic_block *dest, unsigned short val);
	friend void WriteJumpParam4(Basic_block *dest, unsigned int val);
protected:
	set<Basic_block *> predecessors;		// Blocks that came before this.
	int index;				// Block index in the fun_blocks array *or*
				// -1 for the starting ("phantom") block.

	Basic_block *taken;		// For conditional jumps, block taken to by true
				// condition (fall-through); for unconditional jumps, block
				// to jump to; for all others, the next (fall-through) block.
	int taken_index;		// Index in the fun_blocks array, filled after
				// cleaning up unreachable blocks.
	Basic_block *ntaken;	// For conditional jumps, block taken to by false
				// condition (jump); for all others, this is zero.
	int ntaken_index;		// Index in the fun_blocks array, filled after
				// cleaning up unreachable blocks.

	Opcode *jmp_op;	// 0 for no instruction (fall-through) to taken
				// block; otherwise, one of:
				//	conditional jumps:
				//		UC_JNE, UC_CMPS, UC_CONVERSE, UC_LOOPTOP, UC_LOOPTOPS
				//	unconditional jumps:
				//		UC_JMP
				// or the 32-bit versions of these instructions.

	vector<Opcode *> instructions;	// The instructions of the block
	bool reachable;
public:
	Basic_block()
		:	index(0), taken(0), ntaken(0), taken_index(-1), ntaken_index(-1),
			jmp_op(0), reachable(false)
		{ instructions.reserve(100); }
	Basic_block(int ind, Basic_block *t = 0, Basic_block *n = 0, int ins = -1)
		:	index(ind), taken(t), ntaken(n), taken_index(-1), ntaken_index(-1),
			jmp_op(new Opcode(ins)), reachable(false)
		{ if (index != -1) instructions.reserve(100); }
	~Basic_block()
		{
		predecessors.clear();
		for (vector<Opcode *>::iterator it = instructions.begin();
				it != instructions.end(); ++it)
			delete *it;
		instructions.clear();
		delete jmp_op;
		}
	int get_index() const
		{ return index; }
	void set_index(int ind)
		{ index = ind; }
	int get_opcode() const
		{ return jmp_op ? jmp_op->get_opcode() : -1; }
	void clear_jump()
		{ delete jmp_op; jmp_op = 0; }
	int get_last_instruction() const
		{ return instructions.size() ? instructions.back()->get_opcode() : -1; }
	void set_32bit_jump()
		{ if (jmp_op) jmp_op->set_32bit(); }
	bool is_32bit_jump()
		{ return jmp_op ? jmp_op->is_32bit() : false; }
	bool does_not_jump() const
		{ return jmp_op == 0; }
	int get_jump_size() const
		{
		if (!jmp_op)
			return 0;	// Fall-through block or terminating block
		else
			return jmp_op->get_size();
		}
	int get_block_size() const
		{
		int size = get_jump_size();
		for (vector<Opcode *>::const_iterator it = instructions.begin();
				it != instructions.end(); ++it)
			{
			const Opcode *op = *it;
			size += op->get_size();
			}
		return size;
		}
	bool is_jump_block() const
		{ return jmp_op ? (jmp_op->get_opcode() == UC_JMP) : false; }
	bool is_simple_jump_block() const
		{ return is_jump_block() && instructions.empty(); }
	bool is_conditionaljump_block() const
			// instructions shouldn't be empty in this case.
		{ return jmp_op ? (jmp_op->get_opcode() == UC_JNE) : false; }
	bool is_fallthrough_block() const
		{ return !jmp_op && taken; }
	bool is_empty_block() const
		{ return !jmp_op && (instructions.empty()); }
	bool is_end_block() const
		{ return !jmp_op && taken && (taken->index == -1); }
	bool is_childless() const
		{ return !taken && !ntaken; }
	bool no_parents() const
		{ return predecessors.empty(); }
	bool is_orphan() const
		{ return index >= 0 && no_parents(); }
	bool is_reachable() const
		{ return reachable; }
	void mark_reachable()
		{
		if (reachable)
			return;
		reachable = true;
		if (taken)
			taken->mark_reachable();
		if (ntaken)
			ntaken->mark_reachable();
		}
	Basic_block *get_taken()
		{ return taken; }
	Basic_block *get_ntaken()
		{ return ntaken; }
	int get_taken_index()
		{ return taken_index; }
	int get_ntaken_index()
		{ return ntaken_index; }
	bool has_single_predecessor() const
		{ return predecessors.size() == 1; }
	void set_taken(Basic_block *dest)
		{
		if (dest)
			dest->predecessors.insert(this);
		if (taken)
			taken->predecessors.erase(this);
		taken = dest;
		}
	void set_ntaken(Basic_block *dest)
		{
		if (dest)
			dest->predecessors.insert(this);
		if (ntaken)
			ntaken->predecessors.erase(this);
		ntaken = dest;
		}
	void set_targets(int op, Basic_block *t = 0, Basic_block *n = 0)
		{
		clear_jump();
		if (op != -1)
			jmp_op = new Opcode(op);
		set_taken(t);
		set_ntaken(n);
		}
	void unlink_descendants()
		{
		if (taken)
			{
			taken->predecessors.erase(this);
			taken = 0;
			}
		if (ntaken)
			{
			ntaken->predecessors.erase(this);
			ntaken = 0;
			}
		}
	void link_predecessors()
		{
		for (set<Basic_block *>::iterator it = predecessors.begin();
				it != predecessors.end(); ++it)
			{
			Basic_block *block = *it;
			if (block->taken == this)
				block->taken_index = index;
			if (block->ntaken == this)
				block->ntaken_index = index;
			}
		}
	void unlink_predecessors()
		{
		for (set<Basic_block *>::iterator it = predecessors.begin();
				it != predecessors.end(); ++it)
			{
			Basic_block *block = *it;
			if (block->taken == this)
				{
				block->taken = 0;
				block->taken_index = -1;
				}
			if (block->ntaken == this)
				{
				block->ntaken = 0;
				block->ntaken_index = -1;
				}
			}
		predecessors.clear();
		}
	void link_through_block()
		{
		for (set<Basic_block *>::iterator it = predecessors.begin();
				it != predecessors.end(); ++it)
			{
			Basic_block *pred = *it;
				// Do NOT use set_taken, set_ntaken!
			if (pred->taken == this)
				{
				if (taken)	// Check almost unneeded.
					taken->predecessors.insert(pred);
				pred->taken = taken;
				}
			if (pred->ntaken == this)
				{
				if (ntaken)
					{
					ntaken->predecessors.insert(pred);
					pred->ntaken = ntaken;
					}
				else if (taken)
					{
					taken->predecessors.insert(pred);
					pred->ntaken = taken;
					}
				else
					pred->ntaken = 0;
				}
			}
		predecessors.clear();
		unlink_descendants();
		}
	void merge_taken()
		{
		Basic_block *safetaken = taken;
		instructions.insert(instructions.end(), safetaken->instructions.begin(),
				safetaken->instructions.end());
		delete jmp_op;
		jmp_op = safetaken->jmp_op;
		set_taken(safetaken->taken);
		set_ntaken(safetaken->ntaken);
			// Prevent these from being deleted.
		safetaken->instructions.clear();
		safetaken->jmp_op = 0;
		}
	void write(vector<char>& out)
		{
		for (vector<Opcode *>::iterator it = instructions.begin();
				it != instructions.end(); ++it)
			{
			Opcode *op = *it;
			op->write(out);
			}
		if (jmp_op)
			jmp_op->write(out);
		}
#if 0	// For debugging.
	void check() const
		{
		cout << hex << setw(8) << setfill('0') << (int)(this)
			 << '\t' << setw(8) << setfill('0') << (int)(taken)
			 << '\t' << setw(8) << setfill('0') << (int)(ntaken)
			 << '\t';
		for (vector<Opcode *>::const_iterator it = instructions.begin();
				it != instructions.end(); ++it)
			(*it)->write_text();
		if (jmp_op)
			jmp_op->write_text();
		else
			cout << "<no jump>";
		cout << endl;
		}
#endif
	};

/*
 *	Removes last opcode from the instruction block.
 */

inline void PopOpcode(Basic_block *dest)
	{
	if (!dest->instructions.size())
		return;
	Opcode *op = dest->instructions.back();
	dest->instructions.pop_back();
	delete op;
	}

/*
 *	Write an opcode value to the end of the instruction block
 *	and marks it as the last opcode.
 */

inline void WriteOp(Basic_block *dest, unsigned short val)
	{
	Opcode *op = new Opcode(val);
	dest->instructions.push_back(op);
	}

/*
 *	Write a 1-byte value to the end of the jump parameters.
 */

inline void WriteOpParam1(Basic_block *dest, unsigned short val)
	{
	if (!dest->instructions.size())
		return;
	dest->instructions.back()->WriteParam1(val);
	}

/*
 *	Write a 2-byte value to the end of the jump parameters.
 */

inline void WriteOpParam2(Basic_block *dest, unsigned short val)
	{
	if (!dest->instructions.size())
		return;
	dest->instructions.back()->WriteParam2(val);
	}

/*
 *	Write a 4-byte value to the end of the jump parameters.
 */

inline void WriteOpParam4(Basic_block *dest, unsigned int val)
	{
	if (!dest->instructions.size())
		return;
	dest->instructions.back()->WriteParam4(val);
	}

/*
 *	Write a 2-byte value to the end of the jump parameters.
 */

inline void WriteJumpParam2(Basic_block *dest, unsigned short val)
	{
	if (!dest->jmp_op)
		return;
	dest->jmp_op->WriteParam2(val);
	}

/*
 *	Write a 4-byte value to the end of the jump parameters.
 */

inline void WriteJumpParam4(Basic_block *dest, unsigned int val)
	{
	if (!dest->jmp_op)
		return;
	dest->jmp_op->WriteParam4(val);
	}

#endif
