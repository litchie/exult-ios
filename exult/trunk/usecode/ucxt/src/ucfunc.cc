#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ucfunc.h"
#include "newopcode.h"
#include <set>
#include <strstream>

const string VARNAME = "uvar";
const string VARPREFIX = "var";
const unsigned int ASM_DISP_STR_LEN=20;

extern UCData uc;

//#define TEST_V3 false
#define TEST_V3 true

/*UCFunc::~UCFunc()
{
}*/

/* Prints module's data segment */
/*void UCFunc::process_data_seg()
{
	streampos pos = _file->tellg();
	
	unsigned short off = 0;

	// Load all strings & their offsets
	while( off < _datasize )
	{
		assert(!eof());

		char c;
		string data;

		while((c=get())!=0x00)
			data+=c;

		_data.insert(pair<unsigned int, string>(off, data));

		off+=data.size()+1;

	}
	fseekbeg(pos);
}*/

#include "opcodes.h"

/*
 Prints single opcode
 Return number of bytes to advance the code pointer
 Prints first characters of strings referenced
*/
/*unsigned short UCFunc::print_opcode(unsigned char* ptrc, unsigned short coffset,
                            unsigned char* pdataseg,
                            unsigned short* pextern,
                            unsigned short externsize,
                            vector<unsigned char> &intrinsic_buf,
                            int mute,
                            int count_all_opcodes,
                            int count_all_intrinsic,
                            const char** func_table)
{
//  if( count_all_opcodes )
//  _unknown_opcode_count[*ptrc]++;

  // Find the description
  const opcode_desc *pdesc = ( *ptrc >= ( sizeof(opcode_table) / sizeof( opcode_desc ) ) ) ?
                        NULL : opcode_table + ( *ptrc );
  // Unknown opcode
  if( pdesc && ( pdesc->mnemonic == NULL ) )
    pdesc = NULL;

    // Unknown opcode
//  if( ( pdesc == NULL ) && !count_all_opcodes )
//    _unknown_opcode_count[*ptrc]++;

  // Number of bytes to print
  unsigned int nbytes = pdesc ? ( pdesc->nbytes + 1 ) : 1;

  UCc ucc;
  ucc._offset = coffset;

  // Print bytes
  for(unsigned int i = 0; i < nbytes; i++ )
  {
    if(i==0) ucc._id = ptrc[i];
    else     ucc._params.push_back(ptrc[i]);
  }

  _codes.push_back(ucc);//PATRICK
  _raw_opcodes.push_back(NewOpcode(ucc._offset, ucc._id, ucc._params));
  //new MiscOpcode(ucc._offset, ucc._id, ucc._params)
  // Print operands if any
  return nbytes;
}

void UCFunc::process_code_seg(vector<unsigned char> &intrinsic_buf,
                          int mute,
                          int count_all_opcodes,
                          int count_all_intrinsic,
//                          vector<UCc> &uc_codes,
                          const char** func_table)
{
  long pos;
//  unsigned short size;
  unsigned short externsize;
  unsigned short i;
  unsigned short offset;
  unsigned char* p;
  unsigned char* pp;
  unsigned char* pdata;
  unsigned short* pextern;

  pos = ftell();
  unsigned int size = _funcsize - _datasize - SIZEOF_USHORT;
  pp = p = (unsigned char *)malloc(size);
  pdata = (unsigned char *)malloc(_datasize);
  read_vbytes(pdata, _datasize);
  _code_offset = ftell();

  read_vbytes(p, size);
  fseekbeg(pos);
  // Print code segment header
  if( size < 3 * SIZEOF_USHORT )
  {
    printf("Code segment bad!\n");
    free(p);
    free(pdata);
    return;
  }
  // Print argument counter
  _argc = read_ushort(pp);
  pp += SIZEOF_USHORT;
  // Print locals counter
  _localc = read_ushort(pp);
  pp += SIZEOF_USHORT;
  // Print externs section
  externsize = read_ushort(pp);

  pp += SIZEOF_USHORT;
  if( size < ( ( 3 + externsize ) * SIZEOF_USHORT ) )
  {
    printf("Code segment bad!\n");
    free(p);
    free(pdata);
    return;
  }
  size -= ( ( 3 + externsize ) * SIZEOF_USHORT );
  pextern = (unsigned short*)pp;
  for( i = 0; i < externsize; i++ )
  {
    _externs.push_back(read_ushort(pp));

    pp += SIZEOF_USHORT;
  }
  offset = 0;
  // Print opcodes
  while( offset < size )
  {
    unsigned int nbytes = print_opcode(pp, offset, pdata, pextern, externsize,
                              intrinsic_buf, mute,
                              count_all_opcodes,
                              count_all_intrinsic,
                              func_table);
    pp += nbytes;
    offset += nbytes;
  }
  free(p);
  free(pdata);
}

void UCFunc::process_old(ifstream *file, long func, int* found,
                          vector<unsigned char> &intrinsic_buf,
                          bool scan_mode,
                          unsigned long opcode,
                          unsigned long intrinsic,
                          unsigned int &uc_funcid,
                          const char** func_table)
{
  _file = file;

  long bodyoff;
  // Save start offset
  _offset = ftell();
  // Read function header
  _funcid = read_ushort();
  _funcsize = read_ushort();

  // Save body offset
  bodyoff = ftell();
  _datasize = read_ushort();

  if( ( _funcid == func ) || scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ) )
  {
    // Only for matching function or in one of the scan modes
    if( _funcid == func )
    {
      *found = 1;
//      printf("Function at file offset %08lX\n\t.funcnumber\t%04XH\n"
//        "\t.msize\t%04XH\n\t.dsize\t%04XH\n", _offset, _funcid, _funcsize, _datasize);
      { //PATRICK
        uc_funcid = _funcid;
      }
    }
    // Dump function contents
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
      process_data_seg();
//    if( opcode != -1 )
//      memset(opcode_buf, 0, 256);
    if( intrinsic != -1 ) 
    	for(unsigned int i=0; i<intrinsic_buf.size(); i++)
			intrinsic_buf[i]=0;
			
    process_code_seg(intrinsic_buf,
              scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ),
              ( opcode != -1 ), ( intrinsic != -1 ),
              func_table);
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
    {
      do_decompile();
      if(TEST_V3) do_print();
    }

    if( ( ( opcode != -1 ) && _unknown_opcode_count[opcode] > 0 ) ||
      ( ( intrinsic != -1 ) && intrinsic_buf[intrinsic] > 0 ) )
    {
      // Found
      *found = 1;
      if( intrinsic != -1 )
        printf("\tFound function (%04XH) - %d times\n", _funcid,
                                intrinsic_buf[intrinsic]);
      else
        printf("\tFound function (%04XH) - %d times\n", _funcid, _unknown_opcode_count[opcode]);
    }
  }

  genflags();
  // Seek back, then to next function
  fseekbeg(bodyoff);
  fseekcur(_funcsize);
}
*/

/*void process_func(FILE* f, long func, int i, int* found,
                            unsigned char* opcode_buf,
                          unsigned char* intrinsic_buf,
                          int scan_mode,
                          unsigned long opcode,
                          unsigned long intrinsic,
                          unsigned int &uc_funcid,
                          map<unsigned int, string, less<unsigned int> > &uc_data,
                          unsigned int &uc_argc,
                          unsigned int &uc_localc,
                          vector<unsigned int> &uc_externs,
                          vector<UCc> &uc_codes,
                          const char** func_table)
{
  unsigned short s, ds, funcnum;  
  long off, bodyoff;
   * Save start offset *
  off = ftell(f);
   * Read function header *
  fread(&funcnum, sizeof(unsigned short), 1, f);
  fread(&s, sizeof(unsigned short), 1, f);
   * Save body offset *
  bodyoff = ftell(f);
  fread(&ds, sizeof(unsigned short), 1, f);
  if( ( ( func == -1 ) || scan_mode ) && ( opcode == -1 ) && ( intrinsic == -1 ) )
  {
    //cout << "Function     offset    size  data  diff" << endl;
     * Only for general list & scan mode *
    printf("#%03d: %04XH  %08lx  %04x  %04x  %04x\n", i,
                                    funcnum, off, s, ds, s-ds);
  }
  if( ( funcnum == func ) || scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ) )
  {
     * Only for matching function or in one of the scan modes *
    if( funcnum == func )
    {
      *found = 1;
      printf("Function at file offset %08lX\n\t.funcnumber\t%04XH\n"
        "\t.msize\t%04XH\n\t.dsize\t%04XH\n",
        off, funcnum, s, ds);
      { //PATRICK
        uc_funcid = funcnum;
      }
    }
     * Dump function contents *
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
      process_data_seg(f, ds, uc_data);
    if( opcode != -1 ) 
      memset(opcode_buf, 0, 256);
    if( intrinsic != -1 ) 
      memset(intrinsic_buf, 0, 256);
    process_code_seg(f, ds, s, opcode_buf, intrinsic_buf,
              scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ),
              ( opcode != -1 ), ( intrinsic != -1 ),
              uc_argc, uc_localc, uc_externs, uc_codes, func_table);
    if( ( ( opcode != -1 ) && opcode_buf[opcode] > 0 ) ||
      ( ( intrinsic != -1 ) && intrinsic_buf[intrinsic] > 0 ) )
    {
       * Found *
      *found = 1;
      if( intrinsic != -1 )
        printf("\tFound function (%04XH) - %d times\n", funcnum,
                                intrinsic_buf[intrinsic]);
      else
        printf("\tFound function (%04XH) - %d times\n", funcnum, opcode_buf[opcode]);
    }
  }
   * Seek back, then to next function *
  fseek(f, bodyoff, SEEK_SET);
  fseek(f, s, SEEK_CUR);
}*/


/*unsigned short UCFunc::read_ushort()
{
  return ((unsigned short) ((unsigned int)get() + (((unsigned int)get()) << 8)));
}

unsigned short UCFunc::read_ushort(const unsigned char *buff)
{
//  assert(((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)))
//         ==(*(unsigned short *)buff));
  return ((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)));
}

void UCFunc::read_vchars(char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}

void UCFunc::read_vbytes(unsigned char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}

void UCFunc::do_decompile()
{
  // TODO: Generate Unknown opcode count (_unknown_opcode_count)
  // TODO: Generate opcode count (_opcode_count)
  //       eg: _opcode_count[ucc._id]++;

  genflags();

  // setup the extern values
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
    _raw_opcodes[i]->pass1(_externs);

  // setup the string values
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
    _raw_opcodes[i]->pass2(_data);

  re_order();
}

void UCFunc::do_print()
{
//  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
//    _raw_opcodes[i]->print_asm(cout);

  print_asm();
  print_c_externs();
  print_c_head();
  print_c_local();
  print_c_body();
  print_c_tail();

  cout << "----------" << endl << endl << endl;
}

void UCFunc::re_order()
{
  set<unsigned int> _jumps;

  // create a list of jump targets
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
  {
    if(_raw_opcodes[i]->IsaRelativeJump())
      _jumps.insert(dynamic_cast<OpcodeGenericJump *>(_raw_opcodes[i])->target_offset());
  }

  // create the "first" label from the zeroth offset
  Label *currlbl = new Label(_funcid, 0);

  //for each opcode
  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
  {
    // test if the offset is a jump target
    for(set<unsigned int>::iterator j=_jumps.begin(); j!=_jumps.end(); j++)
    {
      // create a new label
      if(_raw_opcodes[i]->offset()==*j)
      {
        _opcodes.push_back(currlbl);
        currlbl=0;
        currlbl = new Label(_funcid, _raw_opcodes[i]->offset());
        break;
      }
    }
    // add the opcode to the current label
    (*currlbl)().push_back(_raw_opcodes[i]);
  }

  // make sure we add the last one
  if((*currlbl)().size()!=0)
    _opcodes.push_back(currlbl);
  currlbl=0;
}

// prints out the external function calls in C output
void UCFunc::print_c_externs()
{
  strstream ss_externs;

  ss_externs << "// externs" << endl;

  for(unsigned int i=0; i<_externs.size(); i++)
    ss_externs << "void " << extern_tostr(_externs[i]) << ";" << endl;
  ss_externs << endl << ends;

  cout << ss_externs.str();
}

string UCFunc::extern_tostr(const unsigned int uc_extern)
{
  strstream ss;
  ss << setfill('0') << setbase(16) << "Func" << setw(4) << uc_extern << "()" << ends;
  return ss.str();
}
*/
/* prints the "head" of the C function, eg:
     void Func0001(uvar var0001)
     {
*/
/*void UCFunc::print_c_head()
{
    strstream ss_func_head;

    ss_func_head << setbase(16) << setfill('0');

    // TODO: replace this "void" with the data type when we find it out
    ss_func_head << "void";

    ss_func_head << " Func" << setw(4) << _funcid << "(";

    for(unsigned int i=0; i<_argc; i++)
    {
      if(i!=0)
        ss_func_head << ", ";
      ss_func_head << VARNAME << " " << VARPREFIX << setw(4) << i;
    }
    ss_func_head << ")" << endl
                 << "{" << endl << ends;

    // TODO: indent the function by two characters
    //uc_indentlvl+=2;

    cout << ss_func_head.str();
}
*/
/* outputs the local variable definitions of the C output */

/*void UCFunc::print_c_local()
{
  strstream strlocal;

  strlocal << setfill('0') << setbase(16);

  vector<unsigned int> ocvi;
  MiscOpcode oc(1, 1, ocvi);
  oc.indent_inc();

  if(_localc)
  {
    for(unsigned int i=_argc; i<(_argc+_localc); i++)
    {
      // TODO: more indenting fuss
      oc.indent(strlocal);
      strlocal << VARNAME << " " << VARPREFIX << setw(4) << i << ";" << endl;
    }
    strlocal << endl;
  }
  strlocal << ends;

  cout << strlocal.str() << endl;
}*/

/* prints out the body of the C function, should be RELATIVLY simple,
   since most of the work should be done */
/*void UCFunc::print_c_body()
{
  cout << endl;
  for(unsigned int i=0; i<_opcodes.size(); i++)
  {
    cout << "-----" << endl;
    _opcodes[i]->print_c(cout);
  }
}*/

/* prints out the tail of the C function */
/*void UCFunc::print_c_tail()
{
  vector<unsigned int> ocvi;
  MiscOpcode oc(1, 1, ocvi);
  oc.indent_dec();

  cout << endl << "}" << endl << endl;
}*/

/*void UCFunc::genflags(const vector<UCc> &uc_codes)
{
  cout << uc_codes.size() << endl;
  for(unsigned int i=0; i<uc_codes.size(); i++)
  {
    if(uc_codes[i]._id==UCC_PUSHF)
      _flagcount.push_back(new FlagData(_funcid, uc_codes[i]._offset, (((unsigned int)uc_codes[i]._params[1])<<8) + (unsigned int)uc_codes[i]._params[0], FlagData::GETFLAG));
    else if(uc_codes[i]._id==UCC_POPF)
      _flagcount.push_back(new FlagData(_funcid, uc_codes[i]._offset, (((unsigned int)uc_codes[i]._params[1])<<8) + (unsigned int)uc_codes[i]._params[0], FlagData::SETFLAG));
  }

}*/

/*void UCFunc::genflags()
{
  //cout << "flags genned" << endl;
  //cout << "gen flags func: " << _funcid << "(" << _raw_opcodes.size() << ")" << endl;

  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
  {
    if(_raw_opcodes[i]->IsaFlag())
    {
      assert(_raw_opcodes[i]->I_Push() || _raw_opcodes[i]->I_Pop());
      assert(!(_raw_opcodes[i]->I_Push() && _raw_opcodes[i]->I_Pop()));

      bool action=true;
      if(_raw_opcodes[i]->I_Push())     action=FlagData::PUSH;
      else if(_raw_opcodes[i]->I_Pop()) action=FlagData::POP;

      //cout << "    " << setw(4) << _funcid << " " << _raw_opcodes[i]->offset() << " "
      //     << (dynamic_cast<OpcodeGenericFlag *> (_raw_opcodes[i]))->flag() << " "
      //     << (action==FlagData::POP ? "pop" : "push") << endl;
    }
  }
}*/

/* prints the "assembler" output of the usecode, currently trying to duplicate
   the output of the original ucdump... */
/*void UCFunc::print_asm()
{
  cout << "Code segment at file offset " << setw(8) << _code_offset << "H" << endl;
  cout << Opcode::SPACER << ".funcnumber" << Opcode::SPACER << setw(4) << _funcid << "H" << endl;
  cout << Opcode::SPACER << ".msize" << Opcode::SPACER << setw(4) << _funcsize << "H" << endl;
  cout << Opcode::SPACER << ".dsize" << Opcode::SPACER << setw(4) << _datasize << "H" << endl;

  // debugging remove comments!
  if(_data.size())
    print_asm_data();

  cout << "Code segment at file offset " << setw(8) << _code_offset << endl;

  cout << Opcode::SPACER << Opcode::SPACER << ".argc " << setw(4) << _argc << "H" << endl;
  cout << Opcode::SPACER << Opcode::SPACER << ".localc " << setw(4) << _localc << "H" << endl;
  cout << Opcode::SPACER << Opcode::SPACER << ".externsize " << setw(4) << _externs.size() << "H" << endl;

  for(unsigned int i=0; i<_externs.size(); i++)
    cout << Opcode::SPACER << Opcode::SPACER << ".extern " << setw(4) << _externs[i] << "H" << endl;

  for(unsigned int i=0; i<_opcodes.size(); i++)
  {
    cout << "-----" << endl;
    _opcodes[i]->print_asm(cout);
  }
}*/

/*void UCFunc::print_asm_data()
{
  // limit of about 70 chars to a line, wrap to the next line if longer then this...
  for(map<unsigned int, string, less<unsigned int> >::iterator i=_data.begin(); i!=_data.end(); i++)
  {
    for(unsigned int j=0; j<i->second.size(); j++)
    {
      if(j==0)
        cout << setw(4) << i->first;
      if((j!=0) && !(j%70))
        cout << "'" << endl;
      if(!(j%70))
        cout << Opcode::SPACER << "db" << Opcode::SPACER << "'";

      cout << i->second[j];
    }
    cout << "'" << endl;
    cout << Opcode::SPACER << "db" << Opcode::SPACER << "00" << endl;
  }
}*/

/*unsigned short UCFunc::read_ushort(const unsigned char *buff)
{
//  assert(((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)))
//         ==(*(unsigned short *)buff));
  return ((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)));
}

void UCFunc::read_vchars(char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}

void UCFunc::read_vbytes(unsigned char *buffer, const unsigned long nobytes)
{
	_file->read(buffer, nobytes);
}*/

inline unsigned short read_ushort(ifstream &f)
{
  return ((unsigned short) ((unsigned int)f.get() + (((unsigned int)f.get()) << 8)));
}

inline unsigned char read_ubyte(ifstream &f)
{
  return (unsigned char)f.get();
}

#if 0
	#define DEBUG_READ
	#define DEBUG_READ_PAIR(X, Y) cout << '\t' << X << '\t' << Y << endl;
#else
	#undef DEBUG_READ
	#define DEBUG_READ_PAIR(X, Y)
#endif

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
		
		#ifdef DEBUG_READ
		cout << "Code Size: " << code_size << endl;
		#endif

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

			ucc_parse_parambytes(ucop, otd);
			code_offset+=otd.num_bytes;

			ucf._opcodes.push_back(ucop);
			#ifdef DEBUG_READ
			cout << setw(4) << code_size << "\t" << setw(4) << code_offset << "\t" << setw(4) << (unsigned int)ucop._offset << "\t" << setw(2) << (unsigned int)ucop._id << "\t";
			for(unsigned int i=0; i<ucop._params.size(); i++)
				cout << setw(2) << (unsigned int)ucop._params[i] << ',';
			cout << endl;
			#endif
		}
	}
}

void ucc_parse_parambytes(UCc &ucop, const UCOpcodeData &otd)
{
	unsigned int first=0;
	
	for(vector<string>::const_iterator s=otd.param_types.begin(); s!=otd.param_types.end(); ++s)
	{
		assert(first<ucop._params.size());
		unsigned int ssize=0;
		// all these are two bytes
		if(*s=="short")           ssize=2;
		else if(*s=="flag")       ssize=2;
		else if(*s=="extoffset")  ssize=2;
		else if(*s=="dataoffset") ssize=2;
		else if(*s=="varoffset")  ssize=2;
		else if(*s=="offset")     ssize=2;
		// and the single one byte type
		else if(*s=="byte")       ssize=1;
		else
		{
			cout << "error: data type '" << *s << "' is not defined. exiting." << endl;
			exit(1);
		}
		assert(ssize!=0);

		if(ssize==1)
			ucop._params_parsed.push_back((unsigned short)((unsigned int)ucop._params[first++]));
		else if(ssize==2)
			ucop._params_parsed.push_back((unsigned short) ((unsigned int)ucop._params[first++] + (((unsigned int)ucop._params[first++]) << 8)));
	}
}


void print_asm_data(UCFunc &ucf, ostream &o);
void print_asm_opcodes(ostream &o, UCFunc &ucf, const vector<UCOpcodeData> &optab);

/* prints the "assembler" output of the usecode, currently trying to duplicate
   the output of the original ucdump... */
void print_asm(UCFunc &ucf, ostream &o, const UCData &uc)
{
	if(uc.verbose()) cout << "Printing function..." << endl;

  o << "Function at file offset " << setw(8) << ucf._offset << "H" << endl;
  o << "\t.funcnumber " << setw(4) << ucf._funcid << "H" << endl;
  o << "\t.msize      " << setw(4) << ucf._funcsize << "H" << endl;
  o << "\t.dsize      " << setw(4) << ucf._datasize << "H" << endl;

  // debugging remove comments!
  if(ucf._data.size())
    print_asm_data(ucf, o);

  o << "Code segment at file offset " << setw(8) << ucf._codeoffset << "H" << endl;
  o << "\t.argc       " << setw(4) << ucf._num_args << "H" << endl;
  o << "\t.localc     " << setw(4) << ucf._num_locals << "H" << endl;
  o << "\t.externsize " << setw(4) << ucf._externs.size() << "H" << endl;

  for(unsigned int i=0; i<ucf._externs.size(); i++)
    o << Opcode::SPACER << "  .extern   " << setw(4) << ucf._externs[i] << "H" << endl;

    //o << "-----" << endl;
    //_opcodes[i]->print_asm(cout);
		print_asm_opcodes(o, ucf, opcode_table_data);
}

void print_asm_data(UCFunc &ucf, ostream &o)
{
	static const unsigned int nochars=70;
  // limit of about 70 chars to a line, wrap to the next line if longer then this...
  for(map<unsigned int, string, less<unsigned int> >::iterator i=ucf._data.begin(); i!=ucf._data.end(); i++)
  {
    for(unsigned int j=0; j<i->second.size(); j++)
    {
      if(j==0)
        o << setw(4) << i->first;
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

string demunge_ocstring(const string &asmstr, const vector<string> &param_types, const vector<unsigned int> &params, const UCc &op);
void output_raw_opcodes(ostream &o, const UCc &op);

void print_asm_opcodes(ostream &o, UCFunc &ucf, const vector<UCOpcodeData> &optab)
{
  for(vector<UCc>::iterator op=ucf._opcodes.begin(); op!=ucf._opcodes.end(); op++)
  {
		// offset
		o << setw(4) << op->_offset << ':';

		if(uc.rawops()) output_raw_opcodes(o, *op);
		else            o << '\t';

		o << demunge_ocstring(optab[op->_id].asm_nmo, optab[op->_id].param_types, op->_params_parsed, *op);

		if(uc.autocomment())
			o << demunge_ocstring(optab[op->_id].asm_comment, optab[op->_id].param_types, op->_params_parsed, *op);

		o << endl;
  }
}

void output_raw_opcodes(ostream &o, const UCc &op)
{
	// chars in opcode
	o << ' ' << setw(2) << (unsigned int)op._id;
	if(op._params.size()) cout << ' ';

	for(unsigned int i=0; i<op._params.size(); i++)
	{
		o << setw(2) << (unsigned int) op._params[i];
		if(i!=op._params.size())
			o << ' ';
	}

	// seperator
	unsigned int numsep = op._params.size();
	//cout << endl << numsep << endl;
	if(numsep>6)
		o << endl << "\t\t\t\t";
	else if (numsep>5)
		o << "\t";
	else if (numsep>2)
		o << "\t\t";
	else
		o << "\t\t\t";
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
	return op._offset + ((param>>15) ? (-1 * (0xFFFF - (unsigned short)param + 1)) : (int)param) + 1 + op._params.size();
}

string demunge_ocstring(const string &asmstr, const vector<string> &param_types, const vector<unsigned int> &params, const UCc &op)
{
	strstream str;
	str << setfill('0') << setbase(16);
	str.setf(ios::uppercase);

	if(asmstr.size()==0) return string(); // for the degenerate case

	bool finished=false; // terminating details are at end-of-while
  unsigned int i=0; // istr index
	unsigned int width=0; // width value for setw()

	while(!finished)
	{
		bool relative(false);
		char c = asmstr[i];
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
					// if it's a relative we need to do some calcs befor outputing it
					// set width to 4 for the moment, and get the next char
					else if(c=='r') { i++; c = asmstr[i]; width=4; relative=true;}
					// width defaults to 4 if it's not specified
					else            width=4;
					
					switch(c)
					{
						case '%': str << '%'; break;
						case '1': assert(params.size()>=1); str << setw(width) << ((relative) ? calcreloffset(op, params[0]) : params[0]); break;
						case '2': assert(params.size()>=2); str << setw(width) << ((relative) ? calcreloffset(op, params[1]) : params[1]); break;
						case '3': assert(params.size()>=3); str << setw(width) << ((relative) ? calcreloffset(op, params[2]) : params[2]); break;
						case '4': assert(params.size()>=4); str << setw(width) << ((relative) ? calcreloffset(op, params[3]) : params[3]); break;
						case '5': assert(params.size()>=5); str << setw(width) << ((relative) ? calcreloffset(op, params[4]) : params[4]); break;
						case '6': assert(params.size()>=6); str << setw(width) << ((relative) ? calcreloffset(op, params[5]) : params[5]); break;
						case '7': assert(params.size()>=7); str << setw(width) << ((relative) ? calcreloffset(op, params[6]) : params[6]); break;
						case '8': assert(params.size()>=8); str << setw(width) << ((relative) ? calcreloffset(op, params[7]) : params[7]); break;
						case '9': assert(params.size()>=9); str << setw(width) << ((relative) ? calcreloffset(op, params[8]) : params[8]); break;
						default:   // we'll silently drop errors... it's the only "clean" way
							str << '%' << c;
					}
				}
				break;
			default: // it's just a character, leave it be
				str << c;
		}

		i++;
    if(i==asmstr.size()) finished=true;
	}
	str << ends;
  return str.str();
}












