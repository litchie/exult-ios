#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "ucdata.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "printucc.h"
#include <algorithm>

UCData::UCData()  : _search_opcode((unsigned long) -1),
                    _search_intrinsic((unsigned long) -1),
                    _search_func((unsigned long)-1), _mode(0), _game(GAME_BG),
                    _noconf(false), _verbose(false)
{
	_opcode_buf.resize(MAX_OPCODE_BUF, 0);
	_intrinsic_buf.resize(MAX_INTRINSIC_BUF, 0);
}

UCData::~UCData()
{
	_file.close();
	for(unsigned int i=0; i<_funcs.size(); i++)
		delete _funcs[i];
}

void UCData::dump_unknown_opcodes()
{
//  if( ( _mode == MODE_DISASSEMBLY ) || ( _mode == MODE_OPCODE_SCAN ) )
	int found = 0;
	for( unsigned int i=0; i<MAX_OPCODE_BUF; i++ )
		if( _opcode_buf[i] )
		{
			if( !found )
			{
				cout << "Undefined opcodes found" << endl;
				found = 1;
			}
			cout << "0x" << setbase(16) << setfill('0') << setw(2) << i
			     << " (" << setbase(10) << (unsigned int)_opcode_buf[i]
			     << " time" << ((_intrinsic_buf[i]>1) ? "s" : "") << ")" << endl;
		}
}

void UCData::dump_unknown_intrinsics()
{
	int found = 0;
	for( unsigned int i=0; i<MAX_INTRINSIC_BUF; i++ )
		if( _intrinsic_buf[i] )
		{
			if( !found )
			{
				cout << "Undefined intrinsic functions found" << endl;
				found = 1;
			}
			cout << "0x" << setbase(16) << setfill('0') << setw(2) << i
			     << " (" << setbase(10) << (unsigned int)_intrinsic_buf[i]
			     << " time" << ((_intrinsic_buf[i]>1) ? "s" : "") << ")" << endl;
		}
}

void UCData::parse_params(const int argc, char **argv)
{
	/* Parse command line */
	for(unsigned int i=1; i<argc; i++)
	{
		if     (strcmp(argv[i], "-si")==0)  _game    = GAME_SI;
		else if(strcmp(argv[i], "-bg")==0)  _game    = GAME_BG;
		else if(strcmp(argv[i], "-l" )==0)  _mode    = MODE_LIST;
		else if(strcmp(argv[i], "-f" )==0)  _mode    = MODE_FLAG_DUMP;
//		else if(strcmp(argv[i], "-a" )==0)  _mode    = MODE_DISASSEMBLE_ALL;
//		else if(strcmp(argv[i], "-c" )==0)  _mode    = MODE_OPCODE_SCAN; // Opcode scan mode
		else if(strcmp(argv[i], "-nc")==0)  _noconf  = true;
		else if(strcmp(argv[i], "-v" )==0)  _verbose = true;
		else if(argv[i][0] != '-')
		{
			char* stopstr;
			/* Disassembly mode */
			_search_func = strtoul(argv[i], &stopstr, 16);
			if( stopstr - argv[i] < strlen(argv[i]) )
				/* Invalid number */
				_search_func = (unsigned long) -1;
			else
			{
				if(verbose()) cout << "Disassembling Function: " << _search_func << endl;
				_mode = MODE_DISASSEMBLY;
			}
		}
		else if((string(argv[i]).size()>2) && string(argv[i]).substr(0, 2)=="-o")
		{
			_output_redirect = string(argv[i]).substr(2, string(argv[i]).size()-2);
			if(verbose()) cout << "Outputting to filename: " << _output_redirect << endl;
		}
		else
		{
			cout << "unsupported parameter " << argv[i] << " detected. countinuing." << endl;
		}
	}
/*  if( argc == 3 )
  {
    if( !strcmp(argv[1], "-o") )
    {
      char* stopstr;
      // Opcode search
      _search_opcode = strtoul(argv[2], &stopstr, 16);
      if( stopstr - argv[2] < strlen(argv[2]) )
        _search_opcode = (unsigned long) -1;
      else
        // Hex opcode OK
        _mode = MODE_OPCODE_SEARCH;
    }
    else if( !strcmp(argv[1], "-i") )
    {
      char* stopstr;
      // Intrinsic function search
      _search_intrinsic = strtoul(argv[2], &stopstr, 16);
      if( stopstr - argv[2] < strlen(argv[2]) )
        _search_intrinsic = (unsigned long) -1;
      else
        // Hex opcode OK
        _mode = MODE_INTRINSIC_SEARCH;
    }
  }
  else*/
}

void UCData::open_usecode(const string &filename)
{
	file_open(filename);
	
	if(fail())
		return;
}

void UCData::disassamble(const char **func_table)
{
	load_funcs(func_table);

	cout << "Looking for function number " << setw(8) << _search_func << endl << endl;

	for(unsigned int i=0; i<_funcs.size(); i++)
	{
		//cout << _funcs[i]->funcid() << ":" << _funcid << endl;
		if(_funcs[i]->funcid()==_funcid)
		{
			output_ucfunc(_funcid, _funcs[i]->data(), _funcs[i]->argc(), _funcs[i]->localc(), _funcs[i]->externs(),
			              _funcs[i]->codes(), func_table);
			break;
		}
		else if(i==_funcs.size()-1)
			printf("Function not found.\n");
	}

	if( _search_func == -1 )
	{
		printf("Functions: %d\n", _funcs.size());
	}

//	if(uc.mode()==MODE_OPCODE_SEARCH)
//		output_flags(funcs);

	dump_unknown_opcodes();
	dump_unknown_intrinsics();
}

void UCData::disassamble_all(const char ** func_table)
{
  cout << "Loading Funcs..." << endl;
  load_funcs(func_table);
  cout << "Funcs Loaded..." << endl;

  for(unsigned int i=0; i<_funcs.size(); i++)
  {
    cout << "Outputting " << i << endl;
    output_ucfunc(_funcs[i]->funcid(), _funcs[i]->data(), _funcs[i]->argc(), _funcs[i]->localc(), _funcs[i]->externs(),
                  _funcs[i]->codes(), func_table);
    cout << "Outputted " << i << endl;
  }

}

void UCData::dump_flags(const char **func_table)
{
  load_funcs(func_table);

  // don't need to delete "flags" the pointers are already owned.
  vector<FlagData *> flags;

  // *BLEH* ugly!
  for(unsigned int i=0; i<_funcs.size(); i++)
    for(unsigned int j=0; j<_funcs[i]->flags().size(); j++)
      flags.push_back(_funcs[i]->flags()[j]);

  cout << "Number of flags found: " << setbase(10) << flags.size() << endl;

  // output per function
  {
    sort(flags.begin(), flags.end(), SortFlagDataLessFunc());

    cout << setbase(16) << setfill('0');
    unsigned int currfunc = (unsigned int)-1;
    for(unsigned int i=0; i<flags.size(); i++)
    {
      if(currfunc!=flags[i]->func())
      {
        cout << "Function: " << setw(4) << flags[i]->func() << endl;
        currfunc=flags[i]->func();
        cout << "              flag  offset" << endl;
      }

      cout << "        ";
      if(flags[i]->access()==FlagData::GETFLAG)
        cout << "push  ";
      else if(flags[i]->access()==FlagData::SETFLAG)
        cout << "pop   ";
      cout << setw(4) << flags[i]->flag()   << "  "
           << setw(4) << flags[i]->offset() << endl;
    }
  }
  // output per flag
  {
    sort(flags.begin(), flags.end(), SortFlagDataLessFlag());

    cout << setbase(16) << setfill('0');
    unsigned int currflag = (unsigned int)-1;
    for(unsigned int i=0; i<flags.size(); i++)
    {
      if(currflag!=flags[i]->flag())
      {
        cout << "Flag: " << setw(4) << flags[i]->flag() << endl;
        currflag=flags[i]->flag();
        cout << "              func  offset" << endl;
      }

      cout << "        ";
      if(flags[i]->access()==FlagData::GETFLAG)
        cout << "push  ";
      else if(flags[i]->access()==FlagData::SETFLAG)
        cout << "pop   ";
      cout << setw(4) << flags[i]->func()   << "  "
           << setw(4) << flags[i]->offset() << endl;
    }
  }
  // don't need to delete "flags" the pointers are already owned.
}

void UCData::list_funcs(const char **func_table)
{
  load_funcs(func_table);

/*  if( ( ( uc._search_func == -1 ) || (uc.mode()==MODE_OPCODE_SCAN) )
     && ( uc._search_opcode == -1 ) && ( uc._search_intrinsic == -1 ) )*/
    cout << "Function     offset    size  data  code" << endl;

  for(unsigned int i=0; i<_funcs.size(); i++)
  {
    cout << "#" << setbase(10) << setw(3) << i << setbase(16) << ": "
         << setw(4) << _funcs[i]->funcid()   << "H  "
         << setw(8) << _funcs[i]->offset()   << "  "
         << setw(4) << _funcs[i]->funcsize() << "  "
         << setw(4) << _funcs[i]->datasize() << "  "
         << setw(4) << _funcs[i]->codesize() << "  "
         << endl;
  }
  cout << endl << "Functions: " << setbase(10) << _funcs.size() << setbase(16) << endl;
}

void UCData::file_open(const string &filename)
{
	/* Open a usecode file */
	_file.open(filename.c_str(), ios::in | ios::binary);
}

class UCFuncNew
{
	public:
		UCFuncNew() {};
		
//	private:
	
		streampos      _offset;      // offset to start of function
		unsigned short _funcid;      // the id of the function
		unsigned short _funcsize;    // the size of the function (bytes)
		streampos      _bodyoffset;  // the file position after the header is read
		
		unsigned short _datasize;    // the size of the data block
		
		map<unsigned int, string, less<unsigned int> > _data;
			// contains the entire data segment in offset from start of segment, and string data pairs
		
		streampos      _code_offset; // the offset to the start of the code segment
		
		unsigned short _num_args;    // the number of arguments
		unsigned short _num_locals;  // the number of local variables
		unsigned short _num_externs; // the number of external function id's
		vector<unsigned short> _externs; // the external function id's
/*

    unsigned int _argc; // number of function parameters
    unsigned int _localc; // number of local variables

    long           _code_offset; // offset to start of code segment in file

*/
};

void UCData::load_funcs(const char **func_table)
{
	int found = 0;

	bool eof=false;
	while( !eof )
	{
		UCFunc *ucfunc = new UCFunc();

/*    if( ( ( uc._search_func == -1 )
        || ( uc.mode() == MODE_OPCODE_SCAN ) )
       && ( uc._search_opcode == -1 ) && ( uc._search_intrinsic == -1 ) )
      cout << "#" << setbase(10) << setw(3) << func.size() * current function number* << setbase(16) << ": ";*/
		streampos startpos = _file.tellg();
		
		ucfunc->process_old(&_file, _search_func, &found, _intrinsic_buf, ( mode() == MODE_OPCODE_SCAN ),
		                    _search_opcode, _search_intrinsic, _funcid, func_table);
		
		streampos endpos = _file.tellg();
		
		_file.seekg(startpos, ios::beg);
		
		UCFuncNew foo;
		readbin_UCFunc(_file, foo);
		
		#if 0
		cout << ucfunc->_code_offset << "\t" << foo._code_offset << endl;
		cout << ucfunc->argc() << "\t" << foo._num_args << endl;
		cout << ucfunc->localc() << "\t" << foo._num_locals << endl;
		cout << ucfunc->_externs.size() << "\t" << foo._num_externs << endl;
		#endif
		assert(ucfunc->funcid()==foo._funcid);
		assert(ucfunc->funcsize()==foo._funcsize);
		assert(ucfunc->datasize()==foo._datasize);
		#if 0 // no idea why these fail... but anyway...
		assert(ucfunc->argc()==foo._num_args);
		assert(ucfunc->localc()==foo._num_locals);
		#endif
		
		_file.seekg(endpos, ios::beg);
		
//    if( ( ( uc.mode() != MODE_OPCODE_SEARCH ) && ( uc.mode() !=MODE_INTRINSIC_SEARCH ) ) || found )
//      num_functions++;
//    if( ( uc.mode() == MODE_OPCODE_SEARCH ) || ( uc.mode() == MODE_INTRINSIC_SEARCH ) )
//      found = 0;

		_funcs.push_back(ucfunc);
		{
   	 	_file.get();
			eof = _file.eof();
			_file.unget();
		}
	}
}

inline unsigned short read_ushort(ifstream &f)
{
  return ((unsigned short) ((unsigned int)f.get() + (((unsigned int)f.get()) << 8)));
}

void UCData::readbin_UCFunc(ifstream &f, UCFuncNew &ucf)
{

	// offset to start of function
	ucf._offset = f.tellg();

	// Read Function Header
	
	ucf._funcid = read_ushort(f);
	ucf._funcsize = read_ushort(f);	
	
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
		streampos start_of_code_seg = f.tellg();
		ucf._code_offset = f.tellg();
		ucf._num_args = read_ushort(f);
		ucf._num_locals = read_ushort(f);
		ucf._num_externs = read_ushort(f);
		
	}
/*

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
*/
}

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


