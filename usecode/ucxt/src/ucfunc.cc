
#include "ucfunc.h"
#include "newopcode.h"
#include <set>
#include <strstream>

const unsigned int SIZEOF_USHORT = 2;

const string VARNAME = "uvar";
const string VARPREFIX = "var";

//#define TEST_V3 false
#define TEST_V3 true

UCFunc::~UCFunc()
{
}

/* Prints module's data segment */
void UCFunc::process_data_seg()
{
  long pos = ftell();

  unsigned short off = 0;

  /* Load all strings & their offsets */
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
  fseek(pos, SEEK_SET);
}

#include "opcodes.h"

/*
 Prints single opcode
 Return number of bytes to advance the code pointer
 Prints first characters of strings referenced
*/
unsigned short UCFunc::print_opcode(unsigned char* ptrc, unsigned short coffset,
                            unsigned char* pdataseg,
                            unsigned short* pextern,
                            unsigned short externsize,
//                            unsigned char* opcode_buf,
                            unsigned char* intrinsic_buf,
                            int mute,
                            int count_all_opcodes,
                            int count_all_intrinsic,
//                            vector<UCc> &uc_codes,
                            const char** func_table)
{
//  if( count_all_opcodes )
//  _unknown_opcode_count[*ptrc]++;

  /* Find the description */
  const opcode_desc *pdesc = ( *ptrc >= ( sizeof(opcode_table) / sizeof( opcode_desc ) ) ) ?
                        NULL : opcode_table + ( *ptrc );
  /* Unknown opcode */
  if( pdesc && ( pdesc->mnemonic == NULL ) )
    pdesc = NULL;

    /* Unknown opcode */
//  if( ( pdesc == NULL ) && !count_all_opcodes )
//    _unknown_opcode_count[*ptrc]++;

  /* Number of bytes to print */
  unsigned int nbytes = pdesc ? ( pdesc->nbytes + 1 ) : 1;

  UCc ucc;
  ucc._offset = coffset;

  /* Print bytes */
  for(unsigned int i = 0; i < nbytes; i++ )
  {
    if(i==0) ucc._id = ptrc[i];
    else     ucc._params.push_back(ptrc[i]);
  }

  _codes.push_back(ucc);//PATRICK
  _raw_opcodes.push_back(NewOpcode(ucc._offset, ucc._id, ucc._params));
  //new MiscOpcode(ucc._offset, ucc._id, ucc._params)
  /* Print operands if any */
  return nbytes;
}

void UCFunc::process_code_seg(//unsigned char* opcode_buf,
                          unsigned char* intrinsic_buf,
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
  fseek(pos, SEEK_SET);
  /* Print code segment header */
  if( size < 3 * SIZEOF_USHORT )
  {
    printf("Code segment bad!\n");
    free(p);
    free(pdata);
    return;
  }
  /* Print argument counter */
  _argc = read_ushort(pp);
  pp += SIZEOF_USHORT;
  /* Print locals counter */ 
  _localc = read_ushort(pp);
  pp += SIZEOF_USHORT;
  /* Print externs section */
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
  /* Print opcodes */
  while( offset < size )
  {
    unsigned int nbytes = print_opcode(pp, offset, pdata, pextern, externsize,
                              /*opcode_buf,*/ intrinsic_buf, mute,
                              count_all_opcodes,
                              count_all_intrinsic,
                              /*uc_codes,*/ func_table);
    pp += nbytes;
    offset += nbytes;
  }
  free(p);
  free(pdata);
}

void UCFunc::process_old(FILE* file, long func, int* found,
//                            unsigned char* opcode_buf,
                          unsigned char* intrinsic_buf,
                          bool scan_mode,
                          unsigned long opcode,
                          unsigned long intrinsic,
                          unsigned int &uc_funcid,
//                          vector<UCc> &uc_codes,
                          const char** func_table)
{
  _file = file;

  long bodyoff;
  /* Save start offset */
  _offset = ftell();
  /* Read function header */
  _funcid = read_ushort();
  _funcsize = read_ushort();

  /* Save body offset */
  bodyoff = ftell();
  _datasize = read_ushort();

  if( ( _funcid == func ) || scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ) )
  {
    /* Only for matching function or in one of the scan modes */
    if( _funcid == func )
    {
      *found = 1;
//      printf("Function at file offset %08lX\n\t.funcnumber\t%04XH\n"
//        "\t.msize\t%04XH\n\t.dsize\t%04XH\n", _offset, _funcid, _funcsize, _datasize);
      { //PATRICK
        uc_funcid = _funcid;
      }
    }
    /* Dump function contents */
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
      process_data_seg();
//    if( opcode != -1 )
//      memset(opcode_buf, 0, 256);
    if( intrinsic != -1 ) 
      memset(intrinsic_buf, 0, 256);
    process_code_seg(/*opcode_buf,*/ intrinsic_buf,
              scan_mode || ( opcode != -1 ) || ( intrinsic != -1 ),
              ( opcode != -1 ), ( intrinsic != -1 ),
              /*uc_codes,*/ func_table);
    if( !scan_mode && ( opcode == -1 ) && ( intrinsic == -1 ) )
    {
      do_decompile();
      if(TEST_V3) do_print();
    }

    if( ( ( opcode != -1 ) && _unknown_opcode_count[opcode] > 0 ) ||
      ( ( intrinsic != -1 ) && intrinsic_buf[intrinsic] > 0 ) )
    {
      /* Found */
      *found = 1;
      if( intrinsic != -1 )
        printf("\tFound function (%04XH) - %d times\n", _funcid,
                                intrinsic_buf[intrinsic]);
      else
        printf("\tFound function (%04XH) - %d times\n", _funcid, _unknown_opcode_count[opcode]);
    }
  }

  genflags();
  /* Seek back, then to next function */
  fseek(bodyoff, SEEK_SET);
  fseek(_funcsize, SEEK_CUR);
}

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


unsigned short UCFunc::read_ushort()
{
  return ((unsigned short) ((unsigned int)getc(_file) + (((unsigned int)getc(_file)) << 8)));
}

unsigned short UCFunc::read_ushort(const unsigned char *buff)
{
//  assert(((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)))
//         ==(*(unsigned short *)buff));
  return ((unsigned short) ((unsigned int)buff[0] + (((unsigned int)buff[1]) << 8)));//*(unsigned short *)buff;
}

void UCFunc::read_vchars(char *buffer, const unsigned long nobytes)
{
  ::fread(buffer, 1, nobytes, _file);
}

void UCFunc::read_vbytes(unsigned char *buffer, const unsigned long nobytes)
{
  ::fread(buffer, 1, nobytes, _file);
}

void UCFunc::do_decompile()
{
  /* TODO: Generate Unknown opcode count (_unknown_opcode_count) */
  /* TODO: Generate opcode count (_opcode_count)
           eg: _opcode_count[ucc._id]++; */

  genflags();

  for(unsigned int i=0; i<_raw_opcodes.size(); i++)
    _raw_opcodes[i]->pass1(_externs);

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
  Label *currlbl = new Label(0);

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
        currlbl = new Label(_raw_opcodes[i]->offset());
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

/* prints out the external function calls in C output */
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

/* prints the "head" of the C function, eg:
     void Func0001(uvar var0001)
     {
*/
void UCFunc::print_c_head()
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

/* outputs the local variable definitions of the C output */

void UCFunc::print_c_local()
{
  strstream strlocal;

  strlocal << setfill('0') << setbase(16);

  if(_localc)
  {
    for(unsigned int i=_argc; i<(_argc+_localc); i++)
    {
      // TODO: more indenting fuss
      strlocal << "    ";
      strlocal << VARNAME << " " << VARPREFIX << setw(4) << i << ";" << endl;
    }
    strlocal << endl;
  }
  strlocal << ends;
  cout << strlocal.str() << endl;
}

/* prints out the body of the C function, should be RELATIVLY simple,
   since most of the work should be done */
void UCFunc::print_c_body()
{

}

/* prints out the tail of the C function */
void UCFunc::print_c_tail()
{

  cout << endl << "}" << endl << endl;
}

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

void UCFunc::genflags()
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
}

/* prints the "assembler" output of the usecode, currently trying to duplicate
   the output of the original ucdump... */
void UCFunc::print_asm()
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
}

void UCFunc::print_asm_data()
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
}

