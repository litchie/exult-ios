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

void	xmlparse(string &s,size_t &pos,XMLnode *x)
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
//					cout << "End of entity '"<<x->entity.id <<"' ("<<x->entity.content<<")"<<endl;
					return;
					}
				XMLnode t;
				++pos;
				xmlparse(s,pos,&t);
				x->nodelist.push_back(t);
				break;
				}
			case '>':
				// End of tag
				++pos; intag=false; if(s[pos]<32) ++pos;
				break;
			default:
				if(intag)
					x->entity.id+=s[pos++];
				else
					{
					if((s[pos]==' '||s[pos]=='\t'||s[pos]==0x0d||s[pos]==0x0a)&&(x->entity.content.length()==0))
	++pos;
					else
					x->entity.content+=s[pos++];
					}
			}
		}
	while(x->entity.content.length()>0&&x->entity.content[x->entity.content.length()-1]<32)
		{
		x->entity.content.erase(x->entity.content.length()-1);
		}
//					cout << "End of entity '"<<x->entity.id <<"' ("<<x->entity.content<<")"<<endl;
}

