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

#ifndef UCFUNC_H
#define UCFUNC_H

#include <map>
#include <vector>
#include <cstdio>
#include <string>
#include <fstream>
#include "ucc.h"

class UCFuncSet
{
	public:
		UCFuncSet(unsigned short new_funcid, unsigned short new_num_args, bool new_return_var, const std::string &new_funcname)
		         : funcid(new_funcid), num_args(new_num_args), return_var(new_return_var), funcname(new_funcname) {};
		~UCFuncSet() {};
		
		unsigned short funcid;      // the id of the function
		unsigned short num_args;    // the number of arguments
		bool           return_var;  // if true, the function returns a variable
		std::string    funcname;    // the name of the function, if it has one
};

typedef std::map<unsigned short, UCFuncSet> FuncMap;
typedef std::pair<unsigned short, UCFuncSet> FuncMapPair;	

//#define DEBUG_GOTOSET
const unsigned int SIZEOF_USHORT = 2;

class FlagData
{
  public:
    enum { SETFLAG=false, GETFLAG=true };
    /* SETFLAG=UCC_POPF, GETFLAG=UCC_PUSHF */
    enum { POP=false, PUSH=true};

    FlagData(const long func, const unsigned short offset,
             const unsigned short flag, const bool access)
            : _func(func), _offset(offset), _flag(flag), _access(access) {};

    long func() const { return _func; };
    unsigned short offset() const { return _offset; };
    unsigned short flag() const { return _flag; };
    bool access() const { return _access; };

  private:
    long           _func;
    unsigned short _offset;
    unsigned short _flag;
    bool           _access;
};

class SortFlagDataLessFlag
{
  public:
    bool operator()(const FlagData &fd1, const FlagData &fd2) const
    {
       return (fd1.flag()<fd2.flag()) ? true :
                  (fd1.flag()>fd2.flag()) ? false :
                      (fd1.func()<fd2.func()) ? true :
                          (fd1.func()>fd2.func()) ? false :
                              (fd1.offset()<fd2.offset());
    };
    bool operator()(const FlagData *fd1, const FlagData *fd2) const
    {
       return (fd1->flag()<fd2->flag()) ? true :
                  (fd1->flag()>fd2->flag()) ? false :
                      (fd1->func()<fd2->func()) ? true :
                          (fd1->func()>fd2->func()) ? false :
                              (fd1->offset()<fd2->offset());
    };
};

class SortFlagDataLessFunc
{
  public:
    bool operator()(const FlagData &fd1, const FlagData &fd2) const
    {
       return (fd1.func()<fd2.func()) ? true :
                  (fd1.func()>fd2.func()) ? false :
                      (fd1.flag()<fd2.flag()) ? true :
                          (fd1.flag()>fd2.flag()) ? false :
                              (fd1.offset()<fd2.offset());
    };
    bool operator()(const FlagData *fd1, const FlagData *fd2) const
    {
       return (fd1->func()<fd2->func()) ? true :
                  (fd1->func()>fd2->func()) ? false :
                      (fd1->flag()<fd2->flag()) ? true :
                          (fd1->flag()>fd2->flag()) ? false :
                              (fd1->offset()<fd2->offset());
    };
};

class UCNode;

class UCNode
{
	public:
		UCNode(UCc *newucc=0) : ucc(newucc) { };
		UCNode(std::vector<UCNode *>::iterator beg, std::vector<UCNode *>::iterator end)
		      : ucc(new UCc()), nodelist(beg, end) { };
		~UCNode() { };
	
		UCc *ucc;
		std::vector<UCNode *> nodelist;
};

#include "opcodes.h"

class GotoSet
{
	public:
		GotoSet() : _offset(0) {};
		GotoSet(const unsigned int offset, UCc *ucc)
		       : _offset(offset)
		{
			add(ucc);
		};
		GotoSet(UCc *ucc) : _offset(ucc->_offset) { add(ucc); };

		std::vector<std::pair<UCc *, bool> > &operator()() { return _uccs; };
		
		UCc &operator[](const unsigned int i) { return *(_uccs[i].first); };
		unsigned int size() const { return _uccs.size(); };

		void add(UCc *ucc, bool gc=false)
		{
			_uccs.push_back(std::pair<UCc *, bool>(ucc, gc));
		};

		/* Just a quick function to remove all the Uccs in _uccs marked for
		   garbage collection. */
		void gc()
		{
			for(GotoSet::iterator j=_uccs.begin(); j!=_uccs.end();)
			{
				#ifdef DEBUG_GOTOSET
				std::cout << "OP: " << opcode_table_data[j->first->_id].ucs_nmo << '\t' << j->second;
				#endif
				if(j->second==true)
				{
					/* ok, we need to take into consideration that the
					   iterator we're removing might ==_uccs.begin() */
					bool begin=false;
					if(j==_uccs.begin()) begin=true;
					
					#ifdef DEBUG_GOTOSET
					std::cout << "\tremoved";
					#endif
					
					GotoSet::iterator rem(j);
					
					if(begin==false) j--;
					
					_uccs.erase(rem);
					
					if(begin==true) j=_uccs.begin();
					else            j++;
					
					if(j==_uccs.end()) std::cout << "POTENTIAL PROBLEM" << std::endl;
				}
				else
					j++;
				
				#ifdef DEBUG_GOTOSET
				std::cout << std::endl;
				#endif
			}
		};
		
		unsigned int offset() const { return _offset; };

		typedef std::vector<std::pair<UCc *, bool> >::iterator iterator;
		
	private:
		unsigned int _offset;
		std::vector<std::pair<UCc *, bool> > _uccs;
};

class UCOpcodeData;

class UCFunc
{
	public:
		UCFunc() : _offset(0), _funcid(0), _funcsize(0), _bodyoffset(0), _datasize(0),
		           _codeoffset(0), _num_args(0), _num_locals(0), _num_externs(0),
		           return_var(false), debugging_info(false), debugging_offset(0) {};

		void output_list(std::ostream &o, unsigned int funcno, const UCOptions &options);
		
		void output_ucs(std::ostream &o, const FuncMap &funcmap, const std::map<unsigned int, std::string> &intrinsics, const UCOptions &options);
		std::ostream &output_ucs_funcname(std::ostream &o, const FuncMap &funcmap,
                                    unsigned int funcid,
                                    unsigned int numargs, bool return_var);
		void output_ucs_node(std::ostream &o, const FuncMap &funcmap, UCNode* ucn, const std::map<unsigned int, std::string> &intrinsics, unsigned int indent, const UCOptions &options);
		void output_ucs_data(std::ostream &o, const FuncMap &funcmap, const std::map<unsigned int, std::string> &intrinsics, const UCOptions &options, unsigned int indent);
		void output_ucs_opcode(std::ostream &o, const FuncMap &funcmap, const std::vector<UCOpcodeData> &optab, const UCc &op, const std::map<unsigned int, std::string> &intrinsics, unsigned int indent);
		
		void parse_ucs(const FuncMap &funcmap, const std::map<unsigned int, std::string> &intrinsics, const UCOptions &options);
		void parse_ucs_pass1(std::vector<UCNode *> &nodes);
		void parse_ucs_pass2(std::vector<GotoSet> &gotoset, const FuncMap &funcmap, const std::map<unsigned int, std::string> &intrinsics);
		std::vector<UCc *> parse_ucs_pass2a(std::vector<std::pair<UCc *, bool> >::reverse_iterator current,
		                               std::vector<std::pair<UCc *, bool> > &vec, unsigned int opsneeded,
		                               const FuncMap &funcmap, const std::map<unsigned int, std::string> &intrinsics);
		void parse_ucs_pass3(std::vector<GotoSet> &gotoset, const std::map<unsigned int, std::string> &intrinsics);

		void output_tt(std::ostream &o);
//	private:
	
		std::vector<GotoSet> gotoset;
		
		std::streampos _offset;      // offset to start of function
		unsigned short _funcid;      // the id of the function
		unsigned short _funcsize;    // the size of the function (bytes)
		std::streampos _bodyoffset;  // the file position after the header is read
		
		unsigned short _datasize;    // the size of the data block
		
		std::map<unsigned int, std::string, std::less<unsigned int> > _data;
			// contains the entire data segment in offset from start of segment, and string data pairs
		
		std::streampos _codeoffset; // the offset to the start of the code segment
		
		unsigned int _num_args;    // the number of arguments
		unsigned int _num_locals;  // the number of local variables
		unsigned int _num_externs; // the number of external function id's
		std::vector<unsigned short> _externs; // the external function id's
		
		std::vector<UCc> _opcodes;
		
		bool           return_var; // does the function return a variable?
		bool           debugging_info;
		unsigned int   debugging_offset;
		std::string    funcname;
		
		unsigned short codesize() const { return _funcsize - _datasize; };
		
		// the following vars are for data compatibility with the original UCFunc
		std::vector<FlagData *>   _flagcount;
		UCNode node;
};

void readbin_U7UCFunc(std::ifstream &f, UCFunc &ucf, const UCOptions &options);
void readbin_U8UCFunc(std::ifstream &f, UCFunc &ucf);
void print_asm(UCFunc &ucf, std::ostream &o, const FuncMap &funcmap, const std::map<unsigned int, std::string> &intrinsics, const UCOptions &options);

#endif

