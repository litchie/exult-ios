
#ifndef UCDATA_H
#define UCDATA_H

#include <string>
#include <cstdio>
#include "ucc.h"
#include "ucfunc.h"

enum { MODE_NONE=0, MODE_DISASSEMBLY=1, MODE_LIST=2, MODE_OPCODE_SCAN=3, MODE_OPCODE_SEARCH=4, MODE_INTRINSIC_SEARCH=5, MODE_FLAG_DUMP=6, MODE_DISASSEMBLE_ALL=7 };
enum { GAME_BG=1, GAME_SI=2 };

const unsigned int MAX_OPCODE_BUF = 256;
const unsigned int MAX_INTRINSIC_BUF = 256;

class UCData
{
	public:
		UCData();
		~UCData();
		void dump_unknowns();
		void parse_params(const int argc, char **argv);
		void open_usecode(const string &filename);
		void load_funcs(const char **func_table);
		void list_funcs(const char **func_table);

		void disassamble(const char **func_table);
		void disassamble_all(const char **func_table);
		void dump_flags(const char **func_table);

 		unsigned int mode() const { return _mode; };
 		void mode(const unsigned int mode) { _mode=mode; };
		unsigned int game() const { return _game; };
 		unsigned long filesize() const { return _filesize; }

		bool fail() const { return _fail; };
		
		unsigned char _opcode_buf[MAX_OPCODE_BUF];
		unsigned char _intrinsic_buf[MAX_INTRINSIC_BUF];

		unsigned long _search_opcode;
		unsigned long _search_intrinsic;
		unsigned long _search_func;
		FILE *_file;

  private:
  		bool _fail;
		unsigned int _mode;
		unsigned int _game;
		
		unsigned long _filesize;

		unsigned int _funcid;

		vector<UCc> _codes;

		vector<UCFunc *> _funcs;

};

#endif

