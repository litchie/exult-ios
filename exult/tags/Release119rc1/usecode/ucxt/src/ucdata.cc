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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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
#include "ops.h"
#include "files/utils.h"

using std::cout;
using std::setw;
using std::endl;
using std::vector;
using std::setbase;
using std::setfill;
using std::string;
using std::ostream;

UCData::UCData() : _search_opcode(-1), _search_intrinsic(-1)
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
		if     (strcmp(argv[i], "-si"   )==0) options._game = UCOptions::GAME_SI;
		else if(strcmp(argv[i], "-bg"   )==0) options._game = UCOptions::GAME_BG;
		else if(strcmp(argv[i], "-u8"   )==0) options._game = UCOptions::GAME_U8;

		else if(strcmp(argv[i], "-a"    )==0) options.mode_all=true;

		else if(strcmp(argv[i], "-nc"   )==0) options.noconf  = true;
		else if(strcmp(argv[i], "-ro"   )==0) options.rawops  = true;
		else if(strcmp(argv[i], "-ac"   )==0) options.autocomment = true;
		else if(strcmp(argv[i], "-uc"   )==0) options.uselesscomment = true;
		else if(strcmp(argv[i], "-v"    )==0) options.verbose = true;
		else if(strcmp(argv[i], "-vv"   )==0) { options.verbose = options.very_verbose = true; }
		else if(strcmp(argv[i], "-dbg"  )==0) options.ucdebug = true;
		else if(strcmp(argv[i], "-ext32")==0) options.force_ext32 = true;
		else if(strcmp(argv[i], "-b"    )==0) options.basic = true;

		else if(strcmp(argv[i], "-fl"   )==0) options.output_list = true;
		else if(strcmp(argv[i], "-fa"   )==0) options.output_asm  = true;
		else if(strcmp(argv[i], "-fz"   )==0) options.output_ucs  = true;
		else if(strcmp(argv[i], "-fs"   )==0) options.output_ucs  = true;
		else if(strcmp(argv[i], "-ff"   )==0) options.output_flag = true;
		else if(strcmp(argv[i], "-ftt"  )==0) options.output_trans_table = true;

		else if(strcmp(argv[i], "--extern-header" )==0) options.output_extern_header = true;
		
		else if(argv[i][0] != '-')
		{
			char* stopstr;
			/* Disassembly mode */
			unsigned int search_func = strtoul(argv[i], &stopstr, 16);
			if( stopstr - argv[i] < static_cast<int>(strlen(argv[i])) )
				/* Invalid number */
			{ /* Do Nothing */ }
			else
			{
				search_funcs.push_back(search_func);
				if(options.verbose) cout << "Disassembling Function: " << search_func << endl;
				options.mode_dis = true;
			}
		}
		else if((string(argv[i]).size()>2) && string(argv[i]).substr(0, 2)=="-o")
		{
			_output_redirect = string(argv[i]).substr(2, string(argv[i]).size()-2);
			if(options.verbose) cout << "Outputting to filename: " << _output_redirect << endl;
		}
		else if((string(argv[i]).size()>2) && string(argv[i]).substr(0, 2)=="-i")
		{
			_input_usecode_file = string(argv[i]).substr(2, string(argv[i]).size()-2);
			if(options.verbose) cout << "Inputting from file: " << _input_usecode_file << endl;
		}
		else
		{
			cout << "unsupported parameter " << argv[i] << " detected. countinuing." << endl;
		}
	}
}

void UCData::open_usecode(const string &filename)
{
	file_open(filename);
	
	if(fail())
		return;
}

// FIXME: Pass ostream to this, rather then cout-ing everything
void UCData::disassamble()
{
	load_funcs();

	if(options.verbose)
	{
		for(vector<unsigned int>::iterator i=search_funcs.begin(); i!=search_funcs.end(); i++)
			cout << "Looking for function number " << setw(8) << (*i) << endl;
		cout << endl;
	}
	
	if(options.output_list)
		cout << "Function       offset    size  data  code" << (options.ucdebug ? " funcname" : "") << endl;

	if(options.output_trans_table)
		cout << "<trans>" << endl;
		
	bool _foundfunc=false; //did we find and print the function?
	for(unsigned int i=0; i<_funcs.size(); i++)
	{
		if(options.mode_all || (options.mode_dis && count(search_funcs.begin(), search_funcs.end(), _funcs[i]->_funcid)))
		{
			_foundfunc=true;
			bool _func_printed=false; // to test if we've actually printed a function ouput

			if(options.output_list)
				_func_printed = _funcs[i]->output_list(cout, i, options);
			
			if(options.output_ucs)
			{
				_funcs[i]->parse_ucs(_funcmap, uc_intrinsics, options);
				_func_printed = _funcs[i]->output_ucs(cout, _funcmap, uc_intrinsics, options);
				//_func_printed=true;
			}

			if(options.output_trans_table)
			{
				_func_printed=_funcs[i]->output_tt(cout);
				//_func_printed=true;
			}
			
			// if we haven't printed one by now, we'll print an asm output.
			if(options.output_asm || (_func_printed==false))
				_funcs[i]->output_asm(cout, _funcmap, uc_intrinsics, options);
		}
	}

	if(!_foundfunc)
		printf("Function not found.\n");

	if(search_funcs.size()==0)
	{
		printf("Functions: %d\n", _funcs.size());
	}

	if(options.output_list)
		cout << endl << "Functions: " << setbase(10) << _funcs.size() << setbase(16) << endl;
	
	if(options.output_trans_table)
		cout << "</>" << endl;
		
	cout << endl;
}

/* FIXME: Need to remove the hard coded opcode numbers (0x42 and 0x43) and replace them
	with 'variables' in the opcodes.txt file, that signify if it's a pop/push and a flag */
void UCData::dump_flags(ostream &o)
{
	if(!(options.game_bg() || options.game_si()))
	{
		o << "This option only works for U7:BG and U7:SI" << endl;
		return;
	}
	load_funcs();
	
	if(options.verbose) cout << "Finding flags..." << endl;
	vector<FlagData> flags;

	// *BLEH* ugly!
	for(vector<UCFunc *>::iterator func=_funcs.begin(); func!=_funcs.end(); func++)
		for(vector<UCc>::iterator op=(*func)->_opcodes.begin(); op!=(*func)->_opcodes.end(); op++)
		{
			if(op->_id==0x42)
				flags.push_back(FlagData((*func)->_funcid, op->_offset, op->_params_parsed[0], FlagData::GETFLAG));
			else if(op->_id==0x43)
				flags.push_back(FlagData((*func)->_funcid, op->_offset, op->_params_parsed[0], FlagData::SETFLAG));
		}
		
	o << "Number of flags found: " << setbase(10) << flags.size() << endl << endl;

	// output per function
	{
		sort(flags.begin(), flags.end(), SortFlagDataLessFunc());
		
		o << setbase(16) << setfill('0');
		int currfunc = -1;
		for(unsigned int i=0; i<flags.size(); i++)
		{
			if(currfunc!=flags[i].func())
			{
				o << "Function: " << setw(4) << flags[i].func() << endl;
				currfunc=flags[i].func();
				o << "              flag  offset" << endl;
			}
			
			o << "        ";
			if(flags[i].access()==FlagData::GETFLAG)
				o << "push  ";
			else if(flags[i].access()==FlagData::SETFLAG)
				o << "pop   ";
			o << setw(4) << flags[i].flag()   << "  "
			  << setw(4) << flags[i].offset() << endl;
		}
	}
	
	// output per flag
	{
		sort(flags.begin(), flags.end(), SortFlagDataLessFlag());
		
		o << setbase(16) << setfill('0');
		unsigned int currflag = static_cast<unsigned int>(-1);
		for(unsigned int i=0; i<flags.size(); i++)
		{
			if(currflag!=flags[i].flag())
			{
				o << "Flag: " << setw(4) << flags[i].flag() << endl;
				currflag=flags[i].flag();
				o << "              func  offset" << endl;
			}
		
		o << "        ";
		if(flags[i].access()==FlagData::GETFLAG)
			o << "push  ";
		else if(flags[i].access()==FlagData::SETFLAG)
			o << "pop   ";
		o << setw(4) << flags[i].func()   << "  "
		  << setw(4) << flags[i].offset() << endl;
		}
	}
}

void UCData::file_open(const string &filename)
{
	/* Open a usecode file */
	U7open(_file, filename.c_str(), false);
}

#undef LOAD_SPEED_TEST

#ifdef LOAD_SPEED_TEST
#include "tools/dbgutils.h"
#endif

void UCData::load_funcs()
{
	if(options.verbose) cout << "Loading functions..." << endl;
	
	#ifdef LOAD_SPEED_TEST
	dbg::DateDiff dd;
	dbg::timeDateDiff(cout);
	dd.start();
	#endif
	
	bool eof=false;
	while( !eof )
	{
		UCFunc *ucfunc = new UCFunc();

		if(options.game_bg() || options.game_si())
			readbin_U7UCFunc(_file, *ucfunc, options);
		else if(options.game_u8())
			readbin_U8UCFunc(_file, *ucfunc);
		else
			exit(-1); // can't happen
		
		/* if we're forcing ext32 on output, this is where we do so.
			if we try doing it before reading it, it'll get corrupted. */
		if(options.force_ext32) ucfunc->ext32=true;
		
		_funcs.push_back(ucfunc);
		{
			_file.get();
			eof = _file.eof();
			_file.unget();
		}
	}
	
	#ifdef LOAD_SPEED_TEST
	dd.end();
	cout << setbase(10) << setfill(' ');
	dd.print_start(cout) << endl;
	dd.print_end(cout) << endl;
	dd.print_diff(cout) << endl;
	cout << setbase(16) << setfill('0');
	#endif
	
	if(options.verbose) cout << "Creating function map..." << endl;
	
	for(vector<UCFunc *>::iterator i=_funcs.begin(); i!=_funcs.end(); i++)
	{
		_funcmap.insert(FuncMapPair((*i)->_funcid, UCFuncSet((*i)->_funcid, (*i)->_num_args, (*i)->return_var, (*i)->funcname)));
	}
/*	for(map<unsigned int, UCFuncSet>::iterator i=_funcmap.begin(); i!=_funcmap.end(); i++)
		cout << i->first << "\t" << i->second.num_args << endl;*/
}

void UCData::output_extern_header(ostream &o)
{
	if(!(options.game_bg() || options.game_si()))
	{
		o << "This option only works for U7:BG and U7:SI" << endl;
		return;
	}
	load_funcs();

	for(vector<UCFunc *>::iterator func=_funcs.begin(); func!=_funcs.end(); func++)
	{
		//(*func)->output_ucs_funcname(o << "extern ", _funcmap, (*func)->_funcid, (*func)->_num_args, (*func)->return_var) << ';' << endl;
		(*func)->output_ucs_funcname(o << "extern ", _funcmap) << ';' << endl;
	}
}


