/*
 *  Copyright (C) 2000-2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "common_types.h"
#include "Configuration.h"
#include "exceptions.h"
#include "utils.h"

#ifndef ALPHA_LINUX_CXX
#  include <cassert>
#  include <cstdio>
#endif
#include <stdio.h>
#include <iostream>
#include <fstream>
#ifdef HAVE_SSTREAM
#include <sstream>
#endif

#ifndef UNDER_CE
using std::atoi;
using std::cerr;
using std::endl;
using std::string;
using std::ostream;
#endif

// isspace could be a macro
#ifndef UNDER_CE
#ifndef isspace
using std::isspace;
#endif
#endif

#define TRACE_CONF 0

#if TRACE_CONF
	#define CTRACE(X) cerr << X << endl
#else
	#define CTRACE(X)
#endif

void	Configuration::value(const string &key, string &ret,const char *defaultvalue) const
{
	const XMLnode *sub=xmltree->subtree(key);
	if(sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void	Configuration::value(const string &key,bool &ret,bool defaultvalue) const
{
	const XMLnode *sub=xmltree->subtree(key);
	if(sub)
		ret = (to_uppercase(sub->value()) == "YES");
	else
		ret = defaultvalue;
}

void	Configuration::value(const string &key,int &ret,int defaultvalue) const
{
	const XMLnode *sub=xmltree->subtree(key);
	if(sub)
		ret = atoi(sub->value().c_str());
	else
		ret = defaultvalue;
}

void	Configuration::set(const string &key, const string &value, bool write_out)
{
	// Break k up into '/' separated elements.
	// start advancing walk, one element at a time, creating nodes
	// as needed.
	// At the end of that, walk is the target node, and we
	// can set the value.

	// We must also properly encode the value before writing it out.
	// Must remember that.

	xmltree->xmlassign(key,value);
	if(write_out)
		write_back();
}

void	Configuration::set(const char *key, const char *value, bool write_out)
{
	string	k(key), v(value);
	set(k, v, write_out);
}

void	Configuration::set(const char *key, const string &value, bool write_out)
{
	string	k(key);
	set(k, value, write_out);
}

void	Configuration::set(const char *key, int value,bool write_out)
{
	string	k(key),v;
	char	buf[32];

	snprintf(buf,32,"%d",value);
	v=buf;
	set(k,v,write_out);
}


bool	Configuration::read_config_string(const string &s)
{
	string	sbuf(s);
	size_t	nn=0;
	while(isspace(s[nn])) ++nn;
	
	assert(s[nn]=='<');
	++nn;
	
	xmltree->xmlparse(sbuf,nn);
	is_file=false;
	return true;
}

bool	Configuration::read_config_file(const string &input_filename, const string &root)
{
	string fname;

	CTRACE("Configuration::read_config_file");
	
	fname=input_filename;
	// Don't frob the filename if it starts with a dot and
	// a slash.
	// Or if it's not a relative path.
	if((fname.find("./")!=0) && (fname[0]!='/'))
	{
#if ((defined XWIN) || (defined BEOS) || (defined MACOSX))
		const char *f1=getenv("HOME");
		if(f1)
		{
			// User has a home directory
			fname=f1;
#if defined(BEOS)
			fname+="/config/settings/";
#elif defined(MACOSX)
			fname+="/Library/Preferences/";
#else
			fname+="/.";
#endif
			fname+=input_filename;
		}
		else
			fname=input_filename;
#else
		// Probably something to do with determining the username
		// and generating a filename in their personal setup area.

		// For now, just read file from current directory
		fname=input_filename;
#endif
	}

	return read_abs_config_file(fname, root);
}


// read config from file, without pre-processing the filename
bool Configuration::read_abs_config_file(const string &input_filename, const string &root)
{
	filename = input_filename;

	CTRACE("Configuration::read_abs_config_file");

	clear(root);

	is_file=true; // set to file, even if file not found

	std::ifstream ifile;
	try {
	   U7open(ifile, filename.c_str(), true);
	}
	catch(exult_exception &) {
		// configuration file not found
		return false;
	}

	if(ifile.fail())
		return false;

	string	sbuf, line;
	// copies the entire contents of the input file into sbuf
	getline(ifile, line);
	while (ifile.good())
	{
	    sbuf += line + "\n";
	    getline(ifile, line);
	}
	
	ifile.close();
	
	CTRACE("Configuration::read_config_file - file read");
	
	read_config_string(sbuf);
	
	is_file=true;
	return true;
}


string	Configuration::dump(void)
{
	return xmltree->dump();
}

ostream &Configuration::dump(ostream &o, const string &indentstr)
{
	xmltree->dump(o, indentstr);
	return o;
}

void Configuration::write_back(void)
{
	if(!is_file)
		return;	// Don't write back if not from a file
	
	std::ofstream ofile;
	try {
	U7open(ofile, filename.c_str(), true);
	} catch (const file_open_exception &)
	{
		std::perror("Failed to write configuration file");
		return;
	}
	if(ofile.fail())
	{
		std::perror("Failed to write configuration file");
		return;
	}
	ofile << dump() << endl;
	ofile.close();
}


std::vector<string>	Configuration::listkeys(const string &key, bool longformat)
{
	std::vector<string>	vs;
	const XMLnode *sub=xmltree->subtree(key);
	if(sub)
		sub->listkeys(key,vs,longformat);
	
	return vs;
}

std::vector<string>	Configuration::listkeys(const char *key, bool longformat)
{
	string s(key);
	return listkeys(s,longformat);
}

void Configuration::clear(const string &new_root)
{
	CTRACE("Configuration::clear");
	
	if(xmltree!=0)
		delete xmltree;
	CTRACE("Configuration::clear - xmltree deleted");
	if(new_root.size())
		rootname=new_root;
	CTRACE("Configuration::clear - new root specified");
	xmltree = new XMLnode(rootname);
	CTRACE("Configuration::clear - fin");
}

void Configuration::getsubkeys(KeyTypeList &ktl, const string &basekey)
{
	xmltree->searchpairs(ktl, basekey, string(), 0);
}





