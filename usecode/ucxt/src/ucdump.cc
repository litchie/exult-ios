/*
	ucxt: Ultima 7 usecode dump/disassembly/convert-to-something-more-readable utility
		Based heavily on, and including code from ucdump created and maintained by:
			Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
	
	Maintainter:
		Patrick Burke (takhisisii@yahoo.com.au)
	
	Original ucdump history, credits and stuff follows:
	
	----------------------------------------------------------------------------------
	
 Ultima 7 usecode dump/disassembly utility
 Distributed under GPL

 Maintainer:
  Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
 
 History:
 - originally written June 99 by Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
 Thanks to Keldon Jones (keldon@umr.edu)
  and Wouter Dijklslag aka Wody Dragon (wody@wody.demon.nl) for their help
 11-Oct-99
  - added "function search by opcode & intrinsic" feature
  - added "unknown opcode & intrinsic counting" feature
 12-Oct-99
  - the good deal of intrinsic functions is now known
 
 The source must be buildable on any Win32 compiler - patch fopen()'s "wb"
  modes for UNIX. Help in porting on non-Win32 platforms is greatly appreciated.
 See source file comments for the description of usecode opcodes & intrinsic functions
  (the latter one differs between BG & SI)
 
 Some general usecode ideas:
 - usecode functions 0x0-0x3ff are shape handlers - called on double-clicks & other
  event with appropriate shapes
 - usecode functions 0x401-0x4ff are NPC handlers - called on double-clicks & other
  event with appropriate NPCs - NPCID + 0x400 (401 for Iolo, 417 for LB etc).
 - usecode functions 0x500-0x5ff is for Wisps & guards (nonNPC characters able to talk)
  (these ranges seems to be hardcoded)
 - stack machine used to execute bytecodes
 - the machine's state is:
  stack
  local variables(forgotten on function exit, first N of them are call arguments -
          first pushed is 0, next are 1, 2...)
  game flags
  ItemRef (???seems to be valid only for top-level functions-event handlers
        or maybe is persistent till quitting usecode executuion???)
  EventID (???seems to be valid only for top-level functions-event handlers
        or maybe is persistent till quitting usecode executuion???)
 - game flags are bytes treated as booleans (0/1), persistent across engine shutdown/restart
  and stored as a simple array (??? 0 or 1 based. Don't remember. Flag 3 means
  - Tetrahedron is down, flag 4 means - Sphere is down) in GAMEDAT\FLAGINIT.
 - usecode can also manipulate items & NPCs by means of intrinsic functions
 - "add" opcode can sum strings (concatenation). Also it can add integer to string
 - any array operations can be peformed on scalar values. In fact, each scalar value is
  treated by the array operations as an array with a single element. Vice versa is also
  true - for instance, ItemsNearItem() function returns an array. Sometimes it is used in
  enum/next loop as an array, but sometimes it is used as an itemref.
 - array indices are 1-based as in VB
 - itemref is a unique ID of the given item. For NPCs, itemref is (-NPCID). For other items,
  itemrefs seems to be non-persistent (not saved to savegame & re-invented on each
  engine startup)??? indexes into engine's item lists
 - there is a value called "referent" which identifies item & stored in U7IBUF
    Maybe Itemref is the same thing?
 - -356 is always an Itemref for Avatar. So, Avatar's NPC ID is possibly 356.
 - usecode execution starts from the event handler function. It is called by the engine
  without arguments (double-click) in some cases. ItemRef & EventID are set before entering usecode.
 - the easiest case is double-click on some item. In this case, a usecode event handler
  function called. Function ID usually matches the shape's Type number
  or is (NPCID + 0x400) for NPCs.
  ItemRef is set to the item double-clicked, EventID is set to 1 (double-click).
 - other causes for the engine to call usecode function:
  - events scheduled by intrinsic functions 1 & 2 have EventID 2
  - item is put on another item - the underlying item's handler is called with EventID 3
    (Penumbra's plaque)
  - usecode Egg - also calls a function with EventID 3
  - use item as a weapon (flammable oil) - EventID 4
  - NPC being beaten to death (examples: Hook in BG, Dracothaxus in FoV,
          Pomdirgun & Rotoluncia in SI) - 7 in SI???
  - Avatar & NPC approaching to some distance??? - 9 in SI
 - hex coords to sextant coords - ( x - 933 ) / 10, ( y - 1134 ) / 10
*/
#define HAVE_CONFIG_H

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <strstream>
#include <iomanip>
#include <vector>
#include <stack>
#include "ucc.h"
#include "opcodes.h"
#include "printucc.h"
#include <map>

#include "ucdata.h"
#include "ucfunc.h"

// include xml configuration stuff
#include "Configuration.h"

//#define DEBUG(x) x
#define DEBUG(x)

// PATRICK
/* Functions */
void usage();
void output_flags(const vector<UCFunc *> &funcs);
void load_usecode_file(UCData &uc);

int main(int argc, char** argv)
{
	/* Preset to no match */
	UCData uc;

	cout << "Ultima 7 usecode disassembler v0.6.1" << endl
	     << "    compiled with " << PACKAGE << " " << VERSION << endl
	     << endl;
	
	// Tends to make life easier
	cout << setfill('0') << setbase(16);
	cout.setf(ios::uppercase);

	// init the compile time tables
	init_static_usecodetables();
	// init the run time tables
	init_usecodetables();
	
	uc.parse_params(argc, argv);

	// ICK! Don't try this at home kids...
	// done because for some reason it started crashing upon piping or redirection to file... vierd.
	// yes, it's a hack to fix an eldritch bug I can't find... it seems appropriate
	ofstream outputstream;
	streambuf *coutbuf;
	if(uc.output_redirect().size())
	{
		outputstream.open(uc.output_redirect().c_str(), ios::out);
		if(outputstream.fail())
		{
			cout << "error. failed to open " << uc.output_redirect() << " for writing. exiting." << endl;
			exit(1);
		}
		coutbuf = cout.rdbuf();
		cout.rdbuf(outputstream.rdbuf());
	}
	
	if( uc.mode() == MODE_NONE )
		usage();

	load_usecode_file(uc);
	
	/* Embedded function table pointer
	   TODO: set to bg_func_table or si_func_table depending on the command line
	*/
	const char** func_table = bg_func_table;
//  int func_table_size = sizeof(bg_func_table);


	if     ( uc.mode() == MODE_DISASSEMBLY )
	{
		uc.disassamble(func_table);
	}
	else if( uc.mode() == MODE_DISASSEMBLE_ALL )
	{
		uc.disassamble_all(func_table);
	}
	else if( uc.mode() == MODE_FLAG_DUMP )
	{
		uc.dump_flags(func_table);
	}
	else if( uc.mode() ==MODE_LIST )
	{
		uc.list_funcs(func_table);
	}
//	if( ( ( uc.mode() == MODE_DISASSEMBLY ) || ( uc.mode() == MODE_OPCODE_SEARCH ) ) && !found )

	// now we clean up the <ick>y ness from before
	if(uc.output_redirect().size())
	{
		cout.rdbuf(coutbuf);
	}
	return 0;
}


void usage()
{
  cout << "Usage:" << endl
       << "\tucxt [-v] [-nc] [-bg | -si] [-ofile] -l" << endl
       << "\t\t- prints list of all functions" << endl
//       << "\tucdump -c - scans the whole usecode file for unknown opcodes" << endl
//       << "\tucdump -o <hex number> - prints list of functions which use "
//       << "the given opcode" << endl
//       << "\tucdump -i <hex number> - prints list of functions which use "
//       << "the given intrinsic function\n" << endl
//       << "\tucxt -f - prints list of all flags x functions" << endl
       << "\tucxt [-v] [-nc] [-bg | -si] [-ofile] <hex number>" << endl
       << "\t\t- disassembles single function to stdout" << endl
       << endl
       << "\t-nc\t- don't look for exult's .xml config file" << endl
       << "\t-bg\t- select the black gate usecode file" << endl
       << "\t-si\t- select the serpent isle usecode file" << endl
       << "\t-v \t- turns on verbose output mode" << endl
       << "\t-ofile\t- output to the specified file" << endl;
  exit(1);
}

void load_usecode_file(UCData &uc)
{
	Configuration config;
	
	if(uc.noconf() == false)
	{
		if(uc.verbose()) cout << "Loading exult configuration file..." << endl;
		if(config.read_config_file("exult.cfg") == false)
		{
			cout << "Failed to locate exult.cfg. Run exult before running ucxt or use the -nc switch. Exiting." << endl;
			exit(1);
		}
	}
	
	string bgpath;
	if(uc.noconf() == false) config.value("config/disk/game/blackgate/path", bgpath);
	string sipath;
	if(uc.noconf() == false) config.value("config/disk/game/serpentisle/path", sipath);
	
	/* ok, to find the usecode file we search: (where $PATH=bgpath or sipath)
		$PATH/static/usecode
		$PATH/STATIC/usecode
		$PATH/static/USECODE
		$PATH/STATIC/USECODE
		./ultima7/static/usecode || ./serpent/static/usecode
		./ultima7/STATIC/usecode || ./serpent/STATIC/usecode
		./ultima7/static/USECODE || ./serpent/static/USECODE
		./ultima7/STATIC/USECODE || ./serpent/STATIC/USECODE
		./ULTIMA7/static/usecode || ./SERPENT/static/usecode
		./ULTIMA7/STATIC/usecode || ./SERPENT/STATIC/usecode
		./ULTIMA7/static/USECODE || ./SERPENT/static/USECODE
		./ULTIMA7/STATIC/USECODE || ./SERPENT/STATIC/USECODE
		./static/usecode
		./STATIC/usecode
		./static/USECODE
		./STATIC/USECODE
		./usecode.bg || ./usecode.si
		./USECODE
		./usecode
		
		Anything I'm missing? <queryfluff>
	*/
	
	/* The capitilisation configurations: (yes, going overkill, typos are BAD!) */
	
	string mucc_sl("static");
	string mucc_sc("STATIC");
	string mucc_ul("usecode");
	string mucc_uc("USECODE");
	string mucc_bgl("ultima7");
	string mucc_bgc("ULTIMA7");
	string mucc_sil("serpent");
	string mucc_sic("SERPENT");
	
	/* The four mysitcal usecode configurations: */
	
	string mucc_ll(string("/") + mucc_sl + "/" + mucc_ul);
	string mucc_cl(string("/") + mucc_sc + "/" + mucc_ul);
	string mucc_lc(string("/") + mucc_sl + "/" + mucc_uc);
	string mucc_cc(string("/") + mucc_sc + "/" + mucc_uc);
	
	string path, ucspecial, mucc_u7l, mucc_u7c;
	if(uc.game()==GAME_BG)
	{
		if(uc.verbose()) cout << "Configuring for bg." << endl;
		path      = bgpath;
		ucspecial = "usecode.bg";
		mucc_u7l  = mucc_bgl;
		mucc_u7c  = mucc_bgc;
	}
	else if(uc.game()==GAME_SI)
	{
		if(uc.verbose()) cout << "Configuring for si." << endl;
		path      = sipath;
		ucspecial = "usecode.si";
		mucc_u7l  = mucc_sil;
		mucc_u7c  = mucc_sic;
	}
	else
	{
		cerr << "Error: uc.game() was not set to GAME_U7 or GAME_SI this can't happen" << endl;
		assert(false); exit(1); // just incase someone decides to compile without asserts;
	}
	
	// an icky exception chain for those who don't use .exult.cfg
	if(uc.noconf()==false)
	{
		uc.open_usecode(path + mucc_ll);
		if(uc.fail())
			uc.open_usecode(path + mucc_cl);
		if(uc.fail())
			uc.open_usecode(path + mucc_lc);
		if(uc.fail())
			uc.open_usecode(path + mucc_cc);
		if(uc.fail())
			uc.open_usecode(mucc_u7l + mucc_ll);
	}
	else
		uc.open_usecode(mucc_u7l + mucc_ll);
		
	if(uc.fail())
		uc.open_usecode(mucc_u7l + mucc_cl);
	if(uc.fail())
		uc.open_usecode(mucc_u7l + mucc_lc);
	if(uc.fail())
		uc.open_usecode(mucc_u7l + mucc_cc);
	if(uc.fail())
		uc.open_usecode(mucc_u7c + mucc_ll);
	if(uc.fail())
		uc.open_usecode(mucc_u7c + mucc_cl);
	if(uc.fail())
		uc.open_usecode(mucc_u7c + mucc_lc);
	if(uc.fail())
		uc.open_usecode(mucc_u7c + mucc_cc);
	if(uc.fail())
		uc.open_usecode(mucc_ll);
	if(uc.fail())
		uc.open_usecode(mucc_cl);
	if(uc.fail())
		uc.open_usecode(mucc_lc);
	if(uc.fail())
		uc.open_usecode(mucc_cc);
	if(uc.fail())
		uc.open_usecode(ucspecial);
	if(uc.fail())
		uc.open_usecode(mucc_uc);
	if(uc.fail())
		uc.open_usecode(mucc_ul);
	// if we get through all this, usecode can't be installed anywhere sane
	if(uc.fail())
	{
		cout << "Error. Could not find usecode file. Exiting." << endl;
		exit(1);
	}
	
	if(uc.fail())
	{
		cout << "Failed to locate usecode file. Exiting." << endl;
		exit(1);
	}
}

