
/*
Copyright (C) 2000  Dancer A.L Vesperman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#if __GNUG__ >= 2
#  pragma implementation
#endif


#include "Configuration.h"

#include <cstdio>
#include <iostream>
#include "utils.h"


using std::atoi;
using std::cerr;
using std::endl;
using std::FILE;
using std::perror;
using std::size_t;
using std::sprintf;

Configuration::Configuration() : xmltree(),filename(""),is_file(false)
{}

Configuration::Configuration(const char *s) : xmltree(),filename(""),is_file(false)
{
	read_config_file(s);
}

Configuration::~Configuration()
{}


std::string	&Configuration::value(const char *key,bool &exists)
{
	std::string	s=key;
	exists=false;
	XMLnode *sub=xmltree.subtree(s);
	if(!sub)
	{
		static std::string dummy("");
		exists=false;
		return dummy;
	}
	else
	{
		exists=true;
		return sub->entity.content;
	}
}

void	Configuration::value(const char *key,std::string &s,const char *defaultvalue)
{
	bool	exists;
	s=value(key,exists);
	if(!exists)
		s=defaultvalue;
}

void	Configuration::value(const char *key,int &n,int defaultvalue)
{
	n=0;
	std::string	s;
	bool	exists;

	s=value(key,exists);
	if(!exists)
		n=defaultvalue;
	else
		n=atoi(s.c_str());
}

// This function does not make sense here. It should be in XMLEntity
static void	xmlassign(XMLnode *walk,std::string &key,std::string &value)
{
	// cout << "xmlassign(" << key << "," << value << ")"<<endl;
	if(key.find('/')==std::string::npos)
	{
		// Must refer to me.
		if(walk->entity.id==key)
			walk->entity.content=value;
		else
			cerr << "Walking the XML tree failed to create a final node." << endl;
		return;
	}
	std::string k;
	k=key.substr(key.find('/')+1);
	std::string k2=k.substr(0,k.find('/'));
	for(std::vector<XMLnode*>::iterator it=walk->nodelist.begin();it!=walk->nodelist.end();++it)
	{
		if((*it)->entity.id==k2)
		{
			xmlassign(*it,k,value);
			return;
		}
	}
	XMLnode *t = new XMLnode;
	t->entity.id=k2;
	walk->nodelist.push_back(t);
	// cout << "New node " << k2 << endl;
	std::vector<XMLnode*>::reverse_iterator rit=walk->nodelist.rbegin();
	xmlassign(*rit,k,value);
}

void	Configuration::set(std::string &key,std::string &value,bool write_out)
{
	std::string	k=key;
	XMLnode *walk;

	walk=&xmltree;

	// Break k up into '/' separated elements.
	// start advancing walk, one element at a time, creating nodes
	// as needed.
	// At the end of that, walk is the target node, and we
	// can set the value.

	// We must also properly encode the value before writing it out.
	// Must remember that.
	if(xmltree.entity.id.length()==0)
	{
		std::string k;
		k=key.substr(0,key.find('/'));
		xmltree.entity.id=k;
	}
	xmlassign(walk,k,value);
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

	sprintf(buf,"%d",value);
	v=buf;
	set(k,v,write_out);
}



extern	void	xmlparse(std::string &s,size_t &pos,XMLnode *x);

bool	Configuration::read_config_string(const std::string &s)
{
	std::string	sbuf(s);
	size_t		nn=1;
	xmlparse(sbuf,nn,&xmltree);
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
			filename+="/.";
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
	FILE	*fp=U7open(filename.c_str(),"r");

	is_file=true; // set to file, even if file not found

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
	extern	void xmldump(std::string &,XMLnode *,int);
	std::string	out("");
	xmldump(out,&xmltree,0);
	return out;
}


void	Configuration::write_back(void)
{
	if(!is_file)
		return;	// Don't write back if not from a file
	std::string	s=dump();
	FILE *fp=U7open(filename.c_str(),"w");
	if(!fp)
	{
		perror("Failed to write configuration file");
		return;
	}
	fwrite(s.c_str(),s.size(),1,fp);
	fclose(fp);
}


std::vector<std::string>	Configuration::listkeys(std::string &key,bool longformat)
{
	std::vector<std::string>	vs;
	XMLnode *sub=xmltree.subtree(key);
	if(!sub)
		return vs;

	for(std::vector<XMLnode*>::const_iterator it=sub->nodelist.begin();
		it!=sub->nodelist.end(); ++it)
	{
		std::string	s=key;
		s+="/";
		if(!longformat)
			s="";
		s+=(*it)->entity.id;
		vs.push_back(s);
	}

	return vs;
}

std::vector<std::string>	Configuration::listkeys(const char *key,bool longformat)
{
	std::string s(key);
	return listkeys(s,longformat);
}

