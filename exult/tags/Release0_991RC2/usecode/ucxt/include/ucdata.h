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

#ifndef UCDATA_H
#define UCDATA_H

#include <string>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "ucfunc.h"

class UCData
{
	public:
		UCData();
		~UCData();
		
		void dump_unknown_opcodes();
		void dump_unknown_intrinsics();
		
		void parse_params(const unsigned int argc, char **argv);
		void open_usecode(const std::string &filename);
		void load_funcs();
		
		void disassamble();
		void dump_flags(std::ostream &o);
		void output_extern_header(std::ostream &o);
		
		std::string output_redirect() const { return _output_redirect; };
		std::string input_usecode_file() const { return _input_usecode_file; };
		
		bool fail() const { return _file.fail(); };
	
		const std::map<unsigned int, UCFuncSet> &funcmap() { return _funcmap; };	
		
		const UCOptions &opt() { return options; };
		
		UCOptions options;
		
	private:
		
		void file_open(const std::string &filename);
		void file_seek_start() { _file.seekg(0, std::ios::beg); };
		void file_seek_end() { _file.seekg(0, std::ios::end); };

		std::ifstream _file;
		
		std::string _output_redirect;
		std::string _input_usecode_file;
		
		unsigned int _funcid;
		
		std::vector<UCc> _codes;
		
		std::vector<UCFunc *> _funcs;
		
		/* Just a quick mapping between funcs and basic data on them.
		   Just something we can quickly pass to the parsing functions
		   so we don't have to give them an entire function to play with. */
		FuncMap _funcmap;
		
		long _search_opcode;
		long _search_intrinsic;
		std::vector<unsigned int> search_funcs;
};

#endif

