
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
		UCFuncSet(unsigned short new_funcid, unsigned short new_num_args)
		         : funcid(new_funcid), num_args(new_num_args) {};
		~UCFuncSet() {};
		
		unsigned short funcid;      // the id of the function
		unsigned short num_args;    // the number of arguments
		
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
		           _codeoffset(0), _num_args(0), _num_locals(0), _num_externs(0) {};

		// temp passing UCData, probably shouldn't need it.
		void output_ucs(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, bool gnubraces=false);
		void output_ucs_node(ostream &o, const FuncMap &funcmap, UCNode* ucn, const map<unsigned int, string> &intrinsics, unsigned int indent);
		void output_ucs_data(ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, unsigned int indent);
		void output_ucs_opcode(ostream &o, const FuncMap &funcmap, const vector<UCOpcodeData> &optab, const UCc &op, const map<unsigned int, string> &intrinsics, unsigned int);
		
		void parse_ucs(const FuncMap &funcmap, const map<unsigned int, string> &intrinsics);
		void parse_ucs_pass1(vector<UCNode *> &nodes);
		void parse_ucs_pass1a(vector<UCNode *> &nodes);
		void parse_ucs_pass2a(vector<GotoSet> &gotoset, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics);
		vector<UCc *> parse_ucs_pass2b(vector<pair<UCc *, bool> >::reverse_iterator current,
		                               vector<pair<UCc *, bool> > &vec, unsigned int opsneeded,
		                               const FuncMap &funcmap, const map<unsigned int, string> &intrinsics);

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
		//vector<pair<unsigned short, pair<unsigned char, vector<unsigned char> > > > _usecode;
		vector<UCc> _opcodes;

		// the following vars are for data compatibility with the original UCFunc
		unsigned short codesize() const { return _funcsize - _datasize; };
		vector<FlagData *>   _flagcount;
		UCNode node;
};

void readbin_UCFunc(ifstream &f, UCFunc &ucf);
void print_asm(UCFunc &ucf, ostream &o, const FuncMap &funcmap, const map<unsigned int, string> &intrinsics, const UCData &uc);

/*class UCFunc
{
  public:
    UCFunc() : _opcode_count(256, 0), _unknown_opcode_count(256, 0), _unknown_intrinsic_count(256, 0) {};
    ~UCFunc();
    void process_old(ifstream *f, long func, int* found,
                          vector<unsigned char> &intrinsic_buf,
                          bool scan_mode,
                          unsigned long opcode,
                          unsigned long intrinsic,
                          unsigned int &uc_funcid,
                          const char** func_table);
    void process_data_seg();
    void process_code_seg(vector<unsigned char> &intrinsic_buf, int mute,
                          int count_all_opcodes,
                          int count_all_intrinsic,
                          const char** func_table);
    unsigned short print_opcode(unsigned char* ptrc, unsigned short coffset,
                            unsigned char* pdataseg,
                            unsigned short* pextern,
                            unsigned short externsize,
                            vector<unsigned char> &intrinsic_buf,
                            int mute,
                            int count_all_opcodes,
                            int count_all_intrinsic,
                            const char** func_table);
    unsigned short print_opcode_old(unsigned char* ptrc, unsigned short coffset,
                            unsigned char* pdataseg,
                            unsigned short* pextern,
                            unsigned short externsize,
                            vector<unsigned char> &intrinsic_buf,
                            int mute,
                            int count_all_opcodes,
                            int count_all_intrinsic,
                            const char** func_table);

    //void genflags(const vector<UCc> &uc_codes);
    const vector<FlagData *> &flags() const { return _flagcount; };

    // LEGACY
    const vector<UCc> &codes() const { return _codes; };

    // temporary until it's removed from ucdump.cc
    const vector<unsigned int> &externs() const { return _externs; };
    const map<unsigned int, string, less<unsigned int> > &data() const
             { return _data; };
    unsigned int localc() const { return _localc; };
    unsigned int argc() const { return _argc; };
    unsigned short funcid() const { return _funcid; };
    long offset() const { return _offset; };
    unsigned short funcsize() const { return _funcsize; };
    unsigned short datasize() const { return _datasize; };
    unsigned short codesize() const { return _funcsize - _datasize; };

//  private:
    unsigned short read_ushort();
    unsigned short read_ushort(const unsigned char *buff);
    void read_vchars(char *buffer, const unsigned long nobytes);
    void read_vbytes(unsigned char *buffer, const unsigned long nobytes);

    // decompiling functions
    void do_decompile();
    void do_print();
    void genflags();

    void re_order();

    void print_asm();
    void print_asm_data();

    void print_c_externs();
    void print_c_head();
    void print_c_local();
    void print_c_body();
    void print_c_tail();

    string extern_tostr(const unsigned int uc_extern);
    // /decompiling functions

    // temp file manipulation functions
		void fseekbeg(const streampos &pos) { _file->seekg(pos, ios::beg); };
		void fseekcur(const streampos &pos) { _file->seekg(pos, ios::cur); };
		streampos ftell() const { return _file->tellg(); };
		
    //int fseek(const long offset, const int mode) { return ::fseek(_file, offset, mode); };
    bool eof() const { return _file->eof(); };
    int get() { return _file->get(); };

    // /temp file manipulation functions

    unsigned short _funcid;
    unsigned short _funcsize;
    unsigned short _datasize;

    unsigned int _argc; // number of function parameters
    unsigned int _localc; // number of local variables

    streampos      _offset;
    long           _code_offset; // offset to start of code segment in file

    ifstream *_file;

    vector<FlagData *>   _flagcount;
    vector<Opcode *>     _raw_opcodes;
    vector<Label *>      _opcodes;
    vector<unsigned int> _externs;
    map<unsigned int, string, less<unsigned int> > _data;
    vector<unsigned int> _opcode_count;

    // currently not used
    vector<unsigned int> _unknown_opcode_count;
    vector<unsigned int> _unknown_intrinsic_count;

    // LEGACY: remove when old c output format is unused
    vector<UCc> _codes;
};*/

#endif

