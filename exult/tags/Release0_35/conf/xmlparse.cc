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


static	void	trim(std::string &s)
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

static	bool	grab_entity(std::string &s,std::size_t &pos,const char *ent)
{
	std::string	t(s.substr(pos));
	if(t.find(ent)==0)
		{
		pos+=std::strlen(ent)+1;
		return true;
		}
	return false;
}

static	std::string	entity_decode(std::string &s,std::size_t &pos)
{
	// This is ugly and slow
	++pos;	// Advance past amp
	if(grab_entity(s,pos,"amp"))
		return std::string("&");
	else
	if(grab_entity(s,pos,"gt"))
		return std::string(">");
	else
	if(grab_entity(s,pos,"lt"))
		return std::string("<");
	return std::string("&");
}

void	xmlparse(std::string &s,std::size_t &pos,XMLnode *x)
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

