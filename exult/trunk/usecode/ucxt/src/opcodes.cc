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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include "opcodes.h"
#include "files/utils.h"
#include "exceptions.h"
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <stack>
#include <sstream>

using std::stringstream;

/*** head2data
#ifndef __STRING
	#if defined __STDC__ && __STDC__
		#define __STRING(x) #x
	#else
		#define __STRING(x) "x"
	#endif
#endif
*/

using std::vector;
using std::ifstream;
using std::cout;
using std::endl;
using std::string;
using std::cerr;
using std::ends;
using std::setw;
using std::setfill;
using std::setbase;
using std::pair;
using std::map;

#define MAX_NO_OPCODES 512
vector<UCOpcodeData> opcode_table_data(MAX_NO_OPCODES);

vector<pair<unsigned int, unsigned int> > opcode_jumps;

map<unsigned int, string> uc_intrinsics;

map<string, pair<unsigned int, bool> > type_size_map;

void ucxtInit::init(const Configuration &config, const UCOptions &options)
{
	datadir = get_datadir(config, options);
	
	misc_data = "u7misc.data";
	misc_root = "misc";
	
	opcodes_data = "u7opcodes.data";
	opcodes_root = "opcodes";
	
	bg_intrinsics_data = "u7bgintrinsics.data";
	bg_intrinsics_root = "intrinsics";
	
	si_intrinsics_data = "u7siintrinsics.data";
	si_intrinsics_root = "intrinsics";
	
	if(options.verbose) cout << "Initing misc..." << endl;
	misc();
	
	if(options.verbose) cout << "Initing opcodes..." << endl;
	opcodes();
	
	if(options.verbose) cout << "Initing intrinsics..." << endl;
	if(options.game_bg())
		intrinsics(bg_intrinsics_data, bg_intrinsics_root);
	else if(options.game_si())
		intrinsics(si_intrinsics_data, si_intrinsics_root);
}

string ucxtInit::get_datadir(const Configuration &config, const UCOptions &options)
{
	string datadir;
	
	// just to handle if people are going to compile with makefile.unix, unsupported, but occasionally useful
	#ifdef HAVE_CONFIG_H
	if(options.noconf == false) config.value("config/ucxt/root", datadir, EXULT_DATADIR);
	#else
	if(options.noconf == false) config.value("config/ucxt/root", datadir, "data/");
	#endif
	
	if(datadir.size() && datadir[datadir.size()-1]!='/' && datadir[datadir.size()-1]!='\\') datadir+='/';
	if(options.verbose) cout << "datadir: " << datadir << endl;

	return datadir;
}

void ucxtInit::misc()
{
	Configuration miscdata(datadir + misc_data, misc_root);

	Configuration::KeyTypeList om;
	miscdata.getsubkeys(om, misc_root + "/offset_munge");
	
	Configuration::KeyTypeList st;
	miscdata.getsubkeys(st, misc_root + "/size_type");
	
	// For each size type (small/long/byte/etc.)
	for(typeof(st.begin()) k=st.begin(); k!=st.end(); ++k)
	{
		bool munge_offset=false;
		
		const string tmpstr(k->first + "/");
		
		/* ... we need to find out if we should munge it's parameter
			that is, it's some sort of goto target (like offset) or such */
		for(typeof(om.begin()) m=om.begin(); m!=om.end(); ++m)
			if(m->first.size()-1==k->first.size())
				if(m->first==tmpstr)
//				if(m->first.compare(0, m->first.size()-1, k->first, 0, k->first.size())==0)
					munge_offset=true;
		
		// once we've got it, add it to the map
		pair<unsigned int, bool> tsm_tmp(strtol(k->second.c_str(), 0, 0), munge_offset);
		type_size_map.insert(pair<string, pair<unsigned int, bool> >(k->first, tsm_tmp));
	}
}

/* constructs the usecode tables from datafiles in the /ucxt hierachy */
void ucxtInit::opcodes()
{
	Configuration opdata(datadir + opcodes_data, opcodes_root);
	
	vector<string> keys = opdata.listkeys(opcodes_root);
		
	#if 1
	for(vector<string>::iterator key=keys.begin(); key!=keys.end(); ++key)
	{
		if((*key)[0]!='!')
		{
			Configuration::KeyTypeList ktl;
		
			opdata.getsubkeys(ktl, *key);
		
			if(ktl.size())
			{
				unsigned int i = strtol(key->substr(key->find_first_of("0")).c_str(), 0, 0);
				opcode_table_data[i] = UCOpcodeData(i, ktl);
			}
		}
	}
	
	#else
	
	string ucxtroot(datadir + "opcodes.txt");

	std::ifstream file;

	try
	{
		U7open(file, ucxtroot.c_str(), true);
	} catch (const file_open_exception& e)
	{
		cerr << e.what() << ". exiting." << endl;
		exit(1);
	}
	
	std::string s;
	while(!file.eof())
	{
		getline(file, s);
		if(s.size() && s[0]=='>')
		{
			UCOpcodeData uco(str2vec(s));
			assert(uco.opcode<MAX_NO_OPCODES);
			opcode_table_data[uco.opcode] = uco;
		}	
	}
	file.close();
	#endif
	
	/* Create an {opcode, parameter_index} array of all opcodes that
		execute a 'jump' statement */
	for(std::vector<UCOpcodeData>::iterator op=opcode_table_data.begin(); op!=opcode_table_data.end(); op++)
	{
		for(unsigned int i=0; i<op->param_sizes.size(); i++)
		{
			if(op->param_sizes[i].second==true) // this is a calculated offset
			{
				opcode_jumps.push_back(std::pair<unsigned int, unsigned int>(op->opcode, i+1)); // parameters are stored as base 1
			}
		}
	}
	
	#if 0
	std::cout << "Calculated Opcode pairs:" << std::endl;
	for(std::vector<std::pair<unsigned int, unsigned int> >::iterator i=opcode_jumps.begin(); i!=opcode_jumps.end(); i++)
		std::cout << setw(4) << i->first << '\t' << setw(4) << i->second << std::endl;
	#endif
}

void ucxtInit::intrinsics(const string &intrinsic_data, const string &intrinsic_root)
{
	Configuration intdata(datadir + intrinsic_data, intrinsic_root);
	
	Configuration::KeyTypeList ktl;
		
	intdata.getsubkeys(ktl, intrinsic_root);
	
	for(Configuration::KeyTypeList::iterator k=ktl.begin(); k!=ktl.end(); k++)
		uc_intrinsics.insert(pair<unsigned int, string>(strtol(k->first.c_str(), 0, 0), k->second));
}

/* To be depricated when I get the complex std::vector<std::string> splitter online */
std::vector<std::string> qnd_ocsplit(const std::string &s)
{
	assert((s[0]=='{') && (s[s.size()-1]=='}'));

	std::vector<std::string> vs;
	std::string tstr;

	for(std::string::const_iterator i=s.begin(); i!=s.end(); ++i)
	{
		if(*i==',')
		{
			vs.push_back(tstr);
			tstr="";
		}
		else if(*i=='{' || *i=='}')
		{ /* nothing */ }
		else
			tstr+=*i;
	}
	if(tstr.size())
		vs.push_back(tstr);

	return vs;
}

std::vector<std::string> str2vec(const std::string &s)
{
	std::vector<std::string> vs;
	unsigned int lasti=0;

	// if it's empty return null
	if(s.size()==0) return vs;

	bool indquote=false;
	for(unsigned int i=0; i<s.size(); i++)
	{
		if(s[i]=='"')
			indquote = !indquote;
		else if(isspace(s[i]) && (!indquote))
		{
			if(lasti!=i)
			{
				if((s[lasti]=='"') && (s[i-1]=='"'))
				{
					if((lasti+1)!=(lasti-1))
						vs.push_back(s.substr(lasti+1, i-lasti-2));
				}
				else
					vs.push_back(s.substr(lasti, i-lasti));
			}

			lasti=i+1;
		}
		if(i==s.size()-1)
		{
			if((s[lasti]=='"') && (s[i]=='"'))
			{
				if((lasti+1)!=(lasti-1))
					vs.push_back(s.substr(lasti+1, i-lasti-2));
			}
			else
				vs.push_back(s.substr(lasti, i-lasti+1));
		}
	}

	#if 0 //test
	for(unsigned int i=0; i<vs.size(); i++)
		std::cout << "\t\"" << vs[i] << "\"" << std::endl;
	#endif ///test

	return vs;
}

void map_type_size(const std::vector<std::string> &param_types, std::vector<std::pair<unsigned int, bool> > &param_sizes)
{
	for(std::vector<std::string>::const_iterator s=param_types.begin(); s!=param_types.end(); ++s)
	{
		map<string, pair<unsigned int, bool> >::iterator tsm(type_size_map.find(*s));
		if(tsm==type_size_map.end())
		{
			cerr << "error: No size type `" << *s << "`" << endl;
			assert(tsm!=type_size_map.end());
		}
		
		param_sizes.push_back(std::pair<unsigned int, bool>(tsm->second.first, tsm->second.second));
	}
}

/*std::vector<std::string> str2vec(const std::string &s)
{
	std::vector<std::string> vs; // the resulting strings
	stack<char> vbound; // the "bounding" chars used to deonte collections of characters
	unsigned int lasti=0;
  std::string currstr; // the current string, gets appended to vs

	// if it's empty return null
	if(s.size()==0) return vs;

	for(unsigned int i=0; i<s.size(); i++)
	{
		bool pushback=false; // do we push the currstr onto the vector now?
		char c = s[i];
		switch(c)
		{*/
			// let's start with the openings...
			/* the general pricipal, since we strip the outermost enclosures,
			   is to only append the "bounding" characters if they're NOT the
			   outer most.
			   NOTE: A subtle exception is the boundaries on the outermost set of
			   bounding chars has the same effect as isspace(), YHBW */
/*			case '{':  if(vs.size()) currstr+=c; vbound.push('}');  break;
			//case '[': if(vs.size()) currstr+=c; vbound.push(']'); break;
			//case '(': if(vs.size()) currstr+=c; vbound.push(')'); break;
			//case '<': if(vs.size()) currstr+=c; vbound.push('>'); break;

			// now the closures...
			case '}':
				if(vbound.top()=='}') vbound.pop();
				if(vbound.size()==0)  pushback=true;
				else                  currstr+=c;
				break;
			//case ']':
			//	break;
			//case ')':
			//	break;
			//case '>':
			//	break;

			// now the ones that have the pretentiousness of being both
			// opening and closing causes
			case '\"': if(vs.size()) currstr+=c; vbound.push('\"'); break;
			case '\'': if(vs.size()) currstr+=c; vbound.push('\''); break;
			case '\"':
				if(vbound.top()=='\"')    vbound.pop();
				else                   vbound.push('\"');
				if(vbound.size()==0) pushback=true;
				else                   currstr+=c;
				break;
			case '\'':
				if(vbound.top()=='\'') vbound.pop();
				if(vbound.size()==0)   pushback=true;
				else                   currstr+=c;
				break;
			
			// not to emulate isspace();
			case ' ':  // ze space
			case '\f': // form-feed
			case '\n': // newline
			case '\r': // carriage return
			case '\t': // horizontal tab
			case '\v': // vertical tab
				pushback=true;
				break;
		}

		if(pushback)
		{
			if(currstr.size())
				vs.push_back(currstr);
			currstr="";
		}
	}

	#if 1 //test
	for(unsigned int i=0; i<vs.size(); i++)
		std::cout << "\t\"" << vs[i] << "\"" << std::endl;
	#endif ///test

	return vs;
}*/
