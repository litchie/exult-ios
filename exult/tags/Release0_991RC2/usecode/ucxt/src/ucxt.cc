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

/*
	ucxt: Ultima 7 usecode dump/disassembly/convert-to-something-more-readable utility
		Based heavily on ucdump created and maintained by:
			Maxim S. Shatskih aka Moscow Dragon (maxim__s@mtu-net.ru)
	
	Maintainter:
		Patrick Burke (takhisisii@yahoo.com.au)
	
	Original ucdump history, credits and stuff moved to Docs/ucxtread.txt
	
	$LBClueless = TRUE;
*/

/* TODO:
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iomanip>
#include <vector>
#include <stack>
#include <map>

#include "ucc.h"
#include "opcodes.h"
#include "ucdata.h"
#include "ucfunc.h"
#include "files/utils.h"

// include xml configuration stuff
#include "Configuration.h"
#include "exult_constants.h"
const std::string c_empty_string; // Ob for exult_constants.h

/* Functions */
void usage();
void open_usecode_file(UCData &uc, const Configuration &config);

UCData uc;

using std::setw;
using std::cerr;
using std::cout;
using std::ios;
using std::string;
using std::endl;

int main(int argc, char** argv)
{
	// Tends to make life easier
	cout << std::setfill('0') << std::setbase(16);
	cout.setf(ios::uppercase);

	// get the parameters
	uc.parse_params(argc, argv);
	if(uc.options.verbose) cout << "Parameters parsed..." << endl;

	Configuration config;
	
	// attempt to find an exult.cfg file... _somewhere_
	if(uc.options.noconf == false)
	{
		if(uc.options.verbose) cout << "Loading exult configuration file..." << endl;
		if(config.read_config_file("exult.cfg") == false)
		{
			cout << "Failed to locate exult.cfg. Run exult before running ucxt or use the -nc switch. Exiting." << endl;
			exit(1);
		}
	}
	
	// init the run time tables
	if(uc.options.verbose) cout << "Initing runtime tables..." << endl;
	
	ucxtInit init;
	init.init(config, uc.options);
	
	#if 0
	{
		cout << "<opcodes>" << endl;
		
		for(typeof(opcode_table_data.begin()) i=opcode_table_data.begin(); i!=opcode_table_data.end(); i++)
		{
			if(i->opcode!=0 && i->name!="NULL")
			{
				cout << "\t<0x" << setw(2) << i->opcode << '>' << endl;
			
				cout << "\t\t<name> " << i->name << " </>" << endl;
				cout << "\t\t<asm_nmo> `" << i->asm_nmo << "` </>" << endl;
				cout << "\t\t<asm_comment> `" << i->asm_comment << "` </>" << endl;
				cout << "\t\t<ucs_nmo> `" << i->ucs_nmo << "` </>" << endl;
				cout << "\t\t<num_bytes> " << i->num_bytes << " </>" << endl;
				
				cout << "\t\t<param_types> {";
				for(typeof(i->param_types.begin()) j=i->param_types.begin(); j!=i->param_types.end(); j++)
					cout << *j << ',';
				cout << "} </>" << endl;
				
				cout << "\t\t<num_pop> " << i->num_pop << " </>" << endl;
				cout << "\t\t<num_push> " << i->num_push << " </>" << endl;
				cout << "\t\t<call_effect> " << i->call_effect << " </>" << endl;
				if(i->flag_return) cout << "\t\t<return/>" << endl;
				if(i->flag_paren) cout << "\t\t<paren/>" << endl;
				if(i->flag_indent_inc) cout << "\t\t<indent_inc/>" << endl;
				if(i->flag_indent_dec) cout << "\t\t<indent_dec/>" << endl;
				if(i->flag_indent_tmpinc) cout << "\t\t<indent_tmpinc/>" << endl;
				if(i->flag_indent_tmpdec) cout << "\t\t<indent_tmpdec/>" << endl;
				
				cout << "\t</>" << endl;
			}
		}
		cout << "</>" << endl;
	}
	#endif
	
	// ICK! Don't try this at home kids...
	// done because for some reason it started crashing upon piping or redirection to file... wierd.
	// yes, it's a hack to fix an eldritch bug I could't find... it seems appropriate
	// FIXME: Problem nolonger exists. Probably should put some 'nice' code in it's place.
	std::ofstream outputstream;
	std::streambuf *coutbuf=0;
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

	if(uc.opt().output_extern_header)
	{
		uc.output_extern_header(cout);
	}
	else if     ( uc.options.mode_dis || uc.options.mode_all )
	{
		uc.disassamble();
	}
	else if( uc.options.output_flag )
	{
		uc.dump_flags(cout);
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
	if(uc.options.noconf == false) config.value("config/disk/game/blackgate/path", bgpath);
	string sipath;
	if(uc.options.noconf == false) config.value("config/disk/game/serpentisle/path", sipath);
	string u8path;
	if(uc.options.noconf == false) config.value("config/disk/game/pagan/path", u8path);
	
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
	
	// These 4 are only specific to BG && SI
	string mucc_sl("static");
	string mucc_sc("STATIC");
	string mucc_ul("usecode");
	string mucc_uc("USECODE");
	
	const string mucc_bgl("ultima7");
	const string mucc_bgc("ULTIMA7");
	const string mucc_sil("serpent");
	const string mucc_sic("SERPENT");
	const string mucc_u8l("pagan");
	const string mucc_u8c("PAGAN");
	
	string path, ucspecial, mucc_l, mucc_c;
	if(uc.options.game_bg())
	{
		if(uc.options.verbose) cout << "Configuring for bg." << endl;
		path      = bgpath;
		ucspecial = "usecode.bg";
		mucc_l  = mucc_bgl;
		mucc_c  = mucc_bgc;
	}
	else if(uc.options.game_si())
	{
		if(uc.options.verbose) cout << "Configuring for si." << endl;
		path      = sipath;
		ucspecial = "usecode.si";
		mucc_l  = mucc_sil;
		mucc_c  = mucc_sic;
	}
	else if(uc.options.game_u8())
	{
		if(uc.options.verbose) cout << "Configuring for u8." << endl;
		path      = u8path;
		ucspecial = "usecode.u8";
		mucc_l  = mucc_u8l;
		mucc_c  = mucc_u8c;
		mucc_sl = "usecode";
		mucc_sc = "USECODE";
		mucc_ul = "eusecode.flx";
		mucc_uc = "EUSECODE.FLX";
	}
	else
	{
		cerr << "Error: uc.game() was not set to GAME_U7 or GAME_SI or GAME_U8 this can't happen" << endl;
		assert(false); exit(1); // just incase someone decides to compile without asserts;
	}
	
	/* The four mystical usecode configurations: */
	
	const string mucc_ll(string("/") + mucc_sl + "/" + mucc_ul);
	const string mucc_cl(string("/") + mucc_sc + "/" + mucc_ul);
	const string mucc_lc(string("/") + mucc_sl + "/" + mucc_uc);
	const string mucc_cc(string("/") + mucc_sc + "/" + mucc_uc);
	
	// an icky exception chain for those who don't use .exult.cfg
	if(uc.input_usecode_file().size())
		uc.open_usecode(uc.input_usecode_file());
	else if(uc.options.noconf==false)
	{
		uc.open_usecode(path + mucc_ll);
		if(uc.fail())
			uc.open_usecode(path + mucc_cl);
		if(uc.fail())
			uc.open_usecode(path + mucc_lc);
		if(uc.fail())
			uc.open_usecode(path + mucc_cc);
		if(uc.fail())
			uc.open_usecode(mucc_l + mucc_ll);
	}
	else
		uc.open_usecode(mucc_l + mucc_ll);
		
	if(uc.fail())
		uc.open_usecode(mucc_l + mucc_cl);
	if(uc.fail())
		uc.open_usecode(mucc_l + mucc_lc);
	if(uc.fail())
		uc.open_usecode(mucc_l + mucc_cc);
	if(uc.fail())
		uc.open_usecode(mucc_c + mucc_ll);
	if(uc.fail())
		uc.open_usecode(mucc_c + mucc_cl);
	if(uc.fail())
		uc.open_usecode(mucc_c + mucc_lc);
	if(uc.fail())
		uc.open_usecode(mucc_c + mucc_cc);
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
	cout << "Ultima 7/8 usecode disassembler v0.6.3" << endl
	#ifdef HAVE_CONFIG_H
	     << "    compiled with " << PACKAGE << " " << VERSION << endl
	#endif
	     << endl;
	
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
	     << "\t\t-ac\t- output automatically generated comments" << endl
	     << "\t\t-uc\t- output automatically generated 'useless' comments" << endl
	     << "\t\t-b\t- only do 'basic' optimisations" << endl
	     << "\t\t-dbg\t- output debugging information if present in USECODE." << endl
	     << "\t\t-ext32\t- 'convert' function to ext32 format if not already." << endl
	     << "\tGame Specifier Flags (only one of these):" << endl
	     << "\t\t-bg\t- select the black gate usecode file" << endl
	     << "\t\t-si\t- select the serpent isle usecode file" << endl
	     << "\t\t-u8\t- select the ultima 8/pagan usecode file (experimental)" << endl
	     << "\tOutput Format Flags (only one of these):" << endl
	     << "\t\t-fl\t- output using brief \"list\" format" << endl
	     << "\t\t-fa\t- output using \"assembler\" format (default)" << endl
	     << "\t\t-fs\t- output using \"exult script\" format" << endl
	     << "\t\t-fz\t- output using \"exult script\" format" << endl
	     << "\t\t-ftt\t- output using the translation table xml format" << endl
	     << "\t\t-ff\t- outputs all flags referenced in the usecode file" << endl
	     << "\t\t\t  sorted both by \"flags within a function\" and" << endl
	     << "\t\t\t  \"functions using flag\"" << endl
	     ;
  exit(1);
}


























