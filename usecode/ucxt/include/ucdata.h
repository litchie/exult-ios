
#ifndef UCDATA_H
#define UCDATA_H

#include <string>
#include <cstdio>
#include <fstream>

class UCData;

#include "ucfunc.h"

class UCc;

enum { MODE_NONE=0, MODE_DISASSEMBLY=1, MODE_ALL=2, MODE_OPCODE_SCAN=3, MODE_OPCODE_SEARCH=4, MODE_INTRINSIC_SEARCH=5, MODE_FLAG_DUMP=6, MODE_DISASSEMBLE_ALL=7 };
enum { GAME_BG=1, GAME_SI=2 };

class UCData
{
	public:
		UCData();
		~UCData();
		
		void dump_unknown_opcodes();
		void dump_unknown_intrinsics();
		
		void parse_params(const unsigned int argc, char **argv);
		void open_usecode(const string &filename);
		void load_funcs();
		void list_funcs();
		
		void disassamble();
		void disassamble_all();
		void dump_flags();
		
 		void mode(const unsigned int mode) { _mode=mode; };
		string output_redirect() const { return _output_redirect; };
		string input_usecode_file() const { return _input_usecode_file; };
		
		bool noconf()      const { return _noconf;      };
		bool rawops()      const { return _rawops;      };
		bool autocomment() const { return _autocomment; };
		bool uselesscomment() const { return _uselesscomment; };
		bool verbose()     const { return _verbose;     };
		bool ucdebug()     const { return _ucdebug;     };
		
 		unsigned int mode() const { return _mode; };
		unsigned int game() const { return _game; };
		
		bool output_list() const { return _output_list; };
		bool output_asm()  const { return _output_asm;  };
		bool output_ucs()  const { return _output_ucs;  };
		bool output_ucz()  const { return _output_ucz;  };
		
		bool mode_all()    const { return _mode_all;    };
		bool mode_dis()    const { return _mode_dis;    };
		
		bool fail() const { return _file.fail(); };
	
		const map<unsigned short, UCFuncSet> &funcmap() { return _funcmap; };	
		
	private:
		
		void file_open(const string &filename);
		void file_seek_start() { _file.seekg(0, ios::beg); };
		void file_seek_end() { _file.seekg(0, ios::end); };

		
		ifstream _file;
		
		bool _noconf;
		bool _rawops;
		bool _autocomment;
		bool _uselesscomment;
		bool _verbose;
		bool _ucdebug;
		
		unsigned int _mode;
		unsigned int _game;
		
		bool _output_list;
		bool _output_asm;
		bool _output_ucs;
		bool _output_ucz;
		
		bool _mode_all;
		bool _mode_dis;
		
		string _output_redirect;
		string _input_usecode_file;
		
		unsigned short _funcid;
		
		vector<UCc> _codes;
		
		vector<UCFunc *> _funcs;
		
		/* Just a quick mapping between funcs and basic data on them.
		   Just something we can quickly pass to the parsing functions
		   so we don't have to give them an entire function to play with. */
		FuncMap _funcmap;
		
		long _search_opcode;
		long _search_intrinsic;
		long _search_func;

};

#endif

