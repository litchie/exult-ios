
#ifndef UCDATA_H
#define UCDATA_H

#include <string>
#include <cstdio>
#include <fstream>

#include "ucfunc.h"

class UCOptions
{
	public:
		UCOptions() : output_extern_header(false)
		{};
		
		bool output_extern_header;
};

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
		
		void disassamble();
		void dump_flags(ostream &o);
		void output_extern_header(ostream &o);
		
		string output_redirect() const { return _output_redirect; };
		string input_usecode_file() const { return _input_usecode_file; };
		
		bool noconf()       const { return _noconf;      };
		bool rawops()       const { return _rawops;      };
		bool autocomment()  const { return _autocomment; };
		bool uselesscomment() const { return _uselesscomment; };
		bool verbose()      const { return _verbose;     };
		bool ucdebug()      const { return _ucdebug;     };
		bool basic()        const { return _basic;       };
		
		bool game_bg()      const { return _game==GAME_BG; };
		bool game_si()      const { return _game==GAME_SI; };
		bool game_u8()      const { return _game==GAME_SI; };
		
		bool output_list()  const { return _output_list; };
		bool output_asm()   const { return _output_asm;  };
		bool output_ucz()   const { return _output_ucz;  };
		bool output_flag()  const { return _output_flag; };
		
		bool mode_all()     const { return _mode_all;    };
		bool mode_dis()     const { return _mode_dis;    };
		
		bool fail()         const { return _file.fail(); };
	
		const map<unsigned short, UCFuncSet> &funcmap() { return _funcmap; };	
		
		const UCOptions &opt() { return options; };
		
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
		bool _basic;
		
		unsigned int _game;
		
		enum { GAME_BG=1, GAME_SI=2, GAME_U8=3 };
		
		bool _output_list;
		bool _output_asm;
		bool _output_ucz;
		bool _output_flag;
		
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

		UCOptions options;
};

#endif

