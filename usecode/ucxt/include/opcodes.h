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

#ifndef OPCODES_H
#define OPCODES_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include "Configuration.h"

std::vector<std::string> qnd_ocsplit(const std::string &s);

void map_type_size(const std::vector<std::string> &param_types, std::vector<std::pair<unsigned int, bool> > &param_sizes);

class UCOpcodeData
{
	public:
		UCOpcodeData() : opcode(0x00), num_bytes(0), num_pop(0),
		                 num_push(0), call_effect(0), flag_return(false),
		                 flag_paren(false), flag_indent_inc(false),
		                 flag_indent_dec(false), flag_indent_tmpinc(false),
		                 flag_indent_tmpdec(false)
		{};
		UCOpcodeData(const std::vector<std::string> &v)
		{
			if((v.size()==12)==false)
			{
				std::cerr << "Error in opcodes file:" << std::endl;
				for(unsigned int i=0; i<v.size(); i++)
					std::cerr << v[i] << '\t';
				std::cerr << std::endl;
			}
			
			assert(v.size()==12);
			opcode = strtol(v[1].c_str(), 0, 0);
			name = v[2];
			asm_nmo = v[3];
			asm_comment = v[4];
			ucs_nmo = v[5];
			num_bytes = strtol(v[6].c_str(), 0, 0);
			param_types = qnd_ocsplit(v[7]);
			num_pop = strtol(v[8].c_str(), 0, 0);
			num_push = strtol(v[9].c_str(), 0, 0);
			call_effect = strtol(v[10].c_str(), 0, 0);
			assert(v[11].size()>=6);
			flag_return        = (v[11][0]=='0') ? false : true;
			flag_paren         = (v[11][1]=='0') ? false : true;
			flag_indent_inc    = (v[11][2]=='0') ? false : true;
			flag_indent_dec    = (v[11][3]=='0') ? false : true;
			flag_indent_tmpinc = (v[11][4]=='0') ? false : true;
			flag_indent_tmpdec = (v[11][5]=='0') ? false : true;
			map_type_size(param_types, param_sizes);
		};
		
		unsigned int   opcode;
		std::string         name;
		std::string         asm_nmo;
		std::string         asm_comment;
		std::string         ucs_nmo;
		unsigned int   num_bytes;
		
		unsigned int   num_pop;
		unsigned int   num_push;
		unsigned int   call_effect;
		bool           flag_return;
		bool           flag_paren;
		bool           flag_indent_inc;
		bool           flag_indent_dec;
		bool           flag_indent_tmpinc;
		bool           flag_indent_tmpdec;
		
		std::vector<std::string> param_types;
		// values caluclated from param_types
		std::vector<std::pair<unsigned int, bool> > param_sizes; // .first==size of parameter in bytes .second==whether to treat it as a relative offset and calculate for it
		
};

extern std::vector<UCOpcodeData> opcode_table_data;
extern std::vector<std::pair<unsigned int, unsigned int> > opcode_jumps;

extern std::map<unsigned int, std::string> bg_uc_intrinsics;
extern std::map<unsigned int, std::string> si_uc_intrinsics;

void init_static_usecodetables();
void init_usecodetables(const Configuration &config, bool noconf, bool verbose);


#endif

