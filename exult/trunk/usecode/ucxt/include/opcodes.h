
#ifndef OPCODES_H
#define OPCODES_H

#include <map>
#include <string>
#include <vector>
#include "Configuration.h"

#define EFF_RELATIVE_JUMP 0x0001
#define EFF_PUSH          0x0002
#define EFF_POP           0x0004
#define EFF_CMP           0x0008
#define EFF_BIMATH        0x0010
#define EFF_UNIMATH       0x0020
#define EFF_CALLI         0x0040
#define EFF_CALLE         0x0080
#define EFF_SINGLELINE    0x0100
#define EFF_EXIT          0x0200
#define EFF_ARRAY         0x0400

#define EFF_STUPIDEFF     0x8000


#define UCC_CMPGT 0x16
#define UCC_CMPLT 0x17
#define UCC_CMPGE 0x18
#define UCC_CMPLE 0x19
#define UCC_CMPNE 0x1A
#define UCC_CMPEQ 0x22

#define UCC_PUSHS        0x1D
#define UCC_PUSHI        0x1F
#define UCC_PUSH         0x21
#define UCC_PUSH_ITEMREF 0x3E
#define UCC_PUSH_EVENTID 0x48
#define UCC_PUSH_TRUE    0x13
#define UCC_PUSH_FALSE   0x14
#define UCC_PUSHF        0x42
#define UCC_PUSHBI       0x44

#define UCC_JNE          0x05
#define UCC_JMP          0x06

#define UCC_ADD          0x09
#define UCC_SUB          0x0A
#define UCC_DIV          0x0B
#define UCC_MUL          0x0C
#define UCC_MOD          0x0D
#define UCC_AND          0x0E
#define UCC_OR           0x0F

#define UCC_NOT          0X10

#define UCC_CALLIS       0x38
#define UCC_CALLI        0x39

#define UCC_CALL         0x24

#define UCC_RET          0x25
#define UCC_RETR         0x32

#define UCC_POP          0x12
#define UCC_POPR         0x2D
#define UCC_POPF         0x43

#define UCC_EXIT2        0x2c
#define UCC_EXIT         0x3f

#define UCC_ARRC         0x1e
#define UCC_IN           0x30
#define UCC_APUT         0x46
#define UCC_AGET         0x26
#define UCC_ARRA         0x4a

#define UCC_ADDSI        0x1c
#define UCC_SAY          0x33
#define UCC_ADDSV        0x2f

// new funcs
//#define UCC_MATHCAT      0xFFFF
#define UCC_IF           0xFFFE
#define UCC_FUNC         0xFFFD
//#define UCC_ARG          0xFFFC
#define UCC_ARR          0xFFFB
#define UCC_STRING       0xFFFA

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
		
		vector<string> param_types;
		vector<pair<unsigned int, bool> > param_sizes; // .first==size of parameter in bytes .second==whether to treat it as a relative offset and calculate for it
		
		unsigned int   num_pop;
		unsigned int   num_push;
		unsigned int   call_effect;
		bool           flag_return;
		bool           flag_paren;
		bool           flag_indent_inc;
		bool           flag_indent_dec;
		bool           flag_indent_tmpinc;
		bool           flag_indent_tmpdec;
};

extern vector<UCOpcodeData> opcode_table_data;
extern vector<pair<unsigned int, unsigned int> > opcode_jumps;

extern map<unsigned int, string> bg_uc_intrinsics;
extern map<unsigned int, string> si_uc_intrinsics;

void init_static_usecodetables(const Configuration &config);
void init_usecodetables(const Configuration &config, bool noconf, bool verbose);

#endif

