/*
	ucxt: Ultima 7 usecode dump/disassembly/convert-to-something-more-readable utility
		Based heavily on, and including code from ucdump created and maintained by:
			Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
	
	Maintainter:
		Patrick Burke (takhisisii@yahoo.com.au)
	
	Original ucdump history, credits and stuff moved to Docs/ucxtread.txt
	
	$LBClueless = TRUE;
*/

/* TODO:
	* enum...next needs to be 'finished'
*/

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
#include <map>

#include "ucc.h"
#include "opcodes.h"
#include "printucc.h"
#include "ucdata.h"
#include "ucfunc.h"
#include "files/utils.h"

// include xml configuration stuff
#include "Configuration.h"
#include "exult_constants.h"
const std::string c_empty_string; // Ob for exult_constants.h

/* Functions */
void usage();
void output_flags(const vector<UCFunc *> &funcs);
void open_usecode_file(UCData &uc, const Configuration &config);

UCData uc;

int main(int argc, char** argv)
{
	cout << "Ultima 7 usecode disassembler v0.6.2" << endl
	#ifdef HAVE_CONFIG_H
	     << "    compiled with " << PACKAGE << " " << VERSION << endl
	#endif
	     << endl;
	
	// Tends to make life easier
	cout << setfill('0') << setbase(16);
	cout.setf(ios::uppercase);

	// get the parameters
	uc.parse_params(argc, argv);
	if(uc.verbose()) cout << "Parameters parsed..." << endl;

	Configuration config;
	
	// attempt to find an exult.cfg file... _somewhere_
	if(uc.noconf() == false)
	{
		if(uc.verbose()) cout << "Loading exult configuration file..." << endl;
		if(config.read_config_file("exult.cfg") == false)
		{
			cout << "Failed to locate exult.cfg. Run exult before running ucxt or use the -nc switch. Exiting." << endl;
			exit(1);
		}
	}
	
	// init the compile time tables
	if(uc.verbose()) cout << "Initing static tables..." << endl;
	init_static_usecodetables(config);

	// init the run time tables
	if(uc.verbose()) cout << "Initing runtime tables..." << endl;
	init_usecodetables(config, uc.noconf(), uc.verbose());
	
	// ICK! Don't try this at home kids...
	// done because for some reason it started crashing upon piping or redirection to file... wierd.
	// yes, it's a hack to fix an eldritch bug I can't find... it seems appropriate
	ofstream outputstream;
	streambuf *coutbuf=0;
	if(uc.output_redirect().size())
	{
		U7open(outputstream, uc.output_redirect().c_str(), false);
		if(outputstream.fail())
		{
			cout << "error. failed to open " << uc.output_redirect() << " for writing. exiting." << endl;
			exit(1);
		}
		coutbuf = cout.rdbuf();
		cout.rdbuf(outputstream.rdbuf());
	}
	// you may now uncover your eyes <grin>

	open_usecode_file(uc, config);

	if     ( uc.mode_dis() || uc.mode_all() )
	{
		uc.disassamble();
	}
	else if( uc.mode() == MODE_FLAG_DUMP )
	{
		uc.dump_flags();
	}
	else
		usage();

	// now we clean up the <ick>y ness from before
	if(uc.output_redirect().size())
	{
		cout.rdbuf(coutbuf);
	}
	
	return 0;
}

void open_usecode_file(UCData &uc, const Configuration &config)
{
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
	
	/* The four mystical usecode configurations: */
	
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
	if(uc.input_usecode_file().size())
		uc.open_usecode(uc.input_usecode_file());
	else if(uc.noconf()==false)
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
		cout << "Failed to locate usecode file. Exiting." << endl;
		exit(1);
	}
}


void usage()
{
	cout << "Usage:" << endl
	     << "\tucxt [options] -a" << endl
	     << "\t\t- prints all of the functions" << endl
	     << "\tucxt [options] <hex function number>" << endl
	     << "\t\t- disassembles single function to stdout" << endl
//       << "\tucdump -c - scans the whole usecode file for unknown opcodes" << endl
//       << "\tucdump -o <hex number> - prints list of functions which use "
//       << "the given opcode" << endl
//       << "\tucdump -i <hex number> - prints list of functions which use "
//       << "the given intrinsic function\n" << endl
//       << "\tucxt -f - prints list of all flags x functions" << endl
	     << endl
	     << "\tMisc Flags (any/all of these):" << endl
	     << "\t\t-nc\t- don't look for exult's .xml config file" << endl
	     << "\t\t-v \t- turns on verbose output mode" << endl
	     << "\t\t-ofile\t- output to the specified file" << endl
	     << "\t\t-ifile\t- load the usecode file specified by the filename" << endl
	     << "\t\t-ro\t- output the raw opcodes in addition to the -f format" << endl
	     << "\t\t-ac\t- output an automatically generated comment" << endl
	     << "\t\t-uc\t- output an automatically generated useless comment" << endl
	     << "\t\t\t  in addition to the -f format" << endl
	     << "\t\t-dbg\t- output debugging information if present in USECODE." << endl
	     << "\tGame Specifier Flags (only one of these):" << endl
	     << "\t\t-bg\t- select the black gate usecode file" << endl
	     << "\t\t-si\t- select the serpent isle usecode file" << endl
	     << "\tOutput Format Flags (only one of these):" << endl
	     << "\t\t-fl\t- output using brief \"list\" format" << endl
	     << "\t\t-fa\t- output using \"assembler\" format (default)" << endl
	     << "\t\t-fs\t- output using \"old exult script\" format" << endl
	     << "\t\t-fz\t- output using \"exult script\" format" << endl
	     ;
  exit(1);
}


























