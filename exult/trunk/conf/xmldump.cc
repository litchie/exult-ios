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

static	string	encode_entity(string &s)
{
	string	ret("");

	for(size_t i=0;i<s.length();i++)
		{
		switch(s[i])
			{
			case '<':
				ret+="&lt;";
				break;
			case '>':
				ret+="&gt;";
				break;
			case '&':
				ret+="&amp;";
				break;
			default:
				ret+=s[i];
			}
		}
	return ret;
}

static	string	indent(int depth)
{
	string s("");

	for(int i=0;i<depth;i++)
		s+=' ';
	return s;
}

void	xmldump(string &s,XMLnode *x,int depth)
{
	s+=indent(depth);
	
	s+="<";
	s+=x->entity.id;
	s+=">\n";
	if(x->entity.id[x->entity.id.length()-1]!='/')
		{
		for(vector<XMLnode>::iterator it=x->nodelist.begin();
			it!=x->nodelist.end();
			++it)
			{
			xmldump(s,it,depth+1);
			}

		if(x->entity.content.length())
			{
			s+=indent(depth);
			s+=encode_entity(x->entity.content);
			}
		if(x->entity.id[0]=='?')
			{
			return;
			}
		if(x->entity.content.length())
			s+="\n";

		s+=indent(depth);
		s+="</";
		s+=x->entity.id;
		s+=">\n";
		}
}
