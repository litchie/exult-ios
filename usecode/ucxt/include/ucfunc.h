
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
		UCFuncSet(unsigned short new_funcid, unsigned short new_num_args, bool new_return_var)
		         : funcid(new_funcid), num_args(new_num_args), return_var(new_return_var) {};
		~UCFuncSet() {};
		
		unsigned short funcid;      // the id of the function
		unsigned short num_args;    // the number of arguments
		bool           return_var;  // if true, the function returns a variable
};

typedef map<unsigned short, UCFuncSet> FuncMap;
typedef pair<unsigned short, UCFuncSet> FuncMapPair;	

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
		UCNode(vector<UCNode *>::iterator beg, vector<UCNode *>::iterator end)
		      : ucc(new UCc()), nodelist(beg, end) { };
		~UCNode() { };
	
		UCc *ucc;
		vector<UCNode *> nodelist;
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

		vector<pair<UCc *, bool> > &operator()() { return _uccs; };
		
		UCc &operator[](const unsigned int i) { return *(_uccs[i].first); };
		unsigned int size() const { return _uccs.size(); };

		void add(UCc *ucc, bool gc=false)
		{
			_uccs.push_back(pair<UCc *, bool>(ucc, gc));
		};

		/* Just a quick function to remove all the Uccs in _uccs marked for
		   garbage collection. */
		void gc()
		{
			for(GotoSet::iterator j=_uccs.begin(); j!=_uccs.end();)
			{
				#ifdef DEBUG_GOTOSET
				cout << "OP: " << opcode_table_data[j->first->_id].ucs_nmo << '\t' << j->second;
				#endif
				if(j->second==true)
				{
					/* ok, we need to take into consideration that the
					   iterator we're removing might ==_uccs.begin() */
					bool begin=false;
					if(j==_uccs.begin()) begin=true;
					
					#ifdef DEBUG_GOTOSET
					cout << "\tremoved";
					#endif
					
					GotoSet::iterator rem(j);
					
					if(begin==false) j--;
					
					_uccs.erase(rem);
					
					if(begin==true) j=_uccs.begin();
					else            j++;
					
					if(j==_uccs.end()) cout << "POTENTIAL PROBLEM" << endl;
				}
				else
					j++;
				
				#ifdef DEBUG_GOTOSET
				cout << endl;
				#endif
			}
		};
		
		unsigned int offset() const { return _offset; };

		typedef vector<pair<UCc *, bool> >::iterator iterator;
		
	private:
		unsigned int _offset;
		vector<pair<UCc *, bool> > _uccs;
};

class UCOpcodeData;

class UCFunc
{
	public:
		UCFunc() : _offset(0), _funcid(0), _funcsize(0), _bodyoffset(0), _datasize(0),
		           _codeoffset(0), _num_args(0), _num_locals(0), _num_externs(0),
		           _return_var(false) {};

		// temp passing UCData, probably shouldn't need it.
		void output_ucs(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool uselesscomment, bool gnubraces=false);
		ostream &output_ucs_funcname(ostream &o, unsigned int funcid, unsigned int num_args, bool return_var);
		void output_ucs_node(ostream &o, const FuncMap &funcmap, UCNode* ucn, const map<unsigned int, string> &intrinsics, unsigned int indent);
		void output_ucs_data(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool uselesscomment, unsigned int indent);
		void output_ucs_opcode(ostream &o, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const UCc &op, const map<unsigned int, string> &intrinsics, unsigned int);
		
		void parse_ucs(const FuncMap &funcmap, const map<unsigned int, string> &intrinsics);
		void parse_ucs_pass1(vector<UCNode *> &nodes);
		void parse_ucs_pass2(vector<GotoSet> &gotoset, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics);
		vector<UCc *> parse_ucs_pass2a(vector<pair<UCc *, bool> >::reverse_iterator current,
		                               vector<pair<UCc *, bool> > &vec, unsigned int opsneeded,
		                               const FuncMap &funcmap, const map<unsigned int, string> &intrinsics);
		void parse_ucs_pass3(vector<GotoSet> &gotoset, const map<unsigned int, string> &intrinsics);

//	private:
	
		vector<GotoSet> gotoset;
		
		streampos      _offset;      // offset to start of function
		unsigned short _funcid;      // the id of the function
		unsigned short _funcsize;    // the size of the function (bytes)
		streampos      _bodyoffset;  // the file position after the header is read
		
		unsigned short _datasize;    // the size of the data block
		
		map<unsigned int, string, less<unsigned int> > _data;
			// contains the entire data segment in offset from start of segment, and string data pairs
		
		streampos      _codeoffset; // the offset to the start of the code segment
		
		unsigned short _num_args;    // the number of arguments
		unsigned short _num_locals;  // the number of local variables
		unsigned short _num_externs; // the number of external function id's
		vector<unsigned short> _externs; // the external function id's
		
		vector<UCc> _opcodes;
		
		bool           _return_var; // does the function return a variable?
		
		// the following vars are for data compatibility with the original UCFunc
		unsigned short codesize() const { return _funcsize - _datasize; };
		vector<FlagData *>   _flagcount;
		UCNode node;
};

void readbin_UCFunc(ifstream &f, UCFunc &ucf);
void print_asm(UCFunc &ucf, ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, const UCData &uc);

#endif

