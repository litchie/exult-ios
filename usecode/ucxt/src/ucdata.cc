#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "ucdata.h"
#include "printucc.h"
#include "opcodes.h"
#include "files/utils.h"

UCData::UCData() : _noconf(false), _rawops(false), _autocomment(false),
                   _verbose(false), _mode(MODE_NONE), _game(GAME_BG),
                   _output_list(false), _output_asm(false), _output_ucs(false),
                   _output_ucz(false), _mode_all(false), _mode_dis(false),
                   _search_opcode(-1),
                   _search_intrinsic(-1),
                   _search_func(-1)
{
}

UCData::~UCData()
{
	_file.close();
	for(unsigned int i=0; i<_funcs.size(); i++)
		delete _funcs[i];
}

void UCData::parse_params(const unsigned int argc, char **argv)
{
	/* Parse command line */
	for(unsigned int i=1; i<argc; i++)
	{
		if     (strcmp(argv[i], "-si")==0)  _game    = GAME_SI;
		else if(strcmp(argv[i], "-bg")==0)  _game    = GAME_BG;

		else if(strcmp(argv[i], "-a" )==0)  { _mode  = MODE_ALL; _mode_all=true; }
		else if(strcmp(argv[i], "-f" )==0)  _mode    = MODE_FLAG_DUMP;
//		else if(strcmp(argv[i], "-a" )==0)  _mode    = MODE_DISASSEMBLE_ALL;
//		else if(strcmp(argv[i], "-c" )==0)  _mode    = MODE_OPCODE_SCAN; // Opcode scan mode

		else if(strcmp(argv[i], "-nc")==0)  _noconf  = true;
		else if(strcmp(argv[i], "-ro")==0)  _rawops  = true;
		else if(strcmp(argv[i], "-ac")==0)  _autocomment  = true;
		else if(strcmp(argv[i], "-v" )==0)  _verbose = true;

		else if(strcmp(argv[i], "-fl" )==0)  _output_list = true;
		else if(strcmp(argv[i], "-fa" )==0)  _output_asm  = true;
		else if(strcmp(argv[i], "-fs" )==0)  _output_ucs  = true;
		else if(strcmp(argv[i], "-fz" )==0)  _output_ucz  = true;

		else if(argv[i][0] != '-')
		{
			char* stopstr;
			/* Disassembly mode */
			_search_func = strtoul(argv[i], &stopstr, 16);
			if( stopstr - argv[i] < (int)strlen(argv[i]) )
				/* Invalid number */
				_search_func = (unsigned long) -1;
			else
			{
				if(verbose()) cout << "Disassembling Function: " << _search_func << endl;
				_mode = MODE_DISASSEMBLY;
				_mode_dis = true;
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

void UCData::disassamble()
{
	load_funcs();

	cout << "Looking for function number " << setw(8) << _search_func << endl << endl;

	if(output_list())
		cout << "Function     offset    size  data  code" << endl;

	bool _foundfunc=false; //did we find and print the function?
	for(unsigned int i=0; i<_funcs.size(); i++)
	{
		//cout << _funcs[i]->_funcid << "\t" << _search_func << endl;
		if(mode_all() || (mode_dis() && (_funcs[i]->_funcid==_search_func)))
		{
			_foundfunc=true;
			bool _func_printed=false; // to test if we've actually printed a function ouput

			if(output_ucs())
			{
				output_ucfunc(_funcid, _funcs[i]->_data, _funcs[i]->_num_args, _funcs[i]->_num_locals, _funcs[i]->_externs,
				              _funcs[i]->_opcodes);
				_func_printed=true;
			}

			if(output_ucz())
			{
				_funcs[i]->parse_ucs(_funcmap);
				_funcs[i]->output_ucs(cout, _funcmap, ((_game == GAME_SI) ? si_uc_intrinsics : bg_uc_intrinsics), false);
				_func_printed=true;
			}

			if(output_list())
			{
			    cout << "#" << setbase(10) << setw(3) << i << setbase(16) << ": "
			         << setw(4) << _funcs[i]->_funcid   << "H  "
			         << setw(8) << _funcs[i]->_offset   << "  "
			         << setw(4) << _funcs[i]->_funcsize << "  "
			         << setw(4) << _funcs[i]->_datasize << "  "
			         << setw(4) << _funcs[i]->codesize() << "  "
			         << endl;
				_func_printed=true;
			}

			// if we haven't printed one by now, we'll print an asm output.
			if(output_asm() || (_func_printed==false))
				print_asm(*_funcs[i], cout, _funcmap, ((_game == GAME_SI) ? si_uc_intrinsics : bg_uc_intrinsics), *this);
		}
	}

	if(!_foundfunc)
		printf("Function not found.\n");

	if( _search_func == -1 )
	{
		printf("Functions: %d\n", _funcs.size());
	}

//	if(uc.mode()==MODE_OPCODE_SEARCH)
//		output_flags(funcs);

	if(output_list())
	  cout << endl << "Functions: " << setbase(10) << _funcs.size() << setbase(16) << endl;

}

void UCData::disassamble_all()
{
	cout << "Loading Funcs..." << endl;
	load_funcs();
	cout << "Funcs Loaded..." << endl;

	for(unsigned int i=0; i<_funcs.size(); i++)
	{
		cout << "Outputting " << i << endl;
		output_ucfunc(_funcs[i]->_funcid, _funcs[i]->_data, _funcs[i]->_num_args, _funcs[i]->_num_locals, _funcs[i]->_externs,
		              _funcs[i]->_opcodes);
		cout << "Outputted " << i << endl;
	}
}

void UCData::dump_flags()
{
  load_funcs();

  // don't need to delete "flags" the pointers are already owned.
  vector<FlagData *> flags;

  // *BLEH* ugly!
  for(unsigned int i=0; i<_funcs.size(); i++)
    for(unsigned int j=0; j<_funcs[i]->_flagcount.size(); j++)
      flags.push_back(_funcs[i]->_flagcount[j]);

  cout << "Number of flags found: " << setbase(10) << flags.size() << endl;

  // output per function
  {
    sort(flags.begin(), flags.end(), SortFlagDataLessFunc());

    cout << setbase(16) << setfill('0');
    int currfunc = -1;
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

void UCData::list_funcs()
{
  load_funcs();

/*  if( ( ( uc._search_func == -1 ) || (uc.mode()==MODE_OPCODE_SCAN) )
     && ( uc._search_opcode == -1 ) && ( uc._search_intrinsic == -1 ) )*/
    cout << "Function     offset    size  data  code" << endl;

  for(unsigned int i=0; i<_funcs.size(); i++)
  {
    cout << "#" << setbase(10) << setw(3) << i << setbase(16) << ": "
         << setw(4) << _funcs[i]->_funcid   << "H  "
         << setw(8) << _funcs[i]->_offset   << "  "
         << setw(4) << _funcs[i]->_funcsize << "  "
         << setw(4) << _funcs[i]->_datasize << "  "
         << setw(4) << _funcs[i]->codesize() << "  "
         << endl;
  }
  cout << endl << "Functions: " << setbase(10) << _funcs.size() << setbase(16) << endl;
}

void UCData::file_open(const string &filename)
{
	/* Open a usecode file */
	U7open(_file, filename.c_str(), false);
}


void UCData::load_funcs()
{
	bool eof=false;
	while( !eof )
	{
		UCFunc *ucfunc = new UCFunc();

/*    if( ( ( uc._search_func == -1 )
        || ( uc.mode() == MODE_OPCODE_SCAN ) )
       && ( uc._search_opcode == -1 ) && ( uc._search_intrinsic == -1 ) )
      cout << "#" << setbase(10) << setw(3) << func.size() * current function number* << setbase(16) << ": ";*/

		readbin_UCFunc(_file, *ucfunc);
		
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
	
	for(vector<UCFunc *>::iterator i=_funcs.begin(); i!=_funcs.end(); i++)
	{
		_funcmap.insert(FuncMapPair((*i)->_funcid, UCFuncSet((*i)->_funcid, (*i)->_num_args)));
	}
/*	for(map<unsigned short, UCFuncSet>::iterator i=_funcmap.begin(); i!=_funcmap.end(); i++)
		cout << i->first << "\t" << i->second.num_args << endl;*/
		
}



