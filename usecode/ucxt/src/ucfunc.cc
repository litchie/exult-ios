/*
 *  Copyright (C) 2001-2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ucdata.h"
#include "ucfunc.h"
#include <set>
#include <strstream>
#include <algorithm>
#include <iomanip>

#include "opcodes.h"

#if 0
	#define DEBUG_INDENT
	#define DEBUG_PARSE
	#define DEBUG_PARSE2
	#define DEBUG_PARSE2a
	#define DEBUG_READ
	#define DEBUG_PRINT
	#define DEBUG_READ_PAIR(X, Y) cout << '\t' << X << '\t' << Y << endl;
#else
	#undef DEBUG_INDENT
	#undef DEBUG_PARSE
	#undef DEBUG_PARSE2
	#undef DEBUG_READ
	#undef DEBUG_PRINT
	#define DEBUG_READ_PAIR(X, Y)
#endif

//#define DEBUG_PARSE2
//#define DEBUG_PARSE2a
//#define DEBUG_PRINT

const string VARNAME = "var";
const string VARPREFIX = "var";
const unsigned int ASM_DISP_STR_LEN=20;

void print_asm_opcode(ostream &o, UCFunc &ucf, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const map<unsigned int, string> &intrinsics, const UCc &op);
string demunge_ocstring(UCFunc &ucf, const FuncMap &funcmap, const string &asmstr, const vector<unsigned int> &params, const map<unsigned int, string> &intrinsics, const UCc &op, bool ucs_output);

/* Assumption the 'var's are in their 'zeroed' state on initialization,
   unless something else is assigned to them. */

inline ostream &tab_indent(const unsigned int indent, ostream &o)
{
	#ifdef DEBUG_INDENT
	o << indent;
	#endif
	
	switch(indent)
	{
		case 0:                    break;
	 	case 1: o << '\t';         break;
	 	case 2: o << "\t\t";       break;
	 	case 3: o << "\t\t\t";     break;
	 	case 4: o << "\t\t\t\t";   break;
	 	case 5: o << "\t\t\t\t\t"; break;
	 	default:
			for(unsigned int i=0; i<indent; ++i) o << '\t';
			break;
	}
	return o;
}

void UCFunc::output_list(ostream &o, unsigned int funcno, bool debug)
{
	o << "#" << std::setbase(10) << std::setw(4) << funcno << std::setbase(16) << ": "
	  << (return_var ? '&' : ' ')
	  << std::setw(4) << _funcid    << "H  "
	  << std::setw(8) << _offset    << "  "
	  << std::setw(4) << _funcsize  << "  "
	  << std::setw(4) << _datasize  << "  "
	  << std::setw(4) << codesize() << "  ";
	
	if(debug)
		o << _data.find(0)->second;
	
	o << endl;
}

void UCFunc::output_ucs(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool uselesscomment)
{
	unsigned int indent=0;
	
	if(_externs.size()) tab_indent(indent, o) << "// externs" << endl;
	// output the 'externs'
	for(vector<unsigned short>::iterator e=_externs.begin(); e!=_externs.end(); e++)
	{
		FuncMap::const_iterator fmp = funcmap.find(*e);
		output_ucs_funcname(tab_indent(indent, o) << "extern ", funcmap, *e, fmp->second.num_args, fmp->second.return_var) << ';' << endl;
	}
	
	if(_externs.size()) o << endl;
	
	// output the function name
	output_ucs_funcname(tab_indent(indent, o), funcmap, _funcid, _num_args, return_var) << endl;
	// start of func
	tab_indent(indent++, o) << '{' << endl;
	
	for(unsigned int i=_num_args; i<_num_args+_num_locals; i++)
		tab_indent(indent, o) << VARNAME << ' ' << VARPREFIX << std::setw(4) << i << ';' << endl;
		
	if(return_var) tab_indent(indent, o) << VARNAME << ' ' << "rr" << ';' << endl;
	
	if(_num_locals>0) o << endl;
	
	output_ucs_data(o, funcmap, intrinsics, uselesscomment, indent);
	
	tab_indent(--indent, o) << '}' << endl;
}

/* outputs the general 'function name' in long format. For function
	declarations and externs */
ostream &UCFunc::output_ucs_funcname(ostream &o, const FuncMap &funcmap,
                                     unsigned int funcid,
                                     unsigned int numargs, bool return_var)
{
	// do we return a variable
	if(return_var) o << VARNAME << ' ';
	
	// output the "function name"
	// TODO: Probably want to grab this from a file in the future...
	//o << demunge_ocstring(*this, funcmap, "%f1", ucc._params_parsed, intrinsics, ucc, true)
	
	FuncMap::const_iterator fmp = funcmap.find(funcid);
	if(fmp->second.funcname.size())
	{
		if(fmp->second.funcname[0]=='&')
			o << fmp->second.funcname.substr(1, fmp->second.funcname.size()-1);
		else
			o << fmp->second.funcname;
	}
	else
		o << "Func" << std::setw(4) << funcid;
	
	// << "Func" << std::setw(4) << funcid
	// output the "function number"
	o << " 0x" << funcid
	// output ObCurly braces
	  << " (";
	
	for(unsigned int i=0; i<numargs; i++)
		o << VARNAME << ' ' << VARPREFIX << std::setw(4) << i << ((i==numargs-1) ? "" : ", ");
	
	o << ")";
	
	return o;
}

void UCFunc::output_ucs_data(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool uselesscomment, unsigned int indent)
{
	vector<unsigned int> labeltmp(1);
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); ++i)
	{
		// we don't want to output the first "jump" (the start of the function)
		if(i!=gotoset.begin())
		{
			labeltmp[0]=i->offset();
			tab_indent(indent++, o) << demunge_ocstring(*this, funcmap, "label%f*_%1:", labeltmp, intrinsics, UCc(), true) << endl;
		}
		
		for(GotoSet::iterator j=(*i)().begin(); j!=(*i)().end(); j++)
		{
			const UCc &ucc = *(j->first);
			
			if(uselesscomment)
				tab_indent(indent, o) << "// Offset: " << std::setw(4) << ucc._offset << endl;

			output_ucs_opcode(o, funcmap, opcode_table_data, ucc, intrinsics, indent);
		}
		if(i!=gotoset.begin()) --indent; //decrement it again to skip the label statement.
		
	}
}

void UCFunc::output_ucs_opcode(ostream &o, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const UCc &op, const map<unsigned int, string> &intrinsics, unsigned int indent)
{
	tab_indent(indent, o) << demunge_ocstring(*this, funcmap, optab[op._id].ucs_nmo, op._params_parsed, intrinsics, op, true) << ';' << endl;
	
	#ifdef DEBUG_PRINT
	for(vector<UCc *>::const_iterator i=op._popped.begin(); i!=op._popped.end(); i++)
	{
		if((*i)->_popped.size())
			output_ucs_opcode(o, funcmap, opcode_table_data, **i, intrinsics, indent+1);
		else
//			tab_indent(indent+1, o) << demunge_ocstring(*this, funcmap, optab[(*i)->_id].ucs_nmo, op._params_parsed, **i) << endl;
			tab_indent(indent+1, o) << optab[(*i)->_id].ucs_nmo << endl;
	}
	#endif
}

void UCFunc::output_ucs_node(ostream &o, const FuncMap &funcmap, UCNode* ucn, const map<unsigned int, string> &intrinsics, unsigned int indent)
{
	if(!ucn->nodelist.empty()) tab_indent(indent, o) << '{' << endl;
	
	if(ucn->ucc!=0)
		print_asm_opcode(tab_indent(indent, o), *this, funcmap, opcode_table_data, intrinsics, *(ucn->ucc));
	
	if(ucn->nodelist.size())
		for(vector<UCNode *>::iterator i=ucn->nodelist.begin(); i!=ucn->nodelist.end(); i++)
		{
			//tab_indent(indent, o);
			output_ucs_node(o, funcmap, *i, intrinsics, indent+1);
		}
			
	// end of func
	if(!ucn->nodelist.empty()) tab_indent(indent, o) << '}' << endl;
}

/* Just a quick function to remove all the ucc structured flagged as removable */
inline void gc_gotoset(vector<GotoSet> &gotoset)
{
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); i++)
	{
		i->gc();
		#ifdef DEBUG_GOTOSET
		cout << "----" << endl;
		#endif
	}
}

void UCFunc::parse_ucs(const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool basic)
{
	for(vector<UCc>::iterator i=_opcodes.begin(); i!=_opcodes.end(); i++)
		node.nodelist.push_back(new UCNode(&(*i)));
	
	parse_ucs_pass1(node.nodelist);
	parse_ucs_pass2(gotoset, funcmap, intrinsics);
	gc_gotoset(gotoset);
	
	if(!basic)
	{
		parse_ucs_pass3(gotoset, intrinsics);
	}
	
	#ifdef DEBUG_PARSE2
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); i++)
	{
		cout << std::setw(4) << i->offset() << endl;
		
		for(GotoSet::iterator j=(*i)().begin(); j!=(*i)().end(); j++)
		{
			cout << '\t' << std::setw(4) << j->first->_offset << '\t' << j->first->_id << endl;
		}
	}
	#endif
}

/* Pass 1 turns the 1-dimentional vector of opcodes, into a 2-dimentional array
   consisting of all the opcodes within two 'goto target offsets'. */
void UCFunc::parse_ucs_pass1(vector<UCNode *> &nodes)
{
	vector<unsigned int> jumps;

	// collect jump references
	for(unsigned int i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->ucc!=0)
		{
			unsigned int isjump=0;
			for(vector<pair<unsigned int, unsigned int> >::iterator op=opcode_jumps.begin(); op!=opcode_jumps.end(); op++)
				if(op->first==nodes[i]->ucc->_id)
				{
					isjump=op->second;
					break;
				}
			
			if(isjump!=0)
			{
				assert(nodes[i]->ucc->_params_parsed.size()>=isjump);
				jumps.push_back(nodes[i]->ucc->_params_parsed[isjump-1]);
			}
		}
	}

	gotoset.push_back(GotoSet());

	for(unsigned int i=0; i<nodes.size(); i++)
	{
		if(nodes[i]->ucc!=0)
		{
			if(count(jumps.begin(), jumps.end(), nodes[i]->ucc->_offset))
			{
				gotoset.push_back(nodes[i]->ucc);
			}
			else
				gotoset.back().add(nodes[i]->ucc);
		}
	}
}

/* In Pass 2 we convert our 2-dimensional 'GotoSet' array into an array with
   each UCc, having it's parameters sitting in it's UCc::_popped vector. Elements
   that are parameters are flagged for removal (Gotoset::()[i]->second=true) from
   the original GotoSet. */
void UCFunc::parse_ucs_pass2(vector<GotoSet> &gotoset, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics)
{
	for(vector<GotoSet>::iterator i=gotoset.begin(); i!=gotoset.end(); ++i)
	{
		parse_ucs_pass2a((*i)().rbegin(), (*i)(), 0, funcmap, intrinsics);
	}
}

extern UCData uc;

vector<UCc *> UCFunc::parse_ucs_pass2a(vector<pair<UCc *, bool> >::reverse_iterator current, vector<pair<UCc *, bool> > &vec, unsigned int opsneeded, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics)
{
	vector<UCc *> vucc;
	unsigned int opsfound=0;
	
	#ifdef DEBUG_PARSE2
	print_asm_opcode(tab_indent(4, cout), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
	#endif
	
	for(;vec.rend()!=current; current++)
	{
		#ifdef DEBUG_PARSE2
		print_asm_opcode(tab_indent(3, cout), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
		#endif
		
		if(current->second==false)
		{
			if((opcode_table_data[current->first->_id].num_pop!=0) || (opcode_table_data[current->first->_id].call_effect!=0))
			{
				//if(opcode_table_data[current->first->_id].num_pop<0x7F)
				{
					#ifdef DEBUG_PARSE2
					print_asm_opcode(tab_indent(3, cout << "0x" << std::setw(2) << current->first->_id << "-"), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
					tab_indent(3, cout << "0x" << std::setw(2) << current->first->_id << "-") << opcode_table_data[current->first->_id].num_pop << endl;
					#endif
					
					unsigned int num_args=0;
					
					if(opcode_table_data[current->first->_id].num_pop>0x7F)
					{
						#ifdef DEBUG_PARSE2a
						cout << "CALL EFFECT: " << opcode_table_data[current->first->_id].num_pop << '\t';
						#endif
						
						unsigned int offset = 0x100 - opcode_table_data[current->first->_id].num_pop - 1;
						num_args = current->first->_params_parsed[offset];
						
						#ifdef DEBUG_PARSE2a
						cout << num_args << endl;
						#endif
					}
					else if(opcode_table_data[current->first->_id].call_effect!=0)
					{
						assert(current->first->_params_parsed.size()>=1);
						assert(_externs.size()>=current->first->_params_parsed[0]);
						FuncMap::const_iterator fmp = funcmap.find(_externs[current->first->_params_parsed[0]]);
						assert(fmp!=funcmap.end());
						#ifdef DEBUG_PARSE2
						cout << "CALL:     " << fmp->second.funcid << '\t' << fmp->second.num_args << endl;
						#endif
						
						num_args = fmp->second.num_args;
					}
					else
					{
						#ifdef DEBUG_PARSE2
						cout << "Non-CALL: \t" << opcode_table_data[current->first->_id].num_pop << endl;
						#endif
						num_args = opcode_table_data[current->first->_id].num_pop;
					}
					
					if(num_args>0)
					{
						/* save the 'current' value as the return value and increment it so it's
						   pointing at the 'next' current value */
						vector<pair<UCc *, bool> >::reverse_iterator ret(current);
						
						ret->first->_popped = parse_ucs_pass2a(++current, vec, num_args, funcmap, intrinsics);
						
						assert(current!=ret);
						
						--current;
						
						assert(current==ret);
						#ifdef DEBUG_PARSE2a
						print_asm_opcode(tab_indent(1, cout), *this, funcmap, opcode_table_data, intrinsics, *(ret->first));
						
						for(vector<UCc *>::iterator i=ret->first->_popped.begin(); i!=ret->first->_popped.end(); i++)
							print_asm_opcode(tab_indent(2, cout), *this, funcmap, opcode_table_data, intrinsics, **i);
						#endif
					}
				}
			}
			if((opsneeded!=0) && (current->second==false))
			{
				// if it's a 'push' opcode and we need items to return that we've popped off the stack...
				if(opcode_table_data[current->first->_id].num_push!=0)
				{
					#ifdef DEBUG_PARSE2
					print_asm_opcode(tab_indent(4, cout << "P-"), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
					#endif
					
					opsfound+=opcode_table_data[current->first->_id].num_push;
					vucc.push_back(current->first);
					current->second=true;
				}
				// if it's a call to a function that returns a variable...
				else if(opcode_table_data[current->first->_id].call_effect!=0)
				{
					FuncMap::const_iterator fmp = funcmap.find(_externs[current->first->_params_parsed[0]]);
					assert(fmp!=funcmap.end());
					
					if(fmp->second.return_var)
					{
						#ifdef DEBUG_PARSE2
						print_asm_opcode(tab_indent(4, cout << "C-"), *this, funcmap, opcode_table_data, intrinsics, *(current->first));
						#endif
					
						opsfound+=1;
						vucc.push_back(current->first);
						current->second=true;
					}
				}
				else
					current->second=true;
					
				// if we've found all the ops we were searching for, return them
				if(opsfound>=opsneeded)
				{
					return vucc;
				}
			}
		}
	}
	
	if(vucc.size()>0) cout << "DID NOT FIND ALL OPCODE PARAMETERS." << endl;
	return vucc;
}

/* The 'optimisation' phase. Attempting to remove as many goto...labels as possible. */
void UCFunc::parse_ucs_pass3(vector<GotoSet> &gotoset, const map<unsigned int, string> &intrinsics)
{

}

inline unsigned short read_ushort(ifstream &f)
{
  return static_cast<unsigned short> (static_cast<unsigned int>(f.get()) + (static_cast<unsigned int>(f.get()) << 8));
}

inline unsigned char read_ubyte(ifstream &f)
{
  return static_cast<unsigned char>(f.get());
}

void ucc_parse_parambytes(UCc &ucop, const UCOpcodeData &otd);

void readbin_UCFunc(ifstream &f, UCFunc &ucf)
{

	// offset to start of function
	ucf._offset = f.tellg();
	DEBUG_READ_PAIR("Offset: ", ucf._offset);

	// Read Function Header
	
	ucf._funcid = read_ushort(f);
	DEBUG_READ_PAIR("FuncID: ", ucf._funcid);
	ucf._funcsize = read_ushort(f);	
	DEBUG_READ_PAIR("FuncSize: ", ucf._funcsize);
	
	// save body offset in case we need it
	ucf._bodyoffset = f.tellg();
	
	ucf._datasize = read_ushort(f);
	
	// process ze data segment!
	{
		streampos pos = f.tellg(); // paranoia
	
		unsigned short off = 0;
		// Load all strings & their offsets
		while( off < ucf._datasize )
		{
			assert(!f.eof());
	
			char c;
			string data;
	
			while((c=f.get())!=0x00)
				data+=c;
	
			ucf._data.insert(pair<unsigned int, string>(off, data));
	
			off+=data.size()+1;
	
		}
		f.seekg(pos, ios::beg); // paranoia
		f.seekg(ucf._datasize, ios::cur); // paranoia
	}

	#if 0	
	if(ucf._funcid==_search_func)
		for(map<unsigned int, string>::iterator i=ucf._data.begin(); i!=ucf._data.end(); i++)
			cout << i->first << "\t" << i->second << endl;
	#endif
	
	// process code segment
	{
		//streampos start_of_code_seg = f.tellg(); // what's this used for?
		ucf._codeoffset = f.tellg();

		// get the number of arguments to the function
		ucf._num_args = read_ushort(f);

		// get the number of local variables
		ucf._num_locals = read_ushort(f);

		// get the number of external function numbers
		ucf._num_externs = read_ushort(f);
		
		// load the external function numbers
		for(unsigned int i=0; i<ucf._num_externs; i++)
			ucf._externs.push_back(read_ushort(f));
		
		// ok, now to load the usecode
		unsigned int code_offset=0;

		unsigned int code_size = ucf._funcsize - ucf._datasize - ((3+ucf._num_externs) * SIZEOF_USHORT);
		
		DEBUG_READ_PAIR("Code Size: ", code_size);

		while(code_offset<(code_size-2)) //TODO: Why the -2?!? it doesn't work otherwise
		{
			UCc ucop;

			ucop._offset = code_offset;

			ucop._id = read_ubyte(f);
			code_offset++;

			const UCOpcodeData &otd = opcode_table_data[ucop._id];

			//assert(((otd.asm_nmo.size()!=0) && (otd.ucs_nmo.size()!=0)));
			for(unsigned int i=0; i<otd.num_bytes; i++)
				ucop._params.push_back(read_ubyte(f));

			// parse the parameters
			ucc_parse_parambytes(ucop, otd);

			code_offset+=otd.num_bytes;

			/* if we're an opcode that sets a return value, we need to mark the
				function as one that returns a value */
			if(otd.flag_return==true)
				ucf.return_var=true;
			
			/* if we're a function debugging opcode, set the debuging flag, and
				assign the variable name string offset
				TODO: Add this to opcodes.txt */
			if((uc.game_bg() || uc.game_si()) && ucop._id==0x4D)
			{
				ucf.debugging_info=true;
				assert(ucop._params_parsed.size()>=2);
				ucf.debugging_offset = ucop._params_parsed[1];
				ucf.funcname = ucf._data.find(0x0000)->second;
			}
			
			ucf._opcodes.push_back(ucop);

			#ifdef DEBUG_READ
			cout << std::setw(4) << code_size << "\t" << std::setw(4) << code_offset << "\t" << std::setw(4) << (unsigned int)ucop._offset << "\t" << std::setw(2) << (unsigned int)ucop._id << "\t";
			for(unsigned int i=0; i<ucop._params.size(); i++)
				cout << std::setw(2) << (unsigned int)ucop._params[i] << ',';
			cout << endl;
			#endif
		}
	}
}

/* calculates the relative offset jump location, used in opcodes jmp && jne */
inline int calcreloffset(const UCc &op, unsigned int param)
{
	/* forumla:
	   real offset = offset of start of current opcode
	               + int of parameter (since you can jump backwards)
	               + 1 (size of "opcode")
	               + size of "opcode" parameter data
	   NOTE: since param is unsigned, a twos-complimant is required:
	         formula: 0xFFFF - (unsigned short)param + 1
	                  ^^^^^^ max of unsighed short
	*/
	return op._offset + ((param>>15) ? (-1 * (0xFFFF - static_cast<unsigned short>(param) + 1)) : static_cast<int>(param)) + 1 + op._params.size();
}

void ucc_parse_parambytes(UCc &ucop, const UCOpcodeData &otd)
{
	unsigned int first=0;
	
	for(vector<pair<unsigned int, bool> >::const_iterator s=otd.param_sizes.begin(); s!=otd.param_sizes.end(); ++s)
	{
		//cout << ucop._id << '\t' << ucop._params.size() << endl;
		
		assert(first<ucop._params.size());
		
		unsigned int ssize=s->first;
		bool offset_munge=s->second;
		
		assert(ssize!=0);

		if(ssize==1)
			ucop._params_parsed.push_back(static_cast<unsigned short>(static_cast<unsigned int>(ucop._params[first++])));
		else if(ssize==2)
			if(offset_munge)
			{
				unsigned int reloffset = calcreloffset(ucop, static_cast<unsigned short> (static_cast<unsigned int>(ucop._params[first++]) + ((static_cast<unsigned int>(ucop._params[first++])) << 8)));
				ucop._params_parsed.push_back(reloffset);
				ucop._jump_offsets.push_back(reloffset);
			}
			else
				ucop._params_parsed.push_back(static_cast<unsigned short> (static_cast<unsigned int>(ucop._params[first++]) + ((static_cast<unsigned int>(ucop._params[first++])) << 8)));
	}
}


void print_asm_data(UCFunc &ucf, ostream &o);
void print_asm_opcodes(ostream &o, UCFunc &ucf, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, const vector<UCOpcodeData> &optab);

/* prints the "assembler" output of the usecode, currently trying to duplicate
   the output of the original ucdump... */
void print_asm(UCFunc &ucf, ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, const UCData &uc)
{
	if(uc.verbose()) cout << "Printing function..." << endl;

	o << "Function at file offset " << std::setw(8) << ucf._offset << "H" << endl;
	o << "\t.funcnumber  " << std::setw(4) << ucf._funcid << "H" << endl;
	o << "\t.msize       " << std::setw(4) << ucf._funcsize << "H" << endl;
	o << "\t.dsize       " << std::setw(4) << ucf._datasize << "H" << endl;

	if(ucf.debugging_info)
		o << "\t  .dbgoffset " << std::setw(4) << ucf.debugging_offset << "H" << endl;
	
	if(ucf._data.size())
    print_asm_data(ucf, o);

	o << "Code segment at file offset " << std::setw(8) << ucf._codeoffset << "H" << endl;
	o << "\t.argc        " << std::setw(4) << ucf._num_args << "H" << endl;
	o << "\t.localc      " << std::setw(4) << ucf._num_locals << "H" << endl;
	o << "\t.externsize  " << std::setw(4) << ucf._externs.size() << "H" << endl;

	for(unsigned int i=0; i<ucf._externs.size(); i++)
		o << '\t' << "  .extern    " << std::setw(4) << ucf._externs[i] << "H" << endl;

	//o << "-----" << endl;
    //_opcodes[i]->print_asm(cout);
	for(vector<UCc>::iterator op=ucf._opcodes.begin(); op!=ucf._opcodes.end(); op++)
		print_asm_opcode(o, ucf, funcmap, opcode_table_data, intrinsics, *op);
}

void print_asm_data(UCFunc &ucf, ostream &o)
{
	static const unsigned int nochars=60;
	// limit of about 60 chars to a line, wrap to the next line if longer then this...
	for(std::map<unsigned int, std::string, std::less<unsigned int> >::iterator i=ucf._data.begin(); i!=ucf._data.end(); i++)
	{
		for(unsigned int j=0; j<i->second.size(); j++)
		{
			if(j==0)
				o << std::setw(4) << i->first;
			if((j!=0) && !(j%nochars))
				o << "'" << endl;
			if(!(j%nochars))
				o << "\tdb\t'";

			o << i->second[j];
		}
		o << "'" << endl;
		o << "\tdb\t00" << endl;
	}
}

void output_raw_opcodes(ostream &o, const UCc &op);

extern UCData uc;

void print_asm_opcode(ostream &o, UCFunc &ucf, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const map<unsigned int, string> &intrinsics, const UCc &op)
{
	// offset
	o << std::setw(4) << op._offset << ':';

	if(uc.rawops()) output_raw_opcodes(o, op);
	else            o << '\t';

	o << demunge_ocstring(ucf, funcmap, optab[op._id].asm_nmo, op._params_parsed, intrinsics, op, false);

	if(uc.autocomment())
		o << demunge_ocstring(ucf, funcmap, optab[op._id].asm_comment, op._params_parsed, intrinsics, op, false);

	o << endl;
}

void output_raw_opcodes(ostream &o, const UCc &op)
{
	// chars in opcode
	o << ' ' << std::setw(2) << static_cast<unsigned int>(op._id);
	if(op._params.size()) cout << ' ';

	for(unsigned int i=0; i<op._params.size(); i++)
	{
		o << std::setw(2) << static_cast<unsigned int>(op._params[i]);
		if(i!=op._params.size())
			o << ' ';
	}

	// seperator
	unsigned int numsep = op._params.size();
	//cout << endl << numsep << endl;
	if(numsep>6)
		o << endl << "\t\t\t";
	else if (numsep>5)
		o << " ";
	else if (numsep>2)
		o << "\t";
	else
		o << "\t\t";
}

inline unsigned int charnum2uint(const char c)
{
	switch(c)
	{
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		default:  return 0;
	}
	return 0; // can't happen
}

string demunge_ocstring(UCFunc &ucf, const FuncMap &funcmap, const string &asmstr, const vector<unsigned int> &params, const map<unsigned int, string> &intrinsics, const UCc &op, bool ucs_output)
{
	std::strstream str;
	str << std::setfill('0') << std::setbase(16);
	str.setf(ios::uppercase);
	size_t	len=asmstr.length();

	if(len==0) return string(); // for the degenerate case

	bool finished=false; // terminating details are at end-of-while
	unsigned int i=0; // istr index
	unsigned int width=0; // width value for setw()

	if(ucs_output && opcode_table_data[op._id].flag_paren) str << '(';
	
	while(!finished&&i<len)
	{
		bool special_call(false); // FIXME: <sigh> temporary exception handling for call (0x24)
		
		char c = asmstr[i];
		
		width = 4; // with defaults to 4
		
		switch(c)
		{
			case '\\':
				i++;
				c = asmstr[i];
				switch(c)
				{
					case '\\': str << '\\'; break;
					case 'n':  str << '\n'; break;
					case 't':  str << '\t'; break;
					case '\'': str << '\''; break;
					case '"':  str << '\"'; break;
					case 'b':  // bell is invalid
					default:   // we'll silently drop errors... it's the only "clean" way
						str << '\\' << c;
				}
				break;
			case '%':
				{
					i++;
					c = asmstr[i];

					// if it's a "byte" set width to 2, and get the next char
					if(c=='b')      { i++; c = asmstr[i]; width=2; }
					// if it's a "short" set width to 4, and get the next char
					else if(c=='s') { i++; c = asmstr[i]; width=4; }
					// if we want to output the 'decimal' value rather then the default hex
					else if(c=='d')
					{
						i++; c = asmstr[i];
						unsigned int t = charnum2uint(c);
						
						if(t!=0)
						{
							assert(params.size()>=t);
							str << std::setbase(10) << params[t-1] << std::setbase(16);
						}
						else if(c=='%')
							str << '%';
						break;
					}
					// if it's the character representation of a text data string we want
					else if(c=='t')
					{
						bool commentformat=false;
						i++; c = asmstr[i];
						
						// if we only want to output the 'short' format of the text (comment format)
						if(c=='c')
						{
							commentformat=true;
							i++; c = asmstr[i];
						}
						
						unsigned int t = charnum2uint(c);
						
						assert(params.size()>=t);
						assert(t!=0);
						string s = ucf._data.find(params[t-1])->second;
						
						if(commentformat)
							if(s.size()>17) s = s.substr(0, 17) + string("...");
						
						// escape the appropriate characters...
						// we'll only do it in the 'full' text output for the moment.
						if(!commentformat)
							for(string::size_type i=0; i<s.size(); i++)
								if((s[i]=='\"') || (s[i]=='\\'))
								{
									// Formerly:
									// z= s.insert(z, 1, '\\');
									// But as we all know, insert is void.
									// What behaviour were we relying on?
									s.insert(i, "\\");
									++i;
								}
						
						str << s;
						break;
					}
					// if it's the intrinsic name we want
					else if(c=='i')
					{
						i++; c = asmstr[i];
						unsigned int t = charnum2uint(c);
						
						assert(params.size()>=t);
						assert(t!=0);
						string s = intrinsics.find(params[t-1])->second;
						str << s;
						break;
					}
					// if it's external function name we want
					else if(c=='f')
					{
						i++; c = asmstr[i];
						
						if(c=='*')
						{
							if(ucf.funcname.size())
							{
								if(ucf.funcname[0]=='&')
									str << ucf.funcname.substr(1, ucf.funcname.size()-1);
								else
									str << ucf.funcname;
							}
							else
								str << "Func" << std::setw(4) << ucf._funcid;
						}
						else
						{
							unsigned int t = charnum2uint(c);
							
							assert(ucf._externs.size()>=t);
							assert(t!=0);
							assert(op._params_parsed.size()>=1);
							
							FuncMap::const_iterator fmp = funcmap.find(ucf._externs[op._params_parsed[t-1]]);
							if(fmp->second.funcname.size())
							{
								if(fmp->second.funcname[0]=='&')
									str << fmp->second.funcname.substr(1, fmp->second.funcname.size()-1);
								else
									str << fmp->second.funcname;
							}
							else
								str << "Func" << std::setw(4) << ucf._externs[op._params_parsed[t-1]];
						}
						break;
					}
					// if it's the character representation of a text data string we want
					else if(c=='p')
					{
						i++; c = asmstr[i];
						unsigned int t = charnum2uint(c);
						
						// FIXME: this is the special 'call' case, it may be a good idea to make more general
						if((t==0) && (c==','))
						{
							special_call=true;
						
							for(vector<UCc *>::const_iterator i=op._popped.begin(); i!=op._popped.end();)
							{
								str << demunge_ocstring(ucf, funcmap, opcode_table_data[(*i)->_id].ucs_nmo, (*i)->_params_parsed, intrinsics, **i, ucs_output);
								if(++i!=op._popped.end())
									str << ", ";
							}
						}
						
						if(t!=0)
						{
							if(t>op._popped.size())
								str << "SOMETHING_GOES_HERE()";
							else
							{
								UCc &ucc(*op._popped[t-1]);
								str << demunge_ocstring(ucf, funcmap, opcode_table_data[ucc._id].ucs_nmo, ucc._params_parsed, intrinsics, ucc, ucs_output);
							}
						}
						break;
					}
					
					if(special_call!=true)
					{
						unsigned int t = charnum2uint(c);
						if(t!=0)
						{
							assert(params.size()>=t);
							str << std::setw(width) << params[t-1];
						}
						else if(c=='%')
							str << '%';
					}
				}
				break;
			default: // it's just a character, leave it be
				str << c;
		}

		i++;
		if(i==asmstr.size()) finished=true;
	}
	
	if(ucs_output && opcode_table_data[op._id].flag_paren) str << ')';
	
	str << std::ends;
	return str.str();
}

