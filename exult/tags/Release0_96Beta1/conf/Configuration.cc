/*
 *  Copyright (C) 2000-2001  The Exult Team
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
#  include <config.h>
#endif

#include "exult_constants.h"
#include "Configuration.h"
#include "exceptions.h"

#ifndef ALPHA_LINUX_CXX
#  include <cstdio>
#endif
#include <iostream>
#include "utils.h"


using std::atoi;
using std::cerr;
using std::endl;
using std::FILE;
using std::snprintf;
using std::string;

void	Configuration::value(const char *key,std::string &ret,const char *defaultvalue) const
{
	const XMLnode *sub=xmltree.subtree(string(key));
	if(sub)
		ret = sub->value();
	else
		ret = defaultvalue;
}

void	Configuration::value(const char *key,bool &ret,bool defaultvalue) const
{
	const XMLnode *sub=xmltree.subtree(string(key));
	if(sub)
		ret = (to_uppercase(sub->value()) == "YES");
	else
		ret = defaultvalue;
}

void	Configuration::value(const char *key,int &ret,int defaultvalue) const
{
	const XMLnode *sub=xmltree.subtree(string(key));
	if(sub)
		ret = atoi(sub->value().c_str());
	else
		ret = defaultvalue;
}

void	Configuration::set(std::string &key,std::string &value,bool write_out)
{
	// Break k up into '/' separated elements.
	// start advancing walk, one element at a time, creating nodes
	// as needed.
	// At the end of that, walk is the target node, and we
	// can set the value.

	// We must also properly encode the value before writing it out.
	// Must remember that.

	xmltree.xmlassign(key,value);
	if(write_out)
		write_back();
}

void	Configuration::set(const char *key,const char *value,bool write_out)
{
	std::string	k(key),v(value);
	set(k,v,write_out);
}

void	Configuration::set(const char *key,const std::string &value,bool write_out)
{
	std::string	k(key),v(value);
	set(k,v,write_out);
}

void	Configuration::set(const char *key,int value,bool write_out)
{
	std::string	k(key),v;
	char	buf[32];

	snprintf(buf,32,"%d",value);
	v=buf;
	set(k,v,write_out);
}


bool	Configuration::read_config_string(const std::string &s)
{
	std::string	sbuf(s);
	std::size_t	nn=1;
	xmltree.xmlparse(sbuf,nn);
	is_file=false;
	return true;
}

bool	Configuration::read_config_file(const char *n)
{
	char		buf[4096];
	std::string	sbuf;

	filename=n;
	// Don't frob the filename if it starts with a dot and
	// a slash.
	if(filename.find("./")!=0)
	{
#if ((defined XWIN) || (defined BEOS))
		const char *f1=getenv("HOME");
		if(f1)
		{
			// User has a home directory
			filename=f1;
#ifndef BEOS
			filename+="/.";
#else
			filename+="/config/settings/";
#endif
			filename+=n;
		}
		else
			filename=n;
#else
		// Probably something to do with deteriming the username
		// and generating a filename in their personal setup area.

		// For now, just read file from current directory
		filename=n;
#endif
	}

	is_file=true; // set to file, even if file not found

	FILE *fp;
	try {
	        fp=U7open(filename.c_str(),"r");
	}
	catch(exult_exception &e) {
	        // configuration file not found
	        return false;
	}

	if(!fp)
		return false;

	while(fgets(buf,sizeof(buf),fp))
		sbuf+=buf;

	fclose(fp);
	read_config_string(sbuf);
	is_file=true;
	return true;
}


std::string	Configuration::dump(void)
{
	return xmltree.dump();
}


void	Configuration::write_back(void)
{
	if(!is_file)
		return;	// Don't write back if not from a file
	std::string	s=dump();
	FILE *fp=U7open(filename.c_str(),"w");
	if(!fp)
	{
		std::perror("Failed to write configuration file");
		return;
	}
	fwrite(s.c_str(),s.size(),1,fp);
	fclose(fp);
}


std::vector<std::string>	Configuration::listkeys(const std::string &key,bool longformat)
{
	std::vector<std::string>	vs;
	const XMLnode *sub=xmltree.subtree(key);
	if(sub)
		sub->listkeys(key,vs,longformat);
	
	return vs;
}

std::vector<std::string>	Configuration::listkeys(const char *key,bool longformat)
{
	std::string s(key);
	return listkeys(s,longformat);
}

