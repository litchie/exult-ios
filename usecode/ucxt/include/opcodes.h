
#ifndef OPCODES_H
#define OPCODES_H

#include <map>
#include <string>
#include <vector>
#include "Configuration.h"

vector<string> qnd_ocsplit(const string &s);

void map_type_size(const vector<string> &param_types, vector<pair<unsigned int, bool> > &param_sizes);

class UCOpcodeData
{
	public:
		UCOpcodeData() : opcode(0x00), num_bytes(0), num_pop(0),
		                 num_push(0), call_effect(0), flag_return(false),
		                 flag_paren(false), flag_indent_inc(false),
		                 flag_indent_dec(false), flag_indent_tmpinc(false),
		                 flag_indent_tmpdec(false)
		{};
		UCOpcodeData(const vector<string> &v)
		{
			if((v.size()==12)==false)
			{
				cerr << "Error in opcodes file:" << endl;
				for(unsigned int i=0; i<v.size(); i++)
					cerr << v[i] << '\t';
				cerr << endl;
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
		string         name;
		string         asm_nmo;
		string         asm_comment;
		string         ucs_nmo;
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
		
		vector<string> param_types;
		// values caluclated from param_types
		vector<pair<unsigned int, bool> > param_sizes; // .first==size of parameter in bytes .second==whether to treat it as a relative offset and calculate for it
		
};

extern vector<UCOpcodeData> opcode_table_data;
extern vector<pair<unsigned int, unsigned int> > opcode_jumps;

extern map<unsigned int, string> bg_uc_intrinsics;
extern map<unsigned int, string> si_uc_intrinsics;

void init_static_usecodetables(const Configuration &config);
void init_usecodetables(const Configuration &config, bool noconf, bool verbose);

#endif

