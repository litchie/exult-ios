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


#include "XMLEntity.h"
#include <iostream>

using std::string;

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

static	string	entity_decode(string &s,std::size_t &pos)
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

void	xmlparse(string &s,std::size_t &pos,XMLnode *x)
{
	bool	intag=true;
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
					trim(x->entity.content);
//					cout << "End of entity(1) '"<<x->entity.id <<"' ("<<x->entity.content<<")"<<endl;
					return;
				}
				XMLnode *t = new XMLnode;
				++pos;
				xmlparse(s,pos,t);
				x->nodelist.push_back(t);
				break;
			}
			case '>':
				// End of tag
				if(s[pos-1]=='/')
				{
					++pos;
					return; // An empty tag
				}
				++pos; intag=false; if(s[pos]<32) ++pos;
				break;
			case '&':
				x->entity.content+=entity_decode(s,pos);
				break;
			default:
				if(intag)
					x->entity.id+=s[pos++];
				else
				{
					x->entity.content+=s[pos++];
				}
		}
	}
	trim(x->entity.content);
//	cout << "End of entity(2) '"<<x->entity.id <<"' ("<<x->entity.content<<")"<<endl;
}

