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

#if __GNUG__ >= 2
#  pragma implementation
#endif

#include "exult_constants.h"
#include "XMLEntity.h"

using std::string;
using std::vector;

static	string	encode_entity(const string &s);
static	string	close_tag(const string &s);


const string	&XMLnode::reference(const string &h,bool &exists)
{
	if(h.find('/')==string::npos)
	{
		// Must refer to me.
		if(id==h)
		{
			exists = true;
			return content;
		}
	}
	else
	{
		// Otherwise we want to split the string at the first /
		// then locate the branch to walk, and pass the rest
		// down.

		string k;
		k=h.substr(h.find('/')+1);
		string k2=k.substr(0,k.find('/'));
		for(std::vector<XMLnode*>::iterator it=nodelist.begin();
			it!=nodelist.end();++it)
		{
			if((*it)->id==k2)
				return (*it)->reference(k,exists);
		}
	}
	
	exists = false;
	return c_empty_string;
}


const XMLnode *XMLnode::subtree(const string &h) const
{
	if(h.find('/') == string::npos)
	{
		// Must refer to me.
		if(id == h)
			return this;
	}
	else
	{
		// Otherwise we want to split the string at the first /
		// then locate the branch to walk, and pass the rest
		// down.

		string k;
		k=h.substr(h.find('/')+1);
		string k2=k.substr(0,k.find('/'));
		for(std::vector<XMLnode*>::const_iterator it=nodelist.begin();
			it!=nodelist.end();++it)
		{
			if((*it)->id==k2)
				return (*it)->subtree(k);
		}
	}

	return 0;
}


string	XMLnode::dump(int depth)
{
	string s(depth,' ');

	s+="<";
	s+=id;
	s+=">\n";
	if(id[id.length()-1]!='/')
	{
		for(std::vector<XMLnode*>::const_iterator it=nodelist.begin();
			it!=nodelist.end();
			++it)
		{
			s += (**it).dump(depth+1);
		}

		if(content.length())
		{
			s += string(depth,' ');
			s += encode_entity(content);
		}
		if(id[0]=='?')
		{
			return s;
		}
		if(content.length())
			s += "\n";

		s += string(depth,' ');
		s += "</";
		s += close_tag(id);
		s += ">\n";
	}
	
	return s;
}


// This function does not make sense here. It should be in XMLEntity
void	XMLnode::xmlassign(string &key,string &value)
{
	if(key.find('/')==string::npos)
	{
		// Must refer to me.
		if(id==key)
			content = value;
		else
			CERR("Walking the XML tree failed to create a final node.");
		return;
	}
	string k;
	k=key.substr(key.find('/')+1);
	string k2=k.substr(0,k.find('/'));
	for(std::vector<XMLnode*>::iterator it=nodelist.begin();it!=nodelist.end();++it)
	{
		if((*it)->id==k2)
		{
			(**it).xmlassign(k,value);
			return;
		}
	}
	
	// No match, so create a new node and do recursion
	XMLnode *t = new XMLnode(k2);
	nodelist.push_back(t);
	(*t).xmlassign(k,value);
}


void	XMLnode::listkeys(const string &key,vector<string> &vs, bool longformat) const
{
	string s(key);
	s+="/";

	for(std::vector<XMLnode*>::const_iterator it=nodelist.begin();
		it!=nodelist.end(); ++it)
	{
		if(!longformat)
			vs.push_back((*it)->id);
		else
			vs.push_back(s + (*it)->id);
	}
}

static	string	encode_entity(const string &s)
{
	string	ret;

	for(string::const_iterator it=s.begin();it!=s.end();++it)
	{
		switch(*it)
		{
			case '<':
				ret+="&lt;";
				break;
			case '>':
				ret+="&gt;";
				break;
			case '"':
				ret+="&quot;";
				break;
			case '\'':
				ret+="&apos;";
				break;
			case '&':
				ret+="&amp;";
				break;
			default:
				ret += *it;
		}
	}
	return ret;
}

static	string	decode_entity(string &s,std::size_t &pos)
{
	std::size_t			old_pos = pos;
	string::size_type	entity_name_len = s.find_last_not_of("; \t\r\n", pos) -pos ;
	string				entity_name = s.substr(pos+1, entity_name_len);
	
	pos += entity_name_len + 2;
	
	if(s == "amp")
		return string("&");
	else
	if(s == "apos")
		return string("'");
	else
	if(s == "quot")
		return string("\"");
	else
	if(s == "lt")
		return string("<");
	else
	if(s == "gt")
		return string(">");
	
	return s.substr(old_pos, entity_name_len+2);
}

static	string	close_tag(const string &s)
{
	if(s.find(" ")==string::npos)
		return s;
	
	return s.substr(0,s.find(" "));
}


static	void	trim(string &s)
{
	// Clean off leading whitespace
	while(s.length()&&s[0]<=32)
	{
		s=s.substr(1);
	}
	// Clean off trailing whitespace
	while(s.length()&&s[s.length()-1]<=32)
	{
		s.erase(s.length()-1);
	}
}

void	XMLnode::xmlparse(string &s,std::size_t &pos)
{
	bool	intag = true;
	
	id = "";
	while(pos<s.length())
	{
		switch(s[pos])
		{
			case '<':
			{
				// New tag?
				if(s[pos+1]=='/')
				{
					// No. Close tag.
					while(s[pos]!='>')
						pos++;
					pos++;
					trim(content);
					return;
				}
				XMLnode *t = new XMLnode;
				++pos;
				t->xmlparse(s,pos);
				nodelist.push_back(t);
				break;
			}
			case '>':
				// End of tag
				if(s[pos-1]=='/')
				{
					++pos;
					return; // An empty tag
				}
				++pos;
				intag = false;
				if(s[pos]<32)
					++pos;
				break;
			case '&':
				content+=decode_entity(s,pos);
				break;
			default:
				if(intag)
					id += s[pos++];
				else
					content += s[pos++];
		}
	}
	trim(content);
}

