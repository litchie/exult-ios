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
#include <iostream>

using std::string;
using std::vector;
using std::ostream;
using std::endl;

static	string	encode_entity(const string &s);
static	string	close_tag(const string &s);

XMLnode::~XMLnode()
{
	for(vector<XMLnode *>::iterator i=nodelist.begin(); i!=nodelist.end(); i++)
		delete *i;
}

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

		if(!no_close)
		{
		s += string(depth,' ');
		
			s += "</";
			s += close_tag(id);
			s += ">\n";
		}
	}
	
	return s;
}

/* Output's a 'nicer' dump of the xmltree, one <tag> value </tag> per line
	the indent characters are specified by indentstr */
void XMLnode::dump(ostream &o, const string &indentstr, const unsigned int depth) const
{
	// indent
	for(unsigned int i=0; i<depth; i++)
		o << indentstr;
	
	// open tag
	o << '<' << id << '>';
	
	// if this tag has a closing tag...
	if(id[id.length()-1]!='/')
	{
		// if we've got some subnodes, terminate this line...
		if(nodelist.size())
		{
			o << endl;
		
			// ... then walk through them outputting them all ...
			for(vector<XMLnode *>::const_iterator it=nodelist.begin(); it!=nodelist.end(); ++it)
				(*it)->dump(o, indentstr, depth+1);
		}
		// ... else, if we have content in this output it.
		else if(content.length())
			o << ' ' << encode_entity(content) << ' ';
		
		// not a clue... it's in XMLnode::dump() so there must be a reason...
		if(id[0]=='?')
			return;
		
		// append a closing tag if there is one.
		if(!no_close)
		{
			// if we've got subnodes, we need to reindent
			if(nodelist.size())
			for(unsigned int i=0; i<depth; i++)
				o << indentstr;
			
			o << "</" << close_tag(id) << '>';
		}
	}
	o << endl;
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
	string::size_type	entity_name_len = s.find_first_of("; \t\r\n", pos) -pos -1;
	
	/* Call me paranoid... but I don't think having an end-of-line or similar
		inside a &...; expression is 'good', valid though it may be. */
	assert(s[pos+entity_name_len+1]==';');
	
	string				entity_name = s.substr(pos+1, entity_name_len);
	
	pos += entity_name_len + 2;
	
	// std::cout << "DECODE: " << entity_name << endl;
	
	if     (entity_name == "amp")  return string("&");
	else if(entity_name == "apos") return string("'");
	else if(entity_name == "quot") return string("\"");
	else if(entity_name == "lt")   return string("<");
	else if(entity_name == "gt")   return string(">");
	
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
					++pos;
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
					if(s[pos-2]=='<')
					{
						++pos;
						return; // An empty tag
					}
					else
					{
						++pos;
						no_close=true;
						return;
					}
				}
				else if((id[0]=='!') && (id[1]=='-') && (id[2]=='-'))
				{
					++pos;
					no_close=true;
					return;
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

void XMLnode::searchpairs(KeyTypeList &ktl, const string &basekey, const string currkey, const unsigned int pos)
{
	//std::cout << basekey << std::endl << '\t' << currkey + id << std::endl << "\t\t" << content << std::endl;
	
	if(basekey==currkey+id)
		for(vector<XMLnode *>::iterator i=nodelist.begin(); i!=nodelist.end(); i++)
			(*i)->selectpairs(ktl, "");
	else
		for(vector<XMLnode *>::iterator i=nodelist.begin(); i!=nodelist.end(); i++)
			(*i)->searchpairs(ktl, basekey, currkey + id + '/', pos);
}

void XMLnode::selectpairs(KeyTypeList &ktl, const std::string currkey)
{
	//std::cout << '>' << currkey + id << std::endl << '\t' << content << std::endl;
	
	ktl.push_back(KeyType(currkey + id, content));
	
	for(vector<XMLnode *>::iterator i=nodelist.begin(); i!=nodelist.end(); i++)
		(*i)->selectpairs(ktl, currkey + id + '/');
}





