
#ifndef UCDATA_H
#define UCDATA_H

#include <string>
#include <cstdio>
#include <fstream>
#include "ucfunc.h"

class UCc;

enum { MODE_NONE=0, MODE_DISASSEMBLY=1, MODE_ALL=2, MODE_OPCODE_SCAN=3, MODE_OPCODE_SEARCH=4, MODE_INTRINSIC_SEARCH=5, MODE_FLAG_DUMP=6, MODE_DISASSEMBLE_ALL=7 };
enum { GAME_BG=1, GAME_SI=2 };

const unsigned int MAX_OPCODE_BUF = 256;
const unsigned int MAX_INTRINSIC_BUF = 256;

class UCData
{
	public:
		UCData();
		~UCData();
		
		void dump_unknown_opcodes();
		void dump_unknown_intrinsics();
		
		void parse_params(const unsigned int argc, char **argv);
		void open_usecode(const string &filename);
		void load_funcs(const char **func_table);
		void list_funcs(const char **func_table);
		
		void disassamble(const char **func_table);
		void disassamble_all(const char **func_table);
		void dump_flags(const char **func_table);
		
		bool noconf() const { return _noconf; };
		bool verbose() const { return _verbose; };
		
 		unsigned int mode() const { return _mode; };
 		void mode(const unsigned int mode) { _mode=mode; };
		unsigned int game() const { return _game; };
		string output_redirect() const { return _output_redirect; };
		
		bool output_list() const { return _output_list; };
		bool output_asm()  const { return _output_asm;  };
		bool output_ucs()  const { return _output_ucs;  };
		
		bool fail() const { return _file.fail(); };
	
	private:
		
		void file_open(const string &filename);
		void file_seek_start() { _file.seekg(0, ios::beg); };
		void file_seek_end() { _file.seekg(0, ios::end); };

		
		ifstream _file;
		
		bool _noconf;
		bool _verbose;
		
		unsigned int _mode;
		unsigned int _game;
		
		bool _output_list;
		bool _output_asm;
		bool _output_ucs;
		
		string _output_redirect;
		
		unsigned short _funcid;
		
		vector<UCc> _codes;
		
		vector<UCFunc *> _funcs;
		
		vector<unsigned char> _opcode_buf;
		vector<unsigned char> _intrinsic_buf;
		
		long _search_opcode;
		long _search_intrinsic;
		long _search_func;

};

#endif

