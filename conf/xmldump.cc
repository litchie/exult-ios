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

void	xmldump(string &s,XMLnode *x,int depth)
{
	for(int i=0;i<depth;i++)
		s+=' ';
	
	s+="<";
	s+=x->entity.id;
	s+=">\n";
	
	for(vector<XMLnode>::iterator it=x->nodelist.begin();
		it!=x->nodelist.end();
		++it)
		{
		for(int i=0;i<depth;i++)
			s+=' ';
		xmldump(s,it,depth+1);
		}
	for(int i=0;i<depth+1;i++)
		s+=' ';
	s+=x->entity.content;
	if(x->entity.id[0]=='?')
		{
		return;
		}
	s+="\n";
	for(int i=0;i<=depth;i++)
		s+=' ';
	
	s+="</";
	s+=x->entity.id;
	s+=">\n";
}
